#ifndef COIN_SOLISTENERDOPPLERELEMENT_H
#define COIN_SOLISTENERDOPPLERELEMENT_H

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

#include <Inventor/elements/SoReplacedElement.h>
#include <Inventor/SbVec3f.h>

class COIN_DLL_API SoListenerDopplerElement : public SoReplacedElement {
  typedef SoReplacedElement inherited;

  SO_ELEMENT_HEADER(SoListenerDopplerElement);
public:
  static void initClass(void);
protected:
  virtual ~SoListenerDopplerElement();

public:
  virtual void init(SoState * state);
  static void setDopplerVelocity(SoState * const state, SoNode * const node,
                                 const SbVec3f & velocity);
  static void setDopplerFactor(SoState * const state, SoNode * const node,
                               float factor);

  static const SbVec3f & getDopplerVelocity(SoState * const state);
  static float getDopplerFactor(SoState * const state);

  virtual void print(FILE * file) const;

protected:
  SbVec3f dopplerVelocity;
  float dopplerFactor;
};

#endif // !COIN_SOLISTENERDOPPLERELEMENT_H
