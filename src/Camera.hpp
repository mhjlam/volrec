#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include "Voxel.hpp"

class Camera {
private:
	// current view index
	int viewIndex;

	// view width
	int viewWidth;

	// view height
	int viewHeight;

	// size of the view
	cv::Size viewSize;

	// calibration matrices
	cv::Mat intrinsicMatrix;
	cv::Mat distortionCoeffs;
	cv::Mat rotationVector;
	cv::Mat translationVector;
	cv::Mat rotationMatrix;

	// Rt matrix and its inverse
	cv::Mat Rt;
	cv::Mat inverseRt;

	// camera focal length and principal points
	float fx, fy, px, py;

	// 3D coordinates in world frame
	cv::Point3f cameraLocation;

private: // functions
	// initialization

	// load intrinsic and extrinsic matrices
	void loadCameraParams(std::string);

	// camera location
	void initCameraLocation(void);

	// inverted R|t matrix
	void initInvertedRt(void);

	// camera corners in world coordinate
	void initCameraWorldCoordinates(void);
	
	// 2D to/from 3D calculation
	cv::Point3f point2DtoWorld3D(cv::Point);
	cv::Point3f camera3DtoWorld3D(cv::Point3f);


public: // variables
	// camera
	std::vector<cv::Point3f> cameraPoints;

	// foreground image
	cv::Mat foreground;

public: // ctor, functions
	Camera(int, int, int, std::string);
	~Camera(void);
	
	// projects voxel to camera view
	cv::Point projectToView(cv::Point3f);
};
