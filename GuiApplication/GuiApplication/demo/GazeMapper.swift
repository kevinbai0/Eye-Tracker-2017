//
//  Calibration.swift
//  GuiApplication
//
//  Created by Kevin Bai on 2017-03-30.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

import Foundation

class GazeMapper {
    var currentI = 0
    var currentJ = 0
    var isTrackingGaze = false
    
    var preciseX: CGFloat = 0
    
    var profiles: [Profile] = []
    
    var numHorizontalGrids = 5
    var numVerticalGrids = 1
    
    init(horizontalGrids: Int, verticalGrids: Int) {
        self.numHorizontalGrids = horizontalGrids
        self.numVerticalGrids = verticalGrids
        
        //load data from calibration
        for i in -2..<3 {
            let file = "CalibrationData/0.0 \(i * 10).0 0.0 320.0 180.0"
            for j in 3..<6 {
                let newFile = file + " \(CGFloat(j * 50) * 3 / 4) \(j * 50).0.txt"
                do {
                    let info = try String(contentsOfFile: newFile)
                    let profile = Profile(headPose: (0.0, CGFloat(i*10), 0.0), headPosition: CGRect(x: 320, y: 180, width:CGFloat(j * 50) * 3 / 4, height: CGFloat(j * 50)))
                    profile.loadCalibratedDataFrom(contents: info)
                    profiles.append(profile)
                } catch { }
            }
        }
    }
    
    func mapPointToGrid(eyeTracker: EyeTracker) -> (Int, Int) {
        //match head profile
        let yaw: CGFloat = eyeTracker.currentHeadPose[1] as! CGFloat
        
        var eyePos = eyeTracker.currentAveragePoint.x
        if !eyeTracker.firstImage {
            //get prev eye position
            var leftBigShift = false
            var rightBigShift = false
            if abs(eyeTracker.currentLeftPupilPoint.x - eyeTracker.currentRightPupilPoint.x) > 5 {
                if abs(eyeTracker.prevLeftPupilPoint.x - eyeTracker.currentLeftPupilPoint.x) > 5 {
                    leftBigShift = true
                }
                if abs(eyeTracker.prevRightPupilPoint.x - eyeTracker.currentRightPupilPoint.x) > 5 {
                    rightBigShift = true
                }
            }
            if leftBigShift && !rightBigShift {
                eyePos = eyeTracker.currentRightPupilPoint.x
            }
            else if !leftBigShift && rightBigShift {
                eyePos = eyeTracker.currentLeftPupilPoint.x
            }
        }
        
        for i in 0..<profiles.count {
            if abs(profiles[i].headPose.1 - yaw) <= 5 {
                //match head size
                if (abs(profiles[i].headPosition.height - eyeTracker.currentHeadSize.height) <= 25) {
                    //find gaze
                    var minDistanceAbs: CGFloat = 100000
                    var minDistance: CGFloat = 100000
                    var minDistPoint = (0,0)
                    for j in 0..<profiles[i].calibratedData.count {
                        let dist = eyePos - profiles[i].calibratedData[j]
                        if abs(dist) < minDistanceAbs {
                            minDistanceAbs = abs(dist)
                            minDistance = dist
                            minDistPoint = (j, 0)
                        }
                    }
                    
                    let p = minDistPoint.0
                    if minDistance < 0 {
                        //it is j-1
                        var x1:CGFloat = 0
                        var x2:CGFloat = 0
                        var y1:CGFloat = 0
                        var y2:CGFloat = 0
                        if p == 0 {
                            x2 = profiles[i].calibratedData[0]
                            y2 = 128
                        }
                        else {
                            x1 = profiles[i].calibratedData[p - 1]
                            x2 = profiles[i].calibratedData[p]
                            y1 = 128 * CGFloat(p * 2 - 1)
                            y2 = 128 * CGFloat((p+1) * 2 - 1)
                        }
                        
                        let m = (y2 - y1) / (x2 - x1)
                        let b = y2 - (m * x2)
                        
                        //new point
                        preciseX = m * eyePos + b
                    }
                    else {
                        var x1:CGFloat = 0
                        var x2:CGFloat = 0
                        var y1:CGFloat = 0
                        var y2:CGFloat = 0
                        if p == 4 {
                            x1 = profiles[i].calibratedData[4]
                            x2 = 200
                            y1 = 128 * CGFloat((p+1) * 2 - 1)
                            y2 = 1280
                        }
                        else {
                            x1 = profiles[i].calibratedData[p]
                            x2 = profiles[i].calibratedData[p + 1]
                            y1 = 128 * CGFloat((p+1) * 2 - 1)
                            y2 = 128 * CGFloat((p + 2) * 2 - 1)
                        }
                        
                        let m = (y2 - y1) / (x2 - x1)
                        let b = y2 - (m * x2)
                        
                        //new point
                        preciseX = m * eyePos + b
                    }
                    
                    return minDistPoint
                }
            }
        }
        return (0,0)
    }
}
