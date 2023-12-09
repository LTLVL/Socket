#include "iostream"
#include <arpa/inet.h>
#include <thread>
#include <queue>
#include <vector>
#include <unistd.h>
#include <string.h>
#include "../Global.h"

using namespace std;
using namespace std;

#define BUF_SIZE 4096
#define QUEUE_SIZE 10

int cnt = 0;

// 服务端的socket
int server_listen;
vector<client> client_list;
char service_name[] = "socket-server";
bool service_run;

void serve_child(int sa);
bool conceal_package( int sa, Packet* receive );


int main(){

    // 我学号的后四位是0833
    int port = 833;
    // 
    int on=1;
    
    // 放IP地址的
    struct sockaddr_in channel;
    memset(&channel, 0, sizeof(channel));
    channel.sin_family = AF_INET;
    channel.sin_addr.s_addr = htonl(INADDR_ANY);
    channel.sin_port = htons(port);

    // 调用socket 用IPv4 IP + port的形式
    // type 用 sock_stream
    // 协议是 tcp
    server_listen = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( server_listen<0 ){
        cout << "socket failed" << endl;
        exit(-1);
    }

    setsockopt(server_listen, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on));

    int b = ::bind(server_listen, (struct sockaddr*) &channel, sizeof(channel) );
    if( b<0 ){
        cout << "bind failed" << endl;
        exit(-1);
    }

    // 建立倾听
    int l = listen(server_listen, QUEUE_SIZE);
    if( l<0 ){
        cout << "listen failed" << endl;
        exit(-1);
    }
    // 先给自己的加进去队列
    client serve;
    memset(&serve, 0, sizeof(serve));
    serve.id = server_listen;
    char* ip_host = inet_ntoa(channel.sin_addr);
    strncpy(serve.ip, ip_host, strlen(ip_host));
    serve.port = (int)ntohs(channel.sin_port);
    client_list.push_back(serve);
    cout << "service already prepared! ip:" << serve.ip << " port:" << serve.port << endl;

    service_run = true;
    thread thread_child;
    // 开始循环accept
    while( client_list.size() < 10 ){
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        cout << "listening port:" << port << endl;
        int sa = accept(server_listen, (sockaddr*)&client_addr, (socklen_t*)&addr_len);

        // 显示连接的客户端信息
        cout << "connected with client:" << endl;
        char ip_address[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), ip_address, INET_ADDRSTRLEN);
        cout << "IP/port: " << ip_address << ":" << (int)ntohs(client_addr.sin_port) << endl;
        // cout << "IP/port: " << client_addr.sin_addr.s_addr << ":" << client_addr.sin_port << endl;
        // 存入
        client temp;
        memset(&temp, 0, sizeof(temp));
        temp.id = sa;
        char* ip = inet_ntoa(client_addr.sin_addr);
        strncpy(temp.ip, ip, strlen(ip));
        temp.port = (int)ntohs(client_addr.sin_port);
        client_list.push_back(temp);
        thread
        thread_child =  thread(serve_child, sa);
        thread_child.detach();
    }

    service_run = false;
    while( client_list.size() != 1 );
    cout << "all client processes have been closed" << endl;
    cout << "server is closed" << endl;
    return 0;
}

// 从目标的客户端读取一个packet
// 客户端只可能发送packet为单位的包
int receive_packet( int sa, Packet* rec ){
    // 读取指针，最大长度
    int index = 0;
    int size = sizeof(Packet);
    while( index < size ){
        int read = ::recv(sa, rec+index, size-index, 0);
        if( read == 0 ) break;
        if( read < 0 ){
            cout << "receive packet from " << sa << " errror!" << endl;
            return -1;
        }
        index += read;
    }
    return index;
}

// 查找函数，查找输入的句柄是否在队列中
// 存在则返回index
// 不存在返回-1
int get_client_index(int sa){
    int index = -1;
    int num = client_list.size();
    for( int i=0; i<num; i++ ){
        if( sa == client_list[i].id )  index = i;
    }
    return index;
}

// 子进程获得socket句柄
// 并循环接受，根据内容返回对应的答复
void serve_child(int sa){
    // 首先发送hello 招呼数据
    char hello[] = "hello!\r\n";
    int check = ::send(sa, hello, sizeof(hello), 0);
    if( check <= 0 ){
        cout << "send to client " << sa << " error!" << endl;
        exit(-1);
    }

    // 开始循环调用receive来获得请求
    // con 变量用于判断是否还存在连接
    bool con = true;
    while( con ){
        // 申请一段Packet长度的内存
        Packet* receive = new Packet;
        // 接受client传送的包
        check = receive_packet(sa, receive);
        // 检查是否出问题
        if( check < 0 ){
            cout << "receive from client " << sa << " error!" << endl;
            // exit(-1);
        }
        // 如果接受到为0，就是断开连接了
        // 手动设置包为close类型
        // 或者服务器要关闭了，也要close
        if( check == 0 || !service_run )    receive->header.operation = CLOSE;
        // 处理得到的包
        con = conceal_package(sa, receive);
    }
}

// 处理包的函数
// 接受句柄和包，并根据包内的信息对句柄所在的client发送不同的内容
// 返回true 正常继续子进程
// 返回false 切断连接 并结束子进程
bool conceal_package( int sa, Packet* receive ){
    // 先拿包的类型
    operation_type op = receive->header.operation;

    // 输出客户端的请求
    cout << "client: " << receive->header.source_id << endl;
    cout << "operation: " << op << endl;
    cout << "type: " << receive->header.type << endl;

    switch (op){
        // 申请时间
        // 返回时间的答复包
        case TIME:{
            time_t now_time = time(nullptr);
            Packet* reply_back = new Packet(server_listen, sa, sizeof(now_time), REPLY, TIME, nullptr);
            strncpy(reply_back->body.data, (const char*)(&now_time), sizeof(now_time));
            ::send(sa, reply_back, sizeof(Packet), 0);
            cnt ++;
            cout << "send time package: " << cnt << endl;
            break;
        }
        // 申请名字
        // 返回服务器的名字
        case NAME:{
            Packet* reply_back = new Packet(server_listen, sa, sizeof(service_name), REPLY, NAME, nullptr);
            strncpy(reply_back->body.data, service_name, sizeof(service_name));
            ::send(sa, reply_back, sizeof(Packet), 0);
            delete reply_back;
            break;
        }
        // 申请获得客户端列表
        // 返回一个含有客户端列表的头
        case ACTIVE_LIST:{
            Packet* reply_back = new Packet(server_listen, sa, sizeof(int), REPLY, ACTIVE_LIST, nullptr);
            for(int i=0; i<client_list.size(); i++){
                reply_back->body.client_list[i] = client_list[i];
            }
            int count = client_list.size();
            strncpy(reply_back->body.data, (const char*)&count, sizeof(count));
            ::send(sa, reply_back, sizeof(Packet), 0);
            delete reply_back;
            break;
        }
        // 申请发送/答复信息
        // 判断是否发送给自己，如果是，就答复收到
        // 如果不是发送给自己的，就按dst发送，如果查不到dst，就返回error的答复
        case MESSAGE:{
            // 查找dst的目标
            int src = receive->header.source_id;
            int dst = receive->header.destination_id;
            int index = get_client_index(dst);

            // 如果没有，就报错
            if( index == -1 ){
                char error_message[] = "Destination not find!\n";
                Packet* reply_back = new Packet(server_listen, sa, sizeof(error_message), ERROR, MESSAGE, nullptr);
                strncpy(reply_back->body.data, error_message, sizeof(error_message));
                ::send(sa, reply_back, sizeof(Packet), 0);
                delete reply_back;
                break;
            }
            
            client con = client_list[index];
            // 如果有的话，判断一下是不是自己，是的话直接打印出来，不需要回复
            if( index == 0 ){
                // 把其中的消息读取出来
                // 先找长度
                int length = receive->header.length;
                if( length == 0 )   break;
                // 再读取
                char message[length+1];
                strncpy( message, receive->body.data, length+1 );
                printf("%s\n", message);
                break;
            }

            // 如果不是的话，直接转发包
            ::send(con.id, receive, sizeof(Packet), 0);
            break;
        }

        // 接受到接触连接的请求
        // 能接受到说明还在连接，则断开连接
        case CLOSE:{
            // 发一个close确认包，解除客户端子线程阻塞
            Packet* reply_back = new Packet(server_listen, sa, 0, REPLY, CLOSE, nullptr);
            ::send(sa, reply_back, sizeof(Packet), 0);
            // 找到现在的连接，把他从client 中删去
            int index = get_client_index(sa);
            client_list.erase(client_list.begin()+index);
            return false;
        }
        default:
            break;
    }
    return true;
}