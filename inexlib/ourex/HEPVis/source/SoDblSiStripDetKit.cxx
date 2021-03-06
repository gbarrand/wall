/*-----------------------------HEPVis----------------------------------------*/
/*                                                                           */
/* Node:             SoDblSiStripDetKit                                         */
/* Description:      Represents a single sided silicon strip detector        */
/* Author:           Joe Boudreau Nov 11 1996                                */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <math.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoAnnotation.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/draggers/SoScale1Dragger.h>
#include <Inventor/draggers/SoTranslate2Dragger.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/nodekits/SoAppearanceKit.h>
#include <Inventor/engines/SoCalculator.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoRotationXYZ.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoResetTransform.h>

#include <HEPVis/SbMath.h>
#include <HEPVis/nodes/SoSiStrips.h>
#include <HEPVis/nodekits/SoDblSiStripDetKit.h>

// This statement is required
SO_KIT_SOURCE(SoDblSiStripDetKit)

// initClass
void SoDblSiStripDetKit::initClass(){
  SO_KIT_INIT_CLASS(SoDblSiStripDetKit,SoBaseKit,"BaseKit");
}

// Constructor
SoDblSiStripDetKit::SoDblSiStripDetKit() {
  SO_KIT_CONSTRUCTOR(SoDblSiStripDetKit);
  SO_NODE_ADD_FIELD(alternateRep, (NULL));
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator,     SoSeparator,             FALSE,            this,\0,  FALSE);

  SO_KIT_ADD_CATALOG_ENTRY(transformSep,     SoSeparator,             FALSE,    topSeparator,\0,  FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(transformFlip,    SoRotation,              FALSE,    transformSep,\0,  FALSE); 
  SO_KIT_ADD_CATALOG_ENTRY(translateDragger, SoTranslate2Dragger,     TRUE,    transformSep,\0,  FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(localTransform,   SoTransform,             FALSE,    topSeparator,\0,  FALSE);

  SO_KIT_ADD_CATALOG_ENTRY(waferSeparator,   SoSeparator,             FALSE,    topSeparator,\0,  FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(waferAppearance,  SoAppearanceKit,         TRUE,   waferSeparator,\0,  TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(wafer,            SoCube,                  FALSE,  waferSeparator,\0,  FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(textSeparator,    SoSeparator,             FALSE,    topSeparator,\0,  FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(textAppearance,   SoAppearanceKit,         FALSE,   textSeparator,\0,  TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(textReset,        SoResetTransform,        FALSE,   textSeparator,\0,  FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(textTranslation,  SoTranslation,           FALSE,   textSeparator,\0,  FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(text,             SoText2,                 FALSE,   textSeparator,\0,  FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(stripsSeparator,  SoSeparator,             FALSE,    topSeparator,\0,  FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(stripsAppearance, SoAppearanceKit,         TRUE,  stripsSeparator,\0,  TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(stripsScale,      SoScale,                 FALSE, stripsSeparator,\0,  FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(stripsDragger,    SoScale1Dragger,         TRUE,  stripsSeparator,\0,  FALSE); 
  SO_KIT_ADD_CATALOG_ENTRY(phiStrips,        SoSiStrips,              FALSE, stripsSeparator,\0,  TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(ninetyDeg,        SoRotationXYZ,           FALSE, stripsSeparator,\0,  FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(zStrips,          SoSiStrips,              FALSE, stripsSeparator,\0,  TRUE);
  SO_KIT_INIT_INSTANCE();
  createInitialStripDet();
}

// Destructor
SoDblSiStripDetKit::~SoDblSiStripDetKit() {
}

SbBool SoDblSiStripDetKit::affectsState() const {
  return FALSE;
}

void SoDblSiStripDetKit::createInitialStripDet() {

  SoCube* theFamousCube = (SoCube*)wafer.getValue();
  SoSiStrips* theFamousPhiStrips = (SoSiStrips*)phiStrips.getValue();
  theFamousCube->width.connectFrom(&theFamousPhiStrips->thickness); 
  theFamousCube->height.connectFrom(&theFamousPhiStrips->width);
  theFamousCube->depth.connectFrom(&theFamousPhiStrips->length);

  SoAppearanceKit *TextAppearance = (SoAppearanceKit *) textAppearance.getValue();  
  TextAppearance->set((char*)"font",(char*)"size 22");
  TextAppearance->set((char*)"material",(char*)"diffuseColor .2 .2 1.0");
  TextAppearance->set((char*)"lightModel",(char*)"model BASE_COLOR");

  // Put Mr. ninety-degree-Rotation there
  SoRotationXYZ *MisterRotation = (SoRotationXYZ*)ninetyDeg.getValue();
  MisterRotation->axis=SoRotationXYZ::X;
  MisterRotation->angle=(float)(M_PI/2);

  // New, for detector aligment
  SoRotation   *MrTransformFlip = (SoRotation *) transformFlip.getValue();
  MrTransformFlip->rotation.setValue(SbVec3f(0,1,0),(float)M_PI/2);
  // End new part

  // Set up the callbacks
  SoEventCallback *myCallback = new SoEventCallback();
  myCallback->addEventCallback(SoMouseButtonEvent::getClassTypeId(),
			       SoDblSiStripDetKit::activateDetector,
			       this);
  myCallback->addEventCallback(SoMouseButtonEvent::getClassTypeId(),
			       SoDblSiStripDetKit::deactivateDetector  ,
			       this);
  myCallback->addEventCallback(SoMouseButtonEvent::getClassTypeId(),
			       SoDblSiStripDetKit::translateDetector,
			       this);
  myCallback->addEventCallback(SoMouseButtonEvent::getClassTypeId(),
			       SoDblSiStripDetKit::detranslateDetector  ,
			       this);
  myCallback->addEventCallback(SoMouseButtonEvent::getClassTypeId(),
			       SoDblSiStripDetKit::activateStrip,
			       this);
  setPart("callbackList[0]",myCallback); 
  isActivated=0;  
  isAligning =0;
  isAlignable = 0;
}

void SoDblSiStripDetKit::translateDetector(void *userData, SoEventCallback *eventCB){

  // Was the event previously handled?
  if (eventCB->isHandled()) return;

  // Which Strip Detector is this being called for?
  SoDblSiStripDetKit *This = (SoDblSiStripDetKit *) userData;

  // Find out whether that's the one that has been picked
  SoHandleEventAction *handleEventAction = eventCB->getAction();
  const SoPickedPoint *pickedPoint = handleEventAction->getPickedPoint();
  if (pickedPoint) {
    SoFullPath          *path        = (SoFullPath *) pickedPoint->getPath();
    SoNode *ancestorNode=NULL;
    for (int i=0;i<path->getLength();i++) {
      ancestorNode  = path->getNodeFromTail(i);
      if (ancestorNode->isOfType(SoDblSiStripDetKit::getClassTypeId()))  break;
    }
    if (This!=ancestorNode) return;
  }
  else {
    return;
  }

  // Is the detector already activated?
  if (This->isAligning) return;
  if (!This->isAlignable) return;

  // Was it a SHIFT_MB2_DOWN  event?
  const SoMouseButtonEvent *event= (SoMouseButtonEvent *) eventCB->getEvent();
  if (!SoMouseButtonEvent::isButtonPressEvent(event,SoMouseButtonEvent::BUTTON2)) return;
  if (event->wasCtrlDown() || !event->wasShiftDown()) return;

  // Then put a dragger on the wafer, set a bunch of flags and say goodbye.
  if (!This->translateDragger.getValue()) {
    SoTranslate2Dragger *theDragger=new SoTranslate2Dragger();
    This->translateDragger.setValue(theDragger);
    ((SoSeparator *) (This->transformSep.getValue()))->addChild(This->translateDragger.getValue());
    
    // Turn off the horrid White an yellow crosses!
    SoCube *MyCube = new SoCube();
    SoCalculator *MyCalculator = new SoCalculator();
    MyCalculator->a.connectFrom(&((SoCube *) This->wafer.getValue())->depth);
    MyCalculator->b.connectFrom(&((SoCube *) This->wafer.getValue())->height);
    MyCalculator->c.connectFrom(&((SoCube *) This->wafer.getValue())->width);
    MyCalculator->expression.set1Value(0,"oa=1.1*a");
    MyCalculator->expression.set1Value(1,"ob=1.1*b");
    MyCalculator->expression.set1Value(2,"oc=1.1*c");
    MyCube->width.connectFrom(&MyCalculator->oa);
    MyCube->height.connectFrom(&MyCalculator->ob);
    MyCube->depth.connectFrom(&MyCalculator->oc); 
    SoSeparator *MySeparator = new SoSeparator();
    SoMaterial  *MyMaterial = new SoMaterial;
    MyMaterial->diffuseColor.setValue(0.6F,0.6F,0);
    MyMaterial->emissiveColor.setValue(0.6F,0.6F,0);
    MySeparator->addChild(MyMaterial);
    MySeparator->addChild(MyCube);
    
    theDragger->setPart("translator",MySeparator);
    theDragger->setPart("translatorActive",NULL);
  }
  
  SoTransform         *theFamousTransform = (SoTransform *) This->localTransform.getValue();
  SoTranslate2Dragger *theFamousDragger = (SoTranslate2Dragger *) This->translateDragger.getValue();
  SbVec3f translation =theFamousDragger->translation.getValue();
  theFamousDragger->translation.setValue(translation[2],translation[1],-translation[0]);

  SoCalculator *MyCalculator2 = new SoCalculator();
  MyCalculator2->A.connectFrom(&theFamousDragger->translation);
  MyCalculator2->expression.set1Value(0,"oA=vec3f(A[2],A[1],-A[0])");

  theFamousTransform->translation.connectFrom(&MyCalculator2->oA);
  This->isAligning=TRUE;
  eventCB->setHandled();
     
}

void SoDblSiStripDetKit::detranslateDetector(void *userData, SoEventCallback *eventCB){
  // Was the event previously handled?
  if (eventCB->isHandled()) return;

  // Which Strip Detector is this being called for?
  SoDblSiStripDetKit *This = (SoDblSiStripDetKit *) userData; // Just as if ...

  // Find out whether that's the one that has been picked
  SoHandleEventAction *handleEventAction = eventCB->getAction();
  const SoPickedPoint *pickedPoint = handleEventAction->getPickedPoint();
  if (pickedPoint) {
    SoFullPath          *path        = (SoFullPath *) pickedPoint->getPath();
    SoNode *ancestorNode=NULL;
    for (int i=0;i<path->getLength();i++) {
      ancestorNode  = path->getNodeFromTail(i);
      if (ancestorNode->isOfType(SoDblSiStripDetKit::getClassTypeId()))  break;
    }
    if (This!=ancestorNode) return;
  }
  else {
    return;
  }

  // Is the detector already deactivated?
  if (!This->isAligning) return;

  // Was it a CTRL_MB2_DOWN  event?
  const SoMouseButtonEvent *event= (SoMouseButtonEvent *) eventCB->getEvent();
  if (!SoMouseButtonEvent::isButtonPressEvent(event,SoMouseButtonEvent::BUTTON2)) return;
  if (event->wasCtrlDown() || !event->wasShiftDown()) return;

  // Take the transform off the wafer
  SoTransform *theFamousTransform = (SoTransform *) This->localTransform.getValue();
  theFamousTransform->translation.disconnect();

  SoSeparator *ParentSeparator = (SoSeparator *) This->transformSep.getValue();
  for (int child=0;child<ParentSeparator->getNumChildren();child++) {
    if (ParentSeparator->getChild(child)->isOfType(SoTranslate2Dragger::getClassTypeId())) {
      ParentSeparator->removeChild(child);
      break;
    }
  }
  This->translateDragger.setValue(NULL);
  This->isAligning=FALSE;
  eventCB->setHandled();
}

void SoDblSiStripDetKit::activateDetector(void *userData, SoEventCallback *eventCB){

  // Was the event previously handled?
  if (eventCB->isHandled()) return;

  // Which Strip Detector is this being called for?
  SoDblSiStripDetKit *This = (SoDblSiStripDetKit *) userData;

  // Find out whether that's the one that has been picked
  SoHandleEventAction *handleEventAction = eventCB->getAction();
  const SoPickedPoint *pickedPoint = handleEventAction->getPickedPoint();
  if (pickedPoint) {
    SoFullPath          *path        = (SoFullPath *) pickedPoint->getPath();
    SoNode *ancestorNode=NULL;
    for (int i=0;i<path->getLength();i++) {
      ancestorNode  = path->getNodeFromTail(i);
      if (ancestorNode->isOfType(SoDblSiStripDetKit::getClassTypeId()))  break;
    }
    if (This!=ancestorNode) return;
  }
  else {
    return;
  }

  // Is the detector already activated?
  if (This->isActivated) return;

  // Was it a CTRL_MB1_DOWN  event?
  const SoMouseButtonEvent *event= (SoMouseButtonEvent *) eventCB->getEvent();
  if (!SoMouseButtonEvent::isButtonPressEvent(event,SoMouseButtonEvent::BUTTON2)) return;
  if (!event->wasCtrlDown() || event->wasShiftDown()) return;

  // Then put a dragger on the wafer, set a bunch of flags and say goodbye.
  if (!This->stripsDragger.getValue()) {
    This->stripsDragger.setValue(new SoScale1Dragger());
    ((SoSeparator *) (This->stripsSeparator.getValue()))->addChild(This->stripsDragger.getValue());
  }
  
  SoScale *theFamousScale = (SoScale *) This->stripsScale.getValue();
  SoScale1Dragger *theFamousDragger = (SoScale1Dragger *) This->stripsDragger.getValue();

  theFamousDragger->setMinScale(0.5);
  theFamousDragger->scaleFactor.setValue(theFamousScale->scaleFactor.getValue());
  theFamousScale->scaleFactor.connectFrom(&theFamousDragger->scaleFactor);
  This->isActivated=TRUE;
  eventCB->setHandled();
     
}

void SoDblSiStripDetKit::deactivateDetector(void *userData, SoEventCallback *eventCB){
  // Was the event previously handled?
  if (eventCB->isHandled()) return;

  // Which Strip Detector is this being called for?
  SoDblSiStripDetKit *This = (SoDblSiStripDetKit *) userData; // Just as if ...

  // Find out whether that's the one that has been picked
  SoHandleEventAction *handleEventAction = eventCB->getAction();
  const SoPickedPoint *pickedPoint = handleEventAction->getPickedPoint();
  if (pickedPoint) {
    SoFullPath          *path        = (SoFullPath *) pickedPoint->getPath();
    SoNode *ancestorNode=NULL;
    for (int i=0;i<path->getLength();i++) {
      ancestorNode  = path->getNodeFromTail(i);
      if (ancestorNode->isOfType(SoDblSiStripDetKit::getClassTypeId()))  break;
    }
    if (This!=ancestorNode) return;
  }
  else {
    return;
  }

  // Is the detector already deactivated?
  if (!This->isActivated) return;

  // Was it a CTRL_MB1_DOWN  event?
  const SoMouseButtonEvent *event= (SoMouseButtonEvent *) eventCB->getEvent();
  if (!SoMouseButtonEvent::isButtonPressEvent(event,SoMouseButtonEvent::BUTTON2)) return;
  if (!event->wasCtrlDown() || event->wasShiftDown()) return;

  // Then put a dragger on the wafer, set a bunch of flags and say goodbye.
  SoScale *theFamousScale = (SoScale *) This->stripsScale.getValue();
  theFamousScale->scaleFactor.disconnect();

  SoSeparator *ParentSeparator = (SoSeparator *) This->stripsSeparator.getValue();
  for (int child=0;child<ParentSeparator->getNumChildren();child++) {
    if (ParentSeparator->getChild(child)->isOfType(SoScale1Dragger::getClassTypeId())) {
      ParentSeparator->removeChild(child);
      break;
    }
  }
  This->stripsDragger.setValue(NULL);
  This->isActivated=FALSE;
  eventCB->setHandled();
}


void SoDblSiStripDetKit::activateStrip(void *userData, SoEventCallback *eventCB){

  // Which Strip Detector is this being called for?
  SoDblSiStripDetKit *This = (SoDblSiStripDetKit *) userData;

  // Reset the text string whenever a button goes down, handled or not:
  ((SoText2 *) (This->text.getValue()))->string="";

  // Was the event previously handled?
  if (eventCB->isHandled()) return;

  // Was it a MB1_DOWN  event?
  const SoMouseButtonEvent *event= (SoMouseButtonEvent *) eventCB->getEvent();
  if (!SoMouseButtonEvent::isButtonPressEvent(event,SoMouseButtonEvent::BUTTON1)) return;
  if (event->wasCtrlDown() || event->wasShiftDown()) return;

  // Has anything been picked?
  SoHandleEventAction *handleEventAction = eventCB->getAction();
  const SoPickedPoint *pickedPoint = handleEventAction->getPickedPoint();
  if (!pickedPoint) return;
  SoFullPath          *path        = (SoFullPath *) pickedPoint->getPath();
  SoNode *ancestorNode=NULL;
  for (int i=0;i<path->getLength();i++) {
    ancestorNode  = path->getNodeFromTail(i);
    if (ancestorNode->isOfType(SoDblSiStripDetKit::getClassTypeId()))  break;
  }
  if (This!=ancestorNode) return;

  // FIRST DEAL WITH PHI
  // Get gain, thickness
  SoSiStrips *ThePhiStrips = (SoSiStrips *) This->phiStrips.getValue();
  SoSiStrips *TheZStrips   = (SoSiStrips *) This->zStrips.getValue();
  if (ThePhiStrips) { 
    float thickness=ThePhiStrips->thickness.getValue(), gain=ThePhiStrips->gain.getValue(); 

    // Find out if you got a strip
    SoFullPath *theFullPath = (SoFullPath *) path;
    if (theFullPath->containsNode(ThePhiStrips)) {
      SoNode *theEndNode= theFullPath->getTail();
      if (theEndNode->isOfType(SoCube::getClassTypeId())) {
	SoCube *theHit = (SoCube *) theEndNode;
	char labelString[256];
	sprintf(labelString,"    %d", (int) ((theHit->width.getValue()-thickness)/gain + 0.5));
	((SoText2 *) (This->text.getValue()))->string=labelString;
	((SoTranslation *) This->textTranslation.getValue())->translation= pickedPoint->getPoint()+ 0.01F*pickedPoint->getNormal();
	eventCB->setHandled();
        return;
      }
    }
  }
  if (TheZStrips) {
    float thickness=TheZStrips->thickness.getValue(), gain=TheZStrips->gain.getValue(); 

    // Find out if you got a strip
    SoFullPath *theFullPath = (SoFullPath *) path;
    if (theFullPath->containsNode(TheZStrips)) {
      SoNode *theEndNode= theFullPath->getTail();
      if (theEndNode->isOfType(SoCube::getClassTypeId())) {
	SoCube *theHit = (SoCube *) theEndNode;
	char labelString[256];
	sprintf(labelString,"    %d", (int) ((theHit->width.getValue()-thickness)/gain + 0.5));
	((SoText2 *) (This->text.getValue()))->string=labelString;
	((SoTranslation *) This->textTranslation.getValue())->translation= pickedPoint->getPoint()+ 0.01F*pickedPoint->getNormal();
	eventCB->setHandled();
        return;
      }
    }
  }
}


// generateAlternateRep
void SoDblSiStripDetKit::generateAlternateRep(SoAction* aAction) {

  clearAlternateRep();  
  SoSiStrips* ThePhiStrips = (SoSiStrips*)phiStrips.getValue();
  SoSiStrips* TheZStrips = (SoSiStrips*)zStrips.getValue();
  if (ThePhiStrips) ThePhiStrips->generateAlternateRep(aAction);
  if (TheZStrips) TheZStrips->generateAlternateRep(aAction);
  alternateRep.setValue(topSeparator.getValue());
}

// clearAlternateRep
void SoDblSiStripDetKit::clearAlternateRep() {
  SoSiStrips* ThePhiStrips = (SoSiStrips*)phiStrips.getValue();
  if (ThePhiStrips) ThePhiStrips->clearAlternateRep();
  SoSiStrips* TheZStrips = (SoSiStrips*)zStrips.getValue();
  if (TheZStrips) TheZStrips->clearAlternateRep();
  alternateRep.setValue(NULL);
}

void SoDblSiStripDetKit::setAlignable(SbBool flag) {
 isAlignable=flag;
}

SoTransform *SoDblSiStripDetKit::getLocalTransform () const {
  SoTransform* theFamousTransform = (SoTransform*)localTransform.getValue();
  return theFamousTransform;
}










