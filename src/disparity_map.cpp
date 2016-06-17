#include "disparity_map.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ximgproc/disparity_filter.hpp>
#include <limits>
#include <iostream>

using namespace std;
using namespace cv;
using namespace cv::ximgproc;

DisparityMap::DisparityMap(){}

DisparityMap::DisparityMap(Mat image): m_image(image){}

void
DisparityMap::preprocessImages(Mat &left, Mat &right)
{
    cvtColor(left, left, COLOR_BGR2GRAY);
    cvtColor(right, right, COLOR_BGR2GRAY);

    Ptr<CLAHE> clahe = createCLAHE(1, Size(2, 2));
    clahe->apply(left, left);
    clahe->apply(right, right);
}

int
DisparityMap::sadAt(int i, int j, Mat left, Mat right, int window_size,
                    int offset)
{
    int half_window_size = (window_size - 1) / 2;
    int sad = 0;
    for(int ii = -half_window_size; ii < half_window_size; ++ii)
    {
        for(int jj = -half_window_size; jj < half_window_size; ++jj)
        {
            sad += abs(left.at<uchar>(i + ii, j + jj) -
                       right.at<uchar>(i + ii, j + jj + offset));
        }
    }

    return sad;
}

DisparityMap
DisparityMap::generateDisparityMap(Mat left, Mat right, bool no_filter)
{
    // Custom Params
    int min_disp = -224, max_disp = 224;
    int window_size = 3;
    int half_window_size = (window_size - 1) / 2;
    int T = 10;

    DisparityMap::preprocessImages(left, right);

    /* Disparity Algorithm */
    const int rows = left.rows;
    const int cols = left.cols * left.channels();

    Mat disparity = left.clone();
    int best_disp = 0;

    for(int i = half_window_size; i < rows - half_window_size; ++i)
    {
        for(int j = half_window_size; j < cols - half_window_size; ++j)
        {
            int min_sad = numeric_limits<int>::max();
            int new_sad = DisparityMap::sadAt(i, j, left, left,
                                              window_size, -window_size);

            if (new_sad > T)
            {
                min_sad = numeric_limits<int>::max();
            }
            else
            {
                int mapped_disp = (255 / max_disp) * best_disp;
                disparity.at<uchar>(i, j) = mapped_disp;
                continue;
            }

            for(int disp = min_disp; disp <= max_disp; ++disp)
            {
                int sad = DisparityMap::sadAt(i, j, left, right,
                                              window_size, disp);

                if(sad < min_sad)
                {
                    min_sad = sad;
                    best_disp = abs(disp);
                }
            }

            int mapped_disp = (255 / max_disp) * best_disp; 
            disparity.at<uchar>(i, j) = mapped_disp;
        }
    }

    no_filter = true;
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
DisparityMap::generateDisparityMap(ImagePair imagePair, bool no_filter)
{
    return DisparityMap::generateDisparityMap(imagePair.getImage1(),
                                              imagePair.getImage2(),
                                              no_filter);
}

Mat
DisparityMap::getImage()
{
    return m_image;
}
