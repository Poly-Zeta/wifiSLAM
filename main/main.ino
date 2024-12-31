#include "Wire.h"
#include <Arduino.h>

#define MAINLOOP_CYCLE_MS 10

#define SSD1306_PAGES_SIZE 8//0~7の8ページ(64pix/8pix)
#define SSD1306_CHARS_SIZE 16//1ページには16文字(128pix/8pix)
#define SSD1306_CHARLINEDATA_SIZE 8//1文字は8line*1Byte

#define FONTDATA_SIZE 59
#define FONTDATA_OFFSET 0x20

#define SENSORS 23
#define SENSORS_DISPLAY_OFFSET 12

TaskHandle_t thp[1];//マルチスレッドのタスクハンドル格納用
const uint8_t delay_th=MAINLOOP_CYCLE_MS-2;

const uint8_t ADDRESS_SSD1306 =  0x3C;
const uint8_t ADDRESS_BNO055  =  0x28;
const uint8_t ADDRESS_BME280  =  0x76;
const uint8_t ADDRESS_WHEELS  =  0x34;
const uint8_t ADDRESS_BlinkM  =  0x09;
const uint8_t ADDRESS_PCA9685  =  0x40;
const uint8_t ADDRESS_ADC1115  =  0x00;//配線中

const uint8_t REG_BNO055_ACC   =  0x08;
const uint8_t REG_BNO055_MAG   =  0x0E;
const uint8_t REG_BNO055_GYRO  =  0x14;
const uint8_t REG_BNO055_QUA   =  0x20;
const uint8_t REG_BNO055_LIA   =  0x28;
const uint8_t ADDRESS_ADS1115  =  0x28;

const uint8_t  ADC_BAT_ADDR =0x00;
const uint8_t  MOTOR_TYPE_ADDR =0x14;
const uint8_t  MOTOR_ENCODER_POLARITY_ADDR =0x15;
const uint8_t  MOTOR_ENCODER_POLARITY =0x00;
const uint8_t  MOTOR_FIXED_PWM_ADDR =0x1F;
const uint8_t  MOTOR_FIXED_SPEED_ADDR =0x33;
const uint8_t  MOTOR_ENCODER_TOTAL_ADDR =0x3C;
const uint8_t  MOTOR_TYPE_WITHOUT_ENCODER =0x00;
const uint8_t  MOTOR_TYPE_TT =0x01;
const uint8_t  MOTOR_TYPE_N20 =0x02;
const uint8_t  MOTOR_TYPE_JGB37_520_12V_110RPM =0x03;

//センサデータ置き場
double sensorsDataBuffer[SENSORS]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int32_t EncodeTotal[4]={0,0,0,0};

uint16_t I2CActiveFlags=0b0000000000000000;

signed long int t_fine;
uint16_t dig_T1,dig_P1;
int16_t dig_T2,dig_T3,dig_P2,dig_P3,dig_P4,dig_P5,dig_P6,dig_P7,dig_P8,dig_P9,dig_H2,dig_H4,dig_H5;
int8_t  dig_H1,dig_H3,dig_H6;

enum SensorsBufferNUM{
  GYRO_X,
  GYRO_Y,
  GYRO_Z,
  MAG_X,
  MAG_Y,
  MAG_Z,
  ACC_X,
  ACC_Y,
  ACC_Z,
  LACC_X,
  LACC_Y,
  LACC_Z,
  QW,
  QX,
  QY,
  QZ,
  HUMID,
  TEMP,
  PRESS,
  L_WHEEL,
  R_WHEEL,
  A_IN0,
  A_IN1
};

enum I2CSensorsNUM{
  IMU_BNO055,
  DISP_SSD1306,
  HUM_BME280,
  WHEELS,
  LED_BlinkM,
  SERVO_PCA9685,
  ADC_ADS1115
};

//ディスプレイ描画バッファ
uint8_t ssd1306_displayBuffer[SSD1306_PAGES_SIZE][SSD1306_CHARS_SIZE]={
  {0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60},
  {0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60},
  {0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60},
  {0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60},
  {0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60},
  {0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60},
  {0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60},
  {0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60}
};

//フォントデータ
const uint8_t fonts[FONTDATA_SIZE][SSD1306_CHARLINEDATA_SIZE]={
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},//   20
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// ! 21 nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// " 22 nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// # 23 nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// $ 24 nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// % 25 nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// & 26 nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// ' 27 nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// ( 28 nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// ) 29 nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// * 2A nodata
  {0x00,0x00,0x10,0x10,0x7C,0x10,0x08,0x00},// + 2B nodata
  {0x00,0x00,0x00,0x00,0x60,0xE0,0x00,0x00},// , 2C 
  {0x00,0x00,0x10,0x10,0x10,0x10,0x10,0x00},// - 2D
  {0x00,0x00,0x00,0x00,0x00,0xC0,0xC0,0x00},// . 2E 
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// / 2F nodata
  {0x00,0x7C,0xC2,0xA2,0x92,0x8A,0x86,0x7C},// 0 30
  {0x00,0x00,0x00,0x00,0x04,0xFE,0xFE,0x00},// 1 31
  {0x00,0x8C,0xC6,0xE2,0xA2,0x92,0x9E,0x8C},// 2 32
  {0x00,0x54,0xD6,0x92,0x92,0x92,0xFE,0x6C},// 3 33
  {0x00,0x70,0x78,0x6C,0x66,0xFE,0xFE,0x60},// 4 34
  {0x00,0x5E,0xDE,0x92,0x92,0x92,0xF2,0x62},// 5 35
  {0x00,0x7C,0xFE,0x96,0x92,0x92,0xF6,0x64},// 6 36
  {0x00,0x00,0x06,0x02,0xC2,0xF2,0x3E,0x0E},// 7 37
  {0x00,0x6C,0xFE,0x92,0x92,0x92,0xFE,0x6C},// 8 38
  {0x00,0x0C,0x5E,0x92,0x92,0x92,0xFE,0x7C},// 9 39
  {0x00,0x00,0x00,0x00,0xCC,0xCC,0x00,0x00},// : 3A
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// ; 3B nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// < 3C nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// = 3D nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// > 3E nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// ? 3F nodata
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// @ 40 nodata
  {0x00,0xE0,0xFC,0x3E,0x26,0x3E,0xFC,0xE0},// A 41
  {0x00,0xFE,0xFE,0x92,0x92,0x92,0x92,0x6C},// B 42
  {0x00,0x7C,0xEE,0xC6,0x82,0x82,0xC6,0x44},// C 43
  {0x00,0xFE,0xFE,0x82,0x82,0x82,0x82,0x7C},// D 44
  {0x00,0xFE,0xFE,0x92,0x92,0x92,0x92,0x82},// E 45
  {0x00,0xFE,0xFE,0x12,0x12,0x12,0x12,0x02},// F 46
  {0x00,0x7C,0xEE,0xC6,0x92,0x92,0xD6,0x74},// G 47
  {0x00,0xFE,0xFE,0x10,0x10,0x10,0xFE,0xFE},// H 48
  {0x00,0x00,0x82,0xFE,0xFE,0xFE,0x82,0x00},// I 49
  {0x00,0x02,0x42,0x82,0x82,0xFE,0x7E,0x02},// J 4A
  {0x00,0xFE,0xFE,0x10,0x18,0x2C,0x46,0x82},// K 4B
  {0x00,0xFE,0xFE,0x80,0x80,0x80,0x80,0x80},// L 4C
  {0x00,0xFE,0xFC,0x08,0x10,0x08,0xFC,0xFE},// M 4D
  {0x00,0xFE,0xFE,0x0C,0x10,0x60,0xFE,0xFE},// N 4E
  {0x00,0x7C,0x82,0x82,0x92,0x82,0x82,0x7C},// O 4F
  {0x00,0xFE,0xFE,0x12,0x12,0x12,0x1E,0x0C},// P 50
  {0x00,0x7C,0x82,0x82,0x92,0xA2,0x42,0xBC},// Q 51
  {0x00,0xFE,0xFE,0x12,0x32,0x72,0xDE,0x8C},// R 52
  {0x00,0x4C,0xDE,0x92,0x92,0x92,0xF6,0x64},// S 53
  {0x00,0x02,0x02,0xFE,0xFE,0xFE,0x02,0x02},// T 54
  {0x00,0x7E,0xFE,0xC0,0xC0,0xC0,0xFE,0x7E},// U 55
  {0x00,0x0E,0x3E,0xE0,0x80,0xE0,0x3E,0x0E},// V 56
  {0x00,0x1E,0x60,0xF8,0x1E,0xF8,0x60,0x1E},// W 57
  {0x00,0xC6,0xC6,0x28,0x10,0x28,0xC6,0xC6},// X 58
  {0x00,0x06,0x0C,0x18,0xF0,0x18,0x0C,0x06},// Y 59
  {0x00,0xC6,0xC6,0xE6,0xD6,0xCE,0xC6,0xC6}// Z 5A
};

//センサデータの描画　毎回全更新してると遅いのでメイン1ループあたり1要素
void SSD1306_displaySensorsData(){
  static uint8_t cnt=SENSORS_DISPLAY_OFFSET;

  uint8_t updatePage,updateChar,updateSize;

  char buf[SSD1306_CHARS_SIZE];

  //択が多いためswitch-caseに置換
  switch(cnt){
    case QW:
      dtostrf(sensorsDataBuffer[cnt],5,2,buf);
      updatePage=0;
      updateChar=2;
      updateSize=5;
      break;
    case QX:
      dtostrf(sensorsDataBuffer[cnt],5,2,buf);
      updatePage=0;
      updateChar=10;
      updateSize=5;
      break;
    case QY:
      dtostrf(sensorsDataBuffer[cnt],5,2,buf);
      updatePage=1;
      updateChar=2;
      updateSize=5;
      break;
    case QZ:
      dtostrf(sensorsDataBuffer[cnt],5,2,buf);
      updatePage=1;
      updateChar=10;
      updateSize=5;
      break;
    case HUMID:
      dtostrf(sensorsDataBuffer[cnt],5,1,buf);
      updatePage=2;
      updateChar=2;
      updateSize=5;
      break;
    case TEMP:
      dtostrf(sensorsDataBuffer[cnt],5,1,buf);
      updatePage=2;
      updateChar=10;
      updateSize=5;
      break;
    case PRESS:
      dtostrf(sensorsDataBuffer[cnt],7,2,buf);
      updatePage=3;
      updateChar=6;
      updateSize=7;
      break;
    case  L_WHEEL:
      dtostrf(sensorsDataBuffer[cnt],7,2,buf);
      updatePage=4;
      updateChar=8;
      updateSize=5;
      break;
    case R_WHEEL:
      dtostrf(sensorsDataBuffer[cnt],7,2,buf);
      updatePage=5;
      updateChar=8;
      updateSize=5;
      break;
    case A_IN0://serialreadのデバッグのため，シリアルで入ってきた変数をここに表示
      dtostrf(sensorsDataBuffer[cnt],7,2,buf);
      updatePage=6;
      updateChar=3;
      updateSize=10;
      break;
    case A_IN1://serialreadのデバッグのため，シリアルで入ってきた変数をここに表示
      dtostrf(sensorsDataBuffer[cnt],7,2,buf);
      updatePage=7;
      updateChar=3;
      updateSize=10;
      break;
  }

  // }else if(cnt==1){//lwh

  // }else if(cnt==1){//rwh

  // }
  
  for(uint8_t chars=0;chars<updateSize;chars++){
    ssd1306_displayBuffer[updatePage][updateChar+chars]=(uint8_t)buf[chars];
  }

  //書き込み
  Wire.beginTransmission(ADDRESS_SSD1306);

  //ページ指定
  Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
    Wire.write(0xB0 | updatePage); //set page start address(B0～B7)

  //表示更新幅指定っぽい
  Wire.write(0b00000000);
    Wire.write(0x21); //set Column Address
      Wire.write(8*updateChar); //Column Start Address(0-127)
      Wire.write(8*(updateChar+updateSize)); //Column Stop Address(0-127)

  Wire.endTransmission();

  for(uint8_t chars=updateChar;chars<updateChar+updateSize;chars++){//ページ内全文字を対象
    Wire.beginTransmission(ADDRESS_SSD1306);
    Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data) Max=31byte

    for(uint8_t cntByteLine=0;cntByteLine<SSD1306_CHARLINEDATA_SIZE;cntByteLine++){
      Wire.write(
        fonts[ssd1306_displayBuffer[updatePage][chars]-FONTDATA_OFFSET][cntByteLine]
      );
    }
    Wire.endTransmission();

  }

  cnt++;
  if(cnt>=SENSORS_DISPLAY_OFFSET+11){//SENSORS){
    cnt=SENSORS_DISPLAY_OFFSET;
  }
  // Serial.print("next cnt:");
  // Serial.print(cnt);
  // Serial.println();
  return;
}

//OLED SSD1306 1ページぶんの表示を更新する関数(処理速度が必要な場面では使用しない)
//Page0から順に書いていって，7まで埋まっているときは全内容をシフトアップしてから7を更新する
void SSD1306_display1LineWithShiftUp(char* input){
  static uint8_t updatePage=0;//0,1,...7と更新していく．8になったら以降はずっとシフトアップ+7を更新
  // Serial.println(input);

  if(updatePage>=SSD1306_PAGES_SIZE){//全ページ更新済なので，バッファシフトアップして7ページ目更新

    //0~6pageのバッファのシフトアップ
    for(uint8_t pages=0;pages<SSD1306_PAGES_SIZE-1;pages++){
      for(uint8_t chars=0;chars<SSD1306_CHARS_SIZE;chars++){
        ssd1306_displayBuffer[pages][chars]=ssd1306_displayBuffer[pages+1][chars];
      }
    }

    //Page7のバッファを更新
    for(uint8_t chars=0;chars<SSD1306_CHARS_SIZE;chars++){
      if(input[chars]=='\0'){
        for(uint8_t tmp=chars;tmp<SSD1306_CHARS_SIZE;tmp++){
          ssd1306_displayBuffer[SSD1306_PAGES_SIZE-1][tmp]=(uint8_t)(0x20);//SP
        }
        break;
      }else{
        ssd1306_displayBuffer[SSD1306_PAGES_SIZE-1][chars]=(uint8_t)input[chars];
      }
    }

    //書き込み
    for(uint8_t page=0;page<SSD1306_PAGES_SIZE;page++){//全ページを対象

      Wire.beginTransmission(ADDRESS_SSD1306);

      //ページ指定
      Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
        Wire.write(0xB0 | page); //set page start address(B0～B7)

      //表示更新幅指定っぽい
      Wire.write(0b00000000);
        Wire.write(0x21); //set Column Address
          Wire.write(0); //Column Start Address(0-127)
          Wire.write(127); //Column Stop Address(0-127)

      Wire.endTransmission();

      for(uint8_t chara=0;chara<SSD1306_CHARS_SIZE;chara++){//ページ内全文字を対象
        Wire.beginTransmission(ADDRESS_SSD1306);
        Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data) Max=31byte

        for(uint8_t cntByteLine=0;cntByteLine<SSD1306_CHARLINEDATA_SIZE;cntByteLine++){
          Wire.write(
            fonts[ssd1306_displayBuffer[page][chara]-FONTDATA_OFFSET][cntByteLine]
          );
        }
        Wire.endTransmission();

      }
    }
  }else{//updatePageを更新

    //updatePageのバッファを更新
    for(uint8_t chars=0;chars<SSD1306_CHARS_SIZE;chars++){
      if(input[chars]=='\0'){
        for(uint8_t tmp=chars;tmp<SSD1306_CHARS_SIZE;tmp++){
          ssd1306_displayBuffer[updatePage][tmp]=(uint8_t)(0x20);//SP
        }
        break;
      }else{
        ssd1306_displayBuffer[updatePage][chars]=(uint8_t)input[chars];
      }
    }

    //書き込み
    Wire.beginTransmission(ADDRESS_SSD1306);

    //ページ指定
    Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
      Wire.write(0xB0 | updatePage); //set page start address(B0～B7)

    //表示更新幅指定っぽい
    Wire.write(0b00000000);
      Wire.write(0x21); //set Column Address
        Wire.write(0); //Column Start Address(0-127)
        Wire.write(127); //Column Stop Address(0-127)

    Wire.endTransmission();

    for(uint8_t chara=0;chara<SSD1306_CHARS_SIZE;chara++){//ページ内全文字を対象
      Wire.beginTransmission(ADDRESS_SSD1306);
      Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data) Max=31byte

      for(uint8_t cntByteLine=0;cntByteLine<SSD1306_CHARLINEDATA_SIZE;cntByteLine++){
        Wire.write(
          fonts[ssd1306_displayBuffer[updatePage][chara]-FONTDATA_OFFSET][cntByteLine]
        );
      }
      Wire.endTransmission();

    }

    //Page番号更新
    updatePage++;
  }

  return;
}

//OLED SSD1306 表示テスト 全フォントを書いていく
void SSD1306_FullFillSample(){
  int fontCount=0;
  uint8_t chara,page,cntByteLine;

  for(page=0;page<SSD1306_PAGES_SIZE;page++){//全ページを対象

    Wire.beginTransmission(ADDRESS_SSD1306);

    //ページ指定
    Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
      Wire.write(0xB0 | page); //set page start address(B0～B7)

    //表示更新幅指定っぽい
    Wire.write(0b00000000);
      Wire.write(0x21); //set Column Address
        Wire.write(0); //Column Start Address(0-127)
        Wire.write(127); //Column Stop Address(0-127)

    Wire.endTransmission();

    for(chara=0;chara<SSD1306_CHARS_SIZE;chara++){//ページ内全文字を対象
      Wire.beginTransmission(ADDRESS_SSD1306);
      Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data) Max=31byte

      for(cntByteLine=0;cntByteLine<SSD1306_CHARLINEDATA_SIZE;cntByteLine++){
        Wire.write(fonts[fontCount][cntByteLine]);
      }
      Wire.endTransmission();

      //どの文字を表示するか決定．配列の添え字．
      fontCount++;
      if(fontCount>=FONTDATA_SIZE){
        fontCount=0;
      }
    }
  }
  return;
}

//OLED SSD1306 初期化
void SSD1306_Init(){
  Wire.beginTransmission(ADDRESS_SSD1306);//※このバイトも含め、以後、合計32byteまで送信できる

  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
  Wire.write(0xAE); //display off
  
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
    Wire.write(0xA8); //Set Multiplex Ratio  0xA8, 0x3F
      Wire.write(0b00111111); //64MUX

  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)    
    Wire.write(0xD3); //Set Display Offset 0xD3, 0x00
      Wire.write(0x00);

  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
    Wire.write(0x40); //Set Display Start Line 0x40

  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
    Wire.write(0xA0); //Set Segment re-map 0xA0/0xA1

  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
    Wire.write(0xC0); //Set COM Output Scan Direction 0xC0,/0xC8

  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
    Wire.write(0xDA); //Set COM Pins hardware configuration 0xDA, 0x02
      Wire.write(0b00010010);

  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
    Wire.write(0x81); //Set Contrast Control 0x81, default=0x7F
      Wire.write(255); //0-255

  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
    Wire.write(0xA4); //Disable Entire Display On

  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
    Wire.write(0xA6); //Set Normal Display 0xA6, Inverse display 0xA7

  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
    Wire.write(0xD5); //Set Display Clock Divide Ratio/Oscillator Frequency 0xD5, 0x80
      Wire.write(0b10000000);

  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
    Wire.write(0x20); //Set Memory Addressing Mode
      Wire.write(0x10); //Page addressing mode

  Wire.endTransmission();

  Wire.beginTransmission(ADDRESS_SSD1306);

  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
    Wire.write(0x22); //Set Page Address
      Wire.write(0); //Start page set
      Wire.write(7); //End page set

  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
    Wire.write(0x21); //set Column Address
      Wire.write(0); //Column Start Address
      Wire.write(127); //Column Stop Address

  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
    Wire.write(0x8D); //Set Enable charge pump regulator 0x8D, 0x14
      Wire.write(0x14);

  Wire.write(0b10000000); //control byte, Co bit = 1, D/C# = 0 (command)
    Wire.write(0xAF); //Display On 0xAF

  Wire.endTransmission();

  return;
}

//OLED SSD1306 表示全クリア
void SSD1306_ClearAll(){
  uint8_t i, j, k;
  
  for(i = 0; i < SSD1306_PAGES_SIZE; i++){//Page(0-7)
    Wire.beginTransmission(ADDRESS_SSD1306);
      Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
        Wire.write(0xB0 | i); //set page start address(B0～B7)
      Wire.write(0b00000000);
        Wire.write(0x21); //set Column Address
          Wire.write(0); //Column Start Address(0-127)
          Wire.write(127); //Column Stop Address(0-127)
    Wire.endTransmission();
  
    for(j = 0; j < SSD1306_CHARS_SIZE; j++){//column = 8byte x 16
      Wire.beginTransmission(ADDRESS_SSD1306);
      Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data)
      for(k = 0; k < SSD1306_CHARLINEDATA_SIZE; k++){ //continue to 31byte
        Wire.write(0x00);
      }
      Wire.endTransmission();
    }
  }
  return;
}

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
    Wire.write(REG_BNO055_MAG);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 6);
    Wire.readBytes(buffer, 6);
  sensorsDataBuffer[MAG_X] = (double)(int16_t)((((uint16_t)buffer[0]) | (((uint16_t)buffer[1]) << 8)))/16.0;
  sensorsDataBuffer[MAG_Y] = (double)(int16_t)((((uint16_t)buffer[2]) | (((uint16_t)buffer[3]) << 8)))/16.0;
  sensorsDataBuffer[MAG_Z] = (double)(int16_t)((((uint16_t)buffer[4]) | (((uint16_t)buffer[5]) << 8)))/16.0;

  Wire.beginTransmission(ADDRESS_BNO055);  
    Wire.write(REG_BNO055_ACC);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 6);
    Wire.readBytes(buffer, 6);
  sensorsDataBuffer[ACC_X] = (double)(int16_t)((((uint16_t)buffer[0]) | (((uint16_t)buffer[1]) << 8)))/100.0;
  sensorsDataBuffer[ACC_Y] = (double)(int16_t)((((uint16_t)buffer[2]) | (((uint16_t)buffer[3]) << 8)))/100.0;
  sensorsDataBuffer[ACC_Z] = (double)(int16_t)((((uint16_t)buffer[4]) | (((uint16_t)buffer[5]) << 8)))/100.0;

  Wire.beginTransmission(ADDRESS_BNO055);  
    Wire.write(REG_BNO055_LIA);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 6);
    Wire.readBytes(buffer, 6);
  sensorsDataBuffer[LACC_X] = (double)(int16_t)((((uint16_t)buffer[0]) | (((uint16_t)buffer[1]) << 8)))/100.0;
  sensorsDataBuffer[LACC_Y] = (double)(int16_t)((((uint16_t)buffer[2]) | (((uint16_t)buffer[3]) << 8)))/100.0;
  sensorsDataBuffer[LACC_Z] = (double)(int16_t)((((uint16_t)buffer[4]) | (((uint16_t)buffer[5]) << 8)))/100.0;

  Wire.beginTransmission(ADDRESS_BNO055);  
    Wire.write(REG_BNO055_QUA);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 8);
    Wire.readBytes(buffer, 8);
  sensorsDataBuffer[QW] = (double)((int16_t)((((uint16_t)buffer[1]) << 8) | ((uint16_t)buffer[0])))*(1.0 / (1 << 14));
  sensorsDataBuffer[QX] = (double)((int16_t)((((uint16_t)buffer[3]) << 8) | ((uint16_t)buffer[2])))*(1.0 / (1 << 14));
  sensorsDataBuffer[QY] = (double)((int16_t)((((uint16_t)buffer[5]) << 8) | ((uint16_t)buffer[4])))*(1.0 / (1 << 14));
  sensorsDataBuffer[QZ] = (double)((int16_t)((((uint16_t)buffer[7]) << 8) | ((uint16_t)buffer[6])))*(1.0 / (1 << 14));

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

    //NDOF(全センサ有効，自動キャリブレーション，自動フュージョンモード)で起動
    BNO055_Write(0x3d, 0x0c,   80);   //operating mode = ndof
  }

//Acc: m/s2
//[UNIT_SEL] : xxxxxxx0b

//Deg/s
//[UNIT_SEL] : xxxxxx0xb

//オイラー角 deg
//[UNIT_SEL] : xxxxx0xxb
  return;
}

//温度センサ BME280 初期化
void BME280_Init(){
  Wire.beginTransmission(ADDRESS_BME280);
  Wire.write(0xF2);
  Wire.write(0x01);
  Wire.endTransmission();

  Wire.beginTransmission(ADDRESS_BME280);
  Wire.write(0xF4);
  Wire.write((0x01<<5)|(0x01<<2)|(0x03));
  Wire.endTransmission();

  Wire.beginTransmission(ADDRESS_BME280);
  Wire.write(0xF5);
  Wire.write((0x05<<5)|(0<<2)|(0));
  Wire.endTransmission();

  BME280_readTrim();
  return;
}

void BME280_readTrim(){
  uint8_t data[32],i=0;                      // Fix 2014/04/06
  Wire.beginTransmission(ADDRESS_BME280);
  Wire.write(0x88);
  Wire.endTransmission();
  Wire.requestFrom(ADDRESS_BME280,24);       // Fix 2014/04/06
  while(Wire.available()){
    data[i] = Wire.read();
    i++;
  }
  
  Wire.beginTransmission(ADDRESS_BME280);    // Add 2014/04/06
  Wire.write(0xA1);                          // Add 2014/04/06
  Wire.endTransmission();                    // Add 2014/04/06
  Wire.requestFrom(ADDRESS_BME280,1);        // Add 2014/04/06
  data[i] = Wire.read();                     // Add 2014/04/06
  i++;                                       // Add 2014/04/06
  
  Wire.beginTransmission(ADDRESS_BME280);
  Wire.write(0xE1);
  Wire.endTransmission();
  Wire.requestFrom(ADDRESS_BME280,7);        // Fix 2014/04/06
  while(Wire.available()){
    data[i] = Wire.read();
    i++;    
  }
  dig_T1 = (data[1] << 8) | data[0];
  dig_T2 = (data[3] << 8) | data[2];
  dig_T3 = (data[5] << 8) | data[4];
  dig_P1 = (data[7] << 8) | data[6];
  dig_P2 = (data[9] << 8) | data[8];
  dig_P3 = (data[11]<< 8) | data[10];
  dig_P4 = (data[13]<< 8) | data[12];
  dig_P5 = (data[15]<< 8) | data[14];
  dig_P6 = (data[17]<< 8) | data[16];
  dig_P7 = (data[19]<< 8) | data[18];
  dig_P8 = (data[21]<< 8) | data[20];
  dig_P9 = (data[23]<< 8) | data[22];
  dig_H1 = data[24];
  dig_H2 = (data[26]<< 8) | data[25];
  dig_H3 = data[27];
  dig_H4 = (data[28]<< 4) | (0x0F & data[29]);
  dig_H5 = (data[30] << 4) | ((data[29] >> 4) & 0x0F); // Fix 2014/04/06
  dig_H6 = data[31];                                   // Fix 2014/04/06
  return;
}

signed long int BME280_calibration_Temperature(signed long int adc_T){
  signed long int var1, var2, T;
  var1 = ((((adc_T >> 3) - ((signed long int)dig_T1<<1))) * ((signed long int)dig_T2)) >> 11;
  var2 = (((((adc_T >> 4) - ((signed long int)dig_T1)) * ((adc_T>>4) - ((signed long int)dig_T1))) >> 12) * ((signed long int)dig_T3)) >> 14;
  
  t_fine = var1 + var2;
  T = (t_fine * 5 + 128) >> 8;
  return T; 
}

unsigned long int BME280_calibration_Pressure(unsigned long int adc_P){
  signed long int var1, var2;
  unsigned long int P;
  var1 = (((signed long int)t_fine)>>1) - (signed long int)64000;
  var2 = (((var1>>2) * (var1>>2)) >> 11) * ((signed long int)dig_P6);
  var2 = var2 + ((var1*((signed long int)dig_P5))<<1);
  var2 = (var2>>2)+(((signed long int)dig_P4)<<16);
  var1 = (((dig_P3 * (((var1>>2)*(var1>>2)) >> 13)) >>3) + ((((signed long int)dig_P2) * var1)>>1))>>18;
  var1 = ((((32768+var1))*((signed long int)dig_P1))>>15);
  if (var1 == 0){
      return 0;
  }    
  P = (((unsigned long int)(((signed long int)1048576)-adc_P)-(var2>>12)))*3125;
  if(P<0x80000000){
      P = (P << 1) / ((unsigned long int) var1);   
  }else{
      P = (P / (unsigned long int)var1) * 2;    
  }
  var1 = (((signed long int)dig_P9) * ((signed long int)(((P>>3) * (P>>3))>>13)))>>12;
  var2 = (((signed long int)(P>>2)) * ((signed long int)dig_P8))>>13;
  P = (unsigned long int)((signed long int)P + ((var1 + var2 + dig_P7) >> 4));
  return P;
}

unsigned long int BME280_calibration_Humidity(unsigned long int adc_H){
  signed long int v_x1;
  
  v_x1 = (t_fine - ((signed long int)76800));
  v_x1 = (((((adc_H << 14) -(((signed long int)dig_H4) << 20) - (((signed long int)dig_H5) * v_x1)) + 
            ((signed long int)16384)) >> 15) * (((((((v_x1 * ((signed long int)dig_H6)) >> 10) * 
            (((v_x1 * ((signed long int)dig_H3)) >> 11) + ((signed long int) 32768))) >> 10) + (( signed long int)2097152)) * 
            ((signed long int) dig_H2) + 8192) >> 14));
  v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * ((signed long int)dig_H1)) >> 4));
  v_x1 = (v_x1 < 0 ? 0 : v_x1);
  v_x1 = (v_x1 > 419430400 ? 419430400 : v_x1);
  return (unsigned long int)(v_x1 >> 12);   
}

//BME280 値更新
//センサ自体の更新レートは低いので，内部にカウンタを設けて更新を順繰りにする
void BME280_getRawData(){
  static uint8_t bme280_cnt=0;
  int i = 0;
  static uint8_t bme280_data[3];
  // uint8_t data[8];
  // unsigned long int buff[3];
  unsigned long int buff;
  signed long int temp_cal;
  unsigned long int press_cal,hum_cal;

//   気圧の情報はレジスタのアドレス　0xF7 0xF8 0xF9に格納されます。
// 温度の情報はレジスタのアドレス　0xFA 0xFB 0xFCに格納されます。
// 湿度の情報はレジスタのアドレス　0xFD 0xFE に格納されます。　湿度だけ2byteです。

  switch(bme280_cnt){
    case 0:
      Wire.beginTransmission(ADDRESS_BME280);
      Wire.write(0xFA);
      Wire.endTransmission();
      Wire.requestFrom(ADDRESS_BME280,3);
      Wire.readBytes(bme280_data, 3);
      break;
    case 1:
      buff=(unsigned long int)((bme280_data[0] << 12) | (bme280_data[1] << 4) | (bme280_data[2] >> 4));
      temp_cal  = BME280_calibration_Temperature(buff);
      sensorsDataBuffer[TEMP]  = (double)temp_cal  /  100.0;
      break;
    case 2:
      Wire.beginTransmission(ADDRESS_BME280);
      Wire.write(0xF7);
      Wire.endTransmission();
      Wire.requestFrom(ADDRESS_BME280,3);
      Wire.readBytes(bme280_data, 3);
      break;
    case 3:
      buff=(unsigned long int)((bme280_data[0] << 12) | (bme280_data[1] << 4) | (bme280_data[2] >> 4));
      press_cal = BME280_calibration_Pressure(buff);
      sensorsDataBuffer[PRESS] = (double)press_cal /  100.0;
      break;
    case 4:
      Wire.beginTransmission(ADDRESS_BME280);
      Wire.write(0xFD);
      Wire.endTransmission();
      Wire.requestFrom(ADDRESS_BME280,2);
      Wire.readBytes(bme280_data, 2);
      break;
    case 5:
      buff=(unsigned long int)((bme280_data[0] << 8) | bme280_data[1]);
      hum_cal   = BME280_calibration_Humidity(buff);
      sensorsDataBuffer[HUMID] = (double)hum_cal / 1024.0;
      break;
  }

  bme280_cnt++;
  if(bme280_cnt>=6){
    bme280_cnt=0;
  }
  return;
}

//センサ値のシリアル出力
void SerialOutput(){
  //起動後時間，角速度，磁気，加速度，線形加速度，四元数，(温度，湿度，気圧，左モータ回転，右モータ回転)
  Serial.print(millis());
  Serial.print(",");

  Serial.print(sensorsDataBuffer[GYRO_X]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[GYRO_Y]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[GYRO_Z]);
  Serial.print(",");
  
  Serial.print(sensorsDataBuffer[MAG_X]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[MAG_Y]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[MAG_Z]);
  Serial.print(",");
  
  Serial.print(sensorsDataBuffer[ACC_X]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[ACC_Y]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[ACC_Z]);
  Serial.print(",");
  
  Serial.print(sensorsDataBuffer[LACC_X]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[LACC_Y]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[LACC_Z]);
  Serial.print(",");
  
  Serial.print(sensorsDataBuffer[QW]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[QX]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[QY]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[QZ]);
  Serial.print(",");
  
  Serial.print(sensorsDataBuffer[TEMP]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[HUMID]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[PRESS]);
  Serial.print(",");

  Serial.print(sensorsDataBuffer[L_WHEEL]);
  Serial.print(",");
  Serial.print(sensorsDataBuffer[R_WHEEL]);
  // Serial.print(",");

  // Serial.print("2000,-2000");
  Serial.println();

  return;
}

void MotorDriver_Init(){
  Wire.beginTransmission(ADDRESS_WHEELS);
  Wire.write(MOTOR_TYPE_ADDR);
  Wire.write(MOTOR_TYPE_JGB37_520_12V_110RPM);
  Wire.endTransmission();
  delay(5);
  
  Wire.beginTransmission(ADDRESS_WHEELS);
  Wire.write(MOTOR_ENCODER_POLARITY_ADDR);
  Wire.write(MOTOR_ENCODER_POLARITY);
  Wire.endTransmission();
  delay(5);

  Wire.beginTransmission(ADDRESS_WHEELS);
  Wire.write(MOTOR_ENCODER_TOTAL_ADDR);
  for(int i = 0; i < 8; i++){
    Wire.write(0x00);
  }
  Wire.endTransmission();
  delay(5);
  return;
}

void WriteMotors(int8_t l_wheelPower,int8_t r_wheelPower){
  Wire.beginTransmission(ADDRESS_WHEELS);
  Wire.write(MOTOR_FIXED_SPEED_ADDR);
  Wire.write(l_wheelPower);
  Wire.write(r_wheelPower);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.endTransmission();
  return;
}

void readEncoders(){
  uint32_t buffer[2];

  Wire.beginTransmission(ADDRESS_WHEELS);
  Wire.write(MOTOR_ENCODER_TOTAL_ADDR);
  Wire.endTransmission();
  Wire.requestFrom(ADDRESS_WHEELS, 8);
    Wire.readBytes((uint8_t*)buffer, 8);
  Wire.endTransmission();

  sensorsDataBuffer[L_WHEEL]=(double)buffer[0];
  sensorsDataBuffer[R_WHEEL]=(double)buffer[1];

  return;
}

void setup() {
  Serial.begin(230400);
  Wire.begin();
  Wire.setClock(400000L);
  delay(10);

  //BlinkMの光を弱くしておく
  Wire.beginTransmission(ADDRESS_BlinkM);
  Wire.write(0x6f);
  Wire.write(0x6e);
  Wire.write(0x04);
  Wire.write(0x04);
  Wire.write(0x04);
  Wire.endTransmission();

  SSD1306_Init(); //OLED ssd1306 初期化
  
  SSD1306_ClearAll();
  delay(300);
  SSD1306_FullFillSample();
  delay(300);
  SSD1306_ClearAll();
  delay(300);

  SSD1306_display1LineWithShiftUp("INIT START");
  delay(300);
  SSD1306_display1LineWithShiftUp("SSD1306 SETUP");//oled SSD1306初期化演出
  delay(100);
  SSD1306_display1LineWithShiftUp("SSD1306 STANDBY");
  delay(300);
  SSD1306_display1LineWithShiftUp("BLINKM SETUP");//BlinkM初期化演出
  delay(100);
  SSD1306_display1LineWithShiftUp("BLINKM STANDBY");



  SSD1306_display1LineWithShiftUp("BNO055 SETUP");//IMU BNO055初期化
  BNO055_Init();
  SSD1306_display1LineWithShiftUp("BNO055 STANDBY");

  SSD1306_display1LineWithShiftUp("BME280 SETUP");//温度計 BME280初期化
  BME280_Init();
  SSD1306_display1LineWithShiftUp("BME280 STANDBY");

  SSD1306_display1LineWithShiftUp("MOTOR SETUP");//モータドライバ初期化
  // MotorDriver_Init();
  SSD1306_display1LineWithShiftUp("MOTOR STANDBY");
  delay(300);

  SSD1306_display1LineWithShiftUp("SETUP COMPLETE");
  delay(1000);
  SSD1306_display1LineWithShiftUp("READY");
  delay(1000);
  
  SSD1306_display1LineWithShiftUp("QW 0.00 QX 0.00");
  SSD1306_display1LineWithShiftUp("QY 0.00 QZ 0.00");
  SSD1306_display1LineWithShiftUp("HU100.0 TE 00.0");
  SSD1306_display1LineWithShiftUp("PRESS 1000.00");
  SSD1306_display1LineWithShiftUp("L-WHEEL 00000");
  SSD1306_display1LineWithShiftUp("R-WHEEL 00000");
  SSD1306_display1LineWithShiftUp("A-0 0000000000");
  SSD1306_display1LineWithShiftUp("A-1 0000000000");

  xTaskCreatePinnedToCore(Core0, "Core0", 8192, NULL, 3, &thp[0], 0); 
}

void loop() {
  unsigned long millis_buf = millis();//1ループの開始時間はとっておく
  
  //IMUの値更新
  BNO055_getRawData();

  //温度計の値更新
  BME280_getRawData();

  // //センサデータのシリアル出力
  // SerialOutput();

  //ホイールエンコーダの読み出し
  // readEncoders();

  if (Serial.available() > 0) {
    String receivedData = Serial.readStringUntil('\n'); // 改行までのデータを読み込む

    //分解
    int commaIndex = receivedData.indexOf(',');
    float linear_x,angular_z;

    if (commaIndex != -1) {
      String linearXStr = receivedData.substring(0, commaIndex);
      String angularZStr = receivedData.substring(commaIndex + 1);
      
      linear_x = linearXStr.toFloat();   // 文字列を浮動小数点数に変換
      angular_z = angularZStr.toFloat();
      
      //書き出し
      //一時的に書き出し先をセンサデータのバッファにして
      //シリアル入力をoledで確認できるようにしておく
      sensorsDataBuffer[A_IN0]=linear_x;
      sensorsDataBuffer[A_IN1]=angular_z;
    }
  }

  //センサデータのディスプレイ表示
  SSD1306_displaySensorsData();

  // Serial.print((millis()-millis_buf));//1ループの具体的処理の末尾時間はここで出す
  // Serial.print("ms");
  // Serial.println();

  //1ループはdefine MAINLOOP_CYCLE_MSの時間で出しておく
  //現在時刻-ループ頭がMAINLOOP_CYCLE_MSを超えるまで待機
  // delay(1);
  int est_clk_c1=millis()-millis_buf;
  if((est_clk_c1)<=delay_th){
    delay(delay_th-est_clk_c1);
  }
  while ((millis() - millis_buf) < MAINLOOP_CYCLE_MS){}

  // Serial.print((millis()-millis_buf));//1ループ全体の時間チェック用
  // Serial.print("ms");
  // Serial.println();
}


//SerialOutput();について
//115200bps / 10bit/char / 1000ms(1sec) *9ms =103.68char/s -> 10msサイクルだと足りてない
//伝送レート2倍にした
void Core0(void *args) {
  // Serial.begin(115200);
  unsigned long millis_buf_c0;
  int est_clk_c0;
  while (1) {
    millis_buf_c0 = millis();
    //センサデータのシリアル出力
    SerialOutput();

    // delay(1);
    est_clk_c0=millis()-millis_buf_c0;
    if((est_clk_c0)<=delay_th){
      delay(delay_th-est_clk_c0);
    }
    while ((millis() - millis_buf_c0) < MAINLOOP_CYCLE_MS){}
  }
}
