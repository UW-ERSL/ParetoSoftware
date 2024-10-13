format compact;
example = 3;

files = {'cube.tetmesh','cbracket.tetmesh','box.tetmesh','gegrabcad.tetmesh'};
scale = [1000,1000,1000,1000];
inputFile = files{example};


fin = fopen(inputFile,'r');
nNodes = fscanf(fin,"%d\n",1);
nodesXYZ = fscanf(fin,"%g",[3,nNodes]);
nodesXYZ = scale(example)*nodesXYZ;
nElems = fscanf(fin,"%d\n",1);
elemNodes = fscanf(fin,"%d\n",[4,nElems]);
fclose(fin);

maxEdges = 1e6;
edges = zeros(maxEdges,2);
nGraphEdges = 0;
edgeCombinations = [[1,2];[1,3];[1,4];[2,3];[2,4];[3,4]];
for elem = 1:nElems
    nodes = elemNodes(:,elem);
    for i = 1:6
        n1 = nodes(edgeCombinations(i,1));
        n2 = nodes(edgeCombinations(i,2));

        duplicate = false;
        for k = 1:nGraphEdges
            if ((edges(k,1) == n1-1) && (edges(k,2) == n2-1))
                duplicate = true;
                break
            end
            if ((edges(k,2) == n1-1) && (edges(k,1) == n2-1))
                duplicate = true;
                break
            end
        end
        if (~duplicate)
            nGraphEdges = nGraphEdges + 1;
            edges(nGraphEdges,1) = n1-1;
            edges(nGraphEdges,2) = n2-1;
        end

    end
end
nGraphNodes = nNodes;
edges = (edges(1:nGraphEdges,:))';
outputFile = strrep(inputFile,'tetmesh','off');
fout = fopen(outputFile,'w');
fprintf(fout,"OFF\n");
fprintf(fout,"%d 0 %d\n",nGraphNodes,nGraphEdges);
fprintf(fout,"%g %g %g\n",nodesXYZ);
fprintf(fout,"%d %d \n",edges);
fclose(fout);