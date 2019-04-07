/*
  Title  : Ethernet Kit
  version: V4.2
  Contact: info@tatco.cc
  Done By: TATCO Inc.
  github : https://github.com/rabee2050/arduino-ethernet
  Youtube: http://tatco.cc

  App - Pro Version:
  iOS    : https://itunes.apple.com/us/app/ethernet-pro/id1330484246?ls=1&mt=8
  Android: https://play.google.com/store/apps/details?id=com.arduino.ethernetkitpro

  App - Free Version:
  iOS    : https://itunes.apple.com/us/app/arduino-ethernet-kit/id1089537034?mt=8
  Android: https://play.google.com/store/apps/details?id=com.arduino.ethernetkit&hl=en

  Important Note for UNO or Leonardo:  Any additional user built-in functions will lead to unstabllity issues
  due of memory shortage. Recommend to use Arduino MEGA in that case.

  Release Notes:
  - V1   Created 10 Oct 2015
  - V2   Updated 04 Nov 2016
  - V3   Updated 05 Oct 2017
  - V4   Updated 07 Oct 2018
  - V4.2 Updated 07 Apr 2019 / Minor Changes

  Note:
  1- This sketch compatable with Eathernet shield and Wiznet W5100
  2- Tested with Mega, Uno, Leo
  3- Uno & Leo pins# 10, 11, 12, 13 used for ethernet shield
  4- Mega Pins# 10, 50, 51, 52, 53 used for ethernet shield

*/

#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>


#define lcdSize 3 //this will define number of LCD on the phone app
String protectionPassword = ""; //This will not allow anyone to add or control your board.
String boardType;


byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDA, 0x02 };//Must be changed if you are using more than one board on the same network.
//IPAddress ip(192, 168, 1, 100); //Uncomment for fixed IP
EthernetServer server(80);
EthernetClient client;

char  pinsMode[54];
int pinsValue[54];
Servo servoArray[53];

String lcd[lcdSize];
String httpAppJsonOk = "HTTP/1.1 200 OK\r\n content-type:application/json \r\n\r\n";
unsigned long serialTimer = millis();
float appBuildVersion = 4.2;

void setup(void)
{

  Serial.begin(115200);
  Serial.println(F("Please wait for IP... "));
//  Ethernet.begin( mac,ip);// Uncomment for fixed IP
  Ethernet.begin(mac); // Comment for fixed IP
  server.begin();
  Serial.println(Ethernet.localIP());
  boardInit();
}

void loop(void)
{

  lcd[0] = "Test 1 LCD";// you can send any data to your mobile phone.
  lcd[1] = "Test 2 LCD";// you can send any data to your mobile phone.
  lcd[2] = analogRead(1);//  send analog value of A1

  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        process(client);
        delay(100);
        client.flush();
        client.stop();
      }
    }
  }

  serialPrintIpAddress();
  update_input();
}

void process(EthernetClient client) {
  String getString = client.readStringUntil('/');
  String arduinoString = client.readStringUntil('/');
  String command = client.readStringUntil('/');

  if (command == "digital") {
    digitalCommand(client);
  }

  if (command == "pwm") {
    pwmCommand(client);
  }

  if (command == "servo") {
    servoCommand(client);
  }

  if (command == "terminal") {
    terminalCommand(client);
  }

  if (command == "mode") {
    modeCommand(client);
  }

  if (command == "allonoff") {
    allonoff(client);
  }

  if (command == "password") {
    changePassword(client);
  }

  if (command == "allstatus") {
    allstatus(client);
  }

}

void changePassword(EthernetClient client) {
  String data = client.readStringUntil('/');
  protectionPassword = data;
  client.print(httpAppJsonOk);
}

void terminalCommand(EthernetClient client) {//Here you recieve data form app terminal
  String data = client.readStringUntil('/');
  client.print(httpAppJsonOk + "Ok from Arduino " + String(random(1, 100)));
  Serial.println(data);
}

void digitalCommand(EthernetClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    pinsValue[pin] = value;
    client.print(httpAppJsonOk + value);
  }
}

void pwmCommand(EthernetClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    pinsValue[pin] = value;
    client.print(httpAppJsonOk + value);
  }
}

void servoCommand(EthernetClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    servoArray[pin].write(value);
    pinsValue[pin] = value;
    client.print(httpAppJsonOk + value);
  }
}

void modeCommand(EthernetClient client) {
  String  pinString = client.readStringUntil('/');
  int pin = pinString.toInt();
  String mode = client.readStringUntil('/');
  if (mode != "servo") {
    servoArray[pin].detach();
  };

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    pinsMode[pin] = 'o';
    pinsValue[pin] = 0;
    allstatus(client);
  }
  if (mode == "push") {
    pinsMode[pin] = 'm';
    pinsValue[pin] = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    allstatus(client);
  }
  if (mode == "schedule") {
    pinsMode[pin] = 'c';
    pinsValue[pin] = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    allstatus(client);
  }

  if (mode == "input") {
    pinsMode[pin] = 'i';
    pinsValue[pin] = 0;
    pinMode(pin, INPUT);
    allstatus(client);
  }

  if (mode == "pwm") {
    pinsMode[pin] = 'p';
    pinsValue[pin] = 0;
    pinMode(pin, OUTPUT);
    analogWrite(pin, 0);
    allstatus(client);
  }

  if (mode == "servo") {
    pinsMode[pin] = 's';
    pinsValue[pin] = 0;
    servoArray[pin].attach(pin);
    servoArray[pin].write(0);
    allstatus(client);
  }
}

void allonoff(EthernetClient client) {
  int value = client.parseInt();
  client.print(httpAppJsonOk);

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (pinsMode[i] == 'o') {
      digitalWrite(i, value);
      pinsValue[i] = value;
    }
  }
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (pinsMode[i] == 'o') {
      digitalWrite(i, value);
      pinsValue[i] = value;
    }
  }
#endif

}

void allstatus(EthernetClient client) {
  int digitalArraySize, analogArraySize;
  if (boardType == "mega") {
    digitalArraySize = 53;
    analogArraySize = 15;
  } else {
    digitalArraySize = 13;
    analogArraySize = 5;
  }
  String dataResponse;
  dataResponse += F("HTTP/1.1 200 OK \r\n");
  dataResponse += F("content-type:application/json \r\n\r\n");
  dataResponse += "{";

  dataResponse += "\"m\":[";//m for mode
  for (byte i = 0; i <= digitalArraySize; i++) {
    dataResponse += "\"";
    dataResponse += pinsMode[i];
    dataResponse += "\"";
    if (i != digitalArraySize)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"v\":[";//v for value
  for (byte i = 0; i <= digitalArraySize; i++) {
    dataResponse += pinsValue[i];
    if (i != digitalArraySize)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"a\":[";//a for analog value
  for (byte i = 0; i <= analogArraySize; i++) {
    dataResponse += analogRead(i);
    if (i != analogArraySize)dataResponse += ",";
  }
  dataResponse += "],";

  dataResponse += "\"l\":[";//l for LCD value
  for (byte i = 0; i <= lcdSize - 1; i++) {
    dataResponse += "\"";
    dataResponse += lcd[i];
    dataResponse += "\"";
    if (i != lcdSize - 1)dataResponse += ",";
  }
  dataResponse += "],";
  dataResponse += "\"t\":\""; //t for Board Type .
  dataResponse += boardType;
  dataResponse += "\",";
  dataResponse += "\"b\":\""; //b for app build version .
  dataResponse += appBuildVersion;
  dataResponse += "\",";
  dataResponse += "\"p\":\""; // p for Password.
  dataResponse += protectionPassword;
  dataResponse += "\"";
  dataResponse += "}";
  client.print(dataResponse);
}

void serialPrintIpAddress() {
  if (Serial.read() > 0) {
    if (millis() - serialTimer > 2000) {
      Serial.println();
      Serial.println("IP address is:");
      Serial.println(Ethernet.localIP());
    }
    serialTimer = millis();
  }

}

void update_input() {
  for (byte i = 0; i < sizeof(pinsMode); i++) {
    if (pinsMode[i] == 'i') {
      pinsValue[i] = digitalRead(i);
    }
  }
}

void boardInit() {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__SAM3X8E__) //Mega or due
  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 || i == 10 || i == 50 || i == 51 || i == 52 || i == 53) {
      pinsMode[i] = 'x';
      pinsValue[i] = 0;
    }
    else {
      pinsMode[i] = 'o';
      pinsValue[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)//Leo or Uno
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1  || i == 10 || i == 11 || i == 12 || i == 13 ) {
      pinsMode[i] = 'x';
      pinsValue[i] = 0;
    }
    else {
      pinsMode[i] = 'o';
      pinsValue[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
#endif

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  boardType = "uno";
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) || defined(__SAM3X8E__)
  boardType = "mega";
#elif defined(__AVR_ATmega32U4__)
  boardType = "leo";
#else
  boardType = "uno";
#endif

}
