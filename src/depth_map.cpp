#include "depth_map.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <limits>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace cv;

DepthMap::DepthMap() {}

DepthMap::DepthMap(Mat image): m_image(image) {}

DepthMap
DepthMap::generateDepthMap(DisparityMap disparity, cv::Mat Q)
{
    cv::Mat_<float> Qf = Q;
    cv::Mat_<float> disparity32F = disparity.getImage();
    cv::Mat_<cv::Vec3f> XYZ(disparity32F.rows,disparity32F.cols);  // Output point cloud
    cv::Mat_<float> Z(disparity32F.rows,disparity32F.cols);
    cv::Mat_<float> vec_tmp(4,1);
    for(int y=0; y<disparity32F.rows; ++y) {
        for(int x=0; x<disparity32F.cols; ++x) {
            vec_tmp(0)=x; vec_tmp(1)=y; vec_tmp(2)=disparity32F.at<float>(y,x); vec_tmp(3)=1;
            vec_tmp = Qf*vec_tmp;
            vec_tmp /= vec_tmp(3);
            cv::Vec3f &point = XYZ.at<cv::Vec3f>(y,x);
            point[0] = vec_tmp(0);
            point[1] = vec_tmp(1);
            point[2] = vec_tmp(2);
            Z.at<float>(y, x) = vec_tmp(1);
        }
    }

    return DepthMap(Z);
}

cv::Mat
DepthMap::getImage() { return m_image; }
