import serial

com = serial.Serial('COM10',230400)

with open('./log/data.csv', 'a') as f:
    print("start")
    while True:
        line = com.readline().strip().decode('utf-8')
        # line = com.readline().decode('utf-8')
        # print(line)
        f.write(line)
        f.write('\n')

f.close()
com.close()
