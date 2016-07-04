#ifndef DISPARITY_MAP_H
#define DISPARITY_MAP_H

#include <string>
#include <opencv2/core/core.hpp>

class DisparityMap{

public:
    DisparityMap();
    DisparityMap(cv::Mat image);

    static DisparityMap generateDisparityMap(
        cv::Mat left, cv::Mat right, std::string objname,
        int window_size, int min_disp, int max_disp,
        bool wls_filter, double wls_lambda, double wls_sigma,
        bool noise_reduction_filter, int noise_reduction_window_size,
        float noise_reduction_threshold);

    static void preprocessImages(cv::Mat &left, cv::Mat &right);
    static cv::Mat normalize_image(cv::Mat image, int window_size,
                                   float threshold);
    static int sadAt(int i, int j, cv::Mat left, cv::Mat right,
                     int window_size, int offset);
    static DisparityMap merge(DisparityMap left, DisparityMap right,
                              int offset);

    cv::Mat getImage();

private:
    cv::Mat m_image;
};
#endif
