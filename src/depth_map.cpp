#include "depth_map.h"
#include <opencv2/calib3d/calib3d.hpp>

using namespace cv;

DepthMap::DepthMap() {}

DepthMap::DepthMap(Mat image): m_image(image) {}

static DepthMap
generateDepthMap(InputArray disparity, InputArray Q,
                           bool handleMissingValues, int ddepth)
{
    Mat depth_map; // output depth map
    reprojectImageTo3D(disparity, depth_map, Q, handleMissingValues, ddepth);

    return DepthMap(depth_map);
}

cv::Mat
DepthMap::getImage() { return m_image; }
