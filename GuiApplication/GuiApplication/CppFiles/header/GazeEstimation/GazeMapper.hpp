//
//  GazeMapper.hpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-04-27.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#ifndef GazeMapper_hpp
#define GazeMapper_hpp

#include <iostream>
#include <vector>
#include <Eigen/Sparse>
#include "Profile.hpp"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace Eigen;

class GazeMapper {
public:
    vector<Profile> leftProfiles;
    vector<Profile> rightProfiles;
    vector<Profile> averageProfiles;
    
    Profile currentLeftProfile, currentRightProfile, currentAverageProfile;
    
    GazeMapper();
    GazeMapper(string folder);
    
    int predict(cv::Vec3f headPose, cv::Rect headSize, cv::Point leftPupilPosition, cv::Point rightPupilPosition);
    
    int preciseLocation = 0;
    
private:
    int prevLeftPupil = -1;
    int prevRightPupil = -1;
    
    int getPreciseLocation(int err, Profile profile, int prediction);
};

#endif /* GazeMapper_hpp */
