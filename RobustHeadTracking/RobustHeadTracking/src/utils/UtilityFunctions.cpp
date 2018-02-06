//
//  UtilityFunctions.cpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-03-19.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#include "../../header/utils/UtilityFunctions.hpp"

Ratio::Ratio(double width, double height) {
    this->width = width;
    this->height = height;
}

u_long now() {
    u_long milliseconds_since_epoch =
    std::chrono::system_clock::now().time_since_epoch() /
    std::chrono::milliseconds(1);
    
    return milliseconds_since_epoch;
}

void scale(cv::Rect *rect, int multiplier, Ratio ratio) {
    double widthRatio = ratio.width;
    double heightRatio = ratio.height;
    
    int scaleWidth = rect->width * (multiplier - 1.0);
    int scaleHeight = rect->height * (multiplier - 1.0);
    
    //make aspect ratio widthRatio : heightRatio
    int newHeight = (double) rect->height * heightRatio / widthRatio;
    rect->y -= (newHeight - rect->height) / 2.0;
    rect->height = newHeight;
    
    //scale
    rect->x -= scaleWidth / 2;
    rect->y -= scaleHeight / 2;
    rect->width += scaleWidth;
    rect->height += scaleHeight;
}

void fitToFrame(cv::Rect *r, int imageWidth, int imageHeight) {
    //readjust height
    if (r->width < imageWidth) {
        if (r->x + r->width > imageWidth) {
            r->x -= r->x + r->width - imageWidth;
        }
        if (r->x < 0) {
            r->x = 0;
        }
    }
    else {
        r->width = imageWidth; r->x = 0;
    }
    if (r->height < imageHeight) {
        if (r->y + r->height > imageHeight) {
            r->y -= r->y + r->height - imageHeight;
        }
        if (r->y < 0) r->y = 0;
    }
    else {
        r->height = imageHeight; r->y = 0;
    }
}

bool shiftedRect(cv::Rect prevRect, cv::Rect newRect, int margin) {
    if (abs(prevRect.x - newRect.x) > margin) return true;
    if (abs(prevRect.y - newRect.y) > margin) return true;
    if (abs(prevRect.width - newRect.width) > margin) return true;
    if (abs(prevRect.height - newRect.height) > margin) return true;
    
    return false;
}

int findLocalMinima(int arr[], int arrCount, int lIndex, Direction direction) {
    int smallest = 10000;
    int countingBase = -1;
    int smallestIndex = -1;
    int prevVal = -1;
    
    if (direction == Right) {
        for (int i = lIndex; i < arrCount; i++) {
            int current = arr[i];
            
            if (prevVal == -1) prevVal = current;

            if (current < prevVal) {
                countingBase = i;
            }
            else if (current > prevVal) {
                if (arr[countingBase] < smallest) {
                    smallest = arr[countingBase];
                    smallestIndex = countingBase;
                }
                countingBase = i;
            }
            
            //if (current == 0 && smallest < 10000) break;
            
            prevVal = current;
        }
        if (smallestIndex == -1) smallestIndex = countingBase;
        return smallestIndex;
    }
    else {
        for (int i = lIndex; i >= 0; i--) {
            int current = arr[i];
            
            if (prevVal == -1) prevVal = current;

            if (current < prevVal) {
                countingBase = i;
            }
            else if (current > prevVal) {
                if (arr[countingBase] < smallest) {
                    smallest = arr[countingBase];
                    smallestIndex = countingBase;
                }
                countingBase = i;
            }
            
            //if (current == 0 && smallest < 10000) break;
            prevVal = current;
        }
        if (smallestIndex == -1) smallestIndex = countingBase;
        return smallestIndex;
    }
}

int getLargestIndex(int *arr, int size) {
    int largest = 0;
    int largestIndex = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > largest) {
            largestIndex = i;
            largest = arr[i];
        }
    }
    return largestIndex;
}

void showHistogram(int *vals, int cols) {
    cv::Mat histogram = cv::Mat::zeros(500, cols, CV_8UC4);
    histogram.setTo(cv::Scalar(255,0,0));
    
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < vals[i] * 10 && vals[i] * 10 < 500; j++) {
            histogram.at<cv::Vec3b>(j, i) = cv::Vec3b(0,0,0);
        }
    }
    flip(histogram, histogram, 0);
    imshow("Histogram", histogram);
}
