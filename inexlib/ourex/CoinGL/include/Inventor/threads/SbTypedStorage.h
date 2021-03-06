#ifndef COIN_SBTYPEDSTORAGE_H
#define COIN_SBTYPEDSTORAGE_H

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

#include <Inventor/C/threads/storage.h>

template <class Type>
class SbTypedStorage {
public:
  SbTypedStorage(unsigned int size) { this->storage = cc_storage_construct(size); }
  SbTypedStorage(unsigned int size, void (*constr)(void *), void (*destr)(void *))
    { this->storage = cc_storage_construct_etc(size, constr, destr); }
  ~SbTypedStorage(void) { cc_storage_destruct(this->storage); }

  Type get(void) { return (Type) cc_storage_get(this->storage); }

private:
  cc_storage * storage;
};

#endif // !COIN_SBTYPEDSTORAGE_H
