//
//  Calibration.swift
//  GuiApplication
//
//  Created by Kevin Bai on 2017-03-30.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

import Foundation

class Calibration {
    var isCollectingData = false
    
    var preciseX: CGFloat = 0
    
    var numHorizontalGrids = 5
    var numVerticalGrids = 1
    
    var desiredHeadPose: (CGFloat, CGFloat, CGFloat) = (0.0,0.0,0.0)
    var desiredHeadPosition: CGRect = CGRect()
    
    var filesAreClosed = true
    
    let pupilLeftURL = URL(fileURLWithPath: "calib2/finalLeftData/data3.txt")
    let pupilRightURL = URL(fileURLWithPath: "calib2/finalRightData/data3.txt")
    var leftFileHandle = FileHandle()
    var rightFileHandle = FileHandle()
    
    init(horizontalGrids: Int, verticalGrids: Int) {
        self.numHorizontalGrids = horizontalGrids
        self.numVerticalGrids = verticalGrids
        /*do {
            try "".write(to: pupilLeftURL, atomically: false, encoding: .utf8)
            try "".write(to: pupilRightURL, atomically: false, encoding: .utf8)
        }
        catch { }*/
    }
    
    var counter = 0
    
    func collectData(i: Int, j: Int, eyeTracker: EyeTracker) {
        if i == -1 {
            //do nothing
            leftFileHandle.closeFile()
            rightFileHandle.closeFile()
        }
        else if i == -2 {
            //pause so do nothing
            leftFileHandle.closeFile()
            rightFileHandle.closeFile()
        }
        else {
            if filesAreClosed {
                do {
                    try leftFileHandle = FileHandle(forUpdating: pupilLeftURL)
                    try rightFileHandle = FileHandle(forUpdating: pupilRightURL)
                }
                catch { }
            }
            
            let leftPupilPoint = eyeTracker.currentLeftPupilPoint
            let rightPupilPoint = eyeTracker.currentRightPupilPoint
            
            /*if (abs(leftPupilPoint.x - rightPupilPoint.x) > 10) {
                return
            }
            if abs(eyeTracker.currentHeadSize.height - desiredHeadPosition.height) > 25 {
                return
            }
            
            let pose = eyeTracker.currentHeadPose as! [CGFloat]
            if abs(desiredHeadPose.1 - pose[1]) > 5 {
                return
            }
            
            let headCenter = CGPoint(x: eyeTracker.currentHeadSize.midX, y: eyeTracker.currentHeadSize.midY)
            if abs(headCenter.x - desiredHeadPosition.midX) > 20 {
                return
            }
            if abs(headCenter.y - desiredHeadPosition.midY) > 20 {
                return
            }*/
            
            if counter < 90 {
                let writeLeft = "\(eyeTracker.currentHeadPose as! [CGFloat])::\(eyeTracker.currentHeadSize)::\(eyeTracker.currentLeftPupilPoint)->\((i,j))\n"
                let writeRight = "\(eyeTracker.currentHeadPose as! [CGFloat])::\(eyeTracker.currentHeadSize)::\(eyeTracker.currentRightPupilPoint)->\((i,j))\n"
                leftFileHandle.seekToEndOfFile()
                rightFileHandle.seekToEndOfFile()
                leftFileHandle.write(writeLeft.data(using: String.Encoding.utf8)!)
                rightFileHandle.write(writeRight.data(using: String.Encoding.utf8)!)
                counter += 1
            }
            else {
                isCollectingData = false
                counter = 0
            }
            
            
            
            /*DispatchQueue.main.async {
                eyeTracker.currentImage.jpgWrite(to: URL(fileURLWithPath: "/users/kbai/desktop/collected/\(eyeTracker.currentHeadPose as! [CGFloat])::\(eyeTracker.currentHeadSize)::\(eyeTracker.currentRightPupilPoint)->\((i,j)).jpg"))
            }*/

        }
    }
}
