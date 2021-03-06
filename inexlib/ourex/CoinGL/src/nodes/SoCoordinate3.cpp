/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2007 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using Coin with software that can not be combined with the GNU
 *  GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See http://www.coin3d.org/ for more information.
 *
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  http://www.sim.no/  sales@sim.no  coin-support@coin3d.org
 *
\**************************************************************************/

/*!
  \class SoCoordinate3 SoCoordinate3.h Inventor/nodes/SoCoordinate3.h
  \brief The SoCoordinate3 class is a node for providing coordinates to shape nodes.
  \ingroup nodes

  When encountering nodes of this type during traversal, the
  coordinates it contains will be put on the state stack for later use
  by shape nodes of types which needs coordinate sets (like SoFaceSet
  nodes or SoPointSet nodes).

  Note that an SoCoordinate3 node will \e replace the coordinates
  already present in the state (if any).

  <b>FILE FORMAT/DEFAULTS:</b>
  \code
    Coordinate3 {
        point 0 0 0
    }
  \endcode

  \sa SoCoordinate4
*/

#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoSubNodeP.h>

#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/actions/SoPickAction.h>
#include <Inventor/elements/SoGLCoordinateElement.h>
#include <Inventor/elements/SoGLVBOElement.h>
#include "../misc/SoVBO.h"
#include "../misc/PointerMap.h"

/*!
  \var SoMFVec3f SoCoordinate3::point
  Coordinate set of 3D points.
*/

class SoCoordinate3P {
 public:
  SoCoordinate3P() : vbo(NULL) { }
  ~SoCoordinate3P() { delete this->vbo; }
  SoVBO * vbo;
};

#define PRIVATE(obj) ((SoCoordinate3P*) PointerMap::get(obj))

// *************************************************************************

SO_NODE_SOURCE(SoCoordinate3);

/*!
  Constructor.
*/
SoCoordinate3::SoCoordinate3(void)
{
  PointerMap::add(this, new SoCoordinate3P);
  SO_NODE_INTERNAL_CONSTRUCTOR(SoCoordinate3);

  SO_NODE_ADD_FIELD(point, (0.0f, 0.0f, 0.0f));
}

/*!
  Destructor.
*/
SoCoordinate3::~SoCoordinate3()
{
  SoCoordinate3P * pimpl = PRIVATE(this);
  PointerMap::remove(this);
  delete pimpl;  
}

// Doc from superclass.
void
SoCoordinate3::initClass(void)
{
  SO_NODE_INTERNAL_INIT_CLASS(SoCoordinate3, SO_FROM_INVENTOR_1|SoNode::VRML1);

  SO_ENABLE(SoGetBoundingBoxAction, SoCoordinateElement);
  SO_ENABLE(SoGLRenderAction, SoGLCoordinateElement);
  SO_ENABLE(SoGLRenderAction, SoGLVBOElement);
  SO_ENABLE(SoPickAction, SoCoordinateElement);
  SO_ENABLE(SoCallbackAction, SoCoordinateElement);
  SO_ENABLE(SoGetPrimitiveCountAction, SoCoordinateElement);
}

// Doc from superclass.
void
SoCoordinate3::doAction(SoAction * action)
{
  SoCoordinateElement::set3(action->getState(), this,
                            point.getNum(), point.getValues(0));
}

// Doc from superclass.
void
SoCoordinate3::GLRender(SoGLRenderAction * action)
{
  SoCoordinate3::doAction(action);
  SoCoordinate3P * pimpl = PRIVATE(this);
  SoState * state = action->getState();
  const int num = this->point.getNum();
  SbBool setvbo = FALSE;
  SoBase::staticDataLock();
  if (SoGLVBOElement::shouldCreateVBO(state, num)) {
    SbBool dirty = FALSE;
    setvbo = TRUE;
    if (pimpl->vbo == NULL) {
      pimpl->vbo = new SoVBO(GL_ARRAY_BUFFER, GL_STATIC_DRAW); 
      dirty =  TRUE;
    }
    else if (pimpl->vbo->getBufferDataId() != this->getNodeId()) {
      dirty = TRUE;
    }
    if (dirty) {
      pimpl->vbo->setBufferData(this->point.getValues(0),
                                num*sizeof(SbVec3f),
                                this->getNodeId());
    }
  }
  else if (pimpl->vbo && pimpl->vbo->getBufferDataId()) {
    // clear buffers to deallocate VBO memory
    pimpl->vbo->setBufferData(NULL, 0, 0);
  }
  SoBase::staticDataUnlock();
  if (setvbo) {
    SoGLVBOElement::setVertexVBO(state, pimpl->vbo);
  }
}

// Doc from superclass.
void
SoCoordinate3::callback(SoCallbackAction * action)
{
  SoCoordinate3::doAction(action);
}

// Doc from superclass.
void
SoCoordinate3::pick(SoPickAction * action)
{
  SoCoordinate3::doAction(action);
}

// Doc from superclass.
void
SoCoordinate3::getBoundingBox(SoGetBoundingBoxAction * action)
{
  SoCoordinate3::doAction(action);
}

// Doc from superclass.
void
SoCoordinate3::getPrimitiveCount(SoGetPrimitiveCountAction * action)
{
  SoCoordinate3::doAction(action);
}

#undef PRIVATE
