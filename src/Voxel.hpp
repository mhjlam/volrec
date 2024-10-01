#pragma once

class Voxel {
public:
	// position
	int x, y, z;

	// color
	float r, g, b;

	// number of views
	int numViews;

	// number of views voxel is visible from
	int numVisible;
};
