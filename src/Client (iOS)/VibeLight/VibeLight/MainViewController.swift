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
        self.edgesForExtendedLayout = .All
        super.viewDidLoad()
    }

    override func didReceiveMemoryWarning()
    {
        super.didReceiveMemoryWarning()
    }

    private func buildCommand(sceneMode: SceneMode, color1: String, color2: String) -> String
    {
        var command: String = ""
        
        command += sceneMode.rawValue
        command += color1
        command += color2
        
        return command
    }

    @IBAction func sunsetSceneButtonPressed(sender: UIButton)
    {
        let sceneType : SceneMode = SceneMode.MULTICOLOR

        MQTTConnection.singletonInstance.client().publishString(self.buildCommand(sceneType, color1: "9c97bf", color2: "ff8023"),
            topic: "/vibelight/api/1.0/",
            qos: 2,
            retain: false
        )
    }

    @IBAction func warmWhiteSceneButtonPressed(sender: UIButton)
    {
        let sceneType : SceneMode = SceneMode.SINGLECOLOR

        MQTTConnection.singletonInstance.client().publishString(self.buildCommand(sceneType, color1: "FFB027", color2: "000000"),
            topic: "/vibelight/api/1.0/",
            qos: 2,
            retain: false
        )
    }

    @IBAction func violetSceneButtonPressed(sender: UIButton)
    {
        let sceneType : SceneMode = SceneMode.MULTICOLOR

        MQTTConnection.singletonInstance.client().publishString(self.buildCommand(sceneType, color1: "A92CCE", color2: "CE2C2C"),
            topic: "/vibelight/api/1.0/",
            qos: 2,
            retain: false
        )
    }

    @IBAction func offSceneButtonPressed(sender: UIButton)
    {
        let sceneType : SceneMode = SceneMode.OFF

        MQTTConnection.singletonInstance.client().publishString(self.buildCommand(sceneType, color1: "000000", color2: "000000"),
            topic: "/vibelight/api/1.0/",
            qos: 2,
            retain: false
        )
    }
}

enum SceneMode : String
{
    case OFF = "0"
    case SINGLECOLOR = "1"
    case MULTICOLOR = "2"
}