# Reconstruct 3D

Uses stereo images for 3D reconstruction. Our focus is on terrain
reconstruction.

## Process

Pre-Process:
* Calibrate your camera with [PyCalibrate](https://github.com/reconstruct-on-the-fly/pycalibrate)

Main-Process:
* Rectify images
* Compute disparities between images
* Refine and filter disparity maps
* Use the disparity map to reconstruct the 3D mesh
  * Refine the model using Laplace for mesh smoothing
  * Simplification with Quadric Error Metrics

## Technologies

We mostly use OpenCV3 to process the images. And the
[Fast-Quadric-Mesh-Simplification](https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification)
for mesh simplification.

## Usage

```shell
usage: reconstruct3d
    [--project-title project_title]
    [--image-pair left right]
    [--set folder_path]
    [--disparity-range min max]
    [--disparity-window size]
    [--no-wls-filter | --wls-filter lambda sigma]
    [--no-noise-reduction-filter | --noise-reduction-filter window_size threshold]
    [--obj-height max_height]
    [--laplace-scale scale]
    [--laplace-iterations iterations]
    [--simplify fraction]
    [--no-reconstruction]
    [--no-disparity]
    [--with-rectification]
    [--help | -h]


 Parameters help
    [--project-title project_title]
         Defines the name of the current project
    [--image-pair left right]
         Receives 2 images to reconstruct the model
    [--set folder_path]
         Receives a set of images to reconstruct the model
    [--disparity-range min max]
         Minimum and maximum disparity must be positive and min > max

    [--disparity-window size]
         Window size for disparity block matching must be odd

    [--no-wls-filter | --wls-filter lambda sigma]
         --no-wls-filter disables wls filter
         --wls-filter lambda default must be positive recommended value 8000
         --wls-filter sigma must range from 0.8 to 2

    [--no-noise-reduction-filter | --noise-reduction-filter window_size threshold]
         --no-noise-reduction disables noise reduction
         --noise-reduction window_size must be odd
         --noise-reduction threshold must range from 0 to 1

    [--obj-height max_height]
         3D model maximum height for object

    [--laplace-scale scale]
         Scale for the laplacian smoothing must range from 0 to 1

    [--laplace-iterations iterations]
         Iterations for executing laplacian smoothing must be positive

    [--simplify fraction]
         Simplification percentage for 3D object must range from 0 to 1

    [--no-reconstruction]
         Disables reconstruction process

    [--no-disparity]
         Disables disparity correspondence

    [--with-rectification]
         Enables rectfication

    [--help | -h]
```

## Setting up

1. Install [Vagrant](https://www.vagrantup.com/) and [Virtualbox](https://www.virtualbox.org/)
2. Clone this repository
3. Go to the repository folder, and create a new virtal machine with:
4. On the VM you need to install the Electron library to use the interface module

```shell
$ vagrant up
$ vagrant ssh
$ cd /vagrant
$ make
```
