//
//  HeadTracker.hpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-03-15.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#ifndef HeadTracker_hpp
#define HeadTracker_hpp

#include "FaceDetector.hpp"
#include "FlowTracker.hpp"
#include "PupilDetector.hpp"
#include "HeadPoseEstimator.hpp"
//dlib headers
#include <dlib/image_processing.h>
#include <dlib/opencv.h>


class HeadTracker {
public:
    dlib::shape_predictor poseModel;
    FaceDetector faceDetector;
    FlowTracker flowTracker;
    PupilDetector pupilDetector;
    HeadPoseEstimator headPoseEstimator;
    
    void trackHead(cv::Mat &src, cv::Mat &dst, cv::Rect *headSize, cv::Vec3f *headPose, cv::Point *leftPupilPoint, cv::Point *rightPupilPoint);
    void findFaceInStaticImage(cv::Mat image, cv::Mat &src, cv::Mat &dst, cv::Rect *headSize, cv::Vec3f *headPose, cv::Point *leftPupilPoint, cv::Point *rightPupilPoint);
    
    HeadTracker();
    HeadTracker(int downSampleRate, int refreshRate);
    //init with files
    HeadTracker(int downSampleRate, int refreshRate, string faceDetectorPath, string dlibShapePredictorPath, string modelFile, string mapFile, string blendshapesFile, string modelContourFile, string edgeTopologyFile);
    
private:
    int downSampleRate = 2;
    int refreshRate = 20; //default value
    
    //after face is found, process it
    void processFace(cv::Mat &original, cv::Mat &originalGray, cv::Mat &resized, cv::Mat &resizedGray, cv::Mat &drawableFrame, cv::Rect *headSize, cv::Vec3f *headPose, cv::Point *leftPupilPosition, cv::Point *rightPupilPosition); //after face is detected do this:
    
    //refresh counter
    int frameCounter = 0;
    
    //prevFrame for optical flow
    cv::Mat prevFrame;
    //points to track for optical flow
    vector<cv::Point2f> pointsToTrack;
    
    //landmark detection
    void facialLandmarkDetection(cv::Mat &img, cv::Mat &drawableFrame, cv::Rect face, vector<cv::Point> &detectedPoints);
};

#endif /* HeadTracker_hpp */
