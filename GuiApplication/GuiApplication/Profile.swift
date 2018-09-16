//
//  Profile.swift
//  GuiApplication
//
//  Created by Kevin Bai on 2017-04-05.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

import Foundation

class Profile {
    var headPose: (CGFloat, CGFloat, CGFloat) = (CGFloat(0), 0, 0)
    var headPosition = CGRect()
    var calibratedData: [CGFloat] = []
    
    init(headPose: (CGFloat, CGFloat, CGFloat), headPosition: CGRect) {
        self.headPose = headPose
        self.headPosition = headPosition
    }
    
    init() {}
    
    func loadCalibratedDataFrom(contents: String) {
        let sections = contents.characters.split(separator: "\n")
        for i in 0..<sections.count {
            var str = String(sections[i])
            let index = str.characters.index(of: " ")
            str = str.substring(to: index!)
            calibratedData.append(CGFloat(Float(str)!))
        }
    }
}
