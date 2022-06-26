#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <WiFiManager.h> 
#include <ESP8266mDNS.h>
#include <FS.h>
#include <time.h>
#include "constants.h"
#include <Math.h>

//=============================================================
// DEFINITIONS
//=============================================================
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

//Char buffer to print on
char str_holder[MAX_COMMAND_IN_ONE_LIST*MAX_CHAR_IN_ONE_COMMAND];
char command_single[MAX_CHAR_IN_ONE_COMMAND];
char command_list[MAX_COMMAND_IN_ONE_LIST][MAX_CHAR_IN_ONE_COMMAND];

File logFile = SPIFFS.open(LOGFILE_PATH, "a");

//=============================================================
// HTTP HANDLERS
//=============================================================
void handleMessageReceived(){
  int arg1 = server.arg("arg1").toInt();
  int arg2 = server.arg("arg2").toInt();
  int arg3 = server.arg("arg3").toInt();
//  Serial.println("message received");
  server.send(200, "text/plain","Message received OK");
}

//=============================================================
// WEBSOCKET HANDLERS
//=============================================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{

//  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
//      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
//        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // Send the current LED status
        webSocket.sendTXT(num, "Connected to ESP", strlen("Connected to ESP"));
        break;
      }


    //=============================================================================
    // Handle list like [var1,var2,var3;var1,var2,var3;...;var1,var2,var3;]
    //=============================================================================
    case WStype_TEXT:
      {
        int var1 = 0;
        int var2 = 0;
        int var3 = 0;
        
//        Serial.printf("[%u] get Text: %s\r\n", num, payload);
  
        //====================================================
        //Read text and Execute Touch event
        //====================================================
//        sprintf(str_holder, "%s", payload);
  
        //Check if is a list
        if( str_holder[0] != '[' or str_holder[strlen(str_holder)-1] != ']'){
//          Serial.println("Not a list");
          break;
        }
  
        else{
//          Serial.println("List received");
  
          int start_pos = 1;
          int stop_pos = 1;
          int count = 0;
          int command_index = 0;
          int command_count = 0;
          int max_pos = strlen(str_holder)-2; //fini par ";]"


          for(int current_pos = 1; current_pos <= max_pos;){
              start_pos = current_pos;
  
              //===================================================
              // Recherche du delimiteur de commandes
              //===================================================
              for(int i = start_pos; i <= max_pos; i++){

                  if(str_holder[i] == ';'){
                      stop_pos = i;
                      current_pos = stop_pos + 1; //point de depart de la prochaine recherche
                      
                      //===================================================
                      // On reconstitue la commande dans command_single
                      //===================================================
                      int k = 0;
                      for(int j = start_pos; j < stop_pos; j++){
                          command_single[k] = str_holder[j];
                          k++;
                      }
                      command_single[k] = NULL;
                      
                      //===================================================
                      strcpy(command_list[command_index],command_single);

                      command_index++;
                      break;
                  }
              }
              
          }
          command_count = command_index;
          
          //================================================================
          // On print le recapitulatif des commandes et on les executes
          //================================================================
          for(int i = 0; i < command_count ; i++){

              //PARSING SINGLE COMMAND
              if(sscanf(command_list[i], "%d,%d,%d", &var1, &var2 ,&var3) <= 0){
//                  Serial.printf("Error parsing text for command: \n%s\n",command_list[i]);
                  webSocket.sendTXT(num, "Error parsing text!");
              }
              else{
                //PUT CALL TO FUNCTION DOING SOMETHING WITH var1,var2,var3
              }
        
          }
          
        }
  
        //====================================================
        webSocket.sendTXT(num, payload);
        
        break;
      }
    case WStype_BIN:
//      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
      
    default:
//      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

//=============================================================
// SETUPS
//=============================================================
void setup_serial(){
  Serial.begin(SERIAL_SPEED);
  while (!Serial) { ; }
//  Serial.println("initialisation");
}

void setup_SPIFFS(){
    if(!SPIFFS.begin()){
//    Serial.println("SPIFF initialisation failed");
  }
}

void setup_wifi(){
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it 

  WiFiManager wifiManager;
  //Uncomment to reset saved settings
//  wifiManager.researoundSettings();
  
  //set custom ip for portal
//  wifiManager.setAPStaticIPConfig(IPAddress(192,168,0,105), IPAddress(192,168,0,1), IPAddress(255,255,255,0));
  //set custom ip for server
  wifiManager.setSTAStaticIPConfig(IPAddress(192,168,0,105), IPAddress(192,168,0,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect(ACCESS_POINT_NAME);
  //or use this for auto generated name ESP + ChipIDwifiManager.setAPStaticIPConfig
  //wifiManager.autoConnect();

  //if you get here you have connected to the WiFi
//  Serial.println("connected to WiFi!");
}

void serveAllFilesInSPIFFS(){
//  Serial.println("List and serve files in SPIFFS folder:");
  Dir dir = SPIFFS.openDir("");
  int filesize = 0;
  char filename[50];
  
    while (dir.next()) {
      filesize = dir.fileSize(); //en octets
      //Put filename String in Char array needed for ServeStatic
      dir.fileName().toCharArray(filename,50); 
//      Serial.print(filename);
//      Serial.print(" - ");
//      Serial.printf("%.2f Ko\n", filesize/1000.);
      server.serveStatic(filename, SPIFFS, filename);
    }
}

void setup_server(){
  //=============================================================
  //Pages declaration
  //=============================================================
//  server.on("/message", handleMessageReceived);
  server.on("/message", handleMessageArgument);

  //=============================================================
  //Files declaration
  //=============================================================
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic(LOGFILE_PATH, SPIFFS, LOGFILE_PATH);
  serveAllFilesInSPIFFS();



  //=============================================================
  // Launch server
  //=============================================================
  server.begin();
//  Serial.println("HTTP server started");

  
  // Start the mDNS responder
  if (MDNS.begin(host_name)) {              
//    Serial.println("mDNS responder started");
    MDNS.addService("http", "tcp", 80);
  } 
  else {
//    Serial.println("Error setting up MDNS responder!");
  }
}

void setup_websocket(){
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void handleMessageArgument() {
  String message = "";

  if(server.arg("command") == "") {
    //parameter not found
    message = "0";  
  } else {
    //parameter found
    message = server.arg("command"); //get value of command
  }
  server.send(200, "text/plain", message);
  byte cb = 0;
  cb = message.toInt();
  Serial.write(cb);
}

void setup() {
  setup_serial();
  setup_wifi();
  setup_SPIFFS();
  setup_server();
  setup_websocket();

}

void loop() {
  MDNS.update();
  server.handleClient();
  webSocket.loop();  
  yield();

}
