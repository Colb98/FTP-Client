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
	menu.Add("Get", &Program::Retrieve);
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
		
		if (s[0] == '\"' && s[s.size() - 1] == '\"')
			s = s.substr(1, s.size() - 2);
		FILE *f = fopen(s.c_str(), "rb");
		char buf[MAX_BUFF + 1];
		int len = 0;

		memset(buf, 0, MAX_BUFF + 1);

		while (len = fread(buf, 1, MAX_BUFF, f)) {
			send(sock, buf, len, 0);
			memset(buf, 0, MAX_BUFF + 1);
		}

		fclose(f);
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

	while (buf[byteRcv-2] != '\r' || buf[byteRcv-1] != '\n') {
		memset(buf, 0, MAX_BUFF + 1);
		byteRcv = recv(sock, buf, MAX_BUFF, 0);

		if (byteRcv <= 0)
			break;

		if (flag)
			fwrite(buf, byteRcv, 1, f);
		else
			cout << buf;
	}
	if (f)
		fclose(f);

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

int Program::Retrieve() {
	string filename;
	string msg;
	sockaddr_in my_addr;

	cin.ignore();
	cout << "Nhap ten file: ";
	getline(cin, filename);

	Port();
	Recv(command, "", 0);
	my_addr.sin_port = htons(dataport);
	data = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SetupIPWS(data, my_addr, false);
	
	msg = "RETR " + filename + "\r\n";
	Recv(command, "", 0);
	Send(command, msg, 0);
	Recv(command, "", 0);

	//Chờ kết nối của server đến socket data
	if (listen(data, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(data);
		WSACleanup();
		return 1;
	}
	SOCKET s = accept(data, NULL, NULL);
	Recv(s, filename, 1);
	Recv(command, "", 0);

	closesocket(data);
	closesocket(s);
	return 0;
}
