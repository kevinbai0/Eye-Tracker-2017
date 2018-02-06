//
//  FaceDetector.cpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-03-18.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#include "../../header/FeatureExtraction/FaceDetector.hpp"


FaceDetector::FaceDetector() {
}

void FaceDetector::setFaceDetectorPath(string filePath) {
    classifier.load(filePath);
}


bool FaceDetector::detectFaceInFrame(cv::Mat &frame) {
    vector<cv::Rect> faces;

    if (faceRegion.width == 0) {
        classifier.detectMultiScale(frame, faces);
        if (faces.size() > 0) {
            faceRegion = faces[0];
            scale(&faceRegion, 1.2, Ratio(3,4));
            fitToFrame(&faceRegion, frame.cols, frame.rows);
            
            prevFaceRegion = faceRegion;
            
            return true;
        }
        return false;
    }
    
    //first double the size of faceRegion
    cv::Rect searchRegion = faceRegion;
    scale(&searchRegion, 2, Ratio(1,1));
    fitToFrame(&searchRegion, frame.cols, frame.rows);
    classifier.detectMultiScale(frame(searchRegion), faces);
    
    if (faces.size() > 0) {
        faceRegion = faces[0];
        faceRegion.x += searchRegion.x;
        faceRegion.y += searchRegion.y;
        scale(&faceRegion, 1.2, Ratio(3,4));
        fitToFrame(&faceRegion, frame.cols, frame.rows);
        
        if (!shiftedRect(prevFaceRegion, faceRegion, 10)) faceRegion = prevFaceRegion;
        else prevFaceRegion = faceRegion;
        
        return true;
    }
    
    return false;
}

bool FaceDetector::testDetection(cv::Mat &frame) {
    cv::Mat region = frame(faceRegion);
    skinDetection(region);
    double percentSkin = getPercentageOfSkin(region);
    
    region.release();
    if (percentSkin > 0.15) {
        return true;
    }
    return false;
}

void FaceDetector::setPointsToTrack(cv::Mat &frame, vector<cv::Point2f> &pointsToTrack) {
    vector<cv::Point2f> points;
    
    cv::Mat mask = cv::Mat::zeros(frame.rows, frame.cols, CV_8UC1);
    
    for (int i = faceRegion.y; i < faceRegion.y + faceRegion.height; i++) {
        uchar* row = mask.ptr(i);
        for (int j = faceRegion.x; j < faceRegion.x + faceRegion.width; j++) {
            row[j] = (uchar) 255;
        }
    }
    
    goodFeaturesToTrack(frame, points, 500, 0.01, 5, mask);
    pointsToTrack = points;
    
    mask.release();
}

void FaceDetector::skinDetection(cv::Mat &img) {
    cv::Mat copy;
    blur(img, copy, cv::Size(3,3));
    cvtColor(copy, copy, CV_BGR2YCrCb);
    for (int i = 0; i < copy.rows; i++) {
        cv::Vec3b* pixels = img.ptr<cv::Vec3b>(i);
        for (int j = 0; j < img.cols; j++) {
            if (i == 0 || j == 0 || i == img.rows - 1 || j == img.cols - 1) {
                pixels[j] = cv::Vec3b(0,0,0);
            }
            else {
                cv::Mat copied = copy(cv::Rect(j-1,i-1,3,3));
                segmentSkinRegion(copied, i, j, img);
                copied.release();
            }
        }
    }
    copy.release();
}

void FaceDetector::segmentSkinRegion(cv::Mat &region, int a, int b, cv::Mat &dst) {
    int minY = 68, maxY = 237, minCb = 139, maxCb = 179, minCr = 77, maxCr = 123;
    
    int y = 0, cb = 0, cr = 0;
    for (int i = 0; i < 3; i++) {
        cv::Vec3b* pixel = region.ptr<cv::Vec3b>(i);
        for (int j = 0; j < 3; j++) {
            y += (int) pixel[j][0];
            cb += (int) pixel[j][1];
            cr += (int) pixel[j][2];
        }
    }
    y /= 9, cb /= 9, cr /= 9;
    cv::Vec3b color(0,0,0);
    //threshold
    if (y >= minY && y <= maxY && cb >= minCb && cb <= maxCb && cr >= minCr && cr <= maxCr) {
        color = cv::Vec3b(255,255,255);
    }
    for (int i = a-1; i < a + 2; i++) {
        cv::Vec3b* pixels = dst.ptr<cv::Vec3b>(i);
        for (int j = b-1; j < b + 2; j++) {
            pixels[j] = color;
        }
    }
}

double FaceDetector::getPercentageOfSkin(cv::Mat &src) {
    int rows = src.rows, cols = src.cols;
    double numWhite = 0;
    double numBlack = 0;
    typedef cv::Point3_<uint8_t> Pixel;
    for (int i = 0; i < rows; i++) {
        Pixel* ptr = src.ptr<Pixel>(0, i);
        const Pixel* ptr_end = ptr + cols;
        for (; ptr != ptr_end; ++ptr) {
            if (ptr->x == 255) numWhite++;
            else numBlack++;
        }
    }
    return numWhite / (numWhite + numBlack);
}
