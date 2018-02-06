//
//  FaceDetector.hpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-03-18.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#ifndef FaceDetector_hpp
#define FaceDetector_hpp

#include "../utils/UtilityFunctions.hpp"

class FaceDetector {
public:
    cv::CascadeClassifier classifier;
    bool detectFaceInFrame(cv::Mat &frame);
    bool testDetection(cv::Mat &frame);
    
    void setPointsToTrack(cv::Mat &frame, vector<cv::Point2f> &pointsToTrack);
    
    cv::Rect faceRegion;
    
    void setFaceDetectorPath(string filePath);
    
    FaceDetector();
private:
    int imageWidth = 640;
    int imageHeight = 360;
    
    cv::Rect prevFaceRegion;
    
    void skinDetection(cv::Mat &img);
    void segmentSkinRegion(cv::Mat &region, int a, int b, cv::Mat &dst);
    double getPercentageOfSkin(cv::Mat &src);
};

#endif /* FaceDetector_hpp */
