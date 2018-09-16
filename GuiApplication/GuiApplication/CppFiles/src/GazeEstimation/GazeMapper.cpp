//
//  GazeMapper.cpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-04-27.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#include "../../header/GazeEstimation/GazeMapper.hpp"

GazeMapper::GazeMapper() { }
GazeMapper::GazeMapper(string folder) {
    for (int i = 0; i < 15; i++) {
        leftProfiles.push_back(Profile(folder + "finalLeftData/data" + to_string(i) + ".txt"));
        rightProfiles.push_back(Profile(folder + "finalRightData/data" + to_string(i) + ".txt"));
        Profile average(leftProfiles[i].headYaw, leftProfiles[i].headHeight);
        
        for (int j = 0; j < 5; j++) average.gridRanges.push_back(Range(1000, 0));
        
        for (int j = 0; j < leftProfiles[0].gridRanges.size() * leftProfiles[0].gridRanges[0].values.size(); j++) {
            //adjust the range
            //append the average
            int gridIndex = j / leftProfiles[0].gridRanges[0].values.size();
            average.gridRanges[gridIndex].append((leftProfiles[i].gridRanges[gridIndex].values[j] + rightProfiles[i].gridRanges[gridIndex].values[j]) / 2);
        }
        averageProfiles.push_back(average);
    }
}

int GazeMapper::predict(cv::Vec3f headPose, cv::Rect headSize, cv::Point leftPupilPosition, cv::Point rightPupilPosition) {
    //match a corresponding profile for head rotation
    int profileRotation = round(headPose[1] / 10) * 10;
    if (profileRotation > 20) profileRotation = 20;
    if (profileRotation < -20) profileRotation = -20;
    
    //match correspoinding profile for head height
    int profileHeight = 250; //height = 250
    if (headSize.height <= 175) profileHeight = 150;
    else if (headSize.height <= 225) profileHeight = 200;
    
    //pick current profile
    for (int i = 0; i < 15; i++) {
        if (leftProfiles[i].headYaw == profileRotation && leftProfiles[i].headHeight == profileHeight) {
            currentLeftProfile = leftProfiles[i];
            currentRightProfile = rightProfiles[i];
            currentAverageProfile = averageProfiles[i];
            break;
        }
    }
    
    int leftPos = leftPupilPosition.x;
    int rightPos = rightPupilPosition.x;
    
    //if there is a big shift than don't change the eye positino
    if (abs(rightPos - leftPos) > 30) {
        int rightEyeDiff = abs(rightPos - prevRightPupil);
        int leftEyeDiff = abs(leftPos - prevLeftPupil);
        if (rightEyeDiff > 30 && leftEyeDiff < 30) rightPos = prevRightPupil;
        else if (leftEyeDiff > 30 && rightEyeDiff < 30) leftPos = prevLeftPupil;
    }
    
    //disregard small changes
    if (abs(prevLeftPupil - leftPos) <= 4) leftPos = prevLeftPupil;
    else prevLeftPupil = leftPos;
    if (abs(prevRightPupil - rightPos) <= 4) rightPos = prevRightPupil;
    else prevRightPupil = rightPos;
    
    int averagePos = (double) (leftPos + rightPos) / 2;
    
    //make predictions
    vector<int> leftPossibleRanges, rightPossibleRanges, averagePossibleRanges;
    int leftPrediction = currentLeftProfile.predict(leftPos, leftPossibleRanges);
    int rightPrediction = currentRightProfile.predict(rightPos, rightPossibleRanges);
    int averagePrediction = currentAverageProfile.predict(averagePos, averagePossibleRanges);
    
    //if predictions are the same, then that is the correct prediction
    if (leftPrediction == rightPrediction) {
        //calculate precise location
        int err1 = leftPos - currentLeftProfile.gridRanges[leftPrediction].average;
        int err2 = rightPos - currentRightProfile.gridRanges[rightPrediction].average;
        if (abs(err1) < abs(err2)) preciseLocation = getPreciseLocation(err1, currentLeftProfile, leftPrediction);
        else preciseLocation = getPreciseLocation(err2, currentRightProfile, rightPrediction);

        return leftPrediction;
    }

    //out of the remaining ones with the same possible ranges, find the one with the least error
    int leastError = 1000, leastIndex = 0;
    bool leastErrorIsRight = false;
    for (int i = 0; i < leftPossibleRanges.size(); i++) {
        int leftError = leftPos - currentLeftProfile.gridRanges[leftPossibleRanges[i]].average;
        if (abs(leftError) < abs(leastError)) {
            leastError = leftError;
            leastIndex = leftPossibleRanges[i];
        }
    }
    for (int i = 0; i < rightPossibleRanges.size(); i++) {
        int rightError = rightPos - currentRightProfile.gridRanges[rightPossibleRanges[i]].average;
        if (abs(rightError) < abs(leastError)) {
            leastErrorIsRight = true;
            leastError = rightError;
            leastIndex = rightPossibleRanges[i];
        }
    }
    
    if (abs(averagePos - currentAverageProfile.gridRanges[averagePrediction].average) < leastError) {
        preciseLocation = getPreciseLocation(averagePos - currentAverageProfile.gridRanges[averagePrediction].average, currentAverageProfile, averagePrediction);
        
        return averagePrediction;
    }
    
    //precise location
    if (leastErrorIsRight) preciseLocation = getPreciseLocation(leastError, currentRightProfile, rightPrediction);
    else preciseLocation = getPreciseLocation(leastError, currentLeftProfile, leftPrediction);

    return leastIndex;
}

int GazeMapper::getPreciseLocation(int err, Profile profile, int prediction) {
    double gridDistance = 0;
    double screenLocation = prediction * 256 + 128;
    if (prediction == 0 && err < 0) gridDistance = profile.gridRanges[0].average;
    else if (prediction == 4 && err > 0) gridDistance = 200 - profile.gridRanges[4].average;
    else if (err < 0) gridDistance = profile.gridRanges[prediction].average - profile.gridRanges[prediction-1].average;
    else if (err > 0) gridDistance = profile.gridRanges[prediction+1].average - profile.gridRanges[prediction].average;
    else return screenLocation;
    
    return screenLocation + (err / gridDistance) * 256;
}
