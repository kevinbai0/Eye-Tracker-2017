//
//  MainVC.swift
//  GuiApplication
//
//  Created by Kevin Bai on 2017-05-11.
//  Copyright © 2017 Kevin Bai. All rights reserved.
//

import Cocoa

class MainVC: NSTabViewController {
    
    var item1 = NSTabViewItem(viewController: DemoVC())
    var item2 = NSTabViewItem(viewController: CalibrationVC())

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do view setup here.
        
        self.addTabViewItem(item1)
        self.addTabViewItem(item2)
    }
    
}
