import socket
import time


if __name__ == '__main__':

    try:
        sock_server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock_server.bind(('192.168.43.115', 9527))

        while True:
                data_bytes, addr = sock_server.recvfrom(1024)
                print(data_bytes)
                print(addr)
                sss = b'003c48c0-e3f6-471a-b698-7506f68455cb|192.168.43.45|0200|ACK|1|\n'
                sock_server.sendto(sss,addr)
    except Exception as e:
        print("[Data_Link]数据回传链路接收数据时发生异常...")


