/**
 * 	Original code written by Xinghan Luo.
 *
 * 	Refactorings of particular note are the changes to the voxel grid size,
 * 	fixing the camera, and adding zoom functionality.
 *
 * 	The required input for this code are still images:
 * 		Four original images named bg[1-4].jpg
 * 		Four images for background subtraction named cam[1-4].png
 * 		Four ini files that contain the values for each camera position named cam[1-4].ini
 * 			In these files, row 1-3 represent the 3x3 intrinsic matrix,
 * 							row 4   are the 1x4 distortion coefficients (may be [0 0 0 0]),
 * 							row 5   is the 1x3 rotation vector,
 * 							row 6   is the 1x3 translation vector.
 *
 * 	KEYBOARD CONTROL:
 * 		T		switch between camera view and top-view
 * 		R		hold to rotate scene counter-clockwise
 * 		V		show/hide scene volume
 * 		G		show/hide scene floor
 * 		C		show/hide camera wireframes
 * 		O		show/hide world frame / origin
 * 		1-4		switch between viewing windows
 *
 * 	MOUSE CONTROL:
 * 		LMB		hold and drag to rotate around origin
 * 		RMB		hold and drag forwards/downwards to zoom in/out
 */

#include "Main.hpp"

#include <vector>
#include <format>
#include <cstdlib>
#include <fstream>
#include <filesystem>

#include <GL/freeglut.h>
#include <opencv2/opencv.hpp>

#include "Scene.hpp"
#include "Camera.hpp"
#include "VoxelGrid.hpp"


void quit() {
	// clean up global objects
	delete 	 Scene3D;
	delete 	 Voxels;
	delete[] Backgrounds;
	delete[] Foregrounds;

	cv::destroyAllWindows();
    std::exit(0);
}

void keyboard(unsigned char key, int x, int y) {
	CV_UNUSED(x);
	CV_UNUSED(y);

    switch (key) {
		// switch view modes
		case 't':
			Scene3D->SwitchView();
			break;

		// rotate
        case 'r':
			Scene3D->RotateView(2.f);
			break;

		// scene volume
		case 'v':
			Scene3D->ShowVolume = !Scene3D->ShowVolume;
			break;

		// grid floor
        case 'g':
			Scene3D->ShowGridFloor = !Scene3D->ShowGridFloor;
			break;

		// cameras
        case 'c':
			Scene3D->ShowCamera = !Scene3D->ShowCamera;
			break;

		// origin
        case 'o':
			Scene3D->ShowWorldFrame = !Scene3D->ShowWorldFrame;
			break;

        case '1':
			ChangeViewingWindow(0);
			break;

        case '2':
			ChangeViewingWindow(1);
			break;

        case '3':
			ChangeViewingWindow(2);
			break;

		case '4':
			ChangeViewingWindow(3);
			break;

		// quit
		case 'q':
		case  27:
			quit();
			break;

		default:
			break;
    }
}

void keyboard2(int key, int x, int y) {
	CV_UNUSED(x);
	CV_UNUSED(y);

	switch (key) {
		case GLUT_KEY_LEFT:
			Scene3D->RotateScene(-10, 0);
			break;

		case GLUT_KEY_RIGHT:
			Scene3D->RotateScene(10, 0);
			break;

		// rotate when in camera view; zoom when in top view
		case GLUT_KEY_UP:
			(Scene3D->CameraView) ? Scene3D->RotateScene(0, 10) : Scene3D->ZoomScene(10);
			break;

		case GLUT_KEY_DOWN:
			(Scene3D->CameraView) ? Scene3D->RotateScene(0, -10) : Scene3D->ZoomScene(-10);
			break;
	}
}

void mouse(int button, int state, int x, int y) {
	CV_UNUSED(x);
	CV_UNUSED(y);

	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		LeftMouseDown = true;
	} 
	else {
		LeftMouseDown = false;
	}

	if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) {
		RightMouseDown = true;
	} 
	else {
		RightMouseDown = false;
	}
}

void motion(int x, int y) {
	if (LeftMouseDown) {
		Scene3D->RotateScene(x - MouseX, y - MouseY);
	}

	if (RightMouseDown) {
		Scene3D->ZoomScene(-(y - MouseY)/2);
	}

    MouseX = x;
    MouseY = y;
}

void passive_motion(int x, int y) {
	MouseX = x;
	MouseY = y;
}

void reshape(int width, int height) {
    Scene3D->Resize(width, height);
}

void update(int value = 0) {
	CV_UNUSED(value);

	cv::imshow("Original image",   Backgrounds[ViewingWindow]);
    cv::imshow("Foreground image", Foregrounds[ViewingWindow]);

	// update camera view
	Scene3D->UpdateView(ViewingWindow);

	// call glut key handler
    keyboard(static_cast<unsigned char>(cv::waitKey(2)), 0, 0);

    glutSwapBuffers();
    glutTimerFunc(18, update, 0);
    glutPostRedisplay();
}

void display() {
    Scene3D->Render(Voxels);
}

void idle() {
    glutPostRedisplay();
}

void initialize_glut(int argc, char **argv) {
    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(700, 10);
    glutCreateWindow("Voxel Representation");

	///////////////////////////////////////////////////////////
	glEnable(GL_DEPTH_TEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 1, 1, 20000);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
	///////////////////////////////////////////////////////////

    glutTimerFunc(8, update, 0);
	glutIdleFunc(idle);
	glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard2);
	glutMouseFunc(mouse);
    glutMotionFunc(motion);
	glutPassiveMotionFunc(passive_motion);
    glutReshapeFunc(reshape);
}

void ChangeViewingWindow(int window) {
	Scene3D->ResetView();
	ViewingWindow = window;
}

cv::Mat GetForeground(cv::Mat& fg_bgr, cv::Mat& bg_hsv) {
	// DEFINE TEMPORARY PARAMETERS
	// size of frame (foreground)
	cv::Size size = fg_bgr.size();

	// background image
	cv::Mat fg_out = cv::Mat(size, CV_8U, 1);

	// temporary result image
	cv::Mat fg_tmp = cv::Mat(size, CV_8U, 1);

	// temporary result of the background
	cv::Mat bg_tmp = cv::Mat(size, CV_8U, 1);

	// COLOR TRANSFORMATION FROM RGB TO HSV FOR THE BACKGROUND MODEL
	// split HSV-channels
	std::vector<cv::Mat> bg_channels;
	cv::split(bg_hsv, bg_channels);

	// COLOR TRANSFORMATION FROM RGB TO HSV FOR THE CURRENT FRAME
	cv::Mat fg_hsv = fg_bgr.clone();
	cv::cvtColor(fg_bgr, fg_hsv, cv::COLOR_BGR2HSV);

	// Split HSV-channels
	std::vector<cv::Mat> fg_channels;
	cv::split(fg_hsv, fg_channels);

	// Definition of the thresholds
	int threshold_h = 20;
	int threshold_s = 20;
	int threshold_v = 40;
	double maxval = 255.0;

	// BACKGROUND SUBTRACTION

	// INITIALIZE BACKGROUND BY H-CHANNEL
	cv::absdiff(fg_channels[0], bg_channels[0], fg_tmp);
	cv::threshold(fg_tmp, fg_out, threshold_h, maxval, cv::THRESH_BINARY);

	// UPDATE BACKGROUND BY S-CHANNEL
	cv::absdiff(fg_channels[1], bg_channels[1], fg_tmp);
	cv::threshold(fg_tmp, bg_tmp, threshold_s, maxval, cv::THRESH_BINARY);
	cv::bitwise_and(fg_out, bg_tmp, fg_out);

	// UPDATE BACKGROUND BY V-CHANNEL
	cv::absdiff(fg_channels[2], bg_channels[2], fg_tmp);
	cv::threshold(fg_tmp, bg_tmp, threshold_v, maxval, cv::THRESH_BINARY);
	cv::bitwise_or(fg_out, bg_tmp, fg_out);

	// Erosion kernel
	cv::Mat element_d = cv::getStructuringElement(cv::MorphShapes::MORPH_CROSS, cv::Size2f(5, 5), cv::Point(3, 3)); // CV_SHAPE_CROSS CV_SHAPE_RECT CV_SHAPE_ELLIPSE

	// Erosion & dilation to connect blobs and remove noisy points
	cv::erode(fg_out, fg_out, 0);
	cv::dilate(fg_out, fg_out, element_d, cv::Point(-1,-1), 2);
	cv::erode(fg_out, fg_out, 0);

	return fg_out;
}

inline bool check_required_files(std::filesystem::path folderPath) {
	for (int i = 0; i < NUM_VIEWS; ++i) {
		std::string camIni = std::format("cam{}.ini", i+1);
		if (!std::filesystem::exists(folderPath / camIni)) {
			return false;
		}
		std::string camPng = std::format("cam{}.png", i+1);
		if (!std::filesystem::exists(folderPath / camPng)) {
			return false;
		}
		std::string bgJpg = std::format("bg{}.jpg", i+1);
		if (!std::filesystem::exists(folderPath / bgJpg)) {
			return false;
		}
	}
	return true;
}

int main(int argc, char* argv[]) {
	// initialize objects
	Scene3D = new Scene(NUM_VIEWS);
	Backgrounds = new cv::Mat[NUM_VIEWS];
    Foregrounds	= new cv::Mat[NUM_VIEWS];

	Camera** cameras = new Camera*[NUM_VIEWS];
	cv::Mat* frames = new cv::Mat[NUM_VIEWS];
    cv::Mat* bgsHSV = new cv::Mat[NUM_VIEWS];

	if (argc < 2) {
		printf("Usage: VolRec <folder>\n\tTarget folder should contain the following files:\n\t\t\"cam[0..4].ini\"\n\t\t\"cam[0..4].png\"\n\t\t\"bg[0..4].jpg\"");
		return 1;
	}

	std::filesystem::path folderPath = std::filesystem::absolute(std::filesystem::current_path() / argv[1]);

	if (!check_required_files(folderPath)) {
		printf("Usage: VolRec <folder>\n\tTarget folder should contain the following files:\n\t\t\"cam[0..4].ini\"\n\t\t\"cam[0..4].png\"\n\t\t\"bg[0..4].jpg\"");
		return 1;
	}

	std::cout << "Loading data..." << std::endl;

	// load data
	for (int i = 0; i < NUM_VIEWS; ++i) {
		// cam config
		std::string camConfigFile = std::format("cam{}.ini", i+1);
		std::filesystem::path camConfigFilePath = folderPath / camConfigFile;
		printf("Loading configuration for camera %i: %s\n", i + 1, camConfigFile.c_str());

		// test if file exists
		std::ifstream file(camConfigFilePath.string());
		if (!file.is_open()) {
			printf("Unable to load camera ini file %s!", camConfigFile.c_str());
			return 1;
		}
		cameras[i] = new Camera(i + 1, VIEW_WIDTH, VIEW_HEIGHT, camConfigFilePath.string());

		// background image
		std::string backgroundFile = std::format("bg{}.jpg", i+1);
		std::filesystem::path backgroundFilePath = folderPath / backgroundFile;
		printf("Loading background image %i: %s\n", i + 1, backgroundFile.c_str());
		
		Backgrounds[i] = cv::imread(backgroundFilePath.string(), 1);
		if (Backgrounds[i].empty()) {
			printf("Unable to load background image %s!", backgroundFile.c_str());
			return 1;
		}

		// frames
		std::string foregroundFile = std::format("cam{}.png", i+1);
		std::filesystem::path foregroundFilePath = folderPath / foregroundFile;
		printf("Loading frame image %i: %s\n\n", i + 1, foregroundFile.c_str());

		frames[i] = cv::imread(foregroundFilePath.string(), 1);
		if (frames[i].empty()) {
			printf("Unable to load frame %s!", foregroundFile.c_str());
			return 1;
		}

		std::cout << "Background subtraction..." << std::endl;

		try {
			// background in HSV (Hue-Saturation-Value)
			cv::cvtColor(Backgrounds[i], bgsHSV[i], cv::COLOR_BGR2HSV);

			// safety test
			if (bgsHSV[i].empty()) {
				printf("Could not create HSV of background %d!", i);
				return 1;
			}

			 // get frame from videos
			Foregrounds[i] = GetForeground(frames[i], bgsHSV[i]);
			cameras[i]->foreground = Foregrounds[i];

			// transfer camera coordinate to 3D scene
			Scene3D->CameraCoordinates.push_back(cameras[i]->cameraPoints);
		}
		catch (...) {
			printf("Error occurred during background subtraction!");
			return 1;
		}
	}

	std::cout << "Volumetric reconstruction..." << std::endl;

	// volumetric reconstruction
	Voxels = new VoxelGrid(NUM_VIEWS, VIEW_WIDTH, VIEW_HEIGHT, cameras);
	Voxels->Update(cameras);

	// Initialize windows
	initialize_glut(argc, argv);

	cv::namedWindow("Original image", 1);
	cv::namedWindow("Foreground image", 1);

	// handle events
	glutMainLoop();

	// cleanup local objects
	delete[] cameras;
	delete[] frames;
	delete[] bgsHSV;

	return 0;
}
