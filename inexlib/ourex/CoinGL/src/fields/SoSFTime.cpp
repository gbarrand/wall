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
  \class SoSFTime SoSFTime.h Inventor/fields/SoSFTime.h
  \brief The SoSFTime class is a container for an SbTime value.
  \ingroup fields

  This field is used where nodes, engines or other field containers
  needs to store a single time representation.

  A field of this type stores its value to file as a floating
  point number.

  \sa SoMFTime
*/

// *************************************************************************

#include <Inventor/fields/SoSFTime.h>

#include <Inventor/C/tidbitsp.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/errors/SoReadError.h>
#include <Inventor/fields/SoSubFieldP.h>

#include "shared.h"

// *************************************************************************

SO_SFIELD_SOURCE(SoSFTime, SbTime, const SbTime &);

// *************************************************************************

// Override from parent class.
void
SoSFTime::initClass(void)
{
  SO_SFIELD_INTERNAL_INIT_CLASS(SoSFTime);
}

// *************************************************************************

// No need to document readValue() and writeValue() here, as the
// necessary information is provided by the documentation of the
// parent classes.
#ifndef DOXYGEN_SKIP_THIS

SbBool
SoSFTime::readValue(SoInput * in)
{
  SbTime t;
  if (!sosftime_read_value(in, t)) return FALSE;
  this->value = t;
  return TRUE;
}

void
SoSFTime::writeValue(SoOutput * out) const
{
  sosftime_write_value(out, this->getValue());
}

#endif // DOXYGEN_SKIP_THIS

// *************************************************************************
