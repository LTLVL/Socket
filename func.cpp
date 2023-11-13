//
// Created by Administrator on 2023/11/13.
//
#include "iostream"

#pragma once

using namespace std;

extern void connect();

extern void disconnect();

extern void queryTime();

extern void queryName();

extern void queryAliveList();

extern void sendMessage();

extern void quit();

void connect() {

}

void disconnect() {

}

void queryTime() {

}

void queryName() {

}

void queryAliveList() {

}

void sendMessage() {

}

void quit(bool *i) {
    *i = true;
    cout << "Good bye!" << endl;
}





