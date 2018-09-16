//
//  UtilityFunctions.hpp
//  RobustHeadTracking
//
//  Created by Kevin Bai on 2017-03-19.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#ifndef UtilityFunctions_hpp
#define UtilityFunctions_hpp

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;

typedef cv::RotatedRect Ellipse;

struct Ratio {
    double width;
    double height;
    
    Ratio(double width, double height);
};

enum Direction {
    Right, Left
};

u_long now();

void scale(cv::Rect *rect, int scaleValue, Ratio ratio);
void fitToFrame(cv::Rect *rect, int imageWidth, int imageHeight);

bool shiftedRect(cv::Rect prevRect, cv::Rect newRect, int margin);

int findLocalMinima(int arr[], int arrCount, int lIndex, Direction direction);
int getLargestIndex(int *arr, int size);

void showHistogram(int *vals, int cols);

#endif /* UtilityFunctions_hpp */
