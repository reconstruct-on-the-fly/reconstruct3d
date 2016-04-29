# Reconstruct 3D

Uses stereo images for 3D reconstruction. Our focus is on terrain
reconstruction.

## Process

Pre-Process:
* Calibrate your camera with [PyCalibrate](https://github.com/reconstruct-on-the-fly/pycalibrate)

Main-Process:
* Rectify images
* Compute disparities between images
* Compute depth from the disparity map
* Use the depth map to reconstruct the 3D mesh


## Technologies

We mostly use OpenCV to process the images.

## Setting up

1. Install [Vagrant](https://www.vagrantup.com/) and Virtualbox
2. Clone this repository
3. Open the new folder, and create a new box:

```shell
$ vagrant up
$ vagrant ssh
```
