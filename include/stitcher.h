#ifndef STITCHER_H
#define STITCHER_H

#include <opencv2/core/core.hpp>
#include "disparity_map.h"

class Stitcher{

public:
    Stitcher();

    static DisparityMap merge(std::vector<DisparityMap> maps,
                              std::string objname);
    static DisparityMap mergePair(DisparityMap left, DisparityMap right);
    static int findOffset(cv::Mat left, cv::Mat right);

};
#endif
