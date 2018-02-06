//
//  HeadPoseEstimator.cpp
//  HeadModel3D
//
//  Created by Kevin Bai on 2017-03-23.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//
#include "../utils/UtilityFunctions.hpp"

#include <eos/fitting/fitting.hpp>
#include <eos/render/texture_extraction.hpp>

using namespace eos;

using eos::core::Landmark;
using eos::core::LandmarkCollection;

class HeadPoseEstimator {
public:
    cv::Vec3f getHeadPose(cv::Mat &img, vector<cv::Point> facialLandmarks);
    void drawWireframe(cv::Mat &src, cv::Mat &dst);
    
    void loadFiles(string modelFile, string mapFile, string blendshapesFile, string modelContourFile, string edgeTopologyFile);
    
    HeadPoseEstimator();
private:
    morphablemodel::MorphableModel morphModel;
    eos::core::LandmarkMapper landmarkMapper;
    vector<morphablemodel::Blendshape> blendShapes;
    fitting::ModelContour modelContour;
    fitting::ContourLandmarks contourLandmarks;
    morphablemodel::EdgeTopology edgeTopology;

    eos::core::Mesh mesh;
    fitting::RenderingParameters renderingParams;
    vector<cv::Vec2f> imagePoints;
    vector<cv::Vec4f> modelPoints;
    vector<int> vertexIndices;
};
