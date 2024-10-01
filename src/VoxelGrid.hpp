#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

class Voxel;
class Camera;
class LookUpTable;

class VoxelGrid {
public:
	VoxelGrid(int,int, int, Camera**);
	~VoxelGrid(void) = default;

private:
	static const int VOXEL_SIZE;
	static const int GRID_TILE;
	static const int GRID_SIZE;

	int numViews;					// number of views
	int viewWidth;   				// view width
	int viewHeight;   				// view height
	int numVoxels; 					// total number of voxels in the acquisition space

	Voxel* voxelList;				// list of all voxels
	LookUpTable** lut;				// look-up table

	void initVoxelList(Camera**);// initialization of the voxel list

public:
	cv::Point3i* VolumeCorners; 	// 8 corners of the acquision space
	std::vector<Voxel*> VisibleVoxels; 	// visible voxels
	
	void Update(Camera**);	// update voxel locations (runtime)
};
