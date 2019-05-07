/*
  Done By TATCO Inc.

  Contact:
  info@tatco.cc

  Release Notes:
  - Created 10 Oct 2015
  - V2 Updated 04 Nov 2016
  - V3 Updated 05 Oct 2017

  Note:
  1- This sketch compatable with Eathernet shield and Wiznet W5100
  2- Tested with Mega, Uno, Leo
  3- Uno & Leo pins# 10, 11, 12, 13 used for ethernet shield
  4- Mega Pins# 10, 50, 51, 52, 53 used for ethernet shield
  5- EthernetBonjour not completely tested, stability issues have to be considered.

*/

#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>
//#include <EthernetBonjour.h>//to resolve host names via MDNS (Multicast DNS)


#define lcd_size 3 //this will define number of LCD on the phone app
int refresh_time = 15; //the data will be updated on the app every 5 seconds.


byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDA, 0x02 };
//IPAddress ip(192, 168, 1, 10); //Uncomment for fixed IP or leave for DHCP
EthernetServer httpServer(80);
EthernetClient client;

char mode_action[54];
int mode_val[54];
Servo myServo[53];

String mode_feedback;
String lcd[lcd_size];
String api, channel, notification, user_id;

String httpOk = "HTTP/1.1 200 OK\r\n Content-Type: text/plain \r\n\r\n";

void setup(void)
{
  while (!Serial) {
    ;// wait for serial port to connect. Needed for Leonardo only
  }
  Serial.begin(9600);
  Serial.println(F("Please wait for IP... "));
  //Ethernet.begin(mac, ip);// Uncomment for fixed IP
  Ethernet.begin(mac); // Comment for fixed IP
  httpServer.begin();
  //  EthernetBonjour.begin("ethernet");//Insted of IP you can use hostname http://ethernet.local to connect in the app.-->> for iOS only
  //  EthernetBonjour.addServiceRecord("ethernet", 80, MDNSServiceTCP);
  Serial.println(Ethernet.localIP());
  boardInit();
}

void loop(void)
{

  lcd[0] = "Test 1 LCD";// you can send any data to your mobile phone.
  lcd[1] = "Test 2 LCD";// you can send any data to your mobile phone.
  lcd[2] = analogRead(1);//  send analog value of A1

  //  EthernetBonjour.run();

  EthernetClient client = httpServer.available();
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
  update_input();
}

void process(EthernetClient client) {
  String getString = client.readStringUntil('/');
  String arduinoString = client.readStringUntil('/');
  String command = client.readStringUntil('/');

  if (command == "terminal") {
    terminalCommand(client);
  }

  if (command == "digital") {
    digitalCommand(client);
  }

  if (command == "analog") {
    analogCommand(client);
  }

  if (command == "servo") {
    servo(client);
  }

  if (command == "mode") {
    modeCommand(client);
  }

  if (command == "allonoff") {
    allonoff(client);
  }

  if (command == "refresh") {
    refresh(client);
  }

  if (command == "allstatus") {
    allstatus(client);
  }

}



void terminalCommand(EthernetClient client) {//Here you recieve data form app terminal
  String data = client.readStringUntil('/');
  Serial.println(data);
  client.print(httpOk);
}

void refresh(EthernetClient client) {
  int value;
  value = client.parseInt();
  refresh_time = value;
  client.print(httpOk);
}

void digitalCommand(EthernetClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    mode_val[pin] = value;
    client.print(httpOk);
  }
}

void analogCommand(EthernetClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    mode_val[pin] = value;
    client.print(httpOk);
  }
}

void servo(EthernetClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    myServo[pin].write(value);
    mode_val[pin] = value;
    client.print(httpOk);
  }
}

void modeCommand(EthernetClient client) {
  int pin = client.parseInt();
  String mode = client.readStringUntil(' ');
  myServo[pin].detach();
  client.print(httpOk);

  if (mode == "/input") {
    pinMode(pin, INPUT);
    mode_action[pin] = 'i';
    mode_val[pin] = 0;
    digitalWrite(pin, LOW);
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as INPUT!"));
  }

  if (mode == "/output") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'o';
    mode_val[pin] = 0;
    digitalWrite(pin, LOW);
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as OUTPUT!"));
  }

  if (mode == "/pwm") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'p';
    mode_val[pin] = 0;
    digitalWrite(pin, LOW);
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as PWM!"));
  }

  if (mode == "/servo") {
    digitalWrite(pin, LOW);
    myServo[pin].attach(pin);
    mode_action[pin] = 's';
    mode_val[pin] = 0;
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as SERVO!"));
  }
}

void allonoff(EthernetClient client) {
  int value = client.parseInt();
  client.print(httpOk);

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
#endif

}

void allstatus(EthernetClient client) {
  //Sending all data in JSON format
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("content-type:application/json"));
  client.println(F("Connection: close"));
  client.println();
  client.println(F("{"));

  client.print(F("\"m\":["));//m for Pin Mode
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

  client.print(F("\"v\":["));// v for Mode value
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

  client.print(F("\"a\":["));// a For Analog
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


  client.print("\"l\":[");// l for LCD
  for (byte i = 0; i <= lcd_size - 1; i++) {
    client.print("\"");
    client.print(lcd[i]);
    client.print("\"");
    if (i != lcd_size - 1)client.print(",");
  }
  client.println("],");

  client.print("\"f\":\"");// f for Feedback.
  client.print(mode_feedback);
  client.println("\",");
  client.print("\"t\":\"");//t for refresh Time .
  client.print(refresh_time);
  client.println("\"");
  client.println(F("}"));
  //  client.stop();
}

void update_input() {
  for (byte i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);
    }
  }
}

void boardInit() {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 || i == 10 || i == 50 || i == 51 || i == 52 || i == 53) {
      mode_action[i] = 'x';
      mode_val[i] = 0;
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }

#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)//Leo or Uno
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1  || i == 10 || i == 11 || i == 12 || i == 13 ) {
      mode_action[i] = 'x';
      mode_val[i] = 0;
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
#endif


}
