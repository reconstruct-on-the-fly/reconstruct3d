#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/stitching.hpp"
#include <opencv2/imgproc.hpp>

#include "camera.h"
#include "image_pair.h"
#include "disparity_map.h"
#include "depth_map.h"
#include "mesh.h"

using namespace cv;
using namespace std;

/* OLD MAIN
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

    DisparityMap disparityMap = DisparityMap::generateDisparityMap(imagePair);

    // DepthMap depthMap = DepthMap::generateDepthMap(disparityMap, Q);

    Mat disp_color;
    applyColorMap(disparityMap.getImage(), disp_color, COLORMAP_JET);

    imwrite("/vagrant/color_diparity_map.jpg", disp_color);
    imwrite("/vagrant/diparity_map.jpg", disparityMap.getImage());
    // imwrite("/vagrant/depth_map.jpg", depthMap.getImage());
}

void test_rectify(char **argv)
{
    string calibration_file = argv[1];
    if(calibration_file == "")

    Camera camera = Camera::createFromFile(calibration_file);

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

    test_disparity(argv);
*/

// Command line instructions
const std::string USAGE = "usage: reconstruct3d <left_image> <right_image> "
                          "<obj_name> "
                          "[--obj-height max_height] "
                          "[--laplace-scale scale] "
                          "[--laplace-iterations iterations] "
                          "[--simplify fraction] "
                          "[--no-reconstruction] "
                          "[--help | -h] "
                          "\n";

// User defined renderer configuration
struct Options {
    std::string left_image_path, right_image_path;
    std::string obj_name;
    float obj_max_height;
    float obj_laplace_scale;
    int obj_laplace_iterations;
    float obj_simplification_fraction;

    bool no_reconstruction;
};

Options parseArgs(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cout << "ERROR: Missing arguments!" << std::endl;
        std::cout << USAGE;
        std::exit(EXIT_FAILURE);
    }

    Options options;

    // Reading Image Pair
    options.left_image_path  = argv[1];
    options.right_image_path = argv[2];

    // Reading output file name
    options.obj_name         = argv[3];

    // Default Values
    options.obj_max_height = 0.3f;
    options.obj_laplace_scale = 0.5f;
    options.obj_laplace_iterations = 15;
    options.obj_simplification_fraction = 0.2; // 80%
    options.no_reconstruction = false;

    for (int i = 4; i < argc; ++i)
    {
        // Reconstruction Arguments
        if (!strcmp(argv[i], "--obj-height"))
            options.obj_max_height = atof(argv[++i]);
        else if (!strcmp(argv[i], "--laplace-scale"))
            options.obj_laplace_scale = atof(argv[++i]);
        else if (!strcmp(argv[i], "--laplace-iterations"))
            options.obj_laplace_iterations = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--simplify"))
            options.obj_simplification_fraction = std::min(1.0, std::max(0.0, atof(argv[++i])));
        else if (!strcmp(argv[i], "--no-reconstruction"))
            options.no_reconstruction = true;

        // Usage Arguments
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
        {
            std::cout << USAGE;
            std::exit(EXIT_SUCCESS);
        }
        else
        {
            std::cout << "ERROR: No valid command [" << argv[i] << "]"
                      << std::endl;
            std::cout << USAGE;
            std::exit(EXIT_SUCCESS);
        }
    }

    return options;
}

cv::Mat loadImage(std::string filename)
{
    cv::Mat image = cv::imread(filename, CV_LOAD_IMAGE_COLOR);

    if (!image.data)
    {
        std::cout << "ERROR: Could not open or find the image " << filename
                  << std::endl;
        std::cout << USAGE;
        exit(EXIT_FAILURE);
    }

    return image;
}

int main(int argc, char** argv)
{
    auto options = parseArgs(argc, argv);

    Mat left_image = loadImage(options.left_image_path);
    Mat right_image = loadImage(options.right_image_path);

    ImagePair input_images(left_image, right_image);

    /* Image Rectification */
    // TODO

    /* Disparity Map */
    //DisparityMap disparityMap = DisparityMap::generateDisparityMap(input_images);

    //Mat disp_color;
    //applyColorMap(disparityMap.getImage(), disp_color, COLORMAP_JET);

    ///* Save Results */
    //imwrite("diparity_map.jpg", disparityMap.getImage());
    //imwrite("color_diparity_map.jpg", disp_color);

    if(!options.no_reconstruction)
    {
        Mesh().generateMesh(input_images.getImage1(), options.obj_name,
                            options.obj_max_height, options.obj_laplace_scale,
                            options.obj_laplace_iterations,
                            options.obj_simplification_fraction);
    }

    return EXIT_SUCCESS;
}
