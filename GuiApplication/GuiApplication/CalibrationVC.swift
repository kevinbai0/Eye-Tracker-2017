//
//  ViewController.swift
//  GuiApplication
//
//  Created by Kevin Bai on 2017-03-26.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

import Cocoa
import AVFoundation

class CalibrationVC: NSViewController, AVCaptureVideoDataOutputSampleBufferDelegate {
    
    let eyeTracker = EyeTracker()
    let calibrationView = CalibrationView(horizontalGrids: 5, verticalGrids: 1)
    let calibrator = Calibration(horizontalGrids: 5, verticalGrids: 1)
    
    lazy var cameraSession : AVCaptureSession = {
        let session = AVCaptureSession();
        session.sessionPreset = AVCaptureSessionPresetHigh
        return session
    }()
    
    let captureDevice = AVCaptureDevice.defaultDevice(withMediaType: AVMediaTypeVideo) as AVCaptureDevice
    
    override func viewDidLoad() {
        self.view.addSubview(calibrationView)
        calibrationView.translatesAutoresizingMaskIntoConstraints = false
        calibrationView.leftAnchor.constraint(equalTo: view.leftAnchor).isActive = true
        calibrationView.rightAnchor.constraint(equalTo: view.rightAnchor).isActive = true
        calibrationView.topAnchor.constraint(equalTo: view.topAnchor).isActive = true
        calibrationView.bottomAnchor.constraint(equalTo: view.bottomAnchor).isActive = true
 
        //start calibrate button
        calibrationView.calibrateButton = NSButton(title: "Calibrate", target: self, action: #selector(startCalibration(sender:)))
        self.view.addSubview(calibrationView.calibrateButton)
        calibrationView.calibrateButton.frame = CGRect(x: 128-64, y: self.view.frame.midY + 50, width: 128, height: 50)
        calibrationView.initSelf(superFrame: self.view.frame)
        startCameraSession()
        
        let dummyTextField = NSTextField(string: "Dummy")
        dummyTextField.alphaValue = 0
        view.addSubview(dummyTextField)
        dummyTextField.becomeFirstResponder()
    }
    
    override func mouseDown(with event: NSEvent) {
        let location = Int(floor(NSEvent.mouseLocation().x / 256))
        calibrationView.currentFillRect = calibrationView.grid[location][0]
        calibrationView.currentI = location
        DispatchQueue.main.async {
            self.calibrationView.calibrateButton.frame = CGRect(x: 256 * CGFloat(location) + 64, y: self.view.frame.midY + 50, width: 128, height: 50)
            self.calibrationView.textField.frame = CGRect(x: location*256, y: 360, width: 256, height: 100)
        }
    }
    
    override func keyUp(with event: NSEvent) {
        calibrationView.calibrateButton.alphaValue = 1
        calibrator.isCollectingData = false
        calibrator.leftFileHandle.closeFile()
        calibrator.rightFileHandle.closeFile()
        DispatchQueue.main.async {
            self.view.setNeedsDisplay(self.view.frame)
        }
    }
    
    func startCalibration(sender: NSButton) {
        sender.alphaValue = 0
        calibrator.isCollectingData = true
        calibrationView.setNeedsDisplay(self.view.frame)
    }
    
    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }

    func processImage() {
        let height: CGFloat = 200
        let width = height * 3 / 4
        calibrationView.desiredHeadPosition = CGRect(x: 640 / 2 - width / 2, y: 360 / 2 - height / 2, width: width, height: height)
        calibrationView.desiredHeadPose = (0,10,0)
        //calibrationView.alignFace(eyeTracker: eyeTracker)
        
        if calibrator.isCollectingData {
            calibrator.desiredHeadPose = calibrationView.desiredHeadPose
            calibrator.desiredHeadPosition = calibrationView.desiredHeadPosition
            calibrator.collectData(i: calibrationView.currentI, j: calibrationView.currentJ, eyeTracker: eyeTracker)
        }
        else {
            calibrator.leftFileHandle.closeFile()
            calibrator.rightFileHandle.closeFile()
            DispatchQueue.main.async {
                self.calibrationView.calibrateButton.alphaValue = 1
                self.view.setNeedsDisplay(self.view.frame)
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
                self.calibrationView.cameraView.image = self.eyeTracker.currentImage;
                self.calibrationView.textField.stringValue = "\(self.eyeTracker.currentHeadPose as! [Int])\n\(self.eyeTracker.currentHeadSize.height)\n\(self.eyeTracker.currentLeftPupilPoint)\n\(self.eyeTracker.currentRightPupilPoint)"

                self.calibrationView.setNeedsDisplay(self.view.frame)
                self.view.setNeedsDisplay(self.view.frame)
            }
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

