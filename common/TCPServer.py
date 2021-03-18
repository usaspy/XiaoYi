#!/usr/bin/python3
# coding=utf-8
'''
TCPServer, 文件传输服务
中心端与各个终端之间建立通信，接收终端发送的流文件，比如：图片、视频、音频
服务端口：9528（TCP）
'''
import socket

def working(_1553b):
    print("[TCPServer]打开文件传输服务...")
    try:
        sock_server = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        sock_server.bind(('0.0.0.0',9528))

        sock_server.listen()
        while True:
            sock, addr = sock_server.accept()
            t = threading.Thread(target=send_command, args=(sock, addr))
            t.setDaemon(True)
            t.start()
    except Exception as e:
        print("[TCPServer]发生通信异常...")
    finally:
        sock_server.close()
        print("[TCPServer]文件传输服务已关闭...")

#控制链路服务器端
#所有命令格式为“命令字串”
def send_command(sock, addr):
    pass

def Transaction(data_bytes,addr):
    pass