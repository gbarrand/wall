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

#ifndef COIN_SOVRMLINDEXEDLINESET_H
#define COIN_SOVRMLINDEXEDLINESET_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/VRMLnodes/SoVRMLIndexedLine.h>

#ifndef SO_END_LINE_INDEX // also defined in SoIndexedFaceSet.h
#define SO_END_LINE_INDEX (-1)
#endif // !SO_END_LINE_INDEX


class COIN_DLL_API SoVRMLIndexedLineSet : public SoVRMLIndexedLine
{
  typedef SoVRMLIndexedLine inherited;
  SO_NODE_HEADER(SoVRMLIndexedLineSet);

public:
  static void initClass(void);
  SoVRMLIndexedLineSet(void);

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);

protected:
  virtual ~SoVRMLIndexedLineSet();
  virtual void generatePrimitives(SoAction * action);
};

#endif // ! COIN_SOVRMLINDEXEDLINESET_H
