#include "Scene.hpp"

#include <fstream>
#include <iostream>

#include <GL/freeglut.h>
#include <opencv2/opencv.hpp>

#include "Voxel.hpp"
#include "VoxelGrid.hpp"

const int Scene::GRID_TILE = 2;
const int Scene::GRID_SIZE = 400;
const int Scene::OFFSET_Z  = 5;

float Scene::eyeX = 0;
float Scene::eyeY = 0;
float Scene::eyeZ = 8000;

unsigned int Scene::width  = 644;
unsigned int Scene::height = 484;

float gl_centerX = 0.0f;
float gl_centerY = 0.0f;
float gl_centerZ = 0.0f;

Scene::Scene(int vN): numViews(vN), viewIndex(-1), viewAngle(0), CameraView(true), ShowVolume(true), ShowGridFloor(true), ShowCamera(true), ShowWorldFrame(true) {
	createFloor();
}

void Scene::createFloor() {
	// edges for floor
	std::vector<cv::Point3f> edge1, edge2, edge3, edge4;
	
	// edge 1
	for (int y = -GRID_SIZE*GRID_TILE; y <= GRID_SIZE*GRID_TILE; y += GRID_SIZE) {
		edge1.push_back(cv::Point3f(
			static_cast<float>(-GRID_SIZE*GRID_TILE),
			static_cast<float>(y), 
			static_cast<float>(OFFSET_Z)
		));
	}

	// edge 2
	for (int x = -GRID_SIZE*GRID_TILE; x <= GRID_SIZE*GRID_TILE; x += GRID_SIZE) {
		edge2.push_back(cv::Point3f(
			static_cast<float>(x),
			static_cast<float>(GRID_SIZE*GRID_TILE),
			static_cast<float>(OFFSET_Z)
		));
	}

	// edge 3
	for (int y = -GRID_SIZE*GRID_TILE; y <= GRID_SIZE*GRID_TILE; y += GRID_SIZE) {
		edge3.push_back(cv::Point3f(
			static_cast<float>(GRID_SIZE*GRID_TILE),
			static_cast<float>(y),
			static_cast<float>(OFFSET_Z)
		));
	}

	// edge 4
	for (int x = -GRID_SIZE*GRID_TILE; x <= GRID_SIZE*GRID_TILE; x += GRID_SIZE) {
		edge4.push_back(cv::Point3f(
			static_cast<float>(x), 
			static_cast<float>(-GRID_SIZE*GRID_TILE), 
			static_cast<float>(OFFSET_Z)
		));
	}

	floorEdges.push_back(edge1);
	floorEdges.push_back(edge2);
	floorEdges.push_back(edge3);
	floorEdges.push_back(edge4);
}


// Draw grid, GRID_SIZE*GRID_SIZE pixels/square four edges of the ground floor grid, with points
void Scene::drawFloor() {
	glLineWidth(1.0f);
	glPushMatrix();
	glBegin(GL_LINES);

	int gSize = (GRID_TILE * 2) + 1;
	
	for (int g = 0; g < gSize; g++) {
		// y lines
		glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
		glVertex3f(floorEdges[0][g].x, floorEdges[0][g].y, floorEdges[0][g].z);
		glVertex3f(floorEdges[2][g].x, floorEdges[2][g].y, floorEdges[2][g].z);
		
		// x lines
		glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
		glVertex3f(floorEdges[1][g].x, floorEdges[1][g].y, floorEdges[1][g].z);
		glVertex3f(floorEdges[3][g].x, floorEdges[3][g].y, floorEdges[3][g].z);
	}

	glEnd();
	glPopMatrix();
}

void Scene::drawVolume(VoxelGrid* grid) {
	glLineWidth(1.0f);
	glPushMatrix();
	glBegin(GL_LINES);

	// bottom
	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3i(grid->VolumeCorners[0].x, grid->VolumeCorners[0].y, grid->VolumeCorners[0].z);
	glVertex3i(grid->VolumeCorners[1].x, grid->VolumeCorners[1].y, grid->VolumeCorners[1].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3i(grid->VolumeCorners[1].x, grid->VolumeCorners[1].y, grid->VolumeCorners[1].z);
	glVertex3i(grid->VolumeCorners[2].x, grid->VolumeCorners[2].y, grid->VolumeCorners[2].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3i(grid->VolumeCorners[2].x, grid->VolumeCorners[2].y, grid->VolumeCorners[2].z);
	glVertex3i(grid->VolumeCorners[3].x, grid->VolumeCorners[3].y, grid->VolumeCorners[3].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3i(grid->VolumeCorners[3].x, grid->VolumeCorners[3].y, grid->VolumeCorners[3].z);
	glVertex3i(grid->VolumeCorners[0].x, grid->VolumeCorners[0].y, grid->VolumeCorners[0].z);

	// top
    glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
    glVertex3i(grid->VolumeCorners[4].x, grid->VolumeCorners[4].y, grid->VolumeCorners[4].z);
    glVertex3i(grid->VolumeCorners[5].x, grid->VolumeCorners[5].y, grid->VolumeCorners[5].z);

    glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
    glVertex3i(grid->VolumeCorners[5].x, grid->VolumeCorners[5].y, grid->VolumeCorners[5].z);
    glVertex3i(grid->VolumeCorners[6].x, grid->VolumeCorners[6].y, grid->VolumeCorners[6].z);

    glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
    glVertex3i(grid->VolumeCorners[6].x, grid->VolumeCorners[6].y, grid->VolumeCorners[6].z);
    glVertex3i(grid->VolumeCorners[7].x, grid->VolumeCorners[7].y, grid->VolumeCorners[7].z);

    glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
    glVertex3i(grid->VolumeCorners[7].x, grid->VolumeCorners[7].y, grid->VolumeCorners[7].z);
    glVertex3i(grid->VolumeCorners[4].x, grid->VolumeCorners[4].y, grid->VolumeCorners[4].z);

    // connection
	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3i(grid->VolumeCorners[0].x, grid->VolumeCorners[0].y, grid->VolumeCorners[0].z);
	glVertex3i(grid->VolumeCorners[4].x, grid->VolumeCorners[4].y, grid->VolumeCorners[4].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3i(grid->VolumeCorners[1].x, grid->VolumeCorners[1].y, grid->VolumeCorners[1].z);
	glVertex3i(grid->VolumeCorners[5].x, grid->VolumeCorners[5].y, grid->VolumeCorners[5].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3i(grid->VolumeCorners[2].x, grid->VolumeCorners[2].y, grid->VolumeCorners[2].z);
	glVertex3i(grid->VolumeCorners[6].x, grid->VolumeCorners[6].y, grid->VolumeCorners[6].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3i(grid->VolumeCorners[3].x, grid->VolumeCorners[3].y, grid->VolumeCorners[3].z);
	glVertex3i(grid->VolumeCorners[7].x, grid->VolumeCorners[7].y, grid->VolumeCorners[7].z);

	glEnd();
	glPopMatrix();
}

void Scene::drawVoxels(VoxelGrid* grid) {
	glPushMatrix();

    // apply default translation
	glTranslatef(0, 0, 0);
	glPointSize(2.0f);
	glBegin(GL_POINTS);

	int vSize = static_cast<int>(grid->VisibleVoxels.size());

	for (int v = 0; v < vSize; v++) {
		glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
		glVertex3i(grid->VisibleVoxels[v]->x, grid->VisibleVoxels[v]->y, grid->VisibleVoxels[v]->z);
	}

	glEnd();
	glPopMatrix();
}


void Scene::drawCameras() {
	glLineWidth(1.0f);
	glPushMatrix();
	glBegin(GL_LINES);

	for (int i = 0; i < numViews; i++) {
		// 0 - 1
		glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
		glVertex3f(CameraCoordinates[i][0].x, CameraCoordinates[i][0].y, CameraCoordinates[i][0].z);
		glVertex3f(CameraCoordinates[i][1].x, CameraCoordinates[i][1].y, CameraCoordinates[i][1].z);

		// 0 - 2
		glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
		glVertex3f(CameraCoordinates[i][0].x, CameraCoordinates[i][0].y, CameraCoordinates[i][0].z);
		glVertex3f(CameraCoordinates[i][2].x, CameraCoordinates[i][2].y, CameraCoordinates[i][2].z);

		// 0 - 3
		glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
		glVertex3f(CameraCoordinates[i][0].x, CameraCoordinates[i][0].y, CameraCoordinates[i][0].z);
		glVertex3f(CameraCoordinates[i][3].x, CameraCoordinates[i][3].y, CameraCoordinates[i][3].z);

		// 0 - 4
		glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
		glVertex3f(CameraCoordinates[i][0].x, CameraCoordinates[i][0].y, CameraCoordinates[i][0].z);
		glVertex3f(CameraCoordinates[i][4].x, CameraCoordinates[i][4].y, CameraCoordinates[i][4].z);

		// 1 - 2
		glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
		glVertex3f(CameraCoordinates[i][1].x, CameraCoordinates[i][1].y, CameraCoordinates[i][1].z);
		glVertex3f(CameraCoordinates[i][2].x, CameraCoordinates[i][2].y, CameraCoordinates[i][2].z);

		// 2 - 3
		glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
		glVertex3f(CameraCoordinates[i][2].x, CameraCoordinates[i][2].y, CameraCoordinates[i][2].z);
		glVertex3f(CameraCoordinates[i][3].x, CameraCoordinates[i][3].y, CameraCoordinates[i][3].z);

		// 3 - 4
		glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
		glVertex3f(CameraCoordinates[i][3].x, CameraCoordinates[i][3].y, CameraCoordinates[i][3].z);
		glVertex3f(CameraCoordinates[i][4].x, CameraCoordinates[i][4].y, CameraCoordinates[i][4].z);

		// 4 - 1
		glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
		glVertex3f(CameraCoordinates[i][4].x, CameraCoordinates[i][4].y, CameraCoordinates[i][4].z);
		glVertex3f(CameraCoordinates[i][1].x, CameraCoordinates[i][1].y, CameraCoordinates[i][1].z);
	}

	glEnd();
	glPopMatrix();
}


void Scene::drawWorldFrame() {
	glLineWidth(1.5f);
	glPushMatrix();
	glBegin(GL_LINES);

	// draw x-axis
	glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
	
	glVertex3f(  0.0f, 0.0f, 0.0f);
	glVertex3f(500.0f, 0.0f, 0.0f);

	// draw y-axis
	glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
	
	glVertex3f(0.0f,   0.0f, 0.0f);
	glVertex3f(0.0f, 500.0f, 0.0f);
		
    // draw z-axis
	glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
	
	glVertex3f(0.0f, 0.0f,   0.0f);
	glVertex3f(0.0f, 0.0f, 500.0f);

	glEnd();
	glPopMatrix();
}


void Scene::Render(VoxelGrid* grid) {
	glEnable(GL_DEPTH_TEST);

	// Clears the screen to black, clears the color and depth buffers, and resets the modelview matrix.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // reset modelview matrix

	if (!CameraView) {
		gluLookAt(eyeX,eyeY,eyeZ, gl_centerX,gl_centerY,gl_centerZ, -1,0,0);
	}
    else {
		gluLookAt(eyeX,eyeY,eyeZ, gl_centerX,gl_centerY,gl_centerZ,  0,0,1);
	}

	// rotate view
	glRotatef(viewAngle, 0.0f, 0.0f, 1.0f);
	glPopMatrix();
	
    if (ShowCamera) {
		drawCameras();
	}
	
	if (ShowGridFloor) {
		drawFloor();
	}
	
	if (ShowVolume) {
		drawVolume(grid);
	}

	drawVoxels(grid);

	if (ShowWorldFrame) {
		drawWorldFrame();
	}

	glFlush();
    glutSwapBuffers();
}

void Scene::Resize(int width, int height) {
    // Reset the viewport to new dimensions
    glViewport(0, 0, width, height);

    // Set current Matrix to projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); //reset projection matrix

    // Calculates aspect ratio of the window.
    gluPerspective(54.0f, width/height, 1.0f, 20000.0f);

    glMatrixMode(GL_MODELVIEW); // set modelview matrix
    glLoadIdentity(); 			// reset modelview matrix

    gluLookAt(eyeX, eyeY, eyeZ, gl_centerX, gl_centerY, gl_centerZ, 0, 1, 0);
}

void Scene::SwitchView() {
	ResetView();
	
	CameraView = !CameraView;
	
	if (!CameraView) {
		// top view
		eyeX = 0;
		eyeY = 0;
		eyeZ = 8000;
	}
}

void Scene::UpdateView(int view) {
	if (!CameraView) {
		return;
	}
	
	if (viewIndex != view) {
		viewIndex = view;
		
		eyeX = CameraCoordinates[viewIndex][0].x;
		eyeY = CameraCoordinates[viewIndex][0].y;
		eyeZ = CameraCoordinates[viewIndex][0].z;
	}
}

void Scene::ResetView() {
	viewAngle =  0;
	viewIndex = -1;
}


void Scene::RotateView(float angle = 1.f) {
	viewAngle += angle;
}

void Scene::RotateScene(int x, int y) {
	if (!CameraView) {
		return;
	}
	
	if (x == 0 && y == 0) {
		return;
	}	
	
	// see http://en.wikipedia.org/wiki/Spherical_coordinates#Cartesian_coordinates
	
    float r 	= sqrt(eyeX*eyeX + eyeY*eyeY + eyeZ*eyeZ);	// distance from origin
    float phi   = atan2(float(eyeY), float(eyeX));			// arctan2(y/x);
    float theta = acos(eyeZ/r);								// arccos(z/r)
	
	float multiplier = (r > 1000) ? 2.f : 1.f;				// adjust for long-distance views
	
	// angles of rotation in radians
	phi   -= float(x) / (100.f * multiplier);
	theta -= float(y) / (100.f * multiplier);
	
	// spherical coordinates
    eyeX = r * cos(phi) * sin(theta);
    eyeY = r * sin(phi) * sin(theta);
    eyeZ = r * cos(theta);
}

void Scene::ZoomScene(int zDelta) {
    float x = gl_centerX - eyeX;
    float y = gl_centerY - eyeY;
    float z = gl_centerZ - eyeZ;
    float n = std::sqrtf(std::powf(x, 2) + std::powf(y, 2) + std::powf(z, 2));

    x /= n; 
	y /= n; 
	z /= n;
    
	x *= 100; 
	y *= 100; 
	z *= 100;
    
	if (zDelta > 0) {
        eyeX += x;
        eyeY += y;
        eyeZ += z;
    }
    else if (zDelta < 0) {
        eyeX -= x;
        eyeY -= y;
        eyeZ -= z;
    }
}
