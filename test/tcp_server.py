import socket

def send_command(sock, addr):
    try:
        sock.send('a'.encode())
    except Exception as e:
        print(e)
    finally:
        return True

if __name__ == '__main__':
    sock_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock_server.bind(('0.0.0.0', 8867))

    sock_server.listen()
    while True:
        sock, addr = sock_server.accept()
        #send_command(sock, addr)
        bt = sock.recv(512)
        if bt:
            print(bt)



