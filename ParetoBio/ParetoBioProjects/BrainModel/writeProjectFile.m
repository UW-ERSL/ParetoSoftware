fileID = fopen('brainParetoProject.prj','w');
fprintf(fileID,"#KEY VERSION\n");
fprintf(fileID,"2023.02\n\n");

fprintf(fileID,"#HEXMESH_VERBOSE\n");
fprintf(fileID,"1\n\n");

fprintf(fileID,"#DEFLATION_VERBOSE\n");
fprintf(fileID,"1\n\n");

fprintf(fileID,"#KEY UNITS\n");
fprintf(fileID,"0\n\n");

fprintf(fileID,"#KEY ASSEMBLY\n");
fprintf(fileID,"1\n\n");

fprintf(fileID,"#KEY GEOMETRY\n");
fprintf(fileID,"brainBox.stl\n\n");

nMaterials = 16;
materialNames = ["LeftCerebralWhite", "RightCerebralWhite","LeftCerebellumlWhite",...
    "RightCerebellumlWhite","LeftCerebralGrey","RightCerebralGrey","Ventricles",...
    "LeftCerebellumlGrey","RightCerebellumlGrey","Midbrain","CorpusCollusum",...
    "BrainStem","CSF","Dura","Skull","Other"];
E = 2e8*(1+1000*rand(1,nMaterials));
nu = 0.28*(1+0.01*rand(1,nMaterials));
rho = 7700*(1+0.02*rand(1,nMaterials));
Y = 5e6*(1+0.02*rand(1,nMaterials));
for matl = 1:nMaterials
    fprintf(fileID,"#KEY PART\n");
    fprintf(fileID,"%d\n\n",matl-1);
    
    fprintf(fileID,"#KEY MATERIALNAME\n");
    fprintf(fileID,"%s\n\n",materialNames(matl));

    fprintf(fileID,"#KEY E\n");
    fprintf(fileID,"%g\n\n",E(matl));

    fprintf(fileID,"#KEY nu\n");
    fprintf(fileID,"%g\n\n",nu(matl));

    fprintf(fileID,"#KEY rho\n");
    fprintf(fileID,"%g\n\n",rho(matl));

    fprintf(fileID,"#KEY Y\n");
    fprintf(fileID,"%g\n\n",Y(matl));

    fprintf(fileID,"#KEY MATERIAL_END\n\n");
 
end

fprintf(fileID,"#KEY MESHFILE\n");
fprintf(fileID,"brainParetoMesh.msh\n\n");

fprintf(fileID,"#KEY BCSET\n");
fprintf(fileID,"0\n\n");

fprintf(fileID,"#KEY NODEBC\n");
fprintf(fileID,"0 1 1 1 0.000e+00 0.000e+00 0.000e+00 0.000e+00 0.000e+00 0.000e+00\n\n");

fprintf(fileID,"#KEY NODEBC\n");
fprintf(fileID,"1 0 0 0 0.000e+00 0.000e+00 0.000e+00 0.000e+00 0.000e+00 0.000e+00\n\n");

fprintf(fileID,"#KEY NODEBC\n");
fprintf(fileID,"2 1 1 1 0.000e+00 0.000e+00 1.000e+04 0.000e+00 0.000e+00 0.000e+00\n\n");

fprintf(fileID,"#KEY DEFLATION\n");
fprintf(fileID,"2 2000\n\n");

fprintf(fileID,"#KEY PRECONDITIONER\n");
fprintf(fileID,"1\n\n");

fprintf(fileID,"#KEY SOLVESTATIC_ONMESH\n");
fprintf(fileID,"0\n\n");


fclose(fileID);
