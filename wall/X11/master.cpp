// Copyright (C) 2010, Guy Barrand. All rights reserved.
// See the file wall.license for terms.

#ifdef INLIB_MEM
#include <inlib/mem>
#endif

#include "../wall/master_dispatch"
#include "../wall/master_sg_serv"

inline bool is_there_input(int a_X11_sock,
                           screens_t& a_screens,
                           inlib::net::inet_socket& a_sg_serv,
                           bool& a_has) {
  std::vector<int> socks;
  socks.push_back(a_X11_sock);
  screens_t::iterator it;
  for(it=a_screens.begin();it!=a_screens.end();++it) {
    inlib::net::base_socket* client = *it;
    socks.push_back(client->socket());
  }
  if(a_sg_serv.is_connected()) socks.push_back(a_sg_serv.socket());
  return inlib::net::is_there_input(socks,a_has);
}

inline bool wait_input(int a_X11_sock,
                       screens_t& a_screens,
                       inlib::net::inet_socket& a_sg_serv){
  std::vector<int> socks;
  socks.push_back(a_X11_sock);
  //std::cout << "sock X11 : " << a_X11_sock << std::endl;
  screens_t::iterator it;
  for(it=a_screens.begin();it!=a_screens.end();++it) {
    inlib::net::base_socket* client = *it;
    socks.push_back(client->socket());
    //std::cout << "sock : " << client->socket() << std::endl;
  }
  if(a_sg_serv.is_connected()) socks.push_back(a_sg_serv.socket());

  std::vector<bool> has_input;
  return inlib::net::wait_input(socks,has_input);
}

#include "../wall/master_render"

#include <X11/Xutil.h>
#include <X11/keysym.h>

inline void X2wall(KeySym a_sym,unsigned int a_state,
                   inlib::wall::event& a_event){
  a_event.type = inlib::wall::EVT_KEYDOWN;
  a_event.key.type = inlib::wall::EVT_KEYDOWN;
    
  if(a_sym==XK_Left) a_event.key.keysym.sym = inlib::wall::KEY_LEFT;
  else if(a_sym==XK_Right) a_event.key.keysym.sym = inlib::wall::KEY_RIGHT;
  else if(a_sym==XK_Up) a_event.key.keysym.sym = inlib::wall::KEY_UP;
  else if(a_sym==XK_Down) a_event.key.keysym.sym = inlib::wall::KEY_DOWN;
  //else if(a_sym==XK_a) a_event.key.keysym.sym = inlib::wall::KEY_a;
  else if(a_sym==XK_Escape) a_event.key.keysym.sym = inlib::wall::KEY_ESCAPE;
  else a_event.key.keysym.sym = inlib::wall::KEY_UNKNOWN;

  unsigned short state = inlib::wall::KMOD_NONE;
  if((a_state & ControlMask)==ControlMask) state |= inlib::wall::KMOD_CTRL();
  if((a_state & ShiftMask)==ShiftMask)     state |= inlib::wall::KMOD_SHIFT();

  a_event.key.keysym.mod = (inlib::wall::mod)state;
}

#include <inlib/web>
#include <inlib/dirmanip>
#include <inlib/wall/strings>

#include <inlib/mparam>
#include <inlib/args>

#include <exlib/zlib>

#ifdef EXLIB_NO_GL
#include <exlib/X11/base_session>
#else
#include <exlib/X11/session>
#endif

#include <iostream>

#ifdef APP_USE_THREAD
#include <inlib/net/sg_serv_thread>
#include <exlib/app/X11/sg_serv>
#else
#include <inlib/net/sg_serv>
#endif

int main(int argc,char* argv[]) {

#include "../wall/master_begin.icc"

  if(verbose) std::cout << "wall_master : open X11 ..." << std::endl;

  if(XInitThreads()==False) { // for _sg_serv.
    std::cout << "wall_master_X11 : XInitThreads failed." << std::endl;
    stop_screens(screens);
    delete server_sock;
    return EXIT_FAILURE;
  }
  
#ifdef EXLIB_NO_GL
  exlib::X11::base_session x11(std::cout);
#else
  exlib::X11::session x11(std::cout);
#endif
  if(!x11.display()) {
    std::cout << "wall_master_X11 : can't initialize X11." << std::endl;
    stop_screens(screens);
    delete server_sock;
    return EXIT_FAILURE;
  }

  if(verbose) {
    std::cout << "wall_master : create X11 window ..." << std::endl;
  }

  Atom atom = ::XInternAtom(x11.display(),"WM_DELETE_WINDOW",False);
  Window win = x11.create_window("wall_master_X11",0,0,www,wwh);
  if(win==0L) {
    std::cout << "wall_master_X11 : can't create a window." << std::endl;
    stop_screens(screens);
    delete server_sock;
    return EXIT_FAILURE;
  }
  x11.map_raise_window(win);
  //x11.show_window(win);

#ifdef EXLIB_NO_GL
#else
  if(verbose) std::cout << "wall_master : glXMakeCurrent ..." << std::endl;
  if(::glXMakeCurrent(x11.display(),win,x11.context())==False){
    std::cout << "glXMakeCurrent failed." << std::endl;
    x11.delete_window(win);
    stop_screens(screens);
    delete server_sock;
    return EXIT_FAILURE;
  }
#endif
  
  /////////////////////////////////////////////////
  /////////////////////////////////////////////////
  /////////////////////////////////////////////////
#ifdef APP_USE_THREAD
  inlib::net::sg_serv_thread _sg_serv(std::cout);
  if(verbose) {
    std::cout << "wall_master : create data client thread ..." << std::endl;
  }
  unsigned int sg_serv_port;
  inlib::mparam<unsigned int>(params_from_env,args,ENV_PREFIX,"sg_serv_port",50800,sg_serv_port);
  _sg_serv.initialize(this_host,sg_serv_port,
		       exlib::sg_serv_connect_func,exlib::sg_serv_disconnect_func,x11.display(),(void*)win);
#else  
  inlib::net::sg_serv _sg_serv(std::cout);
#endif
  
  /////////////////////////////////////////////////
  /////////////////////////////////////////////////
  /////////////////////////////////////////////////

  if(verbose) std::cout << "wall_master : steering ..." << std::endl;

  int fd_X11 = ::XConnectionNumber(x11.display());

  while(true) {
    XEvent xevent;
    ::XNextEvent(x11.display(),&xevent);
    if((xevent.type==Expose)&&(xevent.xany.window==win)) {
      unsigned int wbw = (unsigned int)((float(www)/cols)*float(bw)/float(ww+2*bw));
      unsigned int wbh = (unsigned int)((float(wwh)/rows)*float(bh)/float(wh+2*bh));
#ifdef EXLIB_NO_GL
      render(x11.display(),win,www,wwh,cols,rows,wbw,wbh);
#else
      render(www,wwh,cols,rows,wbw,wbh);
      ::glXSwapBuffers(x11.display(),win);
#endif
      break;
    }
  }

  while(!to_quit) {

    if(screens.empty()) {
      std::cout << "wall_master_X11 : no more screens. Quit." << std::endl;
      break;
    }

    if( _move_timer.active() || _anims_timer.active() ){
      //std::cout << "wall_master_X11 : debug : do timer." << std::endl;

      _move_timer.check_time_out();
      _anims_timer.check_time_out();

      bool have_socks_input;
      if(!is_there_input(fd_X11,screens,_sg_serv.sg_client_sock(),have_socks_input)) {
        std::cout << "wall_master_X11 : is_there_input failed." << std::endl;
        break;
      }

      if(!have_socks_input) continue;
    }

    if(!wait_input(fd_X11,screens,_sg_serv.sg_client_sock())) {
      std::cout << "wall_master_X11 : wait_input failed." << std::endl;
      break;
    }

    if(_sg_serv.sg_client_sock().is_connected()) {
      if(!sg_serv_treat_events(_sg_serv,screens,exlib::decompress_buffer,
                                   ww,wh,bw,bh,cols,rows,doc_dir,verbose)) {}
    }

    if(::XPending(x11.display())) {

      XEvent xevent;
      ::XNextEvent(x11.display(),&xevent);
      if(xevent.type==ClientMessage) {
        if(xevent.xclient.data.l[0]==(long)atom) break;
        continue;

      } else if(xevent.type==Expose) {

        unsigned int wbw = (unsigned int)((float(www)/cols)*float(bw)/float(ww+2*bw));
        unsigned int wbh = (unsigned int)((float(wwh)/rows)*float(bh)/float(wh+2*bh));
#ifdef EXLIB_NO_GL
        render(x11.display(),win,www,wwh,cols,rows,wbw,wbh);
#else
        render(www,wwh,cols,rows,wbw,wbh);
        ::glXSwapBuffers(x11.display(),win);
#endif
        continue;

      } else if(xevent.type==MotionNotify) {
       
        // convert from master window coords to wall coords (with y=0 bottom) :
        int x = xevent.xmotion.x;
        int y = xevent.xmotion.y; //y=0 is top.
        x = int(x*float(wall_ww)/float(www));
        y = int(y*float(wall_wh)/float(wwh));
        y = wall_wh-y;

	inlib::uchar state = 0;
        if((xevent.xmotion.state & Button1MotionMask)==Button1MotionMask){
          state |= inlib::wall::MASK_MOUSEBUTTONDOWN;
        }

	inlib::wall::event event;
	inlib::wall::set_mouse_motion(x,y,state,event);
        all_events.push_event(event);

      } else if(xevent.type==ButtonPress) {
        int x = xevent.xbutton.x;
        int y = xevent.xbutton.y; //y=0 is top.
        x = int(x*float(wall_ww)/float(www));
        y = int(y*float(wall_wh)/float(wwh));
        y = wall_wh-y;

	inlib::wall::event event;
	inlib::wall::set_left_button_down(x,y,event);
        all_events.push_event(event);

      } else if(xevent.type==ButtonRelease) {
        int x = xevent.xbutton.x;
        int y = xevent.xbutton.y; //y=0 is top.
        x = int(x*float(wall_ww)/float(www));
        y = int(y*float(wall_wh)/float(wwh));
        y = wall_wh-y;

	inlib::wall::event event;
	inlib::wall::set_left_button_up(x,y,event);
        all_events.push_event(event);

      } else if(xevent.type==KeyPress) {
        KeySym keySym;
        ::XLookupString(&(xevent.xkey),NULL,0,&keySym,NULL);

        bool shift = false;
        if( (xevent.xkey.state & ShiftMask) == ShiftMask) shift = true;
        bool ctrl = false;
        if( (xevent.xkey.state & ControlMask) == ControlMask) ctrl = true;

        //std::cout << "debug : key press : ctrl " << inlib::to(ctrl)
        //          << std::endl;

        if( ctrl && 
            ( (keySym==XK_Left)  ||
              (keySym==XK_Right) ||
              (keySym==XK_Up)    ||
              (keySym==XK_Down) ) ){
          if(_move_timer.active()) {
            _move_timer.stop();
          } else {
	    inlib::sg::key_move key = inlib::sg::key_left;
	    if(keySym==XK_Left)       key = inlib::sg::key_left;
            else if(keySym==XK_Right) key = inlib::sg::key_right;
            else if(keySym==XK_Up)    key = inlib::sg::key_up;
            else if(keySym==XK_Down)  key = inlib::sg::key_down;
            _move_timer.set_key_shift(key,shift);
            _move_timer.start();
          }
          continue;

        } else {
          inlib::wall::event event;
          X2wall(keySym,xevent.xkey.state,event);
          all_events.push_event(event);
        }

      } else {
        continue;
      }

    } //XPending
  
    fetch_screens_events(std::cout,screens,all_events);

#include "../wall/master_dispatch.icc"

  } //end while(!to_quit)

#include "../wall/master_end.icc"

}
