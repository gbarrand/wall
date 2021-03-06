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
  \class SoShaderProgramCache include/Inventor/caches/SoShaderProgramCache.h
  \brief The SoShaderProgramCache class is used to cache shader programs.
  \ingroup caches
*/

// *************************************************************************

#include <Inventor/caches/SoShaderProgramCache.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/C/tidbitsp.h>
#include <Inventor/SbString.h>

// *************************************************************************

class SoShaderProgramCacheP {
public:
  SbString program;
};

#define PRIVATE(p) ((p)->pimpl)

// *************************************************************************

/*!
  Constructor with \a state being the current state.
*/
SoShaderProgramCache::SoShaderProgramCache(SoState *state)
  : SoCache(state)
{
  PRIVATE(this) = new SoShaderProgramCacheP;
}

/*!
  Destructor.
*/
SoShaderProgramCache::~SoShaderProgramCache()
{
  delete PRIVATE(this);
}

/*!
  Sets the shader program for this cache.
*/
void
SoShaderProgramCache::set(const SbString & program)
{
  PRIVATE(this)->program = program;
}

/*!
  Returns the shader program for this cache.
*/
const SbString &
SoShaderProgramCache::get(void) const
{
  return PRIVATE(this)->program;
}

#undef PRIVATE
