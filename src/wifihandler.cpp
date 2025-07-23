#include <wifihandler.h>
#include <ESPmDNS.h>
// Load Wi-Fi library
#include <WiFi.h>

// Replace with your network credentials
const char *ssid = "hajo";
const char *password = "1234567890";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

extern const uint8_t index_html_start[] asm("_binary_data_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_data_index_html_end");
size_t html_len = 0;

WifiHandler::WifiHandler()
{
    html_len = (size_t)index_html_end - (size_t)index_html_start;
}

void WifiHandler::Init()
{
    // Connect to Wi-Fi network with SSID and password
    Serial.print("Setting AP (Access Point)…");

    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    server.begin();
    MDNS.begin("set");
}


void WifiHandler::HandleRequests(DisplaySettings &dispSettings, BoatStats &boatStats, bool* requestDisplayUpdate)
{
    WiFiClient client = server.available(); // Listen for incoming clients

    if (client)
    {                                  // If a new client connects,
        Serial.println("New Client."); // print a message out in the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                header += c;
                if (c == '\n')
                { // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();

                        // turns the GPIOs on and off
                        if (header.indexOf("GET /settings") >= 0)
                        {
                            String settingsString = "knots:" + String(dispSettings.useKnots) + ";full:" +
                                                    String(dispSettings.fullRefreshTime) + ";short:" +
                                                    String(dispSettings.speedRefreshTime)+ ";lang:" +
                                                    String(dispSettings.language);
                            Serial.println("getting settings");
                            client.println(settingsString);
                        }
                        else if (header.indexOf("GET /modify") >= 0)
                        {
                            int startIndex = 0;
                            while (startIndex >= 0)
                            {
                                int endIndex = header.indexOf('\n', startIndex);
                                if (endIndex == -1)
                                    endIndex = header.length(); // Last line

                                String line = header.substring(startIndex, endIndex);

                                // Check if line starts with keyword
                                if (line.startsWith("GET"))
                                {
                                    if (line.indexOf("?") >= 0)
                                    {
                                        String queryString = line.substring(line.indexOf("?") + 1, line.indexOf(" HTTP/"));
                                        Serial.println("QUERY: " + queryString);
                                        ParseAndUpdateSettings(queryString, dispSettings);
                                    }
                                }

                                startIndex = endIndex + 1;
                                if (startIndex >= header.length())
                                    break;
                            }
                            dispSettings.SaveData();
                            *requestDisplayUpdate = true;
                            client.println("MODIFIED");
                        }else if (header.indexOf("GET /reset") >= 0) {
                            boatStats.Reset();
                            client.println("RESET SUCCESSFUL");
                        }
                        else
                        {
                            client.write(index_html_start, html_len);
                        }
                        // The HTTP response ends with another blank line
                        client.println();
                        // Break out of the while loop
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }
            }
        }
        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println("");
    }
}

void WifiHandler::ParseAndUpdateSettings(String query, DisplaySettings &settings)
{
    int startIndex = 0;
    int delimiterIndex;
    bool lastParsed = false;
    int parsedCount = 0;

    while (!lastParsed)
    {
        delimiterIndex = query.indexOf("&", startIndex);
        if (delimiterIndex == -1)
        {
            lastParsed = true;
            delimiterIndex = query.length();
        }

        String token = query.substring(startIndex, delimiterIndex);
        Serial.println("token: " + token);
        String value_name = token.substring(0, token.indexOf("="));
        String value = token.substring(token.indexOf("=") + 1, token.length());
        Serial.println("name: " + value_name);
        Serial.println("value: " + value);
        startIndex = delimiterIndex + 1;

        if (value_name == "knots")
        {
            if (value == "1")
            {
                settings.useKnots = true;
            }
            else
            {
                settings.useKnots = false;
            }
        }
        else if (value_name == "full")
        {
            settings.fullRefreshTime = value.toInt();
        }
        else if (value_name == "short")
        {
            settings.speedRefreshTime = value.toInt();
        }else if (value_name == "lang")
        {
            settings.language = static_cast<Language>(value.toInt());
        }
    }
}