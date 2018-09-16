//
//  Calibration.swift
//  GuiApplication
//
//  Created by Kevin Bai on 2017-03-29.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

import Foundation

class CalibrationView: NSView {
    
    var currentFillRect = CGRect()
    var currentI = 0
    var currentJ = 0
    
    var grid: [[CGRect]] = []
    var numHorizontalGrids = 5
    var numVerticalGrids = 1
    
    var desiredHeadPose = (CGFloat(0), CGFloat(0), CGFloat(0))
    var desiredHeadPosition = CGRect()
    var headPoseState = (0,0,0)
    var headSizeState = 0
    var headPositionState = (0,0)
    
    let cameraView = NSImageView()
    var currentImage = NSImage()
    var calibrateButton = NSButton()
    var textField = NSTextField()
    
    //flip coordinate system
    override var isFlipped: Bool {
        return true
    }
    override init(frame: NSRect) {
        super.init(frame: NSRect.zero)
    }
    convenience init(horizontalGrids: Int, verticalGrids: Int) {
        self.init(frame: CGRect.zero)
        self.numHorizontalGrids = horizontalGrids
        self.numVerticalGrids = verticalGrids
        
        self.addSubview(cameraView)
        cameraView.alphaValue = 0.5
        cameraView.translatesAutoresizingMaskIntoConstraints = false
        
        cameraView.leftAnchor.constraint(equalTo: self.leftAnchor).isActive = true
        cameraView.bottomAnchor.constraint(equalTo: self.bottomAnchor).isActive = true
        
        self.addSubview(textField)
        textField.alignment = .center
        textField.isEditable = false
        
        textField.frame = CGRect(x: 0, y: 360, width: 256, height: 100)
    }
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    func initSelf(superFrame: CGRect) {
        let gridWidth: CGFloat = superFrame.width / CGFloat(numHorizontalGrids)
        let gridHeight: CGFloat = superFrame.height / CGFloat(numVerticalGrids)
        for i in stride(from: 0, to: CGFloat(numHorizontalGrids), by: 1) {
            var row: [CGRect] = []
            for j in stride(from: 0, to: CGFloat(numVerticalGrids), by: 1) {
                row.append(CGRect(x: i * gridWidth, y: j * gridHeight, width: gridWidth, height: gridHeight))
            }
            grid.append(row)
        }
        currentFillRect = grid[0][0];
    }
    
    func delayAndDraw(i: Int, j: Int) {
        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + 2) {
            self.currentI = i
            self.currentJ = j
            self.currentFillRect = self.grid[i][j]
            self.setNeedsDisplay(self.frame)
            
            self.currentI = -2
            self.currentJ = -2
            //0.5 second pause
            DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) {
                self.currentI = i
                self.currentJ = j
                if (j == self.numVerticalGrids - 1 && i < self.numHorizontalGrids - 1) {
                    self.delayAndDraw(i: i + 1, j: 0)
                }
                else if (j == self.numVerticalGrids - 1 && i == self.numHorizontalGrids - 1) {
                    DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + 2) {
                        self.currentI = -1
                        self.currentJ = -1
                    }
                    return
                }
                else if (j < self.numVerticalGrids - 1) {
                    self.delayAndDraw(i: i, j: j + 1)
                }
            }
        }
    }

    
    
    override func draw(_ dirtyRect: NSRect) {
        //fill current grid
        //draw symbols
        if headPositionState.0 != 0 || headPositionState.1 != 0 || headSizeState != 0 {
            NSColor(calibratedRed: 0, green: 0, blue: 1, alpha: 1).set()
            let translatedRect = CGRect(x: desiredHeadPosition.minX * 2, y: desiredHeadPosition.minY * 2, width: desiredHeadPosition.width * 2, height: desiredHeadPosition.height * 2)
            let headPath = NSBezierPath(rect: translatedRect)
            headPath.fill()
        }
        NSColor(calibratedRed: 0, green: 1, blue: 1, alpha: 1).set()
        if headPoseState.1 == 1 {
            NSBezierPath(rect: CGRect(x: frame.midX, y: frame.midY, width: 50, height: 50)).stroke()
        }
        else if headPoseState.1 == -1 {
            NSBezierPath(rect: CGRect(x: frame.midX, y: frame.midY, width: 50, height: 50)).stroke()
        }
        
        
        NSColor(calibratedRed: 0, green: 1, blue: 0, alpha: 1.0).set()
        let path = NSBezierPath(rect: currentFillRect)
        path.fill()
        
        //print grid
        NSColor(calibratedWhite: 0, alpha: 1).set()
        let gridPath = NSBezierPath()
        gridPath.lineWidth = 1.0
        
        let gridHeight = dirtyRect.height / CGFloat(numVerticalGrids)
        let gridWidth = dirtyRect.width / CGFloat(numHorizontalGrids)
        for i in 0..<numHorizontalGrids {
            gridPath.move(to: CGPoint(x: gridWidth * CGFloat(i), y: 0))
            gridPath.line(to: CGPoint(x: gridWidth * CGFloat(i), y: frame.height))
        }
        for i in 0..<numVerticalGrids {
            gridPath.move(to: CGPoint(x: 0, y: gridHeight * CGFloat(i)))
            gridPath.line(to: CGPoint(x: frame.width, y: gridHeight * CGFloat(i)))
        }
        gridPath.stroke()
        
        NSColor(calibratedRed: 1, green: 0, blue: 0, alpha: 1).set()
        if grid.count == numHorizontalGrids {
            for i in 0..<numHorizontalGrids {
                for j in 0..<numVerticalGrids {
                    let circlePath = NSBezierPath(ovalIn: CGRect(x: grid[i][j].midX-10, y: grid[i][j].midY-10, width: 20, height: 20))
                    circlePath.fill()
                }
            }
        }
    }
    
    func alignFace(eyeTracker: EyeTracker) {
        if eyeTracker.currentHeadSize.height - desiredHeadPosition.height > 25 {
            //greater height
            headSizeState = 1
        }
        else if eyeTracker.currentHeadSize.height - desiredHeadPosition.height < -25 {
            //less than height
            headSizeState = -1
        }
        else {
            headSizeState = 0
        }
        
        let pose = eyeTracker.currentHeadPose as! [CGFloat]
        if desiredHeadPose.0 - pose[0] > 5 {
            //greater pitch
            headPoseState.0 = 1
        }
        else if desiredHeadPose.0 - pose[0] < -5 {
            //less pitch
            headPoseState.0 = -1
        }
        else {
            headPoseState.0 = 0
        }
        
        if desiredHeadPose.1 - pose[1] > 5 {
            //greater pitch
            headPoseState.1 = 1
        }
        else if desiredHeadPose.1 - pose[1] < -5 {
            //less pitch
            headPoseState.1 = -1
        }
        else {
            headPoseState.1 = 0
        }
        
        if desiredHeadPose.2 - pose[2] > 5 {
            //greater pitch
            headPoseState.2 = 1
        }
        else if desiredHeadPose.2 - pose[2] < -5 {
            //less pitch
            headPoseState.2 = -1
        }
        else {
            headPoseState.2 = 0
        }
        
        //head position
        let headCenter = CGPoint(x: eyeTracker.currentHeadSize.midX, y: eyeTracker.currentHeadSize.midY)
        if headCenter.x - desiredHeadPosition.midX > 20 {
            //greater x
            headPositionState.0 = 1
        }
        else if headCenter.x - desiredHeadPosition.midX < -20 {
            //less x
            headPositionState.0 = -1
        }
        else {
            headPositionState.0 = 0
        }
        
        if headCenter.y - desiredHeadPosition.midY > 20 {
            //greater y
            headPositionState.1 = 1
        }
        else if headCenter.y - desiredHeadPosition.midY < -20 {
            //less y
            headPositionState.1 = -1
        }
        else {
            headPositionState.1 = 0
        }
    }
}
