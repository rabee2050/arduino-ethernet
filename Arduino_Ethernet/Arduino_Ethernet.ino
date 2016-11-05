/*
  TATCO Inc.
  Contact:
  info@tatco.cc
  

  created 10 Oct 2015
  by Rabee Alhattawi
  modified 04 Nov 2016
  by Rabee Alhattawi
  https://github.com/rabee2050/Arduino-Ethernet-Kit.git

  Note:
  1- This sketch compatable with Eathernet shield and Wiznet W5100
  2- Tested with Mega, Uno, Leo
  3- Uno & Leo pins# 10, 11, 12, 13 used for ethernet shield
  4- Mega Pins# 10, 50, 51, 52, 53 used for ethernet shield
*/

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetBonjour.h>//to resolve host names via MDNS (Multicast DNS)

byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDA, 0x02 };
//IPAddress ip(192, 168, 1, 10); //Uncomment for fixed IP or leave for DHCP
EthernetServer httpServer(80);
EthernetClient client;

char mode_action[54];
int mode_val[54];

void setup(void)
{
  //  while (!Serial) {
  //    ;// wait for serial port to connect. Needed for Leonardo only
  //  }
  Serial.begin(9600);
  Serial.println(F("Please wait for IP... "));
  //Ethernet.begin(mac, ip);// Uncomment for fixed IP
  Ethernet.begin(mac); // Comment for fixed IP
  httpServer.begin();
  EthernetBonjour.begin("ethernet");//Insted of IP you can use http://ethernet.local inside the app for iOS only
  EthernetBonjour.addServiceRecord("ethernet", 80, MDNSServiceTCP);
  Serial.println(Ethernet.localIP());

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 || i==10 || i == 50 || i == 51 || i == 52 || i == 53) {
      mode_action[i] = 'x';
      mode_val[i] = 'x';
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }

#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1  || i == 10 || i == 11 || i == 12 || i == 13 ) {
      mode_action[i] = 'x';
      mode_val[i] = 'x';
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
#endif



}

void loop(void)
{
  EthernetBonjour.run();
  EthernetClient client = httpServer.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        update_input();
        process(client);
      }
    }
    delay(100);
    client.flush();
    client.stop();
  }
}

void process(EthernetClient client) {
  String a = client.readStringUntil('/');
  a = client.readStringUntil('/');
  String command = client.readStringUntil('/');

  if (command == "digital") {
    digitalCommand(client);
  }

  if (command == "analog") {
    analogCommand(client);
  }

  if (command == "mode") {
    modeCommand(client);
  }

  if (command == "allonoff") {
    allonoff(client);
  }

  if (command == "allstatus") {
    allstatus(client);
  }

}


void digitalCommand(EthernetClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    mode_val[pin] = value;

    client.println(F("HTTP/1.1 200 OK"));
    client.println(F("Content-Type: text/html"));
    client.print(value);
    client.println(F("Connection: close"));
    client.stop();
  }

}

void analogCommand(EthernetClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    mode_val[pin] = value;
  }
  client.stop();
}

void modeCommand(EthernetClient client) {
  int pin;
  pin = client.parseInt();

  String mode = client.readStringUntil(' ');
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  if (mode == "/input") {
    pinMode(pin, INPUT);
    mode_action[pin] = 'i';
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as INPUT!"));
  }

  if (mode == "/output") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'o';
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as OUTPUT!"));
  }

  if (mode == "/pwm") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'p';
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as PWM!"));
  }
  client.stop();
}

void allonoff(EthernetClient client) {
  int pin, value;
  value = client.parseInt();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i]=value;
    }
  }
#endif
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i]=value;
    }
  }
#endif
client.stop();

}

void allstatus(EthernetClient client) {
  //Send all data in JSON format
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("<!DOCTYPE html>"));
  client.println(F("content-type:application/json"));
  client.println(F("Connection: close"));
  client.println();
  client.println(F("{"));

  client.print(F("\"mode\":["));
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    client.print(F("\""));
    client.print(mode_action[i]);
    client.print(F("\""));
    if (i != 53)client.print(F(","));
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    client.print(F("\""));
    client.print(mode_action[i]);
    client.print(F("\""));
    if (i != 13)client.print(F(","));
  }
#endif
  client.println(F("],"));

  client.print(F("\"mode_val\":["));
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    client.print(mode_val[i]);
    if (i != 53)client.print(F(","));
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    client.print(mode_val[i]);
    if (i != 13)client.print(F(","));
  }
#endif
  client.println(F("],"));

  client.print(F("\"analog\":["));
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 15; i++) {
    client.print(analogRead(i));
    if (i != 15)client.print(",");

  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 5; i++) {
    client.print(analogRead(i));
    if (i != 5)client.print(",");
  }
#endif
  client.println("],");

  client.print(F("\"boardname\":\""));
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
  client.println(F("kit_mega\","));
#endif
#if defined(__AVR_ATmega32U4__)//Leo
  client.println(F("kit_leo\","));
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega16U4__)//UNO
  client.println(F("kit_uno\","));
#endif
  client.println(F("\"boardstatus\":1"));
  client.println(F("}"));
  client.stop();
}

void update_input() {
  for (int i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);

    }
  }
}
