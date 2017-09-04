#!/usr/bin/env python3

import socket

if __name__ == '__main__':
    HOST = 'localhost'
    PORT = 10086
    BUFF_SIZE = 1024
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((HOST, PORT))
        sock.send(b'Hello sb')
        data = sock.recv(BUFF_SIZE)
        print('Received: ', repr(data))
