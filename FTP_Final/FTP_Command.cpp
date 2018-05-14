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
	int i = 0;

	while (message[0] != ' ' && i < MAX_BUFF) {
		if (message[0] < '0' || message[0] > '9') {
			memset(code, 0, 10);
			return NULL;
		}

		code[0] = message[0];
		code++; message++;
		i++;
	}
	code[0] = '\0';
	return tmp;
}

//Chuyển số port (2^16) thành 2 số (2^8)
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

//Kiểm tra đường dẫn là tuyệt đối hay tương đối 
//(nếu là tương đối thì phải gắn thêm đường dẫn hiện hành vào)
//Đường dẫn tuyệt đối phải bắt đầu bằng tên ổ đĩa (?) vd: C:\a\b\c\d.txt
bool IsAbsolutePath(string s) {
	int len = s.size();

	//Bỏ dấu ngoặc kép nếu có
	if (s[0] == '\"' && s[len - 1] == '\"') {
		s = s.substr(1);
		s[len - 1] = 0;
		s.pop_back();
	}

	if (isalpha(s[0]) && s[1] == ':')
		return true;
	return false;
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

	//Tìm dấu \ đầu tiên từ phải qua rồi cắt lấy tên file
	for (i = len - 1;i >= 0;i--)
		if (path[i] == '\\')
			break;

	ans = path.substr(i + 1);
	return ans;
}

Program::Program(const SOCKET& c, sockaddr_in &sv) {
	command = c;
	data = 0;
	sv_address = sv;
	current_dir = "";
}

Program::Program(const SOCKET& c, sockaddr_in &sv, string s) {
	command = c;
	data = 0;
	sv_address = sv;
	current_dir = s;
}

int Program::Menu() {
	int nResult;
	int i = 0;
	CMenu menu(this);

	//Thêm các tùy chọn cho menu
	menu.Add("List", &Program::List);
	//menu.Add("Dir", Dir);
	menu.Add("Store", &Program::Store);

login:
	nResult = Login();
	if (nResult != 230) {
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
	Quit();
	return nResult;
}

//Gửi qua socket tin s hoặc nội dung file có đường dẫn là s
//Flag = 0: tin. Flag = 1: file
int Program::Send(SOCKET& sock, string s, int flag) {
	if (flag == 0) {
		send(sock, s.c_str(), s.length(), 0);
	}
	else {
		if (!IsAbsolutePath(s))
			s = current_dir + s;
		FILE *f = fopen(s.c_str(), "rb");
		char buf[MAX_BUFF + 1];
		int len = 0;

		memset(buf, 0, MAX_BUFF + 1);

		while (len = fread(buf, 1, MAX_BUFF, f)) {
			send(sock, buf, len, 0);
			memset(buf, 0, MAX_BUFF + 1);
		}
	}
	return 0;
}

//Nhận từ socket nội dung file có tên s
//Flag = 0: tin (không cần s). Flag = 1: tên file
int Program::Recv(SOCKET& sock, string s, int flag) {
	char buf[MAX_BUFF + 1];
	char Code[10];
	int byteRcv = 0;


	FILE *f = NULL;
	if (flag == 1) {
		if (!IsAbsolutePath(s))
			s = current_dir + s;
		f = fopen(s.c_str(), "wb");
		if (f == NULL)
			return -1;
	}
	memset(buf, 0, MAX_BUFF + 1);
	byteRcv = recv(sock, buf, MAX_BUFF, 0);
	if(byteRcv > 0)
		gc(buf, Code);

	//Ghi vào file hoặc in ra màn hình
	if (flag)
		fwrite(buf, byteRcv, 1, f);
	else
		cout << buf;

	int len = strlen(buf);
	while (buf[len-2] != '\r' && buf[len-1] != '\n' && len >= 2) {
		memset(buf, 0, MAX_BUFF + 1);
		byteRcv = recv(sock, buf, MAX_BUFF, 0);

		if (byteRcv <= 0)
			break;

		if (flag)
			fwrite(buf, byteRcv, 1, f);
		else
			cout << buf;
	}
	return atoi(Code);
}

int Program::Login() {
	string user;
	string pass;
	string msg;

	cout << "User: ";
	cin >> user;

	msg = "USER " + user + "\r\n";
	Send(command, msg, 0);
	Recv(command, "", 0);

	cout << "Password: ";
	cin >> pass;

	msg = "PASS " + pass + "\r\n";
	Send(command, msg, 0);
	return Recv(command, "", 0);
}

int Program::Quit() {
	Send(command, "QUIT\r\n", 0);
	return 0;
}

int Program::List() {
	data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in my_addr;
	
	//Yêu cầu server gửi data trên port "dataport"
	Port();

	//Gắn socket lên ip chính mình và port "dataport"
	my_addr.sin_port = htons(dataport);
	SetupIPWS(data, my_addr, false);
		
	Recv(command, "", 0);
	Send(command, "NLST\r\n", 0);
	Recv(command, "", 0);

	//Chờ kết nối của server đến socket data
	if (listen(data, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(data);
		WSACleanup();
		return 1;
	}
	SOCKET s = accept(data, NULL, NULL);
	Recv(s, "", 0);
	
	closesocket(data);
	closesocket(s);
	return 0;
}

int Program::Port() {
	unsigned short port;
	unsigned char a, b;
	unsigned char c, d, e, f;
	char tmp[100];
	string msg;

	port = rand() % PORT_RANGE + 1024;
	
	PortConvert(port, a, b);
	GetLocalAddress(command, c, d, e, f);
	dataport = port;

	sprintf(tmp, "PORT %u,%u,%u,%u,%u,%u\r\n", c, d, e, f, a, b);
	msg = tmp;

	Send(command, msg, 0);
	return Recv(command, "", 0);
}

int Program::Store() {
	string filename;
	string msg;
	sockaddr_in my_addr;

	cin.ignore();
	cout << "Nhap ten file: ";
	getline(cin, filename);

	Port();
	Recv(command, "", 0);

	//Gắn socket vào port "dataport"
	my_addr.sin_port = htons(dataport);
	data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SetupIPWS(data, my_addr, false);
		

	msg = "STOR " + GetFileName(filename) + "\r\n";
	Send(command, msg, 0);
	Recv(command, "", 0);

	sockaddr_in sv_data_addr;
	sv_data_addr = sv_address;
	sv_data_addr.sin_port = htons(SERVER_D_PORT);
	Connect(data, sv_data_addr);

	Send(data, filename, 1);
	closesocket(data);
	return Recv(command, "", 0);
}

/*
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

*/
