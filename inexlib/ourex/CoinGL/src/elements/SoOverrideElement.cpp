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
  \class SoOverrideElement Inventor/elements/SoOverrideElement.h
  \brief The SoOverrideElement maintains a list of overridable elements and a list over which elements should be overridden.
  \ingroup elements

  Only certain elements can be overridden.


  The remaining class documentation describes a single, special case:

  A common request for functionality is to override only the
  transparency of the full scene graph, or parts of the scene
  graph.

  In the original SGI Inventor, this is nigh impossible, as the API
  was designed to only make it possible to override all or none of the
  fields of a node. So calling SoNode::setOverride() on an SoMaterial
  node will cause all material settings of that node to override all
  material settings further on in the scene graph, and there is no way
  to override only the transparency settings.

  In Coin, we have added in a little hack to overcome this problem,
  since it is such a common request for functionality: to have
  separate transparency override settings, set the environment
  variable \c COIN_SEPARATE_DIFFUSE_TRANSPARENCY_OVERRIDE to "1" to
  enable this hack.

  (Do however note that this will not work when the SoPackedColor or
  SoVertexProperty node is used to specify diffuse color and
  transparency -- only with the SoMaterial node.)

  Here is a complete, stand-alone example which demonstrates how to
  accomplish this:

  \code
  #include <Inventor/Qt/SoQt.h>
  #include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
  #include <Inventor/nodes/SoSeparator.h>
  #include <Inventor/nodes/SoMaterial.h>
  
  // *************************************************************************
  
  const char * scene = "#Inventor V2.1 asci\n"
  "\n"
  "Separator {"
  "   Cone { }"
  "   Translation { translation 1 0 5 }"
  ""
  "   DEF OVERRIDEMATERIAL Material { transparency 0.5 }"
  ""
  "   DEF OBJMATERIAL Material {"
  "      diffuseColor 0.5 0 0"
  "      transparency 0"
  "   }"
  "   Sphere { }"
  "}"
  ;
  
  int
  main(int argc, char ** argv)
  {
    QWidget * window = SoQt::init(argv[0]);
  
    (void)coin_setenv("COIN_SEPARATE_DIFFUSE_TRANSPARENCY_OVERRIDE", "1", TRUE);
  
    SoInput * in = new SoInput;
    in->setBuffer((void *)scene, strlen(scene));
    SoSeparator * root = SoDB::readAll(in);
    assert(root);
    delete in;
  
    root->ref();
  
    SoMaterial * overridemat = (SoMaterial *)
      SoBase::getNamedBase("OVERRIDEMATERIAL", SoMaterial::getClassTypeId());
    assert(overridemat);
  
    overridemat->diffuseColor.setIgnored(TRUE);
    overridemat->setOverride(TRUE);
  
    SoQtExaminerViewer * viewer = new SoQtExaminerViewer(window);
    viewer->setSceneGraph(root);
    viewer->show();
  
    SoQt::show(window);
    SoQt::mainLoop();
  
    delete viewer;
    root->unref();
  
    return 0;
  }
  \endcode
*/

// *************************************************************************

#include <Inventor/elements/SoOverrideElement.h>

#include <assert.h>
#include <stdlib.h>

#include <Inventor/C/tidbits.h>

// *************************************************************************

#define SO_GET_OVERRIDE(flag) \
const SoOverrideElement * const element = \
  (const SoOverrideElement *) getConstElement(state, classStackIndex); \
return (element->flags & flag)

#define SO_SET_OVERRIDE(flag) \
SoOverrideElement * const element = \
  (SoOverrideElement *) getElement(state, classStackIndex); \
if (element && override) \
  element->flags |= flag; \
else if (element) \
  element->flags &= ~flag

// *************************************************************************

static int COIN_SEPARATE_DIFFUSE_TRANSPARENCY_OVERRIDE = -1;

// *************************************************************************

static SbBool
use_separate_transp_diffuse(void)
{
  if (COIN_SEPARATE_DIFFUSE_TRANSPARENCY_OVERRIDE < 0) {
    COIN_SEPARATE_DIFFUSE_TRANSPARENCY_OVERRIDE = 0;

    const char * env =
      coin_getenv("COIN_SEPARATE_DIFFUSE_TRANSPARENCY_OVERRIDE");
    if (env) {
      COIN_SEPARATE_DIFFUSE_TRANSPARENCY_OVERRIDE = atoi(env);
    }
  }
  return COIN_SEPARATE_DIFFUSE_TRANSPARENCY_OVERRIDE ?
    TRUE : FALSE;
}

// *************************************************************************

/*!
\enum SoOverrideElement::FlagBits

FIXME: write doc.
*/

SO_ELEMENT_SOURCE(SoOverrideElement);

/*!
This static method initializes static data for the
SoOverrideElement class.
*/

void
SoOverrideElement::initClass(void)
{
  SO_ELEMENT_INIT_CLASS(SoOverrideElement, inherited);
}

/*!
  The destructor.
*/

SoOverrideElement::~SoOverrideElement(void)
{
}

//! FIXME: write doc.

void
SoOverrideElement::init(SoState * state)
{
  inherited::init(state);
  this->flags = 0;
}

//! FIXME: write doc.

void
SoOverrideElement::push(SoState * state)
{
  inherited::push(state);
  SoOverrideElement * prev = (SoOverrideElement*) this->getNextInStack();
  this->flags = prev->flags;
}

//! FIXME: write doc.

SbBool
SoOverrideElement::matches(const SoElement *element) const
{
  return ((SoOverrideElement*)element)->flags == this->flags;
}

//! FIXME: write doc.

SoElement *
SoOverrideElement::copyMatchInfo(void) const
{
  SoOverrideElement *elem = (SoOverrideElement*)
    this->getTypeId().createInstance();
  elem->flags = this->flags;
  return elem;
}

//! FIXME: write doc.

void
SoOverrideElement::print(FILE * /* file */) const
{
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getAmbientColorOverride(SoState * const state)
{
  SO_GET_OVERRIDE(AMBIENT_COLOR);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getColorIndexOverride(SoState * const state)
{
  SO_GET_OVERRIDE(COLOR_INDEX);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getComplexityOverride(SoState * const state)
{
  SO_GET_OVERRIDE(COMPLEXITY);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getComplexityTypeOverride(SoState * const state)
{
  SO_GET_OVERRIDE(COMPLEXITY_TYPE);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getCreaseAngleOverride(SoState * const state)
{
  SO_GET_OVERRIDE(CREASE_ANGLE);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getDiffuseColorOverride(SoState * const state)
{
  SO_GET_OVERRIDE(DIFFUSE_COLOR);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getDrawStyleOverride(SoState * const state)
{
  SO_GET_OVERRIDE(DRAW_STYLE);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getEmissiveColorOverride(SoState * const state)
{
  SO_GET_OVERRIDE(EMISSIVE_COLOR);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getFontNameOverride(SoState * const state)
{
  SO_GET_OVERRIDE(FONT_NAME);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getFontSizeOverride(SoState * const state)
{
  SO_GET_OVERRIDE(FONT_SIZE);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getLightModelOverride(SoState * const state)
{
  SO_GET_OVERRIDE(LIGHT_MODEL);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getLinePatternOverride(SoState * const state)
{
  SO_GET_OVERRIDE(LINE_PATTERN);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getLineWidthOverride(SoState * const state)
{
  SO_GET_OVERRIDE(LINE_WIDTH);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getMaterialBindingOverride(SoState * const state)
{
  SO_GET_OVERRIDE(MATERIAL_BINDING);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getPointSizeOverride(SoState * const state)
{
  SO_GET_OVERRIDE(POINT_SIZE);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getPickStyleOverride(SoState * const state)
{
  SO_GET_OVERRIDE(PICK_STYLE);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getShapeHintsOverride(SoState * const state)
{
  SO_GET_OVERRIDE(SHAPE_HINTS);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getShininessOverride(SoState * const state)
{
  SO_GET_OVERRIDE(SHININESS);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getSpecularColorOverride(SoState * const state)
{
  SO_GET_OVERRIDE(SPECULAR_COLOR);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getTransparencyOverride(SoState * const state)
{
  SO_GET_OVERRIDE(TRANSPARENCY);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getTransparencyTypeOverride(SoState * const state)
{
  SO_GET_OVERRIDE(TRANSPARENCY_TYPE);
}

/*!
  FIXME: write doc.
*/

SbBool
SoOverrideElement::getPolygonOffsetOverride(SoState * const state)
{
  SO_GET_OVERRIDE(POLYGON_OFFSET);
}

/*!
  Returns normal vector override value.

  \CLASS_FUNCTION_EXTENSION

  \since Coin 2.0
*/
SbBool 
SoOverrideElement::getNormalVectorOverride(SoState * const state)
{
  SO_GET_OVERRIDE(NORMAL_VECTOR);
}

/*!
  Returns normal binding override value.

  \CLASS_FUNCTION_EXTENSION

  \since Coin 2.0
*/
SbBool 
SoOverrideElement::getNormalBindingOverride(SoState * const state)
{
  SO_GET_OVERRIDE(NORMAL_BINDING);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setAmbientColorOverride(SoState * const state,
                                           SoNode * const /* node */,
                                           const SbBool override)
{
  SO_SET_OVERRIDE(AMBIENT_COLOR);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setColorIndexOverride(SoState * const state,
                                         SoNode * const /* node */,
                                         const SbBool override)
{
  SO_SET_OVERRIDE(COLOR_INDEX);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setComplexityOverride(SoState * const state,
                                         SoNode * const /* node */,
                                         const SbBool override)
{
  SO_SET_OVERRIDE(COMPLEXITY);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setComplexityTypeOverride(SoState * const state,
                                             SoNode * const /* node */,
                                             const SbBool override)
{
  SO_SET_OVERRIDE(COMPLEXITY_TYPE);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setCreaseAngleOverride(SoState * const state,
                                          SoNode * const /* node */,
                                          const SbBool override)
{
  SO_SET_OVERRIDE(CREASE_ANGLE);
}

/*!
  Can be used to set diffuse color override. This will also set the
  transparency override. Since we feel this is a design flaw,
  it is possible to override this behaviour by setting an environement
  value called COIN_SEPARATE_DIFFUSE_TRANSPARENCY_OVERRIDE to 1.

  Please note that separate override will not work for the PackedColor
  or SoVertexProperty nodes.
*/
void
SoOverrideElement::setDiffuseColorOverride(SoState * const state,
                                           SoNode * const /* node */,
                                           const SbBool override)
{
  SO_SET_OVERRIDE(DIFFUSE_COLOR);
  if (!use_separate_transp_diffuse()) {
    SO_SET_OVERRIDE(TRANSPARENCY);
  }
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setDrawStyleOverride(SoState * const state,
                                        SoNode * const /* node */,
                                        const SbBool override)
{
  SO_SET_OVERRIDE(DRAW_STYLE);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setEmissiveColorOverride(SoState * const state,
                                            SoNode * const /* node */,
                                            const SbBool override)
{
  SO_SET_OVERRIDE(EMISSIVE_COLOR);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setFontNameOverride(SoState * const state,
                                       SoNode * const /* node */,
                                       const SbBool override)
{
  SO_SET_OVERRIDE(FONT_NAME);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setFontSizeOverride(SoState * const state,
                                       SoNode * const /* node */,
                                       const SbBool override)
{
  SO_SET_OVERRIDE(FONT_SIZE);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setLightModelOverride(SoState * const state,
                                         SoNode * const /* node */,
                                         const SbBool override)
{
  SO_SET_OVERRIDE(LIGHT_MODEL);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setLinePatternOverride(SoState * const state,
                                          SoNode * const /* node */,
                                          const SbBool override)
{
  SO_SET_OVERRIDE(LINE_PATTERN);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setLineWidthOverride(SoState * const state,
                                        SoNode * const /* node */,
                                        const SbBool override)
{
  SO_SET_OVERRIDE(LINE_WIDTH);
}

//! FIXME: write doc.

void
SoOverrideElement::setMaterialBindingOverride(SoState * const state,
                                              SoNode * const /* node */,
                                              const SbBool override)
{
  SO_SET_OVERRIDE(MATERIAL_BINDING);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setPickStyleOverride(SoState * const state,
                                        SoNode * const /* node */,
                                        const SbBool override)
{
  SO_SET_OVERRIDE(PICK_STYLE);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setPointSizeOverride(SoState * const state,
                                        SoNode * const /* node */,
                                        const SbBool override)
{
  SO_SET_OVERRIDE(POINT_SIZE);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setPolygonOffsetOverride(SoState * const state,
                                            SoNode * const /* node */,
                                            const SbBool override)
{
  SO_SET_OVERRIDE(POLYGON_OFFSET);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setShapeHintsOverride(SoState * const state,
                                         SoNode * const /* node */,
                                         const SbBool override)
{
  SO_SET_OVERRIDE(SHAPE_HINTS);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setShininessOverride(SoState * const state,
                                        SoNode * const /* node */,
                                        const SbBool override)
{
  SO_SET_OVERRIDE(SHININESS);
}

/*!
  FIXME: write doc.
*/

void
SoOverrideElement::setSpecularColorOverride(SoState * const state,
                                            SoNode * const /* node */,
                                            const SbBool override)
{
  SO_SET_OVERRIDE(SPECULAR_COLOR);
}


/*!
  Can be used to set the transparency override.

  \sa setDiffuseColorOverride().
*/
void
SoOverrideElement::setTransparencyOverride(SoState * const state,
                                           SoNode * const /* node */,
                                           const SbBool override)
{
  SO_SET_OVERRIDE(TRANSPARENCY);
  if (!use_separate_transp_diffuse()) {
    SO_SET_OVERRIDE(DIFFUSE_COLOR);
  }
}

/*!
  Can be used to set the transparency type override.

  \sa setDiffuseColorOverride().
*/
void
SoOverrideElement::setTransparencyTypeOverride(SoState * const state,
                                               SoNode * const /* node */,
                                               const SbBool override)
{
  SO_SET_OVERRIDE(TRANSPARENCY_TYPE);
}

/*!
  Can be used to set normal vector override.

  \CLASS_FUNCTION_EXTENSION

  \since Coin 2.0
*/
void 
SoOverrideElement::setNormalVectorOverride(SoState * const state,
                                           SoNode * const /* node */,
                                           const SbBool override)
{
  SO_SET_OVERRIDE(NORMAL_VECTOR);
}

/*!
  Can be used to set normal binding override.

  \CLASS_FUNCTION_EXTENSION

  \since Coin 2.0
*/
void 
SoOverrideElement::setNormalBindingOverride(SoState * const state,
                                            SoNode * const /* node */,
                                            const SbBool override)
{
  SO_SET_OVERRIDE(NORMAL_BINDING);
}

