#!/usr/bin/python3
# coding=utf-8

from multiprocessing import Process,Manager
from common import UDPServer
from common import TCPServer

if __name__ == "__main__":
    m = Manager()

    # 数据总线 ，用于存放进程间通信的数据，包括 来自传感器的数据和进程控制开关
    _1553b = m.dict()
    # 控制指令总线，用list实现FIFO的指令执行队列
    _1553a = m.list()
    try:
        print("小伊启动...")

        p1 = Process(target=UDPServer.working, args=(_1553b,), name='p1')
        p2 = Process(target=TCPServer.working, args=(_1553b,), name='p2')

        p1.daemon = True
        p2.daemon = True

        p1.start()
        p2.start()

        p1.join()
        p2.join()
    except Exception as e:
        print(e)
    finally:
        print("小伊跟你说再见...")
