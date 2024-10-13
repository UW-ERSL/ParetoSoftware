/*

                          3D Zernike Moments
    Copyright (C) 2003 by Computer Graphics Group, University of Bonn
           http://www.cg.cs.uni-bonn.de/project-pages/3dsearch/

Code by Marcin Novotni:     marcin@cs.uni-bonn.de

for more information, see the paper:

@inproceedings{novotni-2003-3d,
    author = {M. Novotni and R. Klein},
    title = {3{D} {Z}ernike Descriptors for Content Based Shape Retrieval},
    booktitle = {The 8th ACM Symposium on Solid Modeling and Applications},
    pages = {216--225},
    year = {2003},
    month = {June},
    institution = {Universit\"{a}t Bonn},
    conference = {The 8th ACM Symposium on Solid Modeling and Applications, June 16-20, Seattle, WA}
}
 *---------------------------------------------------------------------------*
 *                                                                           *
 *                                License                                    *
 *                                                                           *
 *  This library is free software; you can redistribute it and/or modify it  *
 *  under the terms of the GNU Library General Public License as published   *
 *  by the Free Software Foundation, version 2.                              *
 *                                                                           *
 *  This library is distributed in the hope that it will be useful, but      *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  Library General Public License for more details.                         *
 *                                                                           *
 *  You should have received a copy of the GNU Library General Public        *
 *  License along with this library; if not, write to the Free Software      *
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                *
 *                                                                           *
\*===========================================================================*/

/*
    This is a demonstration of how the classes may be used to generate the
    3D Zernike descriptors from a given input binary file containing the
    voxel grid representation of the object.

    Notice that in the below case, the file contains a cubic grid, i.e. the
    x-, y-, and z-dimensions are equal (such setting should satisfy most needs).
*/

#include <vector>
#include <complex>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string>

#include "ZernikeDescriptor.h"
#include "SurfVoxels.h"

using namespace std;

int main (int argc, char** argv)
{
    if (argc != 5)
    {
        std::cout << "Usage: " << argv[0] << " meshfilename voxelSize maxOrder outfile" << std::endl;
        return 0;
    }

    string meshfile = argv[1];
    int    voxDim   = atoi( argv[2] );
    int    maxOrder = atoi( argv[3] );
    string outfile  = argv[4];

    SurfVoxels surfVoxels;
    surfVoxels.readMesh( meshfile );
    surfVoxels.setDimension( voxDim );
    auto buf = surfVoxels.getDenseBuffer();

    int nsize = buf.size ();
    double f  = pow ((double)nsize, 1.0/3.0);
    int vdim  = floor (f+0.5);

    double* data = new double[nsize];
    for (int i=0; i< nsize; ++i)
    {
        data[i] = (double)buf[i];
    }

    // compute the zernike descriptors
    ZernikeDescriptor<double, float> zd (data, vdim, maxOrder);

    // save them into an .inv file
    zd.SaveInvariants(outfile.c_str());
}
