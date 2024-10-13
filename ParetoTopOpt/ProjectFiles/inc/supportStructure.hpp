#include <iostream>
#include<time.h>
#include<vector>
#include "TopOptViewer.hpp"
#include "CoreFunctions.hpp"
extern CoreFunctions        coreFunctions;
extern ParetoApplication    topopt;
extern Voxelizer            vox;
extern LiveVariables        liveVar;
extern Renderer             renderer;
class SupportStructures//: public Ui_SupportStructure
{
private:
    double overhangAngle;
    bool* ss_overhangTriangles;
signals:
    void overhangTriangleIdentified(){}
protected:
    void allocateMemorySupportStructure();
    void clearMemorySupportStructure();
public:
    //

    void identifyOverhangTriangles(double overhangAngle);
};
