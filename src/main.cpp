#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/calib3d/calib3d.hpp"
#include <iostream>

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

    StereoSGBM sgbm;
    sgbm.SADWindowSize = 7;
    sgbm.numberOfDisparities = 256; //  The greater, the less susceptible to noise, and less details
    sgbm.preFilterCap = 4;
    sgbm.minDisparity = 0;
    sgbm.uniquenessRatio = 5;
    sgbm.speckleWindowSize = 200;
    sgbm.speckleRange = 2;
    sgbm.disp12MaxDiff = 10;
    sgbm.fullDP = false;
    sgbm.P1 = 600; // no idea
    sgbm.P2 = 2400; // no idea

    Mat disp, disp_n;
    sgbm(image1, image2, disp);
    normalize(disp, disp_n, 0, 255, CV_MINMAX, CV_8U);

    string window_title = "Display Window";

    namedWindow(window_title, WINDOW_NORMAL );
    imshow(window_title, disp_n);
    waitKey(0);
    return 0;
}
