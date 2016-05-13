#include "disparity_map.h"
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
using namespace std;
using namespace cv;

DisparityMap::DisparityMap(){}

DisparityMap::DisparityMap(Mat image): m_image(image){}

DisparityMap
DisparityMap::generateDisparityMap(Mat left, Mat right)
{
    auto n_channels = left.channels();
    cout << n_channels << endl;

    StereoSGBM sgbm;
    sgbm.minDisparity = 0;
    sgbm.numberOfDisparities = 512; // [Must be divisible by 16] CHECK
    sgbm.SADWindowSize = 3;  // [Odd number between 3..11] CHECK
    sgbm.P1 =  1*n_channels*sgbm.SADWindowSize*sgbm.SADWindowSize;
    sgbm.P2 = 100*n_channels*sgbm.SADWindowSize*sgbm.SADWindowSize; // [P2 > P1]
    sgbm.disp12MaxDiff = 100;
    sgbm.preFilterCap = 63;
    sgbm.uniquenessRatio = 10; // [5..15]
    sgbm.speckleWindowSize = 100; // [50..200]
    sgbm.speckleRange = 32; // [1..2] CHECK = 32
    sgbm.fullDP = false;

    //Mat l1, r1;
    //cvtColor(left, l1, CV_BGR2GRAY);
    //cvtColor(right, r1, CV_BGR2GRAY);
    Mat disparity;
    //sgbm(l1, r1, disparity);
    sgbm(left, right, disparity);

    //Mat disparity;

    //StereoBM bm;
    //bm.state->preFilterCap = 31;
    //bm.state->SADWindowSize = 9;
    //bm.state->minDisparity = 0;
    //bm.state->numberOfDisparities = 256;
    //bm.state->textureThreshold = 10;
    //bm.state->uniquenessRatio = 15;
    //bm.state->speckleWindowSize = 100;
    //bm.state->speckleRange = 32;
    //bm.state->disp12MaxDiff = 1;

    //Mat l1, r1;
    //cvtColor(left, l1, CV_BGR2GRAY);
    //cvtColor(right, r1, CV_BGR2GRAY);

    //bm(l1, r1, disparity);

    return DisparityMap(disparity);
}

DisparityMap
DisparityMap::generateDisparityMap(ImagePair imagePair)
{
    return DisparityMap::generateDisparityMap(imagePair.getImage1(),
                                              imagePair.getImage2());
}

Mat
DisparityMap::getImage()
{
    return m_image;
}
