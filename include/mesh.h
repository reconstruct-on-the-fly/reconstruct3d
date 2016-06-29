#ifndef MESH_H
#define MESH_H

#include <vector>
#include <unordered_set>
#include <string>

#include <opencv2/core/core.hpp>
#include "depth_map.h"

class Mesh {

public:
    Mesh();
    Mesh(std::vector<cv::Point3f> vertices, std::vector<cv::Point3i> faces);
    Mesh(std::vector<cv::Point3f> vertices, std::vector<cv::Point3i> faces,
         std::vector< std::unordered_set<int> > _G);

    void generateMesh(DepthMap depthMap, float max_height);
    void generateMesh(cv::Mat image, std::string objname, float max_height,
                      float laplace_scale, int n_laplace_steps,
                      float simplification);

    void createOBJ(std::string filepath);

private:
    std::vector<cv::Point3f> m_vertices;
    std::vector<cv::Point3i> m_faces;
    std::vector< std::unordered_set<int> > G;

    void laplace_smooth(float scale);
    void simplify(std::string filename, std::string obj_name,
                  float simplification);
};

#endif
