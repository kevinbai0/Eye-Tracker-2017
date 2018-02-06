//
//  EyeTracker.h
//  GuiApplication
//
//  Created by Kevin Bai on 2017-03-26.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <AppKit/AppKit.h>

@interface EyeTracker : NSObject

@property bool firstImage;

@property NSImage *currentImage;
@property NSArray *currentHeadPose;
@property CGRect currentHeadSize;
@property CGPoint currentLeftPupilPoint;
@property CGPoint currentRightPupilPoint;
@property CGPoint currentAveragePoint;
@property CGPoint prevLeftPupilPoint;
@property CGPoint prevRightPupilPoint;

@property int currentPrediction;
@property int preciseLocation;

- (void) track: (NSImage *) nsImage finalWidth: (int) finalWidth;

@end
