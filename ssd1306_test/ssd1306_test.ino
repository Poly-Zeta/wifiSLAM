#include "Wire.h"
  
const uint8_t ADDRES_OLED =  0x3C;
const uint32_t Frequensy_OLED = 400000; //Max=400kHz
  
//全画面空データ：52ms
//とりあえずこのプログラムでの全画面任意文字：73ms
//(128/8)*(64/8)なので16*8文字=128文字
//128文字/73ms=1.75文字/1ms
//最大表示更新数は，残りセンサ類とSerialIOとの兼ね合いを考えてからの方がよいかも．
//(I2C自体が上限400kbps=50kBps=50000Bps=50B/1ms，10msループなら3ms弱で5文字更新程度までは許容範囲か？)
//->1ループで1項目更新程度？ +x.xx <-これで5文字,3ms

//本体でのディスプレイ描画には以下を実装したい
//クリーン
//起動演出
//setup ログ(printline->モニタ全域8pxシフトループ)
//loop  ログ(事前に必要txtは出しておいて，必要部分だけ更新)
//0123456789abcdef
//qw+0.00 qx+0.00
//qy+0.00 qz+0.00
//hu100.0 te+00.0
//press1000.00
//l-wheel 00000
//r-wheel 00000
//analogin0 00000
//analogin1 00000

uint8_t DotB1[8]={
  0b11111111,
  0b00000111,
  0b11111111,
  0b00000011,
  0b00000101,
  0b00001001,
  0b00010001,
  0b00100001
};

void setup() {
  Wire.begin();
  Serial.begin(115200);
  
  SSD1306_Init(); //OLED ssd1306 初期化
  delay(10);

  unsigned long millis_buf = millis();
  Clear_Display_All();
  // Serial.print("All: ");
  Serial.print((millis()-millis_buf));
  Serial.print("ms");
  Serial.println();

  millis_buf = millis();
  Display_Pic();
  Serial.print((millis()-millis_buf));
  Serial.print("ms");
  Serial.println();
}
  
void loop() {
    
}
  
void SSD1306_Init(){
  Wire.setClock(Frequensy_OLED);
  delay(100);
    
  Wire.beginTransmission(ADDRES_OLED);//※このバイトも含め、以後、合計32byteまで送信できる
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

  Wire.beginTransmission(ADDRES_OLED);
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
//*******************************************
void Display_Pic(){
  int i,j,m,n;
  uint8_t b = 0, dummy = 0;

  int pages=0;
  
  for(i=0; i<8; i++){
    unsigned long line_buf = millis();
    Wire.beginTransmission(ADDRES_OLED);
      Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command) Max=31byte
        Wire.write(0xB0 | i); //set page start address←垂直開始位置はここで決める(B0～B7)
      Wire.write(0b00000000);
        Wire.write(0x21); //set Column Address
          Wire.write(0); //Column Start Address←水平開始位置はここで決める(0～127)
          Wire.write(127); //Column Stop Address　画面をフルに使う
    Wire.endTransmission();

    for(j=0; j<16; j++){//column = 8byte x 16 ←8バイト毎に水平に連続で16個表示
      unsigned long millis_buf = millis();
      Wire.beginTransmission(ADDRES_OLED);
        Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data) Max=31byte
  
        for(n=0;n<8;n++){
          Wire.write(DotB1[n]);
        }
        // for(m=7; m>=0; m--){
        //   for(n=0; n<8; n++){ //描画1バイトを縦列に変換
        //     dummy = ( DotB1[n] >> m ) & 0x01;
        //     if(dummy > 0){
        //       b = b | (dummy << n);
        //     }
        //   }
        //   Wire.write(b); //SSD1306のGDRAM にデータ書き込み
        //   b = 0;
        // }
  
      Wire.endTransmission(); //これが送信されて初めてディスプレイに表示される
      // Serial.print("char: ");
      // Serial.print((millis()-millis_buf));
      // Serial.print("ms");
      // Serial.println();
    }
    Serial.print("line: ");
    Serial.print((millis()-line_buf));
    Serial.print("ms");
    Serial.println();
  }
}
//**************************************************
void Clear_Display_All(){
  uint8_t i, j, k;
  
  for(i = 0; i < 8; i++){//Page(0-7)
    Wire.beginTransmission(ADDRES_OLED);
      Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command)
        Wire.write(0xB0 | i); //set page start address(B0～B7)
      Wire.write(0b00000000);
        Wire.write(0x21); //set Column Address
          Wire.write(0); //Column Start Address(0-127)
          Wire.write(127); //Column Stop Address(0-127)
    Wire.endTransmission();
  
    for(j = 0; j < 16; j++){//column = 8byte x 16
      Wire.beginTransmission(ADDRES_OLED);
      Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data)
      for(k = 0; k < 8; k++){ //continue to 31byte
        Wire.write(0x00);
      }
      Wire.endTransmission();
    }
  }
}
//**************************************************
