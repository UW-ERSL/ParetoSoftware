fileID = fopen('foamMeshProject.prj','w');


fprintf(fileID,"#KEY VERSION\n");
fprintf(fileID,"2024.02\n\n");

fprintf(fileID,"#HEXMESH_VERBOSE\n");
fprintf(fileID,"0\n\n");

fprintf(fileID,"#KEY UNITS\n");
fprintf(fileID,"0\n\n");

fprintf(fileID,"#KEY ASSEMBLY\n");
fprintf(fileID,"0\n\n");

fprintf(fileID,"#KEY GEOMETRY\n");
fprintf(fileID,"foam.stl\n\n");

nMaterials = 1;

E = 70e6;
nu = 0.3;
rho = 7700;
Y = 5e6;

fprintf(fileID,"#KEY PART\n");
fprintf(fileID,"%d\n\n",0);

fprintf(fileID,"#KEY MATERIALNAME\n");
fprintf(fileID,"Steel\n\n");

fprintf(fileID,"#KEY E\n");
fprintf(fileID,"%g\n\n",E);

fprintf(fileID,"#KEY nu\n");
fprintf(fileID,"%g\n\n",nu);

fprintf(fileID,"#KEY rho\n");
fprintf(fileID,"%g\n\n",rho);

fprintf(fileID,"#KEY Y\n");
fprintf(fileID,"%g\n\n",Y);

fprintf(fileID,"#KEY MATERIAL_END\n\n");

fprintf(fileID,"#KEY MESHFILE\n");
fprintf(fileID,"foamMesh.msh\n\n");

fprintf(fileID,"#KEY BCSET\n");
fprintf(fileID,"0\n\n");

fprintf(fileID,"#KEY NODEBC\n");
fprintf(fileID,"0 1 1 1 0.000e+00 0.000e+00 0.000e+00 0.000e+00 0.000e+00 0.000e+00\n\n");

fprintf(fileID,"#KEY NODEBC\n");
fprintf(fileID,"1 0 0 0 0.000e+00 0.000e+00 0.000e+00 0.000e+00 0.000e+00 0.000e+00\n\n");

fprintf(fileID,"#KEY NODEBC\n");
str = sprintf("2 1 1 1 %0.3g %0.3g %0.3g 0.000e+00 0.000e+00 0.000e+00\n\n", 0,0,-0.1);
fprintf(fileID,str);

fprintf(fileID,"#KEY DEFLATION\n");
fprintf(fileID,"2 500\n\n");

fprintf(fileID,"#KEY PRECONDITIONER\n");
fprintf(fileID,"1\n\n");

%fprintf(fileID,"#KEY SOLVESTATIC_ONMESH\n");
%fprintf(fileID,"0\n\n");


fclose(fileID);
