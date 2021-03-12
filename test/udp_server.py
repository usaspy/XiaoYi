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
                sss = b'66776c7a-c8f4-4143-bfac-b9813fdec4ef|192.168.43.181|0100|ACK|1|15|\n'
                sock_server.sendto(sss,addr)
    except Exception as e:
        print("[Data_Link]数据回传链路接收数据时发生异常...")


