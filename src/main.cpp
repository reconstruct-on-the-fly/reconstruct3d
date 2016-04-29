#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/calib3d/calib3d.hpp"
#include <iostream>
#include "camera.h"
#include "image_pair.h"

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
    if (argc != 3)
    {
        cout <<" Usage: display_image Image1 Image2" << endl;
        return -1;
    }

    Mat image1, image2;
    image1 = imread(argv[1], CV_LOAD_IMAGE_COLOR);
    image2 = imread(argv[2], CV_LOAD_IMAGE_COLOR);

    if (!image1.data || !image2.data)
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }
    Camera camera = Camera::createFromFile("src/input.txt");
    Mat R = Mat_<double>(3,3,.0);
    Mat Q;
    Vec<double, 3> T;
    T[0] = 0; T[1] = 0.1; T[2] = 0;

    ImagePair imagePair(image1, image2, R, T);
    ImagePair newImagePair = imagePair.rectify(camera, Q);

    /* Open Window */

    string window_title = "Display Window";

    namedWindow(window_title, WINDOW_NORMAL );
    imshow(window_title, depth_map);
    waitKey(0);

    return 0;
}
