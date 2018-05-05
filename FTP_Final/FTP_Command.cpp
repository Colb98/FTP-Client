#include "stdafx.h"
using namespace std;


//Thêm \0 vào cuối chuỗi, nếu chuỗi kết thúc bằng \r\n
int EndOfMessage(char* s) {
	int i = 0;
	while ((s[0] != '\r' || s[1] != '\n') && i < 512 - 2)
		s++, i++;

	if (i < 512 - 2) {
		s[2] = '\0';
		return 0;
	}

	if (s[0] == '\r' && s[1] == '\n') {
		s[0] = '\n';
		s[1] = '\0';
		return 0;
	}

	if (s[1] == '\r')
		return 1;

	return 2;
}

//Lấy mã 
char* GetCode(char* message, char* code) {
	char* tmp = code;
	while (message[0] != ' ') {
		code[0] = message[0];
		code++; message++;
	}
	code[0] = '\0';
	return tmp;
}

void PortConvert(unsigned short p, unsigned char &a, unsigned char &b) {
	int tmp[16] = { 0 };
	int i = 0;
	a = b = 0;

	while (i < 16) {
		tmp[i] = p % 2;
		p /= 2;		

		if (i < 8) {
			tmp[i] <<= i;
			b += tmp[i];
		}
		else {
			tmp[i] <<= i - 8;
			a += tmp[i];
		}

		i++;
	}
}

void GetLocalAddress(SOCKET& sock, unsigned char& a, unsigned char& b, unsigned char& c, unsigned char& d) {
	sockaddr_in my_addr;
	int addrlen = sizeof(my_addr);
	getsockname(sock, (sockaddr*)&my_addr, &addrlen);
	a = my_addr.sin_addr.S_un.S_un_b.s_b1;
	b = my_addr.sin_addr.S_un.S_un_b.s_b2;
	c = my_addr.sin_addr.S_un.S_un_b.s_b3;
	d = my_addr.sin_addr.S_un.S_un_b.s_b4;
}

int Menu(SOCKET& sock, sockaddr_in& svip) {
	int nResult;
	int i = 0;
	CMenu menu(&sock, &svip);

	//Thêm các tùy chọn cho menu
	menu.Add("List", List);
	menu.Add("Dir", Dir);
	menu.Add("Store", Store);

login: 
	nResult = Login(sock, svip);
	if (nResult) {
		cout << "Ban muon login lai? (1/0)";
		cin >> nResult;
		if (nResult)
			goto login;
		else
			goto quit;
	}
	
	//Hiện menu, yêu cầu làm
	bool showMenu = true;
	do {
		if (showMenu) {
			menu.Show();
			showMenu = false;
		}
		i = menu.Select();
		cout << endl;
		if (i == 400)
			showMenu = true;
	} while (i != 404);

quit: 
	cout << "Ket thuc ket noi voi server\n";
	Quit(sock, svip);
	return nResult;
}

int Login(SOCKET& sock, sockaddr_in& svip) {
	char user[256] = { 0 };
	char pass[256] = { 0 };
	char SendBuf[1024];
	char RecvBuf[1024];
	char Code[10];
	int val;

	//Nhap username
	cout << "User: ";
	cin >> user;

	sprintf(SendBuf, "USER %s\r\n", user);
	send(sock, SendBuf, strlen(SendBuf), 0);
	recv(sock, RecvBuf, 1024, 0);
	eom(RecvBuf);
	gc(RecvBuf, Code);
	cout << RecvBuf;
	
	//Nhap password
	cout << "Pass: ";
	cin >> pass;

	sprintf(SendBuf, "PASS %s\r\n", pass);
	send(sock, SendBuf, strlen(SendBuf), 0);
	recv(sock, RecvBuf, 1024, 0);
	eom(RecvBuf);
	gc(RecvBuf, Code);
	cout << RecvBuf;

	if (strcmp(Code, "230") == 0)
		val = 0;
	else
		val = -1;

	Sleep(0);
	return val;
}

int Quit(SOCKET &sock, sockaddr_in& svip) {
	char *msg = "QUIT\r\n";
	return send(sock, msg, strlen(msg), 0);
}

int Send(SOCKET &sock, char *msg, sockaddr_in& svip, bool fixedSize) {
	char rcvBuff[512] = { 0 };
	//Giữ code mà server trả về, có lẽ sẽ dùng
	char Code[10];
	int byteRecv = 0;
	int endmsg = 0;
	int len = strlen(msg);
	if (len > 1024 || fixedSize)
		len = 1024;
	send(sock, msg, len, 0);
	return 0;
}

int Recv(SOCKET &sock, sockaddr_in& svip) {
	char rcvBuff[513] = { 0 };
	//Giữ code mà server trả về, có lẽ sẽ dùng
	char Code[10];
	int byteRecv = 0;
	
	//Nhận tin, lấy code
	//Nếu chưa kết thúc tin thì in ra
	do {

		cout << rcvBuff;
		memset(rcvBuff, 0, 512);
		byteRecv = recv(sock, rcvBuff, 512, 0);
		if (byteRecv <= 0)
			break;
		gc(rcvBuff, Code);
		if (atoi(Code) == 425)
			break;
		//rcvBuff[byteRecv] = 0;
		//Nhận đến khi nào không còn byte nào để nhận 
	} while (byteRecv > 0);
	cout << rcvBuff;
	return atoi(Code);
}

//LIST\r\n

int List(SOCKET& sock, sockaddr_in& svip) {
	int code;
	unsigned short port;

	DataPort(sock, svip, port);
	
	
	bool flag = false;
	thread t(RecvThread, &flag, port);
	

	Send(sock, "LIST\r\n", svip);
	Recv(sock, svip);

	//Thay bằng kiểm tra xem server có gửi data không
	//Bắt đầu lắng nghe ở socket nhận data
	if (true)
		flag = true;
	
	code = Recv(sock, svip);

	t.join();
	return 0;
}

int Dir(SOCKET& sock, sockaddr_in& svip) {
	return List(sock, svip);
}

void RecvThread(bool *flag, unsigned short port) {
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKET sock1;
	int nSize = sizeof(sockaddr);
	sockaddr_in my_addr;
	sockaddr_in sv_data_addr;
	my_addr.sin_port = htons(port);

	//Khởi tạo socket cho thread mới
	int nResult = SetupIPWS(sock, my_addr, false);
	if (nResult) {
		//Lỗi khi khởi tạo
		cout << "Co loi khi khoi tao socket" << endl;
		cin.ignore();
		return;
	}

	//Chờ server xác nhận
	while (!flag)
		Sleep(10);


	//Chờ kết nối từ server
	nResult = listen(sock, 3);
	if (nResult < 0) {
		cout << "Loi khi lang nghe ket noi\n";
		WSACleanup();
		return;
	}

	sock1 = accept(sock, (sockaddr*)&sv_data_addr, &nSize);

	Recv(sock1, sv_data_addr);
	closesocket(sock);
	closesocket(sock1);
}

int SendThread(bool *flag, unsigned short port, const char* path, sockaddr_in& svip) {
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int nSize = sizeof(sockaddr);
	sockaddr_in my_addr;
	sockaddr_in sv_data_addr = svip;
	my_addr.sin_port = htons(port);
	FILE *f = fopen(path, "rb");
	char *buffer = new char[1025];
	int val = 1;


	if (f == NULL)
		return 1;

	//Khởi tạo socket cho thread mới
	int nResult = SetupIPWS(sock, my_addr, false);
	if (nResult) {
		//Lỗi khi khởi tạo
		cout << "Co loi khi khoi tao socket" << endl;
		cin.ignore();
		return 1;
	}

	//Chờ server xác nhận
	while (!flag)
		Sleep(10);

	sv_data_addr.sin_port = htons(SERVER_D_PORT);
	//Kết nối đến server
	nResult = Connect(sock, sv_data_addr);
	if (nResult < 0) {
		cout << "Loi khi ket noi data den server\n";
		WSACleanup();
		delete[] buffer;
		return 1;
	}	

	memset(buffer, 0, 1025);
	while(fread(buffer,1024,1,f)){		
		Send(sock, buffer, sv_data_addr, true);
		memset(buffer, 0, 1025);
	}
	Send(sock, buffer, sv_data_addr, true);

	if (feof(f)) {
		val = 0;
		cout << "Read and sent file successfully\n";		
	}
	else 
		cout << "Error\n";
		
	closesocket(sock);
	delete[] buffer;
	return val;
}

int DataPort(SOCKET& sock, sockaddr_in& svip, unsigned short& p) {
	int code;
	unsigned short port;
	unsigned char a, b;
	unsigned char c, d, e, f;

	do {
		port = rand() % PORT_RANGE + 1024;
	} while (port == svip.sin_port);
	//Tách số hiệu port (16 bit) thành 2 số 8bit
	PortConvert(port, a, b);
	//Lấy địa chỉ IP của chính mình 
	GetLocalAddress(sock, c, d, e, f);

	char msg[512];

	//Thông báo port nhận data ở client
	sprintf(msg, "PORT %u,%u,%u,%u,%u,%u\r\n", c, d, e, f, a, b);
	Send(sock, msg, svip);
	code = Recv(sock, svip);

	p = port;
	return code;
}

int Store(SOCKET& sock, sockaddr_in& svip) {
	string path;
	unsigned short port;
	int code;
	bool flag = false;
	char msg[1024];
	
	cin.ignore();
	cout << "Nhap duong dan file: ";
	getline(cin, path);

	//Gửi STOR <tên file>\r\n đến server
	sprintf(msg, "STOR %s\r\n", GetFileName(path).c_str());

	DataPort(sock, svip, port);
	thread t(SendThread, &flag, port, path.c_str(), svip);
	
	Send(sock, msg, svip);
	Recv(sock, svip);

	code = Recv(sock, svip);
	flag = true;
	t.join();
	return 0;
}

string GetFileName(string path) {
	string ans;
	int i;
	int len = path.size();
	//Lấy tên file
	//Nếu địa chỉ có dấu ngoặc kép thì bỏ
	if (path[len - 1] == '\"' && path[0] == '\"') {
		path[len - 1] = path[0] = '\0';
		path.pop_back();
		path = path.substr(1);
		len -= 2;
	}
	
	for (i = len - 1;i >= 0;i--)
		if(path[i] == '\\') 
			break;

	ans = path.substr(i+1);
	return ans;
}