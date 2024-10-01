#include <opencv2/opencv.hpp>

class Scene;
class VoxelGrid;

const int NUM_VIEWS   	= 4;		// number of images/views of the object
const int VIEW_WIDTH 	= 800;		// dimensions of the image/view
const int VIEW_HEIGHT 	= 600;

int ViewingWindow 		= 0;		// current viewing window
int MouseX 			    = 0;		// coordinates of the mouse,
int MouseY 			    = 0;		// required for mouse input

bool LeftMouseDown		= false;	// whether LMB is pressed
bool RightMouseDown		= false;	// whether RMB is pressed

cv::Mat* Foregrounds;			    // foreground images
cv::Mat* Backgrounds;			    // background images

Scene* Scene3D;					    // 3D scene
VoxelGrid* Voxels;	                // voxel representation


void ChangeViewingWindow(int window);
cv::Mat GetForeground(cv::Mat& frame, cv::Mat& background);
