#!/usr/bin/python3
# coding=utf-8
'''
UDPServer, 通信服务
中心端与各个终端之间建立通信，接收采集数据，下发指令
服务端口：9527（UDP）
'''
import socket
from common.udp_transaction import *

def working(_1553b):
    print("[UDPServer]打开通信服务...")
    try:
        sock_server = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        sock_server.bind(('0.0.0.0',9527))

        while True:
            data_bytes,addr = sock_server.recvfrom(1024)
            transaction(data_bytes,addr,_1553b)
    except Exception as e:
        print(e)
        print("[UDPServer]发生通信异常...")
    finally:
        sock_server.close()
        print("[UDPServer]通信服务已关闭...")

#处理来自终端的数据包
def transaction(data_bytes, addr, _1553b):
    try:
        data = data_bytes.decode("utf-8")
        if data.endswith("\n") != True:  #如果不是以'\n'结尾就丢弃，然后退出
            return

        data = data.split('|')  #根据device_type交给不同的处理方法
        if data[2] == '0100':
            transaction_0100(data, addr, _1553b)
        elif data[2] == '0200':
            print(1)
        elif data[2] == '0300':
            print(1)
        else:
            print("设备类型不对") #设备类型不对，丢弃
            return
    except Exception as e:
        print(e)
