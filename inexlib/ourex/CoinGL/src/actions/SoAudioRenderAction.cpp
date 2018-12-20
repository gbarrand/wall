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
  \class SoAudioRenderAction SoAudioRenderAction.h Inventor/actions/SoAudioRenderAction.h
  \brief The SoAudioRenderAction class renders the aural parts of the scene graph.
  \ingroup actions

  Applying this method at a root node for a scene graph, path or
  pathlist will render all sound-related nodes contained within that instance to
  the current SoAudioDevice.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Inventor/actions/SoAudioRenderAction.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoCoordinateElement.h>
#include <Inventor/elements/SoSwitchElement.h>
#include <Inventor/elements/SoUnitsElement.h>
#include <Inventor/elements/SoFocalDistanceElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoSoundElement.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoCoordinate4.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoListener.h>
#include <Inventor/nodekits/SoBaseKit.h>

#ifdef HAVE_VRML97
#include <Inventor/VRMLnodes/SoVRMLSound.h>
#include <Inventor/VRMLnodes/SoVRMLAudioClip.h>
#endif // HAVE_VRML97

class SoAudioRenderActionP
{
public:
  SoAudioRenderActionP(SoAudioRenderAction * master) : master(master) {};
  SoAudioRenderAction *master;
};

#undef PRIVATE
#define PRIVATE(p) ((p)->pimpl)
#undef PUBLIC
#define PUBLIC(p) ((p)->master)

SO_ACTION_SOURCE(SoAudioRenderAction);

void SoAudioRenderAction::initClass()
{
  SO_ACTION_INTERNAL_INIT_CLASS(SoAudioRenderAction, SoAction);

  SO_ENABLE(SoAudioRenderAction, SoModelMatrixElement);
  SO_ENABLE(SoAudioRenderAction, SoSwitchElement);
  SO_ENABLE(SoAudioRenderAction, SoUnitsElement);

  SO_ENABLE(SoAudioRenderAction, SoFocalDistanceElement);
  SO_ENABLE(SoAudioRenderAction, SoProjectionMatrixElement);
  SO_ENABLE(SoAudioRenderAction, SoViewVolumeElement);
  SO_ENABLE(SoAudioRenderAction, SoViewingMatrixElement);

  SO_ENABLE(SoAudioRenderAction, SoSoundElement);
}

SoAudioRenderAction::SoAudioRenderAction()
{
  SO_ACTION_CONSTRUCTOR(SoAudioRenderAction);

  PRIVATE(this) = new SoAudioRenderActionP(this);
}

SoAudioRenderAction::~SoAudioRenderAction()
{
  delete PRIVATE(this);
}

void SoAudioRenderAction::beginTraversal(SoNode *node)
{
  traverse(node);
}

void SoAudioRenderAction::callDoAction(SoAction *action, SoNode *node)
{
  node->doAction(action);
}

void SoAudioRenderAction::callAudioRender(SoAction *action, SoNode *node)
{
  SoAudioRenderAction *audioRenderAction = (SoAudioRenderAction *) action;

  if (node->isOfType(SoListener::getClassTypeId())) {
    SoListener *listener;
    listener = (SoListener *)node;
    listener->audioRender(audioRenderAction);
  }
#ifdef HAVE_VRML97
  else if (node->isOfType(SoVRMLSound::getClassTypeId())) {
    SoVRMLSound *sound;
    sound = (SoVRMLSound *)node;
    sound->audioRender(audioRenderAction);
  }
  else if (node->isOfType(SoVRMLAudioClip::getClassTypeId())) {
    SoVRMLAudioClip *clip;
    clip = (SoVRMLAudioClip *)node;
    clip->audioRender(audioRenderAction);
  }
#endif // HAVE_VRML97
}

/*
FIXME 20021101 thammer: remember to override invalidateState if we
keep an internal state at all. Called from 
SoSceneManager.setAudioRenderAction
*/
 