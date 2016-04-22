#ifndef DEPTH_MAP_H
#define DEPTH_MAP_H

#include <opencv2/core/core.hpp>

class DepthMap {

public:
    DepthMap();
    DepthMap(cv::Mat image);

    static DepthMap generateDepthMap(cv::InputArray disparity,
                                     cv::InputArray Q,
                                     bool handleMissingValues=false,
                                     int ddepth=-1);
private:
    cv::Mat m_image;

};

#endif
