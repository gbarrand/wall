// Copyright (C) 2010, Guy Barrand. All rights reserved.
// See the file wall.license for terms.

#ifdef APP_USE_DCMTK
#ifdef verify
#undef verify
#endif
#endif

#define APPLE_GL

#include "../wall/screen_main"
#include <exlib/sg/pv_holder>
#include <exlib/sg/GL_VIEWER>

#include "GLView.h"

#include <ostream>
#include <exlib/app/Cocoa/NSLog_streambuf.hm>

namespace wall {

//IMPORTANT : pv_holder must come first.

class main : protected exlib::sg::pv_holder, public common::main {
  typedef common::main parent;
public:
  EXLIB_GL_VIEWER_METHODS
public:
  virtual void win_render() {
    //NSLog(@"debug : win_render");
    [[m_GLView openGLContext] makeCurrentContext];
    render();
    [[m_GLView openGLContext] flushBuffer];
  }
public:
  main(std::ostream& a_out,const std::string& a_res_dir,const std::string& a_tmp_dir,
       GLView* a_GLView,
       wall::screen_client& a_screen_client)
  :exlib::sg::pv_holder(a_out)
  ,parent(a_out,exlib::sg::pv_holder::m_mgr_gra,exlib::sg::pv_holder::m_ttf,
          a_res_dir,a_tmp_dir,a_screen_client,false,0)
  ,m_GLView(a_GLView)
  {
    push_home();
  }
  inline virtual ~main(){}
private:
  GLView* m_GLView;
};

}

/*
@interface ForTimer : NSObject {
  wall::main* m_main;
}
- (id)initWithMain:(wall::main*)a_main;
- (void)dealloc;
- (void)work_timer_proc:(id)sender;
@end
@implementation ForTimer
- (id)initWithMain:(wall::main*)a_main{
  self = [super init];
  if(self) {
    m_main = a_main;
  }
  return self;
}
- (void)dealloc {[super dealloc];}
- (void)work_timer_proc:(id)a_sender{
  m_main->do_works();

  inlib::wall::event event;
  bool has_event;
  if(!m_main->m_screen_client.net_poll_event(has_event,event)) {
    m_main->out() << "wall_screen_Cocoa :"
                  << " net_poll_event failed."
                  << std::endl;
    [NSApp terminate:self];
    return;
  }
  if(has_event) {
    bool to_quit = false;
    screen_dispatch(*m_main,m_main->m_screen_client,m_main->m_opener,
                 m_main->out(),event,to_quit);
    if(to_quit) {
      [NSApp terminate:self];
      return;
    }
  }
  if(m_main->to_exit()) {
    //NSLog(@"debug : to_exit");
    m_main->m_screen_client.send_once(inlib::wall::EVT_USER_QUIT());
  }

  (void)a_sender; //rm warning.
}
@end
*/

#include <inlib/app>

int main(int /*argc*/, char* argv[]) {
  bool verbose = false;

  NSLog_streambuf nsbuf;
  std::ostream nsout(&nsbuf);

  ////////////////////////////////////////////////////////////////
  /// wall ///////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
  wall::screen_client screen_client(nsout);

  //inlib::args args(argc,argv);
  inlib::args args;
  bool params_from_env = true;
  std::ostream& wall_out = nsout;
  wall::screen_client& wclient = screen_client;

#include "../wall/screen_client_main.icc"

  if(!wall_status) return EXIT_FAILURE;

  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////

  //CGDirectDisplayID display_id = kCGDirectMainDisplay;

  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSApplication* app = [NSApplication sharedApplication];

  NSScreen* screen = 0;
 {NSArray* scrs = [NSScreen screens];
  int number = [scrs count];
  //NSLog(@"debug : screens %d",number);
  if(int(monitor)<number) {
    screen = (NSScreen*)[scrs objectAtIndex:monitor];
   //NSLog(@"debug : screen %d %lu",monitor,(NSScreen*)[scrs objectAtIndex:0]);
  }}

  NSRect screen_rect = [screen frame];
  //NSLog(@"debug : screen : %g %g %g %g\n",
  //  rect.origin.x,rect.origin.y,
  //  rect.size.width,rect.size.height);}

  // Create window :
  // origin = bottom, left.
  NSRect rect;
  if(full_screen) {
    rect = [screen frame];
    rect.origin.x = 0;
    rect.origin.y = 0;
  } else {
    rect.origin.x = window_x;
    rect.origin.y = screen_rect.size.height-(window_y+window_height),
    rect.size.width = window_width;
    rect.size.height = window_height;
  }

  unsigned int mask = 0;
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 101100 //OSX 10.11
  if(full_screen) {
    mask = NSBorderlessWindowMask;
  } else {
    mask = NSResizableWindowMask | NSTitledWindowMask;
  }
#else
  if(full_screen) {
    mask = NSWindowStyleMaskBorderless;
  } else {
    mask = NSWindowStyleMaskResizable | NSWindowStyleMaskTitled;
  }  
#endif  
  NSWindow* window = [[NSWindow alloc] initWithContentRect:rect
                                       styleMask:mask
                                       backing:NSBackingStoreBuffered
                                       defer:NO //we are the owner.
                                       screen:screen];
  if(full_screen) {
    [window setLevel: NSStatusWindowLevel];
  } else {
    NSString* title = [NSString stringWithUTF8String:"wall_screen_Cocoa"];
    [window setTitle:title];
    [title release];
    [window setShowsResizeIndicator:YES];
  }

  GLView* view = [[GLView alloc] initWithFrame:rect];
  [window setContentView:view];
  [view release];

  //[window setAcceptsMouseMovedEvents:NO];
  [window makeKeyAndOrderFront:app];

  ////////////////////////////////////////////////////////////////
  /// wall::main /////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
  //NSString* bundle_dir = [[NSBundle mainBundle] bundlePath];
  //std::string sbundle_dir = [bundle_dir UTF8String];
  //NSLog(@"debug : bundle dir %@",bundle_dir);
  //std::string res_dir = sbundle_dir+"/Contents/Resources";

  std::string exe_path; //for res_dir
  if(!inlib::program_path(argv[0],exe_path)) {
    nsout << "wall_screen_Cocoa : can't get exe path. Continue anyway."
              << std::endl;
    return EXIT_FAILURE;
  }
  //NSLog(@"debug : exe_path %s",exe_path.c_str());

  //std::string res_dir = exe_path+"/../Resources";
  std::string res_dir = exe_path+"/../res";
  std::string tmp_dir;
  inlib::tmpdir(tmp_dir);
  wall::main wmain(nsout,res_dir,tmp_dir,view,screen_client);
  //wmain.set_camera_factory(new wall::camera_factory(screen_client));
  //wmain.set_camera_work(new wall::cam_work(wmain,screen_client));
  wmain.cursor_visible.value(true);
  wmain.cursor_line_width.value(10);
  wmain.set_menu_pos(menu_col,menu_row);

  if(DOCUMENT.size()) {
    if(!inlib::file::is_empty(DOCUMENT)) {
      wmain.create_gui();
      bool done;
      wmain.opener().open(DOCUMENT,inlib::file::s_format_guessed(),
                          inlib::args(),done);
    }
    //DOCUMENT removed at end because of exlib::iv::file that does not
    //open the file in opener.open().
  }

  //////////////////////////////////////////////////////////////////
  /// steering /////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  [view set_main:&wmain];
  rect = [window frame];
  rect = [window contentRectForFrameRect:rect]; //window content true size.
  [view drawRect:rect];

  [[NSNotificationCenter defaultCenter]
     postNotificationName:NSApplicationWillFinishLaunchingNotification
     object:app];
  [[NSNotificationCenter defaultCenter]
     postNotificationName:NSApplicationDidFinishLaunchingNotification
     object:app];

  //////////////////////////////////////////////////////////////////
  /// main loop ////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  [pool release];
  while(true) { 

   {inlib::wall::event event;
    while(screen_client.get_event(event)) {
      bool to_quit;
      screen_dispatch(wmain,screen_client,nsout,event,to_quit);
      if(to_quit) break;
    }}

    if(wmain.num_cbks()) {
      wmain.do_works();

      if(wmain.to_exit()) {             
        if(!screen_client.send_once(inlib::wall::EVT_USER_QUIT())) break;
      }

      while(true) {
        pool = [[NSAutoreleasePool alloc] init];
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 101100 //OSX 10.11
        NSEvent* event = [app nextEventMatchingMask:NSAnyEventMask
#else
        NSEvent* event = [app nextEventMatchingMask:NSEventMaskAny
#endif    
                              untilDate:[NSDate distantPast]
                              inMode:NSDefaultRunLoopMode
                              dequeue:YES];
        if(event) {
          [app sendEvent:event];
          [app updateWindows];
        }
        [pool release];
        if(!event) break;
      }

      bool have_input;
      if(!screen_client.socket().is_there_input(have_input)) {
        nsout << "wall_screen_Cocoa :"
              << " inlib::net::is_there_input failed."
              << std::endl;
        break;
      }
      if(!have_input) continue;
    }

    //nsout << "debug : look for nsevent... " << std::endl;

    //FIXME : should wait for Cocoa and screen_client.socket() !

   {inlib::wall::event event;
    if(!wait_event(screen_client.socket(),event)) {
      nsout << "wall_screen_Cocoa :"
            << " wait_event failed."
            << std::endl;
      break;
    }
    bool to_quit = false;
    screen_dispatch(wmain,screen_client,nsout,event,to_quit);
    if(to_quit) break;}

    while(true) {
      pool = [[NSAutoreleasePool alloc] init];
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 101100 //OSX 10.11
        NSEvent* event = [app nextEventMatchingMask:NSAnyEventMask
#else
        NSEvent* event = [app nextEventMatchingMask:NSEventMaskAny
#endif    
                            untilDate:[NSDate distantPast]
                            inMode:NSDefaultRunLoopMode
                            dequeue:YES];
      if(event) {
        [app sendEvent:event];
        [app updateWindows];
      }
      [pool release];
      if(!event) break;
    }

    //if(wmain.to_exit()) {             
    //  if(!screen_client.send_once(inlib::wall::EVT_USER_QUIT())) break;
    //}

  }
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  if(DOCUMENT.size()) ::remove(DOCUMENT.c_str());

  //NSLog(@"debug : exiting...");

  return EXIT_SUCCESS;
}
