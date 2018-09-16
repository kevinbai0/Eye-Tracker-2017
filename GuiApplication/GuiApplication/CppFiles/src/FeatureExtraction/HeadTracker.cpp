//
//  HeadTracker.cpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-03-15.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#include "../../header/FeatureExtraction/HeadTracker.hpp"



HeadTracker::HeadTracker() {
}
HeadTracker::HeadTracker(int downSampleRate, int refreshRate) {
    this->downSampleRate = downSampleRate;
    this->refreshRate = refreshRate;
}

HeadTracker::HeadTracker(int downSampleRate, int refreshRate, string faceDetectorPath, string dlibShapePredictorPath, string modelFile, string mapFile, string blendshapesFile, string modelContourFile, string edgeTopologyFile) {
    this->downSampleRate = downSampleRate;
    this->refreshRate = refreshRate;
    
    faceDetector.setFaceDetectorPath(faceDetectorPath);
    dlib::deserialize(dlibShapePredictorPath) >> poseModel;
    headPoseEstimator.loadFiles(modelFile, mapFile, blendshapesFile, modelContourFile, edgeTopologyFile);
}
//face tracking and detection
void HeadTracker::trackHead(cv::Mat &src, cv::Mat &dst, cv::Rect *headSize, cv::Vec3f *headPose, cv::Point *leftPupilPoint, cv::Point *rightPupilPoint) {
    flip(src, src, 1);
    cv::Mat grayFrame, resized, resizedGray, drawableFrame;
    resize(src, resized, cv::Size(src.cols / downSampleRate, src.rows / downSampleRate), CV_INTER_CUBIC);
    resized.copyTo(drawableFrame);
    cvtColor(src, grayFrame, CV_BGR2GRAY);
    cvtColor(resized, resizedGray, CV_BGR2GRAY);
    
    //try to find face with classifier
    //scale down image
    if (faceDetector.detectFaceInFrame(resizedGray)) {
        //update points to track in detected face region
        faceDetector.setPointsToTrack(resizedGray, pointsToTrack);
        
        //process face information
        processFace(src, grayFrame, resized, resizedGray, drawableFrame, headSize, headPose, leftPupilPoint, rightPupilPoint);
        //reset frame counter
        frameCounter = 0;
        
        src.release();
        grayFrame.release();
        resized.release();
        resizedGray.release();
        dst = drawableFrame;
        return;
    }
    
    //if there are no points (for optical flow), then return the frame
    if (pointsToTrack.size() == 0) {
        src.release();
        grayFrame.release();
        resizedGray.release();
        drawableFrame.release();
        dst = resized;
        return;
    }
    
    //if can't find face then use optical flow to track the face
    flowTracker.findFaceWithOpticalFlow(prevFrame, resizedGray, pointsToTrack);
    
    faceDetector.faceRegion = flowTracker.updateFaceSize(faceDetector.faceRegion, resizedGray.cols, resizedGray.rows, pointsToTrack);
    
    //process face information
    processFace(src, grayFrame, resized, resizedGray, drawableFrame, headSize, headPose, leftPupilPoint, rightPupilPoint);
    //every n frames check to make sure that the face region is fully detected
    frameCounter++;
    if (frameCounter >= refreshRate) {
        if (!faceDetector.testDetection(resized)) {
            faceDetector.faceRegion = cv::Rect();
            pointsToTrack = vector<cv::Point2f>();
            frameCounter = 0;
        }
    }
    
    src.release();
    grayFrame.release();
    resized.release();
    resizedGray.release();
    
    dst = drawableFrame;
}

void HeadTracker::findFaceInStaticImage(cv::Mat image, cv::Mat &src, cv::Mat &dst, cv::Rect *headSize, cv::Vec3f *headPose, cv::Point *leftPupilPoint, cv::Point *rightPupilPoint) {
    resize(image, image, cv::Size(image.rows / downSampleRate, image.cols / downSampleRate), CV_INTER_CUBIC);
    cv::Mat original = image.clone(), drawableFrame = image.clone();
    cvtColor(image, image, CV_BGR2GRAY);
    
    //try to find face with classifier
    //scale down image
    faceDetector.detectFaceInFrame(image);
    
    //process face information
    //processFace(src, grayFrame, resized, resizedGray, drawableFrame, headSize, headPose, leftPupilPoint, rightPupilPoint);
    //reset frame counter
    frameCounter = 0;
    
    dst = drawableFrame;
}

void HeadTracker::processFace(cv::Mat &original, cv::Mat &originalGray, cv::Mat &resized, cv::Mat &resizedGray, cv::Mat &drawableFrame, cv::Rect *headSize, cv::Vec3f *headPose, cv::Point *leftPupilPosition, cv::Point *rightPupilPosition) {
    prevFrame = resizedGray;
    
    rectangle(drawableFrame, faceDetector.faceRegion, cv::Scalar(0,0,255));

    for (int i = 0; i < pointsToTrack.size(); i++) {
        //circle(drawableFrame, pointsToTrack[i], 1, cv::Scalar(0,255,0), -1);
    }
    
    vector<cv::Point> landmarkPoints;
    facialLandmarkDetection(resized, drawableFrame, faceDetector.faceRegion, landmarkPoints);

    vector<cv::Point> leftEye, rightEye;
    for (int i = 36; i < 42; i++) leftEye.push_back(landmarkPoints[i]);
    for (int i = 42; i < 48; i++) rightEye.push_back(landmarkPoints[i]);
    
    pupilDetector.initEyeRegions(leftEye, rightEye, resized.cols, resized.rows, downSampleRate);
    pupilDetector.findPupilCoordinates(originalGray, original, drawableFrame);
    
    *headSize = faceDetector.faceRegion;
    //calculate head pose
    *headPose = headPoseEstimator.getHeadPose(resized, landmarkPoints);
    //headPoseEstimator.drawWireframe(resized, drawableFrame);
    *leftPupilPosition = cv::Point(pupilDetector.leftEyeEllipse.center.x, pupilDetector.leftEyeEllipse.center.y);
    *rightPupilPosition = cv::Point(pupilDetector.rightEyeEllipse.center.x, pupilDetector.rightEyeEllipse.center.y);
}

//dlib functions

void HeadTracker::facialLandmarkDetection(cv::Mat &img, cv::Mat &drawableFrame, cv::Rect face, vector<cv::Point> &landmarkPoints) {
    dlib::cv_image<dlib::bgr_pixel> dlibImage(img);
    dlib::rectangle dlibRect(face.x, face.y, face.x + face.width, face.y + face.height);
    dlib::full_object_detection shape = poseModel(dlibImage, dlibRect);
    
    DLIB_CASSERT
    (
     shape.num_parts() == 68,
     "\n\t Invalid inputs were given to this function. "
     << "\n\t d.num_parts():  " << shape.num_parts()
     );
    landmarkPoints = vector<cv::Point>();
    for (int i = 0; i < shape.num_parts(); i++) {
        landmarkPoints.push_back(cv::Point((int) shape.part(i).x(), (int) shape.part(i).y()));
        circle(drawableFrame, landmarkPoints[i], 1, cv::Scalar(0,0,255), -1);
    }
}
