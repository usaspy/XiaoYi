#!/usr/bin/python3
# coding=utf-8
'''
TCPServer, 文件传输服务
中心端与各个终端之间建立通信，接收终端发送的流文件，比如：图片、视频、音频
服务端口：9528（TCP）
'''
import socket
import uuid
import threading

def working(_1553b):
    print("[TCPServer]打开文件传输服务...")
    try:
        sock_server = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        sock_server.bind(('0.0.0.0',9528))

        sock_server.listen()
        while True:
            client_socket, client_addr = sock_server.accept()
            t = threading.Thread(target=getPic, args=(client_socket, client_addr))
            t.setDaemon(True)
            t.start()
    except Exception as e:
        print(e)
        print("[TCPServer]发生通信异常...")
    finally:
        sock_server.close()
        print("[TCPServer]文件传输服务已关闭...")

#控制链路服务器端
#所有命令格式为“命令字串”
def getPic(client_socket, client_addr):
    picpath = 'C:/' + str(uuid.uuid1()) + ".jpg"
    recv_len = recv_file_head(client_socket, 6)
    print('应接收到的数据为:', recv_len)
    recv_picture(client_socket, recv_len, picpath)

#计算图片的字节数
def recv_file_head(socket, headsize):
    recv_data = socket.recv(headsize)
    recv_len = 0

    if len(recv_data) != headsize:
        return 0

    print(recv_data[0], recv_data[1])
    if recv_data[0] == 143 and recv_data[1] == 141:
        recv_len = recv_data[2] * 16777216 + recv_data[3] * 65536 + recv_data[4] * 256 + recv_data[5]

    return recv_len

#接收图片
def recv_picture(socket, size, path):
    # recv_data = 0
    count = 0
    # wb以二进制方式写文件,因为我们的tcp接收数据为二进制byte的类型
    file = open(path, 'wb')
    while True:
        recv_data = socket.recv(size)
        print(count, len(recv_data))
        if len(recv_data) != 0:
            count += len(recv_data)
            file.write(recv_data)
        else:
            file.close()
            break