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
  \class SoNormal SoNormal.h Inventor/nodes/SoNormal.h
  \brief The SoNormal class is a node for providing normals to the state.
  \ingroup nodes

  Coin will automatically calculate normals for you if no SoNormal
  nodes are present in the scene graph, but explicitly setting normals
  is useful for at least two purposes: 1) a potential increase in
  performance, 2) you can calculate and use "incorrect" normals to do
  various special effects.

  <b>FILE FORMAT/DEFAULTS:</b>
  \code
    Normal {
        vector [  ]
    }
  \endcode

  \sa SoNormalBinding
*/

// *************************************************************************

#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoSubNodeP.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/actions/SoPickAction.h>
#include <Inventor/elements/SoNormalElement.h>
#include <Inventor/elements/SoOverrideElement.h>
#include <Inventor/elements/SoGLVBOElement.h>
#include "../misc/SoVBO.h"
#include "../misc/PointerMap.h"

// *************************************************************************

/*!
  \var SoMFVec3f SoNormal::vector
  Sets a pool of normal vectors in the traversal state.
*/

// *************************************************************************

class SoNormalP {
 public:
  SoNormalP() : vbo(NULL) { }
  ~SoNormalP() { delete this->vbo; }

  SoVBO * vbo;
};

#define PRIVATE(obj) ((SoNormalP*) PointerMap::get(obj))

SO_NODE_SOURCE(SoNormal);

/*!
  Constructor.
*/
SoNormal::SoNormal(void)
{
  PointerMap::add(this, new SoNormalP);
  SO_NODE_INTERNAL_CONSTRUCTOR(SoNormal);

  SO_NODE_ADD_FIELD(vector, (NULL));
}

/*!
  Destructor.
*/
SoNormal::~SoNormal()
{
  SoNormalP * pimpl = PRIVATE(this);
  PointerMap::remove(this);
  delete pimpl;
}

// Doc in superclass.
void
SoNormal::initClass(void)
{
  SO_NODE_INTERNAL_INIT_CLASS(SoNormal, SO_FROM_INVENTOR_1|SoNode::VRML1);

  SO_ENABLE(SoCallbackAction, SoNormalElement);
  SO_ENABLE(SoGLRenderAction, SoNormalElement);
  SO_ENABLE(SoGetPrimitiveCountAction, SoNormalElement);
  SO_ENABLE(SoPickAction, SoNormalElement);
}

// Doc in superclass.
void
SoNormal::GLRender(SoGLRenderAction * action)
{
  //
  // FIXME: code to test if all normals are unit length, and store
  // this in some cached variable.  should be passed on to
  // SoGLNormalizeElement to optimize rendering (pederb)
  //
  SoNormal::doAction(action);

  SoNormalP * pimpl = PRIVATE(this);
  SoState * state = action->getState();
  
  SoBase::staticDataLock();
  SbBool setvbo = FALSE;
  const int num = this->vector.getNum();
  if (SoGLVBOElement::shouldCreateVBO(state, num)) {
    setvbo = TRUE;
    SbBool dirty = FALSE;
    if (pimpl->vbo == NULL) {
      pimpl->vbo = new SoVBO(GL_ARRAY_BUFFER, GL_STATIC_DRAW); 
      dirty =  TRUE;
    }
    else if (pimpl->vbo->getBufferDataId() != this->getNodeId()) {
      dirty = TRUE;
    }
    if (dirty) {
      pimpl->vbo->setBufferData(this->vector.getValues(0),
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
    SoGLVBOElement::setNormalVBO(state, pimpl->vbo);
  }
}

// Doc in superclass.
void
SoNormal::doAction(SoAction * action)
{
  SoState * state = action->getState();
  if (!this->vector.isIgnored() &&
      !SoOverrideElement::getNormalVectorOverride(state)) {
    SoNormalElement::set(state, this,
                         this->vector.getNum(), this->vector.getValues(0));
    if (this->isOverride()) {
      SoOverrideElement::setNormalVectorOverride(state, this, TRUE);
    }
  }
}

// Doc in superclass.
void
SoNormal::callback(SoCallbackAction * action)
{
  SoNormal::doAction(action);
}

// Doc in superclass.
void
SoNormal::pick(SoPickAction * action)
{
  SoNormal::doAction(action);
}

// Doc in superclass.
void
SoNormal::getPrimitiveCount(SoGetPrimitiveCountAction * action)
{
  SoNormal::doAction(action);
}

#undef PRIVATE
