// Copyright (C) 2010, Guy Barrand. All rights reserved.
// See the file wall.license for terms.

#include "../wall/screen_main"

#include <exlib/sg/pv_holder>
#include <exlib/sg/GL_VIEWER>

#include <exlib/Windows/session>
#include <exlib/Windows/window>
#include <exlib/Windows/glarea>

namespace wall {

//IMPORTANT : pv_holder must come first.

class main
:protected exlib::sg::pv_holder
,public common::main 
,public exlib::Windows::window
,public exlib::Windows::glarea
{
  typedef common::main parent;
public:
  EXLIB_GL_VIEWER_METHODS
public: //inlib::sg::iui
  virtual void win_render() {wm_paint();}
public: //exlib::Windows::glarea
  virtual void resize(unsigned int a_w,unsigned int a_h){
    //NOTE : this is not called at startup.
    set_size(a_w,a_h);
  }
  virtual void paint(unsigned int a_w,unsigned int a_h) {
    if(!width() && !height()) set_size(a_w,a_h);
    render();
  }
public:
  main(std::ostream& a_out,const std::string& a_res_dir,const std::string& a_tmp_dir,
       unsigned int a_w,unsigned int a_h,unsigned int a_mask,
       wall::screen_client& a_screen_client)
  :exlib::sg::pv_holder(a_out)
  ,parent(a_out,exlib::sg::pv_holder::m_mgr_gra,exlib::sg::pv_holder::m_ttf,
          a_res_dir,a_tmp_dir,a_screen_client,false,0)
  ,exlib::Windows::window(a_w,a_h,a_mask)
  ,exlib::Windows::glarea(exlib::Windows::window::m_hwnd)
  {
    push_home();
  }
  virtual ~main(){}
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
  /// Windows ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
  exlib::Windows::session wdoz(std::cout);

  unsigned int window_mask = 0;
  if(full_screen){
    //window_mask = WS_OVERLAPPEDWINDOW;
    //int hborder = ::GetSystemMetrics(SM_CYCAPTION);
    //window_height += hborder;
    window_mask = WS_POPUP;
    window_mask |= WS_MAXIMIZE;
  } else {
    if(window_no_decorations) {
      window_mask = WS_POPUP;
    } else {
      window_mask = WS_OVERLAPPEDWINDOW;
      int hborder = ::GetSystemMetrics(SM_CYCAPTION);
      window_height += hborder;
    }
  }

  ////////////////////////////////////////////////////////////////
  /// wall::main /////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
 {std::string exe_path; //for res_dir
  if(!inlib::program_path(argv[0],exe_path)) {
    std::cout << "wall_screen : can't get exe path. Continue anyway."
              << std::endl;
    return EXIT_FAILURE;
  }
  std::string res_dir = exe_path+"/../res";
  std::string tmp_dir;
  inlib::tmpdir(tmp_dir);
  wall::main wmain(std::cout,res_dir,tmp_dir,
                   window_width,window_height,window_mask,screen_client);
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

  ::MoveWindow(wmain.window::hwnd(),
               window_x,window_y,window_width,window_height,TRUE);

  wdoz.show_window(wmain.window::hwnd());

  /*
  int fd_win32 = 0;
  std::vector<int> socks;
  socks.push_back(fd_win32);
  socks.push_back(screen_client.socket().socket());
  */

  while(true) { 
    //std::cout << "wall_screen : loop..." << std::endl;

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

     {MSG event;
      if(::PeekMessage(&event,NULL,0,0,PM_REMOVE)) {
        ::TranslateMessage(&event);
        ::DispatchMessage(&event);
      }}
 
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
          std::cout << "wall_screen :"
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

    /*
    if(!inlib::net::wait_input(socks)) {
      std::cout << "wall_screen :"
                << " inlib::net::wait_input failed."
                << std::endl;
      break;
    }
    */

    // How to wait on both Windows thread and screen_client socket ?
  /*
    if(socks.size()==1) {
      while(true){
        MSG event;
        if(::PeekMessage(&event,NULL,0,0,PM_NOREMOVE)) break;
      }
      } else*/ { //2
      bool status = true;
      while(true){
        MSG event;
        if(::PeekMessage(&event,NULL,0,0,PM_NOREMOVE)) break;
        bool have_input;
        if(!screen_client.socket().is_there_input(have_input)) {
          std::cout << "wall_screen :"
                    << " inlib::net::is_there_input failed."
                    << std::endl;
          status = false;
          break;
        }
        if(have_input) break;
      }
      if(!status) break;
    }

   {MSG event;
    if(::PeekMessage(&event,NULL,0,0,PM_REMOVE)) {
      ::TranslateMessage(&event);
      ::DispatchMessage(&event);

    } else { //from screen_client.

      inlib::wall::event event;
      if(!wait_event(screen_client.socket(),event)) {
        std::cout << "wall_screen :"
                  << " wait_event failed."
                  << std::endl;
        break;
      }
      bool to_quit = false;
      screen_dispatch(wmain,screen_client,std::cout,event,to_quit);
      if(to_quit) break;

    }}

  } //while

  } //wmain

  if(DOCUMENT.size()) ::remove(DOCUMENT.c_str());

#ifdef INLIB_MEM
  }inlib::mem::balance(std::cout);
#endif

  if(verbose) {
    std::cout << "wall_screen : exit..." << std::endl;
  }

  return EXIT_SUCCESS;
}
