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
    
    override func viewDidLoad()
    {
        let usernameFromKeychain = KeychainWrapper.singletonInstance.readUsernameFromKeychain()
        let passwordFromKeychain = KeychainWrapper.singletonInstance.readPasswordFromKeychain()
        
        self.username.text = usernameFromKeychain
        self.password.text = passwordFromKeychain
    
        super.viewDidLoad()
    }

    @IBAction func saveButtonPressed(sender: UIButton)
    {
        if (username.text != "" && password.text != "")
        {
            if (KeychainWrapper.singletonInstance.saveUsernameAndPasswordInKeychain(username.text!, password: password.text!))
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
}
