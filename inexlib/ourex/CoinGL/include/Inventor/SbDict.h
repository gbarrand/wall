#ifndef COIN_SBDICT_H
#define COIN_SBDICT_H

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

// *************************************************************************

// Internally, we should use the SbHash template class, since it
// provides a better interface than SbDict, and since the interface of
// SbDict has issues on 64-bit platforms when keeping strict
// compatibility with the original SbDict of SGI Inventor.

#if defined(COIN_INTERNAL) && !defined(COIN_ALLOW_SBDICT)
#error prefer SbHash over SbDict for internal code
#endif // COIN_INTERNAL

// *************************************************************************

#include <stddef.h>
#include <Inventor/SbBasic.h>

class SbPList;

// *************************************************************************

class COIN_DLL_API SbDict {
public:
  SbDict(const int entries = 251);
  SbDict(const SbDict & from);
  ~SbDict();

  SbDict & operator=(const SbDict & from);

  typedef unsigned long Key;

  void applyToAll(void (* rtn)(Key key, void * value)) const;
  void applyToAll(void (* rtn)(Key key, void * value, void * data),
                  void * data) const;
  void clear(void);

  SbBool enter(const Key key, void * const value);
  SbBool find(const Key key, void *& value) const;
  void makePList(SbPList & keys, SbPList & values);
  SbBool remove(const Key key);

  void setHashingFunction(Key (*func)(const Key key));

private:
  struct cc_hash * hashtable;
  static void copyval(Key key, void * value, void * data);
};

// *************************************************************************

#endif // !COIN_SBDICT_H
