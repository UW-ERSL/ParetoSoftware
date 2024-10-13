#include "renderTools.hpp"
#include "CoreFunctions.hpp"
extern LiveVariables liveVar;
extern ParetoApplication topopt;
extern Voxelizer vox;
extern CoreFunctions coreFunctions;

#include <iostream>

///////////////////////////////////////////////////////////////////////////////

RenderToolsDialog :: RenderToolsDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}

///////////////////////////////////////////////////////////////////////////////

RenderToolsDialog :: ~RenderToolsDialog()
{
}

///////////////////////////////////////////////////////////////////////////////
void RenderToolsDialog :: changeDisplayMode()
{

   bool val,val2;
  // liveVa = axisCheckBox->isChecked();
    liveVar.geomTransparent = transparentCheckBox->isChecked();
    liveVar.plotAxis = axisCheckBox->isChecked();
    liveVar.displayEdges = edgeCheckBox->isChecked();
    liveVar.plotElasticityBC = showloadsCheckBox->isChecked();
    liveVar.plotSymmetryPlanes =  symmetryCheckBox->isChecked();
    liveVar.boundingBox =  bboxCheckBox->isChecked();
    liveVar.plotElasticityBC= showloadsCheckBox->isChecked();
    liveVar.plotThermalBC = showThermalLoadsCheckBox->isChecked();
    liveVar.showTriangles= wireframeCheckBox->isChecked(); // is this correct ?


     viewer->updateGL();



 val =  shownondesignCheckBox->isChecked(); // what live variable assoc here ?

} 

//////////////////////////////////////////////////////////////////////////////////
void RenderToolsDialog :: changeDisplay(int displayMode)
{


    liveVar.displayMode = displayMode;
    viewer->updateGL();


}
///////////////////////////////////////////////////////////////////////////////
void RenderToolsDialog :: changeDisplayOrientation(int displayOrientation)
{
// what function to call here?


viewer->updateGL();
}
///////////////////////////////////////////////////////////////////////////////
void RenderToolsDialog :: makeConnections()
{
 /*******************************************************************/
   CheckBox( axisCheckBox, [=]{ changeDisplayMode();});
   CheckBox( edgeCheckBox, [=]{ changeDisplayMode();});
   CheckBox( wireframeCheckBox, [=]{ changeDisplayMode();});
   CheckBox( showloadsCheckBox, [=]{ changeDisplayMode();});
   CheckBox( shownondesignCheckBox, [=]{ changeDisplayMode();});
   CheckBox( symmetryCheckBox, [=]{ changeDisplayMode();});
   CheckBox( bboxCheckBox, [=]{ changeDisplayMode();});
   CheckBox( transparentCheckBox, [=]{ changeDisplayMode();});
   CheckBox( showThermalLoadsCheckBox, [=]{ changeDisplayMode();});
/*********************************************************************/

// Are these correct ?  WARNING
   PushButton( isoSurfacePushButton, [=]{ changeDisplay(DISPLAY_ISO);});
   PushButton( DeformationPushButton, [=]{ changeDisplay(DISPLAY_FINAL_DELTA);});
   PushButton( temperaturePushButton, [=]{ changeDisplay(DISPLAY_TEMPERATURE);});
   PushButton( sensitivityPushButton, [=]{ changeDisplay(DISPLAY_STRESS_OVER_MESH);});
   PushButton( fieldPushButton, [=]{ changeDisplay(DISPLAY_TSFIELD);});
   PushButton( StlPushButton, [=]{ changeDisplay(DISPLAY_GEOM);});
   PushButton( meshPushButton, [=]{ changeDisplay(DISPLAY_MESH);});
/*********************************************************************/


   PushButton( xypPushButton, [=]{ changeDisplayOrientation(VIEW_XYPLUS);});
   PushButton( yzpPushButton, [=]{ changeDisplayOrientation(VIEW_YZPLUS);});
   PushButton( zxpPushButton, [=]{ changeDisplayOrientation(VIEW_XZPLUS);});
   PushButton( zxmPushButton, [=]{ changeDisplayOrientation(VIEW_XZMINUS);});
   PushButton( yzmPushButton, [=]{ changeDisplayOrientation(VIEW_YZMINUS);});
   PushButton( xymPushButton, [=]{ changeDisplayOrientation(VIEW_XYMINUS);});
   PushButton( isoPushButton, [=]{ changeDisplayOrientation(VIEW_ISO);});

/***********************************************************************/
}

///////////////////////////////////////////////////////////////////////////////
