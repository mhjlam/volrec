#include "Camera.hpp"

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

Camera::Camera(int i, int w, int h, std::string iniFile) {
	viewIndex  = i;
	viewWidth  = w;
	viewHeight = h;

	loadCameraParams(iniFile);

	// get the camera parameters and calculate principal matrices and points
	fx = float(intrinsicMatrix.at<float>(0, 1));
	fy = float(intrinsicMatrix.at<float>(1, 1));
	px = float(intrinsicMatrix.at<float>(0, 2));
	py = float(intrinsicMatrix.at<float>(1, 2));

	initCameraLocation();
	initInvertedRt();
	initCameraWorldCoordinates();

	viewSize = cv::Size(viewWidth, viewHeight);
}

Camera::~Camera(void) {
	rotationVector.release();
	translationVector.release();
	intrinsicMatrix.release();
	distortionCoeffs.release();
	rotationMatrix.release();
}

// load calibration files
void Camera::loadCameraParams(std::string iniFile) {
	intrinsicMatrix = cv::Mat(3, 3, CV_32FC1);
	distortionCoeffs = cv::Mat(4, 1, CV_32FC1);
	rotationVector = cv::Mat(1, 3, CV_32FC1);
	translationVector = cv::Mat(1, 3, CV_32FC1);
	rotationMatrix = cv::Mat(3, 3, CV_32FC1);

	// open file
	std::ifstream file(iniFile);

	// load settings
	if (file.is_open()) {
		float value;
		
		for (int y = 0; y < 3; y++) {
			for (int x = 0; x < 3; x++) {
				file >> value;
				intrinsicMatrix.at<float>(y, x) = static_cast<float>(value);
			}
		}

		for (int x = 0; x < 4; x++) {
			file >> value;
			distortionCoeffs.at<float>(x, 0) = value;
		}
		for (int y = 0; y < 3; y++) {
			file >> value;
			rotationVector.at<float>(0, y) = value;
		}
		for (int y = 0; y < 3; y++) {
			file >> value;
			translationVector.at<float>(0, y) = value;
		}
	}
	file.close();
}

void Camera::initCameraLocation(void) {
	float rMatrix[9];
	float rVector[3];

	rVector[0] = rotationVector.at<float>(0, 0);
	rVector[1] = rotationVector.at<float>(0, 1);
	rVector[2] = rotationVector.at<float>(0, 2);

	cv::Mat rMatrixTemp = cv::Mat(3, 3, CV_32FC1, rMatrix);
	cv::Mat rVectorTemp = cv::Mat(1, 3, CV_32FC1, rVector);

	// compute rotation matrix by Rodrigues transform
	cv::Rodrigues(rVectorTemp, rMatrixTemp);

	// save rotation matrix
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			// TODO: optimize
			rotationMatrix.at<float>(i, j) = rMatrixTemp.at<float>(i, j);
		}
	}
	
	// Combine the two transformations into one matrix.
	// Order is important: rotations are not commutative.
	float tmat[4][4] = { 
		{ 1.f, 0.f, 0.f, 0.f },
		{ 0.f, 1.f, 0.f, 0.f },
		{ 0.f, 0.f, 1.f, 0.f },
		{ 
			translationVector.at<float>(0, 0) * -1.f,
			translationVector.at<float>(0, 1) * -1.f,
			translationVector.at<float>(0, 2) * -1.f,
			1.f 
		}
	};

	float rmat[4][4] = { 
		{ rMatrix[0], rMatrix[1], rMatrix[2], 0.f },
		{ rMatrix[3], rMatrix[4], rMatrix[5], 0.f },
		{ rMatrix[6], rMatrix[7], rMatrix[8], 0.f },
		{ 0.f, 0.f, 0.f, 1.f }
	};
	
	// matrix multiplcation
	float rm[4][4];
	for (int i = 0; i <= 3; i++) {
		for (int j = 0; j <= 3; j++) {
			rm[i][j] = 0.0;
			for (int k = 0; k <= 3; k++) {
				rm[i][j] += tmat[i][k] * rmat[k][j];
			}
		}
	}
	
	// save camera location
	cameraLocation = cv::Point3f(
		rm[0][0] + rm[3][0],
		rm[1][1] + rm[3][1],
		rm[2][2] + rm[3][2]
	);
	printf("Camera %i location: %f, %f, %f\n", viewIndex, cameraLocation.x, cameraLocation.y, cameraLocation.z);
}

void Camera::initInvertedRt() {
	// Xi =  K[R/t]Xw
	// t  = -RC

	Rt 		  = cv::Mat(4, 4, CV_32FC1);
	inverseRt = cv::Mat(4, 4, CV_32FC1);

	// [R|t]
	// TODO: optimize
	Rt.at<float>(0, 0) = rotationMatrix.at<float>(0, 0);
	Rt.at<float>(0, 1) = rotationMatrix.at<float>(0, 1);
	Rt.at<float>(0, 2) = rotationMatrix.at<float>(0, 2);
	Rt.at<float>(0, 3) = translationVector.at<float>(0, 0);

	Rt.at<float>(1, 0) = rotationMatrix.at<float>(1, 0);
	Rt.at<float>(1, 1) = rotationMatrix.at<float>(1, 1);
	Rt.at<float>(1, 2) = rotationMatrix.at<float>(1, 2);
	Rt.at<float>(1, 3) = translationVector.at<float>(0, 1);

	Rt.at<float>(2, 0) = rotationMatrix.at<float>(2, 0);
	Rt.at<float>(2, 1) = rotationMatrix.at<float>(2, 1);
	Rt.at<float>(2, 2) = rotationMatrix.at<float>(2, 2);
	Rt.at<float>(2, 3) = translationVector.at<float>(0, 2);

	Rt.at<float>(3, 0) = 0.0;
	Rt.at<float>(3, 1) = 0.0;
	Rt.at<float>(3, 2) = 0.0;
	Rt.at<float>(3, 3) = 1.0;

	inverseRt = Rt.inv();
}

cv::Point Camera::projectToView(cv::Point3f voxel) {
	cv::Point point;
	cv::Mat objectPoints = cv::Mat(3, 3, CV_32FC1);
	cv::Mat imagePoints  = cv::Mat(3, 2, CV_32FC1);

	objectPoints.at<float>(0, 0) = voxel.x;
	objectPoints.at<float>(0, 1) = voxel.y;
	objectPoints.at<float>(0, 2) = voxel.z;

	cv::projectPoints(objectPoints, rotationVector, translationVector, intrinsicMatrix, distortionCoeffs, imagePoints);

	point.x = imagePoints.at<int>(0, 0);
	point.y = imagePoints.at<int>(0, 1);

	objectPoints.release();
	imagePoints.release();

	return point;
}

void Camera::initCameraWorldCoordinates(void) {
	// camera projection center
	cameraPoints.push_back(cameraLocation);

	// image plane corners (in clockwise order):
	// 1. image plane's left upper corner
	cameraPoints.push_back(camera3DtoWorld3D(cv::Point3f(-px, -py, (fx + fy)/2)));
	
	// 2. image plane's right upper corner
	cameraPoints.push_back(camera3DtoWorld3D(cv::Point3f(viewWidth - px, -py, (fx + fy)/2)));
	
	// 3. image plane's right bottom corner
	cameraPoints.push_back(camera3DtoWorld3D(cv::Point3f(viewWidth - px, viewHeight - py, (fx + fy)/2)));
	
	// 4. image plane's left bottom corner
	cameraPoints.push_back(camera3DtoWorld3D(cv::Point3f(-px, viewHeight - py, (fx + fy)/2)));

	// principal point on the image plane
	cameraPoints.push_back(camera3DtoWorld3D(cv::Point3f(px, py, (fx + fy)/2)));
}

// Convert a 2D point on view to 3D world coordinates
cv::Point3f Camera::point2DtoWorld3D(cv::Point pt) {
	return camera3DtoWorld3D(cv::Point3f(float(pt.x - px), float(pt.y - py), (fx + fy)/2));
}

// Convert a 3D camera point to 3D world coordinates
cv::Point3f Camera::camera3DtoWorld3D(cv::Point3f cameraPoint3D) {
	cv::Mat Xc = cv::Mat(4, 1, CV_32FC1);
	Xc.at<float>(0, 0) = cameraPoint3D.x;
	Xc.at<float>(1, 0) = cameraPoint3D.y;
	Xc.at<float>(2, 0) = cameraPoint3D.z;
	Xc.at<float>(3, 0) = 1.f;
	
	//cv::Mat Xw = cv::Mat(4, 1, CV_32FC1);
	cv::Mat Xw = inverseRt * Xc;

	cv::Point3f pt3D = cv::Point3f(Xw.at<float>(0,0), Xw.at<float>(1,0), Xw.at<float>(2,0));

	Xc.release();
	Xw.release();

	return pt3D;
}
