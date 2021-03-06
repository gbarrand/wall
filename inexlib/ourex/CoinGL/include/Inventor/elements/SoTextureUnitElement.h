#ifndef COIN_SOTEXTUREUNITELEMENT_H
#define COIN_SOTEXTUREUNITELEMENT_H

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

#include <Inventor/elements/SoInt32Element.h>

class SoTextureUnitElementP;

class COIN_DLL_API SoTextureUnitElement : public SoInt32Element {
  typedef SoInt32Element inherited;

  SO_ELEMENT_HEADER(SoTextureUnitElement);
public:
  static void initClass(void);
protected:
  virtual ~SoTextureUnitElement();

public:

  virtual void init(SoState * state);
  static void set(SoState * const state, SoNode * const node,
                  const int units);
  static int get(SoState * state);
  
private:
  // just in case it is needed at a later stage
  SoTextureUnitElementP * pimpl;
};

#endif // !COIN_SOTEXTUREUNITELEMENT_H

