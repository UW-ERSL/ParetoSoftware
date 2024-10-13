
% Xlength, Ylength and Zlength are defined in test_FEM_neoHookean3D
% define the bounding box here
xMin = 0; xMax = Xlength;
yMin = 0; yMax = Ylength;
zMin = 0; zMax = Zlength;

% the code starts here
stlFptr = fopen('beam.stl','w');
tri =  {{0, 3, 1}, {1, 3, 2}, {0, 5, 4}, {0, 1, 5}, {1, 6, 5}, {1, 2, 6}, {2, 7, 6},{ 2, 3, 7}, {3, 0, 4}, {3, 4, 7}, {4, 5, 7},{ 5, 6, 7}};
	

x = [xMin,xMax,xMax,xMin,xMin,xMax,xMax,xMin];
y = [yMin,yMin,yMax,yMax,yMin,yMin,yMax,yMax];
z = [zMin,zMin,zMin,zMin,zMax,zMax,zMax,zMax];

fprintf(stlFptr,"solid part\n");
for i = 1:12
    nodes = tri{i};
    v1 = [x(nodes{2}+1)-x(nodes{1}+1), y(nodes{2}+1)-y(nodes{1}+1),z(nodes{2}+1)-z(nodes{1}+1)];
    v2 = [x(nodes{3}+1)-x(nodes{1}+1), y(nodes{3}+1)-y(nodes{1}+1),z(nodes{3}+1)-z(nodes{1}+1)];
    v3 = cross(v1,v2);
    nrm = norm(v3);
    v3 = v3/nrm;
    fprintf(stlFptr,"\tfacet normal %f %f %f \n",v3(1),v3(2),v3(3));
    fprintf(stlFptr,"\t\touter loop \n");

    for j = 1:3
        fprintf(stlFptr,"\t\t\t vertex %f %f %f\n",x(nodes{j}+1),y(nodes{j}+1),z(nodes{j}+1));
    end
    fprintf(stlFptr,"\t\tendloop \n");
    fprintf(stlFptr,"\tendfacet \n");
end

fprintf(stlFptr,"endsolid \n");

fclose(stlFptr);
