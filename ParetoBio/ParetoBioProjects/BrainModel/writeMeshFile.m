close all
format compact

load('Element.mat')
load('Node.mat')
[nElems,nElemCols] = size(element);
[nNodes,nNodeCols] = size(Node);



xMin = min(Node(:,2));
xMax = max(Node(:,2));
yMin = min(Node(:,3));
yMax = max(Node(:,3));
zMin = min(Node(:,4));
zMax = max(Node(:,4));

nodesElems1 = element(1,3:10);
XYZElems1 = Node(nodesElems1,2:4);
%plot3(XYZElems1(:,1),XYZElems1(:,2),XYZElems1(:,3),'*');axis equal
dx = abs(XYZElems1(1,1)-XYZElems1(8,1));
dy = abs(XYZElems1(2,2)-XYZElems1(1,2));
dz = abs(XYZElems1(2,3)-XYZElems1(3,3));

nelx = round((xMax-xMin)/dx);
nely = round((yMax-yMin)/dy);
nelz = round((zMax-zMin)/dz);
% Print in binary format for Pareto
% 1. nelx nely nelz (#number of elements in x, y and z)
% 2. X0 Y0 Z0 (origin)
% 3. dx dy dz (deltax, deltay, deltaz)
% 4. h_nNodes
% 5. (i,j,k,l) of all nodes (i ranges from 0 to nelx, etc), l is the label of node
% 6. h_nElems
% 7. (n1,n2,...,n8) of all elements
% 8. m0 m1 m2 ....) for each element, the id (typically part #), default is 0
% 9. 1 1 0.7 1 0 ....

fileID = fopen('brainParetoMesh.msh','w');
fwrite(fileID,nelx,'uint');
fwrite(fileID,nely,'uint');
fwrite(fileID,nelz,'uint');

fwrite(fileID,xMin,'double');
fwrite(fileID,yMin,'double');
fwrite(fileID,zMin,'double');

fwrite(fileID,dx,'double');
fwrite(fileID,dy,'double');
fwrite(fileID,dz,'double');

fwrite(fileID,nNodes,'uint');
%gridIndices = zeros(nNodes,4);

for node = 1:nNodes
    xi = round((Node(node,2)- xMin)/dx);
    yi = round((Node(node,3)- yMin)/dy);
    zi = round((Node(node,4)- zMin)/dz);
    label = 2; % BC label, default
    if (zi <= 2)
        label = 1; % Dirichlet
    end
    fwrite(fileID,xi,'uint');
    fwrite(fileID,yi,'uint');
    fwrite(fileID,zi,'uint');
    fwrite(fileID,label,'uint');
end

fwrite(fileID,nElems,'uint');
for elem = 1:nElems
    fwrite(fileID,element(elem,3:10)-1,'uint');
end
for elem = 1:nElems
    fwrite(fileID,element(elem,2)-1,'uint'); % material id
    %fwrite(fileID,0,'uint'); % material id
end
for elem = 1:nElems
    fwrite(fileID,1.0,'double'); % pseudo density
end

fclose(fileID);

