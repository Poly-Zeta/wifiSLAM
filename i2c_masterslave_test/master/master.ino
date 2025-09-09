#include "Wire.h"

#define I2C_DEV_ADDR 0x55
#define MAINLOOP_CYCLE_MS 10

//ros側raspiのデモのため，十分高速で動作しているように見える必要がある
//2コア動作にして，片方は10ms cycleでi2c_request->バッファ保存
//もう片方はバッファ読み出し->変換->Serial.print 230400
TaskHandle_t thp[1];//マルチスレッドのタスクハンドル格納用
const uint8_t delay_th=MAINLOOP_CYCLE_MS-2;

uint32_t i = 0;

uint8_t temp[88];

unsigned long sendClkBuffer;
float sendDataBuffer[21];

void setup() {
  Serial.begin(230400);
  // Serial.setDebugOutput(true);
  Wire.begin();

  xTaskCreatePinnedToCore(Core0, "Core0", 8192, NULL, 3, &thp[0], 0); 
}

void loop() {
  unsigned long millis_buf = millis();

  //Write message to the slave
//   Wire.beginTransmission(I2C_DEV_ADDR);
//   Wire.printf("Hello World! %lu", i++);
//   uint8_t error = Wire.endTransmission(true);
//   Serial.printf("endTransmission: %u\n", error);

  //Read bytes from the slave
  uint8_t bytesReceived = Wire.requestFrom(I2C_DEV_ADDR, 88);
  // Serial.printf("requestFrom: %u\n", bytesReceived);
  // if ((bool)bytesReceived) {  //If received more than zero bytes
  if (bytesReceived>=88) {
    // uint8_t temp[bytesReceived];
    Wire.readBytes(temp, bytesReceived);
    // log_print_buf(temp, bytesReceived);
    sendClkBuffer=(temp[3]<<24)|(temp[2]<<16)|(temp[1]<<8)|(temp[0]);
    for(int i=1;i<22;i++){
      char bf[4];
      bf[0]=temp[4*i+0];
      bf[1]=temp[4*i+1];
      bf[2]=temp[4*i+2];
      bf[3]=temp[4*i+3];
      float *p=(float*)bf;
      sendDataBuffer[i-1]=*p;
    }
  }

  int est_clk_c1=millis()-millis_buf;
  if((est_clk_c1)<=delay_th){
    delay(delay_th-est_clk_c1);
  }
  while ((millis() - millis_buf) < MAINLOOP_CYCLE_MS){}
  // delay(30000);
}

void Core0(void *args) {
  unsigned long millis_buf_c0;
  int est_clk_c0;

  while (1) {
    millis_buf_c0 = millis();

    Serial.print(sendClkBuffer);
    
    for(int i=0;i<21;i++){
      Serial.print(",");
      Serial.print(sendDataBuffer[i]);
    }
    Serial.println();

    est_clk_c0=millis()-millis_buf_c0;
    if((est_clk_c0)<=delay_th){
      delay(delay_th-est_clk_c0);
    }
    while ((millis() - millis_buf_c0) < MAINLOOP_CYCLE_MS){}
  }
}