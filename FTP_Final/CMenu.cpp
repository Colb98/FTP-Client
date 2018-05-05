#include "stdafx.h"

using namespace std;

CMenu::CMenu(SOCKET *asock, sockaddr_in* ip) {
	option.push_back("Quit");
	function.push_back(nullptr);
	sock = asock;
	myip = ip;
}


void CMenu::Add(string name, int(*f)(SOCKET&, sockaddr_in&)) {
	option.insert(option.end() - 1, name);
	function.insert(function.end() - 1, f);
}

void CMenu::Show() {
	int i = 0;
	int n = option.size();

	for (;i < n;i++) {
		cout << i + 1 << ". ";
		cout << option[i] << endl;
	}
	cout << endl;
}

int CMenu::Select() {
	string s;
	int i;
	cout << "Ban lua chon: ";
	cin >> s;
	if (s == "m" || s == "menu" || s == "help")
		return 400;

	i = stoi(s);
	if (i == option.size())
		return 404;


	return function[i - 1](*sock,*myip);
}