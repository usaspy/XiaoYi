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
from webapp import vars

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
        if stream[len(stream)-10:] == vars.STREAM_OVER_FLAG: #如果最后10个字节是流结束标记，退出接收循环
            break
    if stream:
        photos = stream.split(vars.PHOTO_FLAG)  #共X张照片
        for photo in photos[0:len(photos)-1]:
            picpath = vars.PHOTOS_PATH + "/" + datetime.datetime.now().strftime("%Y%m%d%H%M%S%f") + ".jpg"
            file = open(picpath, 'wb')
            file.write(photo)
            file.close()
