#ifndef NLELASTICITY_H
#define NLELASTICITY_H
#define TCLMatrix dMatrix
#include <string>
#include <vector>
#include "Eigen/Dense"
 
class NLElasticity {
    
    public:
        NLElasticity();  // declare default constructor
        NLElasticity(const std::string& parameter1, int parameter2, int parameter3, int parameter4, 
                           double parameter5, double parameter6, double parameter7);
        std::string meshType;
        std::string materialModel;
        int nElemX ;
        int nElemY ;
        int nElemZ ;
        double sizeX;
        double sizeY;
        double sizeZ;
        void createHexGridMeshGeneral(int nElemX, int nElemY, int nElemZ, double sizeX, double sizeY, double sizeZ);

        std::string myShape;          // Linear or quadratic shape functions
        int myNumNodes;               // Total number of nodes
        int myNumElems;               // Total number of elements
        int myNodesPerSurface;
        int myNCoord;               //cooordinates = 3 for 3D..
        double myElemSize;           // Common element size determined from total length and #elems
        double myElementLength;      // Length of each element
        int myNodesPerElement;       // 2 for linear, 3 for quadratic element
        std::string myElemShape;     // Element shape (hex, etc.)
        bool myApplyFiniteStrain;
        bool myVerbose;
        int myNonlinearSteps;
        double myE; // Young's modulus
        double myNu; // Poisson's ratio
        std::string myMaterialModel; // Material model
        double myBulkModulus; // Bulk modulus
        double myMu; // Shear modulus
        std::vector<std::vector<int>> myBCtype;
        std::vector<std::vector<double>> myBCvalue;

        std::vector<double> myBodyForce; // Vector for body force
        Eigen::VectorXd mySol;
        Eigen::VectorXd myDeltaSol;
        //std::vector<Eigen::VectorXd> myN; // Shape functions
        //Eigen::MatrixXd myXi;             // Integration points
        //Eigen::VectorXd myWt;             // Integration weights
        //Eigen::MatrixXd myD;              // Material property matrix
        int myDOFPerElem;                 // Degrees of freedom per element
        int myDOFPerNode;                 // Degrees of freedom per element
        //Eigen::MatrixXd myBCvalue;       // Boundary condition values
        std::vector<std::vector<double> > myN; // shape functions
        
        // Define myGradN as a vector of matrices
        std::vector<Eigen::MatrixXd> myGradN;


        std::vector<std::vector<double> > myXi; // local coordinate at quadrature point
        std::vector<double> myWt; // weight
        // Define the struct for myMesh
        struct myMesh {
            std::vector<std::vector<double> > p; // Nodal coordinate matrix
            std::vector<std::vector<int> > q;    // Element connectivity matrix
            std::vector<std::vector<int> > e;    // Boundary matrix (added from previous code)
        };

        myMesh mesh;

        //Eigen::VectorXd mySol;
        Eigen::VectorXd myU;
        Eigen::VectorXd myV;
        Eigen::VectorXd myW;
        Eigen::VectorXd myDeformation;
        double myMaxDelta;
        Eigen::MatrixXd myK;
        Eigen::MatrixXd myF;
        void ShapeFunction2D(const std::vector<double>& xi,
                       std::vector<double>& N,
                       std::vector<std::vector<double>>& gradN);
        std::pair<std::vector<std::vector<double> >, std::vector<std::vector<double> > > ShapeFunction(const std::vector<double>& xi);
        std::vector<double> myNodalDirichlet; // Vector for Dirichlet boundary conditions
        std::vector<int> myFixedDOF; // List of fixed degrees of freedom
        std::vector<int> myFixedNodes; // List of fixed nodes
        std::vector<int> myFreeDOF;

       void applyDirichletOnSurfaceFunction(const std::string& uString,
                                        const std::string& vString,
                                        const std::string& wString,
                                        int EdgeNum);
//void applyDirichletOnSurfaceFunction(std::string uString, std::string vString, std::string wString, int edgeNum);
 
        // Function to compute the Jacobian for an element
        std::vector<std::vector<double> > Jacobian(int elem, const std::vector<double> xi);
            // Function to compute the Jacobian with provided nodes
        std::vector<std::vector<double> > JacobianWithNodes(
            const std::vector<std::vector<double> >& positionNodes, const std::vector<double> xi);
        // Function to get the position of nodes
        std::vector<std::vector<double> > GetNodesPosition(const std::vector<int>& nodes);

        //function to compute the determinant of the Jacobian
        std::vector<std::vector<double>> myD; // Material stiffness matrix

        //function to get the material parameter
        void setMaterialParameter(double E);

        //function to get the youngs modulus
        void setYoungsModulus(double E);

        //function to get the poisson ratio
        void setPoissonsRatio(double nu);

        //setting the material model
        void setMaterialModel(const std::string& materialModel);

        //function to get bulk modulus
        void setBulkModulus(double K1);

        //function to get shear modulus
        void setShearModulus(double mu1);

        //function to compute the D matrix
        void computeDMatrix();

        //function for Dirichlet boundary conditions on DOF
        void applyDirichletOnDOF(double value, int dof);

        //function for Dirichlet boundary conditions on edge
        void applyDirichletOnSurface(double value, int SurfaceNum);

        //function for Dirichlet boundary conditions on surface function

        //function for Neumann boundary condition on a given edge
        void applyNeumanOnSurface(double forceU, double forceV, double forceW, size_t SurfaceNum);
        static const int MAX_SURFACE_NUM = 100;  // Adjust the size accordingly
        static const int NUM_BC_TYPES = 3;  // Assuming there are three types (0, 1, 2), adjust as needed

        // Other members of your class...

        // void NLElasticity::computeElementStiffness(int elem, std::vector<std::vector<double>>& KElem,
        //                         std::vector<double>& fElem, double& vol);

        // void integrateKOverElem(int elem, std::vector<std::vector<double>>& KElem,
        // std::vector<double>& fElem, double& vol);
        void printEigenMatrix(const Eigen::MatrixXd& matrix, const std::string& matrixName);
        void GaussQuad2dQuad(int numGQ, std::vector<double>& xi_GQ, std::vector<double>& wt_GQ);
        void GaussQuad3dHex(int numGQ, std::vector<std::vector<double>>& xi_GQ, std::vector<double>& wt_GQ);
        void printVectorOfVectors(const std::vector<std::vector<double>>& vec);

        void lgwt(int N, double a, double b, std::vector<double>& x, std::vector<double>& w);
        void assembleBC();

        std::vector<double> myfC;
        int myNumDOF;
        std::vector<double> myForcedNodes;
        Eigen::VectorXd myNeumanForce;
         // Declaration for myNumBoundaries
        int myNumBoundaries;
        double energyErrNorm;
        double L2ErrNorm;
        double RelativeL2ErrNorm;
        double RelativeEnergyErrNorm;
        int myProblem;
        int myDomainLength;
        Eigen::VectorXd myElemVol;

       std::vector<double> integrateOverBoundary(int geomEdge, int seg, const std::vector<double>& wt_GQ,
                                 const std::vector<Eigen::MatrixXd>& N2D,
                                 const std::vector<Eigen::MatrixXd>& gradN2D, int dof);
        std::vector<std::vector<double>> transpose(const std::vector<std::vector<double>>& matrix);
        std::vector<std::vector<double>> multiplyMatrix(const std::vector<std::vector<double>>& matrix1, const std::vector<std::vector<double>>& matrix2);
        double determinant(const std::vector<std::vector<double>>& matrix);
        
    // Eigen::MatrixXd computeElasticityTensorGeneralizedNeoHookean(
    // const std::string& materialModel, int NCoords, double K1, double mu1,
    // const Eigen::MatrixXd& B, double J);
    
    std::vector<std::vector<double>> eigenMatrixToStdVector(const Eigen::MatrixXd& matrix);
    double trace(const std::vector<std::vector<double>>& matrix);

    std::vector<std::vector<std::vector<std::vector<double>>>> computeElasticityTensorGeneralizedNeoHookean(
        const std::string& materialModel, int NCoords, double K1, double mu1, const std::vector<std::vector<double>>& B, double J);
    void computeElementStiffnessFiniteStrainSpatialConf(const int elem, Eigen::MatrixXd& KElem, Eigen::VectorXd& fElem);
       Eigen::MatrixXd KirchhoffStress(const std::string& materialModel, int NCoords, double K1, double mu1, const Eigen::MatrixXd& B, double J);
       void assembleK();
    
    void solveNonlinearFEM();
    double getConditionNumber(const Eigen::MatrixXd& matrix);

    void solveFEMProblem();


private:
    
    // double determinant2D(const std::vector<std::vector<double>>& matrix);
    // std::vector<std::vector<double>> transposeMatrix(const std::vector<std::vector<double>>& matrix);
    // std::vector<std::vector<double>> scaleMatrix(const std::vector<std::vector<double>>& matrix, double scalar);
    // std::vector<double> scaleVector(const std::vector<double>& vector, double scalar);
    // std::vector<double> addVectors(const std::vector<double>& a, const std::vector<double>& b);
    // std::vector<std::vector<double>> addMatrices(const std::vector<std::vector<double>>& a,
    //                                              const std::vector<std::vector<double>>& b);
    

};

#endif
