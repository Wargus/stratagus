import collections
import re
import socket
import time


class Game(object):
    def __init__(self, session, gameversion=None, description=None, map=None, player_count=None, **kwargs):
        self.session = session
        self.gameversion = gameversion
        self.description = description
        self.map = map
        self.player_count = player_count
        self.id = time.monotonic()

    def __str__(self):
        return '%d "%s" "%s" "%s" %d' % (self.id, self.gameversion, self.description, self.map, self.player_count)


class Session(object):
    TIMEOUT = 30000

    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.time = time.time()
        self.name = None
        self.game = None
        self.local_host = host
        self.local_port = port

    def should_timeout(self):
        return self.time + Session.TIMEOUT < time.time()

    def update_last_seen(self):
        self.time = time.time()

    def __str__(self):
        return "%s:%d" % (self.host, self.port)


class Server(object):
    def __init__(self):
        localIP = "0.0.0.0"
        localPort = 20001
        self.buffersize = 1024
        self.socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
        self.socket.bind((localIP, localPort))
        print("UDP server up and listening")
        self.host_port_session = {}
        self.messages = collections.deque([], 100)

    @property
    def sessions(self):
        for host in self.host_port_session.values():
            for session in host.values():
                yield session

    def send(self, msg, session):
        print(msg, session)
        self.socket.sendto(msg.encode(errors="ignore"), (session.host, session.port))

    @property
    def usernames(self):
        for session in self.sessions:
            if session.name:
                yield session.name

    @property
    def games(self):
        for session in self.sessions:
            if session.game:
                yield session.game

    def ensure_logged_in(self, session):
        if not session.name:
            self.send("LOGIN_REQUIRED", session)
            self.delete_session(session)

    def PING(self, session, args):
        self.send("PING_OK", session)

    def LIST(self, session, args):
        "Answer the current messages, logged-in users, and active games"
        self.ensure_logged_in(session)
        for msg in self.messages:
            self.send(("MESSAGE %s" % msg), session)
        for session in self.sessions:
            if session.name:
                self.send(("USERNAME %s" % session.name), session)
            if session.game:
                self.send(("GAME %s" % session.game), session)
        self.send("LIST_OK", session)

    def LOGIN(self, session, args):
        name = args.strip()
        if name and name not in self.usernames:
            session.name = name
            self.send("LOGIN_OK", session)
        else:
            self.send("LOGIN_FAILED", session)

    GAME_RE = re.compile('''
    "(?P<gameversion>[^"]+)"\s+
    "(?P<description>[^"]+)"\s+
    "(?P<map>[^"]+)"\s+
    (?P<player_count>\d+)\s+
    (?P<local_host>[0-9\.]+)\s+
    (?P<local_port>\d+)\s+
    ''', re.VERBOSE)

    def CREATE(self, session, args):
        self.ensure_logged_in(session)
        m = self.GAME_RE.match(args)
        if not m:
            self.send("CREATE_MALFORMED", session)
        else:
            session.game = Game(session, **m.groupdict())
            session.local_host = m.group("local_host")
            session.local_port = m.group("local_port")
            self.send("CREATE_OK", session)

    JOIN_RE = re.compile('''
    (?P<id>\d+\.\d+)\s+
    (?P<local_host>[0-9\.]+)\s+
    (?P<local_port>\d+)\s+
    ''', re.VERBOSE)

    def JOIN(self, session, args):
        self.ensure_logged_in(session)
        m = self.JOIN_RE.match(args)
        if not m:
            self.send("JOIN_MALFORMED", session)
        else:
            game_id = float(m.group("id"))
            for game in games:
                if game.id == game_id:
                    session.local_host = m.group("local_host")
                    session.local_port = m.group("local_port")
                    self.socket.sendto(
                        "JOIN_OK %s:%s %s:%s" % (game.session.host, game.session.port, game.session.local_host, game.session.local_port),
                        session
                    )
                    self.send(
                        "JOIN_FROM %s:%s %s:%s" % (session.host, session.port, session.local_host, session.local_port),
                        game.session
                    )
                    break
            else:
                self.send("JOIN_FAILED", session)

    def UNKNOWN_COMMAND(self, session, msg):
        self.send(("UNKNOWN MSG %s" % msg), session)

    def delete_session(self, session):
        del self.host_port_session[session.host][session.port]
        del self.host_port_session[session.host]

    def update_sessions(self):
        to_del = []
        for session in self.sessions:
            if session.should_timeout():
                to_del.append(session)
        for session in to_del:
            print("Timeout for", session)
            self.delete_session(session)

    def listen(self):
        # Listen for incoming datagrams
        while(True):
            self.update_sessions()

            bytesAddressPair = self.socket.recvfrom(self.buffersize)
            message = bytesAddressPair[0].decode(errors="ignore")
            address = bytesAddressPair[1]

            ip, port = address

            session = self.host_port_session.setdefault(ip, {}).setdefault(port, None)
            if not session:
                session = Session(ip, port)
                self.host_port_session[ip][port] = session
            else:
                session.update_last_seen()

            command, *args = message.split(maxsplit=1)
            if args:
                args = args[0]
            print(args)
            method = getattr(self, command, None)
            if method:
                method(session, args)
            else:
                self.UNKNOWN_COMMAND(session, message)

            print(ip, port, message)


if __name__ == "__main__":
    Server().listen()
