import socket
import time




if __name__ == '__main__':
    sock_server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #sock_server.bind(('0.0.0.0', 6688))

    while True:
       print("11111111")
       sock_server.sendto("testing".encode("utf-8"), ("192.168.43.181", 2333))
       time.sleep(0.8)
       data_bytes, addr = sock_server.recvfrom(1024)
       print(data_bytes)



