/**
 * EXPORTER
 * 
 * Generate intrinsics & extrinsics and write to configuration file.
 */

#include <format>
#include <fstream>
#include <iostream>

#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
	cv::uint32_t numGoodFrames = 0;

	// checkerboard properties
	cv::uint32_t xCorners   = 8;					// number of internal corners horizontally
	cv::uint32_t yCorners   = 6;					// number of internal corners vertically
	cv::uint32_t numCorners = xCorners * yCorners;	// total number of internal corners
	cv::uint32_t squareSize = 22;					// square size in mm

	std::cout << "Board corners: " << xCorners << " x " << yCorners << std::endl;
	std::cout << "Total number of corners: " << numCorners << std::endl << std::endl;

	// lists for image and object points
	std::vector<std::vector<cv::Point2f>> imagePoints;
	std::vector<std::vector<cv::Point3f>> objectPoints;

	// sequence of detected corners in the frame
	std::vector<cv::Point2f> detectedCorners;

	// required for calibration
	cv::Size frameSize;

	std::vector<cv::Mat> previews;
	cv::namedWindow("Preview");

	while (true) {
		std::string file = std::format("bg{}.jpg", numGoodFrames);
		cv::Mat frame = cv::imread(file, cv::IMREAD_ANYCOLOR);

		if (!frame.data) {
			break;
		}

		std::cout << "Reading from file " << file << std::endl;

		frameSize = frame.size();
		cv::Mat grayScaleFrame(frameSize, CV_32FC1);
		numGoodFrames++;

		// attempt to find checkerboard corners
		bool found = cv::findChessboardCorners(frame, cv::Size(xCorners, yCorners), detectedCorners);

		if (found) {
			// convert the currentFrame to a gray-scale image
			cv::cvtColor(frame, grayScaleFrame, cv::COLOR_BGR2GRAY);

			// estimate subpixel accuracy on those corners
			cv::cornerSubPix(grayScaleFrame, detectedCorners, cv::Size(11, 11), cv::Size(-1,-1), cv::TermCriteria((cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER), 30, 0.1));

			cv::drawChessboardCorners(frame, cv::Size(xCorners, yCorners), detectedCorners, found);
			previews.push_back(frame);
		}

		// store board parameters if it is valid
		if (detectedCorners.size() == numCorners) {
			std::cout << "Success!" << std::endl;

			std::vector<cv::Point3f> objectPoint;
			for (cv::uint32_t i = 0; i < numCorners; ++i) {
				objectPoint.push_back(cv::Point3i((i / xCorners) * (squareSize + 100), (i % xCorners) * (squareSize + 100), 0));
			}

			imagePoints.push_back(detectedCorners);
			objectPoints.push_back(objectPoint);
			detectedCorners.clear();
		}
		else {
			std::cout << "Error detecting corners in " << file << std::endl;
		}
	}

	if (numGoodFrames < 3) {
		std::cout << "Need at least 3 working views." << std::endl;
		std::cout << "Press any key to exit..." << std::endl;
		cv::waitKey(0);
		return 0;
	}

	cv::Mat intrinsic = cv::Mat(3, 3, CV_64FC1);	// intrinsic/camera matrix (K)
	intrinsic.at<float>(0, 1) = 1.f;				// set focal lengths
	intrinsic.at<float>(1, 1) = 1.f;				// with a ratio of 1.0

	cv::Mat distortion;								// distortion coefficients
	std::vector<cv::Mat> rotation;					// rotation vectors (R)
	std::vector<cv::Mat> translation;				// translation vectors (t)

	std::cout << "Calibrating... Press any key to continue to the next image." << std::endl;

	// calibrate the camera
	cv::calibrateCamera(objectPoints, imagePoints, frameSize, intrinsic, distortion, rotation, translation);

	for (unsigned i = 0; i < numGoodFrames; ++i) {
		// define points for world frame
		std::vector<cv::Point3f> axes;
		axes.push_back(cv::Point3f(0.f, 0.f, 0.f));		// origin
		axes.push_back(cv::Point3f(2.f, 0.f, 0.f));		// x-axis
		axes.push_back(cv::Point3f(0.f, 2.f, 0.f));		// y-axis
		axes.push_back(cv::Point3f(0.f, 0.f, 2.f));		// z-axis

		std::vector<cv::Point2f> projectedAxes;
		cv::projectPoints(axes, rotation[i], translation[i], intrinsic, distortion, projectedAxes);

		// Draw axes
		cv::line(previews[i], projectedAxes[0], projectedAxes[1], CV_RGB(255, 0, 0), 3); // x
		cv::line(previews[i], projectedAxes[0], projectedAxes[2], CV_RGB(0, 255, 0), 3); // y
		cv::line(previews[i], projectedAxes[0], projectedAxes[3], CV_RGB(0, 0, 255), 3); // z

		cv::imshow("Preview", previews[i]);
		cv::waitKey(0);
	}

	std::cout << "Is this correct (Y/N)?" << std::endl;
	char c = cv::waitKey(0);

	if ((c == 'Y' || c == 'y')) {
		for (unsigned i = 0; i < numGoodFrames; ++i) {
			std::string output = std::format("cam{}.ini", i);
			std::cout << "Exporting " << output << std::endl;

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
			std::cout << "Success." << std::endl;
		}

		std::cout << "Finished." << std::endl;
	} else {
		std::cout << "Aborted." << std::endl;
	}

	return 0;
}
