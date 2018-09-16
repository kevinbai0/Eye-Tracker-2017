//
//  main.cpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-03-15.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#include "header/FeatureExtraction/HeadTracker.hpp"
#include "header/utils/NeuralNetwork.hpp"
#include "header/GazeEstimation/GazeMapper.hpp"

using namespace cv;

int main(int argc, const char * argv[]) {
    cv::namedWindow("Window");
    cv::namedWindow("Left eye");
    cv::namedWindow("Right eye");
    cv::moveWindow("Right eye", 200, 20);
    // insert code here...
    HeadTracker tracker(2, 20,
                        "LearnedData/lbpcascade.xml",
                        "LearnedData/shape_predictor_68_face_landmarks.dat",
                        "HeadModelData/sfm_shape_3448.bin",
                        "HeadModelData/sfm_landmarks.txt",
                        "HeadModelData/expression_blendshapes_3448.bin",
                        "HeadModelData/model_contours.json",
                        "HeadModelData/sfm_3448_edge_topology.json");
    
    
    GazeMapper gazeMapper("calib2/");
    
    VideoCapture cap(0);
    while(cv::waitKey(1) != 27) {
        cv::Mat image;
        cap >> image;
        u_long n = now();
        cv::Vec3f cvHeadPose;
        cv::Rect cvHeadSize;
        cv::Point cvLeftPupilPosition;
        cv::Point cvRightPupilPosition;
        
        tracker.trackHead(image, image,&cvHeadSize, &cvHeadPose, &cvLeftPupilPosition, &cvRightPupilPosition);
        
        double prediction = gazeMapper.predict(cvHeadPose, cvHeadSize, cvLeftPupilPosition, cvRightPupilPosition);
        
        cout << prediction << "\n";
        
        //cout << "Time: " << (now() - fpsTime) << "ms\n";
        imshow("Window", image);
    }
}
