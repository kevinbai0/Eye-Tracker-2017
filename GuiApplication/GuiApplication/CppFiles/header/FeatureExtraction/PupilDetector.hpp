//
//  PupilDetector.hpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-03-18.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#ifndef PupilDetector_hpp
#define PupilDetector_hpp

#include "../utils/UtilityFunctions.hpp"

class PupilDetector {
public:
    vector<cv::Point> leftEyeLandmarks; //starting from left corner
    vector<cv::Point> rightEyeLandmarks; //starting from left corner
    
    cv::Mat leftEye, rightEye;
    cv::Rect leftEyeRegion, rightEyeRegion;
    Ellipse leftEyeEllipse, rightEyeEllipse;
    
    void initEyeRegions(vector<cv::Point> leftPoints, vector<cv::Point> rightPoints, int frameWidth, int frameHeight, int upsampleRate);
    void findPupilCoordinates(cv::Mat &grayFrame, cv::Mat &original, cv::Mat &drawableFrame);
    
    PupilDetector();
private:
    //initializing algorithm
    cv::Rect createRegionFromPoints(vector<cv::Point> points, int frameWidth, int frameHeight, int upsampleRate);
    
    //struct element for dilate operation
    cv::Mat structElem = cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3,3));
    
    cv::Rect prevLeftRegion = cv::Rect();
    cv::Rect prevRightRegion = cv::Rect();
    cv::Point prevRightPoint = cv::Point(), prevLeftPoint = cv::Point();
    
    Ellipse detectPupilCenter(cv::Mat &frame, cv::Rect superFrame);
    //find pupil center in an image
    cv::Rect findPupilInBlob(cv::Mat &frame);
    float percentageBlack(cv::Mat &img);
    int cycleCenters(cv::Mat &region, cv::Point *center, cv::Rect superFrame);
    int fitCircle(cv::Mat &region, cv::Point center);
};


#endif /* PupilDetector_hpp */
