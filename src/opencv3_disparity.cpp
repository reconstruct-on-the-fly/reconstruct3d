#include "opencv2/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/ximgproc/disparity_filter.hpp"
#include <iostream>
#include <string>

using namespace cv;
using namespace cv::ximgproc;
using namespace std;

int main(int argc, char **argv)
{
    Mat left, right;
    left = imread(argv[1], IMREAD_COLOR);
    right = imread(argv[2], IMREAD_COLOR);

    Mat left_for_matcher, right_for_matcher;
    resize(left, left_for_matcher, Size(), 0.5, 0.5);
    resize(right, right_for_matcher, Size(), 0.5, 0.5);

    cvtColor(left_for_matcher,  left_for_matcher,  COLOR_BGR2GRAY);
    cvtColor(right_for_matcher, right_for_matcher, COLOR_BGR2GRAY);

    int max_disp = 160;		// For SBM
    int wsize = 3;			// For SBM
    double lambda = 8000.0;		// For WLS Filter
    double sigma = 1.5;		// For WLS Filter
    double vis_mult = 1.0;		// To scale disparity map visualization

    //Ptr<StereoBM> left_matcher = StereoBM::create(max_disp, wsize);
    Ptr<StereoSGBM> left_matcher  = StereoSGBM::create(0, max_disp, wsize);
    left_matcher->setP1(1*wsize*wsize);
    left_matcher->setP2(96*wsize*wsize);
    left_matcher->setPreFilterCap(63);
    left_matcher->setUniquenessRatio(10);
    left_matcher->setSpeckleWindowSize(100);
    left_matcher->setSpeckleRange(32);
    left_matcher->setMode(StereoSGBM::MODE_SGBM_3WAY);
    Ptr<DisparityWLSFilter> wls_filter;
    wls_filter = createDisparityWLSFilter(left_matcher);
    Ptr<StereoMatcher> right_matcher = createRightMatcher(left_matcher);

    // Compute Disparity
    Mat left_disp, right_disp, filtered_disp;
    left_matcher-> compute(left_for_matcher, right_for_matcher, left_disp);
    right_matcher->compute(right_for_matcher, left_for_matcher, right_disp);

    // Filtering
    wls_filter->setLambda(lambda);
    wls_filter->setSigmaColor(sigma);
    wls_filter->filter(left_disp, left, filtered_disp, right_disp);

    // Confidence maps (???)
    Mat conf_map = Mat(left.rows,left.cols,CV_8U);
    conf_map = Scalar(255);
    conf_map = wls_filter->getConfidenceMap();

    // Show images
    Mat raw_disp_vis;
    getDisparityVis(left_disp, raw_disp_vis, vis_mult);
    namedWindow("raw disparity", WINDOW_AUTOSIZE);
    // imshow("raw disparity", raw_disp_vis);
    imwrite("/vagrant/disp_vis.jpg", raw_disp_vis);
    waitKey();

    Mat filtered_disp_vis;
    getDisparityVis(filtered_disp, filtered_disp_vis, vis_mult);
    namedWindow("filtered disparity", WINDOW_AUTOSIZE);
    // imshow("filtered disparity", filtered_disp_vis);
    imwrite("/vagrant/filtered_disp_vis.jpg", filtered_disp_vis);
    waitKey();

    return 0;
}
