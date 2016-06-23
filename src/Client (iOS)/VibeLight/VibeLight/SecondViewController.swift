//
//  SecondViewController.swift
//  VibeLight
//
//  Created by Bastian Raschke on 22.06.16.
//  Copyright © 2016 Bastian Raschke. All rights reserved.
//

import UIKit

class SecondViewController: UIViewController {

    @IBOutlet weak var username: UITextField!
    @IBOutlet weak var password: UITextField!
    

    
    override func viewDidLoad() {
        super.viewDidLoad()
    }


    @IBAction func saveButtonPressed(sender: UIButton) {
        NSLog("Button pressed")
        
        if (username.text != "")
        {
            NSLog(username.text!)
        }
        else
        {
            NSLog("No username provided!");
        }
        

        if (password.text != "")
        {
            NSLog(password.text!)
        }
        else
        {
            NSLog("No password provided!");
        }
    }

}
