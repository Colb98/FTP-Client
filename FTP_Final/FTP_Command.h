#pragma once
#define eom EndOfMessage
#define gc GetCode
#include "stdafx.h"

int EndOfMessage(char*);
char* GetCode(char*, char*);
void PortConvert(unsigned short, unsigned char &, unsigned char &);
void GetLocalAddress(SOCKET&, unsigned char&, unsigned char&, unsigned char&, unsigned char&);
std::string GetFileName(std::string s);
int Menu(SOCKET&, sockaddr_in&);
int Login(SOCKET&, sockaddr_in&);
int Quit(SOCKET&, sockaddr_in&);
int Send(SOCKET&, char*, sockaddr_in&, bool = false);
int Recv(SOCKET&, sockaddr_in&);
int Dir(SOCKET&, sockaddr_in&);
int List(SOCKET&, sockaddr_in&);
void RecvThread(bool *flag, unsigned short port);
int SendThread(bool *flag, unsigned short port, const char*, sockaddr_in&);
//Thông báo port nhận data cho server
int DataPort(SOCKET&, sockaddr_in&, unsigned short&);
int Store(SOCKET& sock, sockaddr_in& svip);