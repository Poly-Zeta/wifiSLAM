#include <Wire.h>
#define I2C_ADDR 0x34

#define ADC_BAT_ADDR 0x00
#define MOTOR_TYPE_ADDR 0x14
#define MOTOR_ENCODER_POLARITY_ADDR 0x15
#define MOTOR_FIXED_PWM_ADDR 0x1F

#define MOTOR_FIXED_SPEED_ADDR 0x33
#define MOTOR_ENCODER_TOTAL_ADDR 0x3C
#define MOTOR_TYPE_WITHOUT_ENCODER 0
#define MOTOR_TYPE_TT 1
#define MOTOR_TYPE_N20 2
#define MOTOR_TYPE_JGB37_520_12V_110RPM 3
u8_t data[20];

bool WireWriteByte(uint8_t val){
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(val);
  if( Wire.endTransmission() != 0 ){
    return false;  
  }
  return true;
}

bool WireWriteDataArray( uint8_t reg,int8_t *val,unsigned int len){
  unsigned int i;
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(reg);
  for(i = 0; i < len; i++){
    Wire.write(val[i]);
  }
  if( Wire.endTransmission() != 0 ){
    return false;
  }
  return true;
}

bool WireReadDataByte(uint8_t reg, uint8_t &val){
  if (!WireWriteByte(reg)){
    return false;
  }
  Wire.requestFrom(I2C_ADDR, 1);
  while (Wire.available()){
    val = Wire.read();
  }
  return true;
}  

int WireReadDataArray( uint8_t reg, uint8_t *val, unsigned int len){
  unsigned char i = 0;  /* Indicate which register we want to read from */
  if (!WireWriteByte(reg)){
    return -1;
  }  /* Read block data */
  Wire.requestFrom(I2C_ADDR, len);
  while (Wire.available()){
    if (i >= len){
      return -1;
    }
    val[i] = Wire.read();
    i++;
  }
  return i;
}

int serial_putc( char c, struct __file * ){
  Serial.write( c );
  return c;
}  

//void printf_begin(void){
//  fdevopen( &serial_putc, 0 );
//}

int8_t MotorType = MOTOR_TYPE_JGB37_520_12V_110RPM;
int8_t MotorEncoderPolarity = 0;  

int32_t encStart[2]={0,0};

void setup(){
  Wire.begin();
  Serial.begin(115200);
//  printf_begin();
  delay(200);
  
  WireWriteDataArray(MOTOR_TYPE_ADDR,&MotorType,1);
  delay(5);
  
  WireWriteDataArray(MOTOR_ENCODER_POLARITY_ADDR,&MotorEncoderPolarity,1);
  delay(5);

  WireWriteDataArray(MOTOR_ENCODER_TOTAL_ADDR,(int8_t*)encStart,8);
  delay(5);
  
  WireReadDataArray(MOTOR_ENCODER_TOTAL_ADDR,(uint8_t*)encStart,8);
}  

int8_t p1[4]={50,50,50,50};
int8_t p2[4]={-20,-20,-20,-20};
int8_t s1[4]={50,50,50,50};
int8_t s2[4]={-20,-20,-20,-20};
int8_t stop[4]={0,0,0,0};
int32_t EncodeTotal[4];

void loop(){
//  uint16_t v;
//  WireReadDataArray(ADC_BAT_ADDR,data,2);
//  
//  v = data[0]+ (data[1]<<8);
//  Serial.print("V = ");
//  Serial.print(v);
//  Serial.println("mV ");
  
  WireReadDataArray(MOTOR_ENCODER_TOTAL_ADDR,(uint8_t*)EncodeTotal,8);
//  Serial.print("E0=");
  Serial.print(EncodeTotal[0]);
  Serial.print(",");
//  Serial.print(", E1=");
  Serial.print(EncodeTotal[1]);

//  Serial.print(abs(EncodeTotal[0]-encStart[0]));
//  Serial.print(",");
//  Serial.print(abs(EncodeTotal[1]-encStart[1]));
  
//  Serial.print(", E2=");
//  Serial.print(EncodeTotal[2],BIN);
//  Serial.print(", E3=");
//  Serial.print(EncodeTotal[3],BIN);
  Serial.println("");
  
//  WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR,p1,4);
//  delay(3000);
//  
//  WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR,p2,4);
//  delay(3000);
  delay(100);
  //WireWriteDataArray(MOTOR_FIXED_PWM_ADDR,s1,4);
}
