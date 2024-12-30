/*  File: BasicWiFiStation.ino
**
**  Not fully de-bugged. 
** Taken largely from: https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/#2
*/

// Load Wi-Fi library
#include <WiFi.h>  //  https://docs.arduino.cc/libraries/wifi/

// Replace with your network credentials (as alternative to manual entry in setup)
String ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Set web server port number to 80
WiFiServer server(80);  // Arduino lib

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

int pagenumber = 0;

void scanWifiNetworks(){
  Serial.println("WiFi scan start...");

  // WiFi.scanNetworks will return the number of networks found
  String first9[9];
  int n = WiFi.scanNetworks();
  Serial.println("Scan done:");
  if (n == 0) {
      Serial.println("no networks found");
  } else {
    Serial.print(n); Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      first9[i]= WiFi.SSID(i);
      Serial.print(first9[i]);
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");

      delay(10);
    }
  }
  String ssid_num = serialInput("Select WiFi host, 1-9");
  ssid = first9[ssid_num.charAt(0)-49] ;
}


// Variable to store the HTTP request
String header;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000; // max elapsed time in msec

//////
void SendHTML(WiFiClient client) {  // This is where you determine what the webpage contains, i.e. the final output
  // Display the HTML web page.  All of this can be modified as desired
  client.println("<!DOCTYPE html><html>");
  client.println("<head>     </head>");
  client.println("<body>");
  client.println("<H1>My ESP webpage</H1><br><br>");
  client.println("<div id='newhtml'></div><br>");
  // standard button used to call function
  client.println("<p><button type=\"button\" onclick=\"myFunction(1)\">Send some text</button></p>");
  client.println("<p><button type=\"button\" onclick=\"myFunction(2)\">Send other text</button></p>");
  client.println("<p><input type=\"text\" id=\"textinput\" value = \"text\"><button type=\"button\" onclick=\"myFunction(0)\">Send this text</button></p>");
  // standard hyperlink text  
  //  client.println("<br><a location.href=\"othertext\">Send other text</a>");

  client.println("</body></html>");//

  // optional javascript used to create function; you can use <a> as well
  client.println("<script>");
  client.println("  pagenum=0;");
  if (pagenumber==0) client.println("  document.getElementById(\"newhtml\").innerHTML = \"initial page\";");
  if (pagenumber==1) client.println("  document.getElementById(\"newhtml\").innerHTML = \"page 1\";");
  if (pagenumber==2) client.println("  document.getElementById(\"newhtml\").innerHTML = \"page 2\";");  
  client.println("  function myFunction(n){ ");
  client.println("    if (n==0) location.href = document.getElementById(\"textinput\").value ;"); 
  client.println("    if (n==1) location.href =  \"sometext\";");  
  client.println("    if (n==2) location.href =  \"othertext\";"); 
  client.println("  } ");

  client.println("</script>");
}
//////
void initWiFiStation() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.print("WiFi connected.");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}
///////
String serialInput(String prompt){   
  Serial.println(prompt);     
  while (Serial.available() == 0) {}     //wait for data available
  String x = Serial.readString();  //read until timeout
  x.trim();                        // remove any \r \n whitespace at the end of the String
  return x;
}


////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(2000);
  Serial.println();
  Serial.println("Basic WiFi Server");

  scanWifiNetworks();

  
  //String ssid = serialInput("Enter WIFI ss_id");
  
  String password = serialInput("Enter password");
  Serial.println(ssid + " " + password);


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);  //?  delay(1000); 
    Serial.print(".");  //opt:   Serial.println("Connecting to WiFi...");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.print("WiFi connected.  Device web server IP address: ");
  Serial.println(WiFi.localIP());

  delay(1000);

  server.begin();

}


void loop(){
    
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) 
  {                             // If a new client connects,
      char c;
      currentTime = millis();
      previousTime = currentTime;
      //Serial.println("New Client.");        // print a message out in the serial port
      String currentLine = "";                // make a String to hold incoming data from the client
      
      while (client.connected() && currentTime - previousTime <= timeoutTime) 
      {  // loop while the client's connected
        currentTime = millis();
        if (client.available()) 
        
        {        // if there are one or more bytes to read from the client,
          c = client.read();             // then read a byte        
          //Serial.write(c);             // print it out the serial monitor (optional)
          header += c;
          if (c == '\n') 
          {                    // if the byte is a newline character        

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
              
              pagenumber = 0;
              if (header.indexOf("GET /sometext") >= 0) 
              {   
                
                pagenumber = 1;

                // User clicked hyperlink in html page. Use this section to cause action, e.g. GPIO functions

              }
              if (header.indexOf("GET /othertext") >= 0) 
              {   
                
                pagenumber = 2;

                // User clicked hyperlink in html page. Use this section to cause action, e.g. GPIO functions

              }


              int endcharnum = header.indexOf(" HTTP");
              String receivedtext = header.substring(5,endcharnum);
              if (receivedtext != "favicon.ico") {
                Serial.println("text received: " + receivedtext);
                if (pagenumber>0) {
                  Serial.println("sending page " + String(pagenumber));
                }
              }           

              // Display the HTML web page
              
              SendHTML(client);

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
          {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
          }
        }
      }
      // Clear the header variable
      if (currentLine != "") {Serial.println("current line: ");Serial.println(currentLine);}
      header = "";
      // Close the connection
      client.stop();
      //Serial.println("Client disconnected.");
      //Serial.println("");
      
  } // end if (client)
}  // end loop
