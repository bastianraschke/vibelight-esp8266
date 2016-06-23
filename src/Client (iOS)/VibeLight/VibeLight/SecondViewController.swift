//
//  SecondViewController.swift
//  VibeLight
//
//  Created by Bastian Raschke on 22.06.16.
//  Copyright © 2016 Bastian Raschke. All rights reserved.
//

import UIKit
import KeychainAccess

class SecondViewController: UIViewController {

    @IBOutlet weak var username: UITextField!
    @IBOutlet weak var password: UITextField!
    
    private var keychain: Keychain = Keychain(service: "de.sicherheitskritisch.VibeLight")

    override func viewDidLoad()
    {
        let usernameFromKeychain = readStringFromKeychain("mqtt_username")
        let passwordFromKeychain = readStringFromKeychain("mqtt_password")
        
        self.username.text = usernameFromKeychain
        self.password.text = passwordFromKeychain
    
    
        super.viewDidLoad()
    }

    @IBAction func saveButtonPressed(sender: UIButton)
    {
        if (username.text != "" && password.text != "")
        {
            if (self.saveUsernameAndPasswordInKeychain(username.text!, password: password.text!))
            {
                self.showAlertMessage("Successful saved!")
            }
            else
            {
                self.showAlertMessage("Error occured!")
            }
        }
        else
        {
            self.showAlertMessage("Please enter an username and a password!")
        }
    }

    private func showAlertMessage(message: String)
    {
        let alertController = UIAlertController(title: nil, message: message, preferredStyle: .Alert)
    
        let defaultAction = UIAlertAction(title: "OK", style: .Default, handler: nil)
        alertController.addAction(defaultAction)

        presentViewController(alertController, animated: true, completion: nil)
    }

    private func readStringFromKeychain(key: String) -> String
    {
        var value: String = ""
    
        do
        {
            let readValue = try keychain.get(key)

            if (readValue != nil)
            {
               value = readValue!
            }
        }
        catch let error
        {
            print("Exception occured: \(error)")
        }
        
        return value
    }

    private func saveUsernameAndPasswordInKeychain(username: String, password: String) -> Bool
    {
        var success: Bool = false
        do
        {
            try keychain.set(username, key: "mqtt_username")
            try keychain.set(password, key: "mqtt_password")
            
            success = true
        }
        catch let error
        {
            print("Exception occured: \(error)")
        }
        
        return success
    }



}
