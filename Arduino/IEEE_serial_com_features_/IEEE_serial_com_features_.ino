// IEEE754 management from http://playground.arduino.cc/Main/IEEE754tools

// Datastream calculation of moments from http://www.johndcook.com/blog/skewness_kurtosis/

#include <IEEE754tools.h>

int n;
float M1, M2, M3, M4;
const int chunk_size = 1024;
float val;

void setup()
{
  Serial.begin(115200);
  Clear();
}

void loop()
{
  val = receiveDouble();
  sendDouble(val);
  Push(val);
  if (n==chunk_size){
    sendDouble(Variance());
    sendDouble(Skewness());
    Clear();
  }
  delay(50);
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


void Clear()
{
  n = 0;
  M1 = M2 = M3 = M4 = 0.0;
}

float getValue() {
  float value = -1;
  while (Serial.available() >= 5) {
    if (Serial.read() == 0xff) {
      value = (Serial.read() << 24) | (Serial.read() << 16) | (Serial.read() << 8) | (Serial.read());
    }
  }
  return value;
}

void Push(float x)
{
    float delta, delta_n, delta_n2, term1;
 
    int n1 = n;
    n++;
    delta = x - M1;
    delta_n = delta / n;
    delta_n2 = delta_n * delta_n;
    term1 = delta * delta_n * n1;
    M1 += delta_n;
    M4 += term1 * delta_n2 * (n*n - 3*n + 3) + 6 * delta_n2 * M2 - 4 * delta_n * M3;
    M3 += term1 * delta_n * (n - 2) - 3 * delta_n * M2;
    M2 += term1;
}

float Variance() 
{
    return M2/(n-1.0);
}

float Mean()
{
    return M1;
}

float StandardDeviation()
{
    return sqrt( Variance() );
}

float Skewness() 
{
    return sqrt(float(n)) * M3/ pow(M2, 1.5);
}

float Kurtosis()
{
    return float(n)*M4 / (M2*M2) - 3.0;
}
// END OF FILE
