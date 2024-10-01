#include "VoxelGrid.hpp"

#include <cstdio>
#include <opencv2/opencv.hpp>

#include "Voxel.hpp"
#include "Camera.hpp"
#include "LookupTable.hpp"

const int VoxelGrid::VOXEL_SIZE = 20;
const int VoxelGrid::GRID_TILE  = 2;
const int VoxelGrid::GRID_SIZE  = 400;

VoxelGrid::VoxelGrid(int views, int w, int h, Camera** camera): numViews(views), viewWidth(w), viewHeight(h) {
    initVoxelList(camera);
}

void VoxelGrid::initVoxelList(Camera** camera) {
    // look-up table for each view
    lut = new LookUpTable*[numViews];
    for (int i = 0; i < numViews; i++) {
        lut[i] = new LookUpTable(viewWidth, viewHeight);
    }

	// 8 corners of the acquisition space
    VolumeCorners = new cv::Point3i[8];

    int halfEdge = GRID_SIZE * GRID_TILE;
    int edge = halfEdge * 2;

	int xL =  -halfEdge;
    int xR =  halfEdge;
    int yL =  -halfEdge;
    int yR =  halfEdge;
    int zL =  0;
    int zR =  halfEdge;

	// bottom volume corners
    VolumeCorners[0] = cv::Point3i(xL, yL, zL);
    VolumeCorners[1] = cv::Point3i(xL, yR, zL);
    VolumeCorners[2] = cv::Point3i(xR, yR, zL);
    VolumeCorners[3] = cv::Point3i(xR, yL, zL);

	// top volume corners
    VolumeCorners[4] = cv::Point3i(xL, yL, zR);
    VolumeCorners[5] = cv::Point3i(xL, yR, zR);
    VolumeCorners[6] = cv::Point3i(xR, yR, zR);
    VolumeCorners[7] = cv::Point3i(xR, yL, zR);

    numVoxels = (edge/VOXEL_SIZE) * (edge/VOXEL_SIZE) * (halfEdge/VOXEL_SIZE);

    // the whole voxel list
    voxelList = new Voxel[numVoxels];

    int remaining = 10;

	printf("Number of voxels: %d\n", numVoxels);
	printf("Initializing look-up table\t");

	// voxel index
    int v = 0;

	// iterate over all voxels
    for (int x = xL; x < xR; x += VOXEL_SIZE) {
        for (int y = yL; y < yR; y += VOXEL_SIZE) {
            for (int z = zL; z < zR; z += VOXEL_SIZE) {
                // initialize the voxels
                voxelList[v].x = x;
				voxelList[v].y = y;
				voxelList[v].z = z;

                voxelList[v].numVisible = 0;
				voxelList[v].numViews = numViews;

                // project 3D voxel to each 2D view and attach to the corresponding pixel's LUT
                for (int i = 0; i < numViews; i++) {
                    cv::Point pt = camera[i]->projectToView(cv::Point3f(
                        static_cast<float>(x), 
                        static_cast<float>(y), 
                        static_cast<float>(z)
                    ));

					// if the voxel is visible in current view, save its index to the LUT of the pixel it projects on
                    if ((pt.x >= 0) && (pt.x < viewWidth) && (pt.y >= 0) && (pt.y < viewHeight)) {
                        lut[i]->PixelLUT[pt.x][pt.y].push_back(v);
                    }
                }

                v++;

                if (int(numVoxels/v) == remaining) {
					printf("|");
                    remaining--;
                }
            }
        }
    }

	printf("\nVoxel list initialized!\n");
}

void VoxelGrid::Update(Camera** camera) {
    for (int v = 0; v < numVoxels; v++) {
        if (voxelList[v].numVisible > 0) {
            voxelList[v].numVisible = 0;
        }
    }

    VisibleVoxels.clear();

    // update the voxel list
    for (int i = 0; i < numViews; i++) {
        for (int c = 0; c < viewWidth; c++) { // column
            for (int r = 0; r < viewHeight; r++) { // row
                // if it is a foreground pixel
                //if (cvGet2D(camera[i]->foreground, y, x).val[0] == 255) {
                if (camera[i]->foreground.at<uint8_t>(r, c) == 255) {
                    // if some voxel is visible at this pixel
                    if (int(lut[i]->PixelLUT[c][r].size()) > 0) {
                        int size = static_cast<int>(lut[i]->PixelLUT[c][r].size());

                        for (int t = 0; t < size; t++) {
                            // visible counter plus one
                            int v = lut[i]->PixelLUT[c][r][t];

                            if (v < numVoxels) {
                                voxelList[v].numVisible++;

                                // if the voxel is visible in all views, mark it as a visible voxel
                                if (voxelList[v].numVisible == numViews) {
                                    VisibleVoxels.push_back(&voxelList[v]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
