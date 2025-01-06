import os
from scipy.spatial.transform import Rotation
from scipy.signal import butter, lfilter
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import animatplot as amp

#copilot4freeのテスト動作も兼ねて．
#タイムスタンプと加速度(LAcc_X,LAcc_Y,LAcc_Z)，クォータニオン(Qw,Qx,Qy,Qz)から，3次元移動の軌跡を描画する
#各値は./log/data.csvに記録されているので，それを読み込んで使用する
#./log/data.csvのフォーマットは以下の通り
# time,Gyro_X,Gyro_Y,Gyro_Z,Mag_X,Mag_Y,Mag_Z,Acc_X,Acc_Y,Acc_Z,LAcc_X,LAcc_Y,LAcc_Z,Qw,Qx,Qy,Qz,Temperature,Humidity,Pressure,L-Wheel,R-Wheel 
#./log/data.csv内にはヘッダ行は存在しない
#描画に必要な項目の単位は以下の通り
#time: ミリ秒
#LAcc: m/s^2
#Qw,Qx,Qy,Qz: 無次元
#軌跡の算出は以下の手順で行う
#1.global座標系とローカル座標系を用意しておく
#2.ローカル座標系とglobal座標系の原点を合わせておく
#3.クオータニオンの値をオイラー角に変換する
#4.3で求めたオイラー角をもとにローカル座標系を回転させる
#5.ローカル座標系の加速度をglobal座標系に変換する
#6.加速度を積分して速度，さらに積分して位置をもとめる
#7.tを進めて3~6を繰り返す
#8.xy平面での軌跡を描画する

#クォータニオンの値をオイラー角(ZYX順)に変換する関数
#引数はクオータニオンの値(w,x,y,z)
#返り値はオイラー角の値(x,y,z)
def quarternion_to_eular(w,x,y,z):
    #ysqr=y*yを求めておく
    ysqr = y * y
    #オイラー角の値を求める
    t0 = +2.0 * (w * x + y * z)
    t1 = +1.0 - 2.0 * (x * x + ysqr)
    X = np.arctan2(t0, t1)
    t2 = +2.0 * (w * y - z * x)
    t2 = np.where(t2>+1.0,+1.0,t2)
    t2 = np.where(t2<-1.0,-1.0,t2)
    Y = np.arcsin(t2)
    t3 = +2.0 * (w * z + x * y)
    t4 = +1.0 - 2.0 * (ysqr + z * z)
    Z = np.arctan2(t3, t4)
    return X,Y,Z

#データを読み込んで軌跡をプロットする関数
def plot():
    #データを読み込む
    data = pd.read_csv('./log/data.csv',header=None)
    #見出しをつける
    data.columns = ['time','Gyro_X','Gyro_Y','Gyro_Z','Mag_X','Mag_Y','Mag_Z','Acc_X','Acc_Y','Acc_Z','LAcc_X','LAcc_Y','LAcc_Z','Qw','Qx','Qy','Qz','Temperature','Humidity','Pressure','L-Wheel','R-Wheel']

    #手順に従って軌跡を描画する
    #1.global座標系とローカル座標系を用意しておく
    #global座標系
    global_position = np.zeros(3)
    global_velocity = np.zeros(3)
    #ローカル座標系
    local_position = np.zeros(3)
    local_velocity = np.zeros(3)
    #2.ローカル座標系とglobal座標系の原点を合わせておく
    global_position = local_position
    global_velocity = local_velocity
    #3.クオータニオンの値をオイラー角に変換する
    #クォータニオンの値を取得
    qw = data['Qw'].values
    qx = data['Qx'].values
    qy = data['Qy'].values
    qz = data['Qz'].values

    #クォータニオンの値をオイラー角に変換
    euler = np.array([quarternion_to_eular(w,x,y,z) for w,x,y,z in zip(qw,qx,qy,qz)])
    #オイラー角を取得
    euler_x = euler[:,0]
    euler_y = euler[:,1]
    euler_z = euler[:,2]
    #オイラー角変化のグラフを描画
    fig, ax = plt.subplots()
    ax.plot(euler_x,label='x')
    ax.plot(euler_y,label='y')
    ax.plot(euler_z,label='z')
    ax.legend()
    plt.show()



    #4.3で求めたオイラー角をもとにローカル座標系を回転させる
    #ローカル座標系の加速度を取得
    local_acceleration = data[['LAcc_X','LAcc_Y','LAcc_Z']].values

    #加速度の値について，直前の加速度の値との差分が小さい場合は0にし，それ以外はそのままの値を使う
    #差分の計算には，事前にコピーしておいた元の値を使う
    local_acceleration_copy = local_acceleration.copy()
    for i in range(len(local_acceleration)-1):
        diff = np.abs(local_acceleration_copy[i+1]-local_acceleration_copy[i])
        local_acceleration[i+1] = np.where(diff<0.1,0,local_acceleration[i+1])

    #加速度の値にローパスフィルタをかける フィルタにはscypiのbutterを使用
    def butter_lowpass(cutoff, fs, order=5):
        nyquist = 0.5 * fs
        normal_cutoff = cutoff / nyquist
        b, a = butter(order, normal_cutoff, btype='low', analog=False)
        return b, a
    def butter_lowpass_filter(data, cutoff, fs, order=5):
        b, a = butter_lowpass(cutoff, fs, order=order)
        y = lfilter(b, a, data)
        return y
    
    #加速度の値にはオフセットがあるので，絶対値で足切りした値をオフセットとして除去
    # local_acceleration = np.where(np.abs(local_acceleration)<0.2,0,local_acceleration)

    #ローパスフィルタをかける
    local_acceleration = butter_lowpass_filter(local_acceleration,40,100,10)


    #ローカル座標系の加速度を描画
    fig, ax = plt.subplots()
    ax.plot(local_acceleration[:,0],label='x')
    ax.plot(local_acceleration[:,1],label='y')
    ax.plot(local_acceleration[:,2],label='z')
    ax.legend()
    plt.show()

    #ローカル座標系の加速度をglobal座標系に変換
    global_acceleration = np.zeros(local_acceleration.shape)
    rotations = np.zeros(local_acceleration.shape)
    # global_acceleration = local_acceleration
    for i in range(len(local_acceleration)):
        quat = np.array([qx[i],qy[i],qz[i],qw[i]])
        rot = Rotation.from_quat(quat)
        #加速度ベクトルを回転させる
        global_acceleration[i]=rot.apply(local_acceleration[i])
        #rotを記録しておく
        rotations[i]=rot.as_euler('xyz')
    #オイラー角を取得
    # euler_rotations_x = rotations[:,0]
    # euler_rotations_y = rotations[:,1]
    # euler_rotations_z = rotations[:,2]
    # #オイラー角変化を描画
    # fig, ax = plt.subplots()
    # ax.plot(euler_rotations_x,label='x')
    # ax.plot(euler_rotations_y,label='y')
    # ax.plot(euler_rotations_z,label='z')
    # ax.legend()
    # plt.show()

    #加速度の変化を描画
    fig, ax = plt.subplots()
    ax.plot(global_acceleration[:,0],label='x')
    ax.plot(global_acceleration[:,1],label='y')
    ax.plot(global_acceleration[:,2],label='z')
    ax.legend()
    plt.show()

    #6.加速度を積分して速度，さらに積分して位置をもとめる
    #時刻を取得
    time = data['time'].values
    #速度と位置を格納する配列を用意
    global_velocity = np.zeros(global_acceleration.shape)
    global_position = np.zeros(global_acceleration.shape)
    for i in range(len(global_acceleration)-1):
        #速度を積分
        global_velocity[i+1] = global_velocity[i] + global_acceleration[i] #* (time[i+1]-time[i]) / 1000
        #位置を積分
        global_position[i+1] = global_position[i] + global_velocity[i] #* (time[i+1]-time[i]) / 1000
    #8.xy平面での軌跡を描画する
    fig, ax = plt.subplots()
    ax.plot(global_velocity[:,0],label='x')
    ax.plot(global_velocity[:,1],label='y')
    # ax.plot(global_acceleration[:,2],label='z')
    # ax.legend()
    # plt.show()

    # fig, ax = plt.subplots()
    ax.plot(global_velocity[:,2],label='z')
    ax.legend()
    plt.show()

    #軌跡を描画
    fig, ax = plt.subplots()
    ax.plot(global_position[:,0],global_position[:,1])
    ax.set_aspect('equal')
    plt.show()

    #3次元軌跡を描画
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    #軸の範囲を設定
    ax.set_xlim(-1000,1000)
    ax.set_ylim(-1000,1000)
    ax.set_zlim(-1000,1000)
    ax.plot(global_position[:,0],global_position[:,1],global_position[:,2])
    plt.show()


if __name__ == '__main__':
    plot()
