/*
TATCO.
rabee@tatco.cc
http://tatco.cc
*/

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

boolean incoming = 0;
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDA, 0x02 };
//IPAddress ip(192, 168, 1, 10); //ENTER YOUR IP ADDRESS HERE!!!
EthernetServer server(80);


void setup()
{
  for (int i = 2; i <= 9; i++) {
    pinMode(i, OUTPUT);
  }
 
   //Ethernet.begin(mac, ip);
  Ethernet.begin(mac);
  
  server.begin();
  Serial.begin(9600);
  Serial.println(Ethernet.localIP());
  // initialize SD card
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  printIPAddress();

}

void loop()
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
       process(client);
      } // end if (client.available())
    } // end while (client.connected())
    delay(100);
    client.stop();// close the connection
  }// end if (client)

}// end if (loop)

void process(EthernetClient client) {
  // read the command
  String command = client.readStringUntil('/');

  // is "digital" command?
  if (command == "digital") {
    digitalCommand(client);
  }

  // is "analog" command?
  if (command == "analog") {
    analogCommand(client);
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
  // Read pin number
  pin = client.parseInt();
  // If the next character is a '/' it means we have an URL
  // with a value like: "/digital/13/1"
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close"); 
  }
  
  client.stop();
 
}

void analogCommand(EthernetClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/analog/5/120"
  if (client.read() == '/') {
    // Read value and execute command
    value = client.parseInt();
    analogWrite(pin, value);
    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close"); 
  }
  client.stop();
}

void allonoff(EthernetClient client) {
  int pin, value, realvalue;
  // Read pin number
  value = client.parseInt();
  for (int i = 2; i <= 9; i++) {
    digitalWrite(i, value);
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close"); 
  }
  client.stop();
}

void allstatus(EthernetClient client) {
  int r;
  int x = 2;
  client.println("HTTP/1.1 200 OK");
  client.println("<!DOCTYPE html>");
  client.println("content-type:application/json");
  client.println("Connection: close");
  client.println();
  client.println("{");
  for (int i = 0; i <= 5; i++) {
    client.print("\"A");
    client.print(i);
    client.print("\":");
    if (i == 5) {
      client.print(analogRead(i));
      client.println(",");
    }
    else {
      client.print(analogRead(i));
      client.println(",");
    }
  }

  for (int i = 2; i <= 9; i++) {
    r = digitalRead(i);
    if (r == 1) {
      x++;
    }

    client.print("\"D");
    client.print(i);
    client.print("\":");
    if (i == 9) {

      client.print(r);
      if (x == 10) {
        client.println(",");
        client.print("\"alldigital");
        client.print("\":");
        client.print(r);
      }
      else {
        client.println(",");
        client.print("\"alldigital");
        client.print("\":");
        client.print(0);
      }
    }
    else {
      client.print(digitalRead(i));
      client.println(",");
    }
  }
  //clos
  
  client.println(",");
   client.print("\"boardname");
  client.print("\":");
  client.print("\"ethernet\"");
  client.println(",");
   client.print("\"boardstatus");
  client.print("\":");
  client.println(1);
  client.println("}");
  client.stop();
  //delay(100);
}

void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}



