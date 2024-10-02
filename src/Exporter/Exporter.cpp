/**
 * Computer Vision 2011/2012
 *
 * ASSIGNMENT 3 - INI WRITER
 *
 * AUTHORS
 * 		Maurits Lam
 * 		Marco van Laar
 *
 * DATE
 * 		2011-12-01
 *
 * NOTES
 * 		Written for OpenCV 2.3 (C++ code).
 *
 * 		Supplementary for Assignment 3; generates intrinsics/extrinsics and writes them to a file.
 */

#include <cstdio>
#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>

// draws world frame axes
void frame(cv::Mat& im, cv::Point2f& origin, cv::Point2f& xPoint, cv::Point2f& yPoint, cv::Point2f& zPoint) {
	line(im, origin, xPoint, CV_RGB(255, 0, 0), 3);	// x-axis (red)
	line(im, origin, yPoint, CV_RGB(0, 255, 0), 3);	// y-axis (green)
	line(im, origin, zPoint, CV_RGB(0, 0, 255), 3);	// z-axis (blue)
}

int main(int argc, char* argv[]) {
	unsigned numGoodFrames = 0;

	// checkerboard properties
	unsigned xCorners   = 8;					// number of internal corners horizontally
	unsigned yCorners   = 6;					// number of internal corners vertically
	unsigned numCorners = xCorners * yCorners;	// total number of internal corners
	unsigned squareSize = 22;					// square size in mm

	printf("Board corners: %i x %i\n", xCorners, yCorners);
	printf("Total number of corners: %i\n", numCorners);
	printf("------------------------\n");

	// lists for image and object points
	std::vector<std::vector<cv::Point2f>> imagePoints;
	std::vector<std::vector<cv::Point3f>> objectPoints;

	// sequence of detected corners in the frame
	std::vector<cv::Point2f> detectedCorners;

	// required for calibration further down
	cv::Size frameSize;

	std::vector<cv::Mat> previews;
	cv::namedWindow("Preview");

	while (true) {
		char file[16];
		sprintf(file, "bg%c.jpg", (49 + numGoodFrames));
		cv::Mat frame = cv::imread(file, cv::IMREAD_ANYCOLOR);

		if (!frame.data) {
			break;
		}

		printf("Reading from file %s... ", file);

		cv::Mat grayScaleFrame(frame.size(), CV_32FC1);
		frameSize = frame.size();
		numGoodFrames++;

		// attempt to find checkerboard corners
		bool found = findChessboardCorners(frame, cv::Size(xCorners, yCorners), detectedCorners);

		if (found) {
			// convert the currentFrame to a gray-scale image
			cvtColor(frame, grayScaleFrame, cv::COLOR_BGR2GRAY);

			// estimate subpixel accuracy on those corners
			cornerSubPix(grayScaleFrame, detectedCorners, cv::Size(11, 11), cv::Size(-1,-1), cv::TermCriteria((cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER), 30, 0.1));

			drawChessboardCorners(frame, cv::Size(xCorners, yCorners), detectedCorners, found);
			previews.push_back(frame);
		}

		// store board parameters if it is valid
		if (detectedCorners.size() == numCorners) {
			printf("Success!\n");

			std::vector<cv::Point3f> obj;

			for (unsigned i = 0; i < numCorners; ++i) {
				obj.push_back(cv::Point3f((i / xCorners) * (squareSize + 100), (i % xCorners) * (squareSize + 100), 0.0f));
			}

			imagePoints.push_back(detectedCorners);
			objectPoints.push_back(obj);
			detectedCorners.clear();
		}
		else {
			printf("Error detecting corners in %s\n", file);
		}
	}

	if (numGoodFrames < 3) {
		printf("Need at least 3 working views.\n");
		printf("Press any key to exit...");
		cv::waitKey(0);
		return 0;
	}

	cv::Mat intrinsic = cv::Mat(3, 3, CV_64FC1);	// intrinsic/camera matrix (K)
	intrinsic.at<float>(0, 1) = 1.f;				// set focal lengths
	intrinsic.at<float>(1, 1) = 1.f;				// with a ratio of 1.0

	cv::Mat distortion;								// distortion coefficients
	std::vector<cv::Mat> rotation;					// rotation vectors (R)
	std::vector<cv::Mat> translation;				// translation vectors (t)

	printf("------------------------\n");
	printf("Calibrating... Press any key to continue to the next image.\n");
	printf("------------------------\n");

	// calibrate the camera
	calibrateCamera(objectPoints, imagePoints, frameSize, intrinsic, distortion, rotation, translation);

	for (unsigned i = 0; i < numGoodFrames; ++i) {
		// define points for world frame
		std::vector<cv::Point3f> axes;
		axes.push_back(cv::Point3f(0.f, 0.f, 0.f));		// origin
		axes.push_back(cv::Point3f(2.f, 0.f, 0.f));		// x-axis
		axes.push_back(cv::Point3f(0.f, 2.f, 0.f));		// y-axis
		axes.push_back(cv::Point3f(0.f, 0.f, 2.f));		// z-axis

		std::vector<cv::Point2f> projectedAxes;
		projectPoints(axes, rotation[i], translation[i], intrinsic, distortion, projectedAxes);
		frame(previews[i], projectedAxes[0], projectedAxes[1], projectedAxes[2], projectedAxes[3]);
		imshow("Preview", previews[i]);
		cv::waitKey(0);
	}

	printf("Is this correct? If not, press 'n' or 'q' to abort.\n");
	char c = cv::waitKey(0);

	if (c != 'q' && c != 'n') {
		for (unsigned i = 0; i < numGoodFrames; ++i) {
			char output[16];
			sprintf(output, "cam%c.ini", (49 + i));
			printf("Exporting %s...", output);

			std::ofstream myfile;
			myfile.open(output);

			// intrinsic
			for (int row = 0; row < intrinsic.rows; ++row) {
				for (int col = 0; col < intrinsic.cols; ++col) {
					myfile << intrinsic.at<double>(row, col) << (col < intrinsic.cols -1 ? ' ' : '\n');
				}
			}

			// distortion
			for (int col = 0; col < distortion.cols - 1; ++col) {
				myfile << distortion.at<double>(0, col) << (col < distortion.cols -2 ? ' ' : '\n');
			}
			// rvec
			for (int row = 0; row < rotation[i].rows; ++row) {
				myfile << rotation[i].at<double>(row, 0) << (row < rotation[i].rows -1 ? ' ' : '\n');
			}

			// tvec
			for (int row = 0; row < translation[i].rows; ++row) {
				myfile << translation[i].at<double>(row, 0) << (row < translation[i].rows -1 ? ' ' : '\n');
			}

			myfile.close();
			printf("Success!\n");
		}

		printf("------------------------\n\n");
		printf("Success!\n");
		printf("Press any key to exit.\n");

		cv::waitKey(0);
	}

	return 0;
}

