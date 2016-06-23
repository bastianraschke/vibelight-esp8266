//
//  MainViewController.swift
//  VibeLight
//
//  Created by Bastian Raschke on 22.06.16.
//  Copyright © 2016 Bastian Raschke. All rights reserved.
//

import UIKit

class MainViewController: UIViewController
{

    override func viewDidLoad()
    {
        super.viewDidLoad()
    }

    override func didReceiveMemoryWarning()
    {
        super.didReceiveMemoryWarning()
    }

    @IBAction func sunsetSceneButtonPressed(sender: UIButton)
    {
        MQTTConnection.singletonInstance.client().publishString("29c97bfff8023", topic: "/vibelight/api/1.0/", qos: 2, retain: false)
    }

    @IBAction func offSceneButtonPressed(sender: UIButton)
    {
        MQTTConnection.singletonInstance.client().publishString("0000000000000", topic: "/vibelight/api/1.0/", qos: 2, retain: false)
    }
}
