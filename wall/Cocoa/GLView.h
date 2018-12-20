// Copyright (C) 2010, Guy Barrand. All rights reserved.
// See the file wall.license for terms.

#include <Cocoa/Cocoa.h>

namespace wall {namespace common {class main;}}

@interface GLView : NSOpenGLView {
  wall::common::main* m_main;
}
- (id)initWithFrame:(NSRect)rect;
- (void)dealloc;
- (void)drawRect:(NSRect)rect;
- (void)set_main:(wall::common::main*)a_main;

@end
