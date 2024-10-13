#include "TriangulateWires.hpp"
#include "ui_saveFiles.hpp"


extern TriangulateWires triwires;

extern LatticeParameters latticeParams;
/************************************************************/
int find_word(FILE *filein,const char *word)
{
    char str[256];
    fpos_t pos;
    fgetpos(filein, &pos);
    while(fscanf(filein, "%s", str) !=EOF) {
        if (strcmp(str,word) == 0)
            return 1;
    }
    // failed, return back to original position
    fsetpos(filein,&pos);
    return 0;
}

/**********************************************************************/
void dumpTriangle(double xTri[3],double yTri[3],double zTri[3],FILE *fptr)
{
    float fbuf[12];
    unsigned short att = 0;
    double triNormal[3],p01[3],p02[3],nrm;

    p01[0] = xTri[1]-xTri[0];
    p01[1] = yTri[1]-yTri[0];
    p01[2] = zTri[1]-zTri[0];

    p02[0] = xTri[2]-xTri[0];
    p02[1] = yTri[2]-yTri[0];
    p02[2] = zTri[2]-zTri[0];
    CROSS3D(triNormal,p01,p02);

    nrm = NORM3D(triNormal);
    fbuf[0] = (float) (triNormal[0]/nrm) ;
    fbuf[1] = (float) (triNormal[1]/nrm) ;
    fbuf[2] = (float) (triNormal[2]/nrm) ;

    fbuf[3]  = (float) (xTri[0]);
    fbuf[4]  = (float) (yTri[0]);
    fbuf[5]  = (float) (zTri[0]);

    fbuf[6]  = (float) (xTri[1]);
    fbuf[7]  = (float) (yTri[1]);
    fbuf[8]  = (float) (zTri[1]);

    fbuf[9]   = (float) (xTri[2]);
    fbuf[10]  = (float) (yTri[2]);
    fbuf[11]  = (float) (zTri[2]);

    fwrite(fbuf, 12*sizeof(float), 1, fptr);
    fwrite(&att,sizeof(unsigned short),1,fptr);

}

/////////////////////////////////////////////////////
SaveFilesDialog::~SaveFilesDialog()
{

}
/////////////////////////////////////////////////////
SaveFilesDialog :: SaveFilesDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/////////////////////////////////////////////////////
void SaveFilesDialog::saveLatticeGraphFileDialog()
{
    int nNodes, nBeams,nodes[2];
    double pt[3];
    static QString latticeGraphFileName,dummy ;

    static QString graphFileExtension("Graph File (*.off)") ;
    latticeGraphFileName = QString::fromStdString(liveVar.projectDirectory + "/" + liveVar.modelName + "_graph.off");
    QString qstr = QFileDialog::getSaveFileName(this,dummy,latticeGraphFileName,graphFileExtension);

    string fileName = qstr.toStdString();
    if (fileName.empty()) return;
    nNodes = triwires.getNumberOfNodes();
    nBeams = triwires.getNumberOfBeams();
    FILE *fptr = fopen(fileName.c_str(),"w");

    if (fptr == NULL){
        return;
    }
    fprintf(fptr,"OFF\n");
    fprintf(fptr,"%d 0 %d\n",nNodes,nBeams);

    for (int i = 0; i < nNodes; i++ ){
        triwires.getNodeLocation(i,pt);
        fprintf(fptr,"%.8g %.8g %.8g\n",pt[0],pt[1],pt[2]);
    }
    for (int i = 0; i < nBeams; i++ ){
        triwires.getBeamNodes(i,nodes);
        fprintf(fptr,"%d %d \n",nodes[0],nodes[1]);
    }

    fclose(fptr);
    viewer->updateGL();

}
/////////////////////////////////////////////////////
void SaveFilesDialog::saveRadiusFileDialog()
{
    int nNodes, nBeams,nodes[2];
    double pt[3];
    static QString radiusFileName,dummy ;

    static QString radiusFileExtension("Radius File (*.txt)") ;
    radiusFileName = QString::fromStdString(liveVar.projectDirectory + "/" + liveVar.modelName + "_radius.txt");
    QString qstr = QFileDialog::getSaveFileName(this,dummy,radiusFileName,radiusFileExtension);

    string fileName = qstr.toStdString();
    if (fileName.empty()) return;
    nBeams = triwires.getNumberOfBeams();
    FILE *fptr = fopen(fileName.c_str(),"w");

    if (fptr == NULL)
        return;

    fprintf(fptr,"%d\n",nBeams);
    for (int i = 0; i < nBeams; i++ )
        fprintf(fptr,"%.8g \n", triwires.getBeamRadius(i));


    fclose(fptr);
    viewer->updateGL();

}

/////////////////////////////////////////////////////
void SaveFilesDialog::saveSolidLatticeFileDialog()
{
    int nNodes, nBeams,nodes[2];
    unsigned int nTriangles;
    double pt[3],xTri[3],yTri[3],zTri[3];
    static QString solidLatticeFileName,dummy ;

    static QString solidLatticeFileExtension("Solid Lattice File (*.stl)") ;
    solidLatticeFileName = QString::fromStdString(liveVar.projectDirectory + "/" + liveVar.modelName + "_solidLattice.stl");
    QString qstr = QFileDialog::getSaveFileName(this,dummy,solidLatticeFileName,solidLatticeFileExtension);

    string fileName = qstr.toStdString();
    if (fileName.empty()) return;
    nBeams = triwires.getNumberOfBeams();
    FILE *fptr = fopen(fileName.c_str(),"wb");

    if (fptr == NULL)
        return;

    char header[80];
    memset(header,'0', 80*sizeof( char));// dummy header
    fwrite(header,80,1, fptr);
    nTriangles = triwires.getNumberOfTriangles();
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
    for (int tri = 0; tri < triwires.getNumberOfTriangles(); tri++) {
        triwires.getTriPoints(tri,xTri,yTri,zTri);
        dumpTriangle(xTri,yTri,zTri,fptr);
    }

    fclose(fptr);

    viewer->updateGL();

}
/////////////////////////////////////////////////////
void SaveFilesDialog::makeConnections(){

    PushButton( saveGraphFilePushButton, [=]{ saveLatticeGraphFileDialog();});
    PushButton( saveRadiusFilePushButton, [=]{ saveRadiusFileDialog();});
    PushButton( saveSolidLatticePushButton, [=]{ saveSolidLatticeFileDialog();});

}
