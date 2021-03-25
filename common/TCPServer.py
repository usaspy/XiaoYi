#!/usr/bin/python3
# coding=utf-8
'''
TCPServer, 有链接通信服务
中心端与各个终端之间建立通信，接收终端发送的流文件，比如：图片、视频、音频
服务端口：9528（TCP）
'''
import socket
import uuid
import threading
import datetime

def working(_1553b):
    print("[TCPServer]打开文件传输服务...")
    try:
        sock_server = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        sock_server.bind(('0.0.0.0',9528))

        sock_server.listen()
        while True:
            client_socket, client_addr = sock_server.accept()
            t = threading.Thread(target=transaction, args=(client_socket, client_addr))
            t.setDaemon(True)
            t.start()
    except Exception as e:
        print(e)
        print("[TCPServer]通信出现异常...")
    finally:
        sock_server.close()
        print("[TCPServer]文件传输服务已关闭...")

#控制链路服务器端
#所有命令格式为“命令字串”
def transaction(client_socket, client_addr):
    stream = b''
    while True:
        recv_data = client_socket.recv(8192)
        stream = stream + recv_data
        if stream[len(stream)-10:] == b"===OVER===": #接收完成python datetime 精确到毫秒
            break
    if stream:
        photos = stream.split(b"===PHOTO===")
        for photo in photos[0:len(photos)-1]:
            picpath = 'C:/' + datetime.datetime.now().strftime("%Y%m%d%H%M%S%f") + ".jpg"
            file = open(picpath, 'wb')
            file.write(photo)
            file.close()


#接收图片
def recv_picture(socket, path):
    # wb以二进制方式写文件,因为我们的tcp接收数据为二进制byte的类型
    picpath = 'C:/' + str(uuid.uuid1()) + ".jpg"
    file = open(path, 'wb')
    while True:
        recv_data = socket.recv(512)
        print(recv_data)