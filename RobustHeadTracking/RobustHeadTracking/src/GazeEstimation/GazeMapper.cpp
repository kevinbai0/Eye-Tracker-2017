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
        
        for (int j = 0; j < 50; j++) {
            //adjust the range
            //append the average
            average.gridRanges[j/10].append((leftProfiles[i].gridRanges[j/10].values[j] + rightProfiles[i].gridRanges[j/10].values[j]) / 2);
        }
        averageProfiles.push_back(average);
    }
}


int counter = 0;

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
    
    if (abs(prevLeftPupil - leftPos) <= 4) leftPos = prevLeftPupil;
    else prevLeftPupil = leftPos;
    if (abs(prevRightPupil - rightPos) <= 4) rightPos = prevRightPupil;
    else prevRightPupil = rightPos;
    
    int averagePos = (double) (leftPos + rightPos) / 2;
    
    vector<int> leftPossibleRanges, rightPossibleRanges, averagePossibleRanges;
    int leftPrediction = currentLeftProfile.predict(leftPos, leftPossibleRanges);
    int rightPrediction = currentRightProfile.predict(rightPos, rightPossibleRanges);
    int averagePrediction = currentAverageProfile.predict(averagePos, averagePossibleRanges);
    
    //if predictions are the same, then that is the correct prediction
    if (leftPrediction == rightPrediction) return leftPrediction;
    
    //however, if they share more than one possibility, account only for ranges that are both posssible
    for (int i = (int) leftPossibleRanges.size()-1; i >= 0; i--) {
        bool matches = false;
        for (int j = 0; j < rightPossibleRanges.size(); j++) {
            if (leftPossibleRanges[i] == rightPossibleRanges[j]) matches = true;
        }
        if (!matches) leftPossibleRanges.erase(leftPossibleRanges.begin() + i);
    }
    for (int i = (int) rightPossibleRanges.size()-1; i >= 0; i--) {
        bool matches = false;
        for (int j = 0; j < leftPossibleRanges.size(); j++) {
            if (rightPossibleRanges[i] == leftPossibleRanges[j]) matches = true;
        }
        if (!matches) rightPossibleRanges.erase(rightPossibleRanges.begin() + i);
    }
    
    //if there were no possible ranges that were the same then find the one with the least error in its original prediction
    if (leftPossibleRanges.size() == 0 && rightPossibleRanges.size() == 0) {
        double errorLeft = abs(currentLeftProfile.gridRanges[leftPrediction].average - leftPos);
        double errorRight = abs(currentRightProfile.gridRanges[rightPrediction].average - rightPos);
        double errorAverage = abs(currentAverageProfile.gridRanges[averagePrediction].average - averagePos);
        if (errorLeft > errorRight && errorAverage > errorRight) return rightPrediction;
        else if (errorRight > errorLeft && errorAverage > errorLeft) return leftPrediction;
        else return averagePrediction;
    }
    
    //out of the remaining ones with the same possible ranges, find the one with the least error
    int leastError = 1000, leastIndex = 0;
    for (int i = 0; i < leftPossibleRanges.size(); i++) {
        int leftError = abs(currentLeftProfile.gridRanges[leftPossibleRanges[i]].average - leftPos);
        int rightError = abs(currentRightProfile.gridRanges[leftPossibleRanges[i]].average - rightPos);
        if (leftError < leastError) {
            leastError = leftError;
            leastIndex = leftPossibleRanges[i];
        }
        if (rightError < leastError) {
            leastError = rightError;
            leastIndex = leftPossibleRanges[i];
        }
    }
    
    if (currentAverageProfile.gridRanges[averagePrediction].average < leastError)
        return averagePrediction;
    
    return leastIndex;
}
