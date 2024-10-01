#pragma once

#include <vector>
#include <string>

#include <opencv2/opencv.hpp>

class VoxelGrid;

class Scene {
private: // variables
	static const int GRID_TILE;
	static const int GRID_SIZE;
	static const int OFFSET_Z;

	// camera view point
	static float eyeX;
	static float eyeY;
	static float eyeZ;
	
	// view dimensions
	static unsigned int width;
	static unsigned int height;
	
	// edge points of the virtual ground floor grid
	std::vector<std::vector<cv::Point3f>> floorEdges;

	// view properties
	int numViews;					// total number of views
	int viewIndex;					// current view
	float viewAngle;				// rotation angle
	
private: // functions
	// functions to display the ground floor grid and volume
	void createFloor();				// create floor
	void drawFloor();				// draw floor tiles
	void drawVolume(VoxelGrid*);	// scene volume
	void drawWorldFrame();			// draw world coordinate frame

	// functions to render the scene
	void drawCameras();				// draws wireframes of the cameras
	void drawVoxels(VoxelGrid*);	// draws the visible voxels
	
public: // variables
	// N camera and corner coordinates 
	std::vector<std::vector<cv::Point3f>> CameraCoordinates;

	// switches
	bool CameraView;				// camera view or top view
	bool ShowVolume;				// box around scene
	bool ShowGridFloor;				// floor of scene
	bool ShowCamera;				// camera locations wrt scene
	bool ShowWorldFrame;			// world frame axes
	
public: // functions
	Scene(int);
	~Scene() = default;

	void Render(VoxelGrid*);		// renders the scene
	void Resize(int, int);			// updates the window after resizing

	void SwitchView();				// enables switching between camera-view and top-view
    void UpdateView(int);			// updates the camera view
	void ResetView();				// resets the view to default values
	void RotateView(float);			// rotates around origin counter-clockwise
	void RotateScene(int x, int y);	// rotates the scene based on mouse input
    void ZoomScene(int);			// zooms in on the scene
};
