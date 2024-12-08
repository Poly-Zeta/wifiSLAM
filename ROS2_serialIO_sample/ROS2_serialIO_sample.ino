#include <Arduino.h>


void send_serial(int left, int right){
  Serial.print(left);
  Serial.print(",");
  Serial.print(right);
  Serial.println();
//  delay(200);
}


void receive_serial() {
    if (Serial.available() > 0) {
        String receivedData = Serial.readStringUntil('\n'); // 改行までのデータを読み込む
        // データのパース（カンマで分割された2つの値を取得）
        float linear_x;
        float angular_z;
        int commaIndex = receivedData.indexOf(',');

        if (commaIndex != -1) {
        String linearXStr = receivedData.substring(0, commaIndex);
        String angularZStr = receivedData.substring(commaIndex + 1);
        
        linear_x = linearXStr.toFloat();   // 文字列を浮動小数点数に変換
        angular_z = angularZStr.toFloat();
        
        // データをシリアルモニターに表示
        Serial.print("linear.x: ");
        Serial.println(linear_x);
        Serial.print("angular.z: ");
        Serial.println(angular_z);
        }
    }
}


void setup() {
    Serial.begin(115200);
}


void loop() {
    static int count=0;
    receive_serial(); // uncomment out to recieve serial data
    send_serial(count,count+1); // uncomment out to send serial data
    count++;
    if(count>8192){count=0;}
    delay(1);
}
