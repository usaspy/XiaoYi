import socket
import time

if __name__ == '__main__':
    try:
        sock_client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        while True:
                    #print(_1553b)
                    #每隔0.3秒将_1553b中数据传给地面站
                    bytes_1553b = "12sfasfasfsaf".encode("utf-8")
                    sock_client.sendto(bytes_1553b,('192.168.100.29', 9527))
                    time.sleep(0.9)
    except Exception as e:
        print(e)
        print("[Data_Link]数据回传链路接收数据时发生异常...")
