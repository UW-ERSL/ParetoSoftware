# -*- coding: utf-8 -*-
"""
Created on Thu Sep 19 10:20:38 2024

@author: ksure
"""
import numpy as np
import os
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

plt.close('all')
class paretoInterface:
	# This is the hybrid version of the HHL algorithm where we explicitly 
	# construct and simulate the QPE circuit. The results are the used in 
	# the  HLL circuit
	def __init__(self):	
		self.nNodes = 0
		self.nElems = 0
		
	def readParetoMesh(self,fileName):
		# reads Pareto mesh (binary format)
		# % 1. nelx nely nelz (#number of elements in x, y and z)
		# % 2. X0 Y0 Z0 (origin)
		# % 3. dx dy dz (deltax, deltay, deltaz)
		# % 4. h_nNodes
		# % 5. (i,j,k,l) of all nodes (i ranges from 0 to nelx, etc), l is the label of node
		# % 6. h_nElems
		# % 7. (n1,n2,...,n8) of all elements
		# % 8. m0 m1 m2 ....) for each element, the id (typically part #), default is 0
		# % 9. 1 1 0.7 1 0 .... pseudoDensity for each element
		with open(fileName, mode='rb') as file: # b is important -> binary
			self.grid = np.fromfile(file, dtype=np.uint32,count = 3)  
			self.origin = np.fromfile(file, dtype=np.double,count = 3)  
			self.delta = np.fromfile(file, dtype=np.double,count = 3) 
			self.nNodes = np.fromfile(file, dtype=np.uint32, count = 1)[0]
			self.nodeArray = np.fromfile(file, dtype=np.uint32, count = 4*self.nNodes) 
			self.nodeArray = self.nodeArray.reshape((self.nNodes,4))
			self.nElems = np.fromfile(file, dtype=np.uint32, count = 1)[0]
			self.elemArray = np.fromfile(file, dtype=np.uint32, count = 8*self.nElems) 
			self.elemArray = self.elemArray.reshape((self.nElems,8))
			self.elemPartIndex = np.fromfile(file, dtype=np.uint32, count = self.nElems)
			self.elemPseudoDensity = np.fromfile(file, dtype=np.double, count = self.nElems)
			self.nElemNeighors = np.fromfile(file, dtype=np.uint32, count = 1)[0]
			self.elemNeighbors = np.fromfile(file, dtype=np.int32, count = self.nElemNeighors* self.nElems)
			
			print("#Nodes = ", self.nNodes,"\n#Elems = ",self.nElems)
			print("#nElemNeighors = ", self.nElemNeighors)
			print(self.elemNeighbors)
			file.close()
			
	def plotMesh(self):
		vertices = np.zeros((self.nNodes,3))
		for i in range(3):
			vertices[:,i] = self.origin[i] + self.delta[i]*self.nodeArray[:,i]
			
		faces = np.ones((4,6*self.nElems))
		faceIndex = np.array([[0,4,7,3],[0,1,5,4],[0,3,2,1],[1,2,6,5],[2,3,7,6],[4,5,6,7]],
					   dtype = np.uint32);
		counter = 0
		nFacesPerHex = 6
		faceColors = np.zeros((6*self.nElems,4))
		for e in range(self.nElems):
			for j in range(nFacesPerHex):
				faces[:,counter] = self.elemArray[e,faceIndex[j,:]]
				faceColors[counter,:] = 1 - self.elemPseudoDensity[e]
				counter = counter+1
		faces = np.transpose(faces.astype(int))
		
		poly3d = [[vertices[vert_id] for vert_id in face] for face in faces]
		fig = plt.figure()
		ax = fig.add_subplot(111, projection='3d')
		ax.add_collection3d(Poly3DCollection(poly3d,
									   facecolors = faceColors,
									   edgecolors='k', alpha=0.6))
		ax.set_xlabel('X')
		ax.set_ylabel('Y')
		ax.set_zlabel('Z')
		ax.set_xlim(np.min(vertices[:,0]),np.max(vertices[:,0]));
		ax.set_ylim(np.min(vertices[:,1]),np.max(vertices[:,1]));
		ax.set_zlim(np.min(vertices[:,2]),np.max(vertices[:,2]));
		ax.set_aspect('equal')
		plt.show()

	
	def writeParetoMesh(self,fileName):
		# Write mesh in binary format
		print("To do")
	
	def runPareto(self,projecFile):
		os.system('"ParetoTopOptBatchMode" ' + projecFile) 
		#subprocess.run('"ParetoTopOptBatchMode" ' + projecFile)
#################################
if __name__ == '__main__':
	interface  = paretoInterface()
	#interface.runPareto("CantileverBeamProject.prj")
	interface.readParetoMesh("./CantileverBeam.msh")
	interface.elemPseudoDensity[0:2000] = 0.2 
	interface.elemPseudoDensity[2001:5000] = 0.7
	#interface.plotMesh()
	#interface.runPareto("CantileverBeamProject.prj")
	
	