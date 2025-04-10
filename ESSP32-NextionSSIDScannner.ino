#include <WiFi.h>
#include "Nextion.h"
#include <Preferences.h>

Preferences preferences;
// WiFi credentials
char selectedSSID[32] = "";  // Placeholder for selected SSID
char password[32] ="";

#define RX2 16 // Communication with Nextion
#define TX2 17

// Nextion components untuk HMI Client 5"
NexButton BtnScan = NexButton(0, 6, "BtnScan");
NexButton BtnConnect = NexButton(0, 7, "BtnConnect");
NexButton BtnDiscc= NexButton(0, 9, "BtnDiscc");
NexCombo CmbSSID = NexCombo(0, 4, "CmbSSID");
NexText TxtStatus = NexText(0, 8, "TxtStatus");
NexText TxtPassword = NexText(0, 5, "TxtPassword");

NexTouch *nex_listen_list[] = {
  &BtnConnect,
  &BtnDiscc,
  &BtnScan,
  NULL
};

// Button callbacks
void BtnScanPopCallback(void *ptr) {
  Serial.println("BtnScanPopCallback");
  TxtStatus.setText("Scanning for SSIDs...");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    TxtStatus.setText("No networks found.");
    return;
  }
  String SSIDs = "";
  for (int i = 0; i < n; ++i) {
    if (i > 0) SSIDs += "\r\n";
    SSIDs += WiFi.SSID(i);
    Serial.println(WiFi.SSID(i));  // Debugging
  }
  String cmdTxt = String("CmbSSID.txt=\"") + String(n) + " Networks\"";
  sendCommand(cmdTxt.c_str());
  String cmdPath = String("CmbSSID.path=\"") + SSIDs + "\"";
  sendCommand(cmdPath.c_str());
  if (!recvRetCommandFinished()) {
    Serial.println("Error updating ComboBox.");
    TxtStatus.setText("Error updating ComboBox.");
    return;
  }
  TxtStatus.setText("Scan complete. Select SSID.");
}



void BtnConnectPopCallback(void *ptr) {
  Serial.println("BtnConnectPopCallback");
  CmbSSID.getSelectedText(selectedSSID, sizeof(selectedSSID));
  TxtPassword.getText(password, sizeof(password));
  if (strcmp(selectedSSID, "Select SSID") == 0 || strlen(selectedSSID) == 0) {
    TxtStatus.setText("Select a valid SSID.");
    return;
  }

  Serial.printf("Selected SSID: %s\n", selectedSSID);
  TxtStatus.setText("Connecting to WiFi...");
  WiFi.begin(selectedSSID, password);

  unsigned long wifiTimeout = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiTimeout < 10000) {
  delay(1000);
  TxtStatus.setText("Connecting...");
  }
    if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Connected to SSID : %s\n", WiFi.SSID().c_str());
    Serial.print("Alamat IP: ");
    Serial.println(WiFi.localIP());
    IPAddress ip = WiFi.localIP();
    String message = "Wifi Connected with IP Address : " + ip.toString();
    TxtStatus.setText(message.c_str());
    } else {
      TxtStatus.setText("WiFi connection failed.");
      delay(500);
      TxtStatus.setText("IDLE");
  }
}

void BtnDisccPopCallback(void *ptr) {
  Serial.println("BtnDisccPopCallback");
  stopConnection();
}


void stopConnection() {
  TxtStatus.setText("Stopping connection...");
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    TxtStatus.setText("Disconnected from WiFi.");
  }
  delay(500);
  TxtStatus.setText("IDLE");
}


void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  nexInit();  
  BtnConnect.attachPop(BtnConnectPopCallback, &BtnConnect);
  BtnDiscc.attachPop(BtnDisccPopCallback, &BtnDiscc);
  BtnScan.attachPop(BtnScanPopCallback, &BtnScan);
  TxtStatus.setText("System ready. Press SCAN.");
  Serial.println("System ready.");
  delay(1000);
}

void loop() {
  nexLoop(nex_listen_list); 
  delay(1000);
}







