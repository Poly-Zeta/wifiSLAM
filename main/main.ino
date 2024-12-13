#include "Wire.h"
#include <Arduino.h>

#define MAINLOOP_CYCLE_MS 10

#define SSD1306_PAGES_SIZE 8//0~7の8ページ(64pix/8pix)
#define SSD1306_CHARS_SIZE 16//1ページには16文字(128pix/8pix)
#define SSD1306_CHARLINEDATA_SIZE 8//1文字は8line*1Byte

#define FONTDATA_SIZE 59
#define FONTDATA_OFFSET 0x20

const uint8_t ADDRESS_SSD1306 =  0x3C;

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
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},// + 2B nodata
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

//OLED SSD1306 1ページぶんの表示を更新する関数(処理速度が必要な場面では使用しない)
//Page0から順に書いていって，7まで埋まっているときは全内容をシフトアップしてから7を更新する
//取りうるサイズが決まっているので引数の配列長は固定，呼び出し側で配列クリアしておく
void SSD1306_display1LineWithShiftUp(char input[SSD1306_CHARS_SIZE]){
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
      ssd1306_displayBuffer[SSD1306_PAGES_SIZE-1][chars]=(uint8_t)input[chars];
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
      ssd1306_displayBuffer[updatePage][chars]=(uint8_t)input[chars];
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

  SSD1306_display1LineWithShiftUp("LINE1 TEST      ");
  delay(1000);
  SSD1306_display1LineWithShiftUp(" LINE2 TEST     ");
  delay(1000);
  SSD1306_display1LineWithShiftUp("  LINE3 TEST    ");
  delay(1000);
  SSD1306_display1LineWithShiftUp("   LINE4 TEST   ");
  delay(1000);
  SSD1306_display1LineWithShiftUp("    LINE5 TEST  ");
  delay(1000);
  SSD1306_display1LineWithShiftUp("     LINE6 TEST ");
  delay(1000);
  SSD1306_display1LineWithShiftUp("      LINE7 TEST");
  delay(1000);
  SSD1306_display1LineWithShiftUp("LINE8 TEST      ");
  delay(1000);
  SSD1306_display1LineWithShiftUp(" LINE9 TEST     ");
  delay(1000);
  SSD1306_display1LineWithShiftUp("  LINE10 TEST   ");

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
  // Serial.print((millis()-millis_buf));//1ループ全体の時間チェック用
  // Serial.print("ms");
  // Serial.println();
}
