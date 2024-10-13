#include "ui_geomDialog.hpp"
#include <iostream>
#include "CoreFunctions.hpp"

// Header the license manager for CADEX
#include <cadex/LicenseManager_Activate.h>

// Headers for readers and writers
#include <cadex/STEP_Reader.hxx>
#include <cadex/IGES_Reader.hxx>
#include <cadex/STL_Writer.hxx>
#include <cadex/STL_WriterParameters.hxx>
#include <cadex/OBJ_Writer.hxx>
#include <cadex/OBJ_WriterParameters.hxx>

//Auxilliary headers for processing faces and triangles
#include <cadex/ModelData_PolyShape.hxx>
#include <cadex/ModelData_PolyShapeList.hxx>
#include <cadex/ModelAlgo_BRepMesherParameters.hxx>
#include <cadex/ModelAlgo_BRepMesher.hxx>
#include <cadex/ModelData_Assembly.hxx>
#include <cadex/ModelData_Body.hxx>
#include <cadex/ModelData_BodyList.hxx>
#include <cadex/ModelData_BRepRepresentation.hxx>
#include <cadex/ModelData_Face.hxx>
#include <cadex/ModelData_IndexedTriangleSet.hxx>
#include <cadex/ModelData_Model.hxx>
#include <cadex/ModelData_Part.hxx>
#include <cadex/ModelData_PolyRepresentation.hxx>
#include <cadex/ModelData_PolyShapeList.hxx>
#include <cadex/ModelData_Shell.hxx>
#include <cadex/ModelData_Solid.hxx>
#include <cadex/ModelData_SceneGraphElementUniqueVisitor.hxx>


//CADEx Key for UW Madison
static const char aKey[] =
    "@CUSTOMER=EVAL @CUSTOMER_APP=EVAL @PRODUCT=ANY @OS=ANY @CONFIGURATION=ANY @VERSI"
    "ON=ANY @UPGRADE_EXPIRATION=20181124 @USE_EXPIRATION=20181124 @VENDOR=http://www."
    "cadexchanger.com @WARNING=This license key may not be disclosed to any third par"
    "ty without prior written permission from vendor. 53RNPZEIHGV7E4VBQOC39TNLOELH0AY"
    "1VA7L6THXILJBILPFDXPESMI77RES9BC4H3JBI2M7LG4MX4LORLQISM1JWH96G190EWIFY7Q4T7ETEG4"
    "9X0UGJ6P6LWD34T80H39DZW0IDNB3WQEVTE6VFAQ67RVGZ86XCPGFJ1QBZOOW9QUO8MTBIYN1NVNGC0V"
    "7IPULF9GVT70BQ1M44VPEDH7YPU6BJRNJ58TJFG0ACBNCF1MG298F1OLANCGJHO465HEF9DR0CA074UV"
    "GV98F1OLO854BI89A9SGJ90SEMBS6V9V2R0L1PZGYT26J4C2EUE874914X4QMMRJHLFVGK4XXQPDTXMC"
    "8C88LF8GJRB0OHYNJHR8JE5R6TQQN11MIE9JE9OQ8XMNFB1KK1KF0KKOYNILXF5AZQKCB5OXLDWEM280"
    "Z4KU4S8MS8M2WRY0J2YKHSLNL4B0JRQ1IQIL3YFKLABKALRCJS7VFSQGGG501JD050FA6D0RDN7QIUCF"
    "2792D20T1HES8V169L0T4V9S23SJT3FL7P7J3HAO2K5QFEDQG3CS8V9SJFI30HG33KRG71F41C1JQJAQ"
    "A94O6IE53KPI5QP23P7C523TGSCK51SJT553N6D433S05C9OZIS67I94B2DPP3S3GXDP3HPR4MOO5LH0"
    "053Q45A48U5X6V6VMV2KZSDM0PJCSY6CAPLA5INRCEKD3MLR5AE41OQ57RDYSMXSCJTCFK97A7SRGG69";

using namespace std;
using namespace cadex;


class PartMesherVisitor : public ModelData_Model::VoidElementVisitor
{
public:
#define MAX_BODIES 20
    ModelData_BodyList::SizeType nBodies;
    int nTrianglesInBody[MAX_BODIES];
    int nFacesInBody[MAX_BODIES];
    int nTrianglesInFaceOfBody[MAX_BODIES][STL_MAX_GROUPS];
    int nTrianglesTotal;
    double triCenterRef[3];
    PartMesherVisitor (const ModelAlgo_BRepMesherParameters& theParam, const Base_ProgressStatus& theStatus) :
        myParam (theParam),
        myStatus (theStatus)
    {}
    virtual void operator() (const ModelData_Part& thePart) override
    {
        if (myStatus.WasCanceled())
            return;
        //each part takes 1/N of the global range (where N is number of parts)
        ModelData_BRepRepresentation aBRep = thePart.BRepRepresentation();
        if (!aBRep) return;

        const ModelData_BodyList& aList = aBRep.Get();
        nBodies = aList.Size();

        nTrianglesTotal= 0;

        //cout << "There are  " << nBodies << " body(ies) " << endl;
        //mesh each B-Rep body
        for (ModelData_BodyList::SizeType i = 0; i < nBodies && !myStatus.WasCanceled(); ++i) {
            nTrianglesInBody[i] = 0;
            ModelAlgo_BRepMesher aMesher (myParam);
            aMesher.ProgressStatus() = myStatus; //connect progress status
            const ModelData_Body& aBody = aList[i];
            if (aBody.BodyType() != ModelData_BT_Sheet && aBody.BodyType() != ModelData_BT_Solid)
                continue;
            ModelData_PolyShapeList aPList = aMesher.Compute (aBody, aBRep);
            ModelData_PolyShapeList::SizeType nFaces = aPList.Size();
            nFacesInBody[i] = nFaces;
            //cout << "Body " << i << " has " << nFaces << " faces " << endl;
            for (ModelData_PolyShapeList::SizeType face = 0; face < nFaces; ++face) {
                const ModelData_PolyVertexSet& aPVS = aPList[face];
                if (aPVS.TypeId() == ModelData_IndexedTriangleSet::GetTypeId()) {
                    ModelData_IndexedTriangleSet theTS = static_cast<const ModelData_IndexedTriangleSet&> (aPVS);
                    ModelData_IndexedTriangleSet::IndexType nTriangles = theTS.NumberOfFaces();
                    nTrianglesInBody[i] += nTriangles;
                    nTrianglesInFaceOfBody[i][face] = nTriangles;
                    for (int tri = 0; tri < nTriangles; ++tri) {
                        triCenterRef[0] = triCenterRef[1] = triCenterRef[2] = 0;
                        for (int k = 0; k < 3; ++k) {// 3 nodes
                            const ModelData_IndexedTriangleSet::CoordType& aV = theTS.Coordinate (tri, k);
                            triCenterRef[0] += aV.X();
                            triCenterRef[1] += aV.Y();
                            triCenterRef[2] += aV.Z();
                        }
                        triCenterRef[0] /= 3.0;
                        triCenterRef[1] /= 3.0;
                        triCenterRef[2] /= 3.0;

                        double triCenter[3],x[3],y[3],z[3];
                        for (int stlTri = 0; stlTri < vox.getNumTriangles(); stlTri++){
                            vox.getTriPts(stlTri,x,y,z);
                            triCenter[0] = (x[0]+x[1]+x[2])/3.0;
                            triCenter[1] = (y[0]+y[1]+y[2])/3.0;
                            triCenter[2] = (z[0]+z[1]+z[2])/3.0;
                            if (DIST3D(triCenter,triCenterRef) < vox.getModelScale()*1e-5){
                                vox.setTriGroup(stlTri,face);
                                break;
                            }
                        }
                    }
                }

            }

            cout << "Body " << i << " has a total of " << nTrianglesInBody[i] << " triangles " << endl;
            cout << "-----------------------------------" << endl;
            nTrianglesTotal += nTrianglesInBody[i];
        }
    }
protected:
    ModelAlgo_BRepMesherParameters  myParam;
    Base_ProgressStatus             myStatus; //connects to shared progress status
};
///////////////////////////////////////////////////////////////////////////////

GeometryDialog :: GeometryDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
    unitMap["meter"] = STL_M;
    unitMap["millimeter"] = STL_MM;
    unitMap["centimeter"] = STL_CM;
    unitMap["inch"] = STL_INCH;
}

///////////////////////////////////////////////////////////////////////////////

GeometryDialog :: ~GeometryDialog()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
void GeometryDialog :: closeDialog()
{
    emit setGeometryComplete();
    parentWidget()->show();
    close();
}

///////////////////////////////////////////////////////////////////////////////
void GeometryDialog :: loadSTL()
{
    //static QString lastSelectedDirectory;
    QString qstr = QFileDialog::getOpenFileName(this,
                                                *new QString("Select Geometry File "),
                                                QString(""),
                                              *new QString( "Mesh Format (*.off *.stl)"));

    string fileName = qstr.toUtf8().constData();
    if (fileName.empty()){
        return;
    }

    if ( (fileName.find(".off") == std::string::npos) && (fileName.find(".stl") == std::string::npos)
         && (fileName.find(".STL") == std::string::npos)){
        writeWarningToLog("File format not recognized; acceptable extensions: .off or .stl");
        return;
    }

    liveVar.geomFileName[liveVar.nSTLFilesLoaded] = fileName;
    bool successfulLoad = coreFunctions.loadGeom(liveVar.assemblyMode);
    if(successfulLoad) {
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.GEOMETRY_UPLOADED = true;
    }

    viewer->updateGL();
    closeDialog();

    static bool firstTime = false;
    if (firstTime){
        QMessageBox msgBox;
        msgBox.setText("Mouse and keyboard behavior\n\n"
                       "To rotate: Use left button\n"
                       "To zoom: Use middle button, or left button + control key\n"
                       "To pan: Use right button\n\n"

                       "To select surface: Use left button + alt key\n"
                       "To unselect: Use right button + alt key\n");
        msgBox.addButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowModality(Qt::NonModal);
        msgBox.show();
        int32_t userReply = msgBox.exec();
        firstTime = false;
    }
}
///////////////////////////////////////////////////////////////////////////////
void GeometryDialog :: loadGeometry()
{
    //static QString lastSelectedDirectory;
    QString qstr = QFileDialog::getOpenFileName(this,
                                                *new QString("Select Geometry File "),
                                                QString(""),
                                              *new QString( "Mesh Format (*.off *.stl *.obj *.step *.stp *.iges .igs)"));

    string fileName = qstr.toUtf8().constData();
    ModelData_Model aModel;


    if (fileName.empty()){
        return;
    }
    //accepted formats, must consider lower and upper cases
    //don't change the order, append if needed.
    char *acceptedFormats[] = {".stl",".STL",".off",".OFF",".obj",".OBJ",".step",".STEP",".stp",".STP",".iges",".IGES",".igs",".IGS"};
    int nFormats = 14;
    int formatId,extensionIndex;
    // check format
    for ( formatId = 0; formatId < nFormats; formatId++){
        extensionIndex = fileName.find(acceptedFormats[formatId]);
        if (extensionIndex != std::string::npos)
            break;
    }
    if (formatId == nFormats){
        string msg("File format not recognized; acceptable extensions: ");
        for ( formatId = 0; formatId < nFormats; formatId++){
            msg.append(acceptedFormats[formatId]);
        }
        writeWarningToLog(msg.c_str());
        return;
    }
    writeMessageToParetoLog("Input format: %s",acceptedFormats[formatId] );

    switch (formatId){
        case 0: case 1: case 2: case 3: case 4: case 5: break;// stl off or obj format nothing to do
        case 6: case 7: case 8: case 9: {// step format, convert to stl format
            //activate the license (aKey must be defined in cadex_license.cxx)
            if (!CADExLicense_Activate (aKey)) {
                writeWarningToLog("Failed to activate CADExchanger license.");
                return;
            }
            writeMessageToParetoLog("Attempting to read step file ");
            STEP_Reader aReader;
            if (!aReader.ReadFile(fileName.c_str()) || !aReader.Transfer (aModel)) {
                writeWarningToLog("Failed to read the file %s", fileName.c_str());
                return;
            }
            break;
        }
        case 10: case 11: case 12: case 13: {// iges format, convert to stl format
            //activate the license (aKey must be defined in cadex_license.cxx)
            if (!CADExLicense_Activate (aKey)) {
                writeWarningToLog("Failed to activate CADExchanger license.");
                return;
            }
            writeMessageToParetoLog("Attempting to read iges file ");
            IGES_Reader aReader;
            if (!aReader.ReadFile(fileName.c_str()) || !aReader.Transfer (aModel)) {
                writeWarningToLog("Failed to read the file %s", fileName.c_str());
                return;
            }
            break;
        }
        default:
            writeWarningToLog("File format %s not recognized",fileName.c_str());
            return;
            break;
    }

    if (formatId > 5){
        if ((formatId == 6) || (formatId == 7) || (formatId == 10) || (formatId == 11))
            fileName.replace(extensionIndex, 5, ".stl\0");

        if ((formatId == 8) || (formatId == 9) || (formatId == 12) || (formatId == 13))
            fileName.replace(extensionIndex, 4, ".stl");

        // now write the stl file
        STL_Writer stlWriter;
        STL_WriterParameters & stlWP = stlWriter.Parameters();

        stlWP.IsText() = false;//write in binary format
        if (!stlWriter.Transfer (aModel) || !stlWriter.WriteFile (fileName.c_str())) {
            writeWarningToLog("Failed to write the file %s", fileName.c_str());
            return;
        }
    }

    // proceed to load .stl, obj or .off file
    liveVar.geomFileName[liveVar.nSTLFilesLoaded] = fileName;
    bool successfulLoad = coreFunctions.loadGeom(liveVar.assemblyMode);
    if(successfulLoad) {
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.GEOMETRY_UPLOADED = true;
    }
    if (formatId > 5) {// // after file is loaded, if STEP or IGES, process faces
        vox.resetGroups();
        Base_ProgressStatus aStatus;
        ModelAlgo_BRepMesherParameters aParam;
        // Use default granularity so that the number of triangles in Brep matches that of STL
        aParam.MergeFaceSets() = false;// each face is meshed separately to find face-triangle mapping.
        PartMesherVisitor aPartMesher (aParam, aStatus);//mesh body(-ies) in each unique part
        ModelData_SceneGraphElementUniqueVisitor aVisitor (aPartMesher);
        aModel.Accept (aVisitor);

        // check to make sure that the number of triangles in stl geom matches the brep
        // This is the default in CADEX, but is not documented. Therefore check.
        if (vox.getNumTriangles() ==  aPartMesher.nTrianglesTotal) {
            cout <<"Proceeding to map triangles to faces" <<endl;
            int triangleCounter,groupCounter;
            triangleCounter = 0;
            groupCounter = 1;//start from 1?

            //cout << "nBodies " << aPartMesher.nBodies << endl;
            vox.resetGroups();
            for (int body = 0; body < aPartMesher.nBodies; body++){
                cout << "body " << body << " has " << aPartMesher.nFacesInBody[body] << " faces " << endl;
                for (int face = 0; face < aPartMesher.nFacesInBody[body]; face++){
                    for (int tri = 0; tri < aPartMesher.nTrianglesInFaceOfBody[body][face]; tri++){
                        vox.setTriGroup(triangleCounter,groupCounter);
                        triangleCounter++;
                    }
                    groupCounter++;
                }
            }
        }
        else{
            ;//writeWarningToLog(" Cannot map faces!");
        }
    }


    viewer->updateGL();
    closeDialog();

    static bool firstTime = false;
    if (firstTime){
        QMessageBox msgBox;
        msgBox.setText("Mouse and keyboard behavior\n\n"
                       "To rotate: Use left button\n"
                       "To zoom: Use middle button, or left button + control key\n"
                       "To pan: Use right button\n\n"

                       "To select surface: Use left button + alt key\n"
                       "To unselect: Use right button + alt key\n");
        msgBox.addButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowModality(Qt::NonModal);
        msgBox.show();
        int32_t userReply = msgBox.exec();
        firstTime = false;
    }
}
///////////////////////////////////////////////////////////////////////////////
void GeometryDialog::scaleGeometry()
{
    double STLArea,STLVolume;
    if (!liveVar.GEOMETRY_UPLOADED) return;

    vox.scaleSTLGeomAboutOrigin(geomScalingSpinBox->value());

    liveVar.modelScale = vox.getModelScale();

    STLVolume = vox.getVolumeOfSTLGeom(&STLArea);
    writeMessageToLog("#Triangles = %d; #Nodes = %d ",vox.getNumTriangles(),vox.getNumTriPoints());
    writeMessageToLog("STL volume = %e; STL Area = %e ",STLVolume,STLArea);

    viewer->updateGL();

}
///////////////////////////////////////////////////////////////////////////////
void GeometryDialog::saveAsOff()
{
    static QString lastSelectedDirectory;
    QString qstr = QFileDialog::getSaveFileName(this,
                                                *new QString("Select save File "),
                                                lastSelectedDirectory,
                                                *new QString( "Mesh Format (*.off )"));

    string fileName = qstr.toUtf8().constData();
    cout<<"File : " << fileName<<endl;
    vox.saveAsOff(fileName.c_str());
}
///////////////////////////////////////////////////////////////////////////////
void GeometryDialog::saveAsSTL()
{
    static QString lastSelectedDirectory;
    QString qstr = QFileDialog::getSaveFileName(this,
                                                *new QString("Select save File "),
                                                lastSelectedDirectory,
                                                *new QString( "Mesh Format (*.stl)"));

    string fileName = qstr.toUtf8().constData();
    cout<<"File : " << fileName<<endl;
    vox.saveAsSTL(fileName.c_str());
}
///////////////////////////////////////////////////////////////////////////////
void GeometryDialog :: updateSTL()
{

    if (!liveVar.GEOMETRY_UPLOADED){
        return;
    }
    QString qstr = QFileDialog::getOpenFileName(this,
                                                *new QString("Select Mesh File "),
                                                QString(""),
                                                *new QString( "Mesh Format (*.off *.obj *.stl)"));

    string fileName = qstr.toUtf8().constData();
    if (fileName.empty()){
        return;
    }
    liveVar.geomFileName[liveVar.nSTLFilesLoaded] = fileName;
    bool successfulUpdate = coreFunctions.updateGeom(liveVar.assemblyMode);

    if(successfulUpdate) {
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.displayField = DISPLAY_FIELD_NONE;
        liveVar.plotElasticityBC = true;
    }
    viewer->updateGL();

    closeDialog();

}
///////////////////////////////////////////////////////////////////////////////
void GeometryDialog :: makeConnections()
{

    //PushButton( loadSTLPushButton, [=]{loadSTL(); });
    PushButton( loadSTLPushButton, [=]{loadGeometry(); });
    PushButton( updateSTLPushButton, [=]{updateSTL(); });
    PushButton( scaleGeometryButton, [=]{scaleGeometry(); });
    PushButton( saveAsOffButton, [=]{saveAsOff(); });
    PushButton( saveAsSTLButton,[=]{saveAsSTL();});
    PushButton( closePushButton, [=]{closeDialog(); });
}

///////////////////////////////////////////////////////////////////////////////
