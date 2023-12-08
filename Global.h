#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    int id;
    char ip[15];
    int port;
} client;

enum operation_type {
    CONNECT = 1,
    CLOSE,
    TIME,
    NAME,
    ACTIVE_LIST,
    MESSAGE,
    EXIT
};

enum message_type {
    REPLY = 1,
    REQUIRE,
    ERROR
};

class PacketHead {
public:
    int source_id;
    int destination_id;
    // data部分的内容长
    int length; //total length of data needed to pass;
    // 消息类型
    message_type type;
    // 消息相关指令
    operation_type operation;

    PacketHead() {}
    PacketHead(int source_id, int destination_id, int length, message_type type, operation_type operation)
            : source_id(source_id)
            , destination_id(destination_id)
            , length(length)
            , type(type)
            , operation(operation)
    {
    }
};

typedef struct {
    char data[256];
    client client_list[10];
} PacketBody;

class Packet {
public:
    PacketHead header;
    PacketBody body;
    Packet() {}
    Packet(int sourece, int destination, int length, message_type type, operation_type op, unsigned char* in_data)
        : header(sourece, destination, length, type, op)
    {
        memset(body.data, 0, 256);
    }
};
