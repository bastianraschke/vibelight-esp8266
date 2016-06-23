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
    
    private var _mqttClient: MQTTClient?
    private var _onSubscriptionMessageObservers = Array<OnSubscriptionMessageObserver>()
    
    private init()
    {
        // Initialize Moscapsule TLS stack
        moscapsule_init()
    }
    
    func connect(forceReconnect: Bool = false)
    {
        // Automatically reconnect if connection is existing when forceReconnect is false
        if ( (_mqttClient != nil && _mqttClient!.isConnected == false) && forceReconnect == false )
        {
            NSLog("Try to reconnect to existing connection...")
        
            _mqttClient!.reconnect();
            return
        }
    
        let mqttHost : String = "mqtt.sicherheitskritisch.de"
        let mqttPort : Int32 = 8883
        let mqttUsername : String = KeychainWrapper.singletonInstance.readUsernameFromKeychain()
        let mqttPassword : String = KeychainWrapper.singletonInstance.readPasswordFromKeychain()

        if (mqttUsername == "" || mqttPassword == "")
        {
            NSLog("The given MQTT credentials are not available!")
            return
        }

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
                for observer in self._onSubscriptionMessageObservers
                {
                    observer.messageWasReceived(mqttMessage)
                }
            }
        
            _mqttClient = MQTT.newConnection(mqttConfig, connectImmediately: true)
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

    func client() -> MQTTClient
    {
        return _mqttClient!;
    }

    func disconnect()
    {
        if (_mqttClient != nil && _mqttClient!.isConnected == true)
        {
            NSLog("Disconnecting...")
            _mqttClient!.disconnect();
        }
    }
    
    func addOnSubscriptionMessageObserver(observer : OnSubscriptionMessageObserver)
    {
        _onSubscriptionMessageObservers.append(observer)
    }

    func removeOnSubscriptionMessageObserver(observer : OnSubscriptionMessageObserver)
    {
        for i in _onSubscriptionMessageObservers.indices
        {
            if _onSubscriptionMessageObservers[i] === observer
            {
                _onSubscriptionMessageObservers.removeAtIndex(i)
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