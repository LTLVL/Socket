//
// Created by Administrator on 2023/11/13.
//
#pragma once

#include "iostream"
#include <arpa/inet.h>
#include <thread>
#include <queue>
#include <unistd.h>
#include <string.h>
#include "Global.h"

using namespace std;


void receiveData(int fd);

void showMenu();

int init();

void checkQueue();

deque<Packet> data_queue;
unsigned char rec_buffer[1024];
bool stop = false;

//10.192.124.127
void menu() {
    bool isQuit = false; //是否退出
    bool isConnected = false; //是否已连接
    bool getList = false; //是否已查询列表
    int rec = 0;
    int fd;
    thread childthread;
    while (!isQuit) {
        showMenu();
        cin >> rec;
        switch (rec) {
            case 1: {
                if (isConnected) {
                    cout << "Error input" << endl;
                    break;
                }
                fd = init();
                const char *IP;
                string temp;
                int port;
                cout << "Please type server IP" << endl;
                cin >> temp;
                IP = temp.c_str();
                cout << "Please type server port" << endl;
                cin >> port;

                //连接
                struct sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_port = htons(port);   // 指定端口并转换大小端
                inet_pton(AF_INET, IP, &addr.sin_addr.s_addr);
                int ret = connect(fd, (struct sockaddr *) &addr, sizeof(addr));

                //校验连接
                if (ret == -1) {
                    perror("connect");
                    exit(0);
                }

                //连接成功
                isConnected = true;
                cout << "Connection success!" << endl; //提示

                //子线程
                childthread = thread(receiveData, fd);
                childthread.detach();
                break;
            }
            case 2: {
                if (!isConnected) {
                    cout << "Error input" << endl;
                    break;
                }
                //封装查询体
                Packet to_send(fd, -1, sizeof(Packet), REQUIRE, CLOSE, nullptr);
                send(fd, &to_send, sizeof(Packet), 0);
                isConnected = false;
                stop = true;
                cout << "Disconnection success!" << endl;
                break;
            }
            case 3: {
                if (!isConnected) {
                    cout << "Error input" << endl;
                    break;
                }
                //封装查询体
                Packet to_send(fd, -1, sizeof(Packet), REQUIRE, TIME, nullptr);
                for (int j = 0; j < 100; j++) {
                    send(fd, &to_send, sizeof(Packet), 0);
                }

                //等待响应
                while (data_queue.empty());

                time_t time = -1;

                for (auto iter = data_queue.begin(); iter != data_queue.end();) {
                    if (iter->header.operation == TIME) {
                        time = *(time_t *) (iter->body.data);
                        data_queue.erase(iter);
                        break;
                    } else {
                        iter++;
                    }
                }

                if (time == -1) {
                    cout << "Not receive time packet yet" << endl;
                } else {
                    cout << ctime(&time) << endl;
                }
                break;
            }
            case 4: {
                if (!isConnected) {
                    cout << "Error input" << endl;
                    break;
                }
                //封装查询体
                Packet to_send(fd, -1, sizeof(Packet), REQUIRE, NAME, nullptr);
                send(fd, &to_send, sizeof(Packet), 0);
                //等待响应
                while (data_queue.empty());

                char name[256];

                for (auto iter = data_queue.begin(); iter != data_queue.end();) {
                    if (iter->header.operation == NAME) {
                        strncpy(name, iter->body.data, iter->header.length + 1);
                        data_queue.erase(iter);
                        break;
                    } else {
                        iter++;
                    }
                }

                if (name[0] == 0) {
                    cout << "Not receive name packet yet" << endl;
                } else {
                    cout << name << endl;
                }
                break;
            }
            case 5: {
                if (!isConnected) {
                    cout << "Error input" << endl;
                    break;
                }
                //封装查询体
                Packet to_send(fd, -1, sizeof(Packet), REQUIRE, ACTIVE_LIST, nullptr);
                send(fd, &to_send, sizeof(Packet), 0);
                //等待响应
                while (data_queue.empty());

                int temp = -1;
                cout << "------------------------------" << endl;
                cout << "\tActive clients" << endl;
                cout << "------------------------------" << endl;
                cout << "num\tip\t\tport\t" << endl;

                for (auto iter = data_queue.begin(); iter != data_queue.end();) {
                    if (iter->header.operation == ACTIVE_LIST) {
                        temp = *(int *) (iter->body.data);
                        for (int i = 0; i < temp; ++i) {
                            cout << iter->body.client_list[i].id << "\t";
                            cout << iter->body.client_list[i].ip << "\t";
                            cout << iter->body.client_list[i].port << "\t" << endl;
                        }
                        data_queue.erase(iter);
                        break;
                    } else {
                        iter++;
                    }
                }
                if (temp == -1) {
                    cout << "Not receive list packet yet" << endl;
                }
                getList = true;
                break;
            }
            case 6: {
                if (!isConnected) {
                    cout << "Error input" << endl;
                    break;
                }
                if (!getList) {
                    cout << "Query client list first" << endl;
                    break;
                }
                int tr_id = -1;
                int dest_id = -1;
                string msg;
                string temp;
                cout << "Type your id" << endl;
                cin >> tr_id;
                cout << "Type your destination id" << endl;
                cin >> dest_id;
                cout << "Type your message" << endl;
                getline(cin, temp);
                getline(cin, msg);


                Packet to_send(tr_id, dest_id, sizeof(Packet), REQUIRE, MESSAGE, nullptr);
                msg.copy(to_send.body.data, msg.size(), 0);
                send(fd, &to_send, sizeof(Packet), 0);
                break;
            }
            case 7: {
                isQuit = true;
                cout << "Good bye!" << endl;
                break;
            }
            default: {
                cout << "Error input" << endl;
                break;
            }
        }
        checkQueue();
    }
}

void showMenu() {
    cout << "Type the index:" << endl;
    cout << "1:Connect" << endl;
    cout << "2:Disconnect" << endl;
    cout << "3:Query Time" << endl;
    cout << "4:Query Name" << endl;
    cout << "5:Query Alive List" << endl;
    cout << "6:Send Message" << endl;
    cout << "7:Quit" << endl;
}

int init() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        exit(0);
    }
    return fd;
}

void receiveData(int fd) {
    int count = 0;
    while (!stop) {
        memset(rec_buffer, 0, sizeof(rec_buffer));

        int received = 0;
        int bytes = 0;

        Packet temp;
        //读取数据
        do {

            bytes = read(fd, &temp, sizeof(Packet) - received);
            //收到错误信息
            if (temp.header.type == ERROR) {
                cout << "Received an error message" << endl;
            }
            if (temp.header.source_id == 0 && temp.header.destination_id == 0) {
                continue;
            }
            if (temp.header.operation == CLOSE) {
                close(fd);
                return;
            }
            if (temp.header.operation == MESSAGE) {
                string message = temp.body.data;
                cout << "Receive message:\"" << message << "\" from " << temp.header.source_id << endl;
            }
            if (bytes < 0) {
                perror("Error reading from socket");
                break;
            }
            if (bytes == 0) {
                //the server is closed
                cout << "The server is down." << endl;
                break;
            }
            received += bytes;
        } while (received < sizeof(Packet));
        if (temp.header.operation == TIME) {
            cout << "Receive time packet:" << count << endl;
            count++;
            continue;
        }

        data_queue.push_front(temp);

    }
}

void checkQueue() {
    //等待响应
    if (data_queue.empty()) {
        return;
    }
    string message;

    for (auto iter = data_queue.begin(); iter != data_queue.end();) {
        if (iter->header.operation == MESSAGE) {
            message = *(char *) (iter->body.data);
            data_queue.erase(iter);
            break;
        } else {
            iter++;
        }
    }

    if (!message.empty()) {
        cout << message << endl;
    }
}







