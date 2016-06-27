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

// Command line instructions
const std::string USAGE = "usage: reconstruct3d <left_image> <right_image> "
                          "<obj_name> "
                          "\n\t[--disparity-range min max] "
                          "\n\t[--disparity-window size] "
                          "\n\t[--no-wls-filter | --wls-filter lambda sigma] "
                          "\n\t[--no-noise-reduction-filter | --noise-reduction-filter window_size threshold] "
                          "\n\t[--obj-height max_height] "
                          "\n\t[--laplace-scale scale] "
                          "\n\t[--laplace-iterations iterations] "
                          "\n\t[--simplify fraction] "
                          "\n\t[--no-reconstruction] "
                          "\n\t[--no-disparity] "
                          "\n\t[--help | -h] "
                          "\n";

const std::string HELP = "\n\n Parameters help"
                          "\n\t[--disparity-range min max] "
                          "\n\t\t Minimum and maximum disparity must be positive and min > max"
                          "\n\n\t[--disparity-window size] "
                          "\n\t\t Window size for disparity block matching must be odd"
                          "\n\n\t[--no-wls-filter | --wls-filter lambda sigma] "
                          "\n\t\t --no-wls-filter disables wls filter"
                          "\n\t\t --wls-filter lambda default must be positive recommended value 8000"
                          "\n\t\t --wls-filter sigma must range from 0.8 to 2 "
                          "\n\n\t[--no-noise-reduction-filter | --noise-reduction-filter window_size threshold] "
                          "\n\t\t --no-noise-reduction disables noise reduction"
                          "\n\t\t --noise-reduction window_size must be odd"
                          "\n\t\t --noise-reduction threshold must range from 0 to 1"
                          "\n\n\t[--obj-height max_height] "
                          "\n\t\t 3D model maximum height for object"
                          "\n\n\t[--laplace-scale scale] "
                          "\n\t\t Scale for the laplacian smoothing must range from 0 to 1"
                          "\n\n\t[--laplace-iterations iterations] "
                          "\n\t\t Iterations for executing laplacian smoothing must be positive"
                          "\n\n\t[--simplify fraction] "
                          "\n\t\t Simplification percentage for 3D object must range from 0 to 1"
                          "\n\n\t[--no-reconstruction] "
                          "\n\t\t Disables reconstruction process "
                          "\n\n\t[--no-disparity] "
                          "\n\t\t Disables disparity correspondence "
                          "\n\n\t[--help | -h] "
                          "\n";

// User defined renderer configuration
struct Options {
    std::string left_image_path, right_image_path;
    std::string obj_name;

    // Disparity
    int min_disparity, max_disparity, disparity_window_size;
    bool noise_reduction_filter, wls_filter;
    int noise_reduction_window_size;
    float noise_reduction_threshold;
    double wls_lambda, wls_sigma;

    bool no_disparity;

    // Reconstruction
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
        for (int i = 1; i < argc; ++i)
        {
            if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
            {
                std::cout << USAGE;
                std::cout << HELP;
                std::exit(EXIT_SUCCESS);
            }

        }
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
    options.disparity_window_size = 3;
    options.min_disparity = 0;
    options.max_disparity = 160;
    options.wls_filter = true;
    options.wls_lambda = 16000.0;
    options.wls_sigma = 2;
    options.noise_reduction_filter = true;
    options.noise_reduction_window_size = 15;
    options.noise_reduction_threshold = 0.4f;
    options.no_disparity = false;

    options.obj_max_height = 0.0f;
    options.obj_laplace_scale = 0.5f;
    options.obj_laplace_iterations = 15;
    options.obj_simplification_fraction = 0.2; // 80%
    options.no_reconstruction = false;

    for (int i = 4; i < argc; ++i)
    {
        // Disparity Arguments
        if (!strcmp(argv[i], "--disparity-range"))
        {
            options.min_disparity = atoi(argv[++i]);
            options.max_disparity = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "--disparity-window"))
            options.disparity_window_size = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--no-wls-filter"))
            options.wls_filter = false;
        else if (!strcmp(argv[i], "--wls-filter"))
        {
            options.wls_filter = true;
            options.wls_lambda = atof(argv[++i]);
            options.wls_sigma  = atof(argv[++i]);
        }
        else if (!strcmp(argv[i], "--no-noise-reduction-filter"))
            options.noise_reduction_filter = false;
        else if (!strcmp(argv[i], "--noise-reduction-filter"))
        {
            options.noise_reduction_filter = true;
            options.noise_reduction_window_size = atoi(argv[++i]);
            options.noise_reduction_threshold   = atof(argv[++i]);
        }

        // Reconstruction Arguments
        else if (!strcmp(argv[i], "--obj-height"))
            options.obj_max_height = atof(argv[++i]);
        else if (!strcmp(argv[i], "--laplace-scale"))
            options.obj_laplace_scale = atof(argv[++i]);
        else if (!strcmp(argv[i], "--laplace-iterations"))
            options.obj_laplace_iterations = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--simplify"))
            options.obj_simplification_fraction = std::min(1.0, std::max(0.0, atof(argv[++i])));

        else if (!strcmp(argv[i], "--no-reconstruction"))
            options.no_reconstruction = true;
        else if (!strcmp(argv[i], "--no-disparity"))
            options.no_disparity = true;

        // Usage Arguments
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
        {
            std::cout << USAGE;
            std::cout << HELP;
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

    /* Image Rectification */
    // TODO

    Mat disparity_image;

    /* Disparity Map */
    if (!options.no_disparity)
    {
        DisparityMap disparityMap = DisparityMap::generateDisparityMap(
                left_image, right_image, options.obj_name,
                options.disparity_window_size, options.min_disparity, options.max_disparity,
                options.wls_filter, options.wls_lambda, options.wls_sigma,
                options.noise_reduction_filter, options.noise_reduction_window_size, options.noise_reduction_threshold
        );
    }
    else
        disparity_image = left_image;


    if(!options.no_reconstruction)
    {
        Mesh().generateMesh(disparity_image, options.obj_name,
                            options.obj_max_height, options.obj_laplace_scale,
                            options.obj_laplace_iterations,
                            options.obj_simplification_fraction);
    }

    return EXIT_SUCCESS;
}
