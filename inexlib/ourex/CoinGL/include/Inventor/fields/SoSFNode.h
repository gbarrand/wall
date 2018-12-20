// Important note: this sourcefile was in full generated by the
// Coin/scripts/templant script from the code in SFNodeAndEngine.tpl.

//$ BEGIN TEMPLATE SFNodeAndEngine(NODE, Node)

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

#ifndef COIN_SOSFNODE_H
#define COIN_SOSFNODE_H

#include <Inventor/fields/SoSField.h>
#include <Inventor/fields/SoSubField.h>

class SoNode;


class COIN_DLL_API SoSFNode : public SoSField {
  typedef SoSField inherited;

  SO_SFIELD_HEADER(SoSFNode, SoNode *, SoNode *);

public:
  static void initClass(void);

  virtual void fixCopy(SbBool copyconnections);
  virtual SbBool referencesCopy(void) const;

private:
  virtual void countWriteRefs(SoOutput * out) const;

  // For accessing the readValue() and writeValue() methods.
  friend class SoMFNode;
};

#endif // !COIN_SOSFNODE_H
//$ END TEMPLATE SFNodeAndEngine