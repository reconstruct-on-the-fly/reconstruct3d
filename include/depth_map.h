#ifndef DEPTH_MAP_H
#define DEPTH_MAP_H

#include <opencv2/core/core.hpp>
#include "disparity_map.h"

class DepthMap {

public:
    DepthMap();
    DepthMap(cv::Mat image);

    static DepthMap generateDepthMap(DisparityMap disparity,
                                     cv::Mat Q,
                                     bool handleMissingValues=true,
                                     int ddepth=-1);

    cv::Mat getImage();

private:
    cv::Mat m_image;

};

#endif
