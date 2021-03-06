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
  \class SoShadowGroup SoShadowGroup.h FXViz/nodes/SoSeparator.h
  \brief The SoShadowGroup node is a group node used for shadow rendering.
  \ingroup fxviz

  Children of this node can recieve shadows, and cast shadows on other children.
  Use the SoShadowStyle node to control shadow casters and shadow receivers.

  Please note that all shadow casters will be rendered twice. Once to
  create the shadow map, and once for normal rendering. If you're
  having performance issues, you should consider reducing the number of
  shadow casters.

  The algorithm used to render the shadows is Variance Shadow Maps
  (http://www.punkuser.net/vsm/). As an extra bonus, all geometry rendered with shadows can also
  be rendered with per fragment phong lighting.

  This node will search its subgraph and calculate shadows for all
  SoSpotLight nodes. The node will use one texture unit for each spot
  light, so for this node to work 100%, you need to have
  num-spotlights free texture units while rendering the subgraph.

  Currently, we only support scenes with maximum two texture units
  active while doing shadow rendering (unit 0 and unit 1). This is due
  to the fact that we emulate the OpenGL shading model in a shader
  program, and we're still working on creating a solution that updates
  the shader program during the scene graph traversal. Right now a
  shader program is created when entering the SoShadowGroup node, and
  this is used for the entire subgraph.


  <b>FILE FORMAT/DEFAULTS:</b>
  \code
    SoShadowGroup {
      isActive TRUE
      intensity 0.5
      precision 0.5
      quality 0.5
      shadowCachingEnabled TRUE
      visibilityRadius -1.0
      visibilityFlag LONGEST_BBOX_EDGE_FACTOR

      epsilon 0.00001
      threshold 0.1
      smoothBorder 0.0

    }
  \endcode

  Example scene graph:
  \code

  # to get some lighting when headlight is turned off in the viewer
  DirectionalLight { direction 0 0 -1 intensity 0.2 }

  ShadowGroup {
    quality 1 # to get per pixel lighting

    ShadowStyle { style NO_SHADOWING }

    SpotLight {
      location -8 -8 8.0
      direction 1 1 -1
      cutOffAngle 0.35
      dropOffRate 0.7
    }

    ShadowStyle { style CASTS_SHADOW_AND_SHADOWED }

    Separator {
      Complexity { value 1.0 }
      Material { diffuseColor 1 1 0 specularColor 1 1 1 shininess 0.9 }
      Shuttle { translation0 -3 1 0 translation1 3 -5 0 speed 0.25 on TRUE }
      Translation { translation -5 0 2 }
      Sphere { radius 2.0 }
    }

    Separator {
      Material { diffuseColor 1 0 0 specularColor 1 1 1 shininess 0.9 }
      Shuttle { translation0 0 -5 0 translation1 0 5 0 speed 0.15 on TRUE }
      Translation { translation 0 0 -3 }
      Cube { depth 1.8 }
    }
    Separator {
      Material { diffuseColor 0 1 0 specularColor 1 1 1 shininess 0.9 }
      Shuttle { translation0 -5 0 0 translation1 5 0 0 speed 0.3 on TRUE }
      Translation { translation 0 0 -3 }
      Cube { }
    }

    ShadowStyle { style SHADOWED }
    Coordinate3 { point [ -10 -10 -3, 10 -10 -3, 10 10 -3, -10 10 -3 ] }
    Material { specularColor 1 1 1 shininess 0.9 }

    Complexity { textureQuality 0.1 }
    Texture2 { image 2 2 3 0xffffff 0x225588 0x225588 0xffffff }
    Texture2Transform { scaleFactor 4 4 }
    FaceSet { numVertices 4 }
  }

  \endcode

  \since Coin 2.5
*/


/*!
  \var SoSFBool SoShadowGroup::isActive

  Use this field to turn shadow rendering for the subgraph
  on/off. Default value is TRUE.
*/

/*!
  \var SoSFFloat SoShadowGroup::intensity

  Not used yet. Provided for TGS Inventor compatibility.
*/

/*!
  \var SoSFFloat SoShadowGroup::precision

  Use to calculate the size of the shadow map. A precision of 1.0
  means the maximum shadow buffer size will be used (typically
  2048x2048 on current graphics cards). Default value is 0.5.
*/

/*!
  \var SoSFFloat SoShadowGroup::quality

  Can be used to tune the shader program complexity. A higher value
  will mean that more calculations are done per-fragment instead of
  per-vertex. Default value is 0.5.

*/

/*!
  \var SoSFBool SoShadowGroup::shadowCachingEnabled

  Not used yet. Provided for TGS Inventor compatibility.
*/

/*!
  \var SoSFFloat SoShadowGroup::visibilityNearRadius

  Can be used to manually set the near clipping plane of the shadow
  maps.  If a negative value is provided, the group will calculate a
  near plane based on the bounding box of the children. Default value
  is -1.0.

  \sa visibilityFlag
*/

/*!
  \var SoSFFloat SoShadowGroup::visibilityRadius

  Can be used to manually set the far clipping plane of the shadow
  maps.  If a negative value is provided, the group will calculate a
  near plane based on the bounding box of the children. Default value
  is -1.0.

  \sa visibilityFlag
*/

/*!
  \var SoSFEnum SoShadowGroup::visibilityFlag

  Determines how visibilityRadius and visibilitNearRadius is used to
  calculate near and far clipping planes for the shadow volume.
*/

/*!
  SoShadowGroup::VisibilityFlag SoShadowGroup::ABSOLUTE_RADIUS

  The absolute values of visibilityNearRadius and visibilityRadius will be used.
*/

/*!
  SoShadowGroup::VisibilityFlag SoShadowGroup::LONGEST_BBOX_EDGE_FACTOR

  The longest bbox edge will be used to determine near and far clipping planes.

*/

/*!
  SoShadowGroup::VisibilityFlag SoShadowGroup::PROJECTED_BBOX_DEPTH_FACTOR

  The bbox depth (projected to face the camera) will be used to calculate the clipping planes.

*/

/*!
  \var SoSFInt32 SoShadowGroup::smoothBorder

  We have some problems with this feature so it's not supported at the
  moment.

  Used to add shadow border smoothing. This is currently done as a
  post processing step on the shadow map. The algorithm used is Gauss
  Smoothing, but in the future we'll probably change this, and use a
  summed area sampling merhod instead. The value should be a
  number between 0 (no smoothing), and 1 (max smoothing).

  If you want to enable smoothing, choosing a low value (~0.1) works
  best in the current implementation.

  Default value is 0.0.
*/

/*!
  \var SoSFFloat SoShadowGroup::epsilon

  Epsilon is used to offset the shadow map depth from the model depth.
  Should be set to as low a number as possible without causing
  flickering in the shadows or on non-shadowed objects. Default value
  is 0.00001.
*/

/*!
  \var SoSFFloat SoShadowGroup::threshold

  Can be used to avoid light bleeding in merged shadows cast from different objects.

  A threshold to completely eliminate all light bleeding can be
  computed from the ratio of overlapping occluder distances from the
  light's perspective. See
  http://forum.beyond3d.com/showthread.php?t=38165 for a discussion
  about this problem.

*/


// use to increase the VSM precision by using all four components
#define DISTRIBUTE_FACTOR 64.0

// use to increase precision by one bit at the cost of some extra processing
#define USE_NEGATIVE 1

// *************************************************************************

#include <FXViz/nodes/SoShadowGroup.h>
#include <Inventor/nodes/SoSubNodeP.h>
#include <Inventor/nodes/SoSpotLight.h>
#include <Inventor/nodes/SoPointLight.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoSceneTexture2.h>
#include <Inventor/nodes/SoTransparencyType.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoVertexShader.h>
#include <Inventor/nodes/SoFragmentShader.h>
#include <Inventor/nodes/SoShaderProgram.h>
#include <Inventor/nodes/SoShaderParameter.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/elements/SoShapeStyleElement.h>
#include <Inventor/elements/SoLightElement.h>
#include <Inventor/elements/SoTextureMatrixElement.h>
#include <Inventor/elements/SoMultiTextureMatrixElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoTextureQualityElement.h>
#include <Inventor/elements/SoMaterialBindingElement.h>
#include <Inventor/elements/SoNormalElement.h>
#include <Inventor/elements/SoOverrideElement.h>
#include <Inventor/elements/SoTextureOverrideElement.h>
#include <Inventor/elements/SoEnvironmentElement.h>
#include <Inventor/annex/FXViz/elements/SoShadowStyleElement.h>
#include <Inventor/annex/FXViz/elements/SoGLShadowCullingElement.h>
#include <Inventor/annex/FXViz/nodes/SoShadowStyle.h>
#include <Inventor/annex/FXViz/nodes/SoShadowCulling.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoTextureUnit.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/SoPath.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/elements/SoShapeStyleElement.h>
#include <Inventor/elements/SoTextureUnitElement.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/elements/SoGLMultiTextureEnabledElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/misc/SoShaderGenerator.h>
#include <Inventor/caches/SoShaderProgramCache.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/SbMatrix.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/C/glue/glp.h>
#include <math.h>
#include "../shaders/SoShader.h"

// *************************************************************************

class SoShadowSpotLightCache {
public:
  SoShadowSpotLightCache(SoState * state,
                         const SoPath * path, SoShadowGroup * scene,
                         const int gausskernelsize,
                         const float gaussstandarddeviation)
  {
    const cc_glglue * glue = cc_glglue_instance(SoGLCacheContextElement::get(state));

    GLint maxsize = 2048;
    GLint maxtexsize = 2048;

    // Testing for maximum proxy texture size doesn't seem to work, so
    // we just have to hardcode the maximum size to 2048 for now.  We
    // still use the proxy texture test in case the maximum size is
    // something smaller than 2048 though.  pederb, 2007-05-03

    // glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &maxsize);
    // glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtexsize);
    // if (maxtexsize < maxsize) maxsize = maxtexsize;

    GLenum internalformat = GL_RGBA16F_ARB;
    GLenum format = GL_RGBA;
    GLenum type = GL_FLOAT;

    while (!coin_glglue_is_texture_size_legal(glue, maxsize, maxsize, 0, internalformat, format, type, TRUE)) {
      maxsize >>= 1;
    }
    const int TEXSIZE = coin_geq_power_of_two((int) (scene->precision.getValue() * SbMin(maxsize, maxtexsize)));

    this->vsm_program = NULL;
    this->vsm_farval = NULL;
    this->vsm_nearval = NULL;
    this->gaussmap = NULL;
    this->texunit = -1;

    this->fragment_farval = new SoShaderParameter1f;
    this->fragment_farval->ref();

    this->fragment_nearval = new SoShaderParameter1f;
    this->fragment_nearval->ref();

    this->createVSMProgram();

    this->path = path->copy();
    this->path->ref();
    assert(((SoFullPath*)path)->getTail()->isOfType(SoSpotLight::getClassTypeId()));

    this->light = (SoSpotLight*) ((SoFullPath*)path)->getTail();
    this->light->ref();
    this->depthmap = new SoSceneTexture2;
    this->depthmap->ref();
    this->depthmap->transparencyFunction = SoSceneTexture2::NONE;
    this->depthmap->size = SbVec2s(TEXSIZE, TEXSIZE);
    this->depthmap->wrapS = SoSceneTexture2::CLAMP_TO_BORDER;
    this->depthmap->wrapT = SoSceneTexture2::CLAMP_TO_BORDER;

    if (this->vsm_program) {
      this->depthmap->setType(SoSceneTexture2::RGBA32F);
      this->depthmap->backgroundColor = SbVec4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else {
      this->depthmap->setType(SoSceneTexture2::DEPTH);
    }
    SoTransparencyType * tt = new SoTransparencyType;
    tt->value = SoTransparencyType::NONE;

    this->depthmap->setSceneTransparencyType(tt);

    this->camera = new SoPerspectiveCamera;
    this->camera->ref();
    this->camera->viewportMapping = SoCamera::LEAVE_ALONE;

    SoSeparator * sep = new SoSeparator;
    sep->addChild(this->camera);

    SoCallback * cb = new SoCallback;
    cb->setCallback(shadowmap_glcallback, this);

    sep->addChild(cb);
    if (this->vsm_program) sep->addChild(this->vsm_program);

    for (int i = 0; i < scene->getNumChildren(); i++) {
      sep->addChild(scene->getChild(i));
    }

    cb = new SoCallback;
    cb->setCallback(shadowmap_post_glcallback, this);
    sep->addChild(cb);

    this->depthmap->scene = sep;
    this->matrix = SbMatrix::identity();

    if (gausskernelsize > 0) {
      this->gaussmap = new SoSceneTexture2;
      this->gaussmap->ref();
      this->gaussmap->transparencyFunction = SoSceneTexture2::NONE;
      this->gaussmap->size = SbVec2s(TEXSIZE, TEXSIZE);
      this->gaussmap->wrapS = SoSceneTexture2::CLAMP_TO_BORDER;
      this->gaussmap->wrapT = SoSceneTexture2::CLAMP_TO_BORDER;

      this->gaussmap->setType(SoSceneTexture2::RGBA32F);
      this->gaussmap->backgroundColor = SbVec4f(1.0f, 1.0f, 1.0f, 1.0f);

      SoShaderProgram * shader = this->createGaussFilter(TEXSIZE, gausskernelsize, gaussstandarddeviation);
      this->gaussmap->scene = this->createGaussSG(shader, this->depthmap);
    }
  }
  ~SoShadowSpotLightCache() {
    if (this->vsm_program) this->vsm_program->unref();
    if (this->vsm_farval) this->vsm_farval->unref();
    if (this->vsm_nearval) this->vsm_nearval->unref();
    if (this->fragment_farval) this->fragment_farval->unref();
    if (this->fragment_nearval) this->fragment_nearval->unref();
    if (this->light) this->light->unref();
    if (this->path) this->path->unref();
    if (this->gaussmap) this->gaussmap->unref();
    if (this->depthmap) this->depthmap->unref();
    if (this->camera) this->camera->unref();
  }

  static void shadowmap_glcallback(void * closure, SoAction * action);
  static void shadowmap_post_glcallback(void * closure, SoAction * action);
  void createVSMProgram(void);
  SoShaderProgram * createGaussFilter(const int texsize, const int size, const float stdev);
  SoSeparator * createGaussSG(SoShaderProgram * program, SoSceneTexture2 * tex);


  SbMatrix matrix;
  SoPath * path;
  SoSpotLight * light;
  SoSceneTexture2 * depthmap;
  SoSceneTexture2 * gaussmap;
  SoPerspectiveCamera * camera;
  float farval;
  float nearval;
  int texunit;

  SoShaderProgram * vsm_program;
  SoShaderParameter1f * vsm_farval;
  SoShaderParameter1f * vsm_nearval;
  SoShaderParameter1f * fragment_farval;
  SoShaderParameter1f * fragment_nearval;
  SoShaderGenerator vsm_vertex_generator;
  SoShaderGenerator vsm_fragment_generator;

  SoColorPacker colorpacker;
  SbColor color;
};

class SoShadowGroupP {
public:
  SoShadowGroupP(SoShadowGroup * master) :
    master(master),
    bboxaction(SbViewportRegion(SbVec2s(100,100))),
    matrixaction(SbViewportRegion(SbVec2s(100,100))),
    spotlightsvalid(FALSE),
    needscenesearch(TRUE),
    shaderprogram(NULL),
    vertexshader(NULL),
    fragmentshader(NULL),
    vertexshadercache(NULL),
    fragmentshadercache(NULL),
    texunit0(NULL),
    texunit1(NULL),
    lightmodel(NULL),
    numtexunitsinscene(1)
  {
    this->shaderprogram = new SoShaderProgram;
    this->shaderprogram->ref();
    this->shaderprogram->setEnableCallback(shader_enable_cb, this);
    this->vertexshader = new SoVertexShader;
    this->vertexshader->ref();
    this->fragmentshader = new SoFragmentShader;
    this->fragmentshader->ref();

    this->cameratransform = new SoShaderParameterMatrix;
    this->cameratransform->name = "cameraTransform";
    this->cameratransform->ref();

    this->shaderprogram->shaderObject.set1Value(0, this->vertexshader);
    this->shaderprogram->shaderObject.set1Value(1, this->fragmentshader);
  }
  ~SoShadowGroupP() {
    if (this->lightmodel) this->lightmodel->unref();
    if (this->texunit0) this->texunit0->unref();
    if (this->texunit1) this->texunit1->unref();
    if (this->vertexshadercache) this->vertexshadercache->unref();
    if (this->fragmentshadercache) this->fragmentshadercache->unref();
    if (this->cameratransform) this->cameratransform->unref();
    if (this->vertexshader) this->vertexshader->unref();
    if (this->fragmentshader) this->fragmentshader->unref();
    if (this->shaderprogram) this->shaderprogram->unref();
    this->deleteSpotLights();
  }

  SoShaderProgram * createVSMProgram(void);

  void getQuality(SoState * state, SbBool & perpixelspot, SbBool & perpixelother) {
    float quality = this->master->quality.getValue();
    perpixelspot = FALSE;
    perpixelother = FALSE;

    if (quality > 0.3) {
      perpixelspot = TRUE;
    }
    if (quality > 0.7) {
      perpixelother = TRUE;
    }
  }
  void deleteSpotLights(void) {
    for (int i = 0; i < this->spotlights.getLength(); i++) {
      delete this->spotlights[i];
    }
    this->spotlights.truncate(0);
  }

  static void shader_enable_cb(void * closure,
                               SoState * state,
                               const SbBool enable);

  void GLRender(SoGLRenderAction * action, const SbBool inpath);
  void setVertexShader(SoState * state);
  void setFragmentShader(SoState * state);
  void updateCamera(SoShadowSpotLightCache * cache, const SbMatrix & transform);
  void renderDepthMap(SoShadowSpotLightCache * cache,
                      SoGLRenderAction * action);
  void updateSpotLights(SoGLRenderAction * action);

  int32_t getFog(SoState * state) {
    return SoEnvironmentElement::getFogType(state);
  }

  SoShadowGroup * master;
  SoSearchAction search;
  SoGetBoundingBoxAction bboxaction;
  SoGetMatrixAction matrixaction;

  SbBool spotlightsvalid;
  SbBool needscenesearch;
  SbList <SoShadowSpotLightCache*> spotlights;

  SoShaderProgram * shaderprogram;
  SoVertexShader * vertexshader;
  SoFragmentShader * fragmentshader;

  SoShaderGenerator vertexgenerator;
  SoShaderGenerator fragmentgenerator;
  SoShaderParameterMatrix * cameratransform;

  SoShaderProgramCache * vertexshadercache;
  SoShaderProgramCache * fragmentshadercache;
  SoShaderParameter1i * texunit0;
  SoShaderParameter1i * texunit1;
  SoShaderParameter1i * lightmodel;

  int numtexunitsinscene;
};

// *************************************************************************

#define PRIVATE(obj) ((obj)->pimpl)
#define PUBLIC(obj) ((obj)->master)

SO_NODE_SOURCE(SoShadowGroup);

/*!
  Default constructor.
*/
SoShadowGroup::SoShadowGroup(void)
{
  PRIVATE(this) = new SoShadowGroupP(this);

  SO_NODE_INTERNAL_CONSTRUCTOR(SoShadowGroup);

  SO_NODE_ADD_FIELD(isActive, (TRUE));
  SO_NODE_ADD_FIELD(intensity, (0.5f));
  SO_NODE_ADD_FIELD(precision, (0.5f));
  SO_NODE_ADD_FIELD(quality, (0.5f));
  SO_NODE_ADD_FIELD(shadowCachingEnabled, (TRUE));
  SO_NODE_ADD_FIELD(visibilityNearRadius, (-1.0f));
  SO_NODE_ADD_FIELD(visibilityRadius, (-1.0f));
  SO_NODE_ADD_FIELD(epsilon, (0.00001f));
  SO_NODE_ADD_FIELD(threshold, (0.1f));
  SO_NODE_ADD_FIELD(smoothBorder, (0.0f));

  SO_NODE_ADD_FIELD(visibilityFlag, (LONGEST_BBOX_EDGE_FACTOR));

  SO_NODE_DEFINE_ENUM_VALUE(VisibilityFlag, LONGEST_BBOX_EDGE_FACTOR);
  SO_NODE_DEFINE_ENUM_VALUE(VisibilityFlag, ABSOLUTE_RADIUS);
  SO_NODE_DEFINE_ENUM_VALUE(VisibilityFlag, PROJECTED_BBOX_DEPTH_FACTOR);
  SO_NODE_SET_SF_ENUM_TYPE(visibilityFlag, VisibilityFlag);

}

/*!
  Destructor.
*/
SoShadowGroup::~SoShadowGroup()
{
  delete PRIVATE(this);
}

// Doc from superclass.
void
SoShadowGroup::initClass(void)
{
  SO_NODE_INTERNAL_INIT_CLASS(SoShadowGroup, SO_FROM_COIN_2_5);
}

void
SoShadowGroup::init(void)
{
  SoShadowGroup::initClass();
  SoShadowStyleElement::initClass();
  SoGLShadowCullingElement::initClass();
  SoShadowStyle::initClass();
  SoShadowCulling::initClass();
}

void
SoShadowGroup::GLRenderBelowPath(SoGLRenderAction * action)
{
  PRIVATE(this)->GLRender(action, FALSE);
}

void
SoShadowGroup::GLRenderInPath(SoGLRenderAction * action)
{
  PRIVATE(this)->GLRender(action, TRUE);
}

void
SoShadowGroup::notify(SoNotList * nl)
{
  // FIXME: examine notification chain, and detect when an SoSpotLight is
  // changed. When this happens we can just invalidate the depth map for
  // that spot light, and not the others.

  SoNotRec * rec = nl->getLastRec();
  if (rec->getBase() != this) {
    // was not notified through a field, subgraph was changed

    rec = nl->getFirstRecAtNode();
    if (rec) {
      SoNode * node = (SoNode*) rec->getBase();
      if (node->isOfType(SoGroup::getClassTypeId())) {
        // first rec was from a group node, we need to search the scene graph again
        PRIVATE(this)->spotlightsvalid = FALSE;
        PRIVATE(this)->needscenesearch = TRUE;
      }
      else {
        PRIVATE(this)->spotlightsvalid = FALSE;        
      }
    }
  }

  if (PRIVATE(this)->vertexshadercache) {
    PRIVATE(this)->vertexshadercache->invalidate();
  }
  if (PRIVATE(this)->fragmentshadercache) {
    PRIVATE(this)->fragmentshadercache->invalidate();
  }
  inherited::notify(nl);
}

#undef PRIVATE

// *************************************************************************

void
SoShadowGroupP::updateSpotLights(SoGLRenderAction * action)
{
  int i;
  SoState * state = action->getState();

  if (!this->spotlightsvalid) {
    float smoothing = PUBLIC(this)->smoothBorder.getValue();
    smoothing = 0.0f; // FIXME: temporary until we have time to fix this feature

    int gaussmatrixsize = 0;
    float gaussstandarddeviation = 0.6f;

    // just hardcode some values for now
    if (smoothing > 0.9) gaussmatrixsize = 7;
    else if (smoothing > 0.5) gaussmatrixsize = 5;
    else if (smoothing > 0.01) gaussmatrixsize = 3;

    const cc_glglue * glue = cc_glglue_instance(SoGLCacheContextElement::get(state));

    if (this->needscenesearch || this->search.getPaths().getLength() == 0) {
      // first, search for texture unit nodes
      this->search.setType(SoTextureUnit::getClassTypeId());
      this->search.setInterest(SoSearchAction::ALL);
      this->search.setSearchingAll(FALSE);
      this->search.apply(PUBLIC(this));
      
      int lastenabled;
      (void) SoMultiTextureEnabledElement::getEnabledUnits(state, lastenabled);
      this->numtexunitsinscene = lastenabled + 1;

      for (i = 0; i < this->search.getPaths().getLength(); i++) {
        SoFullPath * p = (SoFullPath*) this->search.getPaths()[i];
        SoTextureUnit * unit = (SoTextureUnit*) p->getTail();
        if (unit->unit.getValue() >= this->numtexunitsinscene) {
          this->numtexunitsinscene = unit->unit.getValue() + 1;
        }
      }
      if (this->numtexunitsinscene == 0) this->numtexunitsinscene = 1;

      this->search.reset();
      this->search.setType(SoSpotLight::getClassTypeId());
      this->search.setInterest(SoSearchAction::ALL);
      this->search.setSearchingAll(FALSE);
      this->search.apply(PUBLIC(this));
      this->needscenesearch = FALSE;
    }
    int maxunits = cc_glglue_max_texture_units(glue);
    int maxlights = maxunits - this->numtexunitsinscene;
    SoPathList & pl = this->search.getPaths();

    int numlights = 0;
    for (i = 0; i < pl.getLength(); i++) {
      SoSpotLight * sl = (SoSpotLight*)((SoFullPath*)(pl[i]))->getTail();
      if (sl->on.getValue() && (numlights < maxlights)) numlights++;
    }

    if (numlights != this->spotlights.getLength()) {
      // just delete and recreate all if the number of spot lights have changed
      this->deleteSpotLights();
      for (i = 0; i < pl.getLength(); i++) {
        SoSpotLight * sl = (SoSpotLight*)((SoFullPath*)pl[i])->getTail();
        if (sl->on.getValue() && (this->spotlights.getLength() < maxlights)) {
          SoShadowSpotLightCache * cache = new SoShadowSpotLightCache(state, pl[i], PUBLIC(this),
                                                                      gaussmatrixsize, gaussstandarddeviation);
          this->spotlights.append(cache);
        }
      }
    }
    // validate if spot light paths are still valid
    int i2 = 0;
    for (i = 0; i < pl.getLength(); i++) {
      SoPath * path = pl[i];
      SoSpotLight * sl = (SoSpotLight*) ((SoFullPath*)path)->getTail();
      if (sl->on.getValue() && (i2 < maxlights)) {
        SoShadowSpotLightCache * cache = this->spotlights[i2];
        int unit = (maxunits - 1) - i2;
        if (unit != cache->texunit) {
          if (this->vertexshadercache) this->vertexshadercache->invalidate();
          if (this->fragmentshadercache) this->fragmentshadercache->invalidate();
          cache->texunit = unit;
        }
        if (*(cache->path) != *path) {
          cache->path->unref();
          cache->path = path->copy();
        }
        this->matrixaction.apply(path);
        
        this->updateCamera(cache, this->matrixaction.getMatrix());
        i2++;
      }
    }
    this->spotlightsvalid = TRUE;
  }
  for (i = 0; i < this->spotlights.getLength(); i++) {
    SoShadowSpotLightCache * cache = this->spotlights[i];
    assert(cache->texunit >= 0);
    SoTextureUnitElement::set(state, PUBLIC(this), cache->texunit);

    SbMatrix mat = cache->matrix;

    assert(cache->texunit >= 0);

    if (cache->texunit == 0) {
      SoTextureMatrixElement::set(state, PUBLIC(this), mat);
    }
    else {
      SoMultiTextureMatrixElement::set(state, PUBLIC(this), cache->texunit, cache->matrix);
    }

    // glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    this->renderDepthMap(cache, action);

    if (cache->texunit == 0) {
      SoGLTextureEnabledElement::set(state, PUBLIC(this), FALSE);
    }
    else {
      SoGLMultiTextureEnabledElement::set(state, PUBLIC(this), cache->texunit,
                                          SoGLMultiTextureEnabledElement::DISABLED);
    }


  }
  SoTextureUnitElement::set(state, PUBLIC(this), 0);
}

void
SoShadowGroupP::updateCamera(SoShadowSpotLightCache * cache, const SbMatrix & transform)
{
  SoPerspectiveCamera * cam = cache->camera;
  SoSpotLight * light = cache->light;

  SbVec3f pos = light->location.getValue();
  transform.multVecMatrix(pos, pos);
  cam->position.setValue(pos);

  SbVec3f dir = light->direction.getValue();
  transform.multDirMatrix(dir, dir);
  (void) dir.normalize();

  const float cammul = 2.0f;

  cam->orientation.setValue(SbRotation(SbVec3f(0.0f, 0.0f, -1.0f), dir));
  cam->heightAngle.setValue(light->cutOffAngle.getValue() * cammul);

  SoShadowGroup::VisibilityFlag visflag = (SoShadowGroup::VisibilityFlag) PUBLIC(this)->visibilityFlag.getValue();

  float visnear = PUBLIC(this)->visibilityNearRadius.getValue();
  float visfar = PUBLIC(this)->visibilityRadius.getValue();

  if ((visflag == SoShadowGroup::LONGEST_BBOX_EDGE_FACTOR) ||
      (visflag == SoShadowGroup::PROJECTED_BBOX_DEPTH_FACTOR) ||
      ((visnear < 0.0f) || (visfar < 0.0f))) {

    // FIXME: cache bbox in the pimpl class
    this->bboxaction.apply(cache->depthmap->scene.getValue());
    SbXfBox3f xbox = this->bboxaction.getXfBoundingBox();

    SbMatrix mat;
    mat.setTranslate(- cam->position.getValue());
    xbox.transform(mat);
    mat = cam->orientation.getValue().inverse();
    xbox.transform(mat);
    SbBox3f box = xbox.project();

    // Bounding box was calculated in camera space, so we need to "flip"
    // the box (because camera is pointing in the (0,0,-1) direction
    // from origo.
    cache->nearval = -box.getMax()[2];
    cache->farval = -box.getMin()[2];

    // light won't hit the scene
    // if (cache->farval <= 0.0f) return;

    const int depthbits = 16;
    float r = (float) pow(2.0, (double) depthbits);
    float nearlimit = cache->farval / r;

    if (cache->nearval < nearlimit) {
      cache->nearval = nearlimit;
    }

    const float SLACK = 0.001f;

    cache->nearval = cache->nearval * (1.0f - SLACK);
    cache->farval = cache->farval * (1.0f + SLACK);

    if (visflag == SoShadowGroup::LONGEST_BBOX_EDGE_FACTOR) {
      float sx,sy,sz;
      xbox.getSize(sx, sy, sz);
      float smax =  SbMax(SbMax(sx, sy), sz);
      if (visnear > 0.0f) visnear = smax * visnear;
      if (visfar > 0.0f) visfar = smax  * visfar;
    }
    else if (visflag == SoShadowGroup::PROJECTED_BBOX_DEPTH_FACTOR) {
      if (visnear > 0.0f) visnear = cache->farval * visnear; // should be calculated from farval, not nearval
      if (visfar > 0.0f) visfar = cache->farval * visfar;
    }
  }

  if (visnear > 0.0f) cache->nearval = visnear;
  if (visfar > 0.0f) cache->farval = visfar;

  if (cache->nearval != cam->nearDistance.getValue()) {
    cam->nearDistance = cache->nearval;
  }
  if (cache->farval != cam->farDistance.getValue()) {
    cam->farDistance = cache->farval;
  }
  
  float realfarval = cache->farval / float(cos(light->cutOffAngle.getValue() * cammul));

  cache->fragment_farval->value = realfarval;
  cache->vsm_farval->value = realfarval;

  cache->fragment_nearval->value = cache->nearval;
  cache->vsm_nearval->value = cache->nearval;

  SbViewVolume vv = cam->getViewVolume(1.0f);
  SbMatrix affine, proj;

  vv.getMatrices(affine, proj);
  cache->matrix = affine * proj;

#if 0
  fprintf(stderr,"matrix:\n"
          "%.2f %.2f %.2f %.2f\n"
          "%.2f %.2f %.2f %.2f\n"
          "%.2f %.2f %.2f %.2f\n"
          "%.2f %.2f %.2f %.2f\n",
          cache->matrix[0][0], cache->matrix[0][1], cache->matrix[0][2], cache->matrix[0][3],
          cache->matrix[1][0], cache->matrix[1][1], cache->matrix[1][2], cache->matrix[1][3],
          cache->matrix[2][0], cache->matrix[2][1], cache->matrix[2][2], cache->matrix[2][3],
          cache->matrix[3][0], cache->matrix[3][1], cache->matrix[3][2], cache->matrix[3][3]);
#endif

}


void
SoShadowGroupP::renderDepthMap(SoShadowSpotLightCache * cache,
                               SoGLRenderAction * action)
{
  cache->depthmap->GLRender(action);
  if (cache->gaussmap) cache->gaussmap->GLRender(action);
}

void
SoShadowGroupP::setVertexShader(SoState * state)
{
  int i;
  SoShaderGenerator & gen = this->vertexgenerator;
  gen.reset(FALSE);

  SbBool storedinvalid = SoCacheElement::setInvalid(FALSE);

  state->push();

  SbBool perpixelspot = FALSE;
  SbBool perpixelother = FALSE;

  this->getQuality(state, perpixelspot, perpixelother);

  if (this->vertexshadercache) {
    this->vertexshadercache->unref();
  }
  this->vertexshadercache = new SoShaderProgramCache(state);
  this->vertexshadercache->ref();

  // set active cache to record cache dependencies
  SoCacheElement::set(state, this->vertexshadercache);
  const SoNodeList & lights = SoLightElement::getLights(state);

  int numspots = this->spotlights.getLength();

  for (i = 0; i < numspots; i++) {
    SbString str;
    str.sprintf("varying vec4 shadowCoord%d;", i);
    gen.addDeclaration(str, FALSE);

    if (!perpixelspot) {
      str.sprintf("varying vec3 spotVertexColor%d;", i);
      gen.addDeclaration(str, FALSE);
    }
  }

  if (numspots) {
    gen.addDeclaration("uniform mat4 cameraTransform;", FALSE);
  }
  gen.addDeclaration("varying vec3 ecPosition3;", FALSE);
  gen.addDeclaration("varying vec3 fragmentNormal;", FALSE);
  gen.addDeclaration("varying vec3 perVertexColor;", FALSE);

  SbBool dirlight = FALSE;
  SbBool pointlight = FALSE;
  SbBool spotlight = FALSE;
  SbString str;

  gen.addMainStatement("vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;\n"
                       "ecPosition3 = ecPosition.xyz / ecPosition.w;");

  gen.addMainStatement("vec3 normal = normalize(gl_NormalMatrix * gl_Normal);\n"
                       "vec3 eye = -normalize(ecPosition3);\n"
                       "vec4 ambient = vec4(0.0);\n"
                       "vec4 diffuse = vec4(0.0);\n"
                       "vec4 specular = vec4(0.0);");

  gen.addMainStatement("fragmentNormal = normal;");

  if (!perpixelother) {
    for (i = 0; i < lights.getLength(); i++) {
      SoLight * l = (SoLight*) lights[i];
      if (l->isOfType(SoDirectionalLight::getClassTypeId())) {
        str.sprintf("DirectionalLight(%d, normal, ambient, diffuse, specular);", i);
        gen.addMainStatement(str);
        dirlight = TRUE;
      }
      else if (l->isOfType(SoSpotLight::getClassTypeId())) {
        str.sprintf("SpotLight(%d, eye, ecPosition3, normal, ambient, diffuse, specular);", i);
        gen.addMainStatement(str);
        spotlight = TRUE;

      }
      else if (l->isOfType(SoPointLight::getClassTypeId())) {
        str.sprintf("PointLight(%d, eye, ecPosition3, normal, ambient, diffuse, specular);", i);
        gen.addMainStatement(str);
        pointlight = TRUE;
      }
      else {
        SoDebugError::postWarning("SoShadowGroupP::setVertexShader",
                                  "Unknown light type: %s\n",
                                  l->getTypeId().getName().getString());
      }
    }

    if (dirlight) gen.addNamedFunction(SbName("lights/DirectionalLight"), FALSE);
    if (pointlight) gen.addNamedFunction(SbName("lights/PointLight"), FALSE);

    gen.addMainStatement("vec4 color = gl_FrontLightModelProduct.sceneColor + "
                         "  ambient * gl_FrontMaterial.ambient + "
                         "  diffuse * gl_Color +"
                         "  specular * gl_FrontMaterial.specular;\n"
                         );
  }
  else {
    gen.addMainStatement("vec4 color = gl_FrontLightModelProduct.sceneColor;\n");
  }

  if (numspots) {
    gen.addMainStatement("vec4 pos = cameraTransform * ecPosition;\n"); // in world space
  }
  for (i = 0; i < numspots; i++) {
    spotlight = TRUE;
    SoShadowSpotLightCache * cache = this->spotlights[i];
    SbString str;
    str.sprintf("shadowCoord%d = gl_TextureMatrix[%d] * pos;\n", i, cache->texunit); // in light space
    gen.addMainStatement(str);

    if (!perpixelspot) {
      gen.addMainStatement("ambient = vec4(0.0); diffuse = vec4(0.0); specular = vec4(0.0);\n");
      str.sprintf("SpotLight(%d, eye, ecPosition3, normal, ambient, diffuse, specular);", lights.getLength()+i);
      gen.addMainStatement(str);
      str.sprintf("spotVertexColor%d = \n"
                  "  ambient.rgb * gl_FrontMaterial.ambient.rgb + "
                  "  diffuse.rgb * gl_Color.rgb + "
                  "  specular.rgb * gl_FrontMaterial.specular.rgb;\n", i);
      gen.addMainStatement(str);
    }
  }

  if (spotlight) gen.addNamedFunction(SbName("lights/SpotLight"), FALSE);

  int32_t fogType = this->getFog(state);

  switch (fogType) {
  default:
    assert(0 && "unknown fog type");
  case SoEnvironmentElement::NONE:
    // do nothing
    break;
  case SoEnvironmentElement::HAZE:
  case SoEnvironmentElement::FOG:
  case SoEnvironmentElement::SMOKE:
    gen.addMainStatement("gl_FogFragCoord = abs(ecPosition3.z);\n");
    break;
  }
  gen.addMainStatement(
		       "perVertexColor = vec3(clamp(color.r, 0.0, 1.0), clamp(color.g, 0.0, 1.0), clamp(color.b, 0.0, 1.0));"
		       "gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n"
		       "gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;\n"
		       "gl_Position = ftransform();\n"
		       "gl_FrontColor = gl_Color;");


  // never update unless the program has actually changed. Creating a
  // new GLSL program is very slow on current drivers.
  if (this->vertexshader->sourceProgram.getValue() != gen.getShaderProgram()) {
    this->vertexshader->sourceProgram = gen.getShaderProgram();
    this->vertexshader->sourceType = SoShaderObject::GLSL_PROGRAM;
    this->vertexshadercache->set(gen.getShaderProgram());

    if (numspots) {
      this->vertexshader->parameter.set1Value(0, this->cameratransform);
    }
    else {
      this->vertexshader->parameter.setNum(0);
    }
#if 0 // for debugging
    fprintf(stderr,"new vertex program: %s\n",
            gen.getShaderProgram().getString());
#endif
  }


  this->vertexshadercache->set(gen.getShaderProgram());

  state->pop();
  SoCacheElement::setInvalid(storedinvalid);

}

void
SoShadowGroupP::setFragmentShader(SoState * state)
{
  int i;

  SoShaderGenerator & gen = this->fragmentgenerator;
  gen.reset(FALSE);

  SbBool perpixelspot = FALSE;
  SbBool perpixelother = FALSE;
  this->getQuality(state, perpixelspot, perpixelother);

  SbBool storedinvalid = SoCacheElement::setInvalid(FALSE);
  state->push();

  if (this->fragmentshadercache) {
    this->fragmentshadercache->unref();
  }
  this->fragmentshadercache = new SoShaderProgramCache(state);
  this->fragmentshadercache->ref();

  // set active cache to record cache dependencies
  SoCacheElement::set(state, this->fragmentshadercache);

  int numspots = this->spotlights.getLength();
  SbBool dirspot = FALSE;

  if (numspots) {
    SbString eps;
    eps.sprintf("const float EPSILON = %f;",
                PUBLIC(this)->epsilon.getValue());
    gen.addDeclaration(eps, FALSE);
    eps.sprintf("const float THRESHOLD = %f;",
                PUBLIC(this)->threshold.getValue());
    gen.addDeclaration(eps, FALSE);
  }
  for (i = 0; i < numspots; i++) {
    SbString str;
    str.sprintf("uniform sampler2D shadowMap%d;", i);
    gen.addDeclaration(str, FALSE);

    str.sprintf("uniform float farval%d;", i);
    gen.addDeclaration(str, FALSE);

    str.sprintf("uniform float nearval%d;", i);
    gen.addDeclaration(str, FALSE);

    str.sprintf("varying vec4 shadowCoord%d;", i);
    gen.addDeclaration(str, FALSE);

    if (!perpixelspot) {
      str.sprintf("varying vec3 spotVertexColor%d;", i);
      gen.addDeclaration(str, FALSE);
    }
  }

  SbString str;
  if (numspots) {
#ifdef DISTRIBUTE_FACTOR
    str.sprintf("const float DISTRIBUTE_FACTOR = %.1f;\n", DISTRIBUTE_FACTOR);
    gen.addDeclaration(str, FALSE);
#endif
  }
  gen.addDeclaration("varying vec3 ecPosition3;", FALSE);
  gen.addDeclaration("varying vec3 fragmentNormal;", FALSE);
  gen.addDeclaration("varying vec3 perVertexColor;", FALSE);

  if (perpixelspot && numspots) {
    gen.addNamedFunction(SbName("lights/SpotLight"), FALSE);
  }

  const SoNodeList & lights = SoLightElement::getLights(state);

  if (numspots) {
    gen.addNamedFunction("vsm/VsmLookup", FALSE);
  }
  gen.addMainStatement("vec3 eye = -normalize(ecPosition3);\n");
  gen.addMainStatement("vec4 ambient = vec4(0.0);\n"
                       "vec4 diffuse = vec4(0.0);\n"
                       "vec4 specular = vec4(0.0);"
                       "vec4 mydiffuse = gl_Color;\n"
                       "vec4 texcolor = (coin_texunit0_model != 0) ? texture2D(textureMap0, gl_TexCoord[0].xy) : vec4(1.0);\n");

  if (this->numtexunitsinscene > 1) {
    gen.addMainStatement("if (coin_texunit1_model != 0) texcolor *= texture2D(textureMap1, gl_TexCoord[1].xy);\n");
  }
  gen.addMainStatement("vec3 color = perVertexColor;\n"
                       "vec3 scolor = vec3(0.0);\n"
		       "float dist;\n"
		       "float shadeFactor;\n"
		       "vec3 coord;\n"
		       "vec4 map;\n"
                       "mydiffuse.a *= texcolor.a;\n");
  
  if (perpixelspot) {
    for (i = 0; i < numspots; i++) {
      SbString str;
      SbString spotname("SpotLight");
      SbString insidetest(")");

      if (this->spotlights[i]->light->dropOffRate.getValue() < 0.0f) {
        dirspot = TRUE;
        spotname = "DirSpotLight";
        insidetest = "&& coord.x >= 0.0 && coord.x <= 1.0 && coord.y >= 0.0 && coord.y <= 1.0)";
    }
      str.sprintf("diffuse = vec4(0.0); specular = vec4(0);"
                  "dist = %s(%d, eye, ecPosition3, normalize(fragmentNormal), ambient, diffuse, specular);\n"
                  "coord = 0.5 * (shadowCoord%d.xyz / shadowCoord%d.w + vec3(1.0));\n"
                  "map = texture2D(shadowMap%d, coord.xy);\n"
#ifdef USE_NEGATIVE
                  "map = (map + vec4(1.0)) * 0.5;\n"
#endif // USE_NEGATIVE
#ifdef DISTRIBUTE_FACTOR
                  "map.xy += map.zw / DISTRIBUTE_FACTOR;\n"
#endif
                  "shadeFactor = (shadowCoord%d.z > -1.0%s ? VsmLookup(map, (dist - nearval%d) / (farval%d - nearval%d), EPSILON, THRESHOLD) : 1.0;\n"
                  "color += shadeFactor * diffuse.rgb * mydiffuse.rgb;"
                  "scolor += shadeFactor * gl_FrontMaterial.specular.rgb * specular.rgb;\n",
                  spotname.getString(), lights.getLength()+i, i , i, i, i, insidetest.getString(), i, i, i);
      gen.addMainStatement(str);
      
    }
    gen.addMainStatement("color += ambient.rgb * gl_FrontMaterial.ambient.rgb;\n");

    if (perpixelother) {
      gen.addMainStatement("diffuse = vec4(0.0); ambient = vec4(0.0); specular = vec4(0.0);");

      SbBool dirlight = FALSE;
      SbBool pointlight = FALSE;

      for (i = 0; i < lights.getLength(); i++) {
        SoLight * l = (SoLight*) lights[i];
        if (l->isOfType(SoDirectionalLight::getClassTypeId())) {
          str.sprintf("DirectionalLight(%d, normalize(fragmentNormal), ambient, diffuse, specular);", i);
          gen.addMainStatement(str);
          dirlight = TRUE;
        }
        else if (l->isOfType(SoSpotLight::getClassTypeId())) {
          str.sprintf("SpotLight(%d, eye, ecPosition3, normalize(fragmentNormal), ambient, diffuse, specular);", i);
          gen.addMainStatement(str);
        }
        else if (l->isOfType(SoPointLight::getClassTypeId())) {
          str.sprintf("PointLight(%d, eye, ecPosition3, normalize(fragmentNormal), ambient, diffuse, specular);", i);
          gen.addMainStatement(str);
          pointlight = TRUE;
        }
        else {
          SoDebugError::postWarning("SoShadowGroupP::setFragmentShader",
                                    "Unknown light type: %s\n",
                                    l->getTypeId().getName().getString());
        }
      }

      if (dirlight) gen.addNamedFunction(SbName("lights/DirectionalLight"), FALSE);
      if (pointlight) gen.addNamedFunction(SbName("lights/PointLight"), FALSE);

      gen.addMainStatement("color += ambient.rgb * gl_FrontMaterial.ambient.rgb + "
                           "diffuse.rgb * mydiffuse.rgb;\n");
      gen.addMainStatement("scolor += specular.rgb * gl_FrontMaterial.specular.rgb;\n");
    }
  }

  else {
    for (i = 0; i < numspots; i++) {
      SbString insidetest(")");
      if (this->spotlights[i]->light->dropOffRate.getValue() < 0.0f) {
        insidetest = "&& coord.x >= 0.0 && coord.x <= 1.0 && coord.y >= 0.0 && coord.y <= 1.0)";
      }
      SbString str;
      str.sprintf("dist = length(vec3(gl_LightSource[%d].position) - ecPosition3);\n"
                  "coord = 0.5 * (shadowCoord%d.xyz / shadowCoord%d.w + vec3(1.0));\n"
                  "map = texture2D(shadowMap%d, coord.xy);\n"
#ifdef USE_NEGATIVE
                  "map = (map + vec4(1.0)) * 0.5;\n"
#endif // USE_NEGATIVE
#ifdef DISTRIBUTE_FACTOR
                  "map.xy += map.zw / DISTRIBUTE_FACTOR;\n"
#endif
                  "shadeFactor = (shadowCoord%d.z > -1.0%s ? VsmLookup(map, (dist - nearval%d)/(farval%d-nearval%d), EPSILON, THRESHOLD) : 1.0;\n"
                  "color += shadeFactor * spotVertexColor%d;\n",
                  lights.getLength()+i, i , i, i, i,insidetest.getString(), i,i,i,i);
      gen.addMainStatement(str);
    }
  }
  
  gen.addMainStatement("color = vec3(clamp(color.r, 0.0, 1.0), clamp(color.g, 0.0, 1.0), clamp(color.b, 0.0, 1.0));");
  gen.addMainStatement("if (coin_light_model != 0) { color *= texcolor.rgb; color += scolor; }\n"
                       "else color = mydiffuse.rgb * texcolor.rgb;\n");
  
  int32_t fogType = this->getFog(state);

  switch (fogType) {
  default:
    assert(0 && "unknown fog type");
  case SoEnvironmentElement::NONE:
    // do nothing
    break;
  case SoEnvironmentElement::HAZE:
    gen.addMainStatement("float fog = (gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale;\n");
    break;
  case SoEnvironmentElement::FOG:
    gen.addMainStatement("float fog = exp(-gl_Fog.density * gl_FogFragCoord);\n");
    gen.setVersion("#version 110");
    break;
  case SoEnvironmentElement::SMOKE:
    gen.addMainStatement("float fog = exp(-gl_Fog.density * gl_Fog.density * gl_FogFragCoord * gl_FogFragCoord);\n");
    gen.setVersion("#version 110");
    break;
  }
  if (fogType != SoEnvironmentElement::NONE) {
    gen.addMainStatement("color = mix(gl_Fog.color.rgb, color, clamp(fog, 0.0, 1.0));\n");
  }
  
  gen.addMainStatement("gl_FragColor = vec4(color, mydiffuse.a);");
  gen.addDeclaration("uniform sampler2D textureMap0;\n", FALSE);
  gen.addDeclaration("uniform int coin_texunit0_model;\n", FALSE);
  if (this->numtexunitsinscene > 1) {
    gen.addDeclaration("uniform int coin_texunit1_model;\n", FALSE);
    gen.addDeclaration("uniform sampler2D textureMap1;\n", FALSE);
  }
  gen.addDeclaration("uniform int coin_light_model;\n", FALSE);

  if (dirspot) {
    gen.addNamedFunction("lights/DirSpotLight", FALSE);
  }

  // never update unless the program has actually changed. Creating a
  // new GLSL program is very slow on current drivers.
  this->fragmentshader->parameter.setNum(numspots*3);

  if (this->fragmentshader->sourceProgram.getValue() != gen.getShaderProgram()) {
    // invalidate spotlights, and make sure the cameratransform variable is updated
    this->spotlightsvalid = TRUE;
    this->cameratransform->value.touch();
    this->fragmentshader->sourceProgram = gen.getShaderProgram();
    this->fragmentshader->sourceType = SoShaderObject::GLSL_PROGRAM;

    for (int i = 0; i < numspots; i++) {
      SoShadowSpotLightCache * cache = this->spotlights[i];
      SoShaderParameter1i * shadowmap =
        new SoShaderParameter1i();
      SbString str;
      str.sprintf("shadowMap%d", i);
      shadowmap->name = str;
      shadowmap->value = cache->texunit;
      this->fragmentshader->parameter.set1Value(i*3, shadowmap);
    }
#if 0 // for debugging
    fprintf(stderr,"new fragment program: %s\n",
            gen.getShaderProgram().getString());
#endif // debugging

  }

  for (i = 0; i < numspots; i++) {
    SbString str;
    SoShaderParameter1f *farval = this->spotlights[i]->fragment_farval;
    str.sprintf("farval%d", i);
    if (farval->name.getValue() != str) {
      farval->name = str;
    }
    this->fragmentshader->parameter.set1Value(i*3+1, farval);
  }

  for (i = 0; i < numspots; i++) {
    SbString str;
    SoShaderParameter1f *nearval = this->spotlights[i]->fragment_nearval;
    str.sprintf("nearval%d", i);
    if (nearval->name.getValue() != str) {
      nearval->name = str;
    }
    this->fragmentshader->parameter.set1Value(i*3+2, nearval);
  }


  SoShaderParameter1i * texmap =
    new SoShaderParameter1i();
  str.sprintf("textureMap0");
  texmap->name = str;
  texmap->value = 0;

  SoShaderParameter1i * texmap1 = NULL;
  
  if (!this->texunit0) {
    this->texunit0 = new SoShaderParameter1i;
    this->texunit0->ref();
    this->texunit0->name = "coin_texunit0_model";
    this->texunit0->value = 0;
  }
  
  if (this->numtexunitsinscene > 1) {
    if (!this->texunit1) {
      this->texunit1 = new SoShaderParameter1i;
      this->texunit1->ref();
      this->texunit1->name = "coin_texunit1_model";
      this->texunit1->value = 0;
    }
    texmap1 = new SoShaderParameter1i();
    str.sprintf("textureMap1");
    texmap1->name = str;
    texmap1->value = 1;
  }
                       
  if (!this->lightmodel) {
    this->lightmodel = new SoShaderParameter1i;
    this->lightmodel->ref();
    this->lightmodel->name = "coin_light_model";
    this->lightmodel->value = 1;
  }
  
  this->fragmentshader->parameter.set1Value(this->fragmentshader->parameter.getNum(), texmap);
  if (texmap1) this->fragmentshader->parameter.set1Value(this->fragmentshader->parameter.getNum(), texmap1);
  this->fragmentshader->parameter.set1Value(this->fragmentshader->parameter.getNum(), this->texunit0);
  if (this->numtexunitsinscene > 1) this->fragmentshader->parameter.set1Value(this->fragmentshader->parameter.getNum(), this->texunit1);
  this->fragmentshader->parameter.set1Value(this->fragmentshader->parameter.getNum(), this->lightmodel);
  
  this->fragmentshadercache->set(gen.getShaderProgram());
  state->pop();
  SoCacheElement::setInvalid(storedinvalid);
}

void
SoShadowSpotLightCache::createVSMProgram(void)
{
  SoShaderProgram * program = new SoShaderProgram;
  program->ref();

  SoVertexShader * vshader = new SoVertexShader;
  SoFragmentShader * fshader = new SoFragmentShader;

  program->shaderObject.set1Value(0, vshader);
  program->shaderObject.set1Value(1, fshader);

  SoShaderGenerator & vgen = this->vsm_vertex_generator;
  SoShaderGenerator & fgen = this->vsm_fragment_generator;

  vgen.reset(FALSE);

  vgen.addDeclaration("varying vec3 light_vec;", FALSE);
  vgen.addMainStatement("light_vec = (gl_ModelViewMatrix * gl_Vertex).xyz;\n"
                        "gl_Position = ftransform();");

  vshader->sourceProgram = vgen.getShaderProgram();
  vshader->sourceType = SoShaderObject::GLSL_PROGRAM;

  fgen.reset(FALSE);
#ifdef DISTRIBUTE_FACTOR
  SbString str;
  str.sprintf("const float DISTRIBUTE_FACTOR = %.1f;\n", DISTRIBUTE_FACTOR);
  fgen.addDeclaration(str, FALSE);
#endif
  fgen.addDeclaration("varying vec3 light_vec;", FALSE);
  fgen.addDeclaration("uniform float farval;", FALSE);
  fgen.addDeclaration("uniform float nearval;", FALSE);
  fgen.addMainStatement("float l = (length(light_vec) - nearval) / (farval-nearval);\n"
#ifdef DISTRIBUTE_FACTOR
                        "vec2 m = vec2(l, l*l);\n"
                        "vec2 f = fract(m * DISTRIBUTE_FACTOR);\n"

#ifdef USE_NEGATIVE
                        "gl_FragColor.rg = (m - (f / DISTRIBUTE_FACTOR)) * 2.0 - vec2(1.0, 1.0);\n"
                        "gl_FragColor.ba = f * 2.0 - vec2(1.0, 1.0);\n"
#else // USE_NEGATIVE
                        "gl_FragColor.rg = m - (f / DISTRIBUTE_FACTOR);\n"
                        "gl_FragColor.ba = f;\n"
#endif // ! USE_NEGATIVE
#else // DISTRIBUTE_FACTOR
#ifdef USE_NEGATIVE
                        "gl_FragColor = vec4(l*2.0 - 1.0, l*l*2.0 - 1.0, 0.0, 0.0);"
#else // USE_NEGATIVE
                        "gl_FragColor = vec4(l, l*l, 0.0, 0.0);"
#endif // !USE_NEGATIVE
#endif // !DISTRIBUTE_FACTOR
                        );
  fshader->sourceProgram = fgen.getShaderProgram();
  fshader->sourceType = SoShaderObject::GLSL_PROGRAM;

  this->vsm_program = program;
  this->vsm_program->ref();

  this->vsm_farval = new SoShaderParameter1f;
  this->vsm_farval->ref();
  this->vsm_farval->name = "farval";

  this->vsm_nearval = new SoShaderParameter1f;
  this->vsm_nearval->ref();
  this->vsm_nearval->name = "nearval";

  fshader->parameter = this->vsm_farval;
  fshader->parameter.set1Value(1, this->vsm_nearval);
}

void
SoShadowGroupP::shader_enable_cb(void * closure,
                                 SoState * state,
                                 const SbBool enable)
{
  SoShadowGroupP * thisp = (SoShadowGroupP*) closure;
  
  const cc_glglue * glue = cc_glglue_instance(SoGLCacheContextElement::get(state));

  for (int i = 0; i < thisp->spotlights.getLength(); i++) {
    SoShadowSpotLightCache * cache = thisp->spotlights[i];
    int unit = cache->texunit;
    if (unit == 0) {
      if (enable) glEnable(GL_TEXTURE_2D);
      else glDisable(GL_TEXTURE_2D);
    }
    else {
      cc_glglue_glActiveTexture(glue, (GLenum) (int(GL_TEXTURE0) + unit));
      if (enable) glEnable(GL_TEXTURE_2D);
      else glDisable(GL_TEXTURE_2D);
      cc_glglue_glActiveTexture(glue, GL_TEXTURE0);      
    }
  }
}

void
SoShadowGroupP::GLRender(SoGLRenderAction * action, const SbBool inpath)
{
  SoState * state = action->getState();
  const cc_glglue * glue = cc_glglue_instance(SoGLCacheContextElement::get(state));

  SbBool supported =
    cc_glglue_has_framebuffer_objects(glue) &&
    cc_glglue_glversion_matches_at_least(glue, 2, 0, 0) &&
    cc_glglue_glext_supported(glue, "GL_ARB_texture_float");

  if (!supported || !PUBLIC(this)->isActive.getValue()) {
    if (!supported && PUBLIC(this)->isActive.getValue()) {
      static int first = 1;
      if (first) {
        first = 0;
        SbString msg("Unable to render shadows.");
        if (!cc_glglue_has_framebuffer_objects(glue)) msg += " Frame buffer objects not supported.";
        if (!cc_glglue_glversion_matches_at_least(glue, 2, 0, 0)) msg += " OpenGL version < 2.0."; //G.Barrand : msg +=
        if (!cc_glglue_glext_supported(glue, "GL_ARB_texture_float")) msg += " Floating point textures not supported.";
        SoDebugError::postWarning("SoShadowGroupP::GLRender",
                                  msg.getString());
      }
    }
    if (inpath) PUBLIC(this)->SoSeparator::GLRenderInPath(action);
    else PUBLIC(this)->SoSeparator::GLRenderBelowPath(action);
    return;
  }

  state->push();

  if (!this->vertexshadercache || !this->vertexshadercache->isValid(state)) {
    // a bit hackish, but saves creating yet another cache
    this->spotlightsvalid = FALSE;
  }

  SbMatrix camtransform = SoViewingMatrixElement::get(state).inverse();
  if (camtransform != this->cameratransform->value.getValue()) {
    this->cameratransform->value = camtransform;
  }

  SoShadowStyleElement::set(state, PUBLIC(this), SoShadowStyleElement::CASTS_SHADOW_AND_SHADOWED);
  SoShapeStyleElement::setShadowMapRendering(state, TRUE);
  this->updateSpotLights(action);
  SoShapeStyleElement::setShadowMapRendering(state, FALSE);

  if (!this->vertexshadercache || !this->vertexshadercache->isValid(state)) {
    this->setVertexShader(state);
  }
  
  if (!this->fragmentshadercache || !this->fragmentshadercache->isValid(state)) {
    this->setFragmentShader(state);
  }
  this->shaderprogram->GLRender(action);
  
  SoShapeStyleElement::setShadowsRendering(state, TRUE);
  if (inpath) PUBLIC(this)->SoSeparator::GLRenderInPath(action);
  else PUBLIC(this)->SoSeparator::GLRenderBelowPath(action);
  SoShapeStyleElement::setShadowsRendering(state, FALSE);
  state->pop();
}

SoShaderProgram *
SoShadowSpotLightCache::createGaussFilter(const int texsize, const int size, const float gaussstandarddeviation)
{
  SoVertexShader * vshader = new SoVertexShader;
  SoFragmentShader * fshader = new SoFragmentShader;
  SoShaderProgram * program = new SoShaderProgram;

  SoShaderParameterArray2f * offset = new SoShaderParameterArray2f;
  offset->name = "offset";
  SoShaderParameterArray1f * kernel = new SoShaderParameterArray1f;
  kernel->name = "kernelvalue";
  SoShaderParameter1i * baseimage = new SoShaderParameter1i;
  baseimage->name = "baseimage";
  baseimage->value = 0;

  int kernelsize = size*size;

  offset->value.setNum(kernelsize);
  kernel->value.setNum(kernelsize);

  SoShaderGenerator fgen;
  SbString str;

  str.sprintf("const int KernelSize = %d;", kernelsize);
  fgen.addDeclaration(str, FALSE);
  fgen.addDeclaration("uniform vec2 offset[KernelSize];", FALSE);
  fgen.addDeclaration("uniform float kernelvalue[KernelSize];", FALSE);
  fgen.addDeclaration("uniform sampler2D baseimage;", FALSE);

  fgen.addMainStatement(
                        "int i;\n"
                        "vec4 sum = vec4(0.0);\n"
                        "for (i = 0; i < KernelSize; i++) {\n"
                        "  vec4 tmp = texture2D(baseimage, gl_TexCoord[0].st + offset[i]);\n"
                        "  sum += tmp * kernelvalue[i];\n"
                        "}\n"
                        "gl_FragColor = sum;\n"
                        );

  const double sigma = (double) gaussstandarddeviation;
  const int center = size / 2;
  const float dt = 1.0f / float(texsize);

  SbVec2f * offsetptr = offset->value.startEditing();
  float * kernelptr = kernel->value.startEditing();

  int c = 0;
  for (int y = 0; y < size; y++) {
    int dy = SbAbs(y - center);
    for (int x = 0; x < size; x++) {
      int dx = SbAbs(x - center);

      kernelptr[c] = (float) ((1.0 /  (2.0 * M_PI * sigma * sigma)) * exp(- double(dx*dx + dy*dy) / (2.0 * sigma * sigma)));
      offsetptr[c] = SbVec2f(float(x-center) * dt, float(y-center)*dt);
      c++;

    }
  }
  offset->value.finishEditing();
  kernel->value.finishEditing();

  program->shaderObject = vshader;
  program->shaderObject.set1Value(1, fshader);

  fshader->sourceProgram = fgen.getShaderProgram();
  fshader->sourceType = SoShaderObject::GLSL_PROGRAM;

  fshader->parameter.set1Value(0, offset);
  fshader->parameter.set1Value(1, kernel);
  fshader->parameter.set1Value(2, baseimage);

  SoShaderGenerator vgen;
  vgen.addMainStatement("gl_TexCoord[0] = gl_Vertex;\n");
  vgen.addMainStatement("gl_Position = ftransform();");

  vshader->sourceProgram = vgen.getShaderProgram();
  vshader->sourceType = SoShaderObject::GLSL_PROGRAM;

  return program;
}

SoSeparator *
SoShadowSpotLightCache::createGaussSG(SoShaderProgram * program, SoSceneTexture2 * tex)
{
  SoSeparator * sep = new SoSeparator;
  SoOrthographicCamera * camera = new SoOrthographicCamera;
  SoShapeHints * sh = new SoShapeHints;

  const float verts[][3] = {
    {0.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f}

  };

  sh->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
  sh->faceType = SoShapeHints::CONVEX;
  sh->shapeType = SoShapeHints::SOLID;

  sep->addChild(sh);

  camera->position = SbVec3f(0.5f, 0.5f, 2.0f);
  camera->height = 1.0f;
  camera->aspectRatio = 1.0f;
  camera->viewportMapping = SoCamera::LEAVE_ALONE;

  sep->addChild(camera);
  SoTextureUnit * unit = new SoTextureUnit;
  unit->unit = 0;
  sep->addChild(unit);

  sep->addChild(tex);
  sep->addChild(program);

  SoCoordinate3 * coord = new SoCoordinate3;
  sep->addChild(coord);

  coord->point.setValues(0,4,verts);

  SoFaceSet * fs = new SoFaceSet;
  fs->numVertices = 4;
  sep->addChild(fs);

  return sep;
}

void
SoShadowSpotLightCache::shadowmap_glcallback(void * closure, SoAction * action)
{
  if (action->isOfType(SoGLRenderAction::getClassTypeId())) {
    SoState * state = action->getState();
    SoShadowSpotLightCache * thisp = (SoShadowSpotLightCache*) closure;

    SoLazyElement::setLightModel(state, SoLazyElement::BASE_COLOR);
    SoTextureQualityElement::set(state, 0.0f);
    SoMaterialBindingElement::set(state, NULL, SoMaterialBindingElement::OVERALL);
    SoNormalElement::set(state, NULL, 0, NULL, FALSE);


    SoOverrideElement::setNormalVectorOverride(state, NULL, TRUE);
    SoOverrideElement::setMaterialBindingOverride(state, NULL, TRUE);
    SoOverrideElement::setLightModelOverride(state, NULL, TRUE);
    SoTextureOverrideElement::setQualityOverride(state, TRUE);
  }
}

void
SoShadowSpotLightCache::shadowmap_post_glcallback(void * closure, SoAction * action)
{
  if (action->isOfType(SoGLRenderAction::getClassTypeId())) {
    // nothing to do yet
  }
}

#undef PUBLIC
