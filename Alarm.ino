#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Discord_WebHook.h>
#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>
#include <assert.h>
#include <UnixTime.h>

#ifndef STASSID
#define STASSID "ssid-of-your-WiFi"
#define STAPSK "password-of-your-WiFi"
#endif

#define CURL "http://worldtimeapi.org/api/ip"

const char *ssid = STASSID;
const char *psk = STAPSK;
const char *curl = CURL;

String alarm = "????";
int unixTime;

ESP8266WebServer server(80);
HTTPClient http;
WiFiClient client;
UnixTime stamp(-6);

int handleTime(bool flag){
    //If flag is true, return the hour; if not, return the minute
    if (flag)
    {
        String hour;
        hour += alarm[0];
        hour += alarm[1];
        return hour.toInt();
    }
    else
    {
        String minute;
        minute += alarm[3];
        minute += alarm[4];
        return minute.toInt();
    }
}

// The root page
void handleRoot()
{
    server.send(200, "text/html", String("<h1>Hello, World!</h1>") + String("<script>\
        document.querySelector('form').addEventListener(\
            'submit', function(event){\
                event.preventDefault();\
                const time = document.querySelector('input[name=\"newTimeStamp\"]').value;\
                const url = `/data?value=${hora}`;\
                window.location.href = url;\
            });</script>") + String("<h2>Alarm on for:") +
                                      String(alarm) + String("</h2>") + String("<form action=\"/data\" method=\"get\">\
        <input type=\"time\" name=\"newTimeStamp\">\
        <input type=\"submit\">\
    </form>"));
}

void handlePost()
{
    if (http.begin(client, curl))
    {
    }
}

void processTime(JSONVar data)
{
    if (data.hasOwnProperty("unixtime"))
    {
        unixTime = (int)data["unixtime"];
    }
    else
    {
        Serial.println("[JSON] Error on unix transform");
    }
}

void processTime()
{
    // Get the actual time
    if (http.begin(client, curl))
    {
        Serial.println("[HTTP] Connected at " + String(curl));
        // Get the code of our petition
        int httpCode = http.GET();

        if (httpCode > 0)
        {
            Serial.println("[HTTP] GET method succesfully with code " + httpCode);
            // Get the data on String and then parse to JSONVar. For next call "processTime" and give it our JSON
            JSONVar payload = JSON.parse(http.getString());
            processTime(payload);
        }
    }
    else
    {
        Serial.println("[HTTP] Failed to connect at " + String(curl));
    }
}

// When my son initialize do...
void setup()
{
    // Define the serial on 115200 bauds
    Serial.begin(115200);

    // Initial with the lastest ssid and psk
    WiFi.begin(ssid, psk);

    // You should to wait for connect, if you can't; restart.
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.println("[WiFi] Connecting failed... Rebooting...");
        delay(500);
        ESP.restart();
    }
    else
    {
        Serial.println("[WiFi] Connected!");
    }

    // We've to say mine's credentials
    Serial.print("[WiFi] Connected to ");
    Serial.println(ssid);
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());

    processTime();

    // Start on the server
    Serial.println("[HTTP] Starting http server");
    server.on("/", handleRoot);
    server.on("/data", HTTP_GET, []()
              {
        if(server.hasArg("newTimeStamp")){
            alarm = server.arg("newTimeStamp");
            server.send(HTTP_CODE_ACCEPTED, "text/plain", "You're good! Thank you! Data: " + alarm);
        } else {
            server.send(HTTP_CODE_PAYLOAD_TOO_LARGE, "text/plain", "Args missing!!!");
        } });
    server.begin();
    Serial.println("[HTTP] Server started");
}

void loop()
{
    server.handleClient();

    if (alarm != "????")
    {
        if (stamp.hour == handleTime(true) && stamp.minute == handleTime(false))
        {
            //Code for the speaker HERE
        }
    }
    delay(1000);
    stamp.getDateTime((unixTime++));
    Serial.println(stamp.hour);
    Serial.println(stamp.minute);
    Serial.println(stamp.second);
}
