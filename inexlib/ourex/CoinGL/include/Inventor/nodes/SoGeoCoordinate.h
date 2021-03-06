#ifndef COIN_SOGEOCOORDINATE_H
#define COIN_SOGEOCOORDINATE_H

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

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/SbMatrix.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoMFVec3d.h>

class SoGeoCoordinateP;
class SoState;
class SoGeoOrigin;

class COIN_DLL_API SoGeoCoordinate : public SoNode {
  typedef SoNode inherited;

  SO_NODE_HEADER(SoGeoCoordinate);

public:
  static void initClass(void);
  SoGeoCoordinate(void);

  SoMFVec3d point;
  SoMFString geoSystem;

  virtual void doAction(SoAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void pick(SoPickAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

protected:
  virtual ~SoGeoCoordinate();

private:

  SbMatrix getTransform(SoGeoOrigin * origin, const int idx) const;

  SoGeoCoordinateP * pimpl;
};


#endif // COIN_SOGEOCOORDINATE_H
