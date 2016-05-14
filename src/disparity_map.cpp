#include "disparity_map.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/ximgproc/disparity_filter.hpp>

using namespace cv;
using namespace cv::ximgproc;

DisparityMap::DisparityMap(){}

DisparityMap::DisparityMap(Mat image): m_image(image){}

DisparityMap
DisparityMap::generateDisparityMap(Mat left, Mat right, bool gray_filter,
                                   bool no_filter)
{
    // Convert images to gray scale
    if(gray_filter)
    {
        cvtColor(left,  left,  COLOR_BGR2GRAY);
        cvtColor(right, right, COLOR_BGR2GRAY);
    }

    // Creatring SGBM matchers
    int max_disp = 160;
    int wsize = 3;

    Ptr<StereoSGBM> left_matcher = StereoSGBM::create(0, max_disp, wsize);
    left_matcher->setP1(1*wsize*wsize);
    left_matcher->setP2(96*wsize*wsize);
    left_matcher->setPreFilterCap(63);
    left_matcher->setUniquenessRatio(10);
    left_matcher->setSpeckleWindowSize(100);
    left_matcher->setSpeckleRange(32);
    left_matcher->setMode(StereoSGBM::MODE_SGBM_3WAY);

    Ptr<StereoMatcher> right_matcher = createRightMatcher(left_matcher);

    // Compute Disparity Map
    Mat left_disparity, right_disparity;

    left_matcher->compute( left, right, left_disparity);
    right_matcher->compute(right, left, right_disparity);

    Mat disparity;
    double vis_mult = 1.0;  // To scale disparity map visualization

    if(no_filter)
        getDisparityVis(left_disparity, disparity, vis_mult);
    else
    {
        // WLS Filter
        Ptr<DisparityWLSFilter> wls_filter;
        wls_filter = createDisparityWLSFilter(left_matcher);

        double lambda = 8000.0;
        double sigma  = 1.5;
        wls_filter->setLambda(lambda);
        wls_filter->setSigmaColor(sigma);

        Mat filtered_disparity;
        wls_filter->filter(left_disparity, left, filtered_disparity,
                           right_disparity);

        getDisparityVis(filtered_disparity, disparity, vis_mult);
    }

    return DisparityMap(disparity);
}

DisparityMap
DisparityMap::generateDisparityMap(ImagePair imagePair, bool gray_filter,
                                   bool no_filter)
{
    return DisparityMap::generateDisparityMap(imagePair.getImage1(),
                                              imagePair.getImage2(),
                                              gray_filter, no_filter);
}

Mat
DisparityMap::getImage()
{
    return m_image;
}
