//
//  HeadPoseEstimator.cpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-03-24.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#include "../../header/FeatureExtraction/HeadPoseEstimator.hpp"


HeadPoseEstimator::HeadPoseEstimator() { }

void HeadPoseEstimator::loadFiles(string modelFile, string mapFile, string blendshapesFile, string modelContourFile, string edgeTopologyFile) {
    this->morphModel = morphablemodel::load_model(modelFile);
    this->landmarkMapper = eos::core::LandmarkMapper(mapFile);
    this->blendShapes = morphablemodel::load_blendshapes(blendshapesFile);
    this->modelContour = fitting::ModelContour::load(modelContourFile);
    this->contourLandmarks = fitting::ContourLandmarks::load(mapFile);
    this->edgeTopology = morphablemodel::load_edge_topology(edgeTopologyFile);
}

cv::Vec3f HeadPoseEstimator::getHeadPose(cv::Mat &img, vector<cv::Point> facialLandmarks) {
    //convert vector<Point> to landmark collection
    modelPoints = vector<cv::Vec4f>();
    imagePoints = vector<cv::Vec2f>();
    vertexIndices = vector<int>();
    for (int i = 0; i < 68; i++) {
        auto converted = landmarkMapper.convert(to_string(i));
        if (!converted) continue;
        auto vertex = morphModel.get_shape_model().get_mean_at_point(stoi(converted.get()));
        modelPoints.emplace_back(cv::Vec4f(vertex.x(), vertex.y(), vertex.z(), 1.0f));
        vertexIndices.emplace_back(stoi(converted.get()));
        imagePoints.emplace_back(cv::Vec2f(facialLandmarks[i].x, facialLandmarks[i].y));
    }
    
    fitting::ScaledOrthoProjectionParameters pose = fitting::estimate_orthographic_projection_linear(imagePoints, modelPoints, true, img.rows);
    renderingParams = fitting::RenderingParameters(pose, img.cols, img.rows);
    
    // Estimate the shape coefficients by fitting the shape to the landmarks:
    cv::Mat affine_from_ortho = fitting::get_3x4_affine_camera_matrix(renderingParams, img.cols, img.rows);
    vector<float> fitted_coeffs = fitting::fit_shape_to_landmarks_linear(morphModel, affine_from_ortho, imagePoints, vertexIndices);
    
    // Obtain the full mesh with the estimated coefficients:
    mesh = morphModel.draw_sample(fitted_coeffs, vector<float>());
    
    float roll = glm::degrees(glm::roll(renderingParams.get_rotation()));
    float pitch = glm::degrees(glm::pitch(renderingParams.get_rotation()));
    float yaw = glm::degrees(glm::yaw(renderingParams.get_rotation()));
    
    return cv::Vec3f(pitch, yaw, roll);
}
void HeadPoseEstimator::drawWireframe(cv::Mat &src, cv::Mat &dst) {
    glm::mat4x4 modelview = renderingParams.get_modelview();
    glm::mat4x4 projection = renderingParams.get_projection();
    glm::vec4 viewport = fitting::get_opencv_viewport(src.cols, src.rows);
    cv::Scalar colour = cv::Scalar(80,129,255);

    for (const auto& triangle : mesh.tvi) {
        const auto p1 = glm::project({ mesh.vertices[triangle[0]][0], mesh.vertices[triangle[0]][1], mesh.vertices[triangle[0]][2] }, modelview, projection, viewport);
        const auto p2 = glm::project({ mesh.vertices[triangle[1]][0], mesh.vertices[triangle[1]][1], mesh.vertices[triangle[1]][2] }, modelview, projection, viewport);
        const auto p3 = glm::project({ mesh.vertices[triangle[2]][0], mesh.vertices[triangle[2]][1], mesh.vertices[triangle[2]][2] }, modelview, projection, viewport);
        if (render::detail::are_vertices_ccw_in_screen_space(glm::vec2(p1), glm::vec2(p2), glm::vec2(p3))) {
            cv::line(dst, cv::Point(p1.x, p1.y), cv::Point(p2.x, p2.y), colour);
            cv::line(dst, cv::Point(p2.x, p2.y), cv::Point(p3.x, p3.y), colour);
            cv::line(dst, cv::Point(p3.x, p3.y), cv::Point(p1.x, p1.y), colour);
        }
    }
}
