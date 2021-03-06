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
  \class SoPackedColor SoPackedColor.h Inventor/nodes/SoPackedColor.h
  \brief The SoPackedColor class is a node for setting diffuse and transparency material values.
  \ingroup nodes

  This node provides a convenient way of setting diffuse colors and
  transparency values with packed 32-bit RGBA vectors.

  <b>FILE FORMAT/DEFAULTS:</b>
  \code
    PackedColor {
        orderedRGBA 0xccccccff
    }
  \endcode

  \sa SoBaseColor
*/

// *************************************************************************

// FIXME: the name of the "orderedRGBA" field was actually "rgba" in
// the version 2.0 Inventor file format, and the semantics was also
// different (reversed order). This means that Inventor 2.0 files with
// PackedColor nodes will not import properly at the moment.  20020508 mortene.

// *************************************************************************

#include <Inventor/nodes/SoPackedColor.h>
#include <Inventor/nodes/SoSubNodeP.h>

#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoOverrideElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoGLVBOElement.h>
#include <Inventor/C/tidbits.h>
#include "../misc/SoVBO.h"
#include "../misc/PointerMap.h"

// *************************************************************************

/*!
  \var SoMFUInt32 SoPackedColor::orderedRGBA

  Set of packed 32-bit RGBA vectors.

  The most significant 24 bits specifies 8 bits each for the red,
  green and blue components.

  The least significant 8 bits specifies the transparency value, where
  0x00 means completely transparent, and 0xff completely opaque.
*/

// *************************************************************************

class SoPackedColorP {
 public:
  SoPackedColorP() : vbo(NULL) { }
  ~SoPackedColorP() { delete this->vbo; }
  SbBool transparent;
  SbBool checktransparent;
  SoVBO * vbo;
};

#define PRIVATE(obj) ((SoPackedColorP*) PointerMap::get(obj))

SO_NODE_SOURCE(SoPackedColor);

/*!
  Constructor.
*/
SoPackedColor::SoPackedColor()
{
  SoPackedColorP * pimpl = new SoPackedColorP;
  PointerMap::add(this, pimpl);
  SO_NODE_INTERNAL_CONSTRUCTOR(SoPackedColor);
  
  SO_NODE_ADD_FIELD(orderedRGBA, (0xccccccff));

  pimpl->checktransparent = FALSE;
  pimpl->transparent = FALSE;
}

/*!
  Destructor.
*/
SoPackedColor::~SoPackedColor()
{
  SoPackedColorP * pimpl = PRIVATE(this);
  PointerMap::remove(this);
  delete pimpl;
}

// Doc from superclass.
void
SoPackedColor::initClass(void)
{
  SO_NODE_INTERNAL_INIT_CLASS(SoPackedColor, SO_FROM_INVENTOR_2_1);

  SO_ENABLE(SoCallbackAction, SoLazyElement);
  SO_ENABLE(SoGLRenderAction, SoGLLazyElement);
}

// Doc from superclass.
void
SoPackedColor::GLRender(SoGLRenderAction * action)
{
  SoPackedColor::doAction(action);
}

// Doc from superclass.
void
SoPackedColor::doAction(SoAction * action)
{
  SoPackedColorP * pimpl = PRIVATE(this);
  
  (void) this->isTransparent(); // update cached value for transparent
  
  SoState * state = action->getState();
  const int num = this->orderedRGBA.getNum();
  if (!this->orderedRGBA.isIgnored() &&
      num > 0 &&
      !SoOverrideElement::getDiffuseColorOverride(state)) {
    SoLazyElement::setPacked(state, this,
                             num,
                             this->orderedRGBA.getValues(0),
                             pimpl->transparent);
    
    if (state->isElementEnabled(SoGLVBOElement::getClassStackIndex())) {
      SoBase::staticDataLock();
      SbBool setvbo = FALSE;
      if (SoGLVBOElement::shouldCreateVBO(state, num)) {
        SbBool dirty = FALSE;
        setvbo = TRUE;
        if (pimpl->vbo == NULL) {
          pimpl->vbo = new SoVBO(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
          dirty = TRUE;
        }
        else if (pimpl->vbo->getBufferDataId() != this->getNodeId()) {
          dirty = TRUE;
        }
        if (dirty) {
          if (coin_host_get_endianness() == COIN_HOST_IS_BIGENDIAN) {
            pimpl->vbo->setBufferData(this->orderedRGBA.getValues(0),
                                      num*sizeof(uint32_t),
                                      this->getNodeId());
          }
          else {
            // sigh. Need to swap bytes on little endian systems.
            const uint32_t * src = this->orderedRGBA.getValues(0);
            uint32_t * dst = (uint32_t*)
              pimpl->vbo->allocBufferData(num*sizeof(uint32_t),
                                          this->getNodeId());
            for (int i = 0; i < num; i++) {
              uint32_t tmp = src[i];
              dst[i] =
                (tmp << 24) |
                ((tmp & 0xff00) << 8) |
                ((tmp & 0xff0000) >> 8) |
                (tmp >> 24);
            }
          }
        }
      }
      else if (pimpl->vbo) {
        pimpl->vbo->setBufferData(NULL, 0, 0);
      }
      SoBase::staticDataUnlock();
      if (setvbo) {
        SoGLVBOElement::setColorVBO(state, pimpl->vbo);
      }
    }
    if (this->isOverride()) {
      SoOverrideElement::setDiffuseColorOverride(state, this, TRUE);
    }
  }
}

// Doc from superclass.
void
SoPackedColor::callback(SoCallbackAction * action)
{
  SoPackedColor::doAction(action);
}

/*!
  Returns \c TRUE if there is at least one RGBA vector in the set
  which is not completely opaque.
 */
SbBool
SoPackedColor::isTransparent(void)
{
  SoPackedColorP * pimpl = PRIVATE(this);

  if (pimpl->checktransparent) {
    pimpl->checktransparent = FALSE;
    pimpl->transparent = FALSE;
    int n = this->orderedRGBA.getNum();
    for (int i = 0; i < n; i++) {
      if ((this->orderedRGBA[i] & 0xff) != 0xff) {
        pimpl->transparent = TRUE;
        break;
      }
    }
  }
  return pimpl->transparent;
}

// Documented in superclass.
void
SoPackedColor::notify(SoNotList *list)
{
  // Overridden to check for transparency when orderedRGBA changes.

  SoField *f = list->getLastField();
  if (f == &this->orderedRGBA) {
    PRIVATE(this)->checktransparent = TRUE;
  }
  inherited::notify(list);
}

#undef PRIVATE
