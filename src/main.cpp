#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/stitching.hpp"
#include <opencv2/imgproc.hpp>

#include "camera.h"
#include "image_pair.h"
#include "disparity_map.h"
#include "depth_map.h"


using namespace cv;
using namespace std;

inline void print_usage()
{
    cout << "Usage:\n$reconstruct3d calibration_file image1 image2" << endl;
}

void test_disparity(char **argv)
{
    Mat image1, image2;
    image1 = imread(argv[2], CV_LOAD_IMAGE_COLOR);
    image2 = imread(argv[3], CV_LOAD_IMAGE_COLOR);

    if (!image1.data || !image2.data)
    {
        cout << "Could not open or find the image" << endl;
        print_usage();
        exit(EXIT_FAILURE);
    }

    Mat R = Mat::eye(3, 3, CV_64F);
    Vec<double, 3> T = {0, 1, 0};
    ImagePair imagePair(image1, image2, R, T);

    /* Disparity Map */
    DisparityMap disparityMap = DisparityMap::generateDisparityMap(imagePair);

    /* Depth Map */
    // DepthMap depthMap = DepthMap::generateDepthMap(disparityMap, Q);

    Mat disp_color;
    applyColorMap(disparityMap.getImage(), disp_color, COLORMAP_JET);

    /* Save Results */
    imwrite("/vagrant/color_diparity_map.jpg", disp_color);
    imwrite("/vagrant/diparity_map.jpg", disparityMap.getImage());
    // imwrite("/vagrant/depth_map.jpg", depthMap.getImage());
}

void test_rectify(char **argv)
{
    string calibration_file = argv[1];
    if(calibration_file == "")
    {
        cout << "Empty calibration_file" << endl;
        print_usage();
        exit(EXIT_FAILURE);
    }

    Mat image1, image2;
    image1 = imread(argv[2], CV_LOAD_IMAGE_COLOR);
    image2 = imread(argv[3], CV_LOAD_IMAGE_COLOR);

    if (!image1.data || !image2.data)
    {
        cout << "Could not open or find the image" << endl;
        print_usage();
        exit(EXIT_FAILURE);
    }

    /* Camera Creation */
    Camera camera = Camera::createFromFile(calibration_file);

    /* Image Rectification */
    Mat R = Mat::eye(3, 3, CV_64F);
    Mat Q;
    Vec<double, 3> T;
    T[0] = 0; T[1] = 3; T[2] = 0;

    ImagePair imagePair(image1, image2, R, T);
    ImagePair newImagePair = imagePair.rectify(camera, Q);

    imwrite("/vagrant/rectified_image1.jpg", newImagePair.getImage1());
    imwrite("/vagrant/rectified_image2.jpg", newImagePair.getImage2());

}

void test_merge_disparities(char **argv)
{
    Mat image1, image2, image3, image4, image5, image6;
    image1 = imread(argv[2], CV_LOAD_IMAGE_COLOR);
    image2 = imread(argv[3], CV_LOAD_IMAGE_COLOR);
    image3 = imread(argv[4], CV_LOAD_IMAGE_COLOR);
    image4 = imread(argv[5], CV_LOAD_IMAGE_COLOR);
    image5 = imread(argv[6], CV_LOAD_IMAGE_COLOR);
    image6 = imread(argv[7], CV_LOAD_IMAGE_COLOR);

    Mat R = Mat::eye(3, 3, CV_64F);
    Vec<double, 3> T = {0, 1, 0};

    ImagePair imagePair1(image1, image2, R, T);
    ImagePair imagePair2(image2, image3, R, T);
    ImagePair imagePair3(image3, image4, R, T);
    ImagePair imagePair4(image4, image5, R, T);
    ImagePair imagePair5(image5, image6, R, T);

    /* Disparity Map */
    cout << "Generating first disparity map..." << endl;
    DisparityMap disparityMap1 = DisparityMap::generateDisparityMap(imagePair1);
    cout << "Generating second disparity map..." << endl;
    DisparityMap disparityMap2 = DisparityMap::generateDisparityMap(imagePair2);
    cout << "Generating third disparity map..." << endl;
    DisparityMap disparityMap3 = DisparityMap::generateDisparityMap(imagePair3);
    cout << "Generating fourth disparity map..." << endl;
    DisparityMap disparityMap4 = DisparityMap::generateDisparityMap(imagePair4);
    cout << "Generating fifth disparity map..." << endl;
    DisparityMap disparityMap5 = DisparityMap::generateDisparityMap(imagePair5);
    cout << "Merging disparity maps..." << endl;

    /* Apply color maps */
    Mat color_map1, color_map2, color_map3, color_map4, color_map5;
    applyColorMap(disparityMap1.getImage(), color_map1, COLORMAP_JET);
    applyColorMap(disparityMap2.getImage(), color_map2, COLORMAP_JET);
    applyColorMap(disparityMap3.getImage(), color_map3, COLORMAP_JET);
    applyColorMap(disparityMap4.getImage(), color_map4, COLORMAP_JET);
    applyColorMap(disparityMap5.getImage(), color_map5, COLORMAP_JET);

    imwrite("/vagrant/diparity_map_1.jpg", color_map1);
    imwrite("/vagrant/diparity_map_2.jpg", color_map2);
    imwrite("/vagrant/diparity_map_3.jpg", color_map3);
    imwrite("/vagrant/diparity_map_4.jpg", color_map4);
    imwrite("/vagrant/diparity_map_5.jpg", color_map5);

    vector<Mat> maps = { color_map1, color_map2, color_map3, color_map4, color_map5 };
    DisparityMap merged_disp = DisparityMap::merge(maps);
    imwrite("/vagrant/diparity_map_full.jpg", merged_disp.getImage());
}

int main(int argc, char** argv)
{
    /* Argument Checking */
    //if (argc != 4)
    //{
    //    print_usage();
    //    exit(EXIT_FAILURE);
    //}

    test_disparity(argv);

    return EXIT_SUCCESS;
}
