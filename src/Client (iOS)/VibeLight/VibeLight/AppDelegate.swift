//
//  AppDelegate.swift
//  VibeLight
//
//  Created by Bastian Raschke on 22.06.16.
//  Copyright © 2016 Bastian Raschke. All rights reserved.
//

import UIKit
import Moscapsule

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate
{
    var window: UIWindow?

    func application(application: UIApplication, didFinishLaunchingWithOptions launchOptions: [NSObject: AnyObject]?) -> Bool
    {
        // Try to connect initially
        MQTTConnection.singletonInstance.connect()
        
        return true
    }

    func applicationDidEnterBackground(application: UIApplication)
    {
        MQTTConnection.singletonInstance.disconnect()
    }

    func applicationWillEnterForeground(application: UIApplication)
    {
        MQTTConnection.singletonInstance.reconnect()
    }
}

