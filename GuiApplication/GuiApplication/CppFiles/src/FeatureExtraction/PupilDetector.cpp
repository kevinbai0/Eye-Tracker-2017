//
//  PupilDetector.cpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-03-18.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#include "../../header/FeatureExtraction/PupilDetector.hpp"

PupilDetector::PupilDetector() { }

void PupilDetector::initEyeRegions(vector<cv::Point> leftPoints, vector<cv::Point> rightPoints, int frameWidth, int frameHeight, int upsampleRate) {
    this->leftEyeRegion = createRegionFromPoints(leftPoints, frameWidth, frameHeight, upsampleRate);
    this->rightEyeRegion = createRegionFromPoints(rightPoints, frameWidth, frameHeight, upsampleRate);
}

cv::Rect PupilDetector::createRegionFromPoints(vector<cv::Point> points, int frameWidth, int frameHeight, int upsampleRate) {
    //size of points must = 6
    if (points.size() != 6) return cv::Rect();
    
    int minX = 100000, maxX = 0, minY = 100000, maxY = 0;
    for (int i = 0; i < 6; i++) {
        if (points[i].x < minX) minX = points[i].x;
        if (points[i].x > maxX) maxX = points[i].x;
        if (points[i].y < minY) minY = points[i].y;
        if (points[i].y > maxY) maxY = points[i].y;
    }
    
    cv::Rect returnRect = cv::Rect(minX * upsampleRate, minY * upsampleRate, (maxX - minX) * upsampleRate, (maxY - minY) * upsampleRate);
    fitToFrame(&returnRect, frameWidth * upsampleRate, frameHeight * upsampleRate);
    
    return returnRect;
}

void PupilDetector::findPupilCoordinates(cv::Mat &grayFrame, cv::Mat &original, cv::Mat &drawableFrame) {
    //first find rectangle
    if (leftEyeRegion.width > 0 && leftEyeRegion.height > 0) {
        if (!shiftedRect(prevLeftRegion, leftEyeRegion, 4)) leftEyeRegion = prevLeftRegion;
        else prevLeftRegion = leftEyeRegion;
        leftEye = grayFrame(leftEyeRegion);
        
        leftEyeEllipse = detectPupilCenter(leftEye, leftEyeRegion);
    }
    if (rightEyeRegion.width > 0 && rightEyeRegion.width > 0) {
        if (!shiftedRect(prevRightRegion, rightEyeRegion, 4)) rightEyeRegion = prevRightRegion;
        else prevRightRegion = rightEyeRegion;
        
        rightEye = grayFrame(rightEyeRegion);
        rightEyeEllipse = detectPupilCenter(rightEye, rightEyeRegion);
    }
    
    circle(drawableFrame, leftEyeEllipse.center, 3, cv::Scalar(255,0,0), -1);
    cvtColor(leftEye, leftEye, CV_GRAY2BGR);
    
    circle(drawableFrame, rightEyeEllipse.center, 3, cv::Scalar(0,0,255), -1);
    cvtColor(rightEye, rightEye, CV_GRAY2BGR);
}

cv::Mat plotMat = cv::Mat::zeros(200, 500, CV_8UC3);
int prevY1 = 0;
int prevY2 = 0;
int currentX = 0;

Ellipse PupilDetector::detectPupilCenter(cv::Mat &frame, cv::Rect superFrame) {
    //morphological operation: dilate
    cv::Mat frameCopy;
    cv::dilate(frame, frameCopy, structElem);
    
    int rows = frameCopy.rows;
    int cols = frameCopy.cols;
    
    cv::Mat filtered = cv::Mat::zeros(rows, cols, frameCopy.type());
    
    //determine colour threshold
    int max = 0, min = 256;
    int average = 0;
    for (int i = 0; i < rows; i++) {
        uchar *row = frameCopy.ptr(i);
        for (int j = 0; j < cols; j++) {
            if ((int) row[j] < min) min = (int) row[j];
            if ((int) row[j] > max) max = (int) row[j];
            average += (int) row[j];
        }
    }
    //average
    average /= (double) rows * cols;
    //set threshold = threshold * 0.75 as the threshold value
    //cout << "mean: " << threshold << " min: " << min << " max: " << max << " range: " << max - min << "\n";
    int threshold = average * 0.75;
    if (threshold - min <= 0) {
        threshold = average;
    }
    if (threshold - min <= 15) {
        threshold += 15;
    }
    if (threshold - min >= 40) {
        threshold -= 15;
    }
    cv::Point averagePoint = cv::Point(0,0);
    int c = 0;
    //calculate horizontal histogram
    int bin[rows];
    for (int i = 0; i < rows; i++) bin[i] = 0;
    //filter out all colour that exceeds the threshold amount
    for (int i = 0; i < rows; i++) {
        uchar *row = frameCopy.ptr(i);
        uchar *newRow = filtered.ptr(i);
        for (int j = 0; j < cols; j++) {
            if (row[j] > threshold) newRow[j] = 255;
            else {
                newRow[j] = 0;
                averagePoint.x += j; averagePoint.y += i;
                bin[i]++;
                c++;
            }
        }
    }
    
    if (c > 0) {
        averagePoint.x /= c;
        averagePoint.y /= c;
    }
    
    cv::Rect pupilRegion = findPupilInBlob(filtered);
    
    Ellipse pupilEllipse(cv::Point(pupilRegion.x + pupilRegion.width / 2, pupilRegion.y + pupilRegion.height / 2), cv::Size(10,10), 0);
    
    int largestIndex = getLargestIndex(bin, rows);
    
    float percentBlack = 100 * (float) c / ((float) rows * cols);
    float biggestLine = c > 0 ? 100 * (float) bin[largestIndex] / (float) c : 0;
    
    if (currentX == 500) {
        //shift
        plotMat = cv::Mat::zeros(200, 500, CV_8UC4);
        currentX = 0;
    }

    if (currentX > 0) {
        cv::line(plotMat, cv::Point(currentX, percentBlack), cv::Point(currentX - 1, prevY1), cv::Scalar(0,0,255));
        cv::line(plotMat, cv::Point(currentX, biggestLine), cv::Point(currentX - 1, prevY2), cv::Scalar(0,255,0));
    }
    prevY1 = percentBlack;
    prevY2 = biggestLine;
    
    currentX += 1;
    cvtColor(filtered, filtered, CV_GRAY2BGR);
    cv::rectangle(filtered, pupilRegion, cv::Scalar(0,0,255));
    
    filtered.release();
    frameCopy.release();
    
    float ratioX = (float) pupilEllipse.center.x / (float) superFrame.width;
    pupilEllipse.center.x = ratioX * 200;

    return pupilEllipse;
}

cv::Rect PupilDetector::findPupilInBlob(cv::Mat &frame) {
    cv::Rect pupilRegion;
    float max = 0;
    for (int i = 0; i < frame.cols / 2; i++) {
        cv::Rect region(i, 0, frame.cols / 2, frame.rows);
        cv::Mat cropped = frame(region);
        float percent = percentageBlack(cropped);
        if (percent > max) {
            pupilRegion = region;
            max = percent;
        }
    }
    return pupilRegion;
}

float PupilDetector::percentageBlack(cv::Mat &img) {
    int rows = img.rows;
    int cols = img.cols;
    float total = 0;
    cv::Point c(cols / 2, rows / 2);
    float r = pow(floor(min(rows, cols) / 2), 2);
    for (int i = 0; i < rows; i++) {
        uchar *row = img.ptr(i);
        for (int j = 0; j < cols; j++) {
            float distance = pow(c.x - j, 2) + pow(c.y - i, 2);
            if (row[j] == 0 && distance < r) total++;
        }
    }
    
    return total / (float) (rows * cols);
}

int PupilDetector::cycleCenters(cv::Mat &region, cv::Point *center, cv::Rect superFrame) {
    *center = cv::Point(region.cols / 2 + superFrame.x, region.rows / 2);
    return fitCircle(region, cv::Point(region.cols / 2, region.rows / 2));
}


int PupilDetector::fitCircle(cv::Mat &region, cv::Point center) {
    int rows = region.rows;
    int cols = region.cols;
    vector<float> numBlackPixelsPerRadii;
    vector<float> totalNumPixelsPerRadii;
    
    int maxRadii = max(rows, cols);
    
    for (int i = 0; i < maxRadii; i++) {
        numBlackPixelsPerRadii.push_back(0);
        totalNumPixelsPerRadii.push_back(0);
    }
    for (int i = 0; i < rows; i++) {
        uchar *row = region.ptr(i);
        for (int j = 0; j < cols; j++) {
            //calc distance
            int distance = (int) ceil(sqrt(pow(center.x - j, 2) + pow(center.y - i, 2)));
            //if (distance >= maxRadii) cout << distance << " " << maxRadii << "\n";
            if (row[j] == 0) numBlackPixelsPerRadii[distance]++;
            totalNumPixelsPerRadii[distance]++;
        }
    }
    bool lackingPrevLayer = false;
    for (int i = 0; i < maxRadii; i++) {
        float layerPercentage = numBlackPixelsPerRadii[i] / totalNumPixelsPerRadii[i];
        
        if (lackingPrevLayer && layerPercentage >= 0.9) lackingPrevLayer = false;

        if (layerPercentage < 0.9 || totalNumPixelsPerRadii[i] == 0) {
            if (!lackingPrevLayer) lackingPrevLayer = true;
            else return i;
        }
    }
    
    return maxRadii;
}

