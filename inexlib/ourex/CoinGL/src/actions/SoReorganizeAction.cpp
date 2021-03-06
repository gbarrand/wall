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
  \page vbo_rendering Vertex array and VBO rendering in Coin
  
  Coin 2.5 added improved support for OpenGL vertex array and VBO
  rendering.  This might lead to major rendering performance
  improvements compared to the old rendering code. The new rendering
  code has been added for the SoIndexedFaceSet, SoVRMLIndexedFaceSet,
  SoIndexedLineSet, SoVRMLIndexedLineSet, SoPointSet and
  SoVRMLPointSet nodes.

  To take advantage of the improved performance vertex array and VBO
  rendering yields, you'll need to organize your vertex data in a way
  that makes it possible to render it with OpenGL vertex arrays.
  OpenGL vertex array rendering does not support multiple index
  arrays, so all your vertex data (coordinates, normals, colors and
  texture coordinates) must use the same index array; or use OVERALL
  binding. For the indexed nodes, this means that PER_VERTEX_INDEXED
  and OVERALL are the only supported bindings for materials, normals
  and texture coordinates. When PER_VERTEX_INDEXED binding is used,
  the corresponding index field should by empty. This will signal the
  shape to use the coordIndex field for indices. Below is an
  example scene graph that will be rendered using vertex arrays:

  \verbatim

  NormalBinding { value PER_VERTEX_INDEXED }
  Coordinate3 {
    point [
      0 0 0, # 0
      1 0 0, # 1
      2 0 0, # 2 
      0 1 0, # 3
      1 1 0, # 4 
      2 1 0, # 5
      0 2 0, # 6 
      1 2 0, # 7 
      2 2 0, # 8 

      2 0  0, # 9
      2 0 -1, # 10
      2 1  0, # 11
      2 1 -1, # 12
      2 2  0, # 13
      2 2 -1  # 14
    ]
  }
  Normal {
    vector [
      0 0 1, # 0 
      0 0 1, # 1
      0 0 1, # 2
      0 0 1, # 3
      0 0 1, # 4
      0 0 1, # 5
      0 0 1, # 6
      0 0 1, # 7
      0 0 1, # 8

      1 0 0,  # 9
      1 0 0,  # 10
      1 0 0,  # 11
      1 0 0,  # 12
      1 0 0,  # 13
      1 0 0   # 14
    ] 
  }

  IndexedFaceSet {
    coordIndex [
      0, 1, 4, 3, -1,
      1, 2, 5, 4, -1,
      3, 4, 7, 6, -1,
      4, 5, 8, 7, -1,
    
      9, 10, 12, 11, -1,
      11, 12, 14, 13, -1
    ]
    normalIndex [ ] # = use coordIndex
  }
  \endverbatim

  Please note that since only one index array can be used, it might
  be necessary to supply duplicate normals and coordinates to meet this
  requirement.

  Also, if normals are needed, you have to supply them. A shape
  with autogenerated normals can't be rendered using vertex arrays
  (since a single coordinate might get multiple normals).

  The PointSet nodes can always be rendered using vertex arrays since
  these nodes haven't got index arrays, and the only bindings supported
  are PER_VERTEX and OVERALL.

  If it's inconvenient to create vertex array ready scene graphs
  directly from your application, it's also possible to use
  SoReorganizeAction to reorganize the geometry before rendering.
*/

/*!
  \class SoReorganizeAction Inventor/include/SoReorganizeAction.h
  \ingroup actions
  \brief The SoReorganizeAction class reorganizes your scene graph to optimize traversal/rendering.

  Note. This is work-in-progress. pederb, 2005-04-05.

  The code below is an example of a program that applies an
  SoReorganizeAction on a scene graph, converting all shapes into
  shapes that can be rendered using vertex array or VBO rendering.

  \code

  #include <Inventor/SoDB.h>
  #include <Inventor/nodes/SoSeparator.h>
  #include <Inventor/nodes/SoCoordinate3.h>
  #include <Inventor/nodes/SoCoordinate4.h>
  #include <Inventor/nodes/SoNormal.h>
  #include <Inventor/nodes/SoTextureCoordinate2.h>
  #include <Inventor/nodes/SoSeparator.h>
  #include <Inventor/actions/SoWriteAction.h>
  #include <Inventor/actions/SoSearchAction.h>
  #include <Inventor/errors/SoDebugError.h>
  #include <Inventor/nodes/SoShapeHints.h>
  #include <Inventor/SoInput.h>
  #include <Inventor/SoOutput.h>
  #include <Inventor/SoInteraction.h>
  #include <Inventor/actions/SoReorganizeAction.h>
  #include <assert.h>
  #include <stdio.h>
  
  static void strip_node(SoType type, SoNode * root) 
  {
    SoSearchAction sa;
    sa.setType(type);
    sa.setSearchingAll(TRUE);
    sa.setInterest(SoSearchAction::ALL);
    sa.apply(root);

    SoPathList & pl = sa.getPaths();
    for (int i = 0; i < pl.getLength(); i++) {
      SoFullPath * p = (SoFullPath*) pl[i];
        if (p->getTail()->isOfType(type)) {
          SoGroup * g = (SoGroup*) p->getNodeFromTail(1);
          g->removeChild(p->getIndexFromTail(0));
        }
      } 
    sa.reset();  
  }

  int
  main(int argc, char ** argv )
  {
    if (argc < 3) {
      fprintf(stderr,"Usage: reorganize <infile> <outfile> [nostrip]\n");
      return -1;
    }

    SbBool strip = TRUE;
    if (argc > 3) {
      if (strcmp(argv[3], "nostrip") == 0) strip = FALSE;
      else {
        fprintf(stderr,"Usage: reorganize <infile> <outfile> [nostrip]\n");
        return -1;
      }
    }

    SoDB::init();
    SoInteraction::init();
    
    SoInput input;
    SbBool ok = input.openFile(argv[1]);
    if (!ok) {
      fprintf(stderr,"Unable to open file.\n");
      return -1;
    }
    SoSeparator * root = SoDB::readAll(&input); 
  
    SbBool vrml1 = input.isFileVRML1();
    SbBool vrml2 = input.isFileVRML2();

    if (vrml2) {
      fprintf(stderr,"VRML2 not supported yet\n");
      return -1;
    }
  
    if (!root) {
      fprintf(stderr,"Unable to read file.\n");
      return -1;
    }
    root->ref();

    fprintf(stderr,"Applying SoReorganizeAction...");
    SoReorganizeAction reorg;
    reorg.apply(root);
    fprintf(stderr,"done\n");

    SoOutput out;
    if (out.openFile(argv[2])) {      
      if (strip) { // strip coord3, texcoord and normal nodes
        fprintf(stderr,"stripping old nodes from scene graph\n");
        strip_node(SoCoordinate3::getClassTypeId(), root);
        strip_node(SoCoordinate4::getClassTypeId(), root);
        strip_node(SoNormal::getClassTypeId(), root);
        strip_node(SoTextureCoordinate2::getClassTypeId(), root);
      }
      fprintf(stderr,"writing target\n");
      SoWriteAction wa(&out);
      wa.apply(root);
    }
    root->unref();
    return 0;
  } // main()

  \endcode

  \since Coin 2.5

*/

#include <Inventor/SbName.h>
#include <Inventor/actions/SoReorganizeAction.h>

#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodes/SoVertexShape.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoTextureImageElement.h>
#include <Inventor/VRMLnodes/SoVRMLCoordinate.h>
#include <Inventor/VRMLnodes/SoVRMLTextureCoordinate.h>
#include <Inventor/VRMLnodes/SoVRMLNormal.h>
#include <Inventor/VRMLnodes/SoVRMLColor.h>
#include <Inventor/VRMLnodes/SoVRMLShape.h>
#include <Inventor/VRMLnodes/SoVRMLVertexShape.h>
#include <Inventor/VRMLnodes/SoVRMLIndexedFaceSet.h>
#include <Inventor/VRMLnodes/SoVRMLIndexedLineSet.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoNormal.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoTextureCoordinate2.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/caches/SoPrimitiveVertexCache.h>
#include <Inventor/elements/SoMultiTextureEnabledElement.h>
#include <Inventor/elements/SoTextureEnabledElement.h>
#include <Inventor/elements/SoTexture3EnabledElement.h>
#include <Inventor/elements/SoTextureCoordinateElement.h>
#include <Inventor/elements/SoShapeStyleElement.h>
#include <Inventor/elements/SoLightModelElement.h>
#include <Inventor/elements/SoNormalElement.h>
#include <Inventor/elements/SoMultiTextureCoordinateElement.h>
#include <Inventor/SbColor4f.h>
#include <string.h>
#include <assert.h>

#include <coindefs.h> // COIN_STUB()

class SoReorganizeActionP {
 public:
  SoReorganizeActionP(SoReorganizeAction * masterin) 
    : master(masterin),
      gennormals(TRUE),
      gentexcoords(TRUE),
      gentristrips(FALSE),
      genvp(FALSE),
      matchidx(TRUE),
      cbaction(SbViewportRegion(640, 480)),
      pvcache(NULL)
  {
    cbaction.addTriangleCallback(SoVertexShape::getClassTypeId(), triangle_cb, this);
    cbaction.addLineSegmentCallback(SoVertexShape::getClassTypeId(), line_segment_cb, this);

    cbaction.addTriangleCallback(SoVRMLIndexedFaceSet::getClassTypeId(), triangle_cb, this);
    cbaction.addLineSegmentCallback(SoVRMLIndexedLineSet::getClassTypeId(), line_segment_cb, this);
    
    cbaction.addPreCallback(SoVertexShape::getClassTypeId(),
                            pre_shape_cb, this);
    cbaction.addPostCallback(SoVertexShape::getClassTypeId(),
                             post_shape_cb, this);
    
    cbaction.addPreCallback(SoVRMLIndexedFaceSet::getClassTypeId(),
                            pre_shape_cb, this);
    cbaction.addPostCallback(SoVRMLIndexedFaceSet::getClassTypeId(),
                             post_shape_cb, this);
    

    cbaction.addPreCallback(SoVRMLIndexedLineSet::getClassTypeId(),
                            pre_shape_cb, this);
    cbaction.addPostCallback(SoVRMLIndexedLineSet::getClassTypeId(),
                             post_shape_cb, this);
    
  } 
  SoReorganizeAction * master;
  SbBool gennormals;
  SbBool gentexcoords;
  SbBool gentristrips;
  SbBool genvp;
  SbBool matchidx;
  SbList <SbBool> needtexcoords;
  int lastneeded;
  int numtriangles;
  int numlines;
  int numpoints;
  SbBool isvrml;

  SbBool didinit;
  SbBool hastexture;
  SbColor4f diffusecolor;
  SbBool lighting;
  SbBool normalsonstate;

  SoCallbackAction cbaction;
  SoSearchAction sa;
  SoPrimitiveVertexCache * pvcache;

  static SoCallbackAction::Response pre_shape_cb(void * userdata, SoCallbackAction * action, const SoNode * node);
  static SoCallbackAction::Response post_shape_cb(void * userdata, SoCallbackAction * action, const SoNode * node);
  static void triangle_cb(void * userdata, SoCallbackAction * action,
                          const SoPrimitiveVertex * v1,
                          const SoPrimitiveVertex * v2,
                          const SoPrimitiveVertex * v3);

  static void line_segment_cb(void * userdata, SoCallbackAction * action,
                              const SoPrimitiveVertex * v1,
                              const SoPrimitiveVertex * v2);
    
  SbBool initShape(SoCallbackAction * action);
  void replaceNode(SoFullPath * path);
  void replaceIfs(SoFullPath * path);
  void replaceVrmlIfs(SoFullPath * path);
  void replaceIls(SoFullPath * path);
  void replaceVrmlIls(SoFullPath * path);

  SoVertexProperty * createVertexProperty(const SbBool forlines);
};



#define PRIVATE(obj) obj->pimpl

SO_ACTION_SOURCE(SoReorganizeAction);

// Override from parent class.
void
SoReorganizeAction::initClass(void)
{
  SO_ACTION_INTERNAL_INIT_CLASS(SoReorganizeAction, SoSimplifyAction);
}


/*!
  A constructor.
*/

SoReorganizeAction::SoReorganizeAction(SoSimplifier * simplifier)
{
  PRIVATE(this) = new SoReorganizeActionP(this);
  SO_ACTION_CONSTRUCTOR(SoReorganizeAction);
}

/*!
  The destructor.
*/

SoReorganizeAction::~SoReorganizeAction(void)
{
  delete PRIVATE(this);
}

SoSeparator * 
SoReorganizeAction::getSimplifiedSceneGraph(void) const
{
  return NULL;
}

void 
SoReorganizeAction::generateNormals(SbBool onoff)
{
  PRIVATE(this)->gennormals = onoff;
}

SbBool 
SoReorganizeAction::areNormalGenerated(void) const
{
  return PRIVATE(this)->gennormals;
}

void 
SoReorganizeAction::generateTriangleStrips(SbBool onoff)
{
  PRIVATE(this)->gentristrips = onoff;
}

SbBool 
SoReorganizeAction::areTriangleStripGenerated(void) const
{
  return PRIVATE(this)->gentristrips;
}

void 
SoReorganizeAction::generateTexCoords(SbBool onoff)
{
  PRIVATE(this)->gentexcoords = onoff;
}

SbBool 
SoReorganizeAction::areTexCoordsGenerated(void) const
{
  return PRIVATE(this)->gentexcoords;
}

void 
SoReorganizeAction::generateVPNodes(SbBool onoff)
{
  PRIVATE(this)->genvp = onoff;
}

SbBool 
SoReorganizeAction::areVPNodesGenerated(void)
{
  return PRIVATE(this)->genvp;
}

void 
SoReorganizeAction::matchIndexArrays(SbBool onoff)
{
  PRIVATE(this)->matchidx = onoff;
}

SbBool 
SoReorganizeAction::areIndexArraysMatched(void) const
{
  return PRIVATE(this)->matchidx;
}

SoSimplifier * 
SoReorganizeAction::getSimplifier(void) const
{
  return NULL;
}

void 
SoReorganizeAction::apply(SoNode * root)
{
  int i;
  PRIVATE(this)->sa.setType(SoVertexShape::getClassTypeId());
  PRIVATE(this)->sa.setSearchingAll(TRUE);
  PRIVATE(this)->sa.setInterest(SoSearchAction::ALL);
  PRIVATE(this)->sa.apply(root);
  SoPathList & pl = PRIVATE(this)->sa.getPaths();
  for (i = 0; i < pl.getLength(); i++) {
    this->apply(pl[i]);
  }
  PRIVATE(this)->sa.reset();
  
  PRIVATE(this)->sa.setType(SoVRMLIndexedFaceSet::getClassTypeId());
  PRIVATE(this)->sa.setSearchingAll(TRUE);
  PRIVATE(this)->sa.setInterest(SoSearchAction::ALL);
  PRIVATE(this)->sa.apply(root);
  SoPathList & pl2 = PRIVATE(this)->sa.getPaths();
  for (i = 0; i < pl2.getLength(); i++) {
    this->apply(pl2[i]);
  }
  PRIVATE(this)->sa.reset();

  PRIVATE(this)->sa.setType(SoVRMLIndexedLineSet::getClassTypeId());
  PRIVATE(this)->sa.setSearchingAll(TRUE);
  PRIVATE(this)->sa.setInterest(SoSearchAction::ALL);
  PRIVATE(this)->sa.apply(root);
  SoPathList & pl3 = PRIVATE(this)->sa.getPaths();
  for (i = 0; i < pl3.getLength(); i++) {
    this->apply(pl3[i]);
  }
  PRIVATE(this)->sa.reset();
}

void 
SoReorganizeAction::apply(SoPath * path)
{
  PRIVATE(this)->cbaction.apply(path);
  PRIVATE(this)->replaceNode((SoFullPath*) path);
}

void 
SoReorganizeAction::apply(const SoPathList & pathlist, SbBool obeysrules)
{
  for (int i = 0; i < pathlist.getLength(); i++) {
    this->apply(pathlist[i]);
  }
}

void 
SoReorganizeAction::startReport(const char * msg)
{
  COIN_STUB();
}

void 
SoReorganizeAction::finishReport(void)
{
  COIN_STUB();
}

// Documented in superclass.
void
SoReorganizeAction::beginTraversal(SoNode * /* node */)
{
  assert(0 && "should never get here");
}


SoCallbackAction::Response 
SoReorganizeActionP::pre_shape_cb(void * userdata, SoCallbackAction * action, const SoNode * node)
{
  SoReorganizeActionP * thisp = (SoReorganizeActionP*) userdata;
  thisp->didinit = FALSE;
  thisp->isvrml  = node->isOfType(SoVRMLGeometry::getClassTypeId());
  thisp->numtriangles = 0;
  thisp->numpoints = 0;
  thisp->numlines = 0;
  return SoCallbackAction::CONTINUE;
}

SoCallbackAction::Response 
SoReorganizeActionP::post_shape_cb(void * userdata, SoCallbackAction * action, const SoNode * node)
{
  SoReorganizeActionP * thisp = (SoReorganizeActionP*) userdata;

#if 0 // debug
  fprintf(stderr,"shape: %s, numtri: %d, pvcache: %p\n",
          node->getTypeId().getName().getString(),
          thisp->numtriangles,
          thisp->pvcache);
#endif // debug
  return SoCallbackAction::CONTINUE;
}
  
void 
SoReorganizeActionP::triangle_cb(void * userdata, SoCallbackAction * action,
                                 const SoPrimitiveVertex * v1,
                                 const SoPrimitiveVertex * v2,
                                 const SoPrimitiveVertex * v3) 
{
  SoReorganizeActionP * thisp = (SoReorganizeActionP*) userdata;
  
  if (!thisp->didinit) {
    if (thisp->initShape(action)) {
      assert(thisp->pvcache == NULL);
      thisp->pvcache = new SoPrimitiveVertexCache(action->getState());
      thisp->pvcache->ref();
    }
  }

  thisp->numtriangles++;
  if (thisp->pvcache) {
    thisp->pvcache->addTriangle(v1, v2, v3);
  }
}

void 
SoReorganizeActionP::line_segment_cb(void * userdata, SoCallbackAction * action,
                                     const SoPrimitiveVertex * v1,
                                     const SoPrimitiveVertex * v2)
{
  SoReorganizeActionP * thisp = (SoReorganizeActionP*) userdata;
  
  if (!thisp->didinit) {
    if (thisp->initShape(action)) {
      assert(thisp->pvcache == NULL);
      thisp->pvcache = new SoPrimitiveVertexCache(action->getState());
      thisp->pvcache->ref();
    }
  }

  thisp->numlines++;
  if (thisp->pvcache) {
    thisp->pvcache->addLine(v1, v2);
  }
}

SbBool
SoReorganizeActionP::initShape(SoCallbackAction * action)
{
  this->didinit = TRUE;
  SoState * state = action->getState();
  SbBool canrenderasvertexarray = TRUE;
  
  unsigned int shapeflags = SoShapeStyleElement::get(state)->getFlags();
  
  this->lighting = SoLightModelElement::get(state) != SoLightModelElement::BASE_COLOR;
  this->normalsonstate = SoNormalElement::getInstance(state)->getNum() > 0;
  
  SbBool texture0enabled =
    SoTextureEnabledElement::get(state) != FALSE;

  this->hastexture = texture0enabled;

  int lastenabled;
  const SbBool * enabledunits = 
    SoMultiTextureEnabledElement::getEnabledUnits(state, lastenabled);

  this->needtexcoords.truncate(0);
  this->needtexcoords.append(FALSE);

  if (shapeflags & 
      (SoShapeStyleElement::BUMPMAP|
       SoShapeStyleElement::BBOXCMPLX|
       SoShapeStyleElement::INVISIBLE|
       SoShapeStyleElement::BIGIMAGE)) {
    canrenderasvertexarray = FALSE;
  }


  if (canrenderasvertexarray && texture0enabled) {
    const SoTextureCoordinateElement * celem =
      (const SoTextureCoordinateElement *) SoTextureCoordinateElement::getInstance(state);
    switch (celem->getType()) {
    case SoTextureCoordinateElement::DEFAULT:
    case SoTextureCoordinateElement::EXPLICIT:
      this->needtexcoords[0] = TRUE;
      break;
    case SoTextureCoordinateElement::TEXGEN:
      // don't need texcoords for unit0
      break;
    case SoTextureCoordinateElement::FUNCTION:
      this->needtexcoords[0] = TRUE;
    default:
      canrenderasvertexarray = FALSE;
      break;
    }
  }
  
  if (canrenderasvertexarray && enabledunits) {
    const SoMultiTextureCoordinateElement * melem =
      SoMultiTextureCoordinateElement::getInstance(state);
    for (int i = 1; i <= lastenabled; i++) {
      this->needtexcoords.append(FALSE);
      if (enabledunits[i]) {
        // FIXME: multitexturing is not supported yet, since it's not
        // supported by SoVertexProperty. We might fix this later by
        // inserting new nodes though. pederb, 2005-04-28
        canrenderasvertexarray = FALSE; 

        this->hastexture = TRUE;
        switch (melem->getType(i)) {
        case SoTextureCoordinateElement::DEFAULT:
        case SoTextureCoordinateElement::EXPLICIT:
          this->needtexcoords[i] = TRUE;
          break;
        case SoTextureCoordinateElement::TEXGEN:
          // don't need texcoords for unit i
          break;
        case SoTextureCoordinateElement::FUNCTION:
          this->needtexcoords[i] = TRUE;
          break;
        default:
          canrenderasvertexarray = FALSE;
          break;
        }
      }
    }
  }

  if (canrenderasvertexarray) {
    SbColor diffuse = SoLazyElement::getDiffuse(state, 0);
    float transp = SoLazyElement::getTransparency(state, 0); 
    this->diffusecolor = SbColor4f(diffuse, 1.0f - transp);
  }

  return canrenderasvertexarray;
}

void 
SoReorganizeActionP::replaceNode(SoFullPath * path)
{
  if (this->pvcache == NULL) return;  
  this->pvcache->fit(); // needed to do optimize-sort of data
  
  if (this->pvcache->getNumIndices()) {
    if (this->isvrml) {
      this->replaceVrmlIfs(path);
    }
    else {
      this->replaceIfs(path);
    }
  }
  else if (this->pvcache->getNumLineIndices()) {
    if (this->isvrml) {
      this->replaceVrmlIls(path);
    }
    else {
      this->replaceIls(path);
    }
  }
  this->pvcache->unref();
  this->pvcache = NULL;
}

SoVertexProperty * 
SoReorganizeActionP::createVertexProperty(const SbBool forlines)
{
  SoVertexProperty * vp = new SoVertexProperty;
  vp->ref();
  SoVertexProperty::Binding nbind = SoVertexProperty::PER_VERTEX_INDEXED;

  if (!this->lighting || 
      (forlines && !this->normalsonstate)) {
    nbind = SoVertexProperty::OVERALL;
  }                                          
  vp->normalBinding = nbind;

  int numv = this->pvcache->getNumVertices();
  
  if (this->hastexture) {
    vp->texCoord.setNum(numv);
    SbVec2f * dst = vp->texCoord.startEditing();
    const SbVec4f * src = this->pvcache->getTexCoordArray();
    
    for (int i = 0; i < numv; i++) {
      SbVec4f tmp = src[i];
      if (tmp[3] != 0.0f) {
        tmp[0] /= tmp[3];
        tmp[1] /= tmp[3];
      }
      dst[i][0] = tmp[0];
      dst[i][1] = tmp[1];
    }
    vp->texCoord.finishEditing();
  }
  
  vp->vertex.setValues(0, numv,
                       this->pvcache->getVertexArray());
  if (nbind == SoVertexProperty::PER_VERTEX_INDEXED) {
    vp->normal.setValues(0, numv,
                         this->pvcache->getNormalArray());
  }  
  
  vp->materialBinding = SoVertexProperty::OVERALL;  
  vp->orderedRGBA = this->diffusecolor.getPackedValue();
  
  if (this->pvcache->colorPerVertex()) {
    vp->materialBinding = SoVertexProperty::PER_VERTEX_INDEXED;
    uint8_t * src = (uint8_t*) this->pvcache->getColorArray();
    vp->orderedRGBA.setNum(numv);
    uint32_t * dst = vp->orderedRGBA.startEditing();
    for (int i = 0; i < numv; i++) {
      dst[i] = (src[0]<<24)|(src[1]<<16)|(src[2]<<8)|src[3];
      src += 4;
    }
    vp->orderedRGBA.finishEditing();
  }
  vp->unrefNoDelete();
  return vp;
}

void 
SoReorganizeActionP::replaceIfs(SoFullPath * path)
{
  SoNode * parent = path->getNodeFromTail(1);
  if (!parent->isOfType(SoGroup::getClassTypeId())) {
    return;
  }

  SoVertexProperty * vp = this->createVertexProperty(FALSE);
  SoIndexedFaceSet * ifs = new SoIndexedFaceSet;
  ifs->ref();
  ifs->vertexProperty = vp;
  ifs->normalIndex.setNum(0);
  ifs->materialIndex.setNum(0);
  ifs->textureCoordIndex.setNum(0);
  
  int numtri = this->pvcache->getNumIndices() / 3;
  const GLint * indices = this->pvcache->getIndices();
  ifs->coordIndex.setNum(numtri * 4);
  int32_t * ptr = ifs->coordIndex.startEditing();

  
  for (int i = 0; i < numtri; i++) {
    *ptr++ = (int32_t) indices[i*3];
    *ptr++ = (int32_t) indices[i*3+1];
    *ptr++ = (int32_t) indices[i*3+2];
    *ptr++ = -1;
  }
  ifs->coordIndex.finishEditing();

  int idx = path->getIndexFromTail(0);
  path->pop();
  SoGroup * g = (SoGroup*)parent;
  g->replaceChild(idx, ifs);
  path->push(idx);
  ifs->unrefNoDelete();
}

void 
SoReorganizeActionP::replaceVrmlIfs(SoFullPath * path)
{
  SoNode * parent = path->getNodeFromTail(1);
  if (!parent->isOfType(SoGroup::getClassTypeId()) &&
      !parent->isOfType(SoVRMLShape::getClassTypeId())) {
    return;
  }

  SoVRMLIndexedFaceSet * oldifs = (SoVRMLIndexedFaceSet*) path->getTail();
  assert(oldifs->isOfType(SoVRMLIndexedFaceSet::getClassTypeId()));
  SoVRMLIndexedFaceSet * ifs = new SoVRMLIndexedFaceSet;
  ifs->ref();
  ifs->normalPerVertex = this->lighting;
  ifs->colorPerVertex = this->pvcache->colorPerVertex();
  ifs->ccw = oldifs->ccw;
  ifs->solid = oldifs->solid;
  ifs->creaseAngle = oldifs->creaseAngle;
  
  int numv = this->pvcache->getNumVertices();
  
  if (this->hastexture) {
    SoVRMLTextureCoordinate * tc = new SoVRMLTextureCoordinate;
    tc->point.setNum(numv);
    SbVec2f * dst = tc->point.startEditing();
    const SbVec4f * src = this->pvcache->getTexCoordArray();
    
    for (int i = 0; i < numv; i++) {
      SbVec4f tmp = src[i];
      if (tmp[3] != 0.0f) {
        tmp[0] /= tmp[3];
        tmp[1] /= tmp[3];
      }
      dst[i][0] = tmp[0];
      dst[i][1] = tmp[1];
    }
    tc->point.finishEditing();
    ifs->texCoord = tc;
  }

  SoVRMLCoordinate * c = new SoVRMLCoordinate;
  c->point.setValues(0, numv,
                     this->pvcache->getVertexArray());
  ifs->coord = c;

  if (this->lighting) {
    SoVRMLNormal * norm = new SoVRMLNormal;
    norm->vector.setValues(0, numv,
                           this->pvcache->getNormalArray());
    ifs->normal = norm;
  }    
  if (this->pvcache->colorPerVertex()) {
    SoVRMLColor * col = new SoVRMLColor;
    col->color.setNum(numv);
    uint8_t * src = (uint8_t*) this->pvcache->getColorArray();
    SbColor * dst = col->color.startEditing();
    for (int i = 0; i < numv; i++) {
      dst[i] = SbColor(src[0]/255.0f,
                       src[1]/255.0f,
                       src[2]/255.0f);
      src += 4;
    }
    col->color.finishEditing();
    ifs->color = col;
  }

  ifs->normalIndex.setNum(0);
  ifs->colorIndex.setNum(0);
  ifs->texCoordIndex.setNum(0);
  
  int numtri = this->pvcache->getNumIndices() / 3;
  const GLint * indices = this->pvcache->getIndices();
  ifs->coordIndex.setNum(numtri * 4);
  int32_t * ptr = ifs->coordIndex.startEditing();

  for (int i = 0; i < numtri; i++) {
    *ptr++ = (int32_t) indices[i*3];
    *ptr++ = (int32_t) indices[i*3+1];
    *ptr++ = (int32_t) indices[i*3+2];
    *ptr++ = -1;
  }
  ifs->coordIndex.finishEditing();

  int idx = path->getIndexFromTail(0);
  path->pop();
  if (parent->isOfType(SoGroup::getClassTypeId())) {
    SoGroup * g = (SoGroup*)parent;
    g->replaceChild(idx, ifs);
  }
  else {
    SoVRMLShape * shape = (SoVRMLShape*) parent;
    shape->geometry = ifs;
  }
  path->push(idx);
  ifs->unrefNoDelete();
}

void 
SoReorganizeActionP::replaceIls(SoFullPath * path)
{
  SoNode * parent = path->getNodeFromTail(1);
  if (!parent->isOfType(SoGroup::getClassTypeId())) {
    return;
  }

  SoVertexProperty * vp = this->createVertexProperty(TRUE);
  SoIndexedLineSet * ils = new SoIndexedLineSet;
  ils->ref();
  ils->vertexProperty = vp;
  ils->normalIndex.setNum(0);
  ils->materialIndex.setNum(0);
  ils->textureCoordIndex.setNum(0);
  
  int numlines = this->pvcache->getNumLineIndices() / 2;
  const GLint * indices = this->pvcache->getLineIndices();
  ils->coordIndex.setNum(numlines * 3);
  int32_t * ptr = ils->coordIndex.startEditing();
  
  for (int i = 0; i < numlines; i++) {
    *ptr++ = (int32_t) indices[i*2];
    *ptr++ = (int32_t) indices[i*2+1];
    *ptr++ = -1;
  }
  ils->coordIndex.finishEditing();

  int idx = path->getIndexFromTail(0);  
  path->pop();
  SoGroup * g = (SoGroup*)parent;
  g->replaceChild(idx, ils);
  path->push(idx);
  ils->unrefNoDelete();
}

void 
SoReorganizeActionP::replaceVrmlIls(SoFullPath * path)
{
  SoNode * parent = path->getNodeFromTail(1);
  if (!parent->isOfType(SoGroup::getClassTypeId()) &&
      !parent->isOfType(SoVRMLShape::getClassTypeId())) {
    return;
  }

  SoVRMLIndexedLineSet * ils = new SoVRMLIndexedLineSet;
  ils->ref();

  int numv = this->pvcache->getNumVertices();
  int numlines = this->pvcache->getNumLineIndices() / 2;
  const GLint * indices = this->pvcache->getLineIndices();
  ils->coordIndex.setNum(numlines * 3);
  int32_t * ptr = ils->coordIndex.startEditing();
  
  for (int i = 0; i < numlines; i++) {
    *ptr++ = (int32_t) indices[i*2];
    *ptr++ = (int32_t) indices[i*2+1];
    *ptr++ = -1;
  }
  ils->coordIndex.finishEditing();

  SoVRMLCoordinate * c = new SoVRMLCoordinate;
  c->point.setValues(0, numv,
                     this->pvcache->getVertexArray());
  ils->coord = c;
  
  if (this->pvcache->colorPerVertex()) {
    ils->colorPerVertex = TRUE;
    SoVRMLColor * col = new SoVRMLColor;
    col->color.setNum(numv);
    uint8_t * src = (uint8_t*) this->pvcache->getColorArray();
    SbColor * dst = col->color.startEditing();
    for (int i = 0; i < numv; i++) {
      dst[i] = SbColor(src[0]/255.0f,
                       src[1]/255.0f,
                       src[2]/255.0f);
      src += 4;
    }
    col->color.finishEditing();
    ils->color = col;
  }
  ils->colorIndex.setNum(0);
  
  int idx = path->getIndexFromTail(0);
  path->pop();
  if (parent->isOfType(SoGroup::getClassTypeId())) {
    SoGroup * g = (SoGroup*)parent;
    g->replaceChild(idx, ils);
  }
  else {
    SoVRMLShape * shape = (SoVRMLShape*) parent;
    shape->geometry = ils;
  }
  path->push(idx);
  ils->unrefNoDelete();
}
