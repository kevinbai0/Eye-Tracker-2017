//
//  NSImage+Mat.h
//  GuiApplication
//
//  Created by Kevin Bai on 2017-03-26.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#import <opencv2/opencv.hpp>
#import <Cocoa/Cocoa.h>

using namespace std;

@interface NSImage (Mat)

+(NSImage*)imageWithCVMat:(const cv::Mat&)cvMat;
-(id)initWithCVMat:(const cv::Mat&)cvMat;

@property(nonatomic, readonly) cv::Mat CVMat;

@end
