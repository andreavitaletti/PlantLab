//
//    FILE: echoDouble.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.00
// PURPOSE: sends an expanded float as double to PC.
//
// Released to the public domain
//

#include <IEEE754tools.h>

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  float f = receiveDouble();
  sendDouble(f+1);
  delay(100);
}

float receiveDouble()
{
  byte x[8];
  // wait for 8 bytes
  while (Serial.available() < 8);
  for (int i=0; i<8;i++)
    x[i] = Serial.read();
  return doublePacked2Float(x);
}

void sendDouble(float number)
{
  byte x[8] = {
    0,0,0,0, 0,0,0,0          };

  float2DoublePacked(number, x);
  // simple dump, no handshake or packetizing
  for (int i=0; i<8;i++)
    Serial.write(x[i]);
}
// END OF FILE
