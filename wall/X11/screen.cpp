// Copyright (C) 2010, Guy Barrand. All rights reserved.
// See the file wall.license for terms.

#include "../wall/screen_main"

#include <exlib/sg/pv_holder>
#include <exlib/sg/GL_VIEWER>

#include <exlib/X11/session>

namespace wall {

//IMPORTANT : pv_holder must come first.

class main : protected exlib::sg::pv_holder, public common::main {
  typedef common::main parent;
public:
  EXLIB_GL_VIEWER_METHODS
public: //inlib::sg::iui
  virtual void win_render() {
    if(::glXMakeCurrent(m_display,m_win,m_ctx)==False){
      parent::m_out << "main::win_render : glXMakeCurrent failed." << std::endl;
      return;
    }
    render();
    ::glXSwapBuffers(m_display,m_win);
  }
public:
  main(std::ostream& a_out,const std::string& a_res_dir,const std::string& a_tmp_dir,
       Display* a_display,Window a_win,GLXContext a_ctx,
       wall::screen_client& a_screen_client)
  :exlib::sg::pv_holder(a_out)
  ,parent(a_out,exlib::sg::pv_holder::m_mgr_gra,exlib::sg::pv_holder::m_ttf,
          a_res_dir,a_tmp_dir,a_screen_client,false,0)
  ,m_display(a_display)
  ,m_win(a_win)
  ,m_ctx(a_ctx)
  {
    push_home();
  }
  virtual ~main(){}

private:
  Display* m_display;
  Window m_win;
  GLXContext m_ctx;
};

}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

#include <inlib/args>
#include <inlib/app>

#include <iostream>
#include <cstdlib>

#ifdef INLIB_MEM
#include <inlib/mem>
#endif

int main(int argc,char** argv) {
  bool verbose = false;

#ifdef INLIB_MEM
  inlib::mem::set_check_by_class(true);{
#endif

  ////////////////////////////////////////////////////////////////
  /// wall ///////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
  wall::screen_client screen_client(std::cout);

  inlib::args args(argc,argv);
  bool params_from_env = false;
  std::ostream& wall_out = std::cout;
  wall::screen_client& wclient = screen_client;
#include "../wall/screen_client_main.icc"
  if(!wall_status) return EXIT_FAILURE;

  ////////////////////////////////////////////////////////////////
  /// X11 ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
  exlib::X11::session x11(std::cout);
  if(!x11.display()) return EXIT_FAILURE;

  Window win = x11.create_window("wall_screen_X11",window_x,window_y,window_width,window_height);
  if(win==0L) return EXIT_FAILURE;
  if(full_screen){
    int dw = ::XDisplayWidth(x11.display(),::XDefaultScreen(x11.display()));
    int dh = ::XDisplayHeight(x11.display(),::XDefaultScreen(x11.display()));
    ::XMoveResizeWindow(x11.display(),win,0,0,dw,dh);
    x11.set_wm_no_decorations(win);
  } else {
    if(window_no_decorations) {
      x11.set_override_redirect(win);
      //x11.set_wm_no_decorations(win);
    }
  }
  x11.map_raise_window(win);
  //x11.show_window(win);

  ////////////////////////////////////////////////////////////////
  /// wall::main /////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
 {std::string exe_path; //for res_dir
  if(!inlib::program_path(argv[0],exe_path)) {
    std::cout << "wall_screen_X11 : can't get exe path. Continue anyway."
              << std::endl;
    return EXIT_FAILURE;
  }
  std::string res_dir = exe_path+"/../res";
  std::string tmp_dir;
  inlib::tmpdir(tmp_dir);
  wall::main wmain(std::cout,res_dir,tmp_dir,x11.display(),win,x11.context(),screen_client);
  wmain.cursor_visible.value(true);
  wmain.cursor_line_width.value(10);
  wmain.set_menu_pos(menu_col,menu_row);

  if(DOCUMENT.size()) {
    if(!inlib::file::is_empty(DOCUMENT)) {
      wmain.create_gui();
      bool done;
      wmain.opener().open(DOCUMENT,inlib::file::s_format_guessed(),inlib::args(),done);
    }
    //DOCUMENT removed at end because of exlib::iv::file that does not
    //open the file in opener.open().
  }

  //////////////////////////////////////////////////////////////////
  /// steering /////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  while(true) {
    XEvent xevent;
    ::XNextEvent(x11.display(),&xevent);
    if((xevent.type==Expose)&&(xevent.xany.window==win)) {
      wmain.win_render();
      break;
    } else if(xevent.type==ConfigureNotify) {
      int width,height;
      x11.window_size(win,width,height);
      wmain.set_size(width,height);
      wmain.win_render();
    }
  }

  int fd_X11 = ::XConnectionNumber(x11.display());
  std::vector<int> socks;
  socks.push_back(fd_X11);
  socks.push_back(screen_client.socket().socket());
  std::vector<bool> has_input;

  //Atom atom = ::XInternAtom(x11.display(),"WM_DELETE_WINDOW",False);
  while(true) { 
    //std::cout << "wall_screen_X11 : loop..." << std::endl;

   {inlib::wall::event event;
    while(screen_client.get_event(event)) {
      bool to_quit;
      screen_dispatch(wmain,screen_client,std::cout,event,to_quit);
      if(to_quit) break;
    }}

    if(wmain.num_cbks()) {
      wmain.do_works();

      if(wmain.to_exit()) {             
        if(!screen_client.send_once(inlib::wall::EVT_USER_QUIT())) break;
      }

      if(::XPending(x11.display())) {
        XEvent xevent;
        ::XNextEvent(x11.display(),&xevent);
        if(xevent.type==Expose) {
          wmain.win_render();
        } else if(xevent.type==ConfigureNotify) {
          int width,height;
          x11.window_size(win,width,height);
          wmain.set_size(width,height);
          wmain.win_render();
        }
      }

     {bool have_input;
      if(!screen_client.socket().is_there_input(have_input)) {
        std::cout << "exlib_main :"
                  << " inlib::net::is_there_input failed."
                  << std::endl;
        break;
      }
      if(have_input) {
        inlib::wall::event event;
        if(!wait_event(screen_client.socket(),event)) {
          std::cout << "wall_screen_X11 :"
                    << " wait_event failed."
                    << std::endl;
          break;
        }
        bool to_quit = false;
        screen_dispatch(wmain,screen_client,std::cout,event,to_quit);
        if(to_quit) break;
      }}

      continue;
    }

    if(!inlib::net::wait_input(socks,has_input)) {
      std::cout << "wall_screen_X11 :"
                << " inlib::net::wait_input failed."
                << std::endl;
      break;
    }

    if(::XPending(x11.display())) {

      XEvent xevent;
      ::XNextEvent(x11.display(),&xevent);
      if(xevent.type==Expose) {
        wmain.win_render();
      } else if(xevent.type==ConfigureNotify) {
        int width,height;
        x11.window_size(win,width,height);
        wmain.set_size(width,height);
        wmain.win_render();
      }

    } else {

      inlib::wall::event event;
      if(!wait_event(screen_client.socket(),event)) {
        std::cout << "wall_screen_X11 :"
                  << " wait_event failed."
                  << std::endl;
        break;
      }
      bool to_quit = false;
      screen_dispatch(wmain,screen_client,std::cout,event,to_quit);
      if(to_quit) break;

    }

  } //while

  } //wmain

  if(win!=0L) x11.delete_window(win);

  if(DOCUMENT.size()) ::remove(DOCUMENT.c_str());

#ifdef INLIB_MEM
  }inlib::mem::balance(std::cout);
  std::cout << "wall_screen_X11 : exit(mem) ..." << std::endl;
#else  
  if(verbose) std::cout << "wall_screen_X11 : exit ..." << std::endl;
#endif

  return EXIT_SUCCESS;
}
