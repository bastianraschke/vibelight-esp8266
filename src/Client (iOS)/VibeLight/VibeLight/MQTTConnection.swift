//
//  MQTTConnection.swift
//  YoLock
//
//  Created by Bastian Raschke on 02.06.16.
//  Copyright © 2016 Bastian Raschke. All rights reserved.
//

import Moscapsule

class MQTTConnection
{
    static let singletonInstance = MQTTConnection()
    
    var mqttHost: String = ""
    var mqttPort: Int32 = 8883
    var mqttUsername: String = ""
    var mqttPassword: String = ""
    
    var mqttClient: MQTTClient?
    var onSubscriptionMessageObservers = Array<OnSubscriptionMessageObserver>()
    
    private init()
    {
        // Initialize Moscapsule TLS stack
        moscapsule_init()
    }
    
    func connect()
    {
        mqttHost = "mqtt.sicherheitskritisch.de"
        mqttPort = 8883
        mqttUsername = KeychainWrapper.singletonInstance.readUsernameFromKeychain()
        mqttPassword = KeychainWrapper.singletonInstance.readPasswordFromKeychain()

        NSLog("Try to establish new connection...")
    
        do
        {
            let mqttConfig = MQTTConfig(clientId: "VibeLight iOS Client 1.0", host: mqttHost, port: mqttPort, keepAlive: 60)

            guard let caCertificateFile = NSBundle.mainBundle().pathForResource("KlosterTrust_Root_CA", ofType: "crt") else
            {
                throw ConnectError.NoCACertificateFound
            }
            
            mqttConfig.mqttAuthOpts = MQTTAuthOpts(username: mqttUsername, password: mqttPassword)
            mqttConfig.mqttServerCert = MQTTServerCert(cafile: caCertificateFile, capath: nil)
            mqttConfig.mqttTlsOpts = MQTTTlsOpts(tls_insecure: false, cert_reqs: CertReqs.SSL_VERIFY_PEER, tls_version: "tlsv1.1", ciphers: "AES256-SHA")

            mqttConfig.onMessageCallback = { mqttMessage in
                for observer in self.onSubscriptionMessageObservers
                {
                    observer.messageWasReceived(mqttMessage)
                }
            }

            //let newMQTTClient = MQTT.newConnection(mqttConfig) // connectImmediately: true
            
//            // Check if the connection was successful
//            if (newMQTTClient.isConnected == false)
//            {
//                //NSLog("Connection failed! Disconnecting...")
//                //newMQTTClient.disconnect();
//            }
//            else
//            {
//                self.mqttClient = newMQTTClient
//            }
        
            mqttClient = MQTT.newConnection(mqttConfig, connectImmediately: true)
        
        }
        catch ConnectError.NoCACertificateFound
        {
            fatalError("The required CA certificate file was not found! Unable to connect to server!")
        }
        catch _
        {
            NSLog("Unknown error occured while connecting to MQTT server!")
        }
    }
    
    func reconnect(forceReconnect: Bool = false)
    {
        if (forceReconnect == true)
        {
            connect()
        }
        else
        {
            if (mqttClient != nil && mqttClient!.isConnected == false)
            {
                mqttClient!.reconnect();
            }
            else
            {
                connect()
            }
        }
    }
    
    func disconnect()
    {
        if (mqttClient != nil && mqttClient!.isConnected == true)
        {
            NSLog("Disconnecting...")
            mqttClient!.disconnect();
        }
    }
    
    func addOnSubscriptionMessageObserver(observer : OnSubscriptionMessageObserver)
    {
        onSubscriptionMessageObservers.append(observer)
    }

    func removeOnSubscriptionMessageObserver(observer : OnSubscriptionMessageObserver)
    {
        for i in onSubscriptionMessageObservers.indices
        {
            if onSubscriptionMessageObservers[i] === observer
            {
                onSubscriptionMessageObservers.removeAtIndex(i)
                break
            }
        }
    }
}

enum ConnectError: ErrorType
{
    case NoCACertificateFound
}

protocol OnSubscriptionMessageObserver : class
{
    func messageWasReceived(mqttMessage: MQTTMessage)
}