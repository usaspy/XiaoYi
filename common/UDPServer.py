#!/usr/bin/python3
# coding=utf-8
'''
UDPServer, 通信服务
中心端与各个终端之间建立通信，接收采集数据，下发指令
服务端口：9527（UDP）
'''
import socket
import threading
import time
from common.udp_transaction import *

def working(_1553b):
    print("[UDPServer]打开通信服务...")
    thread_list = []
    try:
        sock_server = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        sock_server.bind(('0.0.0.0',9527))

        t1 = threading.Thread(target=device2center, args=(sock_server,_1553b))
        t2 = threading.Thread(target=center2device, args=(sock_server,_1553b))

        thread_list.append(t1)
        thread_list.append(t2)

        for t in thread_list:
            t.setDaemon(True)
            t.start()

        for t in thread_list:
            t.join()

    except Exception as e:
        print(e)
        print("[UDPServer]发生通信异常...")
    finally:
        sock_server.close()
        print("[UDPServer]通信服务已关闭...")

#设备向中心发送数据
def device2center(sock_server, _1553b):
    while True:
        data_bytes, addr = sock_server.recvfrom(512)
        t = threading.Thread(target=transaction, args=(data_bytes, addr, _1553b))
        t.setDaemon(True)
        t.start()

#中心向设备发送数据
def center2device(sock_server,_1553b):
    while True:
        time.sleep(0.5)
        if type(_1553b.get("UDP_SEND")) == list and len(_1553b.get("UDP_SEND")) > 0: #有待发送数据 [('172.1.1.1', 9527), '待发送的内容']
            datas = _1553b.get("UDP_SEND")
            _1553b["UDP_SEND"] = []   # m.dict{}比较特殊,不能直接append或pop
            while True:
                data = datas.pop(0)
                sock_server.sendto(data[1].encode("utf-8"), data[0])
                if len(datas) == 0: break


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
