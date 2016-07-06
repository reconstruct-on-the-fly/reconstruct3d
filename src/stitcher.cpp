#include "stitcher.h"
#include "disparity_map.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Stitcher::Stitcher(){}

DisparityMap
Stitcher::merge(vector<DisparityMap> maps, string objname)
{
    DisparityMap merged_disp = maps.at(0);
    for (int i=1; i<maps.size(); ++i)
    {
        merged_disp = Stitcher::mergePair(merged_disp, maps.at(i));
    }

    Mat color_merged_maps;
    applyColorMap(merged_disp.getImage(), color_merged_maps, COLORMAP_JET);
    imwrite(objname+"_full_disparity.jpg", merged_disp.getImage());
    imwrite(objname+"_color_full_disparity.jpg", color_merged_maps);

    return DisparityMap(merged_disp);
}

DisparityMap
Stitcher::mergePair(DisparityMap left_disp, DisparityMap right_disp)
{
    Mat left = left_disp.getImage();
    Mat right = right_disp.getImage();
    int offset = Stitcher::findOffset(left, right);
    const int rows = left.rows;
    const int cols = right.cols + offset;
    Mat merged_maps = Mat(rows, cols, CV_8UC1);

    cout << "Merging disparities..." << endl;
    Rect roi_left = Rect(0, 0, offset, rows);
    Rect roi_right = Rect(right.cols - offset, 0, offset, right.rows);
    left(roi_left).copyTo(merged_maps(roi_left));
    right(roi_right).copyTo(merged_maps(Rect(right.cols, 0, offset, rows)));


    for(int i = 0; i < rows; ++i)
    {
        for (int j = offset; j < cols - offset; ++j)
        {
            float alpha = (float)(j - offset) / (cols - offset - offset);
            uchar l_pixel = left.at<uchar>(i, j);
            uchar r_pixel = right.at<uchar>(i, j - offset);
            merged_maps.at<uchar>(i, j) = ((1 - alpha) * l_pixel) +
                                          (alpha * r_pixel);
        }
    }

    return DisparityMap(merged_maps);
}

int
Stitcher::findOffset(Mat left, Mat right)
{
    cout << "Finding offset..." << endl;
    Ptr<Feature2D> f2d = xfeatures2d::SURF::create();
    std::vector<KeyPoint> keypoints_1, keypoints_2;
    f2d->detect(left, keypoints_1);
    f2d->detect(right, keypoints_2);

    Mat descriptors_1, descriptors_2;
    f2d->compute(left, keypoints_1, descriptors_1);
    f2d->compute(right, keypoints_2, descriptors_2);

    FlannBasedMatcher matcher;
    std::vector<DMatch> matches;
    matcher.match(descriptors_1, descriptors_2, matches);

    double avg_dist = 0;
    int avg_count = 0;

    for( int i = 0; i < descriptors_1.rows; i++ )
    {
        auto pt1 = keypoints_1[matches[i].queryIdx].pt;
        auto pt2 = keypoints_2[matches[i].trainIdx].pt;

        if (abs(pt1.y - pt2.y) < 20)
        {
            avg_dist += abs(pt1.x - pt2.x);
            ++avg_count;
        }
    }
    return avg_dist / avg_count;
}
