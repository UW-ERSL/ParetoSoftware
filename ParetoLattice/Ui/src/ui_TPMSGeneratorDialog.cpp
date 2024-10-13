#include "ui_TPMSGeneratorDialog.hpp"
#include "CoreFunctions.hpp"

extern Voxelizer vox;
extern Voxelizer voxTemp;
extern bool getYesNoResponse(const char format[],...);
extern LatticeParameters tpmsParams;
extern LiveVariables liveVar;
extern ParetoApplication topopt;

//global variables
double TPMS_kx,TPMS_ky,TPMS_kz,Lx,Ly,Lz;
double lengthScale,rInner,rOuter,thickness,t, n, geomCenter[3], majorAxis[3], sharpness, smoothing;
int     Nx,Ny,Nz, number;
double  bBox[3][2];
int structureIndex;

std::vector<Point3D> boundaryPoints;

/************************************************************/
double sphereDiscrete(double pt[3])
{
    double val,distToCenter;

    distToCenter = DIST3D(pt,geomCenter);
    if ((distToCenter > rInner) && (distToCenter < rOuter))
        val = 1.0;
    else
        val = -1.0;

    return val;
}

/************************************************************/
double randomShape(double pt[3])
{
    double x, y, t, val;
    t = 0.5;
    x = sqrt(pt[0]*pt[0] + pt[1]*pt[1] + pt[2]*pt[2]);
    y = cos(x) + sin(x)*cos(x);

    if(y < t)
        val = t-y*y;
    else
        val = y-t*t;

    return val;
}
/************************************************************/
double sphereSmooth(double pt[3])
{
    double val,distToCenter;

    distToCenter = DIST3D(pt,geomCenter);

    // val = sqrt(pt[0]*pt[0] + pt[1]*pt[1] + pt[2]*pt[2] - t*t);

    if (distToCenter < rInner)
        val = distToCenter- rInner;
    else if ((distToCenter > rInner) && (distToCenter < rOuter))
        val = MIN(distToCenter- rInner,rOuter - distToCenter);
    else
        val = rOuter-distToCenter;


    return val/smoothing;


    // return val;
}

/************************************************************/
double torusSmooth(double pt[3])
{

    double val,majorRadius, minorRadius, distToCenter, dotProd;
    double minorRadiusInner, minorRadiusOuter, dist,crossProdMag;
    double Nz[3], DC[3],crossProd[3];


    distToCenter = DIST3D(pt,geomCenter);

    majorRadius = (rInner + rOuter)/2;
    minorRadius = rOuter-majorRadius;
    thickness = 0.001;

    minorRadiusInner = minorRadius - thickness;
    minorRadiusOuter = minorRadius + thickness;

    Nz[0] = 0;
    Nz[1] = 0;
    Nz[2] = 1;

    DC[0] = pt[0] - geomCenter[0];
    DC[1] = pt[1] - geomCenter[1];
    DC[2] = pt[2] - geomCenter[2];


    dotProd = DOTPROD3D(DC,Nz);
    CROSS3D(crossProd,Nz,DC);

    crossProdMag = (sqrt(pow(crossProd[0], 2) + pow(crossProd[1], 2) + pow(crossProd[2], 2)));

    dist = pow(pow(dotProd,2.0) + pow((crossProdMag-majorRadius),2.0),1.0/2.0);

    if (distToCenter < (majorRadius - minorRadiusOuter))
    {
        val = distToCenter - (majorRadius + minorRadiusOuter);

    }
    else if ((distToCenter > ( majorRadius - minorRadiusOuter) ) && (distToCenter < (majorRadius + minorRadiusOuter)) && (dist > minorRadiusInner) && (dist < minorRadiusOuter) )
    {
        val = (distToCenter + dist) / pow((majorRadius + minorRadiusOuter),2.0);

    }
    else
    {
        val = majorRadius + minorRadiusOuter - distToCenter;

        return val;

    }
}
/************************************************************/
double pTypeTPMS(double pt[3])
{
    double val;
    val = cos(TPMS_kx * pt[0]) + cos(TPMS_ky*pt[1]) + cos(TPMS_kz*pt[2]);
    switch (structureIndex)
    {
    case 0:
        //  Type 1: works fine with closing but outside surface isn't smooth
        if (val < 0)
            return (val/t)*smoothing;
        else
            return -(val-t)*smoothing;
        break;

    case 1:
        // Type 2: both sides smooth but no thickness(surface), closes the side holes completely at the boundaries
        // working code

        if (val<(t/2))

            val = t/2 - val;
        else
            val = MIN((t/2)- val,val - (t/2));

        return val/smoothing;
        break;
    }

}
/************************************************************/

double dTypeTPMS(double pt[3])
{
    double val;

    val = sin(TPMS_kx*pt[0])*sin(TPMS_ky*pt[1])*sin(TPMS_kz*pt[2]) +
            sin(TPMS_kx*pt[0])*cos(TPMS_ky*pt[1])*cos(TPMS_kz*pt[2]) +
            cos(TPMS_kx*pt[0])*sin(TPMS_ky*pt[1])*cos(TPMS_kz*pt[2])+
            cos(TPMS_kx*pt[0])*cos(TPMS_ky*pt[1])*sin(TPMS_kz*pt[2]);


    switch (structureIndex)
    {
    case 0:
        //  Type 1: works fine with closing but outside surface isn't smooth
        if (fabs(val) < t)
            return (val/t)*smoothing;
        else
            return (t-fabs(val))/t*smoothing;
        break;

    case 1:
        // Type 2: both sides smooth but no thickness(surface), closes the side holes completely at the boundaries
        // working code

        if (val<(t/2))

            val = t/2 - val;
        else
            val = MIN((t/2)- val,val - (t/2));
        return val/smoothing;
        break;
    }
}

/************************************************************/
double gTypeTPMS(double pt[3])
{
    double val;

    val = cos(TPMS_kx*pt[0])*sin(TPMS_ky*pt[1]) +
            cos(TPMS_ky*pt[1])*sin(TPMS_kz*pt[2]) +
            cos(TPMS_kz*pt[2])*sin(TPMS_kx*pt[0]);


    switch (structureIndex)
    {
    case 0:
        //  Type 1: works fine with closing but outside surface isn't smooth
        if (fabs(val) < t)
            return (val)*smoothing;
        else
            return (t-fabs(val))*smoothing;
        break;


    case 1:
        // Type 2: both sides smooth but no thickness(surface), closes the side holes completely at the boundaries
        // working code

        if (val<(t/2))

            val = t/2 - val;
        else
            val = MIN((t/2)- val,val - (t/2));
        return val/smoothing;
        break;
    }

}

/************************************************************/
double iwTypeTPMS(double pt[3])
{
    double val;


    val = 2*(cos(TPMS_kx*pt[0])*cos(TPMS_ky*pt[1]) +
            cos(TPMS_ky*pt[1])*cos(TPMS_kz*pt[2]) +
            cos(TPMS_kz*pt[2])*cos(TPMS_kx*pt[0]))
            - (cos(2*TPMS_kx*pt[0]) + cos(2*TPMS_ky*pt[1]) + cos(2*TPMS_kz*pt[2]));

    switch (structureIndex)
    {
    case 0:
        //  Type 1: works fine with closing but outside surface isn't smooth
        if (fabs(val) < t)
            return (val/t)*smoothing;
        else
            return (t-fabs(val))/t*smoothing;
        break;

    case 1:
        // Type 2: both sides smooth but no thickness(surface), closes the side holes completely at the boundaries
        // working code

        if (val<(t/2))

            val = t/2 - val;
        else
            val = MIN((t/2)- val,val - (t/2));
        return val/smoothing;
        break;
    }
}

/************************************************************/
double lidinoidTypeTPMS(double pt[3])
{
    double val;

    val = 0.5*(sin(2*TPMS_kx*pt[0])*cos(TPMS_ky*pt[1])*sin(TPMS_kz*pt[2])
            + sin(2*TPMS_ky*pt[1])*cos(TPMS_kz*pt[2])*sin(TPMS_kx*pt[0])
            + sin(2*TPMS_kz*pt[2])*cos(TPMS_kx*pt[0])*sin(TPMS_ky*pt[1]))
            - 0.5*(cos(2*TPMS_kx*pt[0])*cos(2*TPMS_ky*pt[1]) +
            cos(2*TPMS_ky*pt[1])*cos(2*TPMS_kz*pt[2]) + cos(2*TPMS_kz*pt[2])*cos(2*TPMS_ky*pt[1]));

    switch (structureIndex)
    {
    case 0:
        //  Type 1: works fine with closing but outside surface isn't smooth
        if (fabs(val) < t)
            return (val/t)*smoothing;
        else
            return (t-fabs(val))/t*smoothing;
        break;

    case 1:
        // Type 2: both sides smooth but no thickness(surface), closes the side holes completely at the boundaries
        // working code

        if (val<(t/2))

            val = t/2 - val;
        else
            val = MIN((t/2)- val,val - (t/2));
        return val/smoothing;
        break;
    }
}

/************************************************************/
double shrekTypeTPMS(double pt[3])
{
    double val;

    val = sinh(TPMS_kx*pt[0]) *sinh(TPMS_ky*pt[1]) - sin(TPMS_kz*pt[2]);


    switch (structureIndex)
    {
    case 0:
        //  Type 1: works fine with closing but outside surface isn't smooth
        if (fabs(val) < t)
            return (val/t)*smoothing;
        else
            return (t-fabs(val))/t*smoothing;
        break;

    case 1:
        // Type 2: both sides smooth but no thickness(surface), closes the side holes completely at the boundaries
        // working code

        if (val<(t/2))

            val = t/2 - val;
        else
            val = MIN((t/2)- val,val - (t/2));
        return val/smoothing;
        break;
    }
}


/************************************************************/
double neoviusTPMS(double pt[3])
{
    double val;

    val = 3*(cos(TPMS_kx*pt[0]) + cos(TPMS_ky*pt[1]) + cos(TPMS_kz*pt[2]))
            + 4*(cos(TPMS_kx*pt[0])*cos(TPMS_ky*pt[1])*cos(TPMS_kz*pt[2])) - t;


    switch (structureIndex)
    {
    case 0:
        //  Type 1: works fine with closing but outside surface isn't smooth
        if (fabs(val) < t)
            return (val/t)*smoothing;
        else
            return (t-fabs(val))/t*smoothing;
        break;

    case 1:
        // Type 2: both sides smooth but no thickness(surface), closes the side holes completely at the boundaries
        // working code

        if (val<(t/2))

            val = t/2 - val;
        else
            val = MIN((t/2)- val,val - (t/2));
        return val/smoothing;
        break;
    }
}


/************************************************************/
double dPrimeTPMS(double pt[3])
{
    double val;

    val = 0.5*sin(TPMS_kx*pt[0])*sin(TPMS_ky*pt[1])*sin(TPMS_kz*pt[2])
            + cos(TPMS_kx*pt[0])*cos(TPMS_ky*pt[1])*cos(TPMS_kz*pt[2])
            - 0.5*(cos(2*TPMS_kx*pt[0])*cos(2*TPMS_ky*pt[1])
            + cos(2*TPMS_ky*pt[1])*cos(2*TPMS_kz*pt[2]) + cos(2*TPMS_kz*pt[2])*cos(2*TPMS_kx*pt[0])) - t;

    switch (structureIndex)
    {
    case 0:
        //  Type 1: works fine with closing but outside surface isn't smooth
        if (fabs(val) < t)
            return (val/t)*smoothing;
        else
            return (t-fabs(val))/t*smoothing;
        break;

    case 1:
        // Type 2: both sides smooth but no thickness(surface), closes the side holes completely at the boundaries
        // working code

        if (val<(t/2))

            val = t/2 - val;
        else
            val = MIN((t/2)- val,val - (t/2));
        return val/smoothing;
        break;
    }
}


/////////////////////////////////////////////////////
double boundaryImplicitValue(double pt[3])
{
    double val;

    val = fabs(pt[0]*0 + pt[1] + pt[2])*-0.25;
    cout << "Points: "<< pt[0]<<pt[1]<<pt[2]<<endl;
    cout<<"Assigned value: "<< val << endl;
    return val;
}

/************************************************************/
bool callBackTPMSGenerator(double fractionCompleted)
{
    return true;
}
/////////////////////////////////////////////////////
TPMSDialog::TPMSDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);


    makeConnections();

}
/////////////////////////////////////////////////////
TPMSDialog::~TPMSDialog()
{
    ;
}
/////////////////////////////////////////////////////
void TPMSDialog::showEvent(QShowEvent *e)
{

    TPMSTYPE type;
    TPMSTYPE structure;

    number = tpmsParams.tpmsNumber;
    sharpness = tpmsParams.tpmsEdgeSharpness;
    smoothing = tpmsParams.tpmsSmoothing;
    thickness = tpmsParams.tpmsThickness;

    type = tpmsParams.tpmsType;
    structure = tpmsParams.structureType;

    voxelNumberSpinBox->setValue(number);
    edgeSharpnessSpinBox->setValue(sharpness);
    surfThicknessSpinBox->setValue(thickness);
    surfSmoothingSpinBox->setValue(smoothing);

    tpmsTypeCombo->setCurrentIndex((int)type);
    structureTypeCombo ->setCurrentIndex((int)structure);

}

/////////////////////////////////////////////////////
void TPMSDialog::createShell(void)
{
    string voxFile,stlFile, tpmsType, structureType;
    int numberOfLatticeVoxElems,symmetryIndex;
    bool success;
    std::string type;


    lengthScale = vox.getModelScale();
    tpmsParams.tpmsNumber = voxelNumberSpinBox->value();
    tpmsParams.tpmsEdgeSharpness = edgeSharpnessSpinBox->value();
    tpmsParams.tpmsSmoothing = surfSmoothingSpinBox->value();
    tpmsParams.tpmsThickness = surfThicknessSpinBox->value();
    tpmsParams.tpmsTotal = numTPMSSpinBox->value();


    tpmsParams.tpmsType = (TPMSTYPE) tpmsTypeCombo -> currentIndex();

    numberOfLatticeVoxElems = 1000000;
    sharpness = -0.1;
    smoothing = 5;
    t = 1000.0;
    Nx = 6;
    Ny = 2;
    Nz = 1;


    structureIndex = (TPMSTYPE) structureTypeCombo ->currentIndex();

    symmetryIndex = 1*liveVar.topoptConstraints.xSymmetry +
            2*liveVar.topoptConstraints.ySymmetry +
            3*liveVar.topoptConstraints.zSymmetry;

    voxFile = liveVar.projectDirectory + string("temp.vox");

    success = vox.voxelize(numberOfLatticeVoxElems,liveVar.maxAspectRatio,symmetryIndex,callBackTPMSGenerator);
    if (!success) {
        writeWarningToLog("CoreFunction: Could not create mesh... ");
        liveVar.MESH_APPLIED = false;
        return;
    }
    vox.getBoundingBox(bBox);

    Lx = bBox[0][1] - bBox[0][0];
    Ly = bBox[1][1] - bBox[1][0];
    Lz = bBox[2][1] - bBox[2][0];
    TPMS_kx =  2*3.1415*(Nx/Lx);
    TPMS_ky =  2*3.1415*(Ny/Ly);
    TPMS_kz =  2*3.1415*(Nz/Lz);

    structureIndex = 0;

    switch (tpmsTypeCombo->currentIndex())
    {
    case 0:
        tpmsType = "pTPMS";
        type.assign(tpmsType);
        vox.assignImplicitFunctionAtGridNodes(pTypeTPMS);
        //vox.assignValueAtAllVoxelBoundaryNodes(sharpness*smoothing);
        break;
    case 1:
        tpmsType = "dTPMS";
        type.assign(tpmsType);
        vox.assignImplicitFunctionAtGridNodes(dTypeTPMS);
        vox.assignValueAtAllVoxelBoundaryNodes(sharpness*smoothing);
        break;

    case 2:
        tpmsType = "gTPMS";
        type.assign(tpmsType);
        vox.assignImplicitFunctionAtGridNodes(gTypeTPMS);
        vox.assignValueAtAllVoxelBoundaryNodes(sharpness*smoothing);
        break;

    case 3:
        tpmsType = "iwTPMS";
        type.assign(tpmsType);
        vox.assignImplicitFunctionAtGridNodes(iwTypeTPMS);
        vox.assignValueAtAllVoxelBoundaryNodes(sharpness*smoothing);
        break;

    case 4:
        tpmsType = "lidinoidTPMS";
        type.assign(tpmsType);
        vox.assignImplicitFunctionAtGridNodes(lidinoidTypeTPMS);
        vox.assignValueAtAllVoxelBoundaryNodes(sharpness*smoothing);
        break;
    case 5:
        tpmsType = "shrekTPMS";
        type.assign(tpmsType);
        vox.assignImplicitFunctionAtGridNodes(shrekTypeTPMS);
        vox.assignValueAtAllVoxelBoundaryNodes(sharpness*smoothing);
        break;
    case 6:
        tpmsType = "sphereSmooth";
        type.assign(tpmsType);
        vox.assignImplicitFunctionAtGridNodes(sphereDiscrete);
        vox.assignValueAtAllVoxelBoundaryNodes(sharpness*smoothing);
        break;
    case 7:
        tpmsType = "neoviusTPMS";
        type.assign(tpmsType);
        vox.assignImplicitFunctionAtGridNodes(neoviusTPMS);
        vox.assignValueAtAllVoxelBoundaryNodes(sharpness*smoothing);
        break;
    case 8:
        tpmsType = "dPrimeTPMS";
        type.assign(tpmsType);
        vox.assignImplicitFunctionAtGridNodes(dPrimeTPMS);
        vox.assignValueAtAllVoxelBoundaryNodes(sharpness*smoothing);
        break;

    }


    if (structureIndex == 0)
        structureType = "Skeletal";
    else
        structureType = "Sheet";


    stlFile = liveVar.projectDirectory + liveVar.modelName + string("_") +  string(type) + string(".stl");

    vox.createAndDumpIsoSurfaceFromNodalValues(stlFile.c_str(),0);

    writeMessageToGUIWindow("Result File saved as %s", stlFile.c_str());

    voxTemp.deleteCurrentModel();
    voxTemp.readSTLGeom(stlFile.c_str(),0, false,NULL,false);
    liveVar.displayGeom = DISPLAY_GEOM_OTHER;// lattice loaded via voxTemp
    liveVar.displayField = DISPLAY_FIELD_NONE;

    viewer->updateGL();

    liveVar.geomFileName[liveVar.nSTLFilesLoaded] = stlFile;
    bool successfulLoad = coreFunctions.loadGeom(liveVar.assemblyMode);
    if(successfulLoad) {
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.GEOMETRY_UPLOADED = true;
    }

    voxTemp.deleteCurrentModel();
    voxTemp.readSTLGeom(stlFile.c_str(),0, false,NULL,false);
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;// lattice loaded via voxTemp
    liveVar.displayField = DISPLAY_FIELD_NONE;

    viewer->updateGL();
    closeTPMSDialog();

}

/*****************************************************************/
void TPMSDialog::createTPMS(void)
{
    writeMessageToGUIWindow("use an analytical function to create Minimal Surface");
    printf("Define an analytical function to create Minimal Surface");
}
/////////////////////////////////////////////////////
void TPMSDialog::closeTPMSDialog(void)
{
    viewer->updateGL();
    parentWidget()->show();
    close();
}
/**********************************************************/
void TPMSDialog::makeConnections()
{
    PushButton(createTPMSPushButton, [=]{ createShell();});
    PushButton(closePushButton,[=]{closeTPMSDialog();});

}
/////////////////////////////////////////////////////
