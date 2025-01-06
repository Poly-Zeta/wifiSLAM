//https://qiita.com/Ninagawa123/items/2646c7d3d98943919f80
//https://toxweblog.toxbe.com/2018/07/22/processing-3d-rotate-with-quaternion/
//のコピペ

import processing.serial.*; // シリアルライブラリをインポート

Serial myPort;//シリアルポートのインスタンス
int available_serialport = 0; // シリアル検索プログラムで調べたシリアルポートの番号に設定数値を変更しておく
String arduinoPort = Serial.list()[available_serialport]; //シリアルポートの設定
float[] data = new float[25]; //シリアルのデータを格納する配列を宣言

void setup(){
    lights();
    size(300, 300, P3D); // キャンバスサイズ
    myPort = new Serial(this, arduinoPort, 230400); // シリアルポートの設定
}

void draw(){
    background(230); //背景グレー
    translate(width / 2, height / 2, 0); // 原点を図形の中心に
    stroke(255,0,0);
    line(0, 0, 0, 300, 0, 0);
    stroke(0,255,0);
    line(0, 0, 0, 0, 300, 0);
    stroke(0,0,255);
    line(0, 0, 0, 0, 0, 300);
    stroke(0);
    try{
    float[] euler = quaternion_to_euler_angle(data[2], -data[3], data[4], data[1]);
    rotateX(-euler[0]);
    rotateY(-euler[1]);
    rotateZ(-euler[2]);
//   rotateX(radians(data[0])*90); //dataの中身は重力加速度gなので、90を掛けると角度に概算できる
//   rotateZ(radians(-data[1])*90); //dataの中身は重力加速度gなので、90を掛けると角度に概算できる
    }catch(Exception e){}
    int size = 10;//図形のサイズ倍率。キャンバスを大きくする時に変更可
    box(20*size, 10 * size, size); //GY-521基盤のような直方体を描く
    translate(0, -4 * size, 0);
}

void serialEvent(Serial p){//シリアルを監視
    String inString = myPort.readStringUntil('\n'); //データがあったら改行のところまで読み込む
    if (inString != null) { //シリアルの文字列データが何か入っていれば
        // println(inString); 
        // inString = trim(inString); //シリアル文字列の前後の空白を削除
        data = float(split(inString, ',')); //ピリオドで分割して配列に格納
        // println(data);//受信した配列データをprocessingのコンソールに出力
    }
}

float[] quaternion_to_euler_angle(float x, float y, float z, float w){
    float[] forReturn = new float[3];
    float ysqr = y * y;

    float t0 = 2.0 * (w * x + y * z);
    float t1 = 1.0 - 2.0 * (x * x + ysqr);
    forReturn[0] = atan2(t0, t1);
    
    float t2 = 2.0 * (w * y - z * x);
    t2 = (t2 > 1.0)? 1.0:t2;
    t2 = (t2 < -1.0)? -1.0:t2;
    forReturn[1] = asin(t2);
    
    float t3 = 2.0 * (w * z + x * y);
    float t4 = 1.0 - 2.0 * (ysqr + z * z);
    
    forReturn[2] = atan2(t3, t4);
    println(forReturn);
    return forReturn;
}
