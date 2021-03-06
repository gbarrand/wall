#ifndef COIN_SOMFVEC3S_H
#define COIN_SOMFVEC3S_H

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

#include <Inventor/fields/SoMField.h>
#include <Inventor/fields/SoSubField.h>
#include <Inventor/SbVec3s.h>

class COIN_DLL_API SoMFVec3s : public SoMField {
  typedef SoMField inherited;

  SO_MFIELD_HEADER(SoMFVec3s, SbVec3s, const SbVec3s &);
  SO_MFIELD_SETVALUESPOINTER_HEADER(SbVec3s);
  SO_MFIELD_SETVALUESPOINTER_HEADER(short);

public:
  static void initClass(void);

  void setValues(int start, int num, const short xyz[][3]);
  void set1Value(int idx, short x, short y, short z);
  void set1Value(int idx, const short xyz[3]);
  void setValue(short x, short y, short z);
  void setValue(const short xyz[3]);

}; // SoMFVec3s

#endif // !COIN_SOMFVEC3S_H
