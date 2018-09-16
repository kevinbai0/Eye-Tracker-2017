//
//  Calibration.swift
//  GuiApplication
//
//  Created by Kevin Bai on 2017-03-29.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

import Foundation

class DemoView: NSView {
    
    var currentFillRect = CGRect()
    var currentI = 0
    var currentJ = 0
    
    var grid: [[CGRect]] = []
    var numHorizontalGrids = 4
    var numVerticalGrids = 3
    
    var desiredHeadPose = (CGFloat(0), CGFloat(0), CGFloat(0))
    var desiredHeadPosition = CGRect()
    var headPoseState = (0,0,0)
    var headSizeState = 0
    var headPositionState = (0,0)
    
    let cameraView = NSImageView()
    var currentImage = NSImage()
    var startButton = NSButton()
    var textField = NSTextField(string: "")
    
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
        
        textField.frame = CGRect(x: 0, y: 10, width: 256, height: 200)
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
    
    var preciseX: CGFloat = 0
    
    override func draw(_ dirtyRect: NSRect) {
        NSColor(calibratedRed: 0, green: 1, blue: 0, alpha: 1.0).set()
        let path = NSBezierPath(rect: currentFillRect)
        path.fill()
        
        //circles
        NSColor(calibratedRed: 1, green: 0, blue: 0, alpha: 1).set()
        if grid.count == numHorizontalGrids {
            for i in 0..<numHorizontalGrids {
                for j in 0..<numVerticalGrids {
                    let circlePath = NSBezierPath(ovalIn: CGRect(x: grid[i][j].midX-10, y: grid[i][j].midY-10, width: 20, height: 20))
                    circlePath.fill()
                }
            }
        }
        
        NSColor(calibratedWhite: 0, alpha: 1).set()
        NSBezierPath(ovalIn: CGRect(x: preciseX - 10, y: frame.midY, width: 20, height: 20)).fill()
        
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
    }
}
