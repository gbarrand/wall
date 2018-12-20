// Copyright (C) 2010, Guy Barrand. All rights reserved.
// See the file wall.license for terms.

#include "../wall/screen_main" //must come first, else ther is some clash with dcmtk and Apple /usr/include/AssertMacros.h.

#include "GLView.h"

@implementation GLView

- (id)initWithFrame:(NSRect)a_rect {

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
//#if MAC_OS_X_VERSION_MAX_ALLOWED < 1014
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1060   //macarts.
    [[self openGLContext] setValues:vals forParameter:NSOpenGLCPSwapInterval];
#else
    [[self openGLContext] setValues:vals forParameter:NSOpenGLContextParameterSwapInterval];
#endif    
    m_main = 0;
  }

  [pixelFormat release];
  return self;
}

- (void)dealloc {  
  [super dealloc];
}

- (void)drawRect:(NSRect)rect {
  [[self openGLContext] makeCurrentContext];

  int w = rect.size.width;
  int h = rect.size.height;
  if(m_main) {
    m_main->set_size(w,h);
    m_main->render();
  }

  [[self openGLContext] flushBuffer];
}

- (void)set_main:(wall::common::main*)a_main {m_main = a_main;}

@end
