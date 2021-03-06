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
  \class SoGLRenderAction SoGLRenderAction.h Inventor/actions/SoGLRenderAction.h
  \brief The SoGLRenderAction class renders the scene graph with OpenGL calls.
  \ingroup actions

  Applying this method at a root node for a scene graph, path or
  pathlist will render all geometry contained within that instance to
  the current OpenGL context.
 */

// *************************************************************************

// FIXME: this doesn't seem to end up in the Doxygen-generated
// documentation anywhere, with Doxygen version 1.2.18, at least.
// Find out why. There are also many other typedefs like this in Coin,
// which are not within the scope of a class declaration. 20040707 mortene.

/*!
  \typedef void SoGLRenderPassCB(void * userdata)

  Callback functions for the setPassCallback() method need to be of
  this type.

  \a userdata is a void pointer to any data the application need to
  know of in the callback function (like for instance a \e this
  pointer).

  \sa setPassCallback()
 */

// *************************************************************************

#include <string.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <Inventor/actions/SoGLRenderAction.h>

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/glue/glp.h>
#include <Inventor/C/glue/simage_wrapper.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/C/tidbitsp.h>
#include <Inventor/SbColor.h>
#include <Inventor/SbPlane.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoSubActionP.h>
#include <Inventor/caches/SoBoundingBoxCache.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoDecimationPercentageElement.h>
#include <Inventor/elements/SoDecimationTypeElement.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoGLLightIdElement.h>
#include <Inventor/elements/SoGLRenderPassElement.h>
#include <Inventor/elements/SoGLUpdateAreaElement.h>
#include <Inventor/elements/SoGLViewportRegionElement.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoOverrideElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoShapeHintsElement.h>
#include <Inventor/elements/SoShapeStyleElement.h>
#include <Inventor/elements/SoTextureEnabledElement.h>
#include <Inventor/elements/SoTextureOverrideElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoWindowElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/lists/SoCallbackList.h>
#include <Inventor/lists/SoEnabledElementsList.h>
#include <Inventor/misc/SoGL.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/C/tidbits.h>
#include <stdlib.h>

// *************************************************************************

/*!
  \enum SoGLRenderAction::TransparencyType

  Various settings for how to do rendering of transparent objects in
  the scene. Some of the settings will provide faster rendering, while
  others gives you better quality rendering.

  Note that doing correct rendering of \e multiple transparent objects
  often fails, because to be 100% correct, all polygons needs to be
  rendered in sorted order, and polygons can't intersect each
  other. In a dynamic scene graph it is often impossible to guarantee
  that no polygons intersect, and finding an algorithm that does
  correct sorting of polygons for all possible cases is very hard and
  time-consuming.

  The highest quality transparency mode in the original SGI / TGS Open
  Inventor is SoGLRenderAction::SORTED_OBJECT_BLEND, where all
  transparent objects are rendered in sorted order in a rendering pass
  after all opaque objects. However, this mode does not sort the
  polygons, and if you have an object where some polygon A is behind
  some other polygon B, the transparency will only be correct if A
  happens to be rendered before B. For other camera angles, where B is
  behind A, the transparency will not be correct.

  In Coin we have a new transparency mode that solves some of these
  problems: SoGLRenderAction::SORTED_OBJECT_SORTED_TRIANGLE_BLEND. In
  addition to sorting the objects, all polygons inside each object is
  also sorted back-to-front when rendering. But, if you have
  intersecting objects and/or intersecting polygons, even this
  transparency mode will fail. Also, because of the polygon sorting,
  this transparency mode is quite slow. It is possible to speed things
  up using the SoTransparencyType node, though, which enables you to
  set different transparency modes for different parts of the scene
  graph. If you have only have a few objects where you need to sort
  the polygons, you can use
  SoGLRenderAction::SORTED_OBJECT_SORTED_TRIANGLE_BLEND for those, and
  for instance SoGLRenderAction::SORTED_OBJECT_BLEND for all other
  transparent objects.

  The highest quality transparency mode in Coin is
  SoGLRenderAction::SORTED_LAYERS_BLEND. It is also the only mode that
  overrides all other modes in the scenegraph.

  \sa SoTransparencyType
*/

/*!
  \var SoGLRenderAction::TransparencyType SoGLRenderAction::SCREEN_DOOR

  Transparent triangles are rendered with a dither pattern. This is
  a fast (on most GFX cards) but not-so-high-quality transparency mode.

  One particular feature of this mode is that you are guaranteed that
  it always renders the transparent parts of the scene correct with
  regard to internal depth ordering of objects / polygons, something
  which is not the case for any other transparency mode.

  Polygons rendered with only transparent textures are not shown as
  being transparent when using this mode. The reason being that the
  SCREEN_DOOR mode is working on polygons, not pixels. To render
  polygons with dither pattern, a material node has to be inserted
  into the scenegraph with it's transparency field set.
*/

/*!

  \var SoGLRenderAction::TransparencyType SoGLRenderAction::ADD

  Transparent objects are rendered using additive alpha blending.
  Additive blending is probably mostly used to create special
  transparency effects. The new pixel color is calculated as the
  current pixel color plus the source pixel color multiplied with the
  source pixel alpha value.

*/

/*!
  \var SoGLRenderAction::TransparencyType SoGLRenderAction::DELAYED_ADD

  SoGLRenderAction::DELAYED_ADD Transparent objects are rendered using
  additive alpha blending, in a second rendering pass with depth
  buffer updates disabled.

*/

/*!
  \var SoGLRenderAction::TransparencyType SoGLRenderAction::SORTED_OBJECT_ADD

  Transparent objects are rendered using additive alpha blending.
  Opaque objects are rendered first, and transparent objects are
  rendered back to front with z-buffer updates disabled.

*/

/*!
  \var SoGLRenderAction::TransparencyType SoGLRenderAction::BLEND

  Transparent objects are rendered using multiplicative alpha blending.

  Multiplicative alpha blending is the blending type that is most
  often used to render transparent objects. The new pixel value is
  calculated as the old pixel color multiplied with one minus the
  source alpha value, plus the source pixel color multiplied with the
  source alpha value.

  We recommend that you use this transparency mode if you have only
  one transparent object in your scene, and you know that it will be
  rendered after the opaque objects.

*/

/*!
  \var SoGLRenderAction::TransparencyType SoGLRenderAction::DELAYED_BLEND

  Transparent objects are rendered using multiplicative alpha
  blending, in a second rendering pass with depth buffer updates
  disabled.

  Use this transparency type when you have one transparent object, or
  several transparent object that you know will never overlap (when
  projected to screen). Since the transparent objects are rendered
  after opaque ones, you'll not have to worry about putting the
  transparent objects at the end of your scene graph. It will not be
  as fast as the BLEND transparency type, of course, since the scene
  graph is traversed twice.

*/

/*!
  \var SoGLRenderAction::TransparencyType SoGLRenderAction::SORTED_OBJECT_BLEND

  Transparent objects are rendered using multiplicative alpha
  blending, Opaque objects are rendered first, and transparent objects
  are rendered back to front with z-buffer updates disabled.

  Use this transparency mode when you have several transparent object
  that you know might overlap (when projected to screen). This method
  will require 1 + num_transparent_objects rendering passes. Path
  traversal is used when rendering transparent objects, of course, but
  it might still be slow if you have lots of state changes before your
  transparent object. When using this mode, we recommend placing the
  transparent objects as early as possible in the scene graph to
  minimize traversal overhead.
*/

/*!
  \var SoGLRenderAction::TransparencyType SoGLRenderAction::SORTED_OBJECT_SORTED_TRIANGLE_ADD

  This transparency type is a Coin extension versus the original SGI
  Open Inventor API.

  Transparent objects are rendered back to front, and triangles in
  each object are sorted back to front before rendering.

  See description for SORTED_OBJECT_SORTED_TRIANGLE_BLEND for more
  information about this transparency type.

*/

/*!
  \var SoGLRenderAction::TransparencyType SoGLRenderAction::SORTED_OBJECT_SORTED_TRIANGLE_BLEND

  This transparency type is a Coin extension versus the original SGI
  Open Inventor API.

  Transparent objects are rendered back to front, and triangles in
  each object are sorted back to front before rendering.

  Use this transparency type when you have one (or more) transparent
  object(s) where you know triangles might overlap inside the object.
  This transparency type might be very slow if you have an object with
  lots of triangles, since all triangles have to be sorted before
  rendering, and an unoptimized rendering loop is used when rendering.
  Lines and points are not sorted before rendering. They are rendered
  as in the normal SORTED_OBJECT_BLEND transparency type.

  Please note that this transparency mode does not guarantee
  "correct" transparency rendering. It is almost impossible to find an
  algorithm that will sort triangles correctly in all cases, and
  intersecting triangles are not handled. Also, since each object
  is handled separately, two intersecting object will lead to
  incorrect transparency.
*/

/*!
  \var SoGLRenderAction::TransparencyType SoGLRenderAction::NONE

  This transparency type is a Coin extension versus the Open Inventor
  API.

  Turns off transparency for objects, even if transparency is set using
  an SoMaterial node.

  \since Coin 1.0
*/

/*!
  \var SoGLRenderAction::TransparencyType SoGLRenderAction::SORTED_LAYERS_BLEND

  This transparency type is a Coin extension versus the original SGI
  Open Inventor API.

  By using this transparency type, the SoGLRenderAction will render
  normal and intersecting transparent objects correctly independent of
  rendering order. It is the only transparency type rendering mode
  which is guaranteed to do so.

  This mode is different from all other modes in that it overrides the
  SoTransparencyType nodes in the scenegraph; all objects are drawn using
  SORTED_LAYERS_BLEND.

  There are currently two separate code paths for this mode. Both
  paths are heavily based on OpenGL extensions. The first method is
  based on extensions which are only available on NVIDIA chipsets
  (GeForce3 and above, except GeForce4 MX). These extensions are \c
  GL_NV_texture_shader, \c GL_NV_texture_rectangle or \c
  GL_EXT_texture_rectangle, \c GL_NV_register_combiners, \c
  GL_ARB_shadow and \c GL_ARB_depth_texture.  Please note that this
  transparency type occupy all four texture units on the NVIDIA card
  for all the rendering passes, except the first. Textured surfaces
  will therefore only be textured if they are not occluded by another
  transparent surface.

  The second method utilise the \c GL_ARB_fragment_program
  extension. This extension is currently supported by the GeForceFX
  family and the Radeon 9500 and above. This technique is faster than
  the pure NVIDIA method. The fragment program method will
  automatically be chosen if possible.   
  Please note that one should beware not to place the near-plane too
  close to the camera due to the lack of floating point precision
  control in fragment programs. Doing so may lead to loss of precision
  around the edges and 'jaggedness' of the transparent geometry.

  Setting the environment variable COIN_SORTED_LAYERS_USE_NVIDIA_RC to
  '1' will force the use of former code path instead of the latter,
  even if it is available.

  A rendering context with >= 24 bits depth buffer and 8 bits alpha
  channel must be the current rendering context for this blending mode
  to actually become activated. If the current rendering canvas does
  not have these properties, Coin will fall back on a simpler
  transparency handling mode. If you are using one of the
  window-system binding libraries provided by Systems in Motion,
  e.g. SoXt, SoQt or SoWin, you will need to explicitly enable this in
  your viewer. See the API documentation of the \c setAlphaChannel()
  method of either SoXtGLWidget, SoQtGLWidget or SoWinGLWidget.

  The detection of whether or not the SORTED_LAYERS_BLEND mode can be
  used will be done automatically by the Coin internals. If one or
  more of the necessary conditions listed above are unavailable,
  SoGLRenderAction::SORTED_OBJECT_BLEND will be used as the
  transparency type instead.

  To be able to render correct transparency independent of object
  order, one have to render in multiple passes. This technique is
  based on depth-peeling which strips away depth layers with each
  successive pass. The number of passes is therefore an indication of
  how deep into the scene transparent surfaces will be rendered with
  transparency. A higher number will lead to a lower framerate but
  higher quality for scenes with a lot of transparent surfaces. The
  default number of passes is '4'. This number can be specified using
  the SoGLRenderAction::setSortedLayersNumPasses() or by letting the
  environment variable \c COIN_NUM_SORTED_LAYERS_PASSES or \c
  OIV_NUM_SORTED_LAYERS_PASSES specify the number of passes.

  A more detailed presentation of the algorithm is written by Cass
  Everitt at NVIDIA;

  "Interactive Order-Independent Transparency"
  http:://developer.nvidia.com/object/order_independent_transparency.html

  \since Coin 2.2
  \since TGS Inventor 4.0
*/

// FIXME: 
//  todo: - Add debug printout info concerning choosen blend method. 
//        - Add GL_[NV/HP]_occlusion_test support making the number of passes adaptive.
//        - Maybe pbuffer support to eliminate the slow glCopyTexSubImage2D calls.
//        - Investigate whether the TGS method using only EXT_texture_env_combine is a 
//          feasible method (especially when it comes to speed and number of required 
//          texture units). [If more than two units are needed, then
//          a GeForce3++ card is required, which again is already
//          supported using the NVIDIA method.]
// (20031128 handegar)
//

/*!
  \enum SoGLRenderAction::AbortCode

  The return codes which an SoGLRenderAbortCB callback function should
  use.

  \sa setAbortCallback()
*/
/*!
  \var SoGLRenderAction::AbortCode SoGLRenderAction::CONTINUE
  Continue rendering as usual.
*/
/*!
  \var SoGLRenderAction::AbortCode SoGLRenderAction::ABORT
  Abort the rendering action immediately.
*/
/*!
  \var SoGLRenderAction::AbortCode SoGLRenderAction::PRUNE
  Do not render the current node or any of its children, but continue
  the rendering traversal.
*/
/*!
  \var SoGLRenderAction::AbortCode SoGLRenderAction::DELAY
  Delay rendering of the current node (and its children) until the
  next rendering pass.
*/

/*!
  \typedef typedef SoGLRenderAction::SoGLRenderAbortCB(void * userdata)
  Abort callbacks should be of this type.
  \sa setAbortCallback()
*/

/*!
  \typedef float SoGLSortedObjectOrderCB(void * userdata, SoGLRenderAction * action)

  A callback used for controlling the transparency sorting order.

  \sa setSortedObjectOrderStrategy().
  \since Coin 2.5
*/

/*! 
   \enum SortedObjectOrderStrategy

   Used for enumerating the different transparency sorting strategies.

  \sa setSortedObjectOrderStrategy().
  \since Coin 2.5
*/

/*!
  \var SoGLRenderAction::SortedObjectOrderStrategy SoGLRenderAction::BBOX_CENTER

  Do the sorting based on the center of the object bounding box.

  \sa setSortedObjectOrderStrategy().
  \since Coin 2.5
*/

/*!
  \var SoGLRenderAction::SortedObjectOrderStrategy BBOX_CLOSEST_CORNER

  Do the sorting based on the bounding box corner closest to the camera.

  \sa setSortedObjectOrderStrategy().
  \since Coin 2.5
*/

/*!
  \var SoGLRenderAction::SortedObjectOrderStrategy SoGLRenderAction::BBOX_FARTHEST_CORNER

  Do the sorting based on the bounding box corner farthest from the camera.

  \sa setSortedObjectOrderStrategy().
  \since Coin 2.5
*/

/*!
  \var SoGLRenderAction::SortedObjectOrderStrategy SoGLRenderAction::CUSTOM_CALLBACK

  Use a custom callback to determine the sorting order. 
  
  \sa setSortedObjectOrderStrategy().
  \since Coin 2.5
*/

// *************************************************************************

class SoGLRenderActionP {
public:
  SoGLRenderActionP(SoGLRenderAction * action)
    : action(action) { }

  SoGLRenderAction * action;
  SbViewportRegion viewport;
  int numpasses;
  SoGLRenderAction::TransparencyType transparencytype;
  SbBool smoothing;
  SbBool passupdate;
  SoGLRenderPassCB * passcallback;
  void * passcallbackdata;
  SoGLRenderAction::SoGLRenderAbortCB * abortcallback;
  void * abortcallbackdata;
  uint32_t cachecontext;
  int currentpass;
  SoPathList delayedpaths;
  SbBool delayedpathrender;
  SbBool transparencyrender;
  SoPathList transpobjpaths;
  SoPathList sorttranspobjpaths;
  SbList<float> sorttranspobjdistances;

  SoGetBoundingBoxAction * bboxaction;
  SbVec2f updateorigin, updatesize;
  SbBool needglinit;
  SbBool isrendering;
  SoCallbackList precblist;

  enum { RENDERING_UNSET, RENDERING_SET_DIRECT, RENDERING_SET_INDIRECT };
  int rendering;
  SbBool isDirectRendering(const SoState * state) const;
  int sortedlayersblendpasses;

  GLuint depthtextureid;
  GLuint hilotextureid;
  GLuint * rgbatextureids;
  GLuint sortedlayersblendprogramid;
  unsigned short viewportheight;
  unsigned short viewportwidth;
  SbBool sortedlayersblendinitialized;
  SbMatrix sortedlayersblendprojectionmatrix;
  int sortedlayersblendcounter;
  SbBool usenvidiaregistercombiners;

  SoGLRenderAction::SortedObjectOrderStrategy sortedobjectstrategy;
  SoGLSortedObjectOrderCB * sortedobjectcb;
  void * sortedobjectclosure;

  void setupSortedLayersBlendTextures(const SoState * state);
  void doSortedLayersBlendRendering(const SoState * state, SoNode * node);
  void initSortedLayersBlendRendering(const SoState * state);
  void renderOneBlendLayer(const SoState * state, SbBool shadow, SbBool update_ztex, SoNode * node);
  void texgenEnable(SbBool enable);
  void eyeLinearTexgen();

  // NVIDIA spesific methods for sorted layers blend
  void setupRegisterCombinersNV();
  void renderSortedLayersNV(const SoState * state);

  // ARB_fragment_program spesific methods for sorted layers blend
  void setupFragmentProgram();
  void renderSortedLayersFP(const SoState * state);

public:
  void setupBlending(SoState * state, const SoGLRenderAction::TransparencyType newtype);
  void render(SoNode * node);
  void renderMulti(SoNode * node);
  void renderSingle(SoNode * node);

  // For transparent paths that need to be sorted
  void addSortTransPath(SoPath * path);
};

// *************************************************************************

SO_ACTION_SOURCE(SoGLRenderAction);

static int COIN_GLBBOX = 0;
// *************************************************************************

// Override from parent class.
void
SoGLRenderAction::initClass(void)
{
  SO_ACTION_INTERNAL_INIT_CLASS(SoGLRenderAction, SoAction);

  SO_ENABLE(SoGLRenderAction, SoDecimationPercentageElement);
  SO_ENABLE(SoGLRenderAction, SoDecimationTypeElement);
  SO_ENABLE(SoGLRenderAction, SoGLLightIdElement);
  SO_ENABLE(SoGLRenderAction, SoGLRenderPassElement);
  SO_ENABLE(SoGLRenderAction, SoGLUpdateAreaElement);
  SO_ENABLE(SoGLRenderAction, SoLazyElement);
  SO_ENABLE(SoGLRenderAction, SoOverrideElement);
  SO_ENABLE(SoGLRenderAction, SoTextureOverrideElement);
  SO_ENABLE(SoGLRenderAction, SoWindowElement);
  SO_ENABLE(SoGLRenderAction, SoGLViewportRegionElement);
  SO_ENABLE(SoGLRenderAction, SoGLCacheContextElement);

  const char * env = coin_getenv("COIN_GLBBOX");
  if (env) {
    COIN_GLBBOX = atoi(env);
  }
  else {
    COIN_GLBBOX = 0;
  }
}

// *************************************************************************


static const char * sortedlayersblendprogram = 
"!!ARBfp1.0\n"
"OPTION ARB_precision_hint_nicest;\n"
"TEMP tmp;\n"
"PARAM c0 = {0, 1, 0.0040000002, 0};\n" // 0.004 = precision delta value for float division
"TEMP R0;\n"
"TEMP R1;\n"
"TEMP H0;\n"
"TXP R0.x, fragment.texcoord[3], texture[3], RECT;\n"
"RCP R1.x, fragment.texcoord[3].w;\n"
"MAD R0.x, fragment.texcoord[3].z, R1.x, -R0.x;\n"
"ADD R0.x, c0.z, -R0.x;\n"
"CMP H0.x, R0.x, c0.x, c0.y;\n"
"MOV H0, -H0.x;\n"
"KIL H0;\n"
// -- Adding texture from unit 0 --
// FIXME: This is a hackish solution. Texture settings like
// GL_MODULATE, GL_LUMINANCE etc. are ignored. (20031215 handegar)
"TEX tmp, fragment.texcoord[0], texture[0], 2D;\n"
"MOV tmp.a, 0;\n"
"ADD result.color, fragment.color.primary, tmp;\n"
"END";

#undef THIS
#define THIS this->pimpl

/*!
  Constructor. Sets up the render action for rendering within the
  given \a viewportregion.
*/
SoGLRenderAction::SoGLRenderAction(const SbViewportRegion & viewportregion)
{
  SO_ACTION_CONSTRUCTOR(SoGLRenderAction);

  THIS = new SoGLRenderActionP(this);
  // Can't just push this on the SoViewportRegionElement stack, as the
  // state hasn't been made yet.
  THIS->viewport = viewportregion;

  THIS->passcallback = NULL;
  THIS->passcallbackdata = NULL;
  THIS->smoothing = FALSE;
  THIS->numpasses = 1;
  THIS->transparencytype = SoGLRenderAction::SCREEN_DOOR;
  THIS->delayedpathrender = FALSE;
  THIS->transparencyrender = FALSE;
  THIS->isrendering = FALSE;
  THIS->passupdate = FALSE;
  THIS->bboxaction = new SoGetBoundingBoxAction(viewportregion);
  THIS->updateorigin.setValue(0.0f, 0.0f);
  THIS->updatesize.setValue(1.0f, 1.0f);
  THIS->rendering = SoGLRenderActionP::RENDERING_UNSET;
  THIS->abortcallback = NULL;
  THIS->cachecontext = 0;
  THIS->needglinit = TRUE;
  THIS->sortedlayersblendpasses = 4; 
  THIS->rgbatextureids = NULL;
  THIS->viewportheight = 0;
  THIS->viewportwidth = 0;
  THIS->sortedlayersblendinitialized = FALSE;
  THIS->sortedlayersblendcounter = 0;
  THIS->usenvidiaregistercombiners = FALSE;
  THIS->sortedobjectstrategy = BBOX_CENTER;
  THIS->sortedobjectcb = NULL;
  THIS->sortedobjectclosure = NULL;
}

/*!
  Destructor, frees up all internal resources for action instance.
*/
SoGLRenderAction::~SoGLRenderAction()
{
  delete THIS->bboxaction;
  delete THIS;
}

/*!
  Sets the viewport region for rendering. This will then override the
  region passed in with the constructor.
*/
void
SoGLRenderAction::setViewportRegion(const SbViewportRegion & newregion)
{
  THIS->viewport = newregion;
  THIS->bboxaction->setViewportRegion(newregion);
  // The SoViewportRegionElement is not set here, as it is always
  // initialized before redraw in beginTraversal().
}

/*!
  Returns the viewport region for the rendering action.
*/
const SbViewportRegion &
SoGLRenderAction::getViewportRegion(void) const
{
  return THIS->viewport;
}

/*!
  Sets the area of the OpenGL context canvas we should render into.

  The coordinates for \a origin and \a size should be normalized to be
  within [0.0, 1.0]. The default settings are <0.0, 0.0> for the \a
  origin and <1.0, 1.0> for the \a size, using the full size of the
  rendering canvas.
*/
void
SoGLRenderAction::setUpdateArea(const SbVec2f & origin, const SbVec2f & size)
{
  THIS->updateorigin = origin;
  THIS->updatesize = size;
}

/*!
  Returns information about the area of the rendering context window
  to be updated.
*/
void
SoGLRenderAction::getUpdateArea(SbVec2f & origin, SbVec2f & size) const
{
  origin = THIS->updateorigin;
  size = THIS->updatesize;
}

/*!
  Sets the abort callback.  The abort callback is called by the action
  for each node during traversal to check for abort conditions.

  The callback method should return one of the
  SoGLRenderAction::AbortCode enum values to indicate how the action
  should proceed further.

  Since the client SoGLRenderAbortCB callback function only has a
  single void* argument for the userdata, one has to do some
  additional work to find out which node the callback was made
  for. One can do this by for instance passing along the action
  pointer as userdata, and then call the
  SoGLRenderAction::getCurPath() method. The tail of the path will
  then be the last traversed node. Like this:

  \code
  // set up so we can abort or otherwise intervene with the render
  // traversal:
  myRenderAction->setAbortCallback(MyRenderCallback, myRenderAction);

  // [...]

  SoGLRenderAction::AbortCode
  MyRenderCallback(void * userdata)
  {
    SoGLRenderAction * action = (SoGLRenderAction *)userdata;
    SoNode * lastnode = action->getCurPath()->getTail();

    // [...]
    return SoGLRenderAction::CONTINUE;
  }
  \endcode

  \sa SoGLRenderAction::AbortCode
*/
void
SoGLRenderAction::setAbortCallback(SoGLRenderAbortCB * const func,
                                   void * const userdata)
{
  THIS->abortcallback = func;
  THIS->abortcallbackdata = userdata;
}

/*!
  Sets the transparency rendering method for transparent objects in
  the scene graph.

  \sa SoGLRenderAction::TransparencyType
*/
void
SoGLRenderAction::setTransparencyType(const TransparencyType type)
{ 
  if (THIS->transparencytype != type) {
    THIS->transparencytype = type;
    THIS->needglinit = TRUE;
  }
}

/*!
  Returns the transparency rendering type.
*/
SoGLRenderAction::TransparencyType
SoGLRenderAction::getTransparencyType(void) const
{
  return THIS->transparencytype;
}

/*!
  Sets (or unsets) smoothing. If the smoothing flag is \c on, Coin
  will try to use built-in features from the OpenGL implementation to
  smooth the appearance of otherwise jagged line and point primitives,
  calling

  \verbatim
      glEnable(GL_POINT_SMOOTH);
      glEnable(GL_LINE_SMOOTH);
  \endverbatim

  ...before rendering the scene.

  This is a simple (and computationally non-intensive) way of doing
  anti-aliasing.

  Default value for this flag is to be \c off.
*/
void
SoGLRenderAction::setSmoothing(const SbBool smooth)
{
  if (smooth != THIS->smoothing) {
    THIS->smoothing = smooth;
    THIS->needglinit = TRUE;
  }
}

/*!
  Returns whether smoothing is set or not.
*/
SbBool
SoGLRenderAction::isSmoothing(void) const
{
  return THIS->smoothing;
}

/*!
  Sets the number of rendering passes.  Default is 1, anything greater
  will enable antialiasing through the use of an OpenGL accumulation
  buffer.
*/
void
SoGLRenderAction::setNumPasses(const int num)
{
  THIS->numpasses = num;
}

/*!
  Returns the number of rendering passes done on updates.
*/
int
SoGLRenderAction::getNumPasses(void) const
{
  return THIS->numpasses;
}

/*!
  Sets whether each pass should render to screen or not.
*/
void
SoGLRenderAction::setPassUpdate(const SbBool flag)
{
  THIS->passupdate = flag;
}

/*!
  Returns the value of the "show intermediate updates" flag.

  \sa setPassUpdate()
*/
SbBool
SoGLRenderAction::isPassUpdate(void) const
{
  return THIS->passupdate;
}

/*!
  Sets the pass callback.  The callback is called between each
  rendering pass.
*/
void
SoGLRenderAction::setPassCallback(SoGLRenderPassCB * const func,
                                  void * const userdata)
{
  THIS->passcallback = func;
  THIS->passcallbackdata = userdata;
}

/*!
  Sets the OpenGL cache context key, which is used for deciding when
  to share OpenGL display lists.

  Each SoGLRenderAction has a cache context id. This can be set using
  SoGLRenderAction::setCacheContext(). The cache context id must be
  unique, so that different texture objects and display lists are
  created for uncompatible GL contexts. For instance, when
  SoGLRenderAction traverses an SoTexture2 node, the node checks if it
  has a texture object created for the cache context. If not, a new
  texture object will be created and used when rendering.

  \sa SoGLCacheContextElement::getUniqueCacheContext()
*/
void
SoGLRenderAction::setCacheContext(const uint32_t context)
{
  if (context != THIS->cachecontext) {
    THIS->cachecontext = context;
    this->invalidateState();
  }
}

/*!
  Returns the cache context key for this rendering action instance.
*/
uint32_t
SoGLRenderAction::getCacheContext(void) const
{
  return THIS->cachecontext;
}

/*!
  Sets the number of passes to render in SoGLRenderAction::SORTED_LAYERS_BLEND
  mode. Default number of passes is 4. This number can also be
  adjusted by setting the \c COIN_NUM_SORTED_LAYERS_PASSES or
  \c OIV_NUM_SORTED_LAYERS_PASSES environment variable.
*/
void 
SoGLRenderAction::setSortedLayersNumPasses(int num)
{
  THIS->sortedlayersblendpasses = num;
}
 
/*!
  Returns the number of passes to render when in
  SoGLRenderAction::SORTED_LAYERS_BLEND mode.
*/
int 
SoGLRenderAction::getSortedLayersNumPasses() const
{
  return THIS->sortedlayersblendpasses;
}


// Documented in superclass. Overridden from parent class to
// initialize the OpenGL state.
void
SoGLRenderAction::beginTraversal(SoNode * node)
{
  if (THIS->isrendering) {
    inherited::beginTraversal(node);
    return;
  }

  // If the environment variable COIN_GLBBOX is set to 1, apply a bbox
  // action before rendering.  This will make sure bounding box caches
  // are updated (needed for view frustum culling). The default
  // SoQt/SoWin/SoXt viewers will also apply a SoGetBoundingBoxAction
  // so we don't do this by default yet.
  if (COIN_GLBBOX) {
    THIS->bboxaction->apply(node);
  }
  int err_before_init = GL_NO_ERROR;

  if (sogl_glerror_debugging()) {
    err_before_init = glGetError();
  }
  if (THIS->needglinit) {
    THIS->needglinit = FALSE;

    // we are always using GL_COLOR_MATERIAL in Coin
    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    if (THIS->smoothing) {
      glEnable(GL_POINT_SMOOTH);
      glEnable(GL_LINE_SMOOTH);
    }
    else {
      glDisable(GL_POINT_SMOOTH);
      glDisable(GL_LINE_SMOOTH);
    }
  }

  int err_after_init = GL_NO_ERROR;

  if (sogl_glerror_debugging()) {
    err_after_init = glGetError();
  }

  if (COIN_DEBUG && ((err_before_init != GL_NO_ERROR) || (err_after_init != GL_NO_ERROR))) {
    int err = (err_before_init != GL_NO_ERROR) ? err_before_init : err_after_init;
    SoDebugError::postWarning("SoGLRenderAction::beginTraversal",
                              "GL error %s initialization: %s",
                              (err_before_init != GL_NO_ERROR) ? "before" : "after",
                              coin_glerror_string(err));
  }

  THIS->render(node);
  // GL errors after rendering will be caught in SoNode::GLRenderS().
}

// Documented in superclass. Overridden from parent class to clean up
// the lists of objects which were included in the delayed rendering.
void
SoGLRenderAction::endTraversal(SoNode * node)
{
  inherited::endTraversal(node);
}


/*!
  Used by shape nodes or others which need to know whether or not they
  should immediately render themselves or if they should wait until
  the next pass.
*/
SbBool
SoGLRenderAction::handleTransparency(SbBool istransparent)
{
  SoState * thestate = this->getState();
  const cc_glglue *glue = sogl_glue_instance(thestate);

  SoGLRenderAction::TransparencyType transptype = 
    (SoGLRenderAction::TransparencyType)
    SoShapeStyleElement::getTransparencyType(thestate);


  if (THIS->transparencytype == SORTED_LAYERS_BLEND) {

    // Do not cache anything. We must have full control!
    SoCacheElement::invalidate(thestate);
    
    THIS->sortedlayersblendprojectionmatrix = 
      SoProjectionMatrixElement::get(thestate);

    if (!SoTextureEnabledElement::get(thestate)) {
      if (glue->has_arb_fragment_program && !THIS->usenvidiaregistercombiners) {
        THIS->setupFragmentProgram();
      }
      else {
        THIS->setupRegisterCombinersNV();
      }
    } 

    // Must always return FALSE as everything must be rendered to the
    // RGBA layers (which are blended together at the end of each
    // frame).
    return FALSE;
  }


  // check common cases first
  if (!istransparent || transptype == SoGLRenderAction::NONE || transptype == SoGLRenderAction::SCREEN_DOOR) {
    if (THIS->smoothing) {
      SoLazyElement::enableBlending(thestate, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else SoLazyElement::disableBlending(thestate);
    return FALSE;
  }

  // below this point, shape is transparent, and we know that
  // transparency type is not SCREEN_DOOR or NONE.

  // for the transparency render pass(es) we should always render when
  // we get here.
  if (THIS->transparencyrender) {
    THIS->setupBlending(thestate, transptype);
    return FALSE;
  }
  // check for special case when rendering delayed paths.  we don't
  // want to add these objects to the list of transparent objects, but
  // render right away.
  if (THIS->delayedpathrender) {
    THIS->setupBlending(thestate, transptype);
    return FALSE;
  }
  switch (transptype) {
  case SoGLRenderAction::ADD:
    SoLazyElement::enableBlending(thestate, GL_SRC_ALPHA, GL_ONE);
    return FALSE;
  case SoGLRenderAction::BLEND:
    SoLazyElement::enableBlending(thestate, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return FALSE;
  case SoGLRenderAction::DELAYED_ADD:
  case SoGLRenderAction::DELAYED_BLEND:
    this->addTransPath(this->getCurPath()->copy());
    SoCacheElement::setInvalid(TRUE);
    if (thestate->isCacheOpen()) {
      SoCacheElement::invalidate(thestate);
    }
    return TRUE; // delay render
  case SoGLRenderAction::SORTED_OBJECT_ADD:
  case SoGLRenderAction::SORTED_OBJECT_BLEND:
  case SoGLRenderAction::SORTED_OBJECT_SORTED_TRIANGLE_ADD:
  case SoGLRenderAction::SORTED_OBJECT_SORTED_TRIANGLE_BLEND:
    THIS->addSortTransPath(this->getCurPath()->copy());
    SoCacheElement::setInvalid(TRUE);
    if (thestate->isCacheOpen()) {
      SoCacheElement::invalidate(thestate);
    }
    return TRUE; // delay render  
  default:
    assert(0 && "should not get here");
    break;
  }
  return FALSE;
}

/*!
  Returns the number of the current rendering pass.
*/

int
SoGLRenderAction::getCurPass(void) const
{
  return THIS->currentpass;
}

/*!
  Returns \c TRUE if the render action should abort now based on user
  callback.

  \sa setAbortCallback()
*/
SbBool
SoGLRenderAction::abortNow(void)
{
  if (this->hasTerminated()) return TRUE;

#if COIN_DEBUG && 0 // for dumping the scene graph during GLRender traversals
  static int debug = -1;
  if (debug == -1) {
    const char * env = coin_getenv("COIN_DEBUG_GLRENDER_TRAVERSAL");
    debug = env && (atoi(env) > 0);
  }
  if (debug) {
    const SoFullPath * p = (const SoFullPath *)this->getCurPath();
    assert(p);
    const int len = p->getLength();
    for (int i=1; i < len; i++) { printf("  "); }
    const SoNode * n = p->getTail();
    assert(n);
    printf("%p %s (\"%s\")\n",
           n, n->getTypeId().getName().getString(),
           n->getName().getString());
  }
#endif // debug

  SbBool abort = FALSE;
  if (THIS->abortcallback) {
    switch (THIS->abortcallback(THIS->abortcallbackdata)) {
    case CONTINUE:
      break;
    case ABORT:
      this->setTerminated(TRUE);
      abort = TRUE;
      break;
    case PRUNE:
      // abort this node, but do not abort rendering
      abort = TRUE;
      break;
    case DELAY:
      this->addDelayedPath(this->getCurPath()->copy());
      // prune this node
      abort = TRUE;
      break;
    }
  }
  return abort;
}

/*!
  Let SoGLRenderAction instance know if application is running on the
  local machine or if the rendering instructions are sent over the
  network.

  The flag is used to optimize rendering. For instance should the
  displaylist caching strategy be influenced by this flag to be more
  aggressive with the caching when rendering instructions are passed
  over the network.

  Default value is \c FALSE. The value of the flag will not be changed
  internally from the Coin library code, as it is meant to be
  controlled from client code -- typically from the SoQt / SoXt /
  SoWin / SoGtk libraries.

  \sa getRenderingIsRemote()
 */
void
SoGLRenderAction::setRenderingIsRemote(SbBool isremote)
{
  THIS->rendering = isremote ?
    SoGLRenderActionP::RENDERING_SET_INDIRECT :
    SoGLRenderActionP::RENDERING_SET_DIRECT;
}

/*!
  Returns whether or not the application is running remotely.

  \sa setRenderingIsRemote()
 */
SbBool
SoGLRenderAction::getRenderingIsRemote(void) const
{
  SbBool isdirect;
  if (THIS->rendering == SoGLRenderActionP::RENDERING_UNSET) {
    isdirect = TRUE;
  }
  else {
    isdirect = THIS->rendering == SoGLRenderActionP::RENDERING_SET_DIRECT;
  }
  return !isdirect;
}

/*!
  Adds a path to the list of paths to render after the current pass.
 */
void
SoGLRenderAction::addDelayedPath(SoPath * path)
{
  SoState * thestate = this->getState();
  SoCacheElement::invalidate(thestate);
  assert(!THIS->delayedpathrender);
  THIS->delayedpaths.append(path);
}

/*!
  Returns a flag indicating whether or not we are currently rendering
  from the list of delayed paths of the scene graph.
 */
SbBool
SoGLRenderAction::isRenderingDelayedPaths(void) const
{
  return THIS->delayedpathrender;
}

// Remember a path containing a transparent object for later
// rendering. We know path == this->getCurPath() when we get here.
// This method is only used to add paths that are to be rendered after
// all transparent paths that need sorting have been rendered, so no
// need to calculate distances. Just add to list.
void
SoGLRenderAction::addTransPath(SoPath * path)
{
  THIS->transpobjpaths.append(path);
}

// Documented in superclass. Overridden to reinitialize GL state on
// next apply.
void
SoGLRenderAction::invalidateState(void)
{
  inherited::invalidateState();
  THIS->needglinit = TRUE;
}

// Sort paths with transparent objects before rendering.
void
SoGLRenderAction::doPathSort(void)
{
  // need to cast to SbPList to avoid ref/unref problems
  SbPList * plist = (SbPList *)&THIS->sorttranspobjpaths;
  float * darray = (float *)THIS->sorttranspobjdistances.getArrayPtr();

  int i, j, distance, n = THIS->sorttranspobjdistances.getLength();
  void * ptmp;
  float dtmp;

  // shell sort algorithm (O(nlog(n))
  for (distance = 1; distance <= n/9; distance = 3*distance + 1);
  for (; distance > 0; distance /= 3) {
    for (i = distance; i < n; i++) {
      dtmp = darray[i];
      ptmp = plist->get(i);
      j = i;
      while (j >= distance && darray[j-distance] < dtmp) {
        darray[j] = darray[j-distance];
        plist->set(j, plist->get(j-distance));
        j -= distance;
      }
      darray[j] = dtmp;
      plist->set(j, ptmp);
    }
  }
}

/*!
  Adds a callback which is invoked right before the scene graph traversal
  starts. All necessary GL initialization is then done (e.g. the viewport
  is correctly set), and this callback can be useful to, for instance,
  clear the viewport before rendering, or draw a bitmap in the background
  before rendering etc.

  The callback is only invoked once (before the first rendering pass)
  when multi pass rendering is enabled.

  Please note that SoSceneManager usually adds a callback to clear the
  GL buffers in SoSceneManager::render(). So, if you plan to for
  instance draw an image in the color buffer using this callback, you
  should make sure that the scene manager doesn't clear the buffer.
  This can be done either by calling SoSceneManager::render() with
  both arguments FALSE, or, if you're using one of our GUI toolkits
  (SoXt/SoQt/SoGtk/SoWin), call setClearBeforeRender() on the viewer.

  This method is an extension versus the Open Inventor API.

  \sa removePreRenderCallback().  
*/
void
SoGLRenderAction::addPreRenderCallback(SoGLPreRenderCB * func, void * userdata)
{
  THIS->precblist.addCallback((SoCallbackListCB*) func, userdata);
}

/*!
  Removed a callback added with the addPreRenderCallback() method.

  This method is an extension versus the Open Inventor API.

  \sa addPreRenderCallback()
*/
void
SoGLRenderAction::removePreRenderCallback(SoGLPreRenderCB * func, void * userdata)
{
  THIS->precblist.removeCallback((SoCallbackListCB*) func, userdata);
}

/*!
  
  Sets the strategy used for sorting transparent objects.

  The \e CUSTOM_CALLBACK strategy enables the user to supply a
  callback which is called for each transparent shape. This strategy
  can be used if the built in sorting strategies aren't sufficient.

  The callback should return a floating point value to be used when
  sorting the objects in Coin. This floating point value is
  interpreted as a distance to the camera, and objects with higher
  values will be sorted behind objects with lower values.
  
  The callback will supply the SoGLRenderAction instance, and the path
  to the current object can be found using SoAction::getCurPath().

  \since Coin 2.5 

*/
void 
SoGLRenderAction::setSortedObjectOrderStrategy(const SortedObjectOrderStrategy strategy,
                                               SoGLSortedObjectOrderCB * cb,
                                               void * closure)
{
  THIS->sortedobjectstrategy = strategy;
  THIS->sortedobjectcb = cb;
  THIS->sortedobjectclosure = closure;
}

#undef THIS

// *************************************************************************
// methods in SoGLRenderActionP

// Private function to save transparent paths that need to be sorted.
// The transparent paths that don't need to be sorted are rendered 
// after the sorted ones.
void
SoGLRenderActionP::addSortTransPath(SoPath * path) 
{
  this->sorttranspobjpaths.append(path);
  // check and handle callback first
  if ((this->sortedobjectstrategy == SoGLRenderAction::CUSTOM_CALLBACK) &&
      (this->sortedobjectcb != NULL)) {
    this->sorttranspobjdistances.append(this->sortedobjectcb(this->sortedobjectclosure,
                                                             this->action));
    return;
  }

  SoNode * tail = ((SoFullPath*)path)->getTail();
  float dist;
  SbBox3f bbox;
  // test if we can find the bbox using SoShape::getBoundingBoxCache()
  // or SoShape::computeBBox. This is the common case, and quite a lot
  // faster than using an SoGetBoundingBoxAction.
  if (tail->isOfType(SoShape::getClassTypeId())) { // common case
    SoShape * tailshape = (SoShape*) tail;
    const SoBoundingBoxCache * bboxcache = tailshape->getBoundingBoxCache();
    SbVec3f center;
    
    if (bboxcache && bboxcache->isValid(action->state)) {
      if (bboxcache->isCenterSet()) center = bboxcache->getCenter();
      center = bboxcache->getProjectedBox().getCenter();
      bbox = bboxcache->getProjectedBox();
    }
    else {
      tailshape->computeBBox(action, bbox, center);
    }
    SoModelMatrixElement::get(action->state).multVecMatrix(center, center);
    dist = -SoViewVolumeElement::get(action->state).getPlane(0.0f).getDistance(center);
  }
  else {
    this->bboxaction->setViewportRegion(SoViewportRegionElement::get(action->state));
    this->bboxaction->apply(path);
    SbVec3f center = this->bboxaction->getBoundingBox().getCenter();
    bbox = this->bboxaction->getBoundingBox();
    bbox.transform(SoModelMatrixElement::get(action->state).inverse());
    dist = -SoViewVolumeElement::get(action->state).getPlane(0.0f).getDistance(center);
  }
  if ((this->sortedobjectstrategy == SoGLRenderAction::BBOX_CLOSEST_CORNER) ||
      (this->sortedobjectstrategy == SoGLRenderAction::BBOX_FARTHEST_CORNER)) {
    const SbMatrix & m = SoModelMatrixElement::get(action->state);
    const SbPlane & plane = SoViewVolumeElement::get(action->state).getPlane(0.0f);
    SbVec3f bmin, bmax;
    bmin = bbox.getMin();
    bmax = bbox.getMax();
    
    for (int i = 0; i < 8; i++) {
      SbVec3f tmp(i&1 ? bmin[0] : bmax[0],
                  i&2 ? bmin[1] : bmax[1],
                  i&4 ? bmin[2] : bmax[2]);
      m.multVecMatrix(tmp, tmp);
      float tmpdist = -plane.getDistance(tmp);
      if (i == 0) dist = tmpdist;
      else {
        switch (this->sortedobjectstrategy) {
        case SoGLRenderAction::BBOX_CLOSEST_CORNER: 
          if (tmpdist < dist) dist = tmpdist;
          break;
        case SoGLRenderAction::BBOX_FARTHEST_CORNER:
          if (tmpdist > dist) dist = tmpdist;
          break;
        default:
          assert(0 && "unknown sorting strategy");
          break;
        }
      }
    }
  }
  this->sorttranspobjdistances.append(dist);
}

// Private function which "unwinds" the real value of the "rendering"
// variable.
SbBool
SoGLRenderActionP::isDirectRendering(const SoState * state) const
{
  SbBool isdirect;
  if (this->rendering == RENDERING_UNSET) {
    const cc_glglue * w = sogl_glue_instance(state);
    isdirect = cc_glglue_isdirect(w);
  }
  else {
    isdirect = this->rendering == RENDERING_SET_DIRECT;
  }

  // Update to keep in sync.
  this->action->setRenderingIsRemote(!isdirect);

  return isdirect;
}


//
// render the scene. Called from beginTraversal()
//
void
SoGLRenderActionP::render(SoNode * node)
{
  this->isrendering = TRUE;

  this->currentpass = 0;
  
  SoState * state = this->action->getState();
  state->push();

  SoShapeStyleElement::setTransparencyType(state,
                                           this->transparencytype);

  SoLazyElement::disableBlending(state);
  
  SoViewportRegionElement::set(state, this->viewport);
  SoLazyElement::setTransparencyType(state,
                                     (int32_t)this->transparencytype);

  if (this->transparencytype == SoGLRenderAction::SORTED_LAYERS_BLEND) {
    SoOverrideElement::setTransparencyTypeOverride(state, node, TRUE);
  }

  SoLazyElement::setColorMaterial(state, TRUE);

  SoGLUpdateAreaElement::set(state,
                             this->updateorigin, this->updatesize);

  SoGLCacheContextElement::set(state, this->cachecontext,
                               FALSE, !this->isDirectRendering(state));
  SoGLRenderPassElement::set(state, 0);

  this->precblist.invokeCallbacks((void*) this->action);

  if (this->action->getNumPasses() > 1) {
    // Check if the current OpenGL context has an accumulation buffer
    // (rendering multiple passes doesn't make much sense otherwise).
    GLint accumbits;
    glGetIntegerv(GL_ACCUM_RED_BITS, &accumbits);
    
    if (accumbits == 0) {
      static SbBool first = TRUE;
      if (first) {
        SoDebugError::postWarning("SoGLRenderActionP::render", 
                                  "Multipass rendering requested,\nbut current "
                                  "GL context has no accumulation buffer - "
                                  "falling back to single pass\nrendering.");
        first = FALSE;
      }
      this->renderSingle(node);
    } else {
      this->renderMulti(node);
    }
  } else {
    this->renderSingle(node);
  }

  state->pop();
  this->isrendering = FALSE;
}

//
// render multiple passes (antialiasing)
//
void
SoGLRenderActionP::renderMulti(SoNode * node)
{
  assert(this->numpasses > 1);
  float fraction = 1.0f / float(this->numpasses);

  this->currentpass = 0;
  this->renderSingle(node);
  if (this->action->hasTerminated()) return;

  glAccum(GL_LOAD, fraction);

  for (int i = 1; i < this->numpasses; i++) {
    if (this->passupdate) {
      glAccum(GL_RETURN, float(this->numpasses) / float(i));
    }
    if (this->passcallback) this->passcallback(this->passcallbackdata);
    else glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    this->currentpass = i;
    this->renderSingle(node);

    if (this->action->hasTerminated()) return;
    glAccum(GL_ACCUM, fraction);
  }
  glAccum(GL_RETURN, 1.0f);
}

//
// render a single pass. Might start a transparency or delayed pass
// though.
//
void
SoGLRenderActionP::renderSingle(SoNode * node)
{
  SoState * state = this->action->getState();

  SoGLRenderPassElement::set(state, this->currentpass);
  SoGLCacheContextElement::set(state, this->cachecontext,
                               FALSE, !this->isDirectRendering(state));

  assert(this->delayedpathrender == FALSE);
  assert(this->transparencyrender == FALSE);

  // Truncate just in case
  this->sorttranspobjpaths.truncate(0);
  this->transpobjpaths.truncate(0);
  this->sorttranspobjdistances.truncate(0);
  this->delayedpaths.truncate(0);

  // Do order independent transparency rendering
  if (this->transparencytype == SoGLRenderAction::SORTED_LAYERS_BLEND) {
    GLint depthbits, alphabits;
    glGetIntegerv(GL_DEPTH_BITS, &depthbits);
    glGetIntegerv(GL_ALPHA_BITS, &alphabits);
    
    const cc_glglue * w = sogl_glue_instance(state);
    // FIXME: What should we do when >8bits per channel becomes normal? (20031125 handegar)
    if (cc_glglue_can_do_sortedlayersblend(w) && (depthbits >= 24) && (alphabits == 8)) {
      doSortedLayersBlendRendering(state, node);
    }
    else {
      
      if (!cc_glglue_can_do_sortedlayersblend(w))
        SoDebugError::postWarning("renderSingle", "Sorted layers blend cannot be enabled "
                                  "due to missing OpenGL extensions. Rendering using "
                                  "SORTED_OBJECTS_BLEND instead.");
      else
        SoDebugError::postWarning("renderSingle", "Sorted layers blend cannot be enabled if "
                                  "ALPHA size != 8 (currently %d) or DEPTH size < 24 "
                                  "(currently %d). Rendering using SORTED_OBJECTS_BLEND instead.",
                                  alphabits, depthbits);
      
      // Do regular SORTED_OBJECT_BLEND if sorted layers blend is unsupported
      this->transparencytype = SoGLRenderAction::SORTED_OBJECT_BLEND;
      render(node); // Render again using the fallback transparency type.
    }
   
    return;
  }
  
  this->action->beginTraversal(node);

  if ((this->transpobjpaths.getLength() || this->sorttranspobjpaths.getLength()) && 
      !this->action->hasTerminated()) {

    this->transparencyrender = TRUE;
    // disable writing into the z-buffer when rendering transparent
    // objects
    glDepthMask(GL_FALSE);
    SoGLCacheContextElement::set(state, this->cachecontext,
                                 TRUE, !this->isDirectRendering(state));


    // All paths in the sorttranspobjpaths should be sorted
    // back-to-front and rendered
    this->action->doPathSort();
    int i;
    for (i = 0; i < this->sorttranspobjpaths.getLength(); i++) {
      this->action->apply(this->sorttranspobjpaths[i]);
    }

    // Render all transparent paths that should not be sorted
    this->action->apply(this->transpobjpaths, TRUE);

    // enable depth buffer writes again
    glDepthMask(GL_TRUE);
    this->transparencyrender = FALSE;
  }

  if (this->delayedpaths.getLength() && !this->action->hasTerminated()) {
    this->delayedpathrender = TRUE;
    this->action->apply(this->delayedpaths, TRUE);
    this->delayedpathrender = FALSE;
  }

  // truncate lists to unref paths.
  this->sorttranspobjpaths.truncate(0);
  this->transpobjpaths.truncate(0);
  this->sorttranspobjdistances.truncate(0);
  this->delayedpaths.truncate(0);

}

void 
SoGLRenderActionP::setupBlending(SoState * state, const SoGLRenderAction::TransparencyType transptype)
{

  switch (transptype) {
  case SoGLRenderAction::BLEND:
  case SoGLRenderAction::DELAYED_BLEND:
  case SoGLRenderAction::SORTED_OBJECT_BLEND:
  case SoGLRenderAction::SORTED_OBJECT_SORTED_TRIANGLE_BLEND:
    SoLazyElement::enableBlending(state, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    break;
  case SoGLRenderAction::ADD:
  case SoGLRenderAction::DELAYED_ADD:
  case SoGLRenderAction::SORTED_OBJECT_ADD:
  case SoGLRenderAction::SORTED_OBJECT_SORTED_TRIANGLE_ADD:
    SoLazyElement::enableBlending(state, GL_SRC_ALPHA, GL_ONE);
    break;
  default:
    assert(0 && "should not get here");
    break;
  }
}

void 
SoGLRenderActionP::doSortedLayersBlendRendering(const SoState * state, SoNode * node)
{

  const cc_glglue *glue = sogl_glue_instance(state);
  this->initSortedLayersBlendRendering(state);  
  this->setupSortedLayersBlendTextures(state);
  this->sortedlayersblendinitialized = TRUE;

  glDisable(GL_BLEND);

  // The 'sortedlayersblendcounter' must be global so that it can be
  // reached by 'setupNVRegisterCombiners()' at all times during the
  // scenegraph traversals.
  for(this->sortedlayersblendcounter=0; 
      this->sortedlayersblendcounter < this->sortedlayersblendpasses; 
      this->sortedlayersblendcounter++) {

    // FIXME: A trick here would be to do an occlusion test if
    // possible (EXT_occlusion_test is more or less free). The choosen
    // number of passes would then be treated as the the upper number
    // of passes. (20031208 handegar)
    renderOneBlendLayer(state, this->sortedlayersblendcounter > 0, 
                        this->sortedlayersblendcounter < (this->sortedlayersblendpasses-1), 
                        node);

  }

  // Blend together the aquired RGBA layers
  if (glue->has_arb_fragment_program && !this->usenvidiaregistercombiners)
    renderSortedLayersFP(state);
  else
    renderSortedLayersNV(state);
  
}

void 
SoGLRenderActionP::texgenEnable(SbBool enable)
{
    if (enable) {
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glEnable(GL_TEXTURE_GEN_R);
        glEnable(GL_TEXTURE_GEN_Q);
    }
    else {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDisable(GL_TEXTURE_GEN_R);
        glDisable(GL_TEXTURE_GEN_Q);
    }
}


void 
SoGLRenderActionP::eyeLinearTexgen()
{

  const float col1[] = { 1, 0, 0, 0 };
  const float col2[] = { 0, 1, 0, 0 };
  const float col3[] = { 0, 0, 1, 0 };
  const float col4[] = { 0, 0, 0, 1 };

  glTexGenfv(GL_S,GL_EYE_PLANE, col1);
  glTexGenfv(GL_T,GL_EYE_PLANE, col2);
  glTexGenfv(GL_R,GL_EYE_PLANE, col3);
  glTexGenfv(GL_Q,GL_EYE_PLANE, col4);

  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

}

void
SoGLRenderActionP::renderOneBlendLayer(const SoState * state,
                                       SbBool peel, SbBool updatedepthtexture, SoNode * node)
{

  const cc_glglue * glue = sogl_glue_instance(state);  

  // Setup clearcolor alpha value to 1.0f when blending using NVIDIA
  // extensions. Must do this every time to make sure the alpha-value
  // stays correct.
  GLfloat clearcolor[4];
  glGetFloatv(GL_COLOR_CLEAR_VALUE, clearcolor);
  if (glue->has_arb_fragment_program && !this->usenvidiaregistercombiners)
    glClearColor(clearcolor[0], clearcolor[1], clearcolor[2], 0.0f);
  else
    glClearColor(clearcolor[0], clearcolor[1], clearcolor[2], 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  

  // Clear all errors before traversal, just in case.
  GLenum glerror =  sogl_glerror_debugging() ? glGetError() : GL_NO_ERROR;
  while (glerror) {
    SoDebugError::postWarning("renderOneBlendLayer", 
                              "glError() = %d\n", glerror);
    glerror = glGetError();
  }

  // Do the rendering
  this->action->beginTraversal(node);

  if(peel) { 
    if (glue->has_arb_fragment_program && !this->usenvidiaregistercombiners) { 
      // Fragment program clean-up      
      glDisable(GL_FRAGMENT_PROGRAM_ARB);    
      glDisable(GL_TEXTURE_RECTANGLE_EXT);   
      glDisable(GL_ALPHA_TEST);

      cc_glglue_glActiveTexture(glue, GL_TEXTURE3);
      glDisable(GL_TEXTURE_RECTANGLE_EXT);  
      this->texgenEnable(FALSE);
      
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
      cc_glglue_glActiveTexture(glue, GL_TEXTURE0);
      glDisable(GL_TEXTURE_RECTANGLE_EXT);   
      glDisable(GL_ALPHA_TEST);

    } 
    else {
      // Regular NViDIA register combiner clean-up
      cc_glglue_glActiveTexture(glue, GL_TEXTURE3);
      glDisable(GL_TEXTURE_RECTANGLE_EXT);  
      this->texgenEnable(FALSE);
      
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
      cc_glglue_glActiveTexture(glue, GL_TEXTURE0);
      glDisable(GL_REGISTER_COMBINERS_NV);
      glDisable(GL_ALPHA_TEST);

    }
  }

  if (!glue->has_arb_fragment_program || this->usenvidiaregistercombiners) 
    glDisable(GL_TEXTURE_SHADER_NV);

  // FIXME: It might be a smart thing to use PBuffers for the RGBA
  // layers instead of copying from the framebuffer. The copying seems
  // to be a performance hit for large canvases. (20031127 handegar)

  // copy the RGBA of the layer to a texture
  glEnable(GL_TEXTURE_RECTANGLE_EXT);
  glBindTexture(GL_TEXTURE_RECTANGLE_EXT, this->rgbatextureids[this->sortedlayersblendcounter]);
  glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, 0, 0, 
                      this->viewportwidth, this->viewportheight);

  if (updatedepthtexture) {
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, this->depthtextureid);
    glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, 0, 0, 
                        this->viewportwidth, this->viewportheight);
  }
  
}

void 
SoGLRenderActionP::initSortedLayersBlendRendering(const SoState * state)
{

  if (this->sortedlayersblendinitialized) // Do this only once
    return;

  // Supporting both the TGS envvar and the COIN envvar. If both are
  // present, the COIN envvar will be used. 
  const char * envtgs = coin_getenv("OIV_NUM_SORTED_LAYERS_PASSES");
  if (envtgs && (atoi(envtgs) > 0))
    this->sortedlayersblendpasses = atoi(envtgs);
 
  const char * envcoin = coin_getenv("COIN_NUM_SORTED_LAYERS_PASSES");
  if (envcoin && (atoi(envcoin) > 0))
    this->sortedlayersblendpasses = atoi(envcoin);
  
  const char * envusenvidiarc = coin_getenv("COIN_SORTED_LAYERS_USE_NVIDIA_RC");
  if (envusenvidiarc && (atoi(envusenvidiarc) > 0))
    this->usenvidiaregistercombiners = TRUE;

  this->rgbatextureids = new GLuint[this->sortedlayersblendpasses];

  const cc_glglue * glue = sogl_glue_instance(state);  
  if (glue->has_arb_fragment_program && !this->usenvidiaregistercombiners) {
  
    // Initialize fragment program
    //
    // FIXME: the program id must be bound to the current rendering
    // context, and deallocated when it is destructed. 20040718 mortene.
    glue->glGenProgramsARB(1, &this->sortedlayersblendprogramid);
    glue->glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, this->sortedlayersblendprogramid);
    glue->glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                             (GLsizei)strlen(sortedlayersblendprogram),
                             sortedlayersblendprogram);
   
    // FIXME: Maybe a wrapper for catching fragment program errors
    // should be a part of GLUE... (20031204 handegar)
    GLint errorPos;    
    GLenum err = sogl_glerror_debugging() ? glGetError() : GL_NO_ERROR;
    if (err) {
      glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
      SoDebugError::postWarning("initSortedLayersBlendRendering",
                                "Error in fragment program! (byte pos: %d) '%s'.\n", 
                                errorPos, glGetString(GL_PROGRAM_ERROR_STRING_ARB));
      
    }

    glDisable(GL_FRAGMENT_PROGRAM_ARB);

  }

}

void
SoGLRenderActionP::setupFragmentProgram()
{ 

  if (this->sortedlayersblendcounter == 0)  // Is this not the first pass?
    return;
  const cc_glglue * glue = sogl_glue_instance(this->action->getState()); 


  glEnable(GL_FRAGMENT_PROGRAM_ARB);
  glue->glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, this->sortedlayersblendprogramid);
 
  // UNIT #3
  glMatrixMode(GL_MODELVIEW);   
  cc_glglue_glActiveTexture(glue, GL_TEXTURE3);

  glBindTexture(GL_TEXTURE_RECTANGLE_NV, this->depthtextureid);
  glEnable(GL_TEXTURE_RECTANGLE_NV);
  
  glPushMatrix();
  glLoadIdentity();
  this->eyeLinearTexgen();
  glPopMatrix();  
  this->texgenEnable(TRUE);
    
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();  
  glScalef(this->viewportwidth, this->viewportheight, 1);
  glTranslatef(0.5, 0.5, 0.5);
  glScalef(0.5, 0.5, 0.5);
  glMultMatrixf((float *) this->sortedlayersblendprojectionmatrix);  
  glMatrixMode(GL_MODELVIEW);
    
  glAlphaFunc(GL_GREATER, 0);
  glEnable(GL_ALPHA_TEST);
  
  // UNIT #0
  cc_glglue_glActiveTexture(glue, GL_TEXTURE0);

}


void
SoGLRenderActionP::setupRegisterCombinersNV()
{ 

  //
  // Setting up the texture units to handle the sorted layers blending
  //
  const cc_glglue * glue = sogl_glue_instance(this->action->getState()); 
  glEnable(GL_TEXTURE_SHADER_NV);
        
  // UNIT #0
  cc_glglue_glActiveTexture(glue, GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, this->hilotextureid);
  glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_2D);

  // UNIT #1
  cc_glglue_glActiveTexture(glue, GL_TEXTURE1);
  glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_NV);
  glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NONE);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  this->eyeLinearTexgen();
  this->texgenEnable(TRUE);
  glPopMatrix();

  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glTranslatef(0.0f, 0.0f, 0.5f);
  glScalef(0.0f, 0.0f, 0.5f);
  glMultMatrixf((float *) this->sortedlayersblendprojectionmatrix);
  glMatrixMode(GL_MODELVIEW);
     
  // UNIT #2
  cc_glglue_glActiveTexture(glue, GL_TEXTURE2);
  glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_DOT_PRODUCT_DEPTH_REPLACE_NV);
  glTexEnvi(GL_TEXTURE_SHADER_NV, GL_PREVIOUS_TEXTURE_INPUT_NV, GL_TEXTURE0);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NONE);

  glPushMatrix();
  glLoadIdentity();
  this->eyeLinearTexgen();
  this->texgenEnable(TRUE);
  glPopMatrix();

  glMatrixMode(GL_TEXTURE);
  GLdouble m[16]; 
  m[0   + 0] = 0; m[0   + 1] = 0; m[0   + 2] = 0; m[0   + 3] = 0; 
  m[1*4 + 0] = 0; m[1*4 + 1] = 0; m[1*4 + 2] = 0; m[1*4 + 3] = 0; 
  m[2*4 + 0] = 0; m[2*4 + 1] = 0; m[2*4 + 2] = 0; m[2*4 + 3] = 0; 
  m[3*4 + 0] = 0; m[3*4 + 1] = 0; m[3*4 + 2] = 1; m[3*4 + 3] = 0;
  glLoadMatrixd(m);
  glMultMatrixf((float *) this->sortedlayersblendprojectionmatrix);
  glMatrixMode(GL_MODELVIEW);
        
  // UNIT #0
  cc_glglue_glActiveTexture(glue, GL_TEXTURE0);
       
  if (this->sortedlayersblendcounter > 0) { // Is this not the first pass?

    cc_glglue_glActiveTexture(glue, GL_TEXTURE3);
    glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_TEXTURE_RECTANGLE_NV);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_NONE);

    glPushMatrix();
    glLoadIdentity();
    this->eyeLinearTexgen();
    glPopMatrix();
    this->texgenEnable(TRUE);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(this->viewportwidth, this->viewportheight, 1);
    glTranslatef(.5,.5,.5);
    glScalef(.5,.5,.5);
    glMultMatrixf((float *) this->sortedlayersblendprojectionmatrix);
    glMatrixMode(GL_MODELVIEW);
    
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, this->depthtextureid);
    glEnable(GL_TEXTURE_RECTANGLE_NV);
    
    // UNIT #0
    cc_glglue_glActiveTexture(glue, GL_TEXTURE0);
        
    //
    // Register combiners 1.0 script:    
    //  !!RC1.0                                                      
    //  { 
    //    rgb { spare0 = unsigned_invert(tex3) * col0.a; } 
    //  }         
    //  out.rgb = col0;                                              
    //  out.a = spare0.b; 
    //
    glue->glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);
    glue->glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE3, 
                            GL_UNSIGNED_INVERT_NV, GL_RGB);
    glue->glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, 
                            GL_PRIMARY_COLOR_NV, GL_SIGNED_IDENTITY_NV, GL_ALPHA);
    glue->glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV, GL_ZERO, 
                            GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    glue->glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV, GL_ZERO, 
                            GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    glue->glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB, GL_SPARE0_NV, GL_DISCARD_NV, 
                             GL_DISCARD_NV, GL_ZERO, GL_ZERO, GL_FALSE, GL_FALSE, GL_FALSE);
    glue->glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_ZERO,
                            GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
    glue->glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_ZERO, 
                            GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
    glue->glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_C_NV, GL_ZERO,
                            GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
    glue->glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_D_NV, GL_ZERO,
                            GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
    glue->glCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, 
                             GL_DISCARD_NV, GL_ZERO, GL_ZERO, GL_FALSE, GL_FALSE, GL_FALSE);
    
    glue->glCombinerParameteriNV(GL_COLOR_SUM_CLAMP_NV, 0);
    glue->glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_ZERO, 
                                 GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    glue->glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_ZERO, 
                                 GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    glue->glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_ZERO, 
                                 GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    glue->glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_PRIMARY_COLOR_NV, 
                                 GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    glue->glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_ZERO, 
                                 GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    glue->glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_ZERO, 
                                 GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    glue->glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_SPARE0_NV, 
                                 GL_UNSIGNED_IDENTITY_NV, GL_BLUE);
    
    glEnable(GL_REGISTER_COMBINERS_NV);
    
    glAlphaFunc(GL_GREATER, 0);
    glEnable(GL_ALPHA_TEST);

  }

  glMatrixMode(GL_MODELVIEW);

 
}

void
SoGLRenderActionP::setupSortedLayersBlendTextures(const SoState * state)
{

  const SbViewportRegion & vpr = this->action->getViewportRegion();
  const SbVec2s & canvassize = vpr.getViewportSizePixels();

  // Do we have to reinitialize the textures?
  if (((canvassize[1] != this->viewportheight) || 
       (canvassize[0] != this->viewportwidth)) || 
      !this->sortedlayersblendinitialized) {
        
    const cc_glglue *glue = sogl_glue_instance(state);


    if (this->sortedlayersblendinitialized) {      
      // Remove the old textures to make room for new ones if size has changed.
      glDeleteTextures(1, &this->depthtextureid);
      glDeleteTextures(this->sortedlayersblendpasses, this->rgbatextureids);
    }
    
    // Depth texture setup
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  
    // FIXME: the texture id must be bound to the current rendering
    // context, and deallocated when it is destructed. 20040718 mortene.
    glGenTextures(1, &this->depthtextureid);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, this->depthtextureid);      
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_DEPTH_COMPONENT24, canvassize[0], canvassize[1],
                 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    if (glue->has_arb_fragment_program && !this->usenvidiaregistercombiners) {
      // Not disabled as default by NVIDIA when using fragment programs (according to spec.)
      glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_COMPARE_MODE, GL_NONE); 
    }
    else {
      glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
      glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
    
    // The "register combiner"-way if explicitly choosen or FP is unavailable 
    if(this->usenvidiaregistercombiners) { 
      // HILO texture setup
      GLushort HILOtexture[] = {0, 0};
      // FIXME: the texture id must be bound to the current rendering
      // context, and deallocated when it is destructed. 20040718 mortene.
      glGenTextures(1, &this->hilotextureid);
      glBindTexture(GL_TEXTURE_2D, this->hilotextureid);  
      glTexImage2D(GL_TEXTURE_2D, 0, GL_HILO_NV, 1, 1, 0, GL_HILO_NV, 
                   GL_UNSIGNED_SHORT, &HILOtexture); 
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }    


    // RGBA layers setup 
    // FIXME: What if channels are > 8 bits? This must be examined
    // closer... [Only highend ATI cards supports these resolutions if
    // I'm not mistaken.] (20031126 handegar)
    //
    // FIXME: the texture ids must be bound to the current rendering
    // context, and deallocated when it is destructed. 20040718 mortene.
    glGenTextures(this->sortedlayersblendpasses, this->rgbatextureids);
    for (int i=0;i<sortedlayersblendpasses;++i) {
      glBindTexture(GL_TEXTURE_RECTANGLE_EXT, this->rgbatextureids[i]);  
      glCopyTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA8, 0, 0, canvassize[0], canvassize[1], 0);
      glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    this->viewportwidth = canvassize[0];
    this->viewportheight = canvassize[1];

  }

}

void
SoGLRenderActionP::renderSortedLayersFP(const SoState * state)
{

  const cc_glglue * glue = sogl_glue_instance(state);  

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, this->viewportwidth, 0, this->viewportheight, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glDisable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT);
  
  SbBool cullface = glIsEnabled(GL_CULL_FACE);  
  SbBool lighting = glIsEnabled(GL_LIGHTING);

  glDisable(GL_CULL_FACE);
  glDisable(GL_FRAGMENT_PROGRAM_ARB);  
  glDisable(GL_ALPHA_TEST);

  cc_glglue_glActiveTexture(glue, GL_TEXTURE0);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_BLEND);  
  glDisable(GL_LIGHTING);
  glColor3f(1.0f,1.0f,1.0f);
  glEnable(GL_TEXTURE_RECTANGLE_EXT);  

  for(int i=this->sortedlayersblendpasses-1;i>=0;--i) {
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, this->rgbatextureids[i]);    
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(0, this->viewportheight);
    glVertex2f(0, this->viewportheight);
    glTexCoord2f(this->viewportwidth, this->viewportheight);
    glVertex2f(this->viewportwidth, this->viewportheight);
    glTexCoord2f(this->viewportwidth, 0);
    glVertex2f(this->viewportwidth, 0);
    glEnd();
  }

  glDisable(GL_TEXTURE_RECTANGLE_EXT);
  
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);

  if (cullface)
    glEnable(GL_CULL_FACE);

  if (lighting)
    glEnable(GL_LIGHTING);

}

void
SoGLRenderActionP::renderSortedLayersNV(const SoState * state)
{

  const cc_glglue * glue = sogl_glue_instance(state);  

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, this->viewportwidth, 0, this->viewportheight, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glDisable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT);

  
  // Must make sure that the GL_CULL_FACE state is preserved if the scene
  // contains both solid and non-solid shapes.
  SbBool cullface = glIsEnabled(GL_CULL_FACE);  
  glDisable(GL_CULL_FACE);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
  glEnable(GL_BLEND);  

  cc_glglue_glActiveTexture(glue, GL_TEXTURE0);
      
  //
  //  Register combiners 1.0 script:
  //   !!RC1.0
  //   rgb.out = tex0;
  //   rgb.a = tex0;
  //  
  glue->glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);
  glue->glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_ZERO, 
                          GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  glue->glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_ZERO, 
                          GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  glue->glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV, GL_ZERO, 
                          GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  glue->glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV, GL_ZERO, 
                          GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  glue->glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, 
                           GL_DISCARD_NV, GL_ZERO, GL_ZERO, GL_FALSE, GL_FALSE, GL_FALSE);
  glue->glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_A_NV, GL_ZERO, 
                          GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
  glue->glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_B_NV, GL_ZERO, 
                          GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
  glue->glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_C_NV, GL_ZERO, 
                          GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
  glue->glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_D_NV, GL_ZERO, 
                          GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
  glue->glCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA, GL_DISCARD_NV, GL_DISCARD_NV, 
                           GL_DISCARD_NV, GL_ZERO, GL_ZERO, GL_FALSE, GL_FALSE, GL_FALSE);  

  glue->glCombinerParameteriNV(GL_COLOR_SUM_CLAMP_NV, 0);
  glue->glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_ZERO, 
                               GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  glue->glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_ZERO, 
                               GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  glue->glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_ZERO, 
                               GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  glue->glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_TEXTURE0, 
                               GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  glue->glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_ZERO,
                               GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  glue->glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_ZERO, 
                               GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  glue->glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_TEXTURE0, 
                               GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
  
  glEnable(GL_REGISTER_COMBINERS_NV);  
  glEnable(GL_TEXTURE_RECTANGLE_EXT);  
  
  for(int i=this->sortedlayersblendpasses-1;i>=0;--i) {
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, this->rgbatextureids[i]);    
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(0, this->viewportheight);
    glVertex2f(0, this->viewportheight);
    glTexCoord2f(this->viewportwidth, this->viewportheight);
    glVertex2f(this->viewportwidth, this->viewportheight);
    glTexCoord2f(this->viewportwidth, 0);
    glVertex2f(this->viewportwidth, 0);
    glEnd();
  }

  glDisable(GL_REGISTER_COMBINERS_NV);
  glDisable(GL_TEXTURE_RECTANGLE_EXT);
  
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
 
  if (cullface)
    glEnable(GL_CULL_FACE);
  
}

// *************************************************************************
