//
//  KeychainWrapper.swift
//  VibeLight
//
//  Created by Bastian Raschke on 23.06.16.
//  Copyright © 2016 Bastian Raschke. All rights reserved.
//

import KeychainAccess

class KeychainWrapper
{
    static let singletonInstance = KeychainWrapper()
    private var keychain: Keychain = Keychain(service: "de.sicherheitskritisch.VibeLight")

    func readUsernameFromKeychain() -> String
    {
        return self.readStringFromKeychain("mqtt_username")
    }

    func readPasswordFromKeychain() -> String
    {
        return self.readStringFromKeychain("mqtt_password")
    }

    func saveUsernameAndPasswordInKeychain(username: String, password: String) -> Bool
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
}
