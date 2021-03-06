#ifndef COIN_SBVEC2S_H
#define COIN_SBVEC2S_H

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

#include <stdio.h>
#include <Inventor/SbBasic.h>
#include <Inventor/system/inttypes.h>

class SbVec2us;
class SbVec2b;
class SbVec2i32;
class SbVec2f;
class SbVec2d;

class COIN_DLL_API SbVec2s {
public:
  SbVec2s(void);
  SbVec2s(const short v[2]);
  SbVec2s(const short x, const short y);
  explicit SbVec2s(const SbVec2us & v) { setValue(v); }
  explicit SbVec2s(const SbVec2b & v) { setValue(v); }
  explicit SbVec2s(const SbVec2i32 & v) { setValue(v); }
  explicit SbVec2s(const SbVec2f & v) { setValue(v); }
  explicit SbVec2s(const SbVec2d & v) { setValue(v); }

  int32_t dot(const SbVec2s& v) const;
  const short * getValue(void) const;
  void getValue(short& x, short& y) const;
  void negate(void);
  SbVec2s& setValue(const short v[2]);
  SbVec2s& setValue(short x, short y);
  SbVec2s & setValue(const SbVec2us & v);
  SbVec2s & setValue(const SbVec2b & v);
  SbVec2s & setValue(const SbVec2i32 & v);
  SbVec2s & setValue(const SbVec2f & v);
  SbVec2s & setValue(const SbVec2d & v);
  short& operator [](const int i);
  const short& operator [](const int i) const;
  SbVec2s& operator *=(int d);
  SbVec2s& operator *=(double d);
  SbVec2s& operator /=(int d);
  SbVec2s& operator /=(double d);
  SbVec2s& operator +=(const SbVec2s& u);
  SbVec2s& operator -=(const SbVec2s& u);
  SbVec2s operator-(void) const;
  friend COIN_DLL_API SbVec2s operator *(const SbVec2s& v, int d);
  friend COIN_DLL_API SbVec2s operator *(const SbVec2s& v, double d);
  friend COIN_DLL_API SbVec2s operator *(int d, const SbVec2s& v);
  friend COIN_DLL_API SbVec2s operator *(double d, const SbVec2s& v);
  friend COIN_DLL_API SbVec2s operator /(const SbVec2s& v, int d);
  friend COIN_DLL_API SbVec2s operator /(const SbVec2s& v, double d);
  friend COIN_DLL_API SbVec2s operator +(const SbVec2s& v1, const SbVec2s& v2);
  friend COIN_DLL_API SbVec2s operator -(const SbVec2s& v1, const SbVec2s& v2);
  friend COIN_DLL_API int operator ==(const SbVec2s& v1, const SbVec2s& v2);
  friend COIN_DLL_API int operator !=(const SbVec2s& v1, const SbVec2s& v2);

  void print(FILE * fp) const;

private:
  short vec[2];
};

COIN_DLL_API SbVec2s operator *(const SbVec2s& v, int d);
COIN_DLL_API SbVec2s operator *(const SbVec2s& v, double d);
COIN_DLL_API SbVec2s operator *(int d, const SbVec2s& v);
COIN_DLL_API SbVec2s operator *(double d, const SbVec2s& v);
COIN_DLL_API SbVec2s operator /(const SbVec2s& v, int d);
COIN_DLL_API SbVec2s operator /(const SbVec2s& v, double d);
COIN_DLL_API SbVec2s operator +(const SbVec2s& v1, const SbVec2s& v2);
COIN_DLL_API SbVec2s operator -(const SbVec2s& v1, const SbVec2s& v2);
COIN_DLL_API int operator ==(const SbVec2s& v1, const SbVec2s& v2);
COIN_DLL_API int operator !=(const SbVec2s& v1, const SbVec2s& v2);

#endif // !COIN_SBVEC2S_H
