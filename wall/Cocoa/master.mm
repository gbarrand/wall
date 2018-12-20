// Copyright (C) 2010, Guy Barrand. All rights reserved.
// See the file wall.license for terms.

#ifdef INLIB_MEM
#include <inlib/mem>
#endif

#include "../wall/master_dispatch"
#include "../wall/master_sg_serv"

#include "../wall/master_render"

#include <inlib/web>
#include <inlib/dirmanip>
#include <inlib/wall/strings>

#include <inlib/mparam>
#include <inlib/args>

#include <exlib/zlib>

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
#import <Cocoa/Cocoa.h>

inline void cocoa2wall(unichar a_sym,bool a_ctrl,bool a_shift,
                                    inlib::wall::event& a_event){
  a_event.type = inlib::wall::EVT_KEYDOWN;
  a_event.key.type = inlib::wall::EVT_KEYDOWN;
  if(a_sym==NSLeftArrowFunctionKey) a_event.key.keysym.sym = inlib::wall::KEY_LEFT;
  else if(a_sym==NSRightArrowFunctionKey) a_event.key.keysym.sym = inlib::wall::KEY_RIGHT;
  else if(a_sym==NSUpArrowFunctionKey) a_event.key.keysym.sym = inlib::wall::KEY_UP;
  else if(a_sym==NSDownArrowFunctionKey) a_event.key.keysym.sym = inlib::wall::KEY_DOWN;
  //else if(a_sym==XK_a) a_event.key.keysym.sym = inlib::wall::KEY_a;
  else if(a_sym==27) a_event.key.keysym.sym = inlib::wall::KEY_ESCAPE;
  else a_event.key.keysym.sym = inlib::wall::KEY_UNKNOWN;

  unsigned short state = inlib::wall::KMOD_NONE;
  if(a_ctrl)    state |= inlib::wall::KMOD_CTRL();
  if(a_shift)  state |= inlib::wall::KMOD_SHIFT();

  a_event.key.keysym.mod = (inlib::wall::mod)state;
}

@interface GLView : NSOpenGLView {
  unsigned int m_www;
  unsigned int m_wwh;
  unsigned int m_cols;
  unsigned int m_rows;
  unsigned int m_bw;
  unsigned int m_bh;
  unsigned int m_wall_ww;
  unsigned int m_wall_wh;
  inlib::wall::events* m_events;
  move_timer* m_move_timer;
}
- (id)init:(NSRect)a_rect 
                       www:(unsigned int)a_www
                       wwh:(unsigned int)a_wwh
                      cols:(unsigned int)a_cols
                     rows:(unsigned int)a_rows
                       bw:(unsigned int)a_bw
                       bh:(unsigned int)a_bh
             wall_ww:(unsigned int)a_wall_ww
             wall_wh:(unsigned int)a_wall_wh
             move_timer:(move_timer*)a_move_timer;
- (void)dealloc;
- (void)drawRect:(NSRect)a_rect;

- (void)clear_events;
- (inlib::wall::events*)events;
- (void)mouseDown:(NSEvent*)a_event;
- (void)mouseUp:(NSEvent*)a_event;
- (void)mouseMoved:(NSEvent*)a_event;
- (void)mouseDragged:(NSEvent*)a_event;
- (void)keyDown:(NSEvent*)a_event;
- (BOOL)acceptsFirstResponder;
@end

@implementation GLView
- (id)init:(NSRect)a_rect 
                        www:(unsigned int)a_www
                        wwh:(unsigned int)a_wwh
                       cols:(unsigned int)a_cols
                      rows:(unsigned int)a_rows
                        bw:(unsigned int)a_bw
                        bh:(unsigned int)a_bh 
              wall_ww:(unsigned int)a_wall_ww
              wall_wh:(unsigned int)a_wall_wh
        move_timer:(move_timer*)a_move_timer {

  m_www = 0;
  m_wwh = 0;
  m_cols = 0;
  m_rows = 0;
  m_bw = 0;
  m_bh = 0;
  m_wall_ww = 0;
  m_wall_wh = 0;
  m_move_timer = 0;
  m_events = 0;

  NSOpenGLPixelFormatAttribute att[32];
  int i = 0;
  att[i++] = NSOpenGLPFADoubleBuffer;
  att[i++] = NSOpenGLPFAAccelerated;
  att[i++] = NSOpenGLPFAAccumSize;
  att[i++] = (NSOpenGLPixelFormatAttribute)32;
  att[i++] = NSOpenGLPFAColorSize;
  int colorbits = 32;
  att[i++] = (NSOpenGLPixelFormatAttribute)colorbits;
  att[i++] = NSOpenGLPFADepthSize;
  int depthbits = 32;
  att[i++] = (NSOpenGLPixelFormatAttribute)depthbits;
  att[i] = (NSOpenGLPixelFormatAttribute)0;
  NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:att];

  self = [super initWithFrame:a_rect pixelFormat:pixelFormat];
  if (self) {
    // flush buffer only during the vertical retrace of the monitor
    const GLint vals[1] = {1};
    [[self openGLContext] setValues:vals forParameter:NSOpenGLCPSwapInterval];
    m_www = a_www;
    m_wwh = a_wwh;
    m_cols = a_cols;
    m_rows = a_rows;
    m_bw = a_bw;
    m_bh = a_bh;
    m_wall_ww = a_wall_ww;
    m_wall_wh = a_wall_wh;
    m_move_timer = a_move_timer;
    m_events = new inlib::wall::events();
 }

  [pixelFormat release];
  return self;
}

- (void)dealloc {  
  delete m_events;
  [super dealloc];
}

- (void)drawRect:(NSRect)a_rect {
  [[self openGLContext] makeCurrentContext];
  render(m_www,m_wwh,m_cols,m_rows,m_bw,m_bh);
  [[self openGLContext] flushBuffer];
  (void)a_rect;
}

- (void)clear_events {m_events->clear();}
- (inlib::wall::events*)events {return m_events;}

- (BOOL)acceptsFirstResponder {return YES;}

- (void)mouseDown:(NSEvent*)a_event {
  //NSLog(@"debug : mouse down");
  NSPoint loc = [a_event locationInWindow];

  float x = loc.x;
  float y = loc.y; //y=0 is bottom.
  x = x*float(m_wall_ww)/float(m_www);
  y = y*float(m_wall_wh)/float(m_wwh);

  inlib::wall::event event;
  inlib::wall::set_left_button_down(x,y,event);
  m_events->push_event(event);
}
- (void)mouseUp:(NSEvent*)a_event {
  //NSLog(@"debug : mouse up");
  NSPoint loc = [a_event locationInWindow];

  float x = loc.x;
  float y = loc.y; //y=0 is bottom.
  x = x*float(m_wall_ww)/float(m_www);
  y = y*float(m_wall_wh)/float(m_wwh);

  inlib::wall::event event;
  inlib::wall::set_left_button_up(x,y,event);
  m_events->push_event(event);
}
- (void)mouseMoved:(NSEvent*)a_event {
  NSPoint loc = [a_event locationInWindow];

  float x = loc.x;
  float y = loc.y; //y=0 is bottom.
  x = x*float(m_wall_ww)/float(m_www);
  y = y*float(m_wall_wh)/float(m_wwh);

  inlib::uchar state = 0;

  inlib::wall::event event;
  inlib::wall::set_mouse_motion(x,y,state,event);
  m_events->push_event(event);
}
- (void)mouseDragged:(NSEvent*)a_event {
  NSPoint loc = [a_event locationInWindow];

  float x = loc.x;
  float y = loc.y; //y=0 is bottom.
  x = x*float(m_wall_ww)/float(m_www);
  y = y*float(m_wall_wh)/float(m_wwh);

  inlib::uchar state = inlib::wall::MASK_MOUSEBUTTONDOWN;

  inlib::wall::event event;
  inlib::wall::set_mouse_motion(x,y,state,event);
  m_events->push_event(event);
}
- (void) keyDown:(NSEvent*)a_event {
  //NSLog(@"debug : key down");

  bool shift = [a_event modifierFlags] & NSShiftKeyMask;
  bool ctrl = [a_event modifierFlags] & NSControlKeyMask;

  //NSString* chars = [a_event charactersIgnoringModifiers];
  NSString* chars = [a_event characters];
  unichar keyChar = 0;
  if ( [chars length] == 0 ) return;
  if ( [chars length] != 1 ) {[super keyDown:a_event];return;}

  keyChar = [chars characterAtIndex:0];

  if( ctrl && 
            ( (keyChar==NSLeftArrowFunctionKey)    ||
              (keyChar==NSRightArrowFunctionKey)  ||
              (keyChar==NSUpArrowFunctionKey)      ||
              (keyChar==NSDownArrowFunctionKey)) ){

    if(m_move_timer->active()) {
      m_move_timer->stop();
    } else {
      inlib::sg::key_move key = inlib::sg::key_left;

             if(keyChar==NSLeftArrowFunctionKey)      key = inlib::sg::key_left;
      else if(keyChar==NSRightArrowFunctionKey)   key = inlib::sg::key_right;
      else if(keyChar==NSUpArrowFunctionKey)       key = inlib::sg::key_up;
      else if(keyChar==NSDownArrowFunctionKey)  key = inlib::sg::key_down;

      m_move_timer->set_key_shift(key,shift);
      m_move_timer->start();
    }

  } else {
     inlib::wall::event event;
     cocoa2wall(keyChar,ctrl,shift,event);
     m_events->push_event(event);
  }

  //NSLog(@"debug : GLView::keyDown : ??? %d \"%c\"",keyChar,keyChar);
}
@end

#import <Foundation/NSFileHandle.h>

inline inlib::net::base_socket* find_sock(const screens_t& a_screens,int a_id){
  screens_t::const_iterator it;
  for(it=a_screens.begin();it!=a_screens.end();++it) {
    inlib::net::base_socket* client = *it;
    if(client->socket()==a_id) return client;
  }
  return 0;
}

class dc_params {
public:
  dc_params() 
  :m_screens(0)
  ,m_decompress_func(0)
  ,m_ww(0),m_wh(0),m_bw(0),m_bh(0)
  ,m_cols(0),m_rows(0)
  ,m_doc_dir()
  ,m_verbose(false)
  {}
  virtual ~dc_params(){}
private:
  dc_params(const dc_params&){}
  dc_params& operator=(const dc_params&){return *this;}
public:
  screens_t* m_screens;
  inlib::decompress_func m_decompress_func;
  unsigned int m_ww,m_wh,m_bw,m_bh;
  unsigned int m_cols,m_rows;
  std::string m_doc_dir;
  bool m_verbose;
};

#include <exlib/app/Cocoa/sg_serv.hm>

#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_5
@interface win_delegate : NSObject {
#else
@interface win_delegate : NSObject<NSWindowDelegate> {
#endif
  bool* m_to_quit;
  screens_t* m_screens;
  inlib::wall::events* m_events;
  inlib::net::sg_serv* m_net_sg_serv;
  dc_params* m_dc_params;
  NSFileHandle* m_sg_serv_file_handle;
  NSWindow* m_window;
}
- (id)init:(bool*)a_to_quit screens:(screens_t*)a_cls windows:(NSWindow*)a_window;
- (void)dealloc;
- (void)windowWillClose:(NSNotification*)a_not;
- (NSFileHandle*)add_fh:(int)a_id selector:(SEL)a_selector;
- (void)remove_fh:(NSFileHandle*)a_fh;
- (void)screen_fh_callback:(NSNotification*)a_not;
- (void)sg_serv_fh_callback:(NSNotification*)a_not;
- (void)socks2fhs:(const screens_t*)a_screens;
- (void)clear_events;
- (inlib::wall::events*)events;
- (void)set_sg_serv:(inlib::net::sg_serv*)a_sg_serv;  
- (bool)get_nsevent:(NSApplication*)a_app blocking:(bool)a_blocking to_quit:(bool*)a_to_quit;
- (dc_params*)dc_params;
@end
@implementation win_delegate
- (id)init:(bool*)a_to_quit screens:(screens_t*)a_cls windows:(NSWindow*)a_window {
  m_to_quit = 0;
  m_screens = 0;
  m_events = 0;
  m_net_sg_serv = 0;
  m_dc_params = 0;
  m_sg_serv_file_handle = 0;
  m_window = 0;
  if(self = [super init]) {
    m_to_quit = a_to_quit;
    m_screens = a_cls; 
    m_events = new inlib::wall::events();
    m_dc_params = new dc_params();
    m_window = a_window;
  }
  return self;
}
- (void)dealloc {
  if(m_sg_serv_file_handle) {
    [self remove_fh:m_sg_serv_file_handle];
    m_sg_serv_file_handle = 0;
  }    
  delete m_dc_params;
  delete m_events;
  [super dealloc];
}
- (void)windowWillClose:(NSNotification*)a_not {
  *m_to_quit = true;
  (void)a_not;
}
- (NSFileHandle*)add_fh:(int)a_id selector:(SEL)a_selector {
  //NSLog(@"debug : add_fh : socket %d",a_id);
  NSFileHandle* fh = [[NSFileHandle alloc] initWithFileDescriptor:a_id];
  [fh waitForDataInBackgroundAndNotify];
  [[NSNotificationCenter defaultCenter] addObserver:self
     selector:a_selector name:NSFileHandleDataAvailableNotification object:fh];
  return fh;
}
- (void)remove_fh:(NSFileHandle*)a_fh {
  [[NSNotificationCenter defaultCenter]
    removeObserver:self name:NSFileHandleDataAvailableNotification object:a_fh];
}
- (void)screen_fh_callback:(NSNotification*)a_not {
  NSFileHandle* fh = (NSFileHandle*)[a_not object];
  if(!fh) return;
  //NSLog(@"debug : win_delegate::screen_fh_callback");

  inlib::net::base_socket* client = find_sock(*m_screens,[fh fileDescriptor]);
  if(!client) {
    NSLog(@"win_delegate::screen_fh_callback : client not found for : %d",[fh fileDescriptor]);
    return;
  }

  inlib::wall::event event;
  if(!inlib::wall::wait_event(*client,event)) {}
  m_events->push_event(event,client);

  exlib::post_event(m_window,EXLIB_NSEVENT_FILE_HANDLE);
  
  [fh waitForDataInBackgroundAndNotify];
}
- (void)sg_serv_fh_callback:(NSNotification*)a_not {
  NSFileHandle* fh = (NSFileHandle*)[a_not object];
  if(!fh) return;
  //NSLog(@"debug : win_delegate::sg_serv_fh_callback");

  std::string prot;
  if(!m_net_sg_serv->sg_client_sock().fetch_string(prot)) {
    NSLog(@"debug : win_delegate::sg_serv_fh_callback : fetch_string failed.");
    m_net_sg_serv->stop_sg_client();
    return;
  }

  //if(m_dc_params->m_verbose) {
  //NSLog(@"debug : win_delegate::sg_serv_fh_callback : received prot %s",prot.c_str());
  //}

  if(!sg_serv_dispatch(prot,*m_net_sg_serv,
                           *m_dc_params->m_screens,m_dc_params->m_decompress_func,
                           m_dc_params->m_ww,m_dc_params->m_wh,
                           m_dc_params->m_bw,m_dc_params->m_bh,
                           m_dc_params->m_cols,m_dc_params->m_rows,
                           m_dc_params->m_doc_dir,
                           m_dc_params->m_verbose)){}

  [fh waitForDataInBackgroundAndNotify];
}
- (void)socks2fhs:(const screens_t*)a_screens {
  screens_t::const_iterator it;
  for(it=a_screens->begin();it!=a_screens->end();++it) {
    inlib::net::base_socket* client = *it;
    [self add_fh:client->socket() selector:@selector(screen_fh_callback:)];
  }
}
- (void)clear_events {m_events->clear();}
- (inlib::wall::events*)events {return m_events;}
- (void)set_sg_serv:(inlib::net::sg_serv*)a_sg_serv {m_net_sg_serv = a_sg_serv;}  
- (bool)get_nsevent:(NSApplication*)a_app blocking:(bool)a_blocking to_quit:(bool*)a_to_quit {
  // return "to break".
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  //NSLog(@"debug : get_nsevent : ...");
//#if MAC_OS_X_VERSION_MAX_ALLOWED < 1012
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1060   //macarts.
  NSEvent* event = [a_app nextEventMatchingMask:NSAnyEventMask
#else
  NSEvent* event = [a_app nextEventMatchingMask:NSEventMaskAny
#endif			     
                                      untilDate:a_blocking ? [NSDate distantFuture] : [NSDate distantPast]
                                         inMode:NSDefaultRunLoopMode
                                        dequeue:YES];
  if(event) {
    //NSLog(@"debug : get_nsevent : event %d",[event type]);
//#if MAC_OS_X_VERSION_MAX_ALLOWED < 1012
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1060   //macarts.
    if([event type]==NSApplicationDefined) {
#else
    if([event type]==NSEventTypeApplicationDefined) {
#endif      
      if([event subtype]==EXLIB_NSEVENT_CONNECT) {
        if(m_net_sg_serv) {
	  if(m_sg_serv_file_handle) {
            NSLog(@"win_delegate::get_nsevent : EXLIB_NSEVENT_CONNECT : WARNING : m_sg_serv_file_handle not null !");
	  } else {
            m_sg_serv_file_handle = [self add_fh:m_net_sg_serv->sg_client_sock().socket() selector:@selector(sg_serv_fh_callback:)];
          }
        }
      } else if([event subtype]==EXLIB_NSEVENT_DISCONNECT) {
        if(m_net_sg_serv) {
	  if(!m_sg_serv_file_handle) {
            NSLog(@"win_delegate::get_nsevent : EXLIB_NSEVENT_DISCONNECT : WARNING : m_sg_serv_file_handle is null !");
	  } else {
            [self remove_fh:m_sg_serv_file_handle];
            m_sg_serv_file_handle = 0;
          }
	}
      } else if([event subtype]==EXLIB_NSEVENT_FILE_HANDLE) {
      } else {
        NSLog(@"win_delegate::get_nsevent : NSEventTypeApplicationDefined : %d unknown",[event subtype]);
      }
    } else {
      //printf("win_delegate::get_nsevent : zzzz-001 : %d\n",[event type]);
      [a_app sendEvent:event]; 
      [a_app updateWindows];
      if(*a_to_quit) return true; //do not do a [pool release]; (why ?)
    }
//} else {
//  NSLog(@"debug : loop : null event");
  }
  [pool release];
  return event?false:true;
}
- (dc_params*)dc_params {return m_dc_params;}
@end

//#ifdef APP_USE_THREAD
#include <inlib/net/sg_serv_thread>
//#endif
  
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

#include <iostream>

int main(int argc,char* argv[]) {

  NSLog(@"debug : wall_master_Cocoa : 032");
  
#include "../wall/master_begin.icc"

  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSApplication* app = [NSApplication sharedApplication];

  //NOTE : the menu must not have a standard Quit menu item.
  //            Stopping application is done with the window close button.

  NSString* ns = [[NSString alloc] initWithString:@"MainMenu"];
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1070
  BOOL status = [NSBundle loadNibNamed:ns owner:app];
#else
  BOOL status = [[NSBundle mainBundle] loadNibNamed:ns owner:app topLevelObjects:NULL];
#endif
  if(!status) {
    NSLog(@"debug : load MainMenu nib failed.");
  }
  [ns release];

  unsigned int monitor = 0;
  unsigned int window_x = 0;
  unsigned int window_y = 0;
  unsigned int window_width = www;
  unsigned int window_height = wwh;

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
  rect.origin.x = window_x;
  rect.origin.y = screen_rect.size.height-(window_y+window_height),
  rect.size.width = window_width;
  rect.size.height = window_height;

  //unsigned int mask = NSResizableWindowMask | NSTitledWindowMask;
  unsigned int mask = NSTitledWindowMask;
  mask |= NSClosableWindowMask;
  NSWindow* window = [[NSWindow alloc] initWithContentRect:rect
                                       styleMask:mask
                                       backing:NSBackingStoreBuffered
                                       defer:NO //we are the owner.
                                       screen:screen];
  NSString* title = [NSString stringWithUTF8String:"wall_master_Cocoa"];
  [window setTitle:title];
  [title release];
  //[window setShowsResizeIndicator:YES];
  [window setAcceptsMouseMovedEvents:YES];

  win_delegate* _win_delegate = [[win_delegate alloc] init:&to_quit screens:&screens windows:window];
  [window setDelegate:_win_delegate];

  unsigned int wbw = (unsigned int)((float(www)/cols)*float(bw)/float(ww+2*bw));
  unsigned int wbh = (unsigned int)((float(wwh)/rows)*float(bh)/float(wh+2*bh));

  GLView* view = [[GLView alloc] init:rect 
                  www:www wwh:wwh cols:cols rows:rows bw:wbw bh:wbh
                  wall_ww:wall_ww wall_wh:wall_wh
                  move_timer:&_move_timer];

  [window setContentView:view];
  [view release];

  //app_delegate* app_del = [[app_delegate alloc] initWithWindow:window view:view];
  //[app setDelegate:app_del];

  int opts = (NSTrackingActiveAlways | NSTrackingInVisibleRect |
              NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved);
  NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:rect
                                                 options:opts
                                                   owner:window
                                                userInfo:nil];
  [view addTrackingArea:area];
  //[window makeFirstResponder:view];
  //[window makeKeyWindow];
  //[window setAcceptsMouseMovedEvents:YES];

  [window makeKeyAndOrderFront:app];

  //////////////////////////////////////////////////////////////////
  /// steering /////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
//#ifdef APP_USE_THREAD
  unsigned int data_client_port;
  inlib::mparam<unsigned int>(params_from_env,args,ENV_PREFIX,"data_client_port",50800,data_client_port);
  inlib::net::sg_serv_thread _sg_serv(std::cout);
  _sg_serv.initialize(this_host,data_client_port,sg_serv_connect_func,sg_serv_disconnect_func,window,0);
//#else
//  inlib::net::sg_serv _sg_serv(std::cout);
//#endif
  
 [_win_delegate set_sg_serv:&_sg_serv];

 {dc_params* params = [_win_delegate dc_params];
  params->m_screens = &screens;
  params->m_decompress_func = exlib::decompress_buffer;
  params->m_ww = ww;
  params->m_wh = wh;
  params->m_bw = bw;
  params->m_bh = bh;
  params->m_cols = cols;
  params->m_rows = rows;
  params->m_doc_dir = doc_dir;
  params->m_verbose = verbose;}

  [_win_delegate socks2fhs:&screens];

  [view drawRect:rect];

  [app finishLaunching];

  //////////////////////////////////////////////////////////////////
  /// main loop ////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  [pool release];

  while(!to_quit) {

    if(screens.empty()) {
      std::cout << "wall_master : no more screens. Quit." << std::endl;
      break;
    }

    bool blocking = true;
    if(_move_timer.active() || _anims_timer.active()) {
      _move_timer.check_time_out();
      _anims_timer.check_time_out();
      blocking = false;
    }

    //NSLog(@"debug : get_nsevent (2) ...");

    [view clear_events];
    [_win_delegate clear_events];
    [_win_delegate get_nsevent:app blocking:blocking to_quit:&to_quit];

    //NSLog(@"debug : get_nsevent (2) : end.");
    
    all_events.transfer(*([view events]));
    all_events.transfer(*([_win_delegate events])); //instead of fetch_screens_events in X11 version.
    
#include "../wall/master_dispatch.icc"

  } //end while(!to_quit)

  [[NSNotificationCenter defaultCenter] removeObserver:_win_delegate];

  //NSLog(@"debug : wall_master_Cocoa : exit");

#include "../wall/master_end.icc"

}
