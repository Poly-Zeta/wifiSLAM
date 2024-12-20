#include "Wire.h"
#include <Arduino.h>

#define MAINLOOP_CYCLE_MS 10

#define SSD1306_PAGES_SIZE 8//0~7の8ページ(64pix/8pix)
#define SSD1306_CHARS_SIZE 16//1ページには16文字(128pix/8pix)
#define SSD1306_CHARLINEDATA_SIZE 8//1文字は8line*1Byte

#define FONTDATA_SIZE 59
#define FONTDATA_OFFSET 0x20

#define SENSORS 11

const uint8_t ADDRESS_SSD1306 =  0x3C;
const uint8_t ADDRESS_BNO055  =  0x28;

const uint8_t REG_BNO055_ACC   =  0x08;
const uint8_t REG_BNO055_MAG   =  0x0E;
const uint8_t REG_BNO055_GYRO  =  0x14;
const uint8_t REG_BNO055_QUA   =  0x20;
const uint8_t REG_BNO055_LIA   =  0x28;

//qx,qy,qz,qw,humit,temp,press,l-odom,r-odom,a-in1,a-in2
float system_sensorsDataBuffer[SENSORS]={0,0,0,0,0,0,0,0,0,0,0};

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

void SSD1306_displaySensorsData(double qw,double qx,double qy,double qz){//,float humit, float temp, float pressure, int lwheel, int rwheel){
  static uint8_t cnt=0;

  uint8_t updatePage,updateChar,updateSize;

  char buf[SSD1306_CHARS_SIZE];

  if(cnt==0){//qw
    dtostrf(qw,5,2,buf);
    updatePage=0;
    updateChar=2;
    updateSize=5;
    // Serial.print(qw);
  }else if(cnt==1){//qx
    dtostrf(qx,5,2,buf);
    updatePage=0;
    updateChar=10;
    updateSize=5;
    // Serial.print(qx);
  }else if(cnt==2){//qy
    dtostrf(qy,5,2,buf);
    updatePage=1;
    updateChar=2;
    updateSize=5;
    // Serial.print(qy);
  }else if(cnt==3){//qz
    dtostrf(qz,5,2,buf);
    updatePage=1;
    updateChar=10;
    updateSize=5;
    // Serial.print(qz);
  }
  // else if(cnt==1){//humit

  // }else if(cnt==1){//temp

  // }else if(cnt==1){//pressure

  // }else if(cnt==1){//lwh

  // }else if(cnt==1){//rwh

  // }
  
  // Serial.print("->");
  // Serial.print(buf);
  // Serial.println();


  //updatePageのバッファを更新
  // Serial.print("before:");
  // for(uint8_t chars=0;chars<16;chars++){
  //   Serial.print((char)ssd1306_displayBuffer[updatePage][chars]);
  // }
  // Serial.println();s
  for(uint8_t chars=0;chars<updateSize;chars++){
    ssd1306_displayBuffer[updatePage][updateChar+chars]=(uint8_t)buf[chars];

    // Serial.print("chars:");
    // Serial.print(chars);
    // Serial.print(", (uint8_t)buf[chars]:");
    // Serial.print((uint8_t)buf[chars]);
    // Serial.print(", buf[chars]:");
    // Serial.print(buf[chars]);
    // Serial.print(",");
    // Serial.println();
  }
  // Serial.print("after :");
  // for(uint8_t chars=0;chars<16;chars++){
  //   Serial.print((char)ssd1306_displayBuffer[updatePage][chars]);
  // }
  //   Serial.println();

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
  if(cnt>=4){
    cnt=0;
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

int BNO055_Merge(byte low, byte high){
  int result = low | (high << 8);

  return result;
}

void BNO055_Write(byte reg, byte value, int delayMs){
  Wire.beginTransmission(ADDRESS_BNO055);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
  delay(delayMs);
  return;
}

void BNO055_getRawData(){
//todo
  return;

}

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



void setup() {
  Wire.begin();
  Wire.setClock(400000);
  delay(10);

  Serial.begin(115200);
  // Serial.print("setup start");
  // Serial.println();

  // Serial.print("SSD1306 setup");
  // Serial.println();
  SSD1306_Init(); //OLED ssd1306 初期化
  
  // Serial.print("SSD1306 cleanup");
  // Serial.println();
  SSD1306_ClearAll();
  delay(300);
  SSD1306_FullFillSample();
  delay(300);
  SSD1306_ClearAll();
  delay(300);



  // Serial.print("SSD1306 set end");
  // Serial.println();
  SSD1306_display1LineWithShiftUp("INIT START");
  delay(300);
  SSD1306_display1LineWithShiftUp("SSD1306 SETUP");
  delay(100);
  SSD1306_display1LineWithShiftUp("SSD1306 STANDBY");



  SSD1306_display1LineWithShiftUp("BNO055 SETUP");//IMU BNO055初期化
  // delay(300);
  BNO055_Init();
  SSD1306_display1LineWithShiftUp("BNO055 STANDBY");
  // SSD1306_display1LineWithShiftUp("BNO055 SKIP");


  SSD1306_display1LineWithShiftUp("BME280 SETUP");
  delay(300);
  // SSD1306_display1LineWithShiftUp("BME280 STANDBY");
  SSD1306_display1LineWithShiftUp("BME280 SKIP");
  delay(300);

  // Serial.print("setup end");
  // Serial.println();
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
  SSD1306_display1LineWithShiftUp("---------------");
  SSD1306_display1LineWithShiftUp("---------------");
}

void loop() {
  unsigned long millis_buf = millis();//1ループの開始時間はとっておく
  
  byte buffer[8];    

  //IMUの読み出し
  // -> TODO:書き込み先の配列を引数にした関数にする
  //    センサの生データを関数でもらってきて，main側で/16とかすればよい
  //    レジスタ番地，書き込み先ポインタ，サイズを引数にする
  //角速度，磁気，加速度，線形加速度，四元数
  double GYRO[3];
  Wire.beginTransmission(ADDRESS_BNO055);  
    Wire.write(REG_BNO055_GYRO);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 6);
    Wire.readBytes(buffer, 6);
  GYRO[0] = double(BNO055_Merge(buffer[0], buffer[1]))/16.0;
  GYRO[1] = double(BNO055_Merge(buffer[2], buffer[3]))/16.0;
  GYRO[2] = double(BNO055_Merge(buffer[4], buffer[5]))/16.0;

  double MAG[3];
  Wire.beginTransmission(ADDRESS_BNO055);  
    Wire.write(REG_BNO055_MAG);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 6);
    Wire.readBytes(buffer, 6);
  MAG[0] = double(BNO055_Merge(buffer[0], buffer[1]))/16.0;
  MAG[1] = double(BNO055_Merge(buffer[2], buffer[3]))/16.0;
  MAG[2] = double(BNO055_Merge(buffer[4], buffer[5]))/16.0;

  double ACC[3];
  Wire.beginTransmission(ADDRESS_BNO055);  
    Wire.write(REG_BNO055_ACC);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 6);
    Wire.readBytes(buffer, 6);
  ACC[0] = double(BNO055_Merge(buffer[0], buffer[1]))/100.0;
  ACC[1] = double(BNO055_Merge(buffer[2], buffer[3]))/100.0;
  ACC[2] = double(BNO055_Merge(buffer[4], buffer[5]))/100.0;

  // double LIA[3];
  // Wire.beginTransmission(ADDRESS_BNO055);  
  //   Wire.write(REG_BNO055_LIA);
  // Wire.endTransmission(false);
  // Wire.requestFrom(ADDRESS_BNO055, 6);
  //   Wire.readBytes(buffer, 6);
  // LIA[0] = double(BNO055_Merge(buffer[0], buffer[1]))/100.0;
  // LIA[1] = double(BNO055_Merge(buffer[2], buffer[3]))/100.0;
  // LIA[2] = double(BNO055_Merge(buffer[4], buffer[5]))/100.0;

  double QUA[4];
  Wire.beginTransmission(ADDRESS_BNO055);  
    Wire.write(REG_BNO055_QUA);
  Wire.endTransmission(false);
  Wire.requestFrom(ADDRESS_BNO055, 8);
    Wire.readBytes(buffer, 8);
  QUA[0] = (int16_t)((((uint16_t)buffer[1]) << 8) | ((uint16_t)buffer[0]))*(1.0 / (1 << 14));
  QUA[1] = (int16_t)((((uint16_t)buffer[3]) << 8) | ((uint16_t)buffer[2]))*(1.0 / (1 << 14));
  QUA[2] = (int16_t)((((uint16_t)buffer[5]) << 8) | ((uint16_t)buffer[4]))*(1.0 / (1 << 14));
  QUA[3] = (int16_t)((((uint16_t)buffer[7]) << 8) | ((uint16_t)buffer[6]))*(1.0 / (1 << 14));

  //サンプル QUAT->EULAR(XYZ)
  // double ysqr = QUA[2] * QUA[2];
  // // roll (x-axis rotation)
  // double t0 = +2.0 * (QUA[0] * QUA[1] + QUA[2] * QUA[3]);
  // double t1 = +1.0 - 2.0 * (QUA[1] * QUA[1] + ysqr);
  // double roll = atan2(t0, t1)*57.2957795131;
  // // pitch (y-axis rotation)
  // double t2 = +2.0 * (QUA[0] * QUA[2] - QUA[3] * QUA[1]);
  // t2 = t2 > 1.0 ? 1.0 : t2;
  // t2 = t2 < -1.0 ? -1.0 : t2;
  // double pitch = asin(t2)*57.2957795131;
  // // yaw (z-axis rotation)
  // double t3 = +2.0 * (QUA[0] * QUA[3] + QUA[1] * QUA[2]);
  // double t4 = +1.0 - 2.0 * (ysqr + QUA[3] * QUA[3]);  
  // double yaw = atan2(t3, t4)*57.2957795131;



  //センサデータのシリアル出力コーナー
  //起動後時間，角速度，磁気，加速度，(線形加速度，)四元数，(温度，湿度，気圧，左モータ回転，右モータ回転)
  Serial.print(millis());
  Serial.print(",");

  Serial.print(GYRO[0]);
  Serial.print(",");
  Serial.print(GYRO[1]);
  Serial.print(",");
  Serial.print(GYRO[2]);
  Serial.print(",");
  
  Serial.print(MAG[0]);
  Serial.print(",");
  Serial.print(MAG[1]);
  Serial.print(",");
  Serial.print(MAG[2]);
  Serial.print(",");
  
  Serial.print(ACC[0]);
  Serial.print(",");
  Serial.print(ACC[1]);
  Serial.print(",");
  Serial.print(ACC[2]);
  Serial.print(",");
  
  // Serial.print(LIA[0]);
  // Serial.print(",");
  // Serial.print(LIA[1]);
  // Serial.print(",");
  // Serial.print(LIA[2]);
  // Serial.print(",");
  
  Serial.print(QUA[0]);
  Serial.print(",");
  Serial.print(QUA[1]);
  Serial.print(",");
  Serial.print(QUA[2]);
  Serial.print(",");
  Serial.print(QUA[3]);
  // Serial.print(",");
  
  // Serial.print(roll);
  // Serial.print(",");
  // Serial.print(pitch);
  // Serial.print(",");
  // Serial.print(yaw);
  // Serial.print(",");

  Serial.println();

  SSD1306_displaySensorsData(QUA[0],QUA[1],QUA[2],QUA[3]);

  // Serial.print((millis()-millis_buf));//1ループの具体的処理の末尾時間はここで出す
  // Serial.print("ms");
  // Serial.println();

  //1ループはdefine MAINLOOP_CYCLE_MSの時間で出しておく
  //現在時刻-ループ頭がMAINLOOP_CYCLE_MSを超えるまで待機
  while ((millis() - millis_buf) < MAINLOOP_CYCLE_MS){}
  // Serial.print((millis()-millis_buf));//1ループ全体の時間チェック用
  // Serial.print("ms");
  // Serial.println();
}
