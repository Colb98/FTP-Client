#pragma once
#include "stdafx.h"


class CMenu {
private:
	//Mảng tên dòng tùy chọn
	std::vector<std::string> option;
	//Mảng các hàm
	std::vector<int(*)(SOCKET&, sockaddr_in&)> function;
	//SOCKET hiện hành
	SOCKET *sock;
	sockaddr_in *myip;
public:
	//Khởi tạo
	CMenu(SOCKET*, sockaddr_in*);
	//Thêm dòng mới
	void Add(std::string name, int(*f)(SOCKET&, sockaddr_in&));
	//Hiển thị các dòng tùy chọn
	void Show();
	//Yêu cầu chọn, trả về kết quả của hàm được chọn
	int Select();
};