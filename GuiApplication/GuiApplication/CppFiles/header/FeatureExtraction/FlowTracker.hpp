//
//  FlowTracker.hpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-03-18.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#ifndef FlowTracker_hpp
#define FlowTracker_hpp

#include "../utils/UtilityFunctions.hpp"

class FlowTracker {
public:
    void findFaceWithOpticalFlow(cv::Mat &prevFrame, cv::Mat &currentFrame, vector<cv::Point2f> &initialPoints);
    cv::Rect updateFaceSize(cv::Rect prevFace, int frameWidth, int frameHeight, vector<cv::Point2f> pointsToTrack);

    FlowTracker();
};

#endif /* FlowTracker_hpp */
