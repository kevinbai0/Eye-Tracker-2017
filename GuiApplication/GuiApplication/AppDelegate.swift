//
//  AppDelegate.swift
//  GuiApplication
//
//  Created by Kevin Bai on 2017-03-26.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

import Cocoa

@NSApplicationMain

class AppDelegate: NSObject, NSApplicationDelegate {
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }
}

extension NSImage {
    var jpgData: Data? {
        guard let tiffRepresentation = tiffRepresentation, let bitmapImage = NSBitmapImageRep(data: tiffRepresentation) else { return nil }
        return bitmapImage.representation(using: .JPEG, properties: [:])
    }
    func jpgWrite(to url: URL, options: Data.WritingOptions = .atomic) {
        do {
            try jpgData?.write(to: url, options: options)
        } catch {
            print(error.localizedDescription)
        }
    }
}
