#include "disparity_map.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/ximgproc/disparity_filter.hpp>
#include <limits>
#include <iostream>

using namespace std;
using namespace cv;
using namespace cv::ximgproc;

DisparityMap::DisparityMap(){}

DisparityMap::DisparityMap(Mat image): m_image(image){}

DisparityMap
DisparityMap::generateDisparityMap(Mat left, Mat right, bool gray_filter,
                                   bool no_filter)
{
    // Custom Params
    int min_disp = 0, max_disp = 224;
    int window_size = 3;
    int half_window_size = (window_size - 1) / 2;

    if(gray_filter)
    {
        cvtColor(left, left, COLOR_BGR2GRAY);
        cvtColor(right, right, COLOR_BGR2GRAY);
    }

    /* Disparity Algorithm */
    const int rows = left.rows;
    const int cols = left.cols * left.channels();

    Mat disparity = left.clone();

    for(int i = half_window_size; i < rows - half_window_size; ++i)
    {
        for(int j = half_window_size; j < cols - half_window_size - max_disp+1; ++j)
        {
            int min_sad = numeric_limits<int>::max();
            int best_disp = 0;

            for(int disp = min_disp; disp <= max_disp; ++disp)
            {
                 int sad = 0;
                 for(int ii = -half_window_size; ii < half_window_size; ++ii)
                 {
                     for(int jj = -half_window_size; jj < half_window_size; ++jj)
                     {
                         sad += abs(left.at<uchar>(i + ii, j + jj) -
                                    right.at<uchar>(i + ii, j + jj + disp));
                     }
                 }

                 if(sad < min_sad)
                 {
                     min_sad = sad;
                     best_disp = disp;
                 }
            }

            int mapped_disp = (255 / max_disp) * best_disp; 
            disparity.at<uchar>(i, j) = mapped_disp;
        }
    }

    if(!no_filter)
    {
        double lambda = 8000.0;
        double sigma  = 1.5;

        Ptr<DisparityWLSFilter> wls_filter;
        wls_filter = createDisparityWLSFilterGeneric(false);
        wls_filter->setLambda(lambda);
        wls_filter->setSigmaColor(sigma);

        Mat filtered_disp;
        disparity.convertTo(disparity, CV_16S);
        wls_filter->filter(disparity, left, filtered_disp);
        getDisparityVis(filtered_disp, disparity, 16.0);
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
