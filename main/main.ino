#include "Wire.h"
#include <Arduino.h>

#define MAINLOOP_CYCLE_MS 10

const uint8_t ADDRESS_SSD1306 =  0x3C;

const uint8_t fonts[40][8]={
  {0x00,0x7C,0xC2,0xA2,0x92,0x8A,0x86,0x7C},//0
  {0x00,0x00,0x00,0x00,0x04,0xFE,0xFE,0x00},//1
  {0x00,0x8C,0xC6,0xE2,0xA2,0x92,0x9E,0x8C},//2
  {0x00,0x54,0xD6,0x92,0x92,0x92,0xFE,0x6C},//3
  {0x00,0x70,0x78,0x6C,0x66,0xFE,0xFE,0x60},//4
  {0x00,0x5E,0xDE,0x92,0x92,0x92,0xF2,0x62},//5
  {0x00,0x7C,0xFE,0x96,0x92,0x92,0xF6,0x64},//6
  {0x00,0x00,0x06,0x02,0xC2,0xF2,0x3E,0x0E},//7
  {0x00,0x6C,0xFE,0x92,0x92,0x92,0xFE,0x6C},//8
  {0x00,0x0C,0x5E,0x92,0x92,0x92,0xFE,0x7C},//9
  {0x00,0xE0,0xFC,0x3E,0x26,0x3E,0xFC,0xE0},//A
  {0x00,0xFE,0xFE,0x92,0x92,0x92,0x92,0x6C},//B
  {0x00,0x7C,0xEE,0xC6,0x82,0x82,0xC6,0x44},//C
  {0x00,0xFE,0xFE,0x82,0x82,0x82,0x82,0x7C},//D
  {0x00,0xFE,0xFE,0x92,0x92,0x92,0x92,0x82},//E
  {0x00,0xFE,0xFE,0x12,0x12,0x12,0x12,0x02},//F
  {0x00,0x7C,0xEE,0xC6,0x92,0x92,0xD6,0x74},//G
  {0x00,0xFE,0xFE,0x10,0x10,0x10,0xFE,0xFE},//H
  {0x00,0x00,0x82,0xFE,0xFE,0xFE,0x82,0x00},//I
  {0x00,0x02,0x42,0x82,0x82,0xFE,0x7E,0x02},//J
  {0x00,0xFE,0xFE,0x10,0x18,0x2C,0x46,0x82},//K
  {0x00,0xFE,0xFE,0x80,0x80,0x80,0x80,0x80},//L
  {0x00,0xFE,0xFC,0x08,0x10,0x08,0xFC,0xFE},//M
  {0x00,0xFE,0xFE,0x0C,0x10,0x60,0xFE,0xFE},//N
  {0x00,0x7C,0x82,0x82,0x92,0x82,0x82,0x7C},//O
  {0x00,0xFE,0xFE,0x12,0x12,0x12,0x1E,0x0C},//P
  {0x00,0x7C,0x82,0x82,0x92,0xA2,0x42,0xBC},//Q
  {0x00,0xFE,0xFE,0x12,0x32,0x72,0xDE,0x8C},//R
  {0x00,0x4C,0xDE,0x92,0x92,0x92,0xF6,0x64},//S
  {0x00,0x02,0x02,0xFE,0xFE,0xFE,0x02,0x02},//T
  {0x00,0x7E,0xFE,0xC0,0xC0,0xC0,0xFE,0x7E},//U
  {0x00,0x0E,0x3E,0xE0,0x80,0xE0,0x3E,0x0E},//V
  {0x00,0x1E,0x60,0xF8,0x1E,0xF8,0x60,0x1E},//W
  {0x00,0xC6,0xC6,0x28,0x10,0x28,0xC6,0xC6},//X
  {0x00,0x06,0x0C,0x18,0xF0,0x18,0x0C,0x06},//Y
  {0x00,0xC6,0xC6,0xE6,0xD6,0xCE,0xC6,0xC6},//Z
  {0x00,0x00,0x00,0x00,0xCC,0xCC,0x00,0x00},//:
  {0x00,0x00,0x00,0x00,0x60,0xE0,0x00,0x00},//,
  {0x00,0x00,0x00,0x00,0x00,0xC0,0xC0,0x00},//.
  {0x00,0x00,0x10,0x10,0x10,0x10,0x10,0x00}//-
};

void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.print("setup start");
  Serial.println();

  Serial.print("SSD1306 setup");
  Serial.println();
  SSD1306_Init(); //OLED ssd1306 初期化
  Serial.print("SSD1306 cleanup");
  Serial.println();
  SSD1306_ClearAll();
  SSD1306_FullFillSample();
  SSD1306_ClearAll();
  Serial.print("SSD1306 set end");
  Serial.println();

  //セットアップ表示とスクロールループをここに仕込む
  
  //ここまで．

  Serial.print("setup end");
  Serial.println();
}

void loop() {
  unsigned long millis_buf = millis();//1ループの開始時間はとっておく




  // Serial.print((millis()-millis_buf));//1ループの具体的処理の末尾時間はここで出す
  // Serial.print("ms");
  // Serial.println();

  //1ループはdefine MAINLOOP_CYCLE_MSの時間で出しておく
  //現在時刻-ループ頭がMAINLOOP_CYCLE_MSを超えるまで待機
  while ((millis() - millis_buf) < MAINLOOP_CYCLE_MS){}
}

void SSD1306_FullFillSample(){//表示テスト．
  int fontCount=0;
  uint8_t chara,page,cntByteLine;

  for(page=0;page<8;page++){//全ページを対象

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

    for(chara=0;chara<16;chara++){//ページ内全文字を対象
      Wire.beginTransmission(ADDRESS_SSD1306);
      Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data) Max=31byte

      for(cntByteLine=0;cntByteLine<8;cntByteLine++){
        Wire.write(fonts[fontCount][cntByteLine]);
      }
      Wire.endTransmission();

      Serial.print(page);
      Serial.print(",");
      Serial.print(chara);
      Serial.print(",");
      Serial.print(fontCount);
      Serial.println();

      //どの文字を表示するか決定．配列の添え字．
      fontCount++;
      if(fontCount>=40){
        fontCount=0;
      }
    }
  }
}

void SSD1306_Init(){
  Wire.setClock(400000);
  delay(10);
    
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
}

//**************************************************
void SSD1306_ClearAll(){
  uint8_t i, j, k;
  
  for(i = 0; i < 8; i++){//Page(0-7)
    Wire.beginTransmission(ADDRESS_SSD1306);
      Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
        Wire.write(0xB0 | i); //set page start address(B0～B7)
      Wire.write(0b00000000);
        Wire.write(0x21); //set Column Address
          Wire.write(0); //Column Start Address(0-127)
          Wire.write(127); //Column Stop Address(0-127)
    Wire.endTransmission();
  
    for(j = 0; j < 16; j++){//column = 8byte x 16
      Wire.beginTransmission(ADDRESS_SSD1306);
      Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data)
      for(k = 0; k < 8; k++){ //continue to 31byte
        Wire.write(0x00);
      }
      Wire.endTransmission();
    }
  }
}
//**************************************************



