#pragma once
#define eom EndOfMessage
#define gc GetCode
#define MAX_BUFF 1024
#include "stdafx.h"

int EndOfMessage(char*);
char* GetCode(char*, char*);
void PortConvert(unsigned short, unsigned char &, unsigned char &);
void GetLocalAddress(SOCKET&, unsigned char&, unsigned char&, unsigned char&, unsigned char&);
std::string GetFileName(std::string s);
bool IsAbsolutePath(std::string s);

class Program {
	private:
		SOCKET command;
		SOCKET data;
		std::string current_dir;
		sockaddr_in sv_address;
		unsigned short dataport;

	public:
		int (Program::*f)();
		Program(const SOCKET &c, sockaddr_in &sv);
		Program(const SOCKET &c, sockaddr_in &sv, std::string dir);
		int Menu();
		int Send(SOCKET& sock, std::string s, int flag);
		int Recv(SOCKET& sock, std::string s, int flag);
		int Login();
		int Quit();
		int List();
		int Port();
		int Store();
		int Retrieve();
};

//int Login(SOCKET&, sockaddr_in&);
//int Quit(SOCKET&, sockaddr_in&);
//int Send(SOCKET&, char*, sockaddr_in&, bool = false);
//int Recv(SOCKET&, sockaddr_in&);
//int Dir(SOCKET&, sockaddr_in&);
//int List(SOCKET&, sockaddr_in&);
//void RecvThread(bool *flag, unsigned short port);
//int SendThread(bool *flag, unsigned short port, const char*, sockaddr_in&);
////Thông báo port nhận data cho server
//int DataPort(SOCKET&, sockaddr_in&, unsigned short&);
//int Store(SOCKET& sock, sockaddr_in& svip);