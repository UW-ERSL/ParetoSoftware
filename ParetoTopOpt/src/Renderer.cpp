#include "Renderer.hpp"
#include "CoreFunctions.hpp"
 
using namespace std;

extern LiveVariables liveVar;// liveVariables
extern ParetoApplication topopt;
extern Voxelizer vox;
extern Voxelizer voxTemp;
extern CoreFunctions coreFunctions;
extern void  writeMessageToLog(const char format[],...);
extern void print2DText(int x, int y, const char string[], float clr[3] = NULL);


/************************************************************/
int Renderer::Getdec(char hexchar)
{
    if ((hexchar >= '0') && (hexchar <= '9')) return hexchar - '0';
    if ((hexchar >= 'A') && (hexchar <= 'F')) return hexchar - 'A' + 10;
    if ((hexchar >= 'a') && (hexchar <= 'f')) return hexchar - 'a' + 10;

    cout << "Incorrect character" << endl;
    return -1; // Wrong character
}
/************************************************************/
void Renderer::HexColorToFloat(const char* HexColor, float* FloatColor)
{
    const char* HexColorPtr = HexColor;

    for (int i = 0; i < 3; i++) {
        int IntColor = (Getdec(HexColorPtr[0]) * 16) + Getdec(HexColorPtr[1]);
        FloatColor[i] = (float) IntColor / 255.0;
        HexColorPtr += 2;
    }
}
/************************************************************/
void Renderer::getColorOfPart(int part,GLfloat color[4])
{
    const char *(colors[128]) = {   "6495ED","FA3F17","327F50","772600","D790FF","0A3749","1CE6FF",
                                    "E0962A","0F926A","D157A0","32DBE5","C79B04","806C66","2228AA",
                                    "886F4C","456D75","B77B68","7A87A1","334D66","885578","FAD09F",
                                    "328A9A","32F69F","456648","0086ED","34362D","B4A8BD","00A6AA",
                                    "938A81","575329","00FECF","B05B6F","8CD0FF","3B9700","04AA57",
                                    "C8A1A1","1E6E00","A77500","6367A9","6B002C","CB7E98","549E79",
                                    "7A4900","201625","72418F","BC23FF","99ADC0","3A2465","922329",
                                    "5B4534","FDE8DC","404E55","0089A3","CB7E98","324E72","6A3A4C",
                                    "BF5650","E83000","66796D","321A59","8ADBB4","1E0200","5B4E51",
                                    "32FF00","C895C5","320033","326832","66E1D3","CFCDAC","D0AC94",
                                    "7ED379","012C58","4466AA","32C459","329000","A4E804","4A3B53",
                                    "A05837","324A46","7900D7","223941","006FA6","04F757","63FFAC",
                                    "B79762","004D43","8FB0FF","997D87","5A0007","809693","FEFFE6",
                                    "1B4400","4FC601","3B5DFF","012338","322F80","61615A","BA0900",
                                    "6B7900","00C2A0","32AA92","B903AA","D16100","A30059","DDEFFF",
                                    "000035","7B4F4B","A1C299","300018","0AA6D8","013349","00846F",
                                    "372101","32B500","C2FFED","A079BF","CC0744","C0B9B2","C2FF99",
                                    "001E09","00489C","6F0062","0CBD66","202E09","003391","6FF022",
                                    "ACBDA6","AC2D6A"};

    part = MAX(0,part);
    part = (part%128);

    HexColorToFloat(colors[part],color);
    color[0] = MIN(0.65,color[0]);//avoid dark red since it is used for highlighting
}
/**********************************************************************/
void Renderer::getStaticFEATitle(double *fMin,double *fMax,char title[],int *maxFieldNode)
{
    double fieldMin=0,fieldMax=0,uvwMin[3],uvwMax[3];
    char tempStr[50];
    int dofPerNode=1;

    switch (liveVar.displayField) {
    case DISPLAY_FIELD_DISPLACEMENT:
        fieldMin = 0;
        fieldMax = topopt.getInitialDisplacement();//default
        if (liveVar.displayGeom == DISPLAY_GEOM_TOPOLOGY)
            fieldMax = topopt.getDeformationMax(maxFieldNode);
        switch (liveVar.paretoUnits) {
        case UNITS_METER:
            strcpy(title,"Max Deflection (meter):");
            break;
        case UNITS_MM:
            strcpy(title,"Max Deflection (mm):");
            break;
        case UNITS_INCH:
            strcpy(title,"Max Deflection (inch):");
            break;
        }
        if ((fabs(fieldMax) < 1000.0) && (fabs(fieldMax) > 0.001))
            sprintf(tempStr," %.3f\0",fieldMax);
        else
            sprintf(tempStr," %.3e\0",fieldMax);
        strcat(title,tempStr);
        dofPerNode = 3;
        break;
    case DISPLAY_FIELD_DISPLACEMENT_X:
    case DISPLAY_FIELD_DISPLACEMENT_Y:
    case DISPLAY_FIELD_DISPLACEMENT_Z:
        topopt.getUVWMinMax(uvwMin,uvwMax);
        if (liveVar.displayField == DISPLAY_FIELD_DISPLACEMENT_X){
            fieldMin = uvwMin[0];
            fieldMax = uvwMax[0];
        }
        if (liveVar.displayField == DISPLAY_FIELD_DISPLACEMENT_Y){
            fieldMin = uvwMin[1];
            fieldMax = uvwMax[1];
        }
        if (liveVar.displayField == DISPLAY_FIELD_DISPLACEMENT_Z){
            fieldMin = uvwMin[2];
            fieldMax = uvwMax[2];
        }
        if (liveVar.displayGeom == DISPLAY_GEOM_TOPOLOGY)
            fieldMax = topopt.getDeformationMax(maxFieldNode);
        switch (liveVar.paretoUnits) {
        case UNITS_METER:
            strcpy(title,"Max Deflection (meter):");
            break;
        case UNITS_MM:
            strcpy(title,"Max Deflection (mm):");
            break;
        case UNITS_INCH:
            strcpy(title,"Max Deflection (inch):");
            break;
        }
        if ((fabs(fieldMax) < 1000.0) && (fabs(fieldMax) > 0.001))
            sprintf(tempStr," %.3f\0",fieldMax);
        else
            sprintf(tempStr," %.3e\0",fieldMax);
        strcat(title,tempStr);
        dofPerNode = 3;
        break;
    case DISPLAY_FIELD_STRESS:
        fieldMin = topopt.getMinVonMisesStress();
        fieldMax = topopt.getMaxVonMisesStress();
        if (liveVar.displayGeom == DISPLAY_GEOM_TOPOLOGY)
            fieldMax = topopt.getMaxVonMisesStress(maxFieldNode);
        switch (liveVar.paretoUnits) {
        case UNITS_METER:
            strcpy(title,"Max Stress (N/m^2):");
            break;
        case UNITS_MM:
            strcpy(title,"Max Stress (N/m^2):");
            break;
        case UNITS_INCH:
            strcpy(title,"Max Stress (psi):");
            break;
        }
        if ((fabs(fieldMax) < 1000.0) && (fabs(fieldMax) > 0.001))
            sprintf(tempStr," %.3f\0",fieldMax);
        else
            sprintf(tempStr," %.3e\0",fieldMax);
        strcat(title,tempStr);
        dofPerNode = 3;
        break;
    case DISPLAY_FIELD_TEMPERATURE:
    case DISPLAY_FIELD_MOISTURE:
        topopt.getTemperatureRange(&fieldMin,&fieldMax,maxFieldNode);
        strcpy(title,"Temperature Range: ");
        if ( (fabs(fieldMax)+fabs(fieldMin) < 1000.0) &&
             (fabs(fieldMax)+fabs(fieldMin) > 0.001) )
            sprintf(tempStr," (%.3f, %.3f)\0",fieldMin,fieldMax);
        else
            sprintf(tempStr," (%.3e, %.3e)\0",fieldMin,fieldMax);
        strcat(title,tempStr);
        break;
    case DISPLAY_FIELD_MODAL:
        fieldMin = 0;
        fieldMax = topopt.getInitialDisplacement();//default

        if ((fabs(fieldMax) < 1000.0) && (fabs(fieldMax) > 0.001))
            sprintf(tempStr," %.3f\0",fieldMax);
        else
            sprintf(tempStr," %.3e\0",fieldMax);
        strcat(title,tempStr);
        break;
    }
    *fMin = fieldMin;
    *fMax = fieldMax;
    sprintf(tempStr,"\nElements: %d\n",topopt.getNumElems());
    strcat(title,tempStr);
    sprintf(tempStr,"DOF: %d\n",dofPerNode*topopt.getNumNodes());
    strcat(title,tempStr);

}

/******************************************************************************/
void Renderer::drawAxis(void)
{
    double xyzMin[3],xyzMax[3],scale;

    scale = vox.getModelScale();
    double AxisLength = 0.07*scale;
    double startPt[]= {0,0,0},endPt[] ={0,0,0};
    float color[3];
    glEnable(GL_LIGHTING);

    vox.getBoundingBox(xyzMin,xyzMax);

    startPt[0] = xyzMin[0]-0.025*scale;
    startPt[1] = xyzMin[1]-0.025*scale;
    startPt[2] = xyzMin[2]-0.025*scale;

    endPt[0] =startPt[0]+ AxisLength;endPt[1] = startPt[1];endPt[2] = startPt[2];
    color[0] = 0.25;color[1] = 0.25;color[2] = 0.25;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    drawNiceArrow(startPt,endPt,false,false);

    endPt[0] =startPt[0];endPt[1] = startPt[1]+AxisLength;endPt[2] = startPt[2];
    color[0] = 0.25;color[1] = 0.25;color[2] = 0.25;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    drawNiceArrow(startPt,endPt,false,false);

    endPt[0] =startPt[0];endPt[1] = startPt[1];endPt[2] = startPt[2]+AxisLength;
    color[0] = 0.25;color[1] = 0.25;color[2] = 0.25;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    drawNiceArrow(startPt,endPt,false,false);

    glColor3f(color[0], color[1], color[2]);
    endPt[0] =startPt[0]+ 1.5*AxisLength;endPt[1] = startPt[1];endPt[2] = startPt[2];
    draw3DText("X",endPt,1);
    endPt[0] =startPt[0];endPt[1] = startPt[1]+ 1.5*AxisLength;endPt[2] = startPt[2];
    draw3DText("Y",endPt,1);
    endPt[0] =startPt[0];endPt[1] = startPt[1];endPt[2] = startPt[2]+ 1.5*AxisLength;
    draw3DText("Z",endPt,1);

}
/**********************************************************************/
void  Renderer::drawNormals(void)
{
    double normal[3],p0[3],p1[3],modelScale;

    glColor3f(0.1f,0.1f,0.1f);
    glDisable( GL_LIGHTING );
    modelScale = topopt.getModelScale();
    for (int node =0; node < topopt.getNumNodes(); node++) {
        if (0) {//(topopt.getNormalAtNode(node,normal)){//on boundary
            topopt.getNodeXYZ(node,p0);
            for (int i = 0; i < 3; i++)
                p1[i] = p0[i] + 0.005*modelScale*normal[i];
            glBegin(GL_LINE_LOOP);
            glVertex3f(p0[0],p0[1],p0[2]);
            glVertex3f(p1[0],p1[1],p1[2]);
            glEnd();
        }
        //else don't plot interior elements
    }
    glEnable( GL_LIGHTING );
}
/**********************************************************************/
void Renderer::drawLine(double startPt[3],double endPt[3], float lineWidth)
{
    glLineWidth(lineWidth);

    glBegin(GL_LINES);
        glVertex3d(startPt[0],startPt[1],startPt[2]);
        glVertex3d(endPt[0],endPt[1],endPt[2]);
    glEnd();
}

/**********************************************************************/
void Renderer::drawNiceArrow(double startPt[3],double endPt[3], bool withBaseCap, bool doubleArrow,double baseRadiusScale)
{
    double vec[3],length,baseRadius, dir[3],angle,lengthOfArrow,sinTheta;
    SUBTRACT3D(vec,startPt,endPt);
    length = NORM3D(vec);
    lengthOfArrow = length;
    baseRadius = baseRadiusScale*MIN(length, 0.1*vox.getModelScale());

    // we need to align the desired direction with the z axis
    //i.e., we need to find the rotation axis and the angle of rotation to align
    // rotation axis is perpendicular to direction and z axis (Cross product)
    dir[0] = vec[1]; dir[1] = -vec[0]; dir[2] = 0; // to rotate frame
    sinTheta = NORM3D(dir)/length;

    // angle is the dot product of dir and z axis
    //angle = (180.0/3.1415)*asin(nrm/length);
    angle = (180.0/3.1415)*asin(sinTheta);
    if (vec[2] > 0) angle = 180-angle;
    GLUquadric* q = gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_FILL);
    glPushMatrix();
    glTranslatef(startPt[0], startPt[1], startPt[2]);


    glRotatef(angle, dir[0], dir[1], dir[2]);// Draw the cylinder part of the arrow
    gluCylinder(q, baseRadius, baseRadius, lengthOfArrow, 10, 1);
    gluQuadricOrientation(q, GLU_INSIDE);// Normals for the cap need to be flipped
    gluDisk(q, 0,baseRadius, 10, 1);// Cap the bottom

    glTranslatef(0,0,lengthOfArrow);// Draw the cylinder part of the arrow
    gluDisk(q, 0,3*baseRadius, 10, 1);// Cap the bottom of the cone2
    gluQuadricOrientation(q, GLU_OUTSIDE);// Normals for the cap need to be flipped
    gluCylinder(q, 3*baseRadius, 0.0f, 5*baseRadius, 10, 1);// Draw the cone part of the arrow

    if (doubleArrow) {
        glTranslatef(0,0,5*baseRadius);


        gluQuadricOrientation(q, GLU_OUTSIDE);// Normals for the cap need to be flipped
        gluCylinder(q, 3*baseRadius, 0.0f, 5*baseRadius, 10, 1);// Draw the cone part of the arrow
        gluQuadricOrientation(q, GLU_INSIDE);// Normals for the cap need to be flipped
        gluCylinder(q, 3*baseRadius, 0.0f, 5*baseRadius, 10, 1);// Draw the cone part of the arrow

        gluQuadricOrientation(q, GLU_INSIDE);// Normals for the cap need to be flipped
        gluDisk(q, 3*baseRadius,0, 10, 1);// Cap the bottom of the cone
    }

    glPopMatrix();
    gluDeleteQuadric(q);
}
/**********************************************************************/
void Renderer::drawCylinder(double startPt[3],double endPt[3], double radius, double aspectRatio )
{
    double vec[3],length, dir[3],angle,sinTheta;

    SUBTRACT3D(vec,startPt,endPt);
    length = NORM3D(vec);
    // we need to align the desired direction with the z axis
    //i.e., we need to find the rotation axis and the angle of rotation to align
    // rotation axis is perpendicular to direction and z axis (Cross product)
    dir[0] = vec[1]; dir[1] = -vec[0]; dir[2] = 0; // to rotate frame
    sinTheta = NORM3D(dir)/length;

    // angle is the dot product of dir and z axis
    //angle = (180.0/3.1415)*asin(nrm/length);
    angle = (180.0/3.1415)*asin(sinTheta);
    if (vec[2] > 0) angle = 180-angle;

    drawNiceSphere(startPt,radius );
    glPushMatrix();
    glTranslatef(startPt[0], startPt[1], startPt[2]);
    glRotatef(angle, dir[0], dir[1], dir[2]);
    int nDivisions = 16;
    double step = 2.0 * M_PI / nDivisions;
    glBegin(GL_TRIANGLE_STRIP);
    for (int j = 0; j <= nDivisions; ++j) {
        double a = j * step;
        GLfloat x = aspectRatio*radius * cos(a);
        GLfloat y = radius * sin(a);
        glNormal3f(x / radius, y / radius, 0.0);
        glTexCoord2f(j / (GLfloat) step, 0);
        glVertex3f(x, y, 0);
        glNormal3f(x / radius, y / radius, 0.0);
        glTexCoord2f(j / (GLfloat) step, 1.0f);
        glVertex3f(x, y, length);
    }
    glEnd();
    glPopMatrix();
    drawNiceSphere(endPt,radius );
}
/**********************************************************************/
void Renderer::drawHollowCylinder(double startPt[3],double endPt[3] )
{
    double vec[3],length, dir[3],radiusOuter, radiusInner,angle,lengthOfArrow,sinTheta;

    SUBTRACT3D(vec,startPt,endPt);
    length = NORM3D(vec);
    lengthOfArrow = length;
    radiusOuter = 0.05*length;
    radiusInner = 0.5*radiusOuter;

    // we need to align the desired direction with the z axis
    //i.e., we need to find the rotation axis and the angle of rotation to align
    // rotation axis is perpendicular to direction and z axis (Cross product)
    dir[0] = vec[1]; dir[1] = -vec[0]; dir[2] = 0; // to rotate frame
    sinTheta = NORM3D(dir)/length;

    // angle is the dot product of dir and z axis
    //angle = (180.0/3.1415)*asin(nrm/length);
    angle = (180.0/3.1415)*asin(sinTheta);
    if (vec[2] > 0) angle = 180-angle;
    GLUquadric* q = gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_FILL);
    glPushMatrix();
    glTranslatef(startPt[0], startPt[1], startPt[2]);


    glRotatef(angle, dir[0], dir[1], dir[2]);// Draw the cylinder part of the arrow

    gluQuadricOrientation(q, GLU_INSIDE);// Normals for the cap need to be flipped
    gluCylinder(q, radiusOuter, radiusOuter, lengthOfArrow, 10, 1);
    gluQuadricOrientation(q, GLU_OUTSIDE);// Normals for the cap need to be flipped
    gluCylinder(q, radiusOuter, radiusOuter, lengthOfArrow, 10, 1);


    gluCylinder(q, radiusInner, radiusInner, lengthOfArrow, 10, 1);

    gluQuadricOrientation(q, GLU_OUTSIDE);// Normals for the cap need to be flipped
    gluDisk(q, radiusInner,radiusOuter, 10, 1);// Cap the bottom
    gluQuadricOrientation(q, GLU_INSIDE);// Normals for the cap need to be flipped
    gluDisk(q, radiusInner,radiusOuter, 10, 1);// Cap the bottom


    glTranslatef(0,0,lengthOfArrow);// Draw the cylinder part of the arrow
    gluQuadricOrientation(q, GLU_INSIDE);// Normals for the cap need to be flipped
    gluDisk(q, radiusInner,radiusOuter, 10, 1);// Cap the bottom of the cone2
    gluQuadricOrientation(q, GLU_OUTSIDE);// Normals for the cap need to be flipped
    gluDisk(q, radiusInner,radiusOuter, 10, 1);// Cap the bottom of the cone2

    glPopMatrix();
    gluDeleteQuadric(q);
}
/**********************************************************************/
void Renderer::drawNiceCone(double startPt[3],double dir[3],double height,double baseRadius )
{
    double  angle,axisDir[3];


    axisDir[0] = dir[1];
    axisDir[1] = -dir[0];
    axisDir[2] = 0;
    // angle is the dot product of dir and z axis
    angle = (180.0/3.141592)*asin(sqrt(dir[0]*dir[0] + dir[1]*dir[1]));
    if (dir[2] > 0) angle = 180-angle;
    GLUquadric* q = gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_FILL);
    glPushMatrix();

    glTranslatef(startPt[0], startPt[1], startPt[2]);
    glRotatef(angle, axisDir[0], axisDir[1], axisDir[2]);
    gluQuadricOrientation(q, GLU_INSIDE);// Normals for the cap need to be flipped
    gluDisk(q, 0,baseRadius, 20, 1);// Cap the bottom of the cone
    gluQuadricOrientation(q, GLU_OUTSIDE);// Normals for the cap need to be flipped
    gluCylinder(q,baseRadius, 0.0f, height, 20, 1);// Draw the cone part of the arrow

    glPopMatrix();
    gluDeleteQuadric(q);
}
/************************************************************/
void Renderer::draw3DText(const char *text,double pos[3],double scale)
{
    float _scale = scale*0.03*liveVar.modelScale;
    glPushMatrix();
    glTranslated(pos[0],pos[1],pos[2]);
    glScalef(_scale, _scale, _scale);
    t3dDraw3D(text, -1, 0, 0.2f);
    glPopMatrix();

}
/**********************************************************************/
void Renderer::drawNiceSphere(double centerPt[],double radius )
{
    GLUquadric* q = gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_FILL);
    gluQuadricTexture(q,GLU_TRUE);
    glPushMatrix();
    glTranslatef(centerPt[0], centerPt[1], centerPt[2]);
    gluSphere(q,radius, 20, 20);
    glPopMatrix();
    gluDeleteQuadric(q);
}
void Renderer::plotColorBars(void)
{
    int pareto_viewport[4],width,height,startX,startY,yOffset,nDivisions,partNumWithLowestYield;
    double minValue,maxValue,yieldStress,uvwMin[3],uvwMax[3];
    float color[4],temp;
    char minStr[256],maxStr[256],meanStr[256],title[256],yieldStressStr[256];


    string tempUnitsName;
    coreFunctions.getPropertyUnits(PROPERTY_TEMPERATURE,tempUnitsName);

    color[3] = 0.6;
    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity();

    glGetIntegerv (GL_VIEWPORT, pareto_viewport);
    gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
    glDepthFunc (GL_ALWAYS);

    width = 40;
    height = 2;
    startX = pareto_viewport[2]- 5*width;
    startY = 0.85*pareto_viewport[3];
#ifdef PARETOCREO
	startY = 0.8*pareto_viewport[3];
#endif

    nDivisions = min(400.0,0.6*pareto_viewport[3]/height);
    if (nDivisions%2 == 1) nDivisions++;

    sprintf(yieldStressStr,"");
    yieldStress = HUGE_VAL;
    partNumWithLowestYield = 0;
    for (int m = 0; m < topopt.getNumMaterials(); m++){
        double tempYield;
        topopt.getYieldStressOfMaterial(m,&tempYield);
        if (tempYield < yieldStress) {
            yieldStress = tempYield;
            partNumWithLowestYield = m;
        }
    }
    switch (liveVar.displayField) {
    case DISPLAY_FIELD_DISPLACEMENT:
    case DISPLAY_FIELD_DISPLACEMENT_X:
    case DISPLAY_FIELD_DISPLACEMENT_Y:
    case DISPLAY_FIELD_DISPLACEMENT_Z:
    case DISPLAY_FIELD_MODAL:
        minValue = 0;
        maxValue = topopt.getInitialDisplacement();//default
        topopt.getUVWMinMax(uvwMin,uvwMax);

        if (liveVar.displayGeom == DISPLAY_GEOM_TOPOLOGY)
            maxValue = topopt.getDeformationMax();
        switch (liveVar.paretoUnits) {
        case UNITS_METER:
            strcpy(title,"Deflection (m) ");
            break;
        case UNITS_MM:
            strcpy(title,"Deflection (mm) ");
            break;
        case UNITS_INCH:
            strcpy(title,"Deflection (in) ");
            break;
        }
        if (liveVar.displayField == DISPLAY_FIELD_DISPLACEMENT_X){
            minValue = uvwMin[0];
            maxValue = uvwMax[0];
            strcat(title,"in X");
        }
        else if (liveVar.displayField == DISPLAY_FIELD_DISPLACEMENT_Y){
            minValue = uvwMin[1];
            maxValue = uvwMax[1];
            strcat(title,"in Y");
        }
        else if (liveVar.displayField == DISPLAY_FIELD_DISPLACEMENT_Z){
            minValue = uvwMin[2];
            maxValue = uvwMax[2];
            strcat(title,"in Z");
        }

        break;
    case DISPLAY_FIELD_STRESS:
        minValue = topopt.getMinVonMisesStress();
        maxValue = topopt.getInitialVonMisesStress();
        if (liveVar.displayGeom == DISPLAY_GEOM_TOPOLOGY)
            maxValue = topopt.getMaxVonMisesStress();
        switch (liveVar.paretoUnits) {
        case UNITS_METER:
            strcpy(title,"vonMises (N/m^2) ");
            sprintf(yieldStressStr,"Yield (part %d): %.2e (N/m^2)",partNumWithLowestYield,yieldStress);
            break;
        case UNITS_MM:
            strcpy(title,"vonMises (N/mm^2) ");
            sprintf(yieldStressStr,"Yield (part %d): %.2e (N/mm^2)",partNumWithLowestYield,yieldStress);
            break;
        case UNITS_INCH:
            strcpy(title,"vonMises (psi) ");
            sprintf(yieldStressStr,"Yield (part %d): %.2e (psi)",partNumWithLowestYield,yieldStress);
            break;
        }
        break;
    case DISPLAY_FIELD_TEMPERATURE:
        topopt.getTemperatureRange(&minValue,&maxValue);
        strcpy(title,"Temperature ");
        strcat(title,tempUnitsName.c_str());
        break;
    case DISPLAY_FIELD_MOISTURE:
        topopt.getTemperatureRange(&minValue,&maxValue);
        strcpy(title,"Moisture(%)");
        break;

    }
    print2DText(startX-0.8*width, startY-20*height-height*nDivisions, title);
	int colorBarTop;

	colorBarTop = startY - 20 * height - height*nDivisions + 6*height;

	colorBarTop += nDivisions*height;
    if (liveVar.displayField == DISPLAY_FIELD_TEMPERATURE){
        sprintf(minStr,"%.1f",minValue);
        sprintf(maxStr,"%.1f",maxValue);
        sprintf(meanStr,"%.1f",(minValue+maxValue)/2.0);
    }
    else {
        sprintf(minStr,"%.3g",minValue);
        sprintf(maxStr,"%.3g",maxValue);
        sprintf(meanStr,"%.3g",(minValue+maxValue)/2.0);
    }

    for (int bar = 0; bar < nDivisions; bar++) {
        //bar = 0, corresponds to blue color
        if (liveVar.displayField == DISPLAY_FIELD_MOISTURE)
            temp =  1.0*(bar)/(nDivisions-1);
        else
            temp =  1- 1.0*(bar)/(nDivisions-1);
        getInterpolatedColor(temp, color);
        yOffset = bar*height;
        glNormal3d(0, 0, 1);
#ifdef PARETOCREO
        glColor3fv(color);
		glBegin(GL_QUADS);
		glVertex2d(startX, colorBarTop-yOffset);
		glVertex2d(startX, colorBarTop - yOffset - height);
		glVertex2d(startX + width, colorBarTop-yOffset- height);
		glVertex2d(startX + width, colorBarTop - yOffset);
		glEnd();

		if (bar == 0)
			print2DText(startX + 1.1*width, colorBarTop - yOffset + 24*height, minStr);
		else if (2 * bar == nDivisions)
			print2DText(startX + 1.1*width, colorBarTop - yOffset + 24*height, meanStr);
		else if (bar == nDivisions - 1)
			print2DText(startX + 1.1*width, colorBarTop - yOffset + 24*height, maxStr);
#else
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        glBegin(GL_QUADS);
        glVertex2d(startX,startY-yOffset-30*height);
        glVertex2d(startX,startY-height-yOffset-30*height);
        glVertex2d(startX+width,startY-height-yOffset-30*height);
        glVertex2d(startX+width,startY-yOffset-30*height);
        glEnd();
		if (bar == 0)
			print2DText(startX + 1.1*width, startY - yOffset - 0.5*height, minStr);
		else if (2 * bar == nDivisions)
			print2DText(startX + 1.1*width, startY - yOffset - 0.5*height, meanStr);
		else if (bar == nDivisions - 1)
			print2DText(startX + 1.1*width, startY - yOffset - 0.5*height, maxStr);
#endif
       
    }

    glDepthFunc (GL_LESS);
    glPopMatrix ();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix ();
}

/**********************************************************************/
void Renderer::plotEigenModeOverTriangle(double x[3], double y[3], double z[3],double u[3],double v[3],double w[3],
double scale,double triNormal[3],double fieldMin,double fieldMax)
{
    int i;
    double *fieldToPlot,temp;
    GLfloat color[4] ={1.0, 0.0, 0.0, 1.0};
    double delta[3],xTri[3],yTri[3],zTri[3];

    for (i = 0; i < 3; i++) {
        xTri[i] = x[i] + scale*u[i];
        yTri[i] = y[i] + scale*v[i];
        zTri[i] = z[i] +  scale*w[i];
        delta[i] = sqrt(u[i]*u[i] + v[i]*v[i]+ w[i]*w[i]);
    }
    fieldToPlot = &(delta[0]);

    glBegin(GL_TRIANGLES);
    temp = pow((1.1*fieldToPlot[0]-fieldMin)/(fieldMax-fieldMin),1);
    getInterpolatedColor(temp, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
    glVertex3f(xTri[0],yTri[0],zTri[0]);

    temp = pow((1.1*fieldToPlot[1]-fieldMin)/(fieldMax-fieldMin),1);
    getInterpolatedColor(temp, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
    glVertex3f(xTri[1],yTri[1],zTri[1]);

    temp = pow((1.1*fieldToPlot[2]-fieldMin)/(fieldMax-fieldMin),1);
    getInterpolatedColor(temp, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
    glVertex3f(xTri[2],yTri[2],zTri[2]);

    glEnd();
}
/**********************************************************************/
void Renderer::plotEigenModeOverTriangleRecursive(double x[3],double y[3],double z[3],double scale,double u[3],double v[3],double w[3],
double triNormal[3],double fieldMin,double fieldMax, double edgeLengthDesired,int currentDepth)
{
    // if currentDepth = 0, plot
    int i;
    int modeToDisplay = min(liveVar.nEigenModes,liveVar.displayEigenMode)-1;//offset by 1
    if (currentDepth <= 0) { //plot
        plotEigenModeOverTriangle(x,y,z,u,v,w,scale,triNormal, fieldMin, fieldMax);
        return;
    }

    // find the edge with the most variation
    double LMax = -1e8,L;
    int nodes[]= {0,1,2,0},n1,n2,index;
    for (i = 0; i < 3;i++) {
        n1 = nodes[i];
        n2 = nodes[i+1];
        L = sqrt ( pow((x[n2]-x[n1]),2.0) + pow((y[n2]-y[n1]),2.0) +pow((z[n2]-z[n1]),2.0) );
        if (L > LMax) {
            index = i;
            LMax = L;
        }
    }

    if (LMax < edgeLengthDesired) {
        plotEigenModeOverTriangle(x,y,z,u,v,w,scale,triNormal, fieldMin, fieldMax);
        return;
    }

    // Split this edge and call self twice, with depth reduced
    double x1[3],y1[3],z1[3],x2[3],y2[3],z2[3];
    double u1[3],v1[3],w1[3],u2[3],v2[3],w2[3];
    double un,vn,wn;
    int voxnode;

    switch(index) {
    case 0:
        x1[0] = x[0];         y1[0] = y[0];          z1[0] = z[0];
        x1[1] = (x[0]+x[1])/2;y1[1] = (y[0]+y[1])/2; z1[1] = (z[0]+z[1])/2;
        x1[2] = x[2];         y1[2] = y[2];          z1[2] = z[2];

        x2[0] = x1[1];y2[0] = y1[1];z2[0] = z1[1];
        x2[1] = x[1]; y2[1] = y[1]; z2[1] = z[1];
        x2[2] = x[2]; y2[2] = y[2]; z2[2] = z[2];

        //get the voxel node closest to the mid-point point
        vox.closestVoxelNodeToPoint(x1[1],y1[1],z1[1],&(voxnode));

        if ( voxnode == -1) {
            return;// error!!!
        }
        // get the field values at this node
        un = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxnode];
        vn = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxnode+1];
        wn = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxnode+2];

        //recycle old values wherever you can
        u1[0] = u[0]; v1[0] = v[0]; w1[0] = w[0];
        u1[1] = un;   v1[1] = vn;   w1[1] = wn;
        u1[2] = u[2]; v1[2] = v[2]; w1[2] = w[2];

        u2[0] = un;   v2[0] = vn;   w2[0] = wn;
        u2[1] = u[1]; v2[1] = v[1]; w2[1] = w[1];
        u2[2] = u[2]; v2[2] = v[2]; w2[2] = w[2];

        break;
    case 1:
        x1[0] = x[1];         y1[0] = y[1];         z1[0] = z[1];
        x1[1] = (x[1]+x[2])/2;y1[1] = (y[1]+y[2])/2;z1[1] = (z[1]+z[2])/2;
        x1[2] = x[0];         y1[2] = y[0];         z1[2] = z[0];

        x2[0] = x1[1];y2[0] = y1[1];z2[0] = z1[1];
        x2[1] = x[2]; y2[1] = y[2]; z2[1] = z[2];
        x2[2] = x[0]; y2[2] = y[0]; z2[2] = z[0];

        //get the voxel node closest to the mid-point point
        vox.closestVoxelNodeToPoint(x1[1],y1[1],z1[1],&(voxnode));
        if ( voxnode == -1) {
            return;// error!!!
        }

        // get the field values at this node
        un = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxnode];
        vn = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxnode+1];
        wn = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxnode+2];


        //recycle old values wherever you can
        u1[0] = u[1]; v1[0] = v[1]; w1[0] = w[1];
        u1[1] = un;   v1[1] = vn;   w1[1] = wn;
        u1[2] = u[0]; v1[2] = v[0]; w1[2] = w[0];

        u2[0] = un;   v2[0] = vn;   w2[0] = wn;
        u2[1] = u[2]; v2[1] = v[2]; w2[1] = w[2];
        u2[2] = u[0]; v2[2] = v[0]; w2[2] = w[0];

        break;
    case 2:
        x1[0] = x[2];         y1[0] = y[2];         z1[0] = z[2];
        x1[1] = (x[2]+x[0])/2;y1[1] = (y[2]+y[0])/2;z1[1] = (z[2]+z[0])/2;
        x1[2] = x[1];         y1[2] = y[1];         z1[2] = z[1];

        x2[0] = x1[1];y2[0] = y1[1];z2[0] = z1[1];
        x2[1] = x[0]; y2[1] = y[0]; z2[1] = z[0];
        x2[2] = x[1]; y2[2] = y[1]; z2[2] = z[1];

        //get the voxel node closest to the mid-point point
        vox.closestVoxelNodeToPoint(x1[1],y1[1],z1[1],&(voxnode));
        if ( voxnode == -1) {
            return;// error!!!
        }

        // get the field values at this node
        un = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxnode];
        vn = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxnode+1];
        wn = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxnode+2];

        //recycle old values wherever you can
        u1[0] = u[2]; v1[0] = v[2]; w1[0] = w[2];
        u1[1] = un;   v1[1] = vn;   w1[1] = wn;
        u1[2] = u[1]; v1[2] = v[1]; w1[2] = w[1];

        u2[0] = un;   v2[0] = vn;   w2[0] = wn;
        u2[1] = u[0]; v2[1] = v[0]; w2[1] = w[0];
        u2[2] = u[1]; v2[2] = v[1]; w2[2] = w[1];

        break;
    }
    plotEigenModeOverTriangleRecursive(x1,y1,z1,scale,u1,v1,w1,triNormal,fieldMin,fieldMax,edgeLengthDesired,currentDepth-1);
    plotEigenModeOverTriangleRecursive(x2,y2,z2,scale,u2,v2,w2,triNormal,fieldMin,fieldMax,edgeLengthDesired,currentDepth-1);
}
/**********************************************************************/
void Renderer::displayStatusMessage(void)
{
	int pareto_viewport[4], startX, startY, deltaY;
	char str[256], units[256], materialName[256];
	double materialProperties[7];
	GLfloat clr[4];

#ifndef PARETOWORKS
	return;
#endif
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glGetIntegerv(GL_VIEWPORT, pareto_viewport);
	gluOrtho2D(0, pareto_viewport[2], 0, pareto_viewport[3]);
	glDepthFunc(GL_ALWAYS);

	print2DText(ceil((150+ pareto_viewport[2])/2.0), pareto_viewport[3]-30, liveVar.statusMessage.c_str());

	glDepthFunc(GL_LESS);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

/**********************************************************************/
void Renderer::plotMeshComponentData(void)
{
    int pareto_viewport[4], startX, startY,deltaY;
    char str[256],units[256],materialName[256];
    double materialProperties[7];
    GLfloat clr[4];

    if (!liveVar.plotSummaryResults) return;
    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity();

    glGetIntegerv (GL_VIEWPORT, pareto_viewport);
    gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
    glDepthFunc (GL_ALWAYS);



    deltaY = 30;// font size dependent
    startX = 20;
    startY = deltaY;

#ifdef PARETOWORKS
    startX += 300;
    startY += 80;
    deltaY = 20;
#endif

    startY += deltaY;
    for (int component = 0; component < topopt.getNumberOfMeshComponents(); component++){
        getColorOfPart(component,clr);
        sprintf(str,"Component %d",component);
        print2DText(startX, startY,str,clr);
        startY += deltaY;
    }
    glDepthFunc (GL_LESS);
    glPopMatrix ();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix ();
}
/**********************************************************************/
void Renderer::plotGeomData(void)
{
    int pareto_viewport[4], startX, startY,deltaY;
    char str[256],units[256],materialName[256],volUnits[256];
    double materialProperties[7],area;
    GLfloat clr[4];

    if (!liveVar.plotSummaryResults) return;
    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity();

    glGetIntegerv (GL_VIEWPORT, pareto_viewport);
    gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
    glDepthFunc (GL_ALWAYS);

    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        strcpy(units,"(meter)");
        strcpy(volUnits,"(m^3)");
        break;
    case UNITS_MM:
        strcpy(units,"(mm)");
        strcpy(volUnits,"(mm^3)");
        break;
    case UNITS_INCH:
        strcpy(units,"(inch):");
        strcpy(volUnits,"(in^3)");
        break;
    }

    deltaY = 30;// font size dependent
    startX = 20;
    startY = deltaY;

#ifdef PARETOWORKS
    startX += 300;
    startY += 80;
	deltaY = 20;
#endif

    sprintf(str,"Model: %s ",liveVar.modelName.c_str());
    print2DText(startX, startY,str);
    startY += deltaY;
    sprintf(str,"Length: %.2f %s",0.5*liveVar.modelScale,units);// the modelscale is twice the overall length
    print2DText(startX, startY,str);
    startY += deltaY;
    sprintf(str,"Volume: %.2e %s",0.5*vox.getVolumeOfSTLGeom(&area),volUnits);print2DText(startX, startY,str);

    startY += deltaY;
    for (int part = 0; part < vox.getNumParts(); part++){
        getColorOfPart(part,clr);
        topopt.getMaterialNameOfPart(part,materialName);
        topopt.getMaterialPropertiesOfPart(part,materialProperties,materialName);
        if (topopt.shouldPartBeOptimized(part))
            sprintf(str,"Part %d: %s",part,materialName);
        else
            sprintf(str,"Part %d*: %s",part,materialName);
        print2DText(startX, startY,str,clr);
        startY += deltaY;
    }

    sprintf(str,"#LoadSets: %d",liveVar.nBCSet);
    print2DText(startX, startY,str);

    glDepthFunc (GL_LESS);
    glPopMatrix ();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix ();
}
/**********************************************************************/
void Renderer::plotMeshData(void)
{
    int pareto_viewport[4], startX, startY,deltaY;
    char str[256],materialName[256];
    double materialProperties[7];
    GLfloat clr[4];

    if (!liveVar.plotSummaryResults) return;
    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity();

    glGetIntegerv (GL_VIEWPORT, pareto_viewport);
    gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
    glDepthFunc (GL_ALWAYS);

    deltaY = 30;// font size dependent
    startX = 20;
    startY = deltaY;

#ifdef PARETOWORKS
    startX += 300;
    startY += 80;
    deltaY = 20;
#endif

    sprintf(str,"Model: %s ",liveVar.modelName.c_str());
    print2DText(startX, startY,str);
    startY += deltaY;

    sprintf(str,"#Elements: %d ",topopt.getNumElems());
    print2DText(startX, startY,str);
    startY += deltaY;

    sprintf(str,"#Nodes: %d ",topopt.getNumNodes());
    print2DText(startX, startY,str);
    startY += deltaY;

    startY += deltaY;
    for (int part = 0; part < topopt.getNumberOfParts(); part++){
        getColorOfPart(part,clr);
        topopt.getMaterialNameOfPart(part,materialName);
        topopt.getMaterialPropertiesOfPart(part,materialProperties,materialName);
        sprintf(str,"Part %d: %s",part,materialName);

        print2DText(startX, startY,str,clr);
        startY += deltaY;
    }

    glDepthFunc (GL_LESS);
    glPopMatrix ();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix ();
}

/**********************************************************************/
void Renderer::plotContoursFromAutoDesign(void)
{
    int nContours,nComponents,nPts;
    double pt[3];
    GLfloat clr[4];

    nComponents = topopt.getNumberOfMeshComponents();
    if (nComponents == 0) return;

    glDisable( GL_LIGHTING );
    glLineWidth(2.5);

    for (int component = 0; component < nComponents; component++){
        nContours = topopt.getNumberOfContoursInComponent(component);
        for (int contour = 0; contour < nContours; contour++) {
            nPts = topopt.getNumberOfPointsOnContour(component,contour);
            getColorOfPart(component,clr);
            glColor3f(clr[0],clr[1],clr[2]);
            glBegin(GL_LINE_LOOP);
            for (int ptIndex = 0; ptIndex < nPts; ptIndex++ ){
                if (topopt.getPointOnContour(component,contour,ptIndex,pt)){
                    glVertex3d(pt[0],pt[1],pt[2]);
                }
            }
            glEnd();
        }
    }
    glEnable( GL_LIGHTING );

    if (liveVar.geomTransparent) {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        plotSTLGeom(liveVar.displayEdges);
        glDisable( GL_BLEND );
        if (liveVar.displayGeom == DISPLAY_GEOM_FINAL)
            plotTempGeom(liveVar.displayEdges);
    }
}
/**********************************************************************/
void Renderer::plotFieldOverMesh(void)
{
    int NumElems = topopt.getNumElems();
    int i,elem,m,nodes[8],maxFieldNode;
    double x[8],y[8],z[8],u[8],v[8],w[8],rho,fieldMin,fieldMax,*fieldToPlot;
    double p0[3],p1[3],p2[3],triNormal[3],p01[3],p02[3],scale;

    int quads[6][4] = {{0,3,2,1},{0,1,5,4},{0,4,7,3},{4,5,6,7},{1,2,6,5},{2,3,7,6}};//local nodes of 6 faces
    GLfloat color[4] ={0.1,0.95, 0.1, 0.0};
    char title[256];
    double temp,delta[8],vonmises[8],temperature[8];

    getStaticFEATitle(&fieldMin,&fieldMax,title,&maxFieldNode);
    double modelScale = vox.getModelScale();
    scale = 0.05*modelScale/topopt.getDeformationMax();

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

    if (liveVar.plotAxis) drawAxis();

    glPointSize(6.0);
    liveVar.animationVariable += 3.1415*10/180;
    liveVar.staticAnimationScaling = (1+cos(liveVar.animationVariable))/2;
    scale = 1;
    for (elem = 0; elem < NumElems; elem++) {
        rho = topopt.getElemPseudoDensity(elem);
        if (rho <0.5) continue;

        topopt.getElemNodeDeformation(elem,u,v,w);

        topopt.getElemNodes(elem,nodes);
        topopt.getElemNodeLocation(elem,x,y,z);
        topopt.getElemNodesVonMisesStresses(elem,vonmises);
        topopt.getElemNodesTemperature(elem,temperature);

        for (i = 0; i < 8;i++) {
            x[i] += scale*u[i];
            y[i] += scale*v[i];
            z[i] += scale*w[i];
            delta[i] = sqrt(u[i]*u[i] +   v[i]*v[i] + w[i]*w[i]);

        }

        switch (liveVar.displayField) {
        case DISPLAY_FIELD_DISPLACEMENT:
            fieldToPlot = &(delta[0]);
            break;
        case DISPLAY_FIELD_STRESS:
            fieldToPlot = &(vonmises[0]);
            break;
        case DISPLAY_FIELD_TEMPERATURE:
            fieldToPlot = &(temperature[0]);
            break;
        default:
            fieldToPlot = &(delta[0]);
            break;
        }

        for (m = 0; m < 6;m++) {// 6 faces of a hexelement
            p0[0] = x[quads[m][0]];
            p0[1] = y[quads[m][0]];
            p0[2] = z[quads[m][0]];

            p1[0] = x[quads[m][1]];
            p1[1] = y[quads[m][1]];
            p1[2] = z[quads[m][1]];

            p2[0] = x[quads[m][2]];
            p2[1] = y[quads[m][2]];
            p2[2] = z[quads[m][2]];

            for (int i = 0; i < 3; i++) {
                p01[i] = p0[i] - p1[i];
                p02[i] = p2[i] - p1[i];
            }

            CROSS3D(triNormal,p02,p01);

            glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
            glBegin(GL_QUADS);

            temp = (fieldToPlot[quads[m][0]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);


            temp = (fieldToPlot[quads[m][1]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);

            temp = (fieldToPlot[quads[m][2]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);


            temp = (fieldToPlot[quads[m][3]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();

        }
    }
    glDisable( GL_LIGHTING );

    /*glLineWidth(2.5);
    for (elem = 0; elem < NumElems; elem++) {
        rho = topopt.getElemPseudoDensity(elem);
        if (rho < 0.5) continue;
        topopt.getElemNodeLocationWithDeformation(elem,x,y,z,scale);
        topopt.getElemNodes(elem,nodes);
        for (m = 0; m < 6;m++) {// 6 faces
            // 4 lines per quad
            glBegin(GL_LINE_LOOP);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
        }
    }*/

    plotColorBars();
}
/**********************************************************************/
void Renderer::plotStructuralAnalysisResults(void)
{
    int pareto_viewport[4], startX, startY,deltaY;
    char str[256];
    double yieldStress;
    float clr[] = {0.0f,0.0f,1.0f};

	if (!liveVar.plotSummaryResults) return;

    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity();

    glGetIntegerv (GL_VIEWPORT, pareto_viewport);
    gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
    glDepthFunc (GL_ALWAYS);

    deltaY = 30;// font size dependent
    startX = 20;
    startY = deltaY;

#ifdef PARETOWORKS
    startX += 300;
    startY += 80;
	deltaY = 20;
#endif

	sprintf(str, "Loadset: %d ", liveVar.whichLoadSetElasticityFEASolved);
	print2DText(startX, startY, str);

	startY += deltaY;

    sprintf(str,"#Elements: %d ",topopt.getNumElems());
    print2DText(startX, startY,str);

    startY += deltaY;

    sprintf(str,"#DOF: %d ",3*topopt.getNumNodes());
    print2DText(startX, startY,str);

    startY +=deltaY;
    sprintf(str,"MeshSize: %.3g ",topopt.getMeshElementSize());
    print2DText(startX, startY,str);

    startY +=deltaY;
    sprintf(str,"Deformation Scale: %.2g ",liveVar.deformationScaled);
    print2DText(startX, startY,str);

    char units[256];
    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        strcpy(units,"meter");
        break;
    case UNITS_MM:
        strcpy(units,"mm");
        break;
    case UNITS_INCH:
        strcpy(units,"inch");
        break;
    }
    startY +=deltaY;
    sprintf(str,"Max Displacement: %.3g (%s) ",topopt.getDeformationMax(),units);
    print2DText(startX, startY,str);

    double stressSafetyFactor,tempDouble,maxStress;
    int criticalPart;

    stressSafetyFactor = HUGE_VAL;
    maxStress = -HUGE_VAL;
    criticalPart = 0;

    for (int part = 0; part < vox.getNumParts(); part++){
        int matId = topopt.getMaterialIDofPart(part);
        topopt.getYieldStressOfMaterial(matId,&yieldStress);
        tempDouble = yieldStress/topopt.getInitialMaxStressInPart(part);
        maxStress = MAX(maxStress,topopt.getInitialMaxStressInPart(part));
        if (tempDouble < stressSafetyFactor){
            stressSafetyFactor = tempDouble;
            criticalPart = part;
        }
    }

    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        strcpy(units,"N/m^2");
        break;
    case UNITS_MM:
        strcpy(units,"N/mm^2");
        break;
    case UNITS_INCH:
        strcpy(units,"psi");
        break;
    }

    startY +=deltaY;
    sprintf(str,"Max Stress: %.3g (%s) ",maxStress,units);
    print2DText(startX, startY,str);

    startY += deltaY;
    sprintf(str,"Stress Safety Factor: %.2f ",stressSafetyFactor);
    print2DText(startX, startY,str,clr);

    if (vox.getNumParts() > 1) {
        startY += deltaY;
        sprintf(str,"Critical part: %d ",criticalPart);
        print2DText(startX, startY,str);
    }

	startY += deltaY;
    sprintf(str, "Time taken (sec): %.2f", liveVar.timeForOneFEA);
	print2DText(startX, startY, str);

    glDepthFunc (GL_LESS);
    glPopMatrix ();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix ();
}
/**********************************************************************/
void Renderer::plotThermalAnalysisResults(void)
{
    int pareto_viewport[4], startX, startY,deltaY;
    char str[256];
    float clr[] = {0.0f,0.0f,1.0f};

    if (!liveVar.plotSummaryResults) return;

    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity();

    glGetIntegerv (GL_VIEWPORT, pareto_viewport);
    gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
    glDepthFunc (GL_ALWAYS);

    deltaY = 30;// font size dependent
    startX = 20;
    startY = deltaY;

#ifdef PARETOWORKS
    startX += 300;
    startY += 80;
    deltaY = 20;
#endif

    //sprintf(str, "Loadset: %d ", liveVar.whichLoadSetElasticityFEASolved);
    //print2DText(startX, startY, str);

    startY += deltaY;

    sprintf(str,"#Elements: %d ",topopt.getNumElems());
    print2DText(startX, startY,str);

    startY +=deltaY;
    sprintf(str,"MeshSize: %.3g ",topopt.getMeshElementSize());
    print2DText(startX, startY,str);


    char units[256];
    double tMin,tMax;
    switch (liveVar.temperatureUnits) {
    case UNITS_TEMPERATURE_CELCIUS:
        strcpy(units,"Celcius");
        break;
    case UNITS_TEMPERATURE_KELVIN:
        strcpy(units,"Kelvin");
        break;
    case UNITS_TEMPERATURE_FAHRENHEIT:
        strcpy(units,"Fahrenheit");
        break;
    }
    topopt.getTemperatureRange(&tMin,&tMax);
    startY +=deltaY;
    sprintf(str,"Min Temperature: %.4g (%s) ",tMin,units);
    print2DText(startX, startY,str);
    startY +=deltaY;
    sprintf(str,"Max Temperature: %.4g (%s) ",tMax,units);
    print2DText(startX, startY,str);


    startY +=deltaY;
    sprintf(str,"Zero-Strain T: %.4g (%s) ",liveVar.zeroStrainTemperature,units);
    print2DText(startX, startY,str);

    glDepthFunc (GL_LESS);
    glPopMatrix ();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix ();
}
/**********************************************************************/
void Renderer::plotModalResults(void)
{
    int pareto_viewport[4], startX, startY,deltaY;
    char str[256];
    double yieldStress;

    if (!liveVar.plotSummaryResults) return;

    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity();

    glGetIntegerv (GL_VIEWPORT, pareto_viewport);
    gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
    glDepthFunc (GL_ALWAYS);

    deltaY = 30;// font size dependent
    startX = 20;
    startY = deltaY;

#ifdef PARETOWORKS
    startX += 300;
    startY += 80;
    deltaY = 20;
#endif

    int modeToDisplay = min(liveVar.nEigenModes,liveVar.displayEigenMode)-1;//offset by 1

    sprintf(str,"EigenMode: %d, %0.2f (Hz) ",modeToDisplay+1,liveVar.eigenValues[modeToDisplay]);
    print2DText(startX, startY,str);

    startY +=deltaY;
    sprintf(str,"#Elements: %d ",topopt.getNumElems());
    print2DText(startX, startY,str);

    startY +=deltaY;
    sprintf(str,"MeshSize: %.3g ",topopt.getMeshElementSize());
    print2DText(startX, startY,str);

    startY +=deltaY;
    sprintf(str,"Deformation Scale: %.2g ",liveVar.deformationScaled);
    print2DText(startX, startY,str);

    glDepthFunc (GL_LESS);
    glPopMatrix ();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix ();
}
/******************************************************************************/
void Renderer::plotSupportStructuresFinalTopology(int direction)
{
    // direction can be 0, +/-1 , +/- 2, +/- 2
        if (direction == 0) return;
        double xyzMin[3],xyzMax[3];
        int triNeighbors[3];
        double cutoff = cos(liveVar.imposedOverHangAngle*3.1415/180);
        double triNormal[3],dtPrd,buildDirection[3],center[3],end[3],x[3],y[3],z[3], vertexNormals0[3], vertexNormals1[3], vertexNormals2[3];

        buildDirection[0] = buildDirection[1] = buildDirection[2] = 0;

        if ((direction == 1) || (direction == -1))
            buildDirection[0] = SIGN(direction);
        else if ((direction == 2) || (direction == -2))
            buildDirection[1] = SIGN(direction);
        if ((direction == 3) || (direction == -3))
            buildDirection[2] =  SIGN(direction);

        vox.getBoundingBox(xyzMin,xyzMax);

        glDisable(GL_LIGHTING);
        glLineWidth(1.0);
        glColor3f(1,0,0);

        int savedCounter = topopt.getCurrentIsoCounter();//only on the final stl
        if (savedCounter < 0) return;
        // check if vertex normals are available
        bool vertexNormalsPresent = false;
        topopt.getIsoTriPts(savedCounter,0,x,y,z,triNormal, vertexNormals0, vertexNormals1, vertexNormals2);// get the first triangle
        if (NORM3D(vertexNormals0) > 0.5){// check to see if vertex normals are available
            vertexNormalsPresent = true;
        }

        for (int tri = 0; tri < topopt.getNumIsoTriangles(savedCounter); tri++) {
            topopt.getIsoTriPts(savedCounter,tri,x,y,z,triNormal, vertexNormals0, vertexNormals1, vertexNormals2);
            dtPrd = -DOTPROD3D(triNormal,buildDirection);
            if (vertexNormalsPresent){// look at neighbors to kill noise
                double xTemp[3],yTemp[3],zTemp[3],otherNormal[3],tempNormals[3][3];
                topopt.getIsoTriNeighbors(savedCounter,tri,triNeighbors);
                for (int i = 0; i < 3; i++) {
                    topopt.getIsoTriPts(savedCounter,triNeighbors[i],xTemp,yTemp,zTemp,otherNormal, vertexNormals0, vertexNormals1, vertexNormals2);
                    dtPrd = MIN(dtPrd,-DOTPROD3D(otherNormal,buildDirection));
                }
            }

            if (dtPrd > cutoff){
                center[0] = (x[0]+x[1]+x[2])/3;
                center[1] = (y[0]+y[1]+y[2])/3;
                center[2] = (z[0]+z[1]+z[2])/3;
                end[0] = center[0];
                end[1] = center[1];
                end[2] = center[2];
                if ((direction == 1) || (direction == -1))
                    end[0] = (direction == 1)? xyzMin[0]:xyzMax[0];
                else if ((direction == 2) || (direction == -2))
                    end[1] = (direction == 2)? xyzMin[1]:xyzMax[1];
                else if ((direction == 3) || (direction == -3))
                    end[2] = (direction == 3)? xyzMin[2]:xyzMax[2];

                glBegin(GL_LINES);
                    glVertex3d(center[0],center[1],center[2]);
                    glVertex3d(end[0],end[1],end[2]);
                glEnd();

            }
        }
        glEnable(GL_LIGHTING);
}
/******************************************************************************/
void Renderer::plotSupportStructuresInitialGeometry(int direction)
{
    // direction can be 0, +/-1 , +/- 2, +/- 2
        if (direction == 0) return;
        double xyzMin[3],xyzMax[3],baryCentricCoords[3],triPt[3];

#define NUM_BARYCENTRIC_POINTS 11
    double xi[NUM_BARYCENTRIC_POINTS] = {0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0};
    double eta[NUM_BARYCENTRIC_POINTS] = {0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0};
    double mu[NUM_BARYCENTRIC_POINTS] = {0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0};

        double cutoff = cos(liveVar.imposedOverHangAngle*3.1415/180);
        double triNormal[3],dtPrd,buildDirection[3],end[3],x[3],y[3],z[3];

        buildDirection[0] = buildDirection[1] = buildDirection[2] = 0;

        if ((direction == 1) || (direction == -1))
            buildDirection[0] = SIGN(direction);
        else if ((direction == 2) || (direction == -2))
            buildDirection[1] = SIGN(direction);
        if ((direction == 3) || (direction == -3))
            buildDirection[2] =  SIGN(direction);

        vox.getBoundingBox(xyzMin,xyzMax);

        glDisable(GL_LIGHTING);
        glLineWidth(1.0);
        glColor3f(1,0,0);

        for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
            vox.getNormalOfTriangle(tri,triNormal);
            dtPrd = -DOTPROD3D(triNormal,buildDirection);
            if (dtPrd > 1.05*cutoff){
                for (int n1 = 0; n1 < NUM_BARYCENTRIC_POINTS; n1++){
                    for (int n2 = 0; n2 < NUM_BARYCENTRIC_POINTS; n2++){
                        for (int n3 = 0; n3 < NUM_BARYCENTRIC_POINTS; n3++){
                            if (fabs(xi[n1] + eta[n2] + mu[n3] - 1) > 0.01) continue;

                            baryCentricCoords[0] = xi[n1];
                            baryCentricCoords[1] = eta[n2];
                            baryCentricCoords[2] = mu[n3];

                            vox.getPtOnTriangleAt( tri, baryCentricCoords,triPt);

                            end[0] = triPt[0];
                            end[1] = triPt[1];
                            end[2] = triPt[2];
                            if ((direction == 1) || (direction == -1))
                                end[0] = (direction == 1)? xyzMin[0]:xyzMax[0];
                            else if ((direction == 2) || (direction == -2))
                                end[1] = (direction == 2)? xyzMin[1]:xyzMax[1];
                            else if ((direction == 3) || (direction == -3))
                                end[2] = (direction == 3)? xyzMin[2]:xyzMax[2];

                            glBegin(GL_LINES);
                            glVertex3d(triPt[0],triPt[1],triPt[2]);
                            glVertex3d(end[0],end[1],end[2]);
                            glEnd();

                        }
                    }
                }
            }
        }
        glEnable(GL_LIGHTING);
}
/************************************************************/
void Renderer::getColorOfIsosurface(int isosurfaceId,GLfloat color[4])
{
    getColorOfPart(isosurfaceId, color);
}
/**********************************************************************/
void Renderer::plotLatticeResults(void)
{
    char latticeSpecs[1024];
    int pareto_viewport[4], startX, startY,deltaY;

    return;

    topopt.getLatticeSpecsAsString(latticeSpecs);
    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity();

    glGetIntegerv (GL_VIEWPORT, pareto_viewport);
    gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
    glDepthFunc (GL_ALWAYS);

    if ((pareto_viewport[2] < 0) || (pareto_viewport[3] < 0)) return; //sometimes in ParetoWorks

    deltaY = 30;// depends on the font siz
    startX = 20;
    startY = deltaY;

    print2DText(startX, startY,latticeSpecs);

    glDepthFunc (GL_LESS);
    glPopMatrix ();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix ();
}
/**********************************************************************/
void Renderer::plotLatticeStructure(void)
{
    int nTrusses,nodes[2];
    double pt0[3],pt1[3];
    nTrusses = topopt.getLatticeNumberOfTrussEdges();

    if (nTrusses == 0) return;
    glDisable(GL_LIGHTING);
    glLineWidth(2.0);
    glColor3f(0,0,1);

    for (int truss = 0; truss < nTrusses; truss++){
        if (!topopt.getLatticeNodesOfTrussEdge(truss,nodes)) continue;
        if (!topopt.getLatticeXYZOfTrussNode(nodes[0],pt0)) continue;
        if (!topopt.getLatticeXYZOfTrussNode(nodes[1],pt1)) continue;
        glBegin(GL_LINES);
            glVertex3d(pt0[0],pt0[1],pt0[2]);
            glVertex3d(pt1[0],pt1[1],pt1[2]);
        glEnd();
    }
    glEnable(GL_LIGHTING);

}
/**********************************************************************/
void Renderer::plotSIMPResults(void)
{
    int pareto_viewport[4], startX, startY,deltaY;
    char str[256];
    double timeInSecs;
    float clr[] = {0.0f,0.0f,1.0f};

    if (!liveVar.plotSummaryResults) return;

    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity();

    glGetIntegerv (GL_VIEWPORT, pareto_viewport);
    gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
    glDepthFunc (GL_ALWAYS);

    if ((pareto_viewport[2] < 0) || (pareto_viewport[3] < 0)) return; //sometimes in ParetoWorks

    deltaY = 30;// depends on the font siz
    startX = 20;
    startY = deltaY;
#ifdef PARETOWORKS
    startX += 300;
    startY += 80;
    deltaY = 20;
#endif
    sprintf(str,"Target Vol. Frac: %.3f ",liveVar.desiredVolFraction);
    print2DText(startX, startY,str);

    startY += deltaY;
    sprintf(str,"Current Vol. Frac: %.3f ",topopt.getCurrentVolumeFraction());
    print2DText(startX, startY,str,clr);

    startY += deltaY;
    sprintf(str,"Time Taken: %.2f (secs) ",topopt.getSIMPTimerInSeconds());
    print2DText(startX, startY,str);


    startY += deltaY;
    char units[256];
    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        strcpy(units,"N-m");
        break;
    case UNITS_MM:
        strcpy(units,"N-mm");
        break;
    case UNITS_INCH:
        strcpy(units,"lbf-inch");
        break;
    }
    sprintf(str,"Compliance = %.3g (%s) ",topopt.computeCompliance(),units);
    print2DText(startX, startY,str,clr);



    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        strcpy(units,"meter");
        break;
    case UNITS_MM:
        strcpy(units,"mm");
        break;
    case UNITS_INCH:
        strcpy(units,"inch");
        break;
    }

    double maxDisplacement = topopt.getDeformationMax();
    startY += deltaY;
    sprintf(str,"Max Disp = %.3g (%s) ",maxDisplacement,units);
    print2DText(startX, startY,str);


    startY += deltaY;

    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        strcpy(units,"N/m^2");
        break;
    case UNITS_MM:
        strcpy(units,"N/mm^2");
        break;
    case UNITS_INCH:
        strcpy(units,"psi");
        break;
    }
    double maxStress = topopt.getMaxVonMisesStress();
    sprintf(str,"Max Stress = %.3g (%s) ",maxStress,units);
    print2DText(startX, startY,str);


    glDepthFunc (GL_LESS);
    glPopMatrix ();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix ();
}

/**********************************************************************/
void Renderer::plotTopOptResults(void)
{
    int pareto_viewport[4], startX, startY,deltaY;
    char str[256];
    double actualVolumeFraction,isoCutoff,timeInSecs,yieldStress;
    double allStresses[MAX_BC_SET][MAX_PARTS], allDisplacements[MAX_BC_SET];
    static double displacementSave,stressSave;
    float clr[] = {0.0f,0.0f,1.0f};


    if (!liveVar.plotSummaryResults) return;
    int savedCounter = topopt.getClosestCounterAtVolFraction(liveVar.displayVolFraction,&actualVolumeFraction,&isoCutoff, &timeInSecs);
    if (savedCounter < 0) 	return;

    if ((liveVar.nBCSet > 1) && (liveVar.useAllLoads == 1)){// multiload
        topopt.get_allMaxDisplacementsAtCounter_ML(savedCounter,allDisplacements);
        for (int part = 0; part < vox.getNumParts(); part++){
            topopt.get_allMaxStressesAtCounterForAllLoadsAllPart_ML(savedCounter,allStresses);
        }
    }
    else {// single load
        allDisplacements[0] = topopt.getMaxDispAtCounter(savedCounter);
        for (int part = 0; part < vox.getNumParts(); part++)
            allStresses[0][part] = topopt.getMaxStressInPartAtCounter(savedCounter,part);
    }

    for (int bc = 0; bc < liveVar.nBCSet; bc++) {
        if (!liveVar.activeBCSets[bc]) continue;
        if (allDisplacements[bc] < 1e-30) {// not computed
            allDisplacements[bc] = displacementSave; // not yet updated ... use previous
            for (int part = 0; part < vox.getNumParts(); part++)
                allStresses[bc][part] = stressSave;// from previous run
        }
    }
    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity();

    glGetIntegerv (GL_VIEWPORT, pareto_viewport);
    gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
    glDepthFunc (GL_ALWAYS);

    if ((pareto_viewport[2] < 0) || (pareto_viewport[3] < 0)) return; //sometimes in ParetoWorks

    deltaY = 30;// depends on the font size
    startX = 20;
    startY = deltaY;
#ifdef PARETOWORKS
    startX += 300;
    startY += 80;
	deltaY = 20;
#endif
    sprintf(str,"Target Vol. Frac: %.3f ",liveVar.desiredVolFraction);
    print2DText(startX, startY,str);

    startY += deltaY;
    sprintf(str,"Current Vol. Frac: %.3f ",actualVolumeFraction);
    print2DText(startX, startY,str,clr);

    startY += deltaY;
    sprintf(str,"Time Taken: %.2f (secs) ",timeInSecs);
    print2DText(startX, startY,str);

	startY += deltaY;
	sprintf(str, "Estimated time: %.1f (secs) ", liveVar.estimatedTimeForTopOpt);
	print2DText(startX, startY, str);

    startY +=deltaY;
    sprintf(str,"Deformation Scale: %.2g ",liveVar.deformationScaled);
    print2DText(startX, startY,str);

    startY += deltaY;
    char units[256];
    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        strcpy(units,"N-m");
        break;
    case UNITS_MM:
        strcpy(units,"N-mm");
        break;
    case UNITS_INCH:
        strcpy(units,"lbf-inch");
        break;
    }
    sprintf(str,"Compliance = %.3g (%s) ",topopt.computeCompliance(),units);
    print2DText(startX, startY,str,clr);

    // find the worst safety factor
    double dispSafetyFactor, stressSafetyFactor,tempDouble,modalSafetyFactor,maxDisplacement,maxStress;
    int criticalLoad,criticalPart;

    dispSafetyFactor = 1000000;
    maxStress = -HUGE_VAL;
    maxDisplacement = -HUGE_VAL;
    criticalLoad = 0;
    if ((liveVar.nBCSet > 1) && (liveVar.useAllLoads == 1)){// multiload
        for (int bcSet = 0; bcSet <liveVar.nBCSet; bcSet++) {
            if (!liveVar.activeBCSets[bcSet]) continue;
            tempDouble = liveVar.topoptConstraints.maxDisplacementAllowed/allDisplacements[bcSet];
            maxDisplacement = MAX(maxDisplacement,allDisplacements[bcSet]);
            if (tempDouble < dispSafetyFactor) {
                dispSafetyFactor = tempDouble;
                criticalLoad = bcSet;
            }
        }
    }
    else {
        dispSafetyFactor = MIN(dispSafetyFactor,liveVar.topoptConstraints.maxDisplacementAllowed/allDisplacements[0]);
        maxDisplacement = MAX(maxDisplacement,allDisplacements[0]);
        criticalLoad = 0;
    }
    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        strcpy(units,"meter");
        break;
    case UNITS_MM:
        strcpy(units,"mm");
        break;
    case UNITS_INCH:
        strcpy(units,"inch");
        break;
    }

    displacementSave = liveVar.topoptConstraints.maxDisplacementAllowed/allDisplacements[0];
    startY += deltaY;
    sprintf(str,"Max Disp = %.3g (%s) ",maxDisplacement,units);
    print2DText(startX, startY,str);
    startY += deltaY;
    sprintf(str,"Disp. Safety Factor = %.2f ",dispSafetyFactor);
    print2DText(startX, startY,str,clr);
    if ((liveVar.useAllLoads) && (liveVar.nBCSet > 1)){// multiload
        startY += deltaY;
        sprintf(str,"Critical load = %d ",criticalLoad);
        print2DText(startX, startY,str);
    }

    stressSafetyFactor = 1000000;
    criticalLoad = 0;
    criticalPart = 0;
    if ((liveVar.nBCSet > 1) && (liveVar.useAllLoads == 1)){// multiload
        for (int bcSet = 0; bcSet <liveVar.nBCSet; bcSet++) {
            if (!liveVar.activeBCSets[bcSet]) continue;
            for (int part = 0; part < vox.getNumParts(); part++){
                int matId = topopt.getMaterialIDofPart(part);
                topopt.getYieldStressOfMaterial(matId,&yieldStress);
                maxStress = MAX(maxStress,allStresses[bcSet][part]);
                tempDouble = yieldStress/allStresses[bcSet][part];
                if (tempDouble < stressSafetyFactor) {
                    stressSafetyFactor = tempDouble;
                    criticalLoad = bcSet;
                    criticalPart = part;
                }
            }
        }
    }
    else {
        for (int part = 0; part < vox.getNumParts(); part++){
            int matId = topopt.getMaterialIDofPart(part);
            topopt.getYieldStressOfMaterial(matId,&yieldStress);
            maxStress = MAX(maxStress,allStresses[0][part]);
            tempDouble = yieldStress/allStresses[0][part];
            if (tempDouble < stressSafetyFactor) {
                stressSafetyFactor = tempDouble;
                criticalLoad = 0;
                criticalPart = part;
            }
        }
    }
    startY += deltaY;

    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        strcpy(units,"N/m^2");
        break;
    case UNITS_MM:
        strcpy(units,"N/mm^2");
        break;
    case UNITS_INCH:
        strcpy(units,"psi");
        break;
    }
    stressSafetyFactor = MAX(1,stressSafetyFactor);// below this does not make sense
    stressSave =  yieldStress/stressSafetyFactor;

    sprintf(str,"Max Stress = %.3g (%s) ",maxStress,units);
    print2DText(startX, startY,str);
    startY += deltaY;
    sprintf(str,"Stress Safety Factor = %.2f ",stressSafetyFactor);
    print2DText(startX, startY,str,clr);
    if ((liveVar.useAllLoads) && (liveVar.nBCSet > 1)){// multiload
        startY += deltaY;
        sprintf(str,"Critical load = %d ",criticalLoad);
        print2DText(startX, startY,str);
    }
    if (vox.getNumParts() > 1) {
        startY += deltaY;
        sprintf(str,"Critical part = %d ",criticalPart);
        print2DText(startX, startY,str);
    }

    if (topopt.isEigenConstraintActive()){
        double firstEigenValue = topopt.getFirstEigenValueAtCounter(savedCounter);
        if (firstEigenValue > 0){
            modalSafetyFactor = firstEigenValue/liveVar.topoptConstraints.minEigenValueAllowedHz;
            startY += deltaY;
            sprintf(str,"First Mode = %.2f (Hz)",firstEigenValue);
            print2DText(startX, startY,str);
            startY += deltaY;
            sprintf(str,"Modal Safety Factor = %.2f ",modalSafetyFactor);
            print2DText(startX, startY,str,clr);
        }
    }


    if (liveVar.includeThermalEffects) {
        string strUnits;
        coreFunctions.getPropertyUnits(PROPERTY_TEMPERATURE,strUnits);
        startY += deltaY;
        sprintf(str,"Max Temperature = %.3g (%s) ",topopt.getTemperatureMax(),strUnits.c_str());
        print2DText(startX, startY,str);
    }

    if (liveVar.isTopOptComplete){
         float clr2[] = {0.0f,0.9f,0.2f};
         startY += deltaY;
         print2DText(startX, startY,"TopOpt complete!",clr2);
    }
    glDepthFunc (GL_LESS);
    glPopMatrix ();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix ();
}
void Renderer::plotXYMultiple(vector<double> xData, vector<vector<double>> yData, vector <char *> yLegend,bool smooth,const char *xTitle,const char *yTitle,const char *chartTitle,int width ,int height)
{
    /* Function  usage
    int nSamples = 10;
    vector<double> xData;
    vector<vector<double>> yData;
    vector<char *> yLegend;

    yData.push_back( vector<double>());// two curves
    yData.push_back( vector<double>());
    yLegend.push_back("Data 1");//legend identifiers
    yLegend.push_back("Data 2");
    for (int i = 0; i < nSamples; i++) {
        xData.push_back(10000*i);
        yData[0].push_back(0.6e-4*i);
        yData[1].push_back(0.3e-4*i);

    }
    plotXYMultiple(xData,yData,yLegend,false,"X Values","Y Values","My Chart");
    */
}
/************************************************************/
void Renderer::plotNonDesignSTLGeom(bool displayEdges)
{
    double xTri[3],yTri[3],zTri[3],p01[3],p02[3],triNormal[3];
    GLfloat color[4] ={0,0, 0, 0.2};
    int nGroups,group,part;

    nGroups = vox.getNumTriGroups();

    for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
        part = vox.getParentPartOfTriangle(tri);
        if(topopt.shouldPartBeOptimized(part)) continue;
        vox.getTriPts(tri,xTri,yTri,zTri);
        group = vox.getTriGroup(tri);
        p01[0] = xTri[1]-xTri[0];
        p01[1] = yTri[1]-yTri[0];
        p01[2] = zTri[1]-zTri[0];

        p02[0] = xTri[2]-xTri[0];
        p02[1] = yTri[2]-yTri[0];
        p02[2] = zTri[2]-zTri[0];
        CROSS3D(triNormal,p01,p02);   

        if (vox.isTriangleHighlighted(tri)){
            color[0] = 1;
            color[1] = 0;
            color[2] = 0;
        }
        else{
            getColorOfPart(part,color);
        }
        glDisable(GL_COLOR_MATERIAL);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
        //glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny); // Shininess between 0 and 128
        glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
        glBegin(GL_TRIANGLES);
        glVertex3d(xTri[0],yTri[0],zTri[0]);
        glVertex3d(xTri[1],yTri[1],zTri[1]);
        glVertex3d(xTri[2],yTri[2],zTri[2]);
        glEnd();
        glDisable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        if (displayEdges) {
            glBegin(GL_LINES);
            glVertex3d(xTri[0],yTri[0],zTri[0]);
            glVertex3d(xTri[1],yTri[1],zTri[1]);

            glVertex3d(xTri[1],yTri[1],zTri[1]);
            glVertex3d(xTri[2],yTri[2],zTri[2]);

            glVertex3d(xTri[2],yTri[2],zTri[2]);
            glVertex3d(xTri[0],yTri[0],zTri[0]);
            glEnd();
        }
        glEnable( GL_LIGHTING );
    }
    glDisable( GL_BLEND );
    glDisable(GL_COLOR_MATERIAL);

    glEnable(GL_COLOR_MATERIAL);
    glDisable( GL_BLEND );
    glLineWidth(2.0);
    glColor3f(0,0,0);
    //plot Sharp edges
     double xEdge[2],yEdge[2],zEdge[2];
    for (int e = 0; e < vox.getNumSharpEdges(); e++) {
        vox.getSharpEdgePoints(e,xEdge,yEdge,zEdge);
        glBegin(GL_LINES);
        glVertex3d(xEdge[0],yEdge[0],zEdge[0]);
        glVertex3d(xEdge[1],yEdge[1],zEdge[1]);
        glEnd();
    }
    glDisable(GL_COLOR_MATERIAL);
}
/**********************************************************************/
void Renderer::getInterpolatedColor(double value, float color[3])
{
    // value is between 0 and 1
    //the color values are r,g,b
    value = MIN(MAX(value,0),1);
    color[0] = -value + 2 * value*value;
    color[1] = 4 * value - 4 * value*value;
    color[2] = 1 - 3 * value + 2 * value*value;
}

/**********************************************************************/
void Renderer::plotFieldOverTriangle(double x[3], double y[3], double z[3],double u[3],double v[3],double w[3],double vonmises[3],double temperature[3],double dist[3],
double scale,double triNormal[3],double fieldMin,double fieldMax)
{
    int i;
    double *fieldToPlot,temp;
    GLfloat color[4] ={1.0, 0., 0., 0.75};
    double delta[3],xTri[3],yTri[3],zTri[3];

    // Get the deformations u,v,w or stresses, strains at those nodes
    switch (liveVar.displayField) {
    case DISPLAY_FIELD_DISPLACEMENT:
        fieldToPlot = &(delta[0]);
        break;
    case DISPLAY_FIELD_DISPLACEMENT_X:
        fieldToPlot = &(u[0]);
        break;
    case DISPLAY_FIELD_DISPLACEMENT_Y:
        fieldToPlot = &(v[0]);
        break;
    case DISPLAY_FIELD_DISPLACEMENT_Z:
        fieldToPlot = &(w[0]);
        break;
    case DISPLAY_FIELD_STRESS:
        fieldToPlot = &(vonmises[0]);
        break;
    case DISPLAY_FIELD_TEMPERATURE:
        fieldToPlot = &(temperature[0]);
        scale = 0;
        break;
    case DISPLAY_FIELD_MODAL:
        fieldToPlot = &(delta[0]);
        break;
    default:
        fieldToPlot = &(delta[0]);
        break;
    }
    for (i = 0; i < 3; i++) {
        xTri[i] = x[i] + scale*u[i];
        yTri[i] = y[i] + scale*v[i];
        zTri[i] = z[i] +  scale*w[i];
        delta[i] = sqrt(u[i]*u[i] + v[i]*v[i]+ w[i]*w[i]);
    }
    if (2*(fieldMax - fieldMin)/(fieldMax+fieldMin) < 0.001){
        fieldToPlot[0] = fieldToPlot[1] = fieldToPlot[2] = fieldMin;
    }

    glBegin(GL_TRIANGLES);
    temp = (fieldToPlot[0]-fieldMin)/(fieldMax-fieldMin);
    getInterpolatedColor(temp, color);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
    glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
    glVertex3f(xTri[0],yTri[0],zTri[0]);

    temp = (fieldToPlot[1]-fieldMin)/(fieldMax-fieldMin);
    getInterpolatedColor(temp, color);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
    glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
    glVertex3f(xTri[1],yTri[1],zTri[1]);

    temp = (fieldToPlot[2]-fieldMin)/(fieldMax-fieldMin);
    getInterpolatedColor(temp, color);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
    glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
    glVertex3f(xTri[2],yTri[2],zTri[2]);

    glEnd();

}
/**********************************************************************/
void Renderer::plotFieldOverTriangleRecursive(double x[3],double y[3],double z[3],double scale,double u[3],double v[3],double w[3],double vonmises[3],double t[3],double d[3],
double triNormal[3],double fieldMin,double fieldMax, double edgeLengthDesired,int currentDepth)
{
    // if currentDepth = 0, plot
    int i;

    if (currentDepth <= 0) { //plot
        plotFieldOverTriangle(x,y,z,u,v,w,vonmises,t,d,scale,triNormal, fieldMin, fieldMax);
        return;
    }

    currentDepth = MIN(currentDepth,20);// don't go too deep
    // find the edge with the most variation
    double LMax = -1e8,L;
    bool validElem;
    int nodes[]= {0,1,2,0},n1,n2,index;
    for (i = 0; i < 3;i++) {
        n1 = nodes[i];
        n2 = nodes[i+1];
        L = sqrt ( pow((x[n2]-x[n1]),2.0) + pow((y[n2]-y[n1]),2.0) +pow((z[n2]-z[n1]),2.0) );
        if (L > LMax) {
            index = i;
            LMax = L;
        }
    }
    if (LMax < edgeLengthDesired) {
        plotFieldOverTriangle(x,y,z,u,v,w,vonmises,t,d,scale,triNormal, fieldMin, fieldMax);
        return;
    }

    // Split this edge and call self twice, with depth reduced
    double x1[3],y1[3],z1[3],x2[3],y2[3],z2[3];
    double u1[3],v1[3],w1[3],u2[3],v2[3],w2[3],vonmises1[3],vonmises2[3],pt[3],uvw[3],stresses[6];
    double d1[3],d2[3],dn,voxSize[3];
    double un,vn,wn,vmn,tn,t1[3],t2[3];


    vox.getVoxelSize(voxSize);
    switch(index) {
    case 0:
        x1[0] = x[0];         y1[0] = y[0];          z1[0] = z[0];
        x1[1] = (x[0]+x[1])/2;y1[1] = (y[0]+y[1])/2; z1[1] = (z[0]+z[1])/2;
        x1[2] = x[2];         y1[2] = y[2];          z1[2] = z[2];

        x2[0] = x1[1];y2[0] = y1[1];z2[0] = z1[1];
        x2[1] = x[1]; y2[1] = y[1]; z2[1] = z[1];
        x2[2] = x[2]; y2[2] = y[2]; z2[2] = z[2];


        pt[0] = x1[1];pt[1] = y1[1]; pt[2] = z1[1];
        validElem = topopt.getElasticitySolutionAt(pt,uvw,stresses,&vmn);
        validElem = topopt.getTemperatureAt(pt,&tn);
        un = uvw[0];vn = uvw[1]; wn = uvw[2];

        if (!validElem) dn = 20*voxSize[0];
        else dn = 0;


        if (liveVar.plotAnimation) {
            un *= liveVar.staticAnimationScaling;
            vn *= liveVar.staticAnimationScaling;
            wn *= liveVar.staticAnimationScaling;
            vmn *= liveVar.staticAnimationScaling;
        }

        //recycle old values wherever you can
        u1[0] = u[0]; v1[0] = v[0]; w1[0] = w[0]; vonmises1[0] = vonmises[0];	t1[0] = t[0];	d1[0] = d[0];
        u1[1] = un;   v1[1] = vn;   w1[1] = wn;   vonmises1[1] = vmn;			t1[1] = tn;		d1[1] = dn;
        u1[2] = u[2]; v1[2] = v[2]; w1[2] = w[2]; vonmises1[2] = vonmises[2];	t1[2] = t[2];	d1[2] = d[2];

        u2[0] = un;   v2[0] = vn;   w2[0] = wn;   vonmises2[0] = vmn;			t2[0] = tn;		d2[0] = dn;
        u2[1] = u[1]; v2[1] = v[1]; w2[1] = w[1]; vonmises2[1] = vonmises[1];	t2[1] = t[1];	d2[1] = d[1];
        u2[2] = u[2]; v2[2] = v[2]; w2[2] = w[2]; vonmises2[2] = vonmises[2];	t2[2] = t[2];	d2[2] = d[2];

        break;
    case 1:
        x1[0] = x[1];         y1[0] = y[1];         z1[0] = z[1];
        x1[1] = (x[1]+x[2])/2;y1[1] = (y[1]+y[2])/2;z1[1] = (z[1]+z[2])/2;
        x1[2] = x[0];         y1[2] = y[0];         z1[2] = z[0];

        x2[0] = x1[1];y2[0] = y1[1];z2[0] = z1[1];
        x2[1] = x[2]; y2[1] = y[2]; z2[1] = z[2];
        x2[2] = x[0]; y2[2] = y[0]; z2[2] = z[0];


        pt[0] = x1[1];pt[1] = y1[1]; pt[2] = z1[1];
        validElem = topopt.getElasticitySolutionAt(pt,uvw,stresses,&vmn);
        validElem = topopt.getTemperatureAt(pt,&tn);
        un = uvw[0];vn = uvw[1]; wn = uvw[2];
        if (!validElem) dn = 20*voxSize[0];
        else dn = 0;

        if (liveVar.plotAnimation) {
            un *= liveVar.staticAnimationScaling;
            vn *= liveVar.staticAnimationScaling;
            wn *= liveVar.staticAnimationScaling;
            vmn *= liveVar.staticAnimationScaling;
        }

        //recycle old values wherever you can
        u1[0] = u[1]; v1[0] = v[1]; w1[0] = w[1]; vonmises1[0] = vonmises[1];	t1[0] = t[1];	d1[0] = d[1];
        u1[1] = un;   v1[1] = vn;   w1[1] = wn;   vonmises1[1] = vmn;			t1[1] = tn;		d1[1] = dn;
        u1[2] = u[0]; v1[2] = v[0]; w1[2] = w[0]; vonmises1[2] = vonmises[0];	t1[2] = t[0];	d1[2] = d[0];

        u2[0] = un;   v2[0] = vn;   w2[0] = wn;   vonmises2[0] = vmn;			t2[0] = tn;		d2[0] = dn;
        u2[1] = u[2]; v2[1] = v[2]; w2[1] = w[2]; vonmises2[1] = vonmises[2];	t2[1] = t[2];	d2[1] = d[2];
        u2[2] = u[0]; v2[2] = v[0]; w2[2] = w[0]; vonmises2[2] = vonmises[0];	t2[2] = t[0];	d2[2] = d[0];

        break;
    case 2:
        x1[0] = x[2];         y1[0] = y[2];         z1[0] = z[2];
        x1[1] = (x[2]+x[0])/2;y1[1] = (y[2]+y[0])/2;z1[1] = (z[2]+z[0])/2;
        x1[2] = x[1];         y1[2] = y[1];         z1[2] = z[1];

        x2[0] = x1[1];y2[0] = y1[1];z2[0] = z1[1];
        x2[1] = x[0]; y2[1] = y[0]; z2[1] = z[0];
        x2[2] = x[1]; y2[2] = y[1]; z2[2] = z[1];

        pt[0] = x1[1];pt[1] = y1[1]; pt[2] = z1[1];
        validElem = topopt.getElasticitySolutionAt(pt,uvw,stresses,&vmn);
        validElem = topopt.getTemperatureAt(pt,&tn);
        un = uvw[0];vn = uvw[1]; wn = uvw[2];

        if (!validElem) dn = 20*voxSize[0];
        else dn = 0;

        if (liveVar.plotAnimation) {
            un *= liveVar.staticAnimationScaling;
            vn *= liveVar.staticAnimationScaling;
            wn *= liveVar.staticAnimationScaling;
            vmn *= liveVar.staticAnimationScaling;
        }

        //recycle old values wherever you can
        u1[0] = u[2]; v1[0] = v[2]; w1[0] = w[2]; vonmises1[0] = vonmises[2];	t1[0] = t[2];	d1[0] = d[2];
        u1[1] = un;   v1[1] = vn;   w1[1] = wn;   vonmises1[1] = vmn;			t1[1] = tn;		d1[1] = dn;
        u1[2] = u[1]; v1[2] = v[1]; w1[2] = w[1]; vonmises1[2] = vonmises[1];	t1[2] = t[1];	d1[2] = d[1];

        u2[0] = un;   v2[0] = vn;   w2[0] = wn;   vonmises2[0] = vmn;			t2[0] = tn;		d2[0] = dn;
        u2[1] = u[0]; v2[1] = v[0]; w2[1] = w[0]; vonmises2[1] = vonmises[0];	t2[1] = t[0];	d2[1] = d[0];
        u2[2] = u[1]; v2[2] = v[1]; w2[2] = w[1]; vonmises2[2] = vonmises[1];	t2[2] = t[1];	d2[2] = d[1];

        break;
    }
    plotFieldOverTriangleRecursive(x1,y1,z1,scale,u1,v1,w1,vonmises1,t1,d1,triNormal,fieldMin,fieldMax,edgeLengthDesired,currentDepth-1);
    plotFieldOverTriangleRecursive(x2,y2,z2,scale,u2,v2,w2,vonmises2,t2,d2,triNormal,fieldMin,fieldMax,edgeLengthDesired,currentDepth-1);

}
/**********************************************************************/
void Renderer::plotSymmetryPlanes(bool xSym, bool ySym, bool zSym)
{
    int i;
    double xyzMin[3],xyzMax[3],range[3],pos[3];
    GLfloat color[3];
    glDisable(GL_BLEND);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
    //glEnable(GL_LIGHTING);
    vox.getBoundingBox(xyzMin,xyzMax);
    for (i = 0; i < 3;i++) {
        range[i] = xyzMax[i] - xyzMin[i];
        xyzMax[i] += 0.1*range[i];
        xyzMin[i] -= 0.1*range[i];
    }
    glLineWidth(3.0);
    if (xSym) {
        color[0] = 1.0;
        color[1] = 0.0;
        color[2] = 0.0;
        glColor3fv(color);
        glBegin(GL_LINE_LOOP);
            glVertex3d((xyzMin[0]+xyzMax[0])/2,xyzMin[1],xyzMin[2]);
            glVertex3d((xyzMin[0]+xyzMax[0])/2,xyzMax[1],xyzMin[2]);
            glVertex3d((xyzMin[0]+xyzMax[0])/2,xyzMax[1],xyzMax[2]);
            glVertex3d((xyzMin[0]+xyzMax[0])/2,xyzMin[1],xyzMax[2]);
        glEnd();
        //diagonals
        /*glBegin(GL_LINES);
            glVertex3d((xyzMin[0]+xyzMax[0])/2,xyzMin[1],xyzMin[2]);
            glVertex3d((xyzMin[0]+xyzMax[0])/2,xyzMax[1],xyzMax[2]);
            glVertex3d((xyzMin[0]+xyzMax[0])/2,xyzMax[1],xyzMin[2]);
            glVertex3d((xyzMin[0]+xyzMax[0])/2,xyzMin[1],xyzMax[2]);
        glEnd();*/
    }
    if (ySym) {
        color[0] = 0.0;
        color[1] = 1.0;
        color[2] = 0.0;
        glColor3fv(color);
        glBegin(GL_LINE_LOOP);
            glVertex3d(xyzMin[0],(xyzMin[1]+xyzMax[1])/2,xyzMin[2]);
            glVertex3d(xyzMax[0],(xyzMin[1]+xyzMax[1])/2,xyzMin[2]);
            glVertex3d(xyzMax[0],(xyzMin[1]+xyzMax[1])/2,xyzMax[2]);
            glVertex3d(xyzMin[0],(xyzMin[1]+xyzMax[1])/2,xyzMax[2]);
        glEnd();
        /*glBegin(GL_LINES);
            glVertex3d(xyzMin[0],(xyzMin[1]+xyzMax[1])/2,xyzMin[2]);
            glVertex3d(xyzMax[0],(xyzMin[1]+xyzMax[1])/2,xyzMax[2]);
            glVertex3d(xyzMax[0],(xyzMin[1]+xyzMax[1])/2,xyzMin[2]);
            glVertex3d(xyzMin[0],(xyzMin[1]+xyzMax[1])/2,xyzMax[2]);
        glEnd();
        */

    }
    if (zSym) {
        color[0] = 0.0;
        color[1] = 0.0;
        color[2] = 1.0;
        glColor3fv(color);
        glBegin(GL_LINE_LOOP);
            glVertex3d(xyzMin[0],xyzMin[1],(xyzMin[2]+xyzMax[2])/2);
            glVertex3d(xyzMax[0],xyzMin[1],(xyzMin[2]+xyzMax[2])/2);
            glVertex3d(xyzMax[0],xyzMax[1],(xyzMin[2]+xyzMax[2])/2);
            glVertex3d(xyzMin[0],xyzMax[1],(xyzMin[2]+xyzMax[2])/2);
        glEnd();

        /*glBegin(GL_LINES);
            glVertex3d(xyzMin[0],xyzMin[1],(xyzMin[2]+xyzMax[2])/2);
            glVertex3d(xyzMax[0],xyzMax[1],(xyzMin[2]+xyzMax[2])/2);
            glVertex3d(xyzMax[0],xyzMin[1],(xyzMin[2]+xyzMax[2])/2);
            glVertex3d(xyzMin[0],xyzMax[1],(xyzMin[2]+xyzMax[2])/2);
        glEnd();*/
    }
    glEnable(GL_LIGHTING);

}
/**********************************************************************/
void Renderer::plotThroughCutDirection(int throughCutDirection)
{
    if (throughCutDirection <= 0) return;
    double xyzMin[3],xyzMax[3],scale;

    scale = vox.getModelScale();
    double AxisLength = 0.05*scale;
    double startPt[]= {0,0,0},endPt1[] ={0,0,0},endPt2[] ={0,0,0};
    float color[3];
    glEnable(GL_LIGHTING);

    vox.getBoundingBox(xyzMin,xyzMax);
    startPt[0] = endPt1[0] = endPt2[0] = (xyzMin[0]+xyzMax[0])/2;
    startPt[1] = endPt1[1] = endPt2[1] = (xyzMin[1]+xyzMax[1])/2;
    startPt[2] = endPt1[2] = endPt2[2] = (xyzMin[2]+xyzMax[2])/2;


    color[0] = 65.0/255;color[1] = 175.0/255;color[2] = 244.0/255;

    if (throughCutDirection == 1) {
        AxisLength = max(AxisLength,1.5*(xyzMax[0] - xyzMin[0])/2);
        endPt1[0] += AxisLength;
        endPt2[0] -= AxisLength;
    }
    else if (throughCutDirection == 2) {
        AxisLength = max(AxisLength,1.5*(xyzMax[1] - xyzMin[1])/2);
        endPt1[1] += AxisLength;
        endPt2[1] -= AxisLength;    
    }
    else if (throughCutDirection == 3) {
        AxisLength = max(AxisLength,1.5*(xyzMax[2] - xyzMin[2])/2);
        endPt1[2] += AxisLength;
        endPt2[2] -= AxisLength;
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

    drawNiceArrow(startPt,endPt1,false,true);
    drawNiceArrow(startPt,endPt2,false,true);
    //drawHollowCylinder(endPt1,endPt2);

}
/**********************************************************************/
void Renderer::plotDrawDirection(int drawDirection)
{
    if (drawDirection <= 0) return;
    double xyzMin[3],xyzMax[3],scale;

    scale = vox.getModelScale();
    double AxisLength = 0.05*scale;
    double startPt[]= {0,0,0},endPt1[] ={0,0,0},endPt2[] ={0,0,0};
    float color[3];
    glEnable(GL_LIGHTING);

    vox.getBoundingBox(xyzMin,xyzMax);
    startPt[0] = endPt1[0] = endPt2[0] = (xyzMin[0]+xyzMax[0])/2;
    startPt[1] = endPt1[1] = endPt2[1] = (xyzMin[1]+xyzMax[1])/2;
    startPt[2] = endPt1[2] = endPt2[2] = (xyzMin[2]+xyzMax[2])/2;

    color[0] = 244.0/255.0;color[1] = 194.0/255.0;color[2] = 66.0/255.0;

    if (drawDirection == 1) {
        AxisLength = 1.5*(xyzMax[0] - xyzMin[0])/2;
        endPt1[0] += AxisLength;
        endPt2[0] -= AxisLength;
    }
    else if (drawDirection == 2) {
        AxisLength = 1.5*(xyzMax[1] - xyzMin[1])/2;
        endPt1[1] += AxisLength;
        endPt2[1] -= AxisLength; 
    }
    else if (drawDirection == 3) {
        AxisLength = 1.5*(xyzMax[2] - xyzMin[2])/2;
        endPt1[2] += AxisLength;
        endPt2[2] -= AxisLength;
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

    drawNiceArrow(startPt,endPt1,false,false);
    drawNiceArrow(startPt,endPt2,false,false);
}
/**********************************************************************/
void Renderer::plotGridPattern(int xGrid,int yGrid,int zGrid)
{
    int i;
    double xyzMin[3],xyzMax[3],range[3],xLoc,yLoc,zLoc;

    xGrid = max(1,xGrid);
    yGrid = max(1,yGrid);
    zGrid = max(1,zGrid);

    if (max(max(xGrid,yGrid),zGrid) == 1) return;// nothing to draw

    glDisable(GL_LIGHTING);
    vox.getBoundingBox(xyzMin,xyzMax);

    for (i = 0; i < 3;i++){
        range[i] = xyzMax[i] - xyzMin[i];
        xyzMax[i] += 0.005*range[i];
        xyzMin[i] -= 0.005*range[i];
    }

    glDisable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);

    glColor3f(0.5,0.5,0.5);
    glLineWidth(1.0);
    for (int xCounter = 0; xCounter <= xGrid; xCounter++){
        xLoc = xyzMin[0] + range[0]*xCounter/xGrid;
        glBegin(GL_LINE_LOOP);
            glVertex3d(xLoc,xyzMin[1],xyzMin[2]);
            glVertex3d(xLoc,xyzMax[1],xyzMin[2]);
            glVertex3d(xLoc,xyzMax[1],xyzMax[2]);
            glVertex3d(xLoc,xyzMin[1],xyzMax[2]);
            glVertex3d(xLoc,xyzMin[1],xyzMin[2]);
        glEnd();
    }

    for (int yCounter = 0; yCounter <= yGrid; yCounter++){
        yLoc = xyzMin[1] + range[1]*yCounter/yGrid;
        glBegin(GL_LINE_LOOP);
            glVertex3d(xyzMin[0],yLoc,xyzMin[2]);
            glVertex3d(xyzMax[0],yLoc,xyzMin[2]);
            glVertex3d(xyzMax[0],yLoc,xyzMax[2]);
            glVertex3d(xyzMin[0],yLoc,xyzMax[2]);
            glVertex3d(xyzMin[0],yLoc,xyzMin[2]);
        glEnd();
    }
    for (int zCounter = 0; zCounter <= zGrid; zCounter++){
        zLoc = xyzMin[2] + range[2]*zCounter/zGrid;
        glBegin(GL_LINE_LOOP);
            glVertex3d(xyzMin[0],xyzMin[1],zLoc);
            glVertex3d(xyzMax[0],xyzMin[1],zLoc);
            glVertex3d(xyzMax[0],xyzMax[1],zLoc);
            glVertex3d(xyzMin[0],xyzMax[1],zLoc);
            glVertex3d(xyzMin[0],xyzMin[1],zLoc);
        glEnd();
    }
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
}
/**********************************************************************/
void Renderer::plotCyclicSymmetryPlanes(int zCyclicSymmetry)
{
    int i;
    double xyzMin[3],xyzMax[3],range[3],x[4],y[4],z[4];
    GLfloat color[4];


    if (zCyclicSymmetry <= 1) return;

    glDisable(GL_LIGHTING);
    vox.getBoundingBox(xyzMin,xyzMax);

    for (i = 0; i < 3;i++) {
        range[i] = xyzMax[i] - xyzMin[i];
        xyzMax[i] += 0.02*range[i];
        xyzMin[i] -= 0.02*range[i];

    }
    glDisable(GL_CULL_FACE);
    double angleOfSymmetry,theta;
    angleOfSymmetry = 2*3.1415/zCyclicSymmetry;
    glEnable(GL_LIGHTING);
    color[0] = 0.3;
    color[1] = 0.3;
    color[2] = 0.3;
    color[3] = 1.0;
    x[0] = x[3] = (xyzMin[0] + xyzMax[0])/2.0;
    y[0] = y[3] = (xyzMin[1] + xyzMax[1])/2.0;
    z[0] = xyzMin[2];
    z[1] = xyzMin[2];
    z[2] = xyzMax[2];
    z[3] = xyzMax[2];

    double radius = xyzMax[0] - (xyzMin[0] + xyzMax[0])/2.0;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    for (int i = 0; i < zCyclicSymmetry; i++){
        theta = i*angleOfSymmetry;
        x[1] = (xyzMin[0] + xyzMax[0])/2.0 + radius*cos(theta);
        x[2] = (xyzMin[0] + xyzMax[0])/2.0 + radius*cos(theta);
        y[1] = (xyzMin[1] + xyzMax[1])/2.0 + radius*sin(theta);
        y[2] = (xyzMin[1] + xyzMax[1])/2.0 + radius*sin(theta);
        glBegin(GL_LINE_LOOP);
            glVertex3d(x[0],y[0],z[0]);
            glVertex3d(x[1],y[1],z[1]);
            glVertex3d(x[2],y[2],z[2]);
            glVertex3d(x[3],y[3],z[3]);
        glEnd();
    }

    glEnable(GL_LIGHTING);
    //glDisable( GL_BLEND );
    glEnable(GL_CULL_FACE);

}
/**********************************************************************/
void Renderer::plotBoundingBox()
{
    double xyzMin[3], xyzMax[3];

    glDisable(GL_LIGHTING);

    vox.getBoundingBox(xyzMin,xyzMax);
    glLineWidth(1.0);
    glColor3f(0.9,0.2,0.0);
    glBegin(GL_LINES);
    glVertex3d(xyzMin[0],xyzMin[1],xyzMin[2]);
    glVertex3d(xyzMax[0],xyzMin[1],xyzMin[2]);

    glVertex3d(xyzMin[0],xyzMin[1],xyzMin[2]);
    glVertex3d(xyzMin[0],xyzMax[1],xyzMin[2]);

    glVertex3d(xyzMin[0],xyzMin[1],xyzMin[2]);
    glVertex3d(xyzMin[0],xyzMin[1],xyzMax[2]);

    glVertex3d(xyzMax[0],xyzMin[1],xyzMin[2]);
    glVertex3d(xyzMax[0],xyzMax[1],xyzMin[2]);

    glVertex3d(xyzMax[0],xyzMin[1],xyzMin[2]);
    glVertex3d(xyzMax[0],xyzMin[1],xyzMax[2]);

    glVertex3d(xyzMin[0],xyzMax[1],xyzMin[2]);
    glVertex3d(xyzMax[0],xyzMax[1],xyzMin[2]);

    glVertex3d(xyzMin[0],xyzMax[1],xyzMin[2]);
    glVertex3d(xyzMin[0],xyzMax[1],xyzMax[2]);

    glVertex3d(xyzMin[0],xyzMin[1],xyzMax[2]);
    glVertex3d(xyzMax[0],xyzMin[1],xyzMax[2]);

    glVertex3d(xyzMin[0],xyzMin[1],xyzMax[2]);
    glVertex3d(xyzMin[0],xyzMax[1],xyzMax[2]);

    glVertex3d(xyzMax[0],xyzMin[1],xyzMax[2]);
    glVertex3d(xyzMax[0],xyzMax[1],xyzMax[2]);

    glVertex3d(xyzMin[0],xyzMax[1],xyzMax[2]);
    glVertex3d(xyzMax[0],xyzMax[1],xyzMax[2]);

    glVertex3d(xyzMax[0],xyzMax[1],xyzMin[2]);
    glVertex3d(xyzMax[0],xyzMax[1],xyzMax[2]);

    glEnd();


    char string[256];
    glColor3f(0.1,0.1,0.1);
    for (int i = 0; i < 3; i++)
        if (abs(xyzMin[i]) < 1e-5)
            xyzMin[i] = 0;// round off to 0

    sprintf(string,"(%.2g,%.2g,%.2g)",xyzMin[0],xyzMin[1],xyzMin[2]);
    double pos[3];
    pos[0] = xyzMin[0];
    pos[1] = xyzMin[1];
    pos[2] = xyzMin[2];
    draw3DText(string, pos,0.6);


    sprintf(string,"(%.2g,%.2g,%.2g)",xyzMax[0],xyzMax[1],xyzMax[2]);
    pos[0] = xyzMax[0];
    pos[1] = xyzMax[1];
    pos[2] = xyzMax[2];
    draw3DText(string, pos,0.6);

    glEnable(GL_LIGHTING);
}
/****************************************************************/
void Renderer::selectSTLGeomTriangles(void)
{
    double xTri[3],yTri[3],zTri[3];
    int part;

    // In the render mode
    glBegin(GL_TRIANGLES);
    for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
        part = vox.getParentPartOfTriangle(tri);
        if ( (!liveVar.plotNonDesignGeometry) && (!topopt.shouldPartBeOptimized(part))) continue;
        vox.getTriPts(tri,xTri,yTri,zTri);
        glPushName(tri);
        glVertex3f(xTri[0],yTri[0],zTri[0]);
        glVertex3f(xTri[1],yTri[1],zTri[1]);
        glVertex3f(xTri[2],yTri[2],zTri[2]);
        glPopName();
    }
    glEnd();
}
/****************************************************************/
void Renderer::plotSTLGeom(bool displayEdges)
{
    double xTri[3],yTri[3],zTri[3],p01[3],p02[3],triNormal[3];
    double xEdge[2],yEdge[2],zEdge[2],minLength,maxLength;
    char geomData[2056],tempstr[512],materialName[256];
    GLfloat color[4] ={0,0, 0, 0.25};
    int nGroups,group,part,triGroup;
    double boundingBox[3][2];
    double XCuttingPlane,YCuttingPlane,ZCuttingPlane,triCenter[3];

    glPushMatrix();
    // Provide basic information on the geometry
    sprintf(geomData,"Name: %s \nTriangles: %d\nParts: %d\n",liveVar.modelName.c_str(),vox.getNumTriangles(),vox.getNumParts() );

    vox.getBoundingBox(boundingBox);

    vox.getMinAndMaxLengths(&minLength,&maxLength);
    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        sprintf(tempstr,"Units: m,N,s\nMax length: %.3f meters\n",maxLength);
        break;
    case UNITS_MM:
        sprintf(tempstr,"Units: mm,N,s\nMax length: %.3f mms\n",maxLength);
        break;
    case UNITS_INCH:
        sprintf(tempstr,"Units: in,lb,s\nMax length: %.3f inches\n",maxLength);
        break;
    }
    strcat(geomData,tempstr);
    for (int part = 0; part <vox.getNumParts(); part++) {
        topopt.getMaterialNameOfPart(part,materialName);
        sprintf(tempstr,"Part %d: %s\n",part,materialName);
        strcat(geomData,tempstr);
    }

    //if (!liveVar.geomTransparent) plotDataString(geomData);// if transparent then we may be displaying stresses etc

    nGroups = vox.getNumTriGroups();

    XCuttingPlane = boundingBox[0][0] + liveVar.xCuttingPlanePercent*(boundingBox[0][1] - boundingBox[0][0])/100.0 - 1e-6;
    YCuttingPlane = boundingBox[1][0] + liveVar.yCuttingPlanePercent*(boundingBox[1][1] - boundingBox[1][0])/100.0 - 1e-6;
    ZCuttingPlane = boundingBox[2][0] + liveVar.zCuttingPlanePercent*(boundingBox[2][1] - boundingBox[2][0])/100.0 - 1e-6;


    glLineWidth(2.0);
    std::vector<int>::iterator it;
    for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
        part = vox.getParentPartOfTriangle(tri);

        it = std::find (liveVar.partsToHide.begin(), liveVar.partsToHide.end(), part);
        if (it != liveVar.partsToHide.end())
            continue;

        if ( (!liveVar.plotNonDesignGeometry) &&	(!topopt.shouldPartBeOptimized(part))) continue;

        triGroup = vox.getTriGroup(tri);
        vox.getTriPts(tri,xTri,yTri,zTri);
        if ((xTri[0] < XCuttingPlane) || (xTri[1] < XCuttingPlane) || (xTri[2] < XCuttingPlane))
            continue;
        if ((yTri[0] < YCuttingPlane) || (yTri[1] < YCuttingPlane) || (yTri[2] < YCuttingPlane))
            continue;
        if ((zTri[0] < ZCuttingPlane) || (zTri[1] < ZCuttingPlane) || (zTri[2] < ZCuttingPlane))
            continue;

        group = vox.getTriGroup(tri);
        p01[0] = xTri[1]-xTri[0];
        p01[1] = yTri[1]-yTri[0];
        p01[2] = zTri[1]-zTri[0];

        p02[0] = xTri[2]-xTri[0];
        p02[1] = yTri[2]-yTri[0];
        p02[2] = zTri[2]-zTri[0];

        CROSS3D(triNormal,p01,p02);


        if (vox.isTriangleHighlighted(tri)){
            color[0] = 1;
            color[1] = 0;
            color[2] = 0;
        }
        else{
            getColorOfPart(part,color);// use group ID to color
        }
        glDisable(GL_COLOR_MATERIAL);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
        glNormal3f(triNormal[0],triNormal[1],triNormal[2]);

        glPolygonMode(GL_FRONT, GL_FILL);

        glBegin(GL_TRIANGLES);
        glVertex3f(xTri[0],yTri[0],zTri[0]);
        glVertex3f(xTri[1],yTri[1],zTri[1]);
        glVertex3f(xTri[2],yTri[2],zTri[2]);
        glEnd();

        if (displayEdges){
            glEnable(GL_COLOR_MATERIAL);
            glColor3f(0.0f,0.0f,0.0f);
            glPolygonMode(GL_FRONT, GL_LINE);
            glBegin(GL_TRIANGLES);
                glVertex3f(xTri[0],yTri[0],zTri[0]);
                glVertex3f(xTri[1],yTri[1],zTri[1]);
                glVertex3f(xTri[2],yTri[2],zTri[2]);
            glEnd();
        }
        glPolygonMode(GL_FRONT, GL_FILL);
    }
    glEnable(GL_COLOR_MATERIAL);
    glDisable( GL_BLEND );
    glLineWidth(2.0);
    glColor3f(0,0,0);
    //plot Sharp edges
    for (int e = 0; e < vox.getNumSharpEdges(); e++) {
        vox.getSharpEdgePoints(e,xEdge,yEdge,zEdge);
        if (xEdge[0] < XCuttingPlane)
            continue;
        if (yEdge[1] < YCuttingPlane)
            continue;
        if (zEdge[2] < ZCuttingPlane)
            continue;
        glBegin(GL_LINES);
        glVertex3d(xEdge[0],yEdge[0],zEdge[0]);
        glVertex3d(xEdge[1],yEdge[1],zEdge[1]);
        glEnd();
    }
    if (liveVar.showConcaveEdges) {
        glDisable(GL_DEPTH_TEST);
        //plot Concave edges
        for (int e = 0; e < vox.getNumSharpEdges(); e++) {
            if (vox.isEdgeConcave(e)){
                vox.getSharpEdgePoints(e,xEdge,yEdge,zEdge);
                glLineWidth(4.0);
                glColor3f(1,0,0);
                glBegin(GL_LINES);
                glVertex3d(xEdge[0],yEdge[0],zEdge[0]);
                glVertex3d(xEdge[1],yEdge[1],zEdge[1]);
                glEnd();
            }
        }
        glEnable(GL_DEPTH_TEST);
    }
    glDisable(GL_COLOR_MATERIAL);
    glPopMatrix();

}
/****************************************************************/
void Renderer::plotTempGeom(bool displayEdges)
{
    double xTri[3],yTri[3],zTri[3],p01[3],p02[3],triNormal[3];
    double xEdge[2],yEdge[2],zEdge[2],minLength,maxLength;
    char geomData[2056],tempstr[512],materialName[256];
    GLfloat color[4] ={0,0, 0, 0.2};
    int nGroups,group,part;


    glPushMatrix();
    // Provide basic information on the geometry
    sprintf(geomData,"Name: %s \nTriangles: %d\nParts: %d\n",liveVar.modelName.c_str(),voxTemp.getNumTriangles(),voxTemp.getNumParts() );


    voxTemp.getMinAndMaxLengths(&minLength,&maxLength);
    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        sprintf(tempstr,"Units: m,N,s\nMax length: %.3f meters\n",maxLength);
        break;
    case UNITS_MM:
        sprintf(tempstr,"Units: mm,N,s\nMax length: %.3f mms\n",maxLength);
        break;
    case UNITS_INCH:
        sprintf(tempstr,"Units: in,lb,s\nMax length: %.3f inches\n",maxLength);
        break;
    }
    strcat(geomData,tempstr);

    for (int part = 0; part <voxTemp.getNumParts(); part++) {
        topopt.getMaterialNameOfPart(part,materialName);
        sprintf(tempstr,"Part %d: %s\n",part,materialName);
        strcat(geomData,tempstr);
    }


    nGroups = voxTemp.getNumTriGroups();
    int offset = 16;// to change color
    for (int tri = 0; tri < voxTemp.getNumTriangles(); tri++) {
        part = voxTemp.getParentPartOfTriangle(tri);
        if ( (!liveVar.plotNonDesignGeometry) &&	(!topopt.shouldPartBeOptimized(part))) continue;
        voxTemp.getTriPts(tri,xTri,yTri,zTri);
        group = voxTemp.getTriGroup(tri);
        p01[0] = xTri[1]-xTri[0];
        p01[1] = yTri[1]-yTri[0];
        p01[2] = zTri[1]-zTri[0];

        p02[0] = xTri[2]-xTri[0];
        p02[1] = yTri[2]-yTri[0];
        p02[2] = zTri[2]-zTri[0];
        CROSS3D(triNormal,p01,p02);


        if (voxTemp.isTriangleHighlighted(tri)){
            color[0] = 1;
            color[1] = 0;
            color[2] = 0;
        }
        else{
            getColorOfPart(part+offset,color);
        }
        glDisable(GL_COLOR_MATERIAL);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
        glNormal3f(triNormal[0],triNormal[1],triNormal[2]);

        glPolygonMode(GL_FRONT, GL_FILL);

        glBegin(GL_TRIANGLES);
        glVertex3f(xTri[0],yTri[0],zTri[0]);
        glVertex3f(xTri[1],yTri[1],zTri[1]);
        glVertex3f(xTri[2],yTri[2],zTri[2]);
        glEnd();

        if (displayEdges){
            glEnable(GL_COLOR_MATERIAL);
            glColor3f(0.0f,0.0f,0.0f);
            glPolygonMode(GL_FRONT, GL_LINE);
            glBegin(GL_TRIANGLES);
                glVertex3f(xTri[0],yTri[0],zTri[0]);
                glVertex3f(xTri[1],yTri[1],zTri[1]);
                glVertex3f(xTri[2],yTri[2],zTri[2]);
            glEnd();
        }
        glPolygonMode(GL_FRONT, GL_FILL);


    }
    if (displayEdges) {
    glEnable(GL_COLOR_MATERIAL);
    glDisable( GL_BLEND );
    glLineWidth(2.0);
    glColor3f(0,0,0);

    //plot Sharp edges
    for (int e = 0; e < voxTemp.getNumSharpEdges(); e++) {
        voxTemp.getSharpEdgePoints(e,xEdge,yEdge,zEdge);
        glBegin(GL_LINES);
        glVertex3d(xEdge[0],yEdge[0],zEdge[0]);
        glVertex3d(xEdge[1],yEdge[1],zEdge[1]);
        glEnd();
    }
    if (liveVar.showConcaveEdges) {
        glDisable(GL_DEPTH_TEST);
        //plot Concave edges
        for (int e = 0; e < voxTemp.getNumSharpEdges(); e++) {
            if (voxTemp.isEdgeConcave(e)){
                voxTemp.getSharpEdgePoints(e,xEdge,yEdge,zEdge);
                glLineWidth(4.0);
                glColor3f(1,0,0);
                glBegin(GL_LINES);
                glVertex3d(xEdge[0],yEdge[0],zEdge[0]);
                glVertex3d(xEdge[1],yEdge[1],zEdge[1]);
                glEnd();
            }
        }
        glEnable(GL_DEPTH_TEST);
    }
    glDisable(GL_COLOR_MATERIAL);
    }
    glPopMatrix();
    plotSIMPResults();

}
/**********************************************************************/
void Renderer::plotElasticityBC(void)
{
    int  j,tri,group,bcTypeOfTri[3],tri0,skip, nTrianglesInGroup;
    double x[3],y[3],z[3],center[3],bcValueOfTri[3],nrm,endPt[3],scale,temp[3],axisPt[3],dir[3],tempScalar,pressureData[3],radius;
    double triNormal[3],triNormal0[3],dtprd,loadPt[3];
    float color[3];
    bool isPlanar,textPlotted;

    glEnable( GL_LIGHTING );
    scale =  0.05*vox.getModelScale();
    int bcSet = liveVar.currentBCSet;
	int maxTrianglesToMark = 5;
    //printf("Plotting for BC %d \n",bcSet);
    for (group = 1; group < vox.getNumTriGroups(); group++) {
        textPlotted  = false;
        tri0 = -1;
        isPlanar = true;
        nTrianglesInGroup= 0;
        for (tri = 0; tri < vox.getNumTriangles(); tri++) {
            if (vox.getTriGroup(tri) != group) continue;
            nTrianglesInGroup++;
            vox.getTriPts(tri,x,y,z);
            if (tri0 == -1)// first triangle
                vox.getTriangleNormal(x,y,z,triNormal0);
            else {
                vox.getTriangleNormal(x,y,z,triNormal);
                if (DOTPROD3D(triNormal0,triNormal) < 0.99)
                    isPlanar = false;
            }
            tri0 = tri;// found one triangle with that group
        }
        if (tri0==-1) continue;// no triangle in this group
        for ( j =0; j < 3; j++) {
            bcTypeOfTri[j] = liveVar.BCType[bcSet][group][j];
            bcValueOfTri[j] = liveVar.BCValue[bcSet][group][j];
            pressureData[j] = liveVar.BCValue[bcSet][group][j+3];//only for pressures
            axisPt[j] = liveVar.BCValue[bcSet][group][j+3];//only for torques
            loadPt[j] = liveVar.BCValue[bcSet][group][j+3];//only for remote loads
        }
        nrm = NORM3D(bcValueOfTri);

        if (nTrianglesInGroup < maxTrianglesToMark)
            skip = 1;//mark all
        else
            skip = int(1.0*nTrianglesInGroup/ maxTrianglesToMark) + 1;// don't mark on all triangles ... ugly

        for (tri = 0; tri < vox.getNumTriangles(); tri+=skip) {
            if (vox.getTriGroup(tri) != group) continue;
            vox.getTriPts(tri,x,y,z);
            center[0] = (x[0]+x[1]+x[2])/3.0;
            center[1] = (y[0]+y[1]+y[2])/3.0;
            center[2] = (z[0]+z[1]+z[2])/3.0;

            if ( (bcTypeOfTri[0] == ELASTICITY_DIRICHLET) ||
                 (bcTypeOfTri[1] == ELASTICITY_DIRICHLET) ||
                 (bcTypeOfTri[2] == ELASTICITY_DIRICHLET)) {
#ifdef PARETOCREO
                // plot blue cones
                color[0] = 0;
                color[1] = 0;
                color[2] = 80.0/256.0;
                double dir[3],height,baseRadius;
                height = scale*0.2;
                baseRadius = scale*0.1;
                double basePt[3];

                if (bcTypeOfTri[0] == ELASTICITY_DIRICHLET) {
                    if (triNormal[0] < 0)
                        dir[0] = -1.0;
                    else
                        dir[0] = 1.0;
                    dir[1] = 0.0;
                    dir[2] = 0.0;
                    basePt[0] = center[0] + height*dir[0];
                    basePt[1] = center[1] + height*dir[1];
                    basePt[2] = center[2] + height*dir[2];
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                    drawNiceCone(basePt,dir,height,baseRadius);
                }
                if (bcTypeOfTri[1] == ELASTICITY_DIRICHLET) {
                    dir[0] = 0.0;
                    if (triNormal[1] < 0)
                        dir[1] = -1.0;
                    else
                        dir[1] = 1.0;
                    dir[2] = 0.0;
                    basePt[0] = center[0] + height*dir[0];
                    basePt[1] = center[1] + height*dir[1];
                    basePt[2] = center[2] + height*dir[2];
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                    drawNiceCone(basePt,dir,height,baseRadius);
                }
                if (bcTypeOfTri[2] == ELASTICITY_DIRICHLET) {
                    dir[0] = 0.0;
                    dir[1] = 0.0;
                    if (triNormal[2] < 0)
                        dir[2] = -1.0;
                    else
                        dir[2] = 1.0;
                    basePt[0] = center[0] + height*dir[0];
                    basePt[1] = center[1] + height*dir[1];
                    basePt[2] = center[2] + height*dir[2];
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                    drawNiceCone(basePt,dir,height,baseRadius);
                }

#else

                color[0] = 0;
                color[1] = 1;
                color[2] = 0;
                if (bcTypeOfTri[0] == ELASTICITY_DIRICHLET) {
                    endPt[0] = center[0] + 0.25*scale;
                    endPt[1] = center[1];
                    endPt[2] = center[2];
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                    drawNiceArrow(center,endPt,true,false,0.15);
                }
                if (bcTypeOfTri[1] == ELASTICITY_DIRICHLET) {
                    endPt[0] = center[0] ;
                    endPt[1] = center[1]+ 0.25*scale;
                    endPt[2] = center[2];
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                    drawNiceArrow(center,endPt,true,false,0.15);
                }
                if (bcTypeOfTri[2] == ELASTICITY_DIRICHLET) {
                    endPt[0] = center[0] ;
                    endPt[1] = center[1];
                    endPt[2] = center[2]+ 0.25*scale;
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                    drawNiceArrow(center,endPt,true,false,0.15);
                }
#endif
            }
            if ( (bcTypeOfTri[0] == ELASTICITY_RETAIN) ||
                 (bcTypeOfTri[1] == ELASTICITY_RETAIN) ||
                 (bcTypeOfTri[2] == ELASTICITY_RETAIN)) {
                color[0] = 0.0;
                color[1] = 0.0;
                color[2] = 0.8;

                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                radius = 0.1*scale;
                drawNiceSphere(center,radius);
            }
            if (((bcTypeOfTri[0] == ELASTICITY_NEUMANN) || (bcTypeOfTri[0] == ELASTICITY_BEARING_LOAD)) && (nrm > 0)) {

#ifdef PARETOCREO
                color[0] =  205.0/255.0;
                color[1] = 133.0/255.0;//matches Creo
                color[2] = 63.0/255.0;
#else

                if (bcTypeOfTri[0] == ELASTICITY_BEARING_LOAD){
                    color[0] = 105.0/255.0;
                    color[1] = 160.0/255.0;
                    color[2] = 200.0/255.0;
                }
                else {
                    color[0] =  205.0/255.0;
                    color[1] = 20.0/255.0;//matches SolidWorks
                    color[2] = 147.0/255.0;
                }
#endif

                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                endPt[0] = center[0] + scale*bcValueOfTri[0]/nrm;
                endPt[1] = center[1] + scale*bcValueOfTri[1]/nrm;
                endPt[2] = center[2] + scale*bcValueOfTri[2]/nrm;
                // Check against normal of triangle
                dtprd = DOTPROD3D(bcValueOfTri,triNormal);
                for (int i = 0; i < 3;i++)
                    temp[i] = endPt[i]-center[i];
                if ((isPlanar) & (dtprd < 0)) {// shift arrow so that it is visible outside the body
                    for (int i = 0; i < 3;i++) {
                        endPt[i] -= 1.2*temp[i];
                        center[i] -= 1.2*temp[i];
                    }
                }
                drawNiceArrow(center,endPt,false,false);
                if (!textPlotted){
                    char forceStr[256];
                    double forceLocation[3];
                    if ((nrm >= 0.1) && (nrm < 1e5))
                        sprintf(forceStr,"%.2f",nrm);
                    else
                        sprintf(forceStr,"%.2e",nrm);
                    switch (liveVar.paretoUnits) {
                    case UNITS_METER:
                        strcat(forceStr," N");
                        break;
                    case UNITS_MM:
                        strcat(forceStr," N");
                        break;
                    case UNITS_INCH:
                        strcat(forceStr," lb");
                        break;
                    }
                    if (((isPlanar) && (dtprd < 0)) || (bcValueOfTri[0] < 0)) {
                        for (int i = 0; i < 3; i++)
                            forceLocation[i] = center[i] - 0.5*(endPt[i] - center[i]);
                        draw3DText(forceStr,forceLocation);
                    }
                    else {
                        for (int i = 0; i < 3; i++)
                            forceLocation[i] = endPt[i] + 0.5*(endPt[i] - center[i]);
                        draw3DText(forceStr,forceLocation);
                    }
                    textPlotted = true;// plot only once
                }
            }
            if ((bcTypeOfTri[0] == ELASTICITY_OUTPUT) && (nrm > 0)) {
                color[0] = 0;
                color[1] = 1.0;
                color[2] = 0.0;
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                endPt[0] = center[0] + scale*bcValueOfTri[0]/nrm;
                endPt[1] = center[1] + scale*bcValueOfTri[1]/nrm;
                endPt[2] = center[2] + scale*bcValueOfTri[2]/nrm;
                // Check against normal of triangle
                dtprd = DOTPROD3D(bcValueOfTri,triNormal);
                for (int i = 0; i < 3;i++)
                    temp[i] = endPt[i]-center[i];
                if ((isPlanar) && (dtprd < 0)) {// shift arrow so that it is visible outside the body
                    for (int i = 0; i < 3;i++) {
                        endPt[i] -= 1.2*temp[i];
                        center[i] -= 1.2*temp[i];
                    }
                }
                drawNiceArrow(center,endPt,false,false);
            }
            if ((bcTypeOfTri[0] == ELASTICITY_PRESSURE) ) {
#ifdef PARETOCREO
				color[0] = 205.0 / 255.0;
				color[1] = 133.0 / 255.0;//matches Creo
				color[2] = 63.0 / 255.0;
#else
				color[0] = 1.0;
				color[1] = 20.0 / 255.0;//matches SolidWorks
				color[2] = 147.0 / 255.0;
#endif
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                if (isPlanar) {
                    endPt[0] = center[0] + scale*bcValueOfTri[0]/nrm;
                    endPt[1] = center[1] + scale*bcValueOfTri[1]/nrm;
                    endPt[2] = center[2] + scale*bcValueOfTri[2]/nrm;
                    // Check against normal of triangle
                    dtprd = DOTPROD3D(bcValueOfTri,triNormal);
                    for (int i = 0; i < 3;i++)
                        temp[i] = endPt[i]-center[i];
                    if (dtprd < 0) {// shift arrow so that it is visible outside the body
                        for (int i = 0; i < 3;i++) {
                            endPt[i] -= 1.2*temp[i];
                            center[i] -= 1.2*temp[i];
                        }
                    }
                    drawNiceArrow(center,endPt,false,false);
                }
                else {// non planar surface
                    vox.getNormalOfTriangle(tri,triNormal);
                    endPt[0] = center[0] + scale*triNormal[0];
                    endPt[1] = center[1] + scale*triNormal[1];
                    endPt[2] = center[2] + scale*triNormal[2];
                    for (int i = 0; i < 3;i++)
                        temp[i] = endPt[i]-center[i];
                    for (int i = 0; i < 3;i++) {
                        endPt[i] += .2*temp[i];
                        center[i] += .2*temp[i];
                    }
                    drawNiceArrow(endPt,center,false,false);
                }

                if (!textPlotted){
                    char forceStr[256];
                    double forceLocation[3],pressure;
                    pressure = pressureData[0];
                    if ((pressure >= 0.1) && (pressure < 1e5))
                        sprintf(forceStr,"%.1f",pressure);
                    else
                        sprintf(forceStr,"%.1e",pressure);
                    switch (liveVar.paretoUnits) {
                    case UNITS_METER:
                        strcat(forceStr," N/m^2");
                        break;
                    case UNITS_MM:
                        strcat(forceStr," N/mm^2");
                        break;
                    case UNITS_INCH:
                        strcat(forceStr," lb/in^2");
                        break;
                    }
                    if ((isPlanar) & (dtprd < 0))  {
                        for (int i = 0; i < 3; i++)
                            forceLocation[i] = center[i] - 0.5*(endPt[i] - center[i]);
                        draw3DText(forceStr,forceLocation);
                    }
                    else {
                        for (int i = 0; i < 3; i++)
                            forceLocation[i] = endPt[i] + 0.5*(endPt[i] - center[i]);
                        draw3DText(forceStr,forceLocation);
                    }
                    textPlotted = true;// plot only
                }
            }
            if ((bcTypeOfTri[0] == ELASTICITY_TORQUE) && (nrm > 0)) {
#ifdef PARETOCREO
				color[0] = 205.0 / 255.0;
				color[1] = 133.0 / 255.0;//matches Creo
				color[2] = 63.0 / 255.0;
#else
				color[0] = 1.0;
				color[1] = 20.0 / 255.0;//matches SolidWorks
				color[2] = 147.0 / 255.0;
#endif
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                SUBTRACT3D(temp,center,axisPt);
                CROSS3D(dir,bcValueOfTri,temp);//direction of arrow
                tempScalar = NORM3D(dir);
                endPt[0] = center[0] - scale*dir[0]/tempScalar;
                endPt[1] = center[1] - scale*dir[1]/tempScalar;
                endPt[2] = center[2] - scale*dir[2]/tempScalar;
                drawNiceArrow(center,endPt,false,false);
                if (!textPlotted){
                    char forceStr[256];
                    double forceLocation[3];
                    if ((nrm >= 0.1) && (nrm < 1e5))
                        sprintf(forceStr,"%.1f",nrm);
                    else
                        sprintf(forceStr,"%.1e",nrm);
                    switch (liveVar.paretoUnits) {
                    case UNITS_METER:
                        strcat(forceStr," N-m");
                        break;
                    case UNITS_MM:
                        strcat(forceStr," N-mm");
                        break;
                    case UNITS_INCH:
                        strcat(forceStr," lb-in");
                        break;
                    }
                    for (int i = 0; i < 3; i++)
                        forceLocation[i] = endPt[i] + 0.5*(endPt[i] - center[i]);
                    draw3DText(forceStr,forceLocation);
                    textPlotted = true;// plot only once
                }
            }
            if ((bcTypeOfTri[0] == ELASTICITY_REMOTE_LOAD) && (nrm > 0)) {
#ifdef PARETOCREO
                color[0] = 205.0 / 255.0;
                color[1] = 133.0 / 255.0;//matches Creo
                color[2] = 63.0 / 255.0;
#else
                color[0] = 1.0;
                color[1] = 20.0 / 255.0;//matches SolidWorks
                color[2] = 147.0 / 255.0;
#endif
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

                endPt[0] = loadPt[0] + scale*bcValueOfTri[0]/nrm;
                endPt[1] = loadPt[1] + scale*bcValueOfTri[1]/nrm;
                endPt[2] = loadPt[2] + scale*bcValueOfTri[2]/nrm;
                drawNiceArrow(loadPt,endPt,false,false);
                drawLine(loadPt,center);
                if (!textPlotted){
                    char forceStr[256];
                    double forceLocation[3];
                    if ((nrm >= 0.1) && (nrm < 1e5))
                        sprintf(forceStr,"%.1f",nrm);
                    else
                        sprintf(forceStr,"%.1e",nrm);
                    switch (liveVar.paretoUnits) {
                    case UNITS_METER:
                        strcat(forceStr," N");
                        break;
                    case UNITS_MM:
                        strcat(forceStr," N");
                        break;
                    case UNITS_INCH:
                        strcat(forceStr," lb");
                        break;
                    }
                    for (int i = 0; i < 3; i++)
                        forceLocation[i] = endPt[i] + 0.5*(endPt[i] - loadPt[i]);
                    draw3DText(forceStr,forceLocation);
                    textPlotted = true;// plot only once
                }
            }
        }
    }
    if (NORM3D(liveVar.ElasticityAcceleration[bcSet]) > 0){
        double partCenter[3],nrm,xyzMin[3],xyzMax[3],accelerationTextLocation[3];
        nrm = NORM3D(liveVar.ElasticityAcceleration[bcSet]);
        color[0] = 255.0;
        color[1] = 0.0;//matches SolidWorks
        color[2] = 0.0;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        vox.getBoundingBox(xyzMin,xyzMax);
        for (int i = 0; i < 3; i++)
            partCenter[i] = (xyzMin[i] + xyzMax[i])/2;

        endPt[0] = partCenter[0] + scale*liveVar.ElasticityAcceleration[bcSet][0]/nrm;
        endPt[1] = partCenter[1] + scale*liveVar.ElasticityAcceleration[bcSet][1]/nrm;
        endPt[2] = partCenter[2] + scale*liveVar.ElasticityAcceleration[bcSet][2]/nrm;
        drawNiceArrow(partCenter,endPt,false,false);

        drawNiceSphere(partCenter,0.2*scale);

        char accelerationStr[256];
        if ((nrm >= 0.1) && (nrm < 1e5))
            sprintf(accelerationStr,"%.1f",nrm);
        else
            sprintf(accelerationStr,"%.1e",nrm);
        switch (liveVar.paretoUnits) {
        case UNITS_METER:
            strcat(accelerationStr," m/s^2");
            break;
        case UNITS_MM:
            strcat(accelerationStr," mm/s^2");
            break;
        case UNITS_INCH:
            strcat(accelerationStr," in/s^2");
            break;
        }
        for (int i = 0; i < 3; i++)
            accelerationTextLocation[i] = partCenter[i] + 0.5*(partCenter[i] - endPt[i]);
        draw3DText(accelerationStr,accelerationTextLocation);
    }
    if (fabs(liveVar.ElasticityAcceleration[bcSet][3]) > 0){// radial acceleration
        double partCenter[3],nrm,xyzMin[3],xyzMax[3],accelerationTextLocation[3];

        nrm = fabs(liveVar.ElasticityAcceleration[bcSet][3]);
        color[0] = 255.0;
        color[1] = 0.0;//matches SolidWorks
        color[2] = 0.0;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        vox.getBoundingBox(xyzMin,xyzMax);
        for (int i = 0; i < 3; i++)
            partCenter[i] = (xyzMin[i] + xyzMax[i])/2;

        endPt[0] = partCenter[0];
        endPt[1] = partCenter[1];
        endPt[2] = partCenter[2] + scale;// z axis
        if (liveVar.ElasticityAcceleration[bcSet][3] > 0)
            drawNiceArrow(partCenter,endPt,false,true);
        else
            drawNiceArrow(endPt,partCenter,false,true);


        char accelerationStr[256];
        if ((nrm >= 0.1) && (nrm < 1e5))
            sprintf(accelerationStr,"%.1f (rad/sec)",nrm);
        else
            sprintf(accelerationStr,"%.1e (rad/sec)",nrm);

        for (int i = 0; i < 3; i++)
            accelerationTextLocation[i] = endPt[i] + 0.5*(endPt[i] - partCenter[i]);
        draw3DText(accelerationStr,accelerationTextLocation);
    }
    glEnable(GL_LIGHTING);
    glPointSize(1.0);
    glLineWidth(1.0);
    glDisable(GL_BLEND);

    if (liveVar.plotAxis) drawAxis();
    if (liveVar.plotTopOptConstraints)
        plotSymmetryPlanes(liveVar.topoptConstraints.xSymmetry,
                           liveVar.topoptConstraints.ySymmetry,
                           liveVar.topoptConstraints.zSymmetry);
}
/**********************************************************************/
void Renderer::plotThermalBC(void)
{
    int tri,group,bcTypeOfTri,tri0,skip, nTrianglesInGroup;
    double x[3],y[3],z[3],center[3],bcValueOfTri,endPt[3],scale;
    double triNormal[3],triNormal0[3];
    float color[3];
    bool isPlanar,textPlotted;
    string tempUnitsName,heatFluxName,heatCoefficientName;
    coreFunctions.getPropertyUnits(PROPERTY_TEMPERATURE,tempUnitsName);
    coreFunctions.getPropertyUnits(PROPERTY_HEAT_FLUX,heatFluxName);
    coreFunctions.getPropertyUnits(PROPERTY_HEAT_COEFFICIENT,heatCoefficientName);


    glEnable( GL_LIGHTING );
    scale =  0.02*vox.getModelScale();
    int bcSet = liveVar.currentBCSet;
    for (group = 1; group < vox.getNumTriGroups(); group++) {
        textPlotted  = false;
        tri0 = -1;
        isPlanar = true;
        nTrianglesInGroup= 0;
        for (tri = 0; tri < vox.getNumTriangles(); tri++) {
            if (vox.getTriGroup(tri) != group) continue;
            nTrianglesInGroup++;
            vox.getTriPts(tri,x,y,z);
            if (tri0 == -1)// first triangle
                vox.getTriangleNormal(x,y,z,triNormal0);
            else {
                vox.getTriangleNormal(x,y,z,triNormal);
                if (DOTPROD3D(triNormal0,triNormal) < 0.99)
                    isPlanar = false;
            }
            tri0 = tri;// found one triangle with that group
        }
        if (tri0==-1) continue;// no triangle in this group

        bcTypeOfTri = liveVar.ThermalBCType[bcSet][group];
        bcValueOfTri = liveVar.ThermalBCValue[bcSet][group];

        if (nTrianglesInGroup < 5)
            skip = 1;//mark all
        else
            skip = int(nTrianglesInGroup/10.0) + 1;// don't mark on all triangles ... ugly

        for (tri = 0; tri < vox.getNumTriangles(); tri+=skip) {
            if (vox.getTriGroup(tri) != group) continue;
            vox.getTriPts(tri,x,y,z);
            center[0] = (x[0]+x[1]+x[2])/3.0;
            center[1] = (y[0]+y[1]+y[2])/3.0;
            center[2] = (z[0]+z[1]+z[2])/3.0;
            vox.getNormalOfTriangle(tri,triNormal);
            if ( bcTypeOfTri == THERMAL_DIRICHLET)  {
                color[0] = 0;color[1] = 0;color[2] = 1;
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                endPt[0] = center[0] + scale*triNormal[0];
                endPt[1] = center[1] + scale*triNormal[1];
                endPt[2] = center[2] + scale*triNormal[2];

                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                drawNiceSphere(center,scale*0.3);
                if (!textPlotted){
                    char tempStr[256];
                    double tempLocation[3];
                    sprintf(tempStr,"%.0f",bcValueOfTri);
                    strcat(tempStr,tempUnitsName.c_str());
                    if (triNormal[0] < -0.9)// negative x direction
                        for (int i = 0; i < 3; i++)
                            tempLocation[i] = endPt[i] + 3*scale*triNormal[i];
                    else
                        for (int i = 0; i < 3; i++)
                            tempLocation[i] = endPt[i];

                    draw3DText(tempStr,tempLocation);
                    textPlotted = true;// plot only once
                }
            }

            if ((bcTypeOfTri == THERMAL_NEUMANN) && (fabs(bcValueOfTri) > 0)) {
                color[0] = 1;
                color[1] = 0;
                color[2] = 0;
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

                if (bcValueOfTri > 0) {
                    endPt[0] = center[0] + 1.25*scale*triNormal[0];
                    endPt[1] = center[1] + 1.25*scale*triNormal[1];
                    endPt[2] = center[2] + 1.25*scale*triNormal[2];

                    center[0] += 0.25*scale*triNormal[0];
                    center[1] += 0.25*scale*triNormal[1];
                    center[2] += 0.25*scale*triNormal[2];

                    drawNiceArrow(endPt,center,false,false,0.1);
                }
                else {
                    endPt[0] = center[0] + scale*triNormal[0];
                    endPt[1] = center[1] + scale*triNormal[1];
                    endPt[2] = center[2] + scale*triNormal[2];
                    drawNiceArrow(center,endPt,false,false,0.1);
                }
                if (!textPlotted){
                    char tempStr[256];
                    double tempLocation[3];
                    sprintf(tempStr,"%.0f",bcValueOfTri);
                    strcat(tempStr,heatFluxName.c_str());
                    for (int i = 0; i < 3; i++)
                        tempLocation[i] = endPt[i] + 0.5*(endPt[i] - center[i]);

                    draw3DText(tempStr,tempLocation);
                    textPlotted = true;// plot only once
                }
            }
            if ((bcTypeOfTri == THERMAL_CONVECTION) && (fabs(bcValueOfTri) > 0)) {
                color[0] = 0;
                color[1] = 1;
                color[2] = 0;
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

                endPt[0] = center[0] + scale*triNormal[0];
                endPt[1] = center[1] + scale*triNormal[1];
                endPt[2] = center[2] + scale*triNormal[2];
                drawNiceArrow(center,endPt,false,true);
                if (!textPlotted){
                    char tempStr[256];
                    double tempLocation[3];
                    if (liveVar.temperatureUnits == UNITS_TEMPERATURE_CELCIUS)
                        sprintf(tempStr,"%.0f(W/m^2-C); %.0f  ",bcValueOfTri,liveVar.ThermalBCRef[bcSet][group]);
                    else if (liveVar.temperatureUnits == UNITS_TEMPERATURE_KELVIN)
                        sprintf(tempStr,"%.0f(W/m^2-K); %.0f  ",bcValueOfTri,liveVar.ThermalBCRef[bcSet][group]);
                    else
                        sprintf(tempStr,"%.0f(W/m^2-F); %.0f  ",bcValueOfTri,liveVar.ThermalBCRef[bcSet][group]);
                    strcat(tempStr,tempUnitsName.c_str());
                    for (int i = 0; i < 3; i++)
                        tempLocation[i] = endPt[i] + 0.5*(endPt[i] - center[i]);

                    draw3DText(tempStr,tempLocation);
                    textPlotted = true;// plot only once
                }
            }
            if ((bcTypeOfTri == THERMAL_RADIATION) && (fabs(bcValueOfTri) > 0)) {
                color[0] = 0;
                color[1] = 0.5;
                color[2] = 1;
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

                endPt[0] = center[0] + 1.5*scale*triNormal[0];
                endPt[1] = center[1] + 1.5*scale*triNormal[1];
                endPt[2] = center[2] + 1.5*scale*triNormal[2];
                drawNiceArrow(center,endPt,true,true);
                if (!textPlotted){
                    char tempStr[256];
                    double tempLocation[3];
                    sprintf(tempStr,"%.2f; %.0f ",bcValueOfTri,liveVar.ThermalBCRef[bcSet][group]);

                    strcat(tempStr,tempUnitsName.c_str());
                    for (int i = 0; i < 3; i++)
                        tempLocation[i] = endPt[i] + 0.5*(endPt[i] - center[i]);

                    draw3DText(tempStr,tempLocation);
                    textPlotted = true;// plot only once
                }
            }
        }
    }
    glEnable(GL_LIGHTING);
    glPointSize(1.0);
    glLineWidth(1.0);
    glDisable(GL_BLEND);

}
/**********************************************************************/
void Renderer::plotFieldOverTriangleSmooth(double x[3], double y[3], double z[3],double u[3],double v[3],double w[3],double vonmises[3],
double temperature[3],double dist[3],
double scale,double triNormal[3],double vertexNormals0[3],double vertexNormals1[3],double vertexNormals2[3],double fieldMin,double fieldMax)
{
    int i;
    double *fieldToPlot,temp;
    GLfloat color[4] ={1.0, 0., 0., 0.75};
    double delta[3],xTri[3],yTri[3],zTri[3];

    // Get the deformations u,v,w or stresses, strains at those nodes
    switch (liveVar.displayField) {
    case DISPLAY_FIELD_DISPLACEMENT:
        fieldToPlot = &(delta[0]);
        break;
    case DISPLAY_FIELD_DISPLACEMENT_X:
        fieldToPlot = &(u[0]);
        break;
    case DISPLAY_FIELD_DISPLACEMENT_Y:
        fieldToPlot = &(v[0]);
        break;
    case DISPLAY_FIELD_DISPLACEMENT_Z:
        fieldToPlot = &(w[0]);
        break;
    case DISPLAY_FIELD_STRESS:
        fieldToPlot = &(vonmises[0]);
        break;
    case DISPLAY_FIELD_TEMPERATURE:
        fieldToPlot = &(temperature[0]);
        scale = 0;
        break;
    default:
        fieldToPlot = &(delta[0]);
        break;
    }
    for (i = 0; i < 3; i++) {
        xTri[i] = x[i] + scale*u[i];
        yTri[i] = y[i] + scale*v[i];
        zTri[i] = z[i] +  scale*w[i];
        delta[i] = sqrt(u[i]*u[i] + v[i]*v[i]+ w[i]*w[i]);
    }
    glBegin(GL_TRIANGLES);
    temp = (fieldToPlot[0]-fieldMin)/(fieldMax-fieldMin);
    temp = MIN(MAX(temp,0),1);

    color[0] = -temp + 2*temp*temp;color[1] = temp*(1-temp)*4; color[2] = 1 - 3*temp +2*temp*temp;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

    glNormal3f(vertexNormals0[0],vertexNormals0[1],vertexNormals0[2]);
    glVertex3f(xTri[0],yTri[0],zTri[0]);

    temp = (fieldToPlot[1]-fieldMin)/(fieldMax-fieldMin);
    temp = MIN(MAX(temp,0),1);

    color[0] = -temp + 2*temp*temp;color[1] = temp*(1-temp)*4; color[2] = 1 - 3*temp +2*temp*temp;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
    glNormal3f(vertexNormals1[0],vertexNormals1[1],vertexNormals1[2]);
    glVertex3f(xTri[1],yTri[1],zTri[1]);

    temp = (fieldToPlot[2]-fieldMin)/(fieldMax-fieldMin);
    temp = MIN(MAX(temp,0),1);

    color[0] = -temp + 2*temp*temp;color[1] = temp*(1-temp)*4; color[2] = 1 - 3*temp +2*temp*temp;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
    glNormal3f(vertexNormals2[0],vertexNormals2[1],vertexNormals2[2]);
    glVertex3f(xTri[2],yTri[2],zTri[2]);

    glEnd();

}
/**********************************************************************/
void Renderer::plotMeshSurface(void)
{
    int elem,nodes[8],neighboringElemsOfElem[27],neighboringElem,isoCounter,partId;
    double x[8],y[8],z[8],temp,endPt[3],tsCutoff,u[8],v[8],w[8],uElem,vElem,wElem,deltaElem,elemStress;
    double p0[3],p1[3],p2[3],triNormal[3],p01[3],p02[3],scale,maxDeformation,currentVolFraction,timeInSec;
    double minStress,maxStress,temperatureMin,temperatureMax,elemTemperature;
    double boundingBox[3][2],XCuttingPlane,YCuttingPlane,ZCuttingPlane;
    double elemCenter[3],neighborCenter[3];

    vox.getBoundingBox(boundingBox);

    int quads[6][4] = {{0,4,7,3},{0,1,5,4},{0,3,2,1},{1,2,6,5},{2,3,7,6},{4,5,6,7}};
    GLfloat color[4] ={0.1,0.95, 0.1, 0.0};
    double modelScale;
    bool neighboringElemsExists[7];

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

    modelScale = vox.getModelScale();
    maxDeformation = topopt.getDeformationMax();
    maxStress = topopt.getMaxVonMisesStress();
    minStress = topopt.getMinVonMisesStress();
    topopt.getTemperatureRange(&temperatureMin,&temperatureMax);
    if (liveVar.isDeformationScaled)
        scale = 0.05*modelScale/maxDeformation;
    else
        scale = 1;

    if (liveVar.plotAxis) drawAxis();
    liveVar.deformationScaled = scale;

    isoCounter = topopt.getCurrentIsoCounter();
    tsCutoff = topopt.getCurrentTSCutoff();
    glLineWidth(1.5);
    glPointSize(5.0);
    isoCounter = topopt.getClosestCounterAtVolFraction(liveVar.displayVolFraction,&currentVolFraction,&tsCutoff,&timeInSec);

    XCuttingPlane = boundingBox[0][0] + liveVar.xCuttingPlanePercent*(boundingBox[0][1] - boundingBox[0][0])/100.0;
    YCuttingPlane = boundingBox[1][0] + liveVar.yCuttingPlanePercent*(boundingBox[1][1] - boundingBox[1][0])/100.0;
    ZCuttingPlane = boundingBox[2][0] + liveVar.zCuttingPlanePercent*(boundingBox[2][1] - boundingBox[2][0])/100.0;


    for (elem = 0; elem < topopt.getNumElems(); elem++) {
        if  (topopt.getElemPseudoDensity(elem) < 0.5) continue;
        partId = topopt.getPartIdOfElem(elem);
        topopt.getAllNeighboringElementsOfElem(elem,neighboringElemsOfElem);
        // 0 is self, 1 is x-, 2 is y-,3 is z-, 4 is x+,5 is y+, 6 is z+
        neighboringElemsExists[0] = true;//self
        topopt.getElemCenter(elem,elemCenter);
        if (elemCenter[0] < XCuttingPlane)
            continue;
        if (elemCenter[1] < YCuttingPlane)
            continue;
        if (elemCenter[2] < ZCuttingPlane)
            continue;
        for (int m = 1; m < 7; m++) {// start with 1 for neighbors
            neighboringElemsExists[m] = false;
            neighboringElem = neighboringElemsOfElem[m];
            if (neighboringElem == -1) continue;
            if (topopt.getElemPseudoDensity(neighboringElem) < 0.5) continue;
            if (!topopt.doesElemExistsAtTSCounter(neighboringElem,isoCounter)) continue;
            topopt.getElemCenter(neighboringElem,neighborCenter);
            if (neighborCenter[0] < XCuttingPlane)
                continue;
            if (neighborCenter[1] < YCuttingPlane)
                continue;
            if (neighborCenter[2] < ZCuttingPlane)
                continue;
            neighboringElemsExists[m] = true;
        }
        topopt.getElemNodes(elem,nodes);
        color[0] = 0.0;
        color[1] = 0.7;
        color[2] = 0.1;// default


        getColorOfPart(partId,color);
        switch (liveVar.displayField){
        case DISPLAY_FIELD_NONE:
            topopt.getElemNodeLocation(elem,x,y,z);
            /*if (!topopt.doesElementHaveManifoldBoundary(elem)){// non-manifold element
                color[0] = 1.0;
                color[1] = 0.0;
                color[2] = 0.0;
            }*/
            break;
        case DISPLAY_FIELD_DISPLACEMENT:case DISPLAY_FIELD_MODAL:
            topopt.getElemNodeLocationWithDeformation(elem,x,y,z,scale);
            topopt.getElemNodeDeformation(elem,u,v,w);
            uElem = vElem = wElem = 0;
            for (int i = 0; i <8; i++){
                uElem += u[i];
                vElem += v[i];
                wElem += w[i];
            }
            uElem /=8;vElem /=8;wElem /=8;
            deltaElem = sqrt(uElem*uElem + vElem*vElem + wElem*wElem);
            temp = deltaElem/maxDeformation;
            getInterpolatedColor(temp,color);
            break;
        case DISPLAY_FIELD_STRESS:
            topopt.getElemNodeLocationWithDeformation(elem,x,y,z,scale);
            elemStress = topopt.getElemVonMisesStress(elem);
            temp = (elemStress-minStress)/(maxStress-minStress);
            getInterpolatedColor(temp,color);
            break;
        case DISPLAY_FIELD_TEMPERATURE:
            topopt.getElemNodeLocation(elem,x,y,z);
            elemTemperature = topopt.getTemperatureInElem(elem);
            temp = (elemTemperature-temperatureMin)/(temperatureMax-temperatureMin);
            getInterpolatedColor(temp,color);
            break;
        default: topopt.getElemNodeLocation(elem,x,y,z);break;
        }

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

        for (int m = 0; m < 6;m++) {// consider the six quads
            // make sure all 4 nodes are on boundary
            if (neighboringElemsExists[m+1]) continue;//offset by 1, don't plot boundary if neighbor exists

            // use 3 pts to find the normal
            p0[0] = x[quads[m][0]];
            p0[1] = y[quads[m][0]];
            p0[2] = z[quads[m][0]];

            p1[0] = x[quads[m][1]];
            p1[1] = y[quads[m][1]];
            p1[2] = z[quads[m][1]];

            p2[0] = x[quads[m][2]];
            p2[1] = y[quads[m][2]];
            p2[2] = z[quads[m][2]];


            for (int i = 0; i < 3; i++) {
                p01[i] = p1[i] - p0[i];
                p02[i] = p2[i] - p0[i];
            }
            CROSS3D(triNormal,p01,p02);

            glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
            glBegin(GL_QUADS);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();

            glColor3f(0.0f,0.0f,0.0f);
            glDisable( GL_LIGHTING );
            // 4 lines per quad
            glBegin(GL_LINE_LOOP);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
            glEnable( GL_LIGHTING );
        }
    }

    if (liveVar.geomTransparent) plotGeometry();



    scale = 0.02*modelScale;
    glDisable( GL_LIGHTING );
    if (liveVar.plotElasticityBC) {
        int node;
        double force[3],pt[3],magnitude;

        // Now plot all the boundary conditions on nodes
        for (node = 0;node < topopt.getNumNodes();node++) {
            if (!topopt.isNodeFree(node)) {//fixed
                glColor3f(0.0f,0.0f,0.0f);
                topopt.getNodeXYZ(node,pt);
                glColor3f(0.0f,0.0f,0.0f);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                drawNiceSphere(pt,0.1*scale);
                continue;
            }

            topopt.getNeumannForce(node,force);
            magnitude = NORM3D(force);
            if (magnitude >0) {// force exists on Neumann node

                topopt.getNodeXYZ(node,pt);

                endPt[0] = pt[0] + scale*force[0]/magnitude;
                endPt[1] = pt[1]+ scale*force[1]/magnitude;
                endPt[2] = pt[2]+ scale*force[2]/magnitude;
                glColor3f(0.9f,0.0f,0.0f);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                drawNiceArrow(pt,endPt,true,false,0.05);
            }
        }
    }

    if (liveVar.plotThermalBC) {
        int node;
        double thermalLoad,pt[3];

        // Now plot all the boundary conditions on nodes
        for (node = 0;node < topopt.getNumNodes();node++) {
            if (!topopt.isNodeFree_thermal(node)) {//fixed temp
                glColor3f(0.0f,0.0f,0.0f);
                topopt.getNodeXYZ(node,pt);
                glBegin(GL_POINTS);
                glVertex3f(pt[0],pt[1],pt[2]);
                glEnd();
                continue;
            }

            thermalLoad = topopt.getThermalForce(node);
            if (fabs(thermalLoad) >0) {// force exists on Neumann node
                glColor3f(0.9f,0.0f,0.0f);
                topopt.getNodeXYZ(node,pt);
                glBegin(GL_POINTS);
                glVertex3f(pt[0],pt[1],pt[2]);
                glEnd();
            }
        }
    }
    if  (liveVar.displayField == DISPLAY_FIELD_NONE)
        plotMeshData();
    else {
        double fieldMin,fieldMax;
        int maxFieldNode;
        char title[256];

        getStaticFEATitle(&fieldMin,&fieldMax,title,&maxFieldNode);
        plotStructuralAnalysisResults();
        plotColorBars();
    }
}
void Renderer::plotDisplacementOverMeshSurface(void)
{
    int elem,nodes[8],neighboringElemsOfElem[27],neighboringElem;
    double x[8],y[8],z[8],fieldMin,fieldMax,*fieldToPlot;
    double u[8],v[8],w[8],deformation[8];
    double p0[3],p1[3],p2[3],triNormal[3],p01[3],p02[3];
    int quads[6][4] = {{0,4,7,3},{0,1,5,4},{0,3,2,1},{1,2,6,5},{2,3,7,6},{4,5,6,7}},maxFieldNode;
    GLfloat color[4] ={0.1,0.95, 0.1, 1.0};
    bool neighboringElemsExists[7];
    char title[256];
    double temp;
    int deltaY,startX,startY;
    double boundingBox[3][2],XCuttingPlane,YCuttingPlane,ZCuttingPlane;
    double elemCenter[3],neighborCenter[3];

    vox.getBoundingBox(boundingBox);
    XCuttingPlane = boundingBox[0][0] + liveVar.xCuttingPlanePercent*(boundingBox[0][1] - boundingBox[0][0])/100.0;
    YCuttingPlane = boundingBox[1][0] + liveVar.yCuttingPlanePercent*(boundingBox[1][1] - boundingBox[1][0])/100.0;
    ZCuttingPlane = boundingBox[2][0] + liveVar.zCuttingPlanePercent*(boundingBox[2][1] - boundingBox[2][0])/100.0;
    double modelScale = vox.getModelScale();
    liveVar.deformationScaled  = 0.05*modelScale/topopt.getDeformationMax();


    deltaY = 30;// font size dependent
    startX = 20;
    startY = deltaY;
    print2DText(startX, startY,liveVar.graphicsText.c_str());

    getStaticFEATitle(&fieldMin,&fieldMax,title,&maxFieldNode);

    sprintf(title,"Displacement");
    if (liveVar.plotAxis) drawAxis();
    glLineWidth(1.5);
    glPointSize(5.0);
    glDisable( GL_LIGHTING );
    for (elem = 0; elem < topopt.getNumElems(); elem++) {
        if  (topopt.getElemPseudoDensity(elem) <1e-6) continue;
        topopt.getElemCenter(elem,elemCenter);
        if (elemCenter[0] < XCuttingPlane)
            continue;
        if (elemCenter[1] < YCuttingPlane)
            continue;
        if (elemCenter[2] < ZCuttingPlane)
            continue;
        topopt.getAllNeighboringElementsOfElem(elem,neighboringElemsOfElem);
        // 0 is self, 1 is x-, 2 is y-,3 is z-, 4 is x+,5 is y+, 6 is z+
        neighboringElemsExists[0] = true;//self
        for (int m = 1; m < 7; m++) {// start with 1 for neighbors
            neighboringElemsExists[m] = false;
            neighboringElem = neighboringElemsOfElem[m];
            if (neighboringElem == -1) continue;
            if (topopt.getElemPseudoDensity(neighboringElem) <1e-6) continue;
            topopt.getElemCenter(neighboringElem,neighborCenter);
            if (neighborCenter[0] < XCuttingPlane)
                continue;
            if (neighborCenter[1] < YCuttingPlane)
                continue;
            if (neighborCenter[2] < ZCuttingPlane)
                continue;
            neighboringElemsExists[m] = true;
        }
        topopt.getElemNodes(elem,nodes);
        topopt.getElemNodeLocation(elem,x,y,z);
        topopt.getElemNodeDeformation(elem,u,v,w);

        for (int i =0; i < 8; i++)
            deformation[i] = sqrt(u[i]*u[i] + v[i]*v[i] + w[i]*w[i]);
        for (int i =0; i < 8; i++){
            x[i] += liveVar.deformationScaled*u[i];
            y[i] += liveVar.deformationScaled*v[i];
            z[i] += liveVar.deformationScaled*w[i];
        }
        fieldToPlot = &(deformation[0]);
        for (int m = 0; m < 6;m++) {// consider the six quads
            // make sure all 4 nodes are on boundary
            if (neighboringElemsExists[m+1]) continue;//offset by 1, don't plot boundary if neighbor exists

            // use 3 pts to find the normal
            p0[0] = x[quads[m][0]];
            p0[1] = y[quads[m][0]];
            p0[2] = z[quads[m][0]];

            p1[0] = x[quads[m][1]];
            p1[1] = y[quads[m][1]];
            p1[2] = z[quads[m][1]];

            p2[0] = x[quads[m][2]];
            p2[1] = y[quads[m][2]];
            p2[2] = z[quads[m][2]];

            for (int i = 0; i < 3; i++) {
                p01[i] = p1[i] - p0[i];
                p02[i] = p2[i] - p0[i];
            }
            CROSS3D(triNormal,p01,p02);

            glBegin(GL_QUADS);
            glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
            temp = (fieldToPlot[quads[m][0]]-fieldMin)/(fieldMax-fieldMin);

            getInterpolatedColor(temp, color);

            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);

            temp = (fieldToPlot[quads[m][1]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);

            temp = (fieldToPlot[quads[m][2]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);

            temp = (fieldToPlot[quads[m][3]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
            glEnable( GL_LIGHTING );
        }
    }
    plotColorBars();
    if (liveVar.geomTransparent) plotGeometry();
    glDisable( GL_LIGHTING );

}
/**********************************************************************/
void Renderer::plotTemperatureOverMeshSurface(void)
{
    int elem,nodes[8],neighboringElemsOfElem[27],neighboringElem;
    double x[8],y[8],z[8],fieldMin,fieldMax,*fieldToPlot;
    double p0[3],p1[3],p2[3],triNormal[3],p01[3],p02[3];
    int quads[6][4] = {{0,4,7,3},{0,1,5,4},{0,3,2,1},{1,2,6,5},{2,3,7,6},{4,5,6,7}},maxFieldNode;
    GLfloat color[4] ={0.1,0.95, 0.1, 1.0};
    bool neighboringElemsExists[7];
    char title[256];
    double temp,temperature[8];

    getStaticFEATitle(&fieldMin,&fieldMax,title,&maxFieldNode);
    if (liveVar.plotAxis) drawAxis();
    glLineWidth(1.5);
    glPointSize(5.0);
    glDisable( GL_LIGHTING );
    for (elem = 0; elem < topopt.getNumElems(); elem++) {
        if  (topopt.getElemPseudoDensity(elem) <1e-6) continue;
        topopt.getAllNeighboringElementsOfElem(elem,neighboringElemsOfElem);
        // 0 is self, 1 is x-, 2 is y-,3 is z-, 4 is x+,5 is y+, 6 is z+
        neighboringElemsExists[0] = true;//self
        for (int m = 1; m < 7; m++) {// start with 1 for neighbors
            neighboringElemsExists[m] = false;
            neighboringElem = neighboringElemsOfElem[m];
            if (neighboringElem == -1) continue;
            if (topopt.getElemPseudoDensity(neighboringElem) <1e-6) continue;
            neighboringElemsExists[m] = true;
        }
        topopt.getElemNodes(elem,nodes);
        topopt.getElemNodeLocation(elem,x,y,z);

        topopt.getElemNodesTemperature(elem,temperature);
        fieldToPlot = &(temperature[0]);
        for (int m = 0; m < 6;m++) {// consider the six quads
            // make sure all 4 nodes are on boundary
            if (neighboringElemsExists[m+1]) continue;//offset by 1, don't plot boundary if neighbor exists

            // use 3 pts to find the normal
            p0[0] = x[quads[m][0]];
            p0[1] = y[quads[m][0]];
            p0[2] = z[quads[m][0]];

            p1[0] = x[quads[m][1]];
            p1[1] = y[quads[m][1]];
            p1[2] = z[quads[m][1]];

            p2[0] = x[quads[m][2]];
            p2[1] = y[quads[m][2]];
            p2[2] = z[quads[m][2]];

            for (int i = 0; i < 3; i++) {
                p01[i] = p1[i] - p0[i];
                p02[i] = p2[i] - p0[i];
            }
            CROSS3D(triNormal,p01,p02);

            glBegin(GL_QUADS);
            glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
            temp = (fieldToPlot[quads[m][0]]-fieldMin)/(fieldMax-fieldMin);

            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);

            temp = (fieldToPlot[quads[m][1]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);


            temp = (fieldToPlot[quads[m][2]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);


            temp = (fieldToPlot[quads[m][3]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
            glEnable( GL_LIGHTING );
        }
    }
    plotColorBars();
    if (liveVar.geomTransparent) plotGeometry();
    glDisable( GL_LIGHTING );

}
/**********************************************************************/
void Renderer::plotNothing(void)
{
    ;
}
/**********************************************************************/
void Renderer::plotGeometry(bool plotGeom )
{
	displayStatusMessage();
    if (vox.getNumTriangles() == 0) return;


    if (liveVar.plotAxis) drawAxis();
    if (liveVar.plotElasticityBC) plotElasticityBC();
    if (liveVar.plotThermalBC) plotThermalBC();
    if (liveVar.plotBoundingBox) plotBoundingBox();
    if (liveVar.plotSupportStructures)
        plotSupportStructuresInitialGeometry(liveVar.topoptConstraints.AMBuildDirection);
    if (liveVar.plotTopOptConstraints) {
        plotSymmetryPlanes(liveVar.topoptConstraints.xSymmetry,
                           liveVar.topoptConstraints.ySymmetry,
                           liveVar.topoptConstraints.zSymmetry);

        if (liveVar.topoptConstraints.isCyclicSymmetryActive)
            plotCyclicSymmetryPlanes(liveVar.topoptConstraints.cyclicSymmetry);
        if (liveVar.topoptConstraints.isDrawDirectionActive)
            plotDrawDirection(liveVar.topoptConstraints.drawDirection);
        if (liveVar.topoptConstraints.isExtrudeActive)
            plotThroughCutDirection(liveVar.topoptConstraints.extrudeDirection);
        if ((liveVar.topoptConstraints.isxGridActive) ||
                (liveVar.topoptConstraints.isyGridActive) ||
                (liveVar.topoptConstraints.iszGridActive))
            plotGridPattern(liveVar.topoptConstraints.xGridPattern,
                            liveVar.topoptConstraints.yGridPattern,
                            liveVar.topoptConstraints.zGridPattern);
    }


    if (!plotGeom) {
        //plotGeomData();
        return;// when plotting Native CAD, we may not want to show the STL geom
    }
    if (liveVar.geomTransparent) {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        plotSTLGeom(liveVar.displayEdges);
        glDisable( GL_BLEND );
        if (liveVar.displayGeom == DISPLAY_GEOM_FINAL)
            plotTempGeom(liveVar.displayEdges);
    }
    else{
        if (liveVar.displayGeom == DISPLAY_GEOM_FINAL)
            plotTempGeom(liveVar.displayEdges);
        else
            plotSTLGeom(liveVar.displayEdges);

        plotGeomData();
    }
    //glDisable(GL_LIGHTING);
}
/**********************************************************************/
void Renderer::plotMesh(void)
{
    int NumElems = topopt.getNumElems();
    int elem,m,nodes[8],templateId,nTemplates;
    double x[8],y[8],z[8],temp,endPt[3];
    double p0[3],p1[3],p2[3],triNormal[3],p01[3],p02[3],scale,maxDeformation;
    int edges[12][2] = {{0,1},{1,2},{2,3},{3,0},{0,4},{1,5},{2,6},{3,7},{4,5},{5,6},{6,7},{7,4}};
    int quads[6][4] = {{0,3,2,1},{0,1,5,4},{0,4,7,3},{4,5,6,7},{1,2,6,5},{2,3,7,6}};
    GLfloat color[4] ={0.1,0.95, 0.1, 0.0};
    bool withDeformation,showTemplateColors;
    double modelScale;


    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glPointSize(6.0);
    withDeformation = false;
    if (withDeformation){
        modelScale = vox.getModelScale();
        maxDeformation = topopt.getDeformationMax();
        scale = 0.05*modelScale/maxDeformation;
    }
    else
        scale = 1.0;
    nTemplates = topopt.getNumTemplateElements();
    if (liveVar.plotAxis) drawAxis();

    bool inverted = false;//set to true if you want to plot elements that are outside
    showTemplateColors = false;// set to true to plot elements based on colors
    for (elem = 0; elem < NumElems; elem++) {
        if ((inverted) && (topopt.getElemPseudoDensity(elem) > 0.5)) continue;
        if ((!inverted) && (topopt.getElemPseudoDensity(elem) < 0.5)) continue;

        if (withDeformation)
            topopt.getElemNodeLocationWithDeformation(elem,x,y,z,scale);
        else
            topopt.getElemNodeLocation(elem,x,y,z);

        topopt.getElemNodes(elem,nodes);
        templateId = topopt.getTemplateId(elem);
        temp = 0.5;
        getInterpolatedColor(temp, color);
        int part = topopt.getPartIdOfElem(elem);


        getColorOfPart(part,color);

        if (showTemplateColors) {
            if (templateId == 0){
                temp = 0.5;
               getInterpolatedColor(temp, color);
            }
            else if(templateId == 1){
                color[0] = 1;color[1] = 0; color[2] = 0;
            }
            else if(templateId == 2){
                color[0] = 0;color[1] = 0; color[2] = 1;
            }
            else if(templateId == 3){
                color[0] = 0;color[1] = 1; color[2] = 1;
            }
            else if(templateId == 4){
                color[0] = 1;color[1] = 0; color[2] = 1;
            }
            else if ((nTemplates > 4) && (nTemplates < 1000)){
                templateId = topopt.getTemplateId(elem);

                templateId = (templateId)%nTemplates;
                temp = 1.0*templateId/(nTemplates);
                temp += 0.5;
                temp = (temp > 1)? (1.5-temp):temp;

                getInterpolatedColor(temp, color);
            }
        }

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        for (m = 0; m < 6;m++) {
            p0[0] = x[quads[m][0]];
            p0[1] = y[quads[m][0]];
            p0[2] = z[quads[m][0]];

            p1[0] = x[quads[m][1]];
            p1[1] = y[quads[m][1]];
            p1[2] = z[quads[m][1]];

            p2[0] = x[quads[m][2]];
            p2[1] = y[quads[m][2]];
            p2[2] = z[quads[m][2]];


            for (int i = 0; i < 3; i++) {
                p01[i] = p1[i] - p0[i];
                p02[i] = p2[i] - p0[i];
            }
            CROSS3D(triNormal,p01,p02);

            glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
            glBegin(GL_QUADS);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
        }
    }

    glColor3f(0.1,0.1,0.1);

    for (elem = 0; elem < NumElems; elem++) {
        if ((inverted) && (topopt.getElemPseudoDensity(elem) > 0.5)) continue;
        if ((!inverted) && (topopt.getElemPseudoDensity(elem) < 0.5)) continue;
        if (topopt.getElemPseudoDensity(elem) < 0.5)
            glLineWidth(0.2);
        else
            glLineWidth(0.5);
        if (withDeformation)
            topopt.getElemNodeLocationWithDeformation(elem,x,y,z,scale);
        else
            topopt.getElemNodeLocation(elem,x,y,z);
        topopt.getElemNodes(elem,nodes);
        for (m = 0; m < 6;m++) {// 6 faces
            // 4 lines per quad
            glBegin(GL_LINE_LOOP);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
        }
    }

    glColor3f(0.9,0.0,0.0);
    if (withDeformation) return;
    if (liveVar.plotElasticityBC) {
        double force[3],pt[3],magnitude;
        glPointSize(50.0);
        // Now plot all the boundary conditions on nodes
        for (int node = 0;node < topopt.getNumNodes();node++) {
            if (!topopt.doesNodeExist(node)) continue;
            if (!topopt.isNodeFree(node)) {//fixed
                glColor3f(0.0f,0.0f,0.0f);
                topopt.getNodeXYZ(node,pt);
                glBegin(GL_POINTS);
                glVertex3f(pt[0],pt[1],pt[2]);
                glEnd();
                continue;
            }
            if (topopt.isNodeRetained(node)) {//node retained
                glColor3f(0.0f,1.0f,1.0f);
                topopt.getNodeXYZ(node,pt);
                glBegin(GL_POINTS);
                glVertex3f(pt[0],pt[1],pt[2]);
                glEnd();
                continue;
            }
            if (topopt.isNodeOutput(node)) {//node output for compliance
                glColor3f(0.5f,0.5f,0.2f);
                topopt.getNodeXYZ(node,pt);
                glBegin(GL_POINTS);
                glVertex3f(pt[0],pt[1],pt[2]);
                glEnd();
                continue;
            }
            topopt.getNeumannForce(node,force);
            magnitude = NORM3D(force);

            if (magnitude >0) {// force exists on Neumann node
                double scale = 0.015*modelScale;
                topopt.getNodeXYZ(node,pt);

                endPt[0] = pt[0] + scale*force[0]/magnitude;
                endPt[1] = pt[1]+ scale*force[1]/magnitude;
                endPt[2] = pt[2]+ scale*force[2]/magnitude;
                glColor3f(0.9f,0.0f,0.0f);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                drawNiceArrow(pt,endPt,true,false,0.15);
            }
        }
    }
    if (liveVar.plotThermalBC) {
        int node;
        double pt[3];
        glPointSize(10.0);
        // Now plot all the boundary conditions on nodes
        for (node = 0;node < topopt.getNumNodes();node++) {
            if (topopt.isTemperatureFixedOnNode(node)) {//fixed
                glColor3f(0.0f,0.0f,0.0f);
                topopt.getNodeXYZ(node,pt);
                glBegin(GL_POINTS);
                glVertex3f(pt[0],pt[1],pt[2]);
                glEnd();
            }
            else if (topopt.isThereHeatFluxOnNode(node)) {
                glColor3f(1.0f,0.0f,0.0f);
                topopt.getNodeXYZ(node,pt);
                glBegin(GL_POINTS);
                glVertex3f(pt[0],pt[1],pt[2]);
                glEnd();
            }
        }
    }
    glEnable( GL_LIGHTING );

    if (liveVar.geomTransparent) plotGeometry();
    if (liveVar.plotNormals) drawNormals();
}
/**********************************************************************/
void Renderer::plotMeshComponents(void)
{
    int NumElems = topopt.getNumElems();
    int elem,m,nodes[8],meshComponent;
    double x[8],y[8],z[8],temp;
    double p0[3],p1[3],p2[3],triNormal[3],p01[3],p02[3],scale,maxDeformation;
    int edges[12][2] = {{0,1},{1,2},{2,3},{3,0},{0,4},{1,5},{2,6},{3,7},{4,5},{5,6},{6,7},{7,4}};
    int quads[6][4] = {{0,3,2,1},{0,1,5,4},{0,4,7,3},{4,5,6,7},{1,2,6,5},{2,3,7,6}};
    GLfloat color[4] ={0.1,0.95, 0.1, 0.0};
    double modelScale;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glPointSize(6.0);
    modelScale = vox.getModelScale();
    maxDeformation = topopt.getDeformationMax();
    scale = 0.05*modelScale/maxDeformation;


    if (topopt.getNumberOfMeshComponents() == 0) return;
    if (liveVar.plotAxis) drawAxis();
    for (elem = 0; elem < NumElems; elem++) {
        if (topopt.getElemPseudoDensity(elem) > 0.5) continue; // only plot outside elements
        topopt.getElemNodeLocation(elem,x,y,z);
        topopt.getElemNodes(elem,nodes);
        meshComponent = topopt.getMeshComponentOfElem(elem);
        if (meshComponent == -1) continue;
        //if (meshComponent != 4) continue;
        getColorOfPart(meshComponent,color);

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        for (m = 0; m < 6;m++) {
            p0[0] = x[quads[m][0]];
            p0[1] = y[quads[m][0]];
            p0[2] = z[quads[m][0]];

            p1[0] = x[quads[m][1]];
            p1[1] = y[quads[m][1]];
            p1[2] = z[quads[m][1]];

            p2[0] = x[quads[m][2]];
            p2[1] = y[quads[m][2]];
            p2[2] = z[quads[m][2]];


            for (int i = 0; i < 3; i++) {
                p01[i] = p1[i] - p0[i];
                p02[i] = p2[i] - p0[i];
            }
            CROSS3D(triNormal,p01,p02);

            glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
            glBegin(GL_QUADS);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
        }
    }

    glColor3f(0.1,0.1,0.1);
    glLineWidth(0.2);
    for (elem = 0; elem < 0*NumElems; elem++) {
        if (topopt.getElemPseudoDensity(elem) > 0.5) continue;
        topopt.getElemNodeLocation(elem,x,y,z);
        topopt.getElemNodes(elem,nodes);
        for (m = 0; m < 6;m++) {// 6 faces
            // 4 lines per quad
            glBegin(GL_LINE_LOOP);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
        }
    }
    glEnable( GL_LIGHTING );
    plotMeshComponentData();
    if (liveVar.geomTransparent) plotGeometry();
}
/**********************************************************************/
void Renderer::plotMeshWithPseudoDensity(void)
{
    int NumElems = topopt.getNumElems();
    int elem,m,nodes[8],nTemplates;
    double x[8],y[8],z[8],temp;
    double p0[3],p1[3],p2[3],triNormal[3],p01[3],p02[3],scale,maxDeformation;
    int quads[6][4] = {{0,3,2,1},{0,1,5,4},{0,4,7,3},{4,5,6,7},{1,2,6,5},{2,3,7,6}};
    GLfloat color[4] ={0.1,0.95, 0.1, 0.0};
    bool withDeformation;


    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    double modelScale;

    glPointSize(6.0);
    modelScale = vox.getModelScale();
    maxDeformation = topopt.getDeformationMax();
    scale = 0.05*modelScale/maxDeformation;
    withDeformation = false;
    nTemplates = topopt.getNumTemplateElements();
    if (liveVar.plotAxis) drawAxis();

    for (elem = 0; elem < NumElems; elem++) {
        if (topopt.getElemPseudoDensity(elem) < liveVar.plotElementsWithDensityGreaterThan) continue;
        if (withDeformation)
            topopt.getElemNodeLocationWithDeformation(elem,x,y,z,scale);
        else
            topopt.getElemNodeLocation(elem,x,y,z);

        topopt.getElemNodes(elem,nodes);

        temp = 1- topopt.getElemPseudoDensity(elem);
        color[0] = temp;color[1] = temp; color[2] = temp;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        for (m = 0; m < 6;m++) {
            p0[0] = x[quads[m][0]];
            p0[1] = y[quads[m][0]];
            p0[2] = z[quads[m][0]];

            p1[0] = x[quads[m][1]];
            p1[1] = y[quads[m][1]];
            p1[2] = z[quads[m][1]];

            p2[0] = x[quads[m][2]];
            p2[1] = y[quads[m][2]];
            p2[2] = z[quads[m][2]];


            for (int i = 0; i < 3; i++) {
                p01[i] = p1[i] - p0[i];
                p02[i] = p2[i] - p0[i];
            }
            CROSS3D(triNormal,p01,p02);

            glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
            glBegin(GL_QUADS);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();

        }
    }

    glDisable( GL_LIGHTING );
    glColor3f(0.1,0.1,0.1);
    for (elem = 0; elem < NumElems; elem++) {
        if (topopt.getElemPseudoDensity(elem) < liveVar.plotElementsWithDensityGreaterThan) continue;
        glLineWidth(topopt.getElemPseudoDensity(elem)*2);
        if (withDeformation)
            topopt.getElemNodeLocationWithDeformation(elem,x,y,z,scale);
        else
            topopt.getElemNodeLocation(elem,x,y,z);
        topopt.getElemNodes(elem,nodes);
        for (m = 0; m < 6;m++) {// 6 faces
            // 4 lines per quad
            glBegin(GL_LINE_LOOP);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
        }
    }

    glColor3f(0.9,0.0,0.0);

    if (withDeformation) return;
    //#define PLOTMESHBC

#ifdef PLOTMESHBC
    int node;
    double force[3],pt[3],magnitude;
    glPointSize(10.0);
    // Now plot all the boundary conditions on nodes
    for (node = 0;node < topopt.getNumNodes();node++) {
        if (!topopt.isNodeFree(node)) {//fixed
            glColor3f(0.0f,0.0f,0.0f);
            topopt.getNodeXYZ(node,pt);
            glBegin(GL_POINTS);
            glVertex3f(pt[0],pt[1],pt[2]);
            glEnd();
            continue;
        }
        if (topopt.isNodeRetained(node)) {//node retained
            glColor3f(0.0f,1.0f,1.0f);
            topopt.getNodeXYZ(node,pt);
            glBegin(GL_POINTS);
            glVertex3f(pt[0],pt[1],pt[2]);
            glEnd();
            continue;
        }
        if (topopt.isNodeOutput(node)) {//node output for compliance
            glColor3f(0.5f,0.5f,0.2f);
            topopt.getNodeXYZ(node,pt);
            glBegin(GL_POINTS);
            glVertex3f(pt[0],pt[1],pt[2]);
            glEnd();
            continue;
        }
        topopt.getNeumannForce(node,force);
        magnitude = NORM3D(force);

        if (magnitude >0) {// force exists on Neumann node
            double scale = 0.03*modelScale/magnitude;
            glColor3f(0.9f,0.0f,0.0f);
            topopt.getNodeXYZ(node,pt);
            glBegin(GL_POINTS);
            glVertex3f(pt[0],pt[1],pt[2]);
            glEnd();
        }
    }
    glEnable( GL_LIGHTING );
#endif
    if (liveVar.geomTransparent)
        plotGeometry();
}
/**********************************************************************/
void Renderer::plotMeshWithTopologicalSensitivity(void)
{
    int NumElems = topopt.getNumElems();
    int elem,m,nodes[8],nTemplates,isoCounter,tsElem,pseudoDensity;
    double x[8],y[8],z[8],temp;
    double p0[3],p1[3],p2[3],triNormal[3],p01[3],p02[3],scale,maxDeformation;
    int quads[6][4] = {{0,3,2,1},{0,1,5,4},{0,4,7,3},{4,5,6,7},{1,2,6,5},{2,3,7,6}};
    GLfloat color[4] ={0.1,0.95, 0.1, 0.1};

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glPointSize(6.0);

    if (liveVar.plotAxis) drawAxis();

    glDisable(GL_COLOR_MATERIAL);
    isoCounter = topopt.getCurrentIsoCounter();
    for (elem = 0; elem < NumElems; elem++) {

        pseudoDensity = topopt.getElemPseudoDensity(elem);
        tsElem = topopt.getTopSensAtElemAtCounter(elem,isoCounter);

        if (pseudoDensity > 0.5)
            temp = 0;
        else
            temp = 1- tsElem;
        topopt.getElemNodeLocation(elem,x,y,z);
        topopt.getElemNodes(elem,nodes);

        color[0] = temp;color[1] = temp; color[2] = temp;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        for (m = 0; m < 6;m++) {
            p0[0] = x[quads[m][0]];
            p0[1] = y[quads[m][0]];
            p0[2] = z[quads[m][0]];

            p1[0] = x[quads[m][1]];
            p1[1] = y[quads[m][1]];
            p1[2] = z[quads[m][1]];

            p2[0] = x[quads[m][2]];
            p2[1] = y[quads[m][2]];
            p2[2] = z[quads[m][2]];


            for (int i = 0; i < 3; i++) {
                p01[i] = p1[i] - p0[i];
                p02[i] = p2[i] - p0[i];
            }
            CROSS3D(triNormal,p01,p02);

            glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
            glBegin(GL_QUADS);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();

        }
    }

    glColor3f(0.1,0.1,0.1);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    for (elem = 0; elem < NumElems; elem++) {
        glLineWidth(topopt.getElemPseudoDensity(elem)*2);
        topopt.getElemNodeLocation(elem,x,y,z);
        topopt.getElemNodes(elem,nodes);
        for (m = 0; m < 6;m++) {// 6 faces
            // 4 lines per quad
            glBegin(GL_LINE_LOOP);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
        }
    }

    glColor3f(0.9,0.0,0.0);

    glDisable( GL_BLEND );
    if (liveVar.geomTransparent)
        plotGeometry();
}
/**********************************************************************/
void Renderer::plotTemperatureOverMesh(void)
{
    int NumElems = topopt.getNumElems();
    int elem,m,nodes[8],maxFieldNode;
    double x[8],y[8],z[8],rho,fieldMin,fieldMax,*fieldToPlot;
    double p0[3],p1[3],p2[3],triNormal[3],p01[3],p02[3];
    int quads[6][4] = {{0,3,2,1},{0,1,5,4},{0,4,7,3},{4,5,6,7},{1,2,6,5},{2,3,7,6}};
    GLfloat color[4] ={0.1,0.95, 0.1, 0.0};
    char title[256];
    double temp,temperature[8];

    getStaticFEATitle(&fieldMin,&fieldMax,title,&maxFieldNode);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    if (liveVar.plotAxis) drawAxis();
    glPointSize(6.0);
    for (elem = 0; elem < NumElems; elem++) {
        rho = topopt.getElemPseudoDensity(elem);
        if (rho <1e-6) continue;
        topopt.getElemNodeLocation(elem,x,y,z);
        topopt.getElemNodesTemperature(elem,temperature);
        topopt.getElemNodes(elem,nodes);

        fieldToPlot = &(temperature[0]);

        for (m = 0; m < 6;m++) {
            p0[0] = x[quads[m][0]];
            p0[1] = y[quads[m][0]];
            p0[2] = z[quads[m][0]];

            p1[0] = x[quads[m][1]];
            p1[1] = y[quads[m][1]];
            p1[2] = z[quads[m][1]];

            p2[0] = x[quads[m][2]];
            p2[1] = y[quads[m][2]];
            p2[2] = z[quads[m][2]];

            for (int i = 0; i < 3; i++) {
                p01[i] = p1[i] - p0[i];
                p02[i] = p2[i] - p0[i];
            }
            CROSS3D(triNormal,p01,p02);
            temp = (fieldToPlot[0]-fieldMin)/(fieldMax-fieldMin);
            glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
            glBegin(GL_QUADS);

            temp = (fieldToPlot[quads[m][0]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);


            temp = (fieldToPlot[quads[m][1]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);


            temp = (fieldToPlot[quads[m][2]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);


            temp = (fieldToPlot[quads[m][3]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();

        }
    }
    plotColorBars();
    if (liveVar.geomTransparent) plotGeometry();
}
/**********************************************************************/
void Renderer::plotStressOnMesh(void)
{
    int NumElems = topopt.getNumElems();
    int elem,m,nodes[8],nTemplates,whichNode;
    double x[8],y[8],z[8],rho,temp,vonMises,vonMisesMax;
    double p0[3],p1[3],p2[3],triNormal[3],p01[3],p02[3],scale,maxDeformation;
    int edges[12][2] = {{0,1},{1,2},{2,3},{3,0},{0,4},{1,5},{2,6},{3,7},{4,5},{5,6},{6,7},{7,4}};
    int quads[6][4] = {{0,3,2,1},{0,1,5,4},{0,4,7,3},{4,5,6,7},{1,2,6,5},{2,3,7,6}};
    GLfloat color[4] ={0.1,0.95, 0.1, 0.0};
    bool withDeformation;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    double modelScale;

    glPointSize(6.0);
    modelScale = vox.getModelScale();
    maxDeformation = topopt.getDeformationMax();
    scale = 0.05*modelScale/maxDeformation;
    withDeformation = false;
    nTemplates = topopt.getNumTemplateElements();
    if (liveVar.plotAxis) drawAxis();
    vonMisesMax = topopt.getMaxVonMisesStress(&whichNode);
    for (elem = 0; elem < NumElems; elem++) {
        rho = topopt.getElemPseudoDensity(elem);
        if (rho <0.5) continue;

        topopt.getElemNodeLocation(elem,x,y,z);
        vonMises = topopt.getElemVonMisesStress(elem);
        topopt.getElemNodes(elem,nodes);

        temp = vonMises/vonMisesMax;
        temp = (temp < 0)? 0:temp;
        temp = (temp > 1)? 1:temp;

        //temp = (temp < 0.45)? 0.5:0.99;

        if (nTemplates == 1)
            getInterpolatedColor(temp, color);

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        for (m = 0; m < 6;m++) {
            p0[0] = x[quads[m][0]];
            p0[1] = y[quads[m][0]];
            p0[2] = z[quads[m][0]];

            p1[0] = x[quads[m][1]];
            p1[1] = y[quads[m][1]];
            p1[2] = z[quads[m][1]];

            p2[0] = x[quads[m][2]];
            p2[1] = y[quads[m][2]];
            p2[2] = z[quads[m][2]];

            for (int i = 0; i < 3; i++) {
                p01[i] = p1[i] - p0[i];
                p02[i] = p2[i] - p0[i];
            }
            CROSS3D(triNormal,p01,p02);

            glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
            glBegin(GL_QUADS);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();

        }
    }
    glDisable( GL_LIGHTING );
    glLineWidth(2.5);
    glColor3f(0.1,0.1,0.1);
    for (elem = 0; elem < NumElems; elem++) {
        rho = topopt.getElemPseudoDensity(elem);
        if (rho <0.5 ) continue;
        if (withDeformation)
            topopt.getElemNodeLocationWithDeformation(elem,x,y,z,scale);
        else
            topopt.getElemNodeLocation(elem,x,y,z);
        topopt.getElemNodes(elem,nodes);
        for (m = 0; m < 6;m++) {// 6 faces
            // 4 lines per quad
            glBegin(GL_LINE_LOOP);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
        }
    }
    if (liveVar.geomTransparent) plotGeometry();
}
/**********************************************************************/
void Renderer::plotFieldOverSTL(void)
{
    double triNormal[3],fieldMin,fieldMax;
    double x[3], y[3],z[3],u[3],v[3],w[3],vonmises[3], dist[3],temperature[3];
    char title[256];
    int maxFieldNode;
    Timer animationTimer;

    glDisable(GL_COLOR_MATERIAL);
    displayStatusMessage();
    if (liveVar.plotAxis) drawAxis();
    if (liveVar.plotElasticityBC) plotElasticityBC();
    if (liveVar.plotThermalBC) plotThermalBC();
    if (liveVar.plotBoundingBox) plotBoundingBox();
    if (liveVar.plotTopOptConstraints)
        plotSymmetryPlanes(liveVar.topoptConstraints.xSymmetry,
                           liveVar.topoptConstraints.ySymmetry,
                           liveVar.topoptConstraints.zSymmetry);


    getStaticFEATitle(&fieldMin,&fieldMax,title,&maxFieldNode);

    double modelScale = vox.getModelScale();
    liveVar.deformationScaled  = 0.05*modelScale/max(1e-16,topopt.getDeformationMax());
    if (liveVar.deformationScaled  < 0) liveVar.deformationScaled  = 1;
    if (!liveVar.isDeformationScaled) liveVar.deformationScaled  = 1;

    liveVar.animationVariable += 3.1415*10/180;
    switch (liveVar.displayField) {
        case DISPLAY_FIELD_DISPLACEMENT:
        case DISPLAY_FIELD_DISPLACEMENT_X:
        case DISPLAY_FIELD_DISPLACEMENT_Y:
        case DISPLAY_FIELD_DISPLACEMENT_Z:
        case DISPLAY_FIELD_STRESS:
        case DISPLAY_FIELD_TEMPERATURE:
             liveVar.staticAnimationScaling = (1+cos(liveVar.animationVariable))/2;// 0 to 1
            break;
        case DISPLAY_FIELD_MODAL:
             liveVar.staticAnimationScaling = sin(liveVar.animationVariable);//-1 to 1
            break;
    }
    if (liveVar.plotAnimation) animationTimer.start();
    for (int tri = 0; tri < topopt.getNumTessellationTriangles(); tri++) {
        topopt.getTessellationVerticesOfTriangle(tri,x,y,z,triNormal);
        switch (liveVar.displayField) {
            case DISPLAY_FIELD_DISPLACEMENT:
            case DISPLAY_FIELD_DISPLACEMENT_X:
            case DISPLAY_FIELD_DISPLACEMENT_Y:
            case DISPLAY_FIELD_DISPLACEMENT_Z:
            case DISPLAY_FIELD_STRESS:
                topopt.getTessellationElasticitySolutionOfTriangle(tri,u,v,w,vonmises);
                break;
            case DISPLAY_FIELD_TEMPERATURE:
                topopt.getTessellationThermalSolutionOfTriangle(tri, temperature);
                //cout << temperature[0] << " " <<temperature[1] <<  " " << temperature[2] << endl;
                break;
            case DISPLAY_FIELD_MODAL:
                topopt.getTessellationModalSolutionOfTriangle(tri,u,v,w);
                break;
        }

        if (liveVar.plotAnimation) {
            for (int i =0; i < 3;i++) {
                u[i] *= liveVar.staticAnimationScaling;
                v[i] *= liveVar.staticAnimationScaling;
                w[i] *= liveVar.staticAnimationScaling;
                vonmises[i] *= liveVar.staticAnimationScaling;
            }
        }
        plotFieldOverTriangle(x,y,z,u,v,w,vonmises,temperature,dist,liveVar.deformationScaled,triNormal,fieldMin,fieldMax);
    }
    if (liveVar.geomTransparent)  plotGeometry();
    if (liveVar.plotAnimation) {
        while (animationTimer.getElapsedTimeInSec() < 0.05)// don't animate too fast
            ;
    }
    switch (liveVar.displayField) {
        case DISPLAY_FIELD_DISPLACEMENT:
        case DISPLAY_FIELD_DISPLACEMENT_X:
        case DISPLAY_FIELD_DISPLACEMENT_Y:
        case DISPLAY_FIELD_DISPLACEMENT_Z:
        case DISPLAY_FIELD_STRESS:
            plotStructuralAnalysisResults();
            plotColorBars();
            break;
        case DISPLAY_FIELD_TEMPERATURE:
            plotThermalAnalysisResults();
            plotColorBars();
            break;
        case DISPLAY_FIELD_MODAL:
            plotModalResults();
            break;
    }
}
/**********************************************************************/
void Renderer::plotWorkSpacePatch(void)
// Plot mesh with different colors for workspace patch
{
    GLfloat color[4] ={0.0, 0.0, 0.0};
    int grp,NumNodes = topopt.getNumNodes();
    double pt[3];

    glPointSize(8.0);
    glDisable( GL_LIGHTING );
    for (int node = 0; node < NumNodes; node++) {
        if (!topopt.doesNodeExist(node)) continue;
        grp = topopt.getWorkspaceGroup(node);
        if (topopt.getNumNodesInGroup(grp) == 0) continue;
        getColorOfPart(grp,color);
        glColor3f(color[0],color[1],color[2]);
        topopt.getNodeXYZ(node,pt);
        glBegin(GL_POINTS);
        glVertex3f(pt[0],pt[1],pt[2]);
        glEnd();
    }
    glEnable( GL_LIGHTING );
}
/**********************************************************************/
void Renderer::plotFinalSTL(void)
{
    int savedCounter,isoSurfaceId;
    double actualVolumeFraction,isoCutoff,timeInSecs,triCenter[3],boundingBox[3][2];
    double xTri[3],yTri[3],zTri[3],triNormal[3],vertexNormals0[3], vertexNormals1[3], vertexNormals2[3],p01[3],p02[3];

	displayStatusMessage();
    savedCounter = topopt.getClosestCounterAtVolFraction(liveVar.displayVolFraction,&actualVolumeFraction,&isoCutoff, &timeInSecs);
    if (savedCounter < 0) 	return;
    if (topopt.getNumIsoTriangles(savedCounter) == 0) return;// the iso surface has not been saved as yet

    if (liveVar.plotAxis) drawAxis();
    if (liveVar.plotElasticityBC) plotElasticityBC();
    if (liveVar.plotThermalBC) plotThermalBC();
    if (liveVar.plotBoundingBox) plotBoundingBox();
    if (liveVar.plotTopOptConstraints)
        plotSymmetryPlanes(liveVar.topoptConstraints.xSymmetry,
                           liveVar.topoptConstraints.ySymmetry,
                           liveVar.topoptConstraints.zSymmetry);

    GLfloat color[4];
    color[0] = 1.0;
    color[1] = 0.0;
    color[2] = 0.0;
    color[3] = 0.9;

    topopt.getBoundingBox(boundingBox);
    float cuttingValue;
    double cuttingXYZ[3];

    cuttingValue = liveVar.cuttingPlaneValue;

    for (int i =0; i < 3; i++)
        cuttingXYZ[i] = (boundingBox[i][0] + boundingBox[i][1])/2 + (cuttingValue)*(boundingBox[i][1] - boundingBox[i][0])/2;

    // check if vertex normals are available
    bool vertexNormalsPresent = false;
    topopt.getIsoTriPts(savedCounter,0,xTri,yTri,zTri,triNormal,vertexNormals0, vertexNormals1, vertexNormals2);// get the first triangle

    if (NORM3D(vertexNormals0) > 0.5)// check to see if vertex normals are available
        vertexNormalsPresent = true;

    glEnable( GL_LIGHTING );
    for (int tri = 0; tri < topopt.getNumIsoTriangles(savedCounter); tri++) {
        topopt.getIsoTriPts(savedCounter,tri,xTri,yTri,zTri,triNormal,vertexNormals0, vertexNormals1, vertexNormals2);

        if (liveVar.cuttingPlane != NO_CUTTINGPLANE){//there is a cutting plane
            triCenter[0] = (xTri[0] + xTri[1] + xTri[2])/3;
            triCenter[1] = (yTri[0] + yTri[1] + yTri[2])/3;
            triCenter[2] = (zTri[0] + zTri[1] + zTri[2])/3;
            switch (liveVar.cuttingPlane) {
            case XYPLUS_CUTTINGPLANE:
                if (triCenter[2] > cuttingXYZ[2]) continue;
                break;
            case XYMINUS_CUTTINGPLANE:
                if (triCenter[2] < cuttingXYZ[2]) continue;
                break;
            case XZPLUS_CUTTINGPLANE:
                if (triCenter[1] > cuttingXYZ[1]) continue;
                break;
            case XZMINUS_CUTTINGPLANE:
                if (triCenter[1] < cuttingXYZ[1]) continue;
                break;
            case YZPLUS_CUTTINGPLANE:
                if (triCenter[0] > cuttingXYZ[0]) continue;
                break;
            case YZMINUS_CUTTINGPLANE:
                if (triCenter[0] < cuttingXYZ[0]) continue;
                break;
            }
        }
        isoSurfaceId = topopt.getIsoTriPartNum(savedCounter,tri);//  isosurface id from 0 onwards
        getColorOfIsosurface(isoSurfaceId,color);
        // else use previous color
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
         //glColor4f(color[0],color[1],color[2],color[3]);
        if (vertexNormalsPresent) {
            glBegin(GL_TRIANGLES);
                            glNormal3f(vertexNormals0[0],vertexNormals0[1],vertexNormals0[2]);
                            glVertex3d(xTri[0],yTri[0],zTri[0]);
                            glNormal3f(vertexNormals1[0],vertexNormals1[1],vertexNormals1[2]);
                            glVertex3d(xTri[1],yTri[1],zTri[1]);
                            glNormal3f(vertexNormals2[0],vertexNormals2[1],vertexNormals2[2]);
                            glVertex3d(xTri[2],yTri[2],zTri[2]);
                        glEnd();
        }
        else {
            p01[0] = xTri[1]-xTri[0];
            p01[1] = yTri[1]-yTri[0];
            p01[2] = zTri[1]-zTri[0];

            p02[0] = xTri[2]-xTri[0];
            p02[1] = yTri[2]-yTri[0];
            p02[2] = zTri[2]-zTri[0];
            CROSS3D(triNormal,p01,p02);
            double nrm = NORM3D(triNormal);
            SCALE(triNormal,nrm);
            glNormal3f(1.0*triNormal[0],1.0*triNormal[1],1.0*triNormal[2]);
            glBegin(GL_TRIANGLES);
            glVertex3d(xTri[0],yTri[0],zTri[0]);
            glVertex3d(xTri[1],yTri[1],zTri[1]);
            glVertex3d(xTri[2],yTri[2],zTri[2]);
            glEnd();
        }
        if (false){
            glLineWidth(0.5);
            glEnable(GL_COLOR_MATERIAL);
            glColor3f(0.0f,0.0f,0.0f);

            glBegin(GL_LINE_STRIP);
            glVertex3f(xTri[0],yTri[0],zTri[0]);
            glVertex3f(xTri[1],yTri[1],zTri[1]);
            glVertex3f(xTri[2],yTri[2],zTri[2]);
            glVertex3f(xTri[0],yTri[0],zTri[0]);
            glEnd();
            glDisable(GL_COLOR_MATERIAL);
        }


    }

    glDisable(GL_BLEND);
    glLineWidth(2.0);
    glDisable(GL_COLOR_MATERIAL);

    if (liveVar.geomTransparent)   plotGeometry();
    if (liveVar.plotNonDesignGeometry) plotNonDesignSTLGeom(false);
    if (liveVar.plotSupportStructures)
        plotSupportStructuresFinalTopology(liveVar.topoptConstraints.AMBuildDirection);

    glDisable(GL_LIGHTING);
    if (liveVar.LATTICE_DESIGNED) {
        plotLatticeResults();
    }
    else
        plotTopOptResults();
}
/**********************************************************************/
void Renderer::plotFieldOverNonDesignSTL(void)
{
    double triNormal[3],p01[3],p02[3],fieldMin,fieldMax,maxFieldPt[3];
    double xTri[3], yTri[3], zTri[3],u[3],v[3],w[3],vonmises[3],voxelsize[3], dist[3],temperature[3],pt[3],uvw[3],stresses[6];
    char title[256];
    int maxDepth,maxFieldNode,tri, part;
    bool validElem;
    double ptClosest[3];


    getStaticFEATitle(&fieldMin,&fieldMax,title,&maxFieldNode);
    double modelScale = vox.getModelScale();
    double scale = 0.025*modelScale/topopt.getDeformationMax();
    if (scale < 0) scale = 1;
    maxDepth = 20;
    if (liveVar.plotAxis) drawAxis();
    double edgeLengthDesired = 0.05*modelScale/liveVar.displayQuality;
    vox.getVoxelSize(voxelsize);
    edgeLengthDesired = MAX(MAX(MAX(edgeLengthDesired,2*voxelsize[0]),2*voxelsize[1]),2*voxelsize[2]);
    topopt.getNodeXYZ(maxFieldNode,maxFieldPt);

    vox.distanceOfPointToSTLGeom(maxFieldPt,ptClosest,&tri);
    for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
        part = vox.getParentPartOfTriangle(tri);
        if(topopt.shouldPartBeOptimized(part)) continue;

        vox.getTriPts(tri,xTri,yTri,zTri);
        for (int i =0; i < 3; i++) {
            pt[0] = xTri[i];pt[1] = yTri[i]; pt[2] = zTri[i];
            validElem = topopt.getElasticitySolutionAt(pt,uvw,stresses,&vonmises[i]);
            validElem = topopt.getTemperatureAt(pt,&temperature[i]);
            u[i] = uvw[0];v[i] = uvw[1]; w[i] = uvw[2];
            if (!validElem) dist[i] = 20*voxelsize[0];
            else dist[i] = 0;
        }
        if (liveVar.plotAnimation) {
            for (int i =0; i < 3;i++) {
                u[i] *= liveVar.staticAnimationScaling;
                v[i] *= liveVar.staticAnimationScaling;
                w[i] *= liveVar.staticAnimationScaling;
                vonmises[i] *= liveVar.staticAnimationScaling;
            }
        }
        p01[0] = xTri[1]-xTri[0];
        p01[1] = yTri[1]-yTri[0];
        p01[2] = zTri[1]-zTri[0];

        p02[0] = xTri[2]-xTri[0];
        p02[1] = yTri[2]-yTri[0];
        p02[2] = zTri[2]-zTri[0];
        CROSS3D(triNormal,p01,p02);
        plotFieldOverTriangleRecursive(xTri,yTri,zTri,scale,u,v,w,vonmises,temperature,dist,triNormal,fieldMin,fieldMax,edgeLengthDesired,maxDepth);
    }

    plotColorBars();
    if (liveVar.geomTransparent)
        plotGeometry();
}
/**********************************************************************/
void Renderer::plotFieldOverFinalSTL(void)
{
    double xTri[3],yTri[3],zTri[3],triNormal[3],offset[]={0,0,0},modelScale,fieldMin,fieldMax, vertexNormals0[3], vertexNormals1[3], vertexNormals2[3];
    GLfloat color[4] ={0.7, 0.3, 0.0, 0.9};
    GLfloat shiny[4] ={100,100, 100, 1};
    double dist[3],u[3],v[3],w[3],vonmises[3],temperature[3];
    int maxFieldNode;
    char title[256];
    int savedCounter;
    Timer animationTimer;

    if (!liveVar.isTopOptComplete) return;

    if (liveVar.plotAxis) drawAxis();
    if (liveVar.plotElasticityBC) plotElasticityBC();
    if (liveVar.plotThermalBC) plotThermalBC();
    if (liveVar.plotBoundingBox) plotBoundingBox();
    if (liveVar.plotTopOptConstraints)
        plotSymmetryPlanes(liveVar.topoptConstraints.xSymmetry,
                           liveVar.topoptConstraints.ySymmetry,
                           liveVar.topoptConstraints.zSymmetry);

    savedCounter = topopt.getCurrentIsoCounter();//can only plot field on the final stl

    modelScale = vox.getModelScale();
    liveVar.deformationScaled  = 0.025*modelScale/topopt.getDeformationMax();
    if (liveVar.deformationScaled  < 0) liveVar.deformationScaled  = 1;
    if (!liveVar.isDeformationScaled) liveVar.deformationScaled  = 1;

    getStaticFEATitle(&fieldMin,&fieldMax,title,&maxFieldNode);

    // check if vertex normals are available
    bool vertexNormalsPresent = false;
    topopt.getIsoTriPts(savedCounter,0,xTri,yTri,zTri,triNormal,vertexNormals0,vertexNormals1,vertexNormals2);// get the first triangle
    if (NORM3D(vertexNormals0) > 0.5)// check to see if vertex normals are available
        vertexNormalsPresent = true;

    liveVar.animationVariable += 3.1415*10/180;
    liveVar.staticAnimationScaling = (1+cos(liveVar.animationVariable))/2;

    if (liveVar.plotAnimation) animationTimer.start();
    for (int tri = 0; tri < topopt.getNumIsoTriangles(savedCounter); tri++) {
        topopt.getIsoTriPts(savedCounter,tri,xTri,yTri,zTri,triNormal,vertexNormals0,vertexNormals1,vertexNormals2);
        switch (liveVar.displayField) {
        case DISPLAY_FIELD_DISPLACEMENT:
        case DISPLAY_FIELD_STRESS:
           topopt.getFinalIsoElasticitySolutionOfTriangle(tri,u,v,w,vonmises);
           break;
        case DISPLAY_FIELD_TEMPERATURE:
            topopt.getFinalIsoTemperatureOfTriangle(tri,temperature);
            break;
        case DISPLAY_FIELD_MODAL:
            topopt.getFinalModalSolutionOfTriangle(tri,u,v,w);
            break;
        }
        if (liveVar.plotAnimation) {
            for (int i =0; i < 3;i++) {
                u[i] *= liveVar.staticAnimationScaling;
                v[i] *= liveVar.staticAnimationScaling;
                w[i] *= liveVar.staticAnimationScaling;
                vonmises[i] *= liveVar.staticAnimationScaling;
            }
        }
        if (!vertexNormalsPresent)
            plotFieldOverTriangle(xTri,yTri,zTri,u,v,w,vonmises,temperature,dist,liveVar.deformationScaled,triNormal,fieldMin,fieldMax);
        else
            plotFieldOverTriangleSmooth(xTri,yTri,zTri,u,v,w,vonmises,temperature,dist,liveVar.deformationScaled,triNormal,vertexNormals0,vertexNormals1,vertexNormals2,fieldMin,fieldMax);
    }
    glDisable( GL_BLEND );
    glLineWidth(2.0);
    glDisable(GL_COLOR_MATERIAL);


    if (liveVar.geomTransparent)  plotGeometry();
    plotFieldOverNonDesignSTL();

    switch (liveVar.displayField) {
        case DISPLAY_FIELD_DISPLACEMENT:
        case DISPLAY_FIELD_STRESS:
            plotTopOptResults();
            plotColorBars();
            break;
        case DISPLAY_FIELD_TEMPERATURE:
            plotTopOptResults();
            break;
        case DISPLAY_FIELD_MODAL:
            plotTopOptResults();
            break;
    }
    if (liveVar.plotAnimation) {
        while (animationTimer.getElapsedTimeInSec() < 0.05)// don't animate to fast
            ;
    }
}
/**********************************************************************/
void Renderer::plotMeshSurfacePseudoDensity(void)
{
    int elem,nodes[8],neighboringElemsOfElem[27],neighboringElem;
    double x[8],y[8],z[8],temp,endPt[3];
    double p0[3],p1[3],p2[3],triNormal[3],p01[3],p02[3],scale,maxDeformation;
    int quads[6][4] = {{0,4,7,3},{0,1,5,4},{0,3,2,1},{1,2,6,5},{2,3,7,6},{4,5,6,7}};
    GLfloat color[4] ={0.1,0.95, 0.1, 0.0};
    double modelScale;
    bool neighboringElemsExists[7];


    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

    modelScale = vox.getModelScale();
    maxDeformation = topopt.getDeformationMax();
    scale = 0.05*modelScale/maxDeformation;
    if (liveVar.plotAxis) drawAxis();
    glLineWidth(0.2);
    glPointSize(5.0);
    for (elem = 0; elem < topopt.getNumElems(); elem++) {
        if (topopt.getElemPseudoDensity(elem) < 1e-6) continue;
        topopt.getAllNeighboringElementsOfElem(elem,neighboringElemsOfElem);
        // 0 is self, 1 is x-, 2 is y-,3 is z-, 4 is x+,5 is y+, 6 is z+
        neighboringElemsExists[0] = true;//self
        for (int m = 1; m < 7; m++) {// start with 1 for neighbors
            neighboringElemsExists[m] = false;
            neighboringElem = neighboringElemsOfElem[m];
            if (neighboringElem == -1) continue;
            if (topopt.getElemPseudoDensity(neighboringElem) < 1e-6) continue;
            neighboringElemsExists[m] = true;
        }
        topopt.getElemNodes(elem,nodes);
        topopt.getElemNodeLocation(elem,x,y,z);
        temp = topopt.getElemPseudoDensity(elem);
        getInterpolatedColor(temp, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        for (int m = 0; m < 6;m++) {// consider the six quads
            // make sure all 4 nodes are on boundary
            if (neighboringElemsExists[m+1]) continue;//offset by 1, don't plot boundary if neighbor exists
            // use 3 pts to find the normal
            p0[0] = x[quads[m][0]];
            p0[1] = y[quads[m][0]];
            p0[2] = z[quads[m][0]];

            p1[0] = x[quads[m][1]];
            p1[1] = y[quads[m][1]];
            p1[2] = z[quads[m][1]];

            p2[0] = x[quads[m][2]];
            p2[1] = y[quads[m][2]];
            p2[2] = z[quads[m][2]];


            for (int i = 0; i < 3; i++) {
                p01[i] = p1[i] - p0[i];
                p02[i] = p2[i] - p0[i];
            }
            CROSS3D(triNormal,p01,p02);

            glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
            glBegin(GL_QUADS);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();

            glColor3f(0.0f,0.0f,0.0f);
            glDisable( GL_LIGHTING );
            // 4 lines per quad
            glBegin(GL_LINE_LOOP);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
            glEnable( GL_LIGHTING );
        }
    }

    glDisable( GL_LIGHTING );
    if (liveVar.plotElasticityBC) {
        int node;
        double force[3],pt[3],magnitude;

        // Now plot all the boundary conditions on nodes
        for (node = 0;node < topopt.getNumNodes();node++) {
            if (!topopt.isNodeFree(node)) {//fixed
                glColor3f(0.0f,0.0f,0.0f);
                topopt.getNodeXYZ(node,pt);
                glBegin(GL_POINTS);
                glVertex3f(pt[0],pt[1],pt[2]);
                glEnd();
                continue;
            }
            if (topopt.isNodeRetained(node)) {//node retained
                glColor3f(0.0f,1.0f,1.0f);
                topopt.getNodeXYZ(node,pt);
                glBegin(GL_POINTS);
                glVertex3f(pt[0],pt[1],pt[2]);
                glEnd();
                continue;
            }
            if (topopt.isNodeOutput(node)) {//node output for compliance
                glColor3f(0.5f,0.5f,0.2f);
                topopt.getNodeXYZ(node,pt);
                glBegin(GL_POINTS);
                glVertex3f(pt[0],pt[1],pt[2]);
                glEnd();
                continue;
            }
            topopt.getNeumannForce(node,force);
            magnitude = NORM3D(force);

            if (magnitude >0) {// force exists on Neumann node
                double scale = 0.015*modelScale;
                topopt.getNodeXYZ(node,pt);

                endPt[0] = pt[0] + scale*force[0]/magnitude;
                endPt[1] = pt[1]+ scale*force[1]/magnitude;
                endPt[2] = pt[2]+ scale*force[2]/magnitude;
                glColor3f(0.9f,0.0f,0.0f);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
                drawNiceArrow(pt,endPt,true,false,0.15);
            }
        }
    }

    if (liveVar.geomTransparent) plotGeometry();
    if (liveVar.plotNormals) drawNormals();
}
/**********************************************************************/
void Renderer::plotEigenModeOverSTL(void)
{
    double triNormal[3],p01[3],p02[3],fieldMin,fieldMax;
    double xTri[3], yTri[3], zTri[3],u[3],v[3],w[3],delta[3],voxelsize[3],deformationMax;
    char title[256];
    int maxDepth,voxelNodes[3];
    Timer animationTimer;


    if (liveVar.plotAxis) drawAxis();
    int modeToDisplay = min(liveVar.nEigenModes,liveVar.displayEigenMode)-1;//offset by 1
    deformationMax = 0;
    liveVar.animationVariable += 3.1415*10/180;
    if (liveVar.plotAnimation)
        liveVar.modalAnimationScaling = sin(liveVar.animationVariable);
    else
        liveVar.modalAnimationScaling = 1.0;

    for (int node = 0; node < topopt.getNumNodes(); node++) {
        u[0] = liveVar.eigenModes[modeToDisplay][3*node];
        v[0] = liveVar.eigenModes[modeToDisplay][3*node+1];
        w[0] = liveVar.eigenModes[modeToDisplay][3*node+2];
        delta[0] = sqrt(u[0]*u[0] + v[0]*v[0]+ w[0]*w[0]);
        deformationMax = MAX(deformationMax,delta[0]);
    }
    if (liveVar.plotAnimation) animationTimer.start();
    double modelScale = vox.getModelScale();
    double scale = 0.025*modelScale/deformationMax;
    maxDepth = 20;
    double edgeLengthDesired = 0.25*modelScale/liveVar.displayQuality;
    vox.getVoxelSize(voxelsize);
    edgeLengthDesired = MAX(MAX(MAX(edgeLengthDesired,2*voxelsize[0]),2*voxelsize[1]),2*voxelsize[2]);
    fieldMin = 0;
    fieldMax = deformationMax;
    for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
        vox.getTriPts(tri,xTri,yTri,zTri);

        //get the voxel node closest to the three points
        vox.closestVoxelNodeToPoint(xTri[0],yTri[0],zTri[0],&(voxelNodes[0]));
        vox.closestVoxelNodeToPoint(xTri[1],yTri[1],zTri[1],&(voxelNodes[1]));
        vox.closestVoxelNodeToPoint(xTri[2],yTri[2],zTri[2],&(voxelNodes[2]));

        if ( ( voxelNodes[0] == -1) || ( voxelNodes[1] == -1) || ( voxelNodes[2] == -1) ) {
            continue;// error!!!
        }
        // get the field values at these nodes
        for (int i =0; i < 3; i++) {
            u[i] = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxelNodes[i]];
            v[i] = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxelNodes[i]+1];
            w[i] = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxelNodes[i]+2];
        }
        p01[0] = xTri[1]-xTri[0];
        p01[1] = yTri[1]-yTri[0];
        p01[2] = zTri[1]-zTri[0];

        p02[0] = xTri[2]-xTri[0];
        p02[1] = yTri[2]-yTri[0];
        p02[2] = zTri[2]-zTri[0];
        CROSS3D(triNormal,p01,p02);
        plotEigenModeOverTriangleRecursive(xTri,yTri,zTri,scale,u,v,w,triNormal,fieldMin,fieldMax,edgeLengthDesired,maxDepth);
    }
    if (liveVar.geomTransparent) plotGeometry();
    if (liveVar.plotAnimation) {
        while (animationTimer.getElapsedTimeInSec() < 0.05)// don't animate too fast
            ;
    }

    plotModalResults();
}
/**********************************************************************/
void Renderer::plotEigenModeOverFinalSTL(void)
{
    double triNormal[3],p01[3],p02[3],fieldMin,fieldMax;
    double  vertexNormals0[3], vertexNormals1[3], vertexNormals2[3];
    double xTri[3], yTri[3], zTri[3],u[3],v[3],w[3],delta[3],voxelsize[3],deformationMax;
    char title[256];
    int maxDepth,voxelNodes[3];
    Timer animationTimer;


    if (liveVar.plotAxis) drawAxis();
    int modeToDisplay = min(liveVar.nEigenModes,liveVar.displayEigenMode)-1;//offset by 1
    deformationMax = 0;
    liveVar.animationVariable += 3.1415*10/180;
    if (liveVar.plotAnimation)
        liveVar.modalAnimationScaling = sin(liveVar.animationVariable);
    else
        liveVar.modalAnimationScaling = 1.0;

    for (int node = 0; node < topopt.getNumNodes(); node++) {
        u[0] = liveVar.eigenModes[modeToDisplay][3*node];
        v[0] = liveVar.eigenModes[modeToDisplay][3*node+1];
        w[0] = liveVar.eigenModes[modeToDisplay][3*node+2];
        delta[0] = sqrt(u[0]*u[0] + v[0]*v[0]+ w[0]*w[0]);
        deformationMax = MAX(deformationMax,delta[0]);
    }
    if (liveVar.plotAnimation) animationTimer.start();
    double modelScale = vox.getModelScale();
    double scale = 0.025*modelScale/deformationMax;
    maxDepth = 20;
    double edgeLengthDesired = 0.25*modelScale/liveVar.displayQuality;
    vox.getVoxelSize(voxelsize);
    edgeLengthDesired = MAX(MAX(MAX(edgeLengthDesired,2*voxelsize[0]),2*voxelsize[1]),2*voxelsize[2]);
    fieldMin = 0;
    fieldMax = deformationMax;
    int savedCounter = topopt.getCurrentIsoCounter();//can only plot field on the final stl

    for (int tri = 0; tri < topopt.getNumTessellationTriangles(); tri++) {
        topopt.getIsoTriPts(savedCounter,tri,xTri,yTri,zTri,triNormal,vertexNormals0,vertexNormals1,vertexNormals2);


        //get the voxel node closest to the three points
        voxelNodes[0] =  topopt.findNonVoidNodelosestTo(xTri[0],yTri[0],zTri[0],2.0);
        voxelNodes[1] =  topopt.findNonVoidNodelosestTo(xTri[1],yTri[1],zTri[1],2.0);
        voxelNodes[2] =  topopt.findNonVoidNodelosestTo(xTri[2],yTri[2],zTri[2],2.0);
        /*
        vox.closestVoxelNodeToPoint(xTri[0],yTri[0],zTri[0],&(voxelNodes[0]));
        vox.closestVoxelNodeToPoint(xTri[1],yTri[1],zTri[1],&(voxelNodes[1]));
        vox.closestVoxelNodeToPoint(xTri[2],yTri[2],zTri[2],&(voxelNodes[2]));
        */
        if ( ( voxelNodes[0] == -1) || ( voxelNodes[1] == -1) || ( voxelNodes[2] == -1) ) {
            continue;// error!!!
        }
        // get the field values at these nodes
        for (int i =0; i < 3; i++) {
            u[i] = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxelNodes[i]];
            v[i] = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxelNodes[i]+1];
            w[i] = liveVar.modalAnimationScaling*liveVar.eigenModes[modeToDisplay][3*voxelNodes[i]+2];
        }
        p01[0] = xTri[1]-xTri[0];
        p01[1] = yTri[1]-yTri[0];
        p01[2] = zTri[1]-zTri[0];

        p02[0] = xTri[2]-xTri[0];
        p02[1] = yTri[2]-yTri[0];
        p02[2] = zTri[2]-zTri[0];
        CROSS3D(triNormal,p01,p02);
        plotEigenModeOverTriangleRecursive(xTri,yTri,zTri,scale,u,v,w,triNormal,fieldMin,fieldMax,edgeLengthDesired,maxDepth);
    }
    if (liveVar.geomTransparent) plotGeometry();
    if (liveVar.plotAnimation) {
        while (animationTimer.getElapsedTimeInSec() < 0.05)// don't animate too fast
            ;
    }

    plotTopOptResults();
}
/****************************************************************/
void Renderer::plotBooleanFieldOverSTLMesh(bool* fieldVariable,bool displayEdges )
{
    // given an STL mesh with field variable the size of numTriangles, plot selectively whether
    // the field varialbe is true or false over the mesh
    double xTri[3],yTri[3],zTri[3],p01[3],p02[3],triNormal[3];
    double xEdge[2],yEdge[2],zEdge[2],minLength,maxLength;
    char geomData[2056],tempstr[512],materialName[256];
    GLfloat color[4] ={0,0, 0, 0.2};
    GLfloat shiny[4] ={100,100, 100, 1};
    int nGroups,group,part;

    glPushMatrix();
    // Provide basic information on the geometry
    sprintf(geomData,"Name: %s \nTriangles: %d\nParts: %d\n",liveVar.modelName.c_str(),vox.getNumTriangles(),vox.getNumParts() );


    vox.getMinAndMaxLengths(&minLength,&maxLength);
    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        sprintf(tempstr,"Units: m,N,s\nMax length: %.3f meters\n",maxLength);
        break;
    case UNITS_MM:
        sprintf(tempstr,"Units: mm,N,s\nMax length: %.3f mms\n",maxLength);
        break;
    case UNITS_INCH:
        sprintf(tempstr,"Units: in,lb,s\nMax length: %.3f inches\n",maxLength);
        break;
    }
    strcat(geomData,tempstr);
    for (int part = 0; part <vox.getNumParts(); part++) {
        topopt.getMaterialNameOfPart(part,materialName);
        sprintf(tempstr,"Part %d: %s\n",part,materialName);
        strcat(geomData,tempstr);
    }

    //if (!liveVar.geomTransparent) plotDataString(geomData);// if transparent then we may be displaying stresses etc

    nGroups = vox.getNumTriGroups();

    for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
        part = vox.getParentPartOfTriangle(tri);
        if ( (!liveVar.plotNonDesignGeometry) &&	(!topopt.shouldPartBeOptimized(part))) continue;
        vox.getTriPts(tri,xTri,yTri,zTri);
        group = vox.getTriGroup(tri);
        p01[0] = xTri[1]-xTri[0];
        p01[1] = yTri[1]-yTri[0];
        p01[2] = zTri[1]-zTri[0];

        p02[0] = xTri[2]-xTri[0];
        p02[1] = yTri[2]-yTri[0];
        p02[2] = zTri[2]-zTri[0];
        CROSS3D(triNormal,p01,p02);


        if (fieldVariable[tri]){ // if the triangle is set to true, highlight it
            color[0] = 1.0;
            color[1] = 0.0;
            color[2] = 0.0;
        }
        else{
            getColorOfPart(part,color);
        }
        glDisable(GL_COLOR_MATERIAL);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
        glNormal3f(triNormal[0],triNormal[1],triNormal[2]);

        glPolygonMode(GL_FRONT, GL_FILL);

        glBegin(GL_TRIANGLES);
        glVertex3f(xTri[0],yTri[0],zTri[0]);
        glVertex3f(xTri[1],yTri[1],zTri[1]);
        glVertex3f(xTri[2],yTri[2],zTri[2]);
        glEnd();

        if (displayEdges){
            glEnable(GL_COLOR_MATERIAL);
            glColor3f(0.0f,0.0f,0.0f);
            glPolygonMode(GL_FRONT, GL_LINE);
            glBegin(GL_TRIANGLES);
            glVertex3f(xTri[0],yTri[0],zTri[0]);
            glVertex3f(xTri[1],yTri[1],zTri[1]);
            glVertex3f(xTri[2],yTri[2],zTri[2]);
            glEnd();
        }

    }
    glEnable(GL_COLOR_MATERIAL);
    glDisable( GL_BLEND );
    glLineWidth(2.0);
    glColor3f(0,0,0);
    //plot Sharp edges
    for (int e = 0; e < vox.getNumSharpEdges(); e++) {
        vox.getSharpEdgePoints(e,xEdge,yEdge,zEdge);
        glBegin(GL_LINES);
        glVertex3d(xEdge[0],yEdge[0],zEdge[0]);
        glVertex3d(xEdge[1],yEdge[1],zEdge[1]);
        glEnd();
    }
    if (liveVar.showConcaveEdges) {
        glDisable(GL_DEPTH_TEST);
        //plot Concave edges
        for (int e = 0; e < vox.getNumSharpEdges(); e++) {
            if (vox.isEdgeConcave(e)){
                vox.getSharpEdgePoints(e,xEdge,yEdge,zEdge);
                glLineWidth(4.0);
                glColor3f(1,0,0);
                glBegin(GL_LINES);
                glVertex3d(xEdge[0],yEdge[0],zEdge[0]);
                glVertex3d(xEdge[1],yEdge[1],zEdge[1]);
                glEnd();
            }
        }
        glEnable(GL_DEPTH_TEST);
    }
    glDisable(GL_COLOR_MATERIAL);
    glPopMatrix();
}
/**********************************************************************/
