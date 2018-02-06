//
//  EyeTracker.m
//  GuiApplication
//
//  Created by Kevin Bai on 2017-03-26.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#import "CppFiles/header/FeatureExtraction/HeadTracker.hpp"
#import "CppFiles/header/GazeEstimation/GazeMapper.hpp"
#import "NSImage+Mat.h"
#import "EyeTracker.h"

@implementation EyeTracker

HeadTracker tracker(2, 20,
                    "LearnedData/lbpcascade.xml",
                    "LearnedData/shape_predictor_68_face_landmarks.dat",
                    "HeadModelData/sfm_shape_3448.bin",
                    "HeadModelData/sfm_landmarks.txt",
                    "HeadModelData/expression_blendshapes_3448.bin",
                    "HeadModelData/model_contours.json",
                    "HeadModelData/sfm_3448_edge_topology.json");

GazeMapper gazeMapper("calib2/");

int i = 0;
int j = 0;

- (void) track: (NSImage *) nsImage finalWidth: (int) finalWidth {
    // create the cv mat
    @autoreleasepool {
        _firstImage = true;
        
        cv::Mat image = [nsImage CVMat]; /*cv::imread("/users/kbai/desktop/data/" + to_string(i) + " " + to_string(j) + ".png");
        j++;
        if (j == 100) {
            i++;
            j = 0;
        }*/
        u_long n = now();
        cvtColor(image, image, CV_BGR2RGB);
        cv::Vec3f cvHeadPose;
        cv::Rect cvHeadSize;
        cv::Point cvLeftPupilPosition;
        cv::Point cvRightPupilPosition;
        
        tracker.trackHead(image, image, &cvHeadSize, &cvHeadPose, &cvLeftPupilPosition, &cvRightPupilPosition);
        if (_currentHeadPose != nil) _firstImage = false;
        
        _currentHeadSize = CGRectMake(cvHeadSize.x, cvHeadSize.y, cvHeadSize.width, cvHeadSize.height);
        _currentHeadPose = @[[NSNumber numberWithFloat: cvHeadPose[0]], [NSNumber numberWithFloat: cvHeadPose[1]], [NSNumber numberWithFloat: cvHeadPose[2]]];
        
        _prevLeftPupilPoint = _currentLeftPupilPoint;
        _prevRightPupilPoint = _currentRightPupilPoint;
        
        _currentLeftPupilPoint = CGPointMake(cvLeftPupilPosition.x, cvLeftPupilPosition.y);
        _currentRightPupilPoint = CGPointMake(cvRightPupilPosition.x, cvRightPupilPosition.y);
        _currentAveragePoint = CGPointMake((cvLeftPupilPosition.x + cvRightPupilPosition.x) / 2.0, (cvLeftPupilPosition.y + cvRightPupilPosition.y) / 2.0);
        
        int newHeight = finalWidth / 1.7777;
        
        _currentPrediction = gazeMapper.predict(cvHeadPose, cvHeadSize, cvLeftPupilPosition, cvRightPupilPosition);
        _preciseLocation = gazeMapper.preciseLocation;
        
        for (int i = 0; i < 5; i++) {
            Range rL = gazeMapper.currentLeftProfile.gridRanges[i];
            cv::rectangle(image, cv::Point(rL.min, 10 * (i+1)), cv::Point(rL.max, 10 * (i+1)+5), cv::Scalar(255,0,0), -1);
            Range rR = gazeMapper.currentRightProfile.gridRanges[i];
            cv::rectangle(image, cv::Point(rR.min, 10 * (i+1)+50), cv::Point(rR.max, 10 * (i+1)+55), cv::Scalar(0,0,255), -1);
        }
        
        resize(image, image, cv::Size(finalWidth, newHeight), CV_INTER_CUBIC);

        _currentImage = nil;
        _currentImage = [NSImage imageWithCVMat:image];
        
        image.release();
    }
}

@end
