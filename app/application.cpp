/*

* DHT22-MQTT *

Questo programma legge un sensore DHT (temperatura e umidita`) e pubblica
i dati sul server MQTT

Versione 0.0.3 del 2018.7.18 (e successivi giorni)
- Modifiche per nuova versione Sming
- Cambio tempi d'invio e controllo d'invio valori diversi (memorie)
- Aggiunto temperatura percepita e temperatura di condensazione acqua

Versione 0.0.2 del 2017.4.27
- Aggiunto watchdog, ma non mi pare funzioni.
- Elimnazione output debug

*/

/* LICENSE

The MIT License (MIT)

Copyright (c) 2018 davide

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <SmingCore/Platform/WDT.h>
#include <Libraries/DHTesp/DHTesp.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
    //#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
    //#define WIFI_PWD "PleaseEnterPass"
#endif

// ... and/or MQTT username and password
#ifndef MQTT_USERNAME
    #define MQTT_USERNAME ""
    #define MQTT_PWD ""
#endif

// ... and/or MQTT host and port
#ifndef MQTT_HOST
	#define MQTT_HOST "level1"
	#ifndef ENABLE_SSL
		#define MQTT_PORT 1883
	#else
		#define MQTT_PORT 8883
	#endif
#endif

/* (My) User settings */
//#define MQTT_PUBLISH "I/Casa/Cantina/Fumetti"             // Percorso di pubblicazione dei dati MQTT, senza "Tipo"
//#define MQTT_PUBLISH "I/Casa/Cantina/Fumetti/Debugs"      // Percorso di pubblicazione dei dati MQTT, senza "Tipo"
#define MQTT_PUBLISHH "I/Casa/Cantina/Fumetti/Umidita"      // Percorso di pubblicazione dei dati MQTT, senza "Tipo"
#define MQTT_PUBLISHT "I/Casa/Cantina/Fumetti/Temperatura"  // Percorso di pubblicazione dei dati MQTT, senza "Tipo"
#define MQTT_SUBSCRIBE "I/#"                                // Percorso di lettura dei dati MQTT
#define MQTT_IDST "ST1"                                     // Identificatore
#define MQTT_IDRH "RH1"                                     // Identificatore
// Nuovi parametri/valori della libreria DHT (2018)
#define MQTT_PUBLISHHI "I/Casa/Cantina/Fumetti/HeatIndex"  // Temperatura percepita
#define MQTT_PUBLISHDP "I/Casa/Cantina/Fumetti/DewPoint"   // Temperatura di condensazione

// DHT setup
#define WORK_PIN 2 // GPIO2

//Memorie
float hmem = 0;
float tmem = 0;
float himem = 0;
float dpmem = 0;

DHTesp dht;


// Forward declarations
void startMqttClient();
//void onMessageReceived(String topic, String message);
void publishMessage();	// Serve questa dichiarazione ?

Timer procTimer;

// MQTT client
// For quick check you can use: http://www.hivemq.com/demos/websocket-client/ (Connection= test.mosquitto.org:8080)
MqttClient *mqtt;

/*
// Check for MQTT Disconnection
void checkMQTTDisconnect(TcpClient& client, bool flag){

    // Called whenever MQTT connection is failed.
    if (flag == true)
        Serial.println("MQTT Broker Disconnected!!");
    else
        Serial.println("MQTT Broker Unreachable!!");

    // Restart connection attempt after few seconds
    procTimer.initializeMs(2 * 1000, startMqttClient).start(); // every 2 seconds
}
*/

/*
void onMessageDelivered(uint16_t msgId, int type) {
	Serial.printf("Message with id %d and QoS %d was delivered successfully.", msgId, (type==MQTT_MSG_PUBREC? 2: 1));
}
*/

// Publish our message
void publishMessage()
{
    WDT.enable(true);
    //WDT.alive();

    Serial.println("\t\t DHT improved lib");
    Serial.println("wait 1 second for the sensor to boot up");

    //disable watchdog
    //WDT.enable(false);
    //wait for sensor startup
    delay(1000);
    WDT.alive();

    /*first reading method (Adafruit compatible) */
    Serial.print("Read using Adafruit API methods\n");
    float h = dht.getHumidity();
    float t = dht.getTemperature();
    WDT.alive();
    delay(1000);
    float hi = dht.computeHeatIndex(t, h);
    float dp = dht.computeDewPoint(t, h);
    WDT.alive();

    // Sempre controllare se e` attiva, senno` non spedisce
    if (mqtt->getConnectionState() != eTCS_Connected)
    {
        startMqttClient(); // Auto reconnect
        WDT.alive();
        delay(1000);
    }

/*
    // MyDebug: invio comunque i valori, perche` non capisco
    mqtt->publish(String(MQTT_PUBLISH)+":Start", "{ \"--\" : \"--\", \"------\" : \"--\" }");
    mqtt->publish(String(MQTT_PUBLISHH)+":Debug", "{ \"ID\" : \"" + String(MQTT_IDRH)+ "\", \"Valore\" : \"" + String(h) + "\" }");
    mqtt->publish(String(MQTT_PUBLISHT)+":Debug", "{ \"ID\" : \"" + String(MQTT_IDST)+ "\", \"Valore\" : \"" + String(t) + "\" }");
    mqtt->publish(String(MQTT_PUBLISHHI)+":Debug", "{ \"ID\" : \"" + String(MQTT_IDST)+ "\", \"Valore\" : \"" + String(hi) + "\" }");
    mqtt->publish(String(MQTT_PUBLISHDP)+":Debug", "{ \"ID\" : \"" + String(MQTT_IDST)+ "\", \"Valore\" : \"" + String(dp) + "\" }");
    mqtt->publish(String(MQTT_PUBLISHH)+":Debug", "{ \"ID\" : \"" + String(MQTT_IDRH)+ "mem\", \"Valore\" : \"" + String(hmem) + "\" }");
    mqtt->publish(String(MQTT_PUBLISHT)+":Debug", "{ \"ID\" : \"" + String(MQTT_IDST)+ "mem\", \"Valore\" : \"" + String(tmem) + "\" }");
    mqtt->publish(String(MQTT_PUBLISHHI)+":Debug", "{ \"ID\" : \"" + String(MQTT_IDST)+ "mem\", \"Valore\" : \"" + String(himem) + "\" }");
    mqtt->publish(String(MQTT_PUBLISHDP)+":Debug", "{ \"ID\" : \"" + String(MQTT_IDST)+ "mem\", \"Valore\" : \"" + String(dpmem) + "\" }");
*/

    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    // if (isnan(t) || isnan(h)) // old version
    if (dht.getStatus() != DHTesp::ERROR_NONE)
    {
        Serial.println("Failed to read from DHT");
        // Ho dovuto aggiungerla anche qua, perche` senno` si blocca il timer, non so perche`.
        //if (mqtt->getConnectionState() != eTCS_Connected)
        //startMqttClient(); // Auto reconnect
        WDT.alive();
        //if (mqtt->getConnectionState() != eTCS_Connected)
        //{
        //    startMqttClient(); // Auto reconnect
        //    WDT.alive();
        //    delay(1000);
        //}
        //mqtt->publish(String(MQTT_PUBLISH)+":Debug", "{ \"ID\" : \"Error\", \"Valore\" : \"Errore lettura DHT\" }");
    } else {
        Serial.print("\tHumidity: ");
        Serial.print(h);
        Serial.print("% Temperature: ");
        Serial.print(t);
        Serial.print(" *C\n");

        if ( h != hmem )
        {
            //if (mqtt->getConnectionState() != eTCS_Connected)
            //{
            //    startMqttClient(); // Auto reconnect
            //    WDT.alive();
            //    delay(1000);
            //}

            Serial.println("Let's publish messages now!");
            // Umidita`
            Serial.println(String(MQTT_PUBLISHH) + " ID:" + String(MQTT_IDRH)+ " Valore:" + String(h));
            mqtt->publish(MQTT_PUBLISHH, "{ \"ID\" : \"" + String(MQTT_IDRH)+ "\", \"Valore\" : \"" + String(h) + "\" }");
            Serial.println("******************************************");
            WDT.alive();

            hmem = h;
        }

        if ( t != tmem )
        {
            //if (mqtt->getConnectionState() != eTCS_Connected)
            //{
            //    startMqttClient(); // Auto reconnect
            //    WDT.alive();
            //    delay(1000);
            //}

            Serial.println("Let's publish messages now!");
            // Temperatura
            Serial.println(String(MQTT_PUBLISHT) + " ID:" + String(MQTT_IDST)+ " Valore:" + String(t));
            mqtt->publish(MQTT_PUBLISHT, "{ \"ID\" : \"" + String(MQTT_IDST)+ "\", \"Valore\" : \"" + String(t) + "\" }");
            Serial.println("******************************************");
            WDT.alive();

            tmem = t;
        }



        //  Other goodies:
        //
        //  Heatindex is the percieved temperature taking humidity into account
        //  More: https://en.wikipedia.org/wiki/Heat_index
        //
        Serial.print("\tHeatindex: ");
        Serial.print(hi);
        Serial.print("*C\n");

        //
        //  Dewpoint is the temperature where condensation starts.
        //  Water vapors will start condensing on an object having this temperature or below.
        //  More: https://en.wikipedia.org/wiki/Dew_point
        // 
        Serial.printf("\tDewpoint: ");
        Serial.print(dp);
        Serial.print("*C\n");

        WDT.alive();

        if ( hi != himem )
        {
            //if (mqtt->getConnectionState() != eTCS_Connected)
            //{
            //    startMqttClient(); // Auto reconnect
            //    WDT.alive();
            //    delay(1000);
            //}

            Serial.println("Let's publish messages now!");
            // Heatindex
            Serial.println(String(MQTT_PUBLISHHI) + " ID:" + String(MQTT_IDST)+ " Valore:" + String(hi));
            mqtt->publish(MQTT_PUBLISHHI, "{ \"ID\" : \"" + String(MQTT_IDST)+ "\", \"Valore\" : \"" + String(hi) + "\" }");
            Serial.println("******************************************");
            WDT.alive();

            himem = hi;
        }

        if ( dp != dpmem )
        {
            //if (mqtt->getConnectionState() != eTCS_Connected)
            //{
            //    startMqttClient(); // Auto reconnect
            //    WDT.alive();
            //    delay(1000);
            //}

            Serial.println("Let's publish messages now!");
            // Dewpoint
            Serial.println(String(MQTT_PUBLISHDP) + " ID:" + String(MQTT_IDST)+ " Valore:" + String(dp));
            mqtt->publish(MQTT_PUBLISHDP, "{ \"ID\" : \"" + String(MQTT_IDST)+ "\", \"Valore\" : \"" + String(dp) + "\" }");
            Serial.println("******************************************");
            WDT.alive();

            dpmem = dp;
        }

        WDT.alive();
    }

    WDT.alive();
    //WDT.enable(false);
    //Serial.println("WDT.enable(false)");
}

/*
// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
    Serial.print(topic);
    Serial.print(":\r\n\t"); // Pretify alignment for printing
    Serial.println(message);
}
*/

// Run MQTT client
void startMqttClient()
{
    Serial.println("Function startMqttClient() [re]started ..");
    // Blocco, secondo me non lo devo piu` fermare, per come ho fatto il software
    //procTimer.stop();
    if(!mqtt->setWill("last/will","The connection from this device is lost:(", 1, true)) {
        debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
    }
    mqtt->connect("espFumetti", MQTT_USERNAME, MQTT_PWD, true);
#ifdef ENABLE_SSL
    mqtt->addSslOptions(SSL_SERVER_VERIFY_LATER);

    #include <ssl/private_key.h>
    #include <ssl/cert.h>

    mqtt->setSslClientKeyCert(default_private_key, default_private_key_len,
                              default_certificate, default_certificate_len, NULL, true);

#endif

    // Assign a disconnect callback function
    //mqtt->setCompleteDelegate(checkMQTTDisconnect);
    //mqtt->subscribe(MQTT_SUBSCRIBE);
}

// Will be called when WiFi station was connected to AP
//void connectOk()
void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
    Serial.println("1) connectOk, startMqttClient, publishMessage");

    // Run MQTT client
    startMqttClient();

    publishMessage();
    // Start publishing loop
    procTimer.initializeMs(900 * 1000, publishMessage).start(); // ogni 15 min (900 * 1000)

    Serial.println("1) Primo WDT.alive");
    WDT.alive();

}

// Will be called when WiFi station timeout was reached
//void connectFail()
void connectFail(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason)
{
    Serial.println("I'm NOT CONNECTED. Need help :(");

    // .. some you code for device configuration ..
}

void init()
{
    //enable watchdog
    //WDT.enable(true);

    Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
    Serial.systemDebugOutput(true); // Debug output to serial

    dht.setup(WORK_PIN, DHTesp::DHT22);

    //mqtt = new MqttClient(MQTT_HOST, MQTT_PORT, onMessageReceived);
    mqtt = new MqttClient(MQTT_HOST, MQTT_PORT);
    WifiStation.config(WIFI_SSID, WIFI_PWD);
    WifiStation.enable(true);
    WifiAccessPoint.enable(false);

    // Run our method when station was connected to AP (or not connected)
    //WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start
    // Run our method when station was connected to AP (or not connected)
    WifiEvents.onStationDisconnect(connectFail);
    WifiEvents.onStationGotIP(gotIP);

    WDT.alive();
}
