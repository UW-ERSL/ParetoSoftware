#include "TriangulateWires.hpp"
#include "ui_solidLatticeDialog.hpp"

extern TriangulateWires triwires;
extern LatticeParameters latticeParams;
extern Voxelizer voxTemp;
extern bool getYesNoResponse(const char format[],...);

/////////////////////////////////////////////////////
LatticeRadiiDialog::LatticeRadiiDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/////////////////////////////////////////////////////
LatticeRadiiDialog::~LatticeRadiiDialog()
{

}
/////////////////////////////////////////////////////
void LatticeRadiiDialog::showEvent(QShowEvent *e)
{
    double minBeamLength;
    minBeamLength = triwires.getMinEdgeLength();
    averageRadius->setText(QString::number(minBeamLength/10));
}
/////////////////////////////////////////////////////
void LatticeRadiiDialog::loadLatticeRadiiFileDialog()
{
    QString qstr2 = QFileDialog::getOpenFileName(this,
                                                 *new QString("Select Graph Radius File "),
                                                 QString(""),
                                                 *new QString( "Radius File (*.*)"));

    string latticeRadiusFile = qstr2.toUtf8().constData();
    // read the radius file
    ifstream ifile(latticeRadiusFile, ios::in);
    if( ifile.fail() ) {
        cout << "Error: Can't open input file " << latticeRadiusFile << endl;
        return;
    }
    int  numEdgesInput;
    double radius;
    vector<double> beamRadii;
    beamRadii.resize(triwires.getNumberOfBeams());
    ifile >> numEdgesInput; // how many need to be reset
    triwires.setBeamRadius(1.0);//default for memory creation
    for( size_t i = 0; i < numEdgesInput; i++) {
        ifile >>radius;
        triwires.setBeamRadius(i,radius);
        beamRadii[i] = radius;
    }
    ifile.close();
}
/////////////////////////////////////////////////////
void LatticeRadiiDialog::createSolidLattice1()
{
    double avgRadius, radius,minBeamLength,beamCenter[3];
    double boundingBox[3][2],modelCenter[3],L[3];
    int nBeams;
    int xGradient,yGradient,zGradient;

    int circularDivision;
    circularDivision = circularDivision1SpinBox->value();
    triwires.setPolySides(circularDivision);

    minBeamLength = triwires.getMinEdgeLength();
    averageRadius->setValidator( new QDoubleValidator(this) );
    avgRadius = averageRadius->text().toDouble();

    triwires.setBeamRadius(avgRadius);// initialize for memory
    xGradient = xGradientSpinBox->value();
    yGradient = yGradientSpinBox->value();
    zGradient = zGradientSpinBox->value();

    triwires.getBoundingBox(boundingBox);
    for (int i =0; i < 3; i++){
        L[i] = (boundingBox[i][1] - boundingBox[i][0])/2;
        modelCenter[i] = (boundingBox[i][1] + boundingBox[i][0])/2;
    }
    nBeams = triwires.getNumberOfBeams();
    for( size_t i = 0; i < nBeams; i++) {
        triwires.getBeamCenter(i,beamCenter);
        radius = avgRadius;
        radius += (xGradient/100.0)*(beamCenter[0]-modelCenter[0])*avgRadius/L[0];
        radius += (yGradient/100.0)*(beamCenter[1]-modelCenter[1])*avgRadius/L[1];
        radius += (zGradient/100.0)*(beamCenter[2]-modelCenter[2])*avgRadius/L[2];
        triwires.setBeamRadius(i,radius);
    }


    triwires.setUnion(true);
    triwires.triangulate();
    cout <<"Done creating 3D lattice geometry " << endl;
    writeMessageToLog("#Triangles: %d", triwires.getNumberOfTriangles());

    liveVar.displayGeom = DISPLAY_GEOM_OTHER;// lattice loaded via voxTemp
    liveVar.displayField = DISPLAY_FIELD_NONE;
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void LatticeRadiiDialog::createSolidLattice2()
{
    int circularDivision;
    circularDivision = circularDivision2SpinBox->value();
    triwires.setPolySides(circularDivision);
    triwires.setUnion(true);
    triwires.triangulate();

    writeMessageToLog("#Triangles: %d", triwires.getNumberOfTriangles());
    liveVar.displayGeom = DISPLAY_GEOM_OTHER;// lattice loaded via voxTemp
    liveVar.displayField = DISPLAY_FIELD_NONE;
    viewer->updateGL();
}

/////////////////////////////////////////////////////
void LatticeRadiiDialog::closeLatticeRadiiDialog()
{
    viewer->updateGL();
    parentWidget()->show();
    close();
}
/**********************************************************/
void LatticeRadiiDialog::makeConnections()
{
    PushButton(loadLatticeRadiiPushButton,[=]{loadLatticeRadiiFileDialog();});
    PushButton(createLatticePushButton1,[=]{createSolidLattice1();});
    PushButton(createLatticePushButton2,[=]{createSolidLattice2();});
    PushButton(closePushButton,[=]{closeLatticeRadiiDialog();});

}
/////////////////////////////////////////////////////
