#!/usr/bin/python3
# coding=utf-8

from multiprocessing import Process,Manager
from common import UDPServer
from common import TCPServer
from webapp import WebApp

if __name__ == "__main__":
    m = Manager()
    # 1553B数据总线 ，用于进程间通信的数据传递
    _1553b = m.dict()
    _1553b['UDP_SEND'] = []  #初始化UDP发送队列

    try:
        p1 = Process(target=UDPServer.working, args=(_1553b,), name='p1')
        p2 = Process(target=TCPServer.working, args=(_1553b,), name='p2')
        p3 = Process(target=WebApp.working, args=(_1553b,), name='webapp')
        p1.daemon = True
        p2.daemon = True
        p3.daemon = True

        p1.start()
        p2.start()
        p3.start()

        p1.join()
        p2.join()
        p3.join()

        print("小伊已启动...")
    except Exception as e:
        print(e)
    finally:
        print("小伊跟你说再见...")
