//
// Created by Administrator on 2023/11/13.
//
#pragma once

#include "iostream"
#include "func.cpp"
extern void menu();

using namespace std;

void showMenu();

void menu() {
    bool isQuit = false;
    int rec = 0;
    while (!isQuit) {
        showMenu();
        cin >> rec;
        switch (rec) {
            case 1: {
                connect();
            }
            case 2: {
                disconnect();
            }
            case 3: {
                queryTime();
            }
            case 4: {
                queryName();
            }
            case 5: {
                queryAliveList();
            }
            case 6: {
                sendMessage();
            }
            case 7: {
                quit(&isQuit);
                break;
            }
            default: {
                cout << "error input" << endl;
                break;
            }
        }
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