#include "disparity_map.h"
#include <opencv2/core/core.hpp>
#include "opencv2/calib3d/calib3d.hpp"

using namespace cv;

DisparityMap::DisparityMap(){}

DisparityMap::DisparityMap(Mat image): m_image(image){}

DisparityMap
DisparityMap::generateDisparityMap(Mat left, Mat right)
{
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

    Mat disparity;
    sgbm(left, right, disparity);

    return DisparityMap(disparity);
}

Mat
DisparityMap::getImage()
{
    return m_image;
}
