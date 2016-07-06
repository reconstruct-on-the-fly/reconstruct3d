#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>
#include <glob.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "camera.h"
#include "image_pair.h"
#include "disparity_map.h"
#include "stitcher.h"
#include "depth_map.h"
#include "mesh.h"

using namespace cv;
using namespace std;

// Command line instructions
const std::string USAGE = "usage: reconstruct3d "
                          "\n\t[--project-title project_title] "
                          "\n\t[--image-pair left right] "
                          "\n\t[--set folder_path] "
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
                          "\n\t[--with-rectification] "
                          "\n\t[--help | -h] "
                          "\n";

const std::string HELP = "\n\n Parameters help"
                          "\n\t[--project-title project_title] "
                          "\n\t\t Defines the name of the current project"
                          "\n\t[--image-pair left right] "
                          "\n\t\t Receives 2 images to reconstruct the model"
                          "\n\t[--set folder_path] "
                          "\n\t\t Receives a set of images to reconstruct the model"
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
                          "\n\n\t[--with-rectification] "
                          "\n\t\t Enables rectfication "
                          "\n\n\t[--help | -h] "
                          "\n";

// User defined renderer configuration
struct Options {
    std::string project_title;
    bool image_pair;
    std::string left_image_path, right_image_path;
    std::string folder_path;

    //Rectification
    bool no_rectification;

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
    if (argc < 2)
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

    // Default Values
    options.no_rectification = true;
    options.project_title = "project";
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

    options.obj_max_height = 1.0f;
    options.obj_laplace_scale = 0.5f;
    options.obj_laplace_iterations = 15;
    options.obj_simplification_fraction = 0.2; // 80%
    options.no_reconstruction = false;

    for (int i = 1; i < argc; ++i)
    {
        // Rectification Arguments
        if (!strcmp(argv[i], "--with-rectification"))
            options.no_rectification = false;

        // Input Arguments
        else if (!strcmp(argv[i], "--project-title"))
        {
            options.project_title = string(argv[++i]);
        }
        else if (!strcmp(argv[i], "--image-pair"))
        {
            options.image_pair = true;
            options.left_image_path = string(argv[++i]);
            options.right_image_path = string(argv[++i]);
        }
        else if (!strcmp(argv[i], "--set"))
        {
            options.image_pair = false;
            options.folder_path = string(argv[++i]);
        }

        // Disparity Arguments
        else if (!strcmp(argv[i], "--disparity-range"))
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

vector<cv::Mat> loadImages(std::string folder)
{
    vector<cv::Mat> images;
    vector<String> filenames;
    glob(folder, filenames);

    for (string filename : filenames)
    {
        images.push_back(loadImage(filename));
    }

    if (images.empty())
    {
        std::cout << "ERROR: Could not find any images in " << folder
                  << endl;
        std::cout << USAGE;
        exit(EXIT_FAILURE);
    }

    return images;
}

int main(int argc, char** argv)
{
    auto options = parseArgs(argc, argv);

    vector<Mat> images;
    if (options.image_pair)
    {
        images.push_back(loadImage(options.left_image_path));
        images.push_back(loadImage(options.right_image_path));
    }
    else
    {
        images = loadImages(options.folder_path);
    }

    /* Image Rectification */
    ImagePair rectifed_pair;

    if(!options.no_rectification && options.image_pair)
    {
        Mat left_image = loadImage(options.left_image_path);
        Mat right_image = loadImage(options.right_image_path);

        rectifed_pair  = ImagePair(images[0], images[1]).rectify(options.project_title);
    }
    else
        rectifed_pair = ImagePair(images[0], images[1]);

    /* Disparity Map */
    Mat disparityImage;

    if (!options.no_disparity)
    {
        if (options.image_pair)
        {
            DisparityMap disparityMap = DisparityMap::generateDisparityMap(
                    images[0], images[1], options.project_title,
                    options.disparity_window_size, options.min_disparity,options.max_disparity,
                    options.wls_filter, options.wls_lambda, options.wls_sigma,
                    options.noise_reduction_filter, options.noise_reduction_window_size, 
                    options.noise_reduction_threshold);

            disparityImage = disparityMap.getImage();
        } 
        else
        {
            vector<DisparityMap> maps;

            for (size_t i = 0; i < images.size() - 1; i++)
            {
                DisparityMap disparityMap = DisparityMap::generateDisparityMap(
                   images[i], images[i+1], options.project_title+to_string(i),
                   options.disparity_window_size, options.min_disparity,options.max_disparity,
                   options.wls_filter, options.wls_lambda, options.wls_sigma,
                   options.noise_reduction_filter, options.noise_reduction_window_size, 
                   options.noise_reduction_threshold);
                maps.push_back(disparityMap);
            }

            DisparityMap mergedDisp = Stitcher::merge(maps, options.project_title);
            disparityImage = mergedDisp.getImage();
        }
    }
    else
        disparityImage = images[0];

    if(!options.no_reconstruction)
    {
        Mesh().generateMesh(disparityImage, options.project_title,
                            options.obj_max_height, options.obj_laplace_scale,
                            options.obj_laplace_iterations,
                            options.obj_simplification_fraction);
    }

    return EXIT_SUCCESS;
}
