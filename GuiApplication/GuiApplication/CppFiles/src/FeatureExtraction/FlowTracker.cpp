//
//  FlowTracker.cpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-03-18.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#include "../../header/FeatureExtraction/FlowTracker.hpp"

FlowTracker::FlowTracker() {}

void FlowTracker::findFaceWithOpticalFlow(cv::Mat &prevFrame, cv::Mat &currentFrame, vector<cv::Point2f> &initialPoints) {
    cv::Mat status;
    vector<float> err;
    vector<cv::Point2f> newPointsToTrack;
    
    calcOpticalFlowPyrLK(prevFrame, currentFrame, initialPoints, newPointsToTrack, status, err);
    status.release();
    
    vector<cv::Point2f> pointsToKeep;
    
    //remove points with too much error
    for (int i = 0; i < newPointsToTrack.size(); i++) {
        if (err[i] <= 20) pointsToKeep.push_back(newPointsToTrack[i]);
    }
    
    initialPoints = pointsToKeep;
}

cv::Rect FlowTracker::updateFaceSize(cv::Rect prevFace, int frameWidth, int frameHeight, vector<cv::Point2f> pointsToTrack) {
    //remove any points that are not on the screen
    cv::Point2f averagePoint(0,0);
    float minX = 1000, maxX = 0, minY = 1000, maxY = 0;
    for (int i = 0 ; i < pointsToTrack.size(); i++) {
        //remove any points that are off the screens
        float x = pointsToTrack[i].x, y = pointsToTrack[i].y;
        if (x < 0 || x > frameWidth || y < 0 || y > frameHeight) {
            pointsToTrack.erase(pointsToTrack.begin() + i);
            i--;
        }
        //find average point by finding sum of all points first:
        averagePoint.x += x;
        averagePoint.y += y;
        //find min and max points
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    }
    //find average point
    averagePoint.x /= pointsToTrack.size();
    averagePoint.y /= pointsToTrack.size();
    
    cv::Point c = averagePoint;
    //find distance of center point to each of the min and max values
    float leftDiff = c.x - minX;
    float rightDiff = maxX - c.x;
    float topDiff = c.y - minY;
    float botDiff = maxY - c.y;
    
    /*
     the size of the frame will not differ by more than n
     pixels in width and height between 2 consecutive frames
     Below - find the new face boundaries
     */
    int borderWidth = 0, borderHeight = 0;;
    if (abs(leftDiff + rightDiff - prevFace.width) < 20)  borderWidth = leftDiff + rightDiff;
    if (abs(topDiff + botDiff - prevFace.height) < 20)  borderHeight = topDiff + botDiff;
    
    if (borderHeight == 0 && borderWidth == 0) {
        //if the border size = 0, then keep the size the same as before
        borderHeight = prevFace.height;
        borderWidth = prevFace.height * 3.0 / 4.0;
        prevFace.x = c.x - borderWidth / 2;
        prevFace.y = c.y - borderHeight / 2;
    }
    else if (borderHeight == 0) {
        borderHeight = borderWidth * 4.0 / 3.0;
        prevFace.x = c.x - borderWidth / 2;
        prevFace.y = c.y - borderHeight / 2;
    }
    else if (borderWidth == 0) {
        borderWidth = borderHeight * 3.0 / 4.0;
        prevFace.x = c.x - borderWidth / 2;
        prevFace.y = c.y - borderHeight / 2;
    }
    else { //if border size is found simply make the border that size with average point as center
        prevFace.width = borderWidth;
        prevFace.height = borderHeight;
        prevFace.x = c.x - borderWidth / 2;
        prevFace.y = c.y - borderHeight / 2;
    }

    fitToFrame(&prevFace, frameWidth, frameHeight);
    return prevFace;
}
