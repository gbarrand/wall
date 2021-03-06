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
  \class SoMFString SoMFString.h Inventor/fields/SoMFString.h
  \brief The SoMFString class is a container for SbString values.
  \ingroup fields

  This field is used where nodes, engines or other field containers
  needs to store arrays of strings.

  \sa SoSFString

*/

// *************************************************************************

#include <assert.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoSubFieldP.h>
#include <Inventor/SoInput.h>
#include <Inventor/errors/SoDebugError.h>

#include "shared.h"

// *************************************************************************

SO_MFIELD_SOURCE(SoMFString, SbString, const SbString &);

// *************************************************************************

// Override from parent class.
void
SoMFString::initClass(void)
{
  SO_MFIELD_INTERNAL_INIT_CLASS(SoMFString);
}

// No need to document readValue() and writeValue() here, as the
// necessary information is provided by the documentation of the
// parent classes.
#ifndef DOXYGEN_SKIP_THIS

SbBool
SoMFString::read1Value(SoInput * in, int idx)
{
  assert(idx < this->maxNum);
  return in->read(this->values[idx]);
}

void
SoMFString::write1Value(SoOutput * out, int idx) const
{
  sosfstring_write_value(this, out, (*this)[idx]);
}

#endif // DOXYGEN_SKIP_THIS

/*!
  Set field to contain \a num \a strings from index \a start,
  replacing any strings already present at the given indices.
*/
void
SoMFString::setValues(const int start, const int numarg, const char * strings[])
{
  if (start+numarg > this->maxNum) this->allocValues(start+numarg);
  else if (start+numarg > this->num) this->num = start+numarg;

  for (int i=0; i < numarg; i++) this->values[i+start] = SbString(strings[i]);
  this->valueChanged();
}

/*!
  Set field to contain a single string, from \a str.
*/
void
SoMFString::setValue(const char * str)
{
  this->setValue(SbString(str));
}

/*!
  Remove all text from \a fromchar on \a fromline and to \a tochar
  on \a toline, including all lines between \a fromline and \a toline.
  Merge \a fromline and \a toline after deletion.
*/
void
SoMFString::deleteText(const int fromline, const int fromchar,
                       const int toline, const int tochar)
{
#if COIN_DEBUG && 1 // debug
  if (fromline < 0 || toline >= this->getNum() || fromline > toline ||
      (fromline == toline && fromchar >= tochar) ||
      fromchar < 0 || fromchar >= (*this)[fromline].getLength() ||
      tochar < 0 || tochar >= (*this)[toline].getLength()) {
    SoDebugError::post("SoMFString::deleteText",
                       "invalid argument(s), [%d, %d - %d, %d]",
                       fromline, fromchar, toline, tochar);
    return;
  }
#endif // debug

  if (fromline == toline) {
    this->values[fromline].deleteSubString(fromchar, tochar);
  }
  else {
    this->values[fromline].deleteSubString(fromchar, -1);
    this->values[toline].deleteSubString(0, tochar);
    // Merge.
    this->values[fromline] += (*this)[toline];
    // Delete intermediate strings + toline string.
    this->deleteValues(fromline + 1, toline - fromline);
  }
}

// *************************************************************************
