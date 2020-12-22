import socket
import struct
import random


if __name__ == "__main__":
    # Listen for incoming datagrams
    localIP = "127.0.0.1"
    localPort = 9292
    buffersize = 1024
    sock = socket.socket(family=socket.AF_INET)
    sock.bind((localIP, localPort))
    sock.listen(5)

    while True:
        print("TCP server up and listening on", localIP, localPort)
        (clientsocket, address) = sock.accept()
        print("connection", address)
        num_state = 0
        num_actions = 0
        act = 0
        long_size = struct.calcsize('!l')

        while(True):
            command = clientsocket.recv(1)
            if not command:
                break
            print(command)
            if command == b"I":
                num_state = ord(clientsocket.recv(1))
                num_actions = ord(clientsocket.recv(1))
                state_unpack_fmt = "!" + "l" * num_state
                print("setup", num_state, num_actions)
            elif command == b"R":
                r = b""
                while len(r) < long_size:
                    r += clientsocket.recv(long_size - len(r))
                reward = struct.unpack("!l", r)[0]
                print("reward", reward)
            elif command == b"S":
                r = b""
                expected = long_size * num_state
                while len(r) < expected:
                    r += clientsocket.recv(expected - len(r))
                args = struct.unpack(state_unpack_fmt, r)
                print("step", args)
                # act = random.choice(range(num_actions))
                act = act % num_actions
                print("action", act)
                clientsocket.sendall(bytearray([act]))
                act += 1
            elif command == b"E":
                e = ord(clientsocket.recv(1))
                print("end", e)
