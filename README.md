# CLI socket messanger - sockchat!

## About the project
I started this project in order to learn the socket programming, and to see how can I handle quite a big project by myself. I am beginner, so there are a lot of bugs and vulnerabilities in my code (which I hope I will fix some day).

## Install
- Just clone the repository
- Run ```make```
- Enjoy fully builded client and server!

## Run
- For server: ```./scs```
- For client: ```./scc server_ip username```

## Protocol
- Name: SockChatProtocol - scp
- Version: 0.1
- Segments:
    - 1 byte: length
    - 8 bytes: client username
    - 8 bytes: server username
    - (len - 17) bytes: message

## TODO
- develop a protocol
- redo the users registration process (its kinda dumb at the moment)
- make a usable interface (it's a total trash at the moment lol)

