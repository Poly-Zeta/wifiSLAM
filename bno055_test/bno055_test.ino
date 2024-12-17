#include <Wire.h>

byte ADDRESS = 0x28;
byte EULER_REGISTER = 0x1A;

int merge(byte low, byte high){
  int result = low | (high << 8);
  if (result > 32767){
    result -= 65536;  
  }
  return result;
}

void writeToBNO(byte reg, byte val, int dly){
  Wire.beginTransmission(ADDRESS);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission(false);
  delay(dly);
}

void initBNO() {
  Wire.beginTransmission(ADDRESS);  
  Wire.write(0x00);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS, 1);
  if(Wire.read() == 0xa0){
    Serial.println("BNO055 found.");
    writeToBNO(0x3d, 0x00, 80);   //operating mode = config mode
    writeToBNO(0x3f, 0x20, 1000); //sys_trigger = rst_sys
    writeToBNO(0x3e, 0x00, 80);   //pwr_mode = normal mode
    writeToBNO(0x3f, 0x80, 1000);   //sys trigger = clk_sel ex_osc
    writeToBNO(0x3d, 0x0c, 80);   //operating mode = ndof
  }else{
    while(1){
      Serial.println("BNO055 not found..");
      delay(1000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  initBNO();
}

void loop() {
  // put your main code here, to run repeatedly:
  int euler[6];
  
  Wire.beginTransmission(ADDRESS);  
  Wire.write(EULER_REGISTER);
  Wire.endTransmission(false);
  
  Wire.requestFrom(ADDRESS, 6);
  byte buffer[6];    
  Wire.readBytes(buffer, 6);

  euler[0] = merge(buffer[0], buffer[1]);
  euler[1] = merge(buffer[2], buffer[3]);
  euler[2] = merge(buffer[4], buffer[5]);

  float yaw = float(euler[0]) / 16.0;
  float roll = float(euler[1]) / 16.0;
  float pitch = float(euler[2]) / 16.0;

  Serial.print("yaw = ");
  Serial.print(yaw);
  Serial.print(" roll = ");
  Serial.print(roll);
  Serial.print(" pitch = ");
  Serial.println(pitch);
  delay(50);
}
