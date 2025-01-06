#include "Wire.h"
#include <Arduino.h>

#define MAINLOOP_CYCLE_MS 10
#define SENSORS 23
#define SENSORS_DISPLAY_OFFSET 12

const uint8_t delay_th=MAINLOOP_CYCLE_MS-2;

const uint8_t ADDRESS_BNO055   =  0x28;

const uint8_t REG_BNO055_ACC   =  0x08;
const uint8_t REG_BNO055_MAG   =  0x0E;
const uint8_t REG_BNO055_GYRO  =  0x14;
const uint8_t REG_BNO055_QUA   =  0x20;
const uint8_t REG_BNO055_LIA   =  0x28;
const uint8_t ADDRESS_ADS1115  =  0x28;

//センサデータ置き場
double sensorsDataBuffer[SENSORS]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//Gxyz,Axyz
double sensorsOffset[SENSORS]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

bool isCalibrated=false;

enum SensorsBufferNUM{
  GYRO_X, //0
  GYRO_Y, //1
  GYRO_Z, //2
  MAG_X,  //3
  MAG_Y,  //4
  MAG_Z,  //5
  ACC_X,  //6
  ACC_Y,  //7
  ACC_Z,  //8
  LACC_X, //9
  LACC_Y, //10
  LACC_Z, //11
  QW,     //12
  QX,     //13
  QY,     //14
  QZ,     //15
  HUMID,  //16
  TEMP,   //17
  PRESS,  //18
  L_WHEEL,//19
  R_WHEEL,//20
  A_IN0,  //21
  A_IN1   //22
};

//IMU BNO055 コンフィグ用write関数
void BNO055_Write(byte reg, byte value, int delayMs){
  Wire.beginTransmission(ADDRESS_BNO055);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
  delay(delayMs);
  return;
}

//IMU BNO055 値の読み出し
void BNO055_getRawData(){
  uint8_t buffer[8];

  Wire.beginTransmission(ADDRESS_BNO055);  
    Wire.write(REG_BNO055_GYRO);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 6);
    Wire.readBytes(buffer, 6);
  sensorsDataBuffer[GYRO_X] = (double)(int16_t)((((uint16_t)buffer[0]) | (((uint16_t)buffer[1]) << 8)))/16.0;
  sensorsDataBuffer[GYRO_Y] = (double)(int16_t)((((uint16_t)buffer[2]) | (((uint16_t)buffer[3]) << 8)))/16.0;
  sensorsDataBuffer[GYRO_Z] = (double)(int16_t)((((uint16_t)buffer[4]) | (((uint16_t)buffer[5]) << 8)))/16.0;

  Wire.beginTransmission(ADDRESS_BNO055);  
    Wire.write(0x61);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 6);
    Wire.readBytes(buffer, 6);
  sensorsOffset[GYRO_X] = (double)(int16_t)((((uint16_t)buffer[0]) | (((uint16_t)buffer[1]) << 8)))/16.0;
  sensorsOffset[GYRO_Y] = (double)(int16_t)((((uint16_t)buffer[2]) | (((uint16_t)buffer[3]) << 8)))/16.0;
  sensorsOffset[GYRO_Z] = (double)(int16_t)((((uint16_t)buffer[4]) | (((uint16_t)buffer[5]) << 8)))/16.0;

//   Wire.beginTransmission(ADDRESS_BNO055);  
//     Wire.write(REG_BNO055_MAG);
//   Wire.endTransmission(false);
//   Wire.requestFrom(ADDRESS_BNO055, 6);
//     Wire.readBytes(buffer, 6);
//   sensorsDataBuffer[MAG_X] = (double)(int16_t)((((uint16_t)buffer[0]) | (((uint16_t)buffer[1]) << 8)))/16.0;
//   sensorsDataBuffer[MAG_Y] = (double)(int16_t)((((uint16_t)buffer[2]) | (((uint16_t)buffer[3]) << 8)))/16.0;
//   sensorsDataBuffer[MAG_Z] = (double)(int16_t)((((uint16_t)buffer[4]) | (((uint16_t)buffer[5]) << 8)))/16.0;

  Wire.beginTransmission(ADDRESS_BNO055);  
    Wire.write(REG_BNO055_ACC);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 6);
    Wire.readBytes(buffer, 6);
  sensorsDataBuffer[ACC_X] = (double)(int16_t)((((uint16_t)buffer[0]) | (((uint16_t)buffer[1]) << 8)))/100.0;
  sensorsDataBuffer[ACC_Y] = (double)(int16_t)((((uint16_t)buffer[2]) | (((uint16_t)buffer[3]) << 8)))/100.0;
  sensorsDataBuffer[ACC_Z] = (double)(int16_t)((((uint16_t)buffer[4]) | (((uint16_t)buffer[5]) << 8)))/100.0;

    Wire.beginTransmission(ADDRESS_BNO055);  
    Wire.write(0x55);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 6);
    Wire.readBytes(buffer, 6);
  sensorsOffset[ACC_X] = (double)(int16_t)((((uint16_t)buffer[0]) | (((uint16_t)buffer[1]) << 8)))/100.0;
  sensorsOffset[ACC_Y] = (double)(int16_t)((((uint16_t)buffer[2]) | (((uint16_t)buffer[3]) << 8)))/100.0;
  sensorsOffset[ACC_Z] = (double)(int16_t)((((uint16_t)buffer[4]) | (((uint16_t)buffer[5]) << 8)))/100.0;

  Wire.beginTransmission(ADDRESS_BNO055);  
    Wire.write(REG_BNO055_LIA);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 6);
    Wire.readBytes(buffer, 6);
  sensorsDataBuffer[LACC_X] = (double)(int16_t)((((uint16_t)buffer[0]) | (((uint16_t)buffer[1]) << 8)))/100.0;
  sensorsDataBuffer[LACC_Y] = (double)(int16_t)((((uint16_t)buffer[2]) | (((uint16_t)buffer[3]) << 8)))/100.0;
  sensorsDataBuffer[LACC_Z] = (double)(int16_t)((((uint16_t)buffer[4]) | (((uint16_t)buffer[5]) << 8)))/100.0;

//   Wire.beginTransmission(ADDRESS_BNO055);  
//     Wire.write(REG_BNO055_QUA);
//   Wire.endTransmission(false);
//   Wire.requestFrom(ADDRESS_BNO055, 8);
//     Wire.readBytes(buffer, 8);
//   sensorsDataBuffer[QW] = (double)((int16_t)((((uint16_t)buffer[1]) << 8) | ((uint16_t)buffer[0])))*(1.0 / (1 << 14));
//   sensorsDataBuffer[QX] = (double)((int16_t)((((uint16_t)buffer[3]) << 8) | ((uint16_t)buffer[2])))*(1.0 / (1 << 14));
//   sensorsDataBuffer[QY] = (double)((int16_t)((((uint16_t)buffer[5]) << 8) | ((uint16_t)buffer[4])))*(1.0 / (1 << 14));
//   sensorsDataBuffer[QZ] = (double)((int16_t)((((uint16_t)buffer[7]) << 8) | ((uint16_t)buffer[6])))*(1.0 / (1 << 14));

  return;
}

//IMU BNO055 初期化
void BNO055_Init(){
  Wire.beginTransmission(ADDRESS_BNO055);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(ADDRESS_BNO055, 1);
  if(Wire.read() == 0xa0){
    //設定
    BNO055_Write(0x3d, 0x00,   80);   //operating mode = config mode

    //システムリセット
    BNO055_Write(0x3f, 0x20, 1000);   //sys_trigger = rst_sys

    //電源モードノーマル
    BNO055_Write(0x3e, 0x00,   80);   //pwr_mode = normal mode

    //外部発振器の使用らしい．要らんのでは
    // BNO055_Write(0x3f, 0x80, 1000);   //sys trigger = clk_sel ex_osc

    //下に変更：NDOF(全センサ有効，自動キャリブレーション，自動フュージョンモード)で起動
    // BNO055_Write(0x3d, 0x0c,   80);   //operating mode = ndof
    
    //IMU(加速度とジャイロ，自動キャリブレーション，自動フュージョンモード)で起動
    BNO055_Write(0x3d, 0x08,   80);   //operating mode = ndof
  }

//Acc: m/s2
//[UNIT_SEL] : xxxxxxx0b

//Deg/s
//[UNIT_SEL] : xxxxxx0xb

//オイラー角 deg
//[UNIT_SEL] : xxxxx0xxb
  return;
}

//センサ値のシリアル出力
void SerialOutput(){
//   Serial.print(millis());
//   Serial.print(",");

//   Serial.print(sensorsDataBuffer[GYRO_X]);
//   Serial.print(",");
//   Serial.print(sensorsDataBuffer[GYRO_Y]);
//   Serial.print(",");
//   Serial.print(sensorsDataBuffer[GYRO_Z]);
//   Serial.print(",");

//     Serial.print(sensorsOffset[GYRO_X]);
//   Serial.print(",");
//   Serial.print(sensorsOffset[GYRO_Y]);
//   Serial.print(",");
//   Serial.print(sensorsOffset[GYRO_Z]);
//   Serial.print(",");
  
//   Serial.print(sensorsDataBuffer[MAG_X]);
//   Serial.print(",");
//   Serial.print(sensorsDataBuffer[MAG_Y]);
//   Serial.print(",");
//   Serial.print(sensorsDataBuffer[MAG_Z]);
//   Serial.print(",");
  
//   Serial.print(sensorsDataBuffer[ACC_X]-sensorsOffset[ACC_X]);
//   Serial.print(",");
//   Serial.print(sensorsDataBuffer[ACC_Y]-sensorsOffset[ACC_Y]);
//   Serial.print(",");
//   Serial.print(sensorsDataBuffer[ACC_Z]-sensorsOffset[ACC_Z]);
//   Serial.print(",");

//   Serial.print(sensorsOffset[ACC_X]);
//   Serial.print(",");
//   Serial.print(sensorsOffset[ACC_Y]);
//   Serial.print(",");
//   Serial.print(sensorsOffset[ACC_Z]);
//   Serial.print(",");

  Serial.print(sensorsDataBuffer[LACC_X]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[LACC_Y]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[LACC_Z]);
  Serial.print(",");


  // Serial.print(",");

  // Serial.print("2000,-2000");
  Serial.println();

  return;
}


void setup() {
  Serial.begin(230400);
  Wire.begin();
  Wire.setClock(100000L);
  delay(10);

  BNO055_Init();
}

void loop() {
    unsigned long millis_buf = millis();//1ループの開始時間はとっておく
    uint8_t buffer[8];

    if(!isCalibrated){
        Wire.beginTransmission(ADDRESS_BNO055);  
        Wire.write(0x35);
        Wire.endTransmission(false);
        Wire.requestFrom(ADDRESS_BNO055, 1);
        Wire.readBytes(buffer, 1);
        // Serial.print(buffer[0],BIN);
        // Serial.print(",");
        Serial.print((buffer[0] & 0b00111100));
        Serial.print(",");
        if((buffer[0] & 0b00111100) == 0b00111100){
            isCalibrated=true;
            Wire.beginTransmission(ADDRESS_BNO055);
            Wire.write(0x00);
            Wire.endTransmission();

            Wire.requestFrom(ADDRESS_BNO055, 1);
            if(Wire.read() == 0xa0){
                //設定
                BNO055_Write(0x3d, 0x00,   80);   //operating mode = config mode

                Wire.beginTransmission(ADDRESS_BNO055);  
                Wire.write(0x55);
                Wire.endTransmission(false);
                Wire.requestFrom(ADDRESS_BNO055, 6);
                Wire.readBytes(buffer, 6);
                sensorsOffset[ACC_X] = (double)(int16_t)((((uint16_t)buffer[0]) | (((uint16_t)buffer[1]) << 8)))/100.0;
                sensorsOffset[ACC_Y] = (double)(int16_t)((((uint16_t)buffer[2]) | (((uint16_t)buffer[3]) << 8)))/100.0;
                sensorsOffset[ACC_Z] = (double)(int16_t)((((uint16_t)buffer[4]) | (((uint16_t)buffer[5]) << 8)))/100.0;

                Serial.print(sensorsOffset[ACC_X]);
                Serial.print(",");
                Serial.print(sensorsOffset[ACC_Y]);
                Serial.print(",");
                Serial.print(sensorsOffset[ACC_Z]);
                Serial.print(",");
                Serial.println();
                
                delay(3000);
                
                //IMU(加速度とジャイロ，自動キャリブレーション，自動フュージョンモード)で起動
                BNO055_Write(0x3d, 0x08,   80);   //operating mode = ndof
            }
        }
    }
    
    //IMUの値更新
    BNO055_getRawData();

    SerialOutput();

    //1ループはdefine MAINLOOP_CYCLE_MSの時間で出しておく
    //現在時刻-ループ頭がMAINLOOP_CYCLE_MSを超えるまで待機
    // delay(1);
    int est_clk_c1=millis()-millis_buf;
    if((est_clk_c1)<=delay_th){
        delay(delay_th-est_clk_c1);
    }
    while ((millis() - millis_buf) < MAINLOOP_CYCLE_MS){}

}


