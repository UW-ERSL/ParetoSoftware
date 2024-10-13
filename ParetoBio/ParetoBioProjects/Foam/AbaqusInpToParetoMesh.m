% read .inp file and save as .msh file with loading, etc

clear all
format compact
S = readlines("FoamMicrostrutureCompression.inp");

elementDetected = 0;
nodeDetected = 0;

for i = 1:size(S)
    if (~nodeDetected) && (contains(S(i),"*Node"))
        startNodeLine = i;
        nodeDetected = 1;
    end
    if (~elementDetected) && (contains(S(i),"*Element"))
        endNodeLine = i-1;
        elementDetected= 1;
        startElemLine = i;
    end
    if (contains(S(i),"*Nset"))
        endElemLine = i-1;
        break;
    end
end
nNodes = endNodeLine - startNodeLine;
nElems = endElemLine -startElemLine;
Node = zeros(nNodes,3);

for i = startNodeLine+1: endNodeLine
    data = sscanf(S(i),"%d, %e, %e, %e");
    Node(data(1),:) = data(2:4);
end
element = zeros(nElems,8);
for i = startElemLine+1: endElemLine
    data = sscanf(S(i),"%d, %d, %d, %d, %d, %d, %d, %d, %d");
    element(data(1),:) = data(2:9);
end

%% now write the mesh with node labels

xMin = min(Node(:,1));
xMax = max(Node(:,1));
yMin = min(Node(:,2));
yMax = max(Node(:,2));
zMin = min(Node(:,3));
zMax = max(Node(:,3));

nodesElems1 = element(1,1:8);
XYZElems1 = Node(nodesElems1,1:3);
%plot3(XYZElems1(:,1),XYZElems1(:,2),XYZElems1(:,3),'*');axis equal
dx = max(XYZElems1(:,1)) - min(XYZElems1(:,1));
dy = max(XYZElems1(:,2)) - min(XYZElems1(:,2));
dz = max(XYZElems1(:,3)) - min(XYZElems1(:,3));

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

fileID = fopen('foamMesh.msh','w');
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
count = 0;
for node = 1:nNodes
    xi = round((Node(node,1)- xMin)/dx);
    yi = round((Node(node,2)- yMin)/dy);
    zi = round((Node(node,3)- zMin)/dz);
    label = 0; 
    if (zi <= 1) % bottom layers
        label = 1;
    end
    if (zi >= nelz-2) %  top few layers
        label = 2;
        count = count + 1;
    end
    fwrite(fileID,xi,'uint');
    fwrite(fileID,yi,'uint');
    fwrite(fileID,zi,'uint');
    fwrite(fileID,label,'uint');
end
fwrite(fileID,nElems,'uint');
for elem = 1:nElems
    fwrite(fileID,element(elem,:)-1,'uint');
end
for elem = 1:nElems
    fwrite(fileID,0,'uint'); % material id
end
for elem = 1:nElems
    fwrite(fileID,1.0,'double'); % pseudo density
end

fclose(fileID);

