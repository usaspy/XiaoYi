#!/usr/bin/python3
# coding=utf-8
'''
UDPServer, 通信服务
中心端与各个终端之间建立通信，接收采集数据，下发指令
服务端口：9527（UDP）
'''
import socket

def working(_1553b):
    print("[UDPServer]打开通信服务...")
    try:
        sock_server = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        sock_server.bind(('0.0.0.0',9527))

        while True:
            data_bytes,addr = sock_server.recvfrom(1024)
            transaction(data_bytes,addr)
    except Exception as e:
        print("[UDPServer]发生通信异常...")
    finally:
        sock_server.close()
        print("[UDPServer]通信服务已关闭...")

#处理来自终端的数据包
def transaction(data_bytes,addr):
    pass