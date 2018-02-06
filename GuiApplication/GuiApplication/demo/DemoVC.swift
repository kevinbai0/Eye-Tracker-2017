//
//  ViewController.swift
//  GuiApplication
//
//  Created by Kevin Bai on 2017-03-26.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

import Cocoa
import AVFoundation

class DemoVC: NSViewController, AVCaptureVideoDataOutputSampleBufferDelegate {
    
    let eyeTracker = EyeTracker()
    let demoView = DemoView(horizontalGrids: 5, verticalGrids: 1)
    let gazeMapper = GazeMapper(horizontalGrids: 5, verticalGrids: 1)
    
    let dummyTextField = NSTextField()
    
    lazy var cameraSession : AVCaptureSession = {
        let session = AVCaptureSession();
        session.sessionPreset = AVCaptureSessionPresetHigh
        return session
    }()
    
    let captureDevice = AVCaptureDevice.defaultDevice(withMediaType: AVMediaTypeVideo) as AVCaptureDevice
    
    override func viewDidLoad() {
        self.view.addSubview(dummyTextField)
        dummyTextField.becomeFirstResponder()
        
        self.view.addSubview(demoView)
        demoView.translatesAutoresizingMaskIntoConstraints = false
        demoView.leftAnchor.constraint(equalTo: view.leftAnchor).isActive = true
        demoView.rightAnchor.constraint(equalTo: view.rightAnchor).isActive = true
        demoView.topAnchor.constraint(equalTo: view.topAnchor).isActive = true
        demoView.bottomAnchor.constraint(equalTo: view.bottomAnchor).isActive = true
 
        demoView.startButton = NSButton(title: "Start", target: self, action: #selector(startDemo(sender:)))
        self.view.addSubview(demoView.startButton)
        demoView.startButton.translatesAutoresizingMaskIntoConstraints = false
        demoView.startButton.centerXAnchor.constraint(equalTo: self.view.centerXAnchor).isActive = true
        demoView.startButton.topAnchor.constraint(equalTo: self.view.topAnchor, constant: 10).isActive = true
        startCameraSession()
    }
    
    func startDemo(sender: NSButton) {
        sender.alphaValue = 0
        demoView.initSelf(superFrame: self.view.frame)
        gazeMapper.isTrackingGaze = true
        demoView.setNeedsDisplay(self.view.frame)
    }
    
    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }
    func processImage() {
        
        DispatchQueue.main.async {
            guard let currentHeadPose = (self.eyeTracker.currentHeadPose) as? Int else { return }
            self.demoView.textField.stringValue = "\(currentHeadPose)\n\(self.eyeTracker.currentHeadSize.height)\n\(self.eyeTracker.currentLeftPupilPoint)\n\(self.eyeTracker.currentRightPupilPoint)"
        }
        if gazeMapper.isTrackingGaze {
            var ij = gazeMapper.mapPointToGrid(eyeTracker: eyeTracker)
            
            ij = (Int(eyeTracker.currentPrediction), 0)
            demoView.preciseX = gazeMapper.preciseX
            demoView.preciseX = CGFloat(eyeTracker.preciseLocation)
            DispatchQueue.main.async {
                self.demoView.currentFillRect = self.demoView.grid[ij.0][ij.1]
                self.demoView.setNeedsDisplay(self.view.frame)
            }
        }
    }
    
    
    func captureOutput(_ captureOutput: AVCaptureOutput!, didOutputSampleBuffer sampleBuffer: CMSampleBuffer!, from connection: AVCaptureConnection!) {
        let image = getImageFromSampleBuffer(sampleBuffer: sampleBuffer)
        if (image.size.width > 0) {
            eyeTracker.track(image, finalWidth: 1280)
            processImage()
            //show image
            DispatchQueue.main.async {
                self.demoView.cameraView.image = self.eyeTracker.currentImage;
                self.demoView.setNeedsDisplay(self.view.frame)
            }
        }
    }
    
    override func keyUp(with event: NSEvent) {
        let num = floor(demoView.currentFillRect.minX / 256)
        print("min: \(demoView.currentFillRect.minX) num: \(num)")
        if num == 0 {
            demoView.textField.stringValue += "a"
        }
        else if num == 1 {
            demoView.textField.stringValue += "b"
        }
        else if num == 2 {
            demoView.textField.stringValue += "c"
        }
        else if num == 3 {
            demoView.textField.stringValue += "d"
        }
        else if num == 4 {
            demoView.textField.stringValue += "e"
        }
    }
    
    override func mouseDown(with event: NSEvent) {
        let location = CGFloat(Int(floor(NSEvent.mouseLocation().x / 256)))
        DispatchQueue.main.async {
            self.demoView.textField.frame = CGRect(x: location*256, y: 10, width: 256, height: 200)
        }
    }
    
    
    func startCameraSession() {
        super.viewDidLoad()
        
        do {
            let deviceInput = try AVCaptureDeviceInput(device: captureDevice)
            
            cameraSession.beginConfiguration() // 1
            
            if (cameraSession.canAddInput(deviceInput) == true) {
                cameraSession.addInput(deviceInput)
            }
            
            let dataOutput = AVCaptureVideoDataOutput() // 2
            
            dataOutput.videoSettings = [(kCVPixelBufferPixelFormatTypeKey as NSString) : NSNumber(value: kCVPixelFormatType_32BGRA as UInt32)] // 3
            
            dataOutput.alwaysDiscardsLateVideoFrames = true // 4
            
            if (cameraSession.canAddOutput(dataOutput) == true) {
                cameraSession.addOutput(dataOutput)
            }
            
            cameraSession.commitConfiguration() //5
            
            let queue = DispatchQueue(label: "camera.queue") // 6
            dataOutput.setSampleBufferDelegate(self, queue: queue) // 7
            
        }
        catch let error as NSError {
            NSLog("\(error), \(error.localizedDescription)")
        }
        cameraSession.startRunning()
    }
    
    func getImageFromSampleBuffer(sampleBuffer: CMSampleBuffer) -> NSImage {
        guard let imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer) else {
            return NSImage()
        }
        
        CVPixelBufferLockBaseAddress(imageBuffer, CVPixelBufferLockFlags(rawValue: 0))
        
        let baseAddress = CVPixelBufferGetBaseAddress(imageBuffer)
        
        let bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer)
        
        let width = CVPixelBufferGetWidth(imageBuffer)
        let height = CVPixelBufferGetHeight(imageBuffer)
        
        let colorSpace = CGColorSpaceCreateDeviceRGB()
        
        let bitmapInfo = CGBitmapInfo(rawValue: CGImageAlphaInfo.noneSkipFirst.rawValue | CGBitmapInfo.byteOrder32Little.rawValue)
        
        guard let context = CGContext(data: baseAddress, width: width, height: height, bitsPerComponent: 8, bytesPerRow: bytesPerRow, space: colorSpace, bitmapInfo: bitmapInfo.rawValue) else {
            CVPixelBufferUnlockBaseAddress(imageBuffer, CVPixelBufferLockFlags(rawValue: 0))
            return NSImage()
        }
        
        guard let quartzImage = context.makeImage() else {
            CVPixelBufferUnlockBaseAddress(imageBuffer, CVPixelBufferLockFlags(rawValue: 0))
            return NSImage()
        }
        
        CVPixelBufferUnlockBaseAddress(imageBuffer, CVPixelBufferLockFlags(rawValue: 0))
        
        let image = NSImage(cgImage: quartzImage, size: CGSize(width: width, height: height))
        
        return image
    }
}

