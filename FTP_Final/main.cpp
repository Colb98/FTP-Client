// FTP_Final.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

/*
int main() {
	FILE *f = fopen("D:\ocr.png", "rb");
	FILE *f1 = fopen("D:\ocr2.png", "wb");
	if (!f)
		return 0;
	char *buffer = new char[1024];
	int counter = 0;
	while (fread(buffer, 1024, 1, f) ==  1) {		
		counter += 1024;
		//cout << buffer;
		fwrite(buffer, 1024, 1, f1);
		memset(buffer, 0, 1024);
	}
	if (feof(f))
		cout << "\nDone\n";
	else
		cout << "\nError\n";
	cout << endl << endl << counter;
	return 0;
}
/**/

int main()
{
	WSADATA SData;
	int iResult = WSAStartup(0x0202, &SData);
	if (iResult != 0) {
		cout << "KHONG THE KHOI DONG WINSOCK";
		return 1;
	}

	cout << "KHOI TAO SOCKET THANH CONG: \n";
	cout << "Phien ban: " << SData.wVersion << "\n";
	cout << "Phien ban co the ho tro: " << SData.wHighVersion << "\n";
	cout << "Ghi chu: " << SData.szDescription << "\n";
	cout << "Thong tin cau hinh: " << SData.szSystemStatus << "\n";

	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in my_addr;
	sockaddr_in sv_cmd_addr;

	int nResult = SetupIPWS(sockfd, my_addr);
	if (nResult) {
		//Lỗi khi khởi tạo
		cout << "Co loi khi khoi tao socket" << endl;
		cin.ignore();
		return 1;
	}
	
	//Sửa chỗ này thành IP của máy ảo
	char c_serveraddr[50] = "192.168.18.128";
	//cin >> c_serveraddr;
	//cin.ignore();

	sv_cmd_addr.sin_addr.s_addr = inet_addr(c_serveraddr);
	sv_cmd_addr.sin_port = htons(SERVER_C_PORT);
	sv_cmd_addr.sin_family = AF_INET;
	
	cout << "Connecting to FPT server at: " << c_serveraddr << endl;
	nResult = Connect(sockfd, sv_cmd_addr);
	if (nResult) {
		//Gap loi
		cout << "Loi khi ket noi server. Chuong trinh se tu dong" << endl;
		Sleep(3000);
		WSACleanup();
		return 1;
	}

	cout << "Connected on port " << ntohs(my_addr.sin_port) << "!!\n\n";
	int timeout = 1000;

	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int)))
	{
		perror("setsockopt");
		return -1;
	}

	char RecvBuf[1024];
	recv(sockfd, RecvBuf, 1024, 0);
	eom(RecvBuf);
	cout << RecvBuf;

	nResult = Menu(sockfd, sv_cmd_addr);

	closesocket(sockfd);
	WSACleanup();
	return 0;
}

/**/
