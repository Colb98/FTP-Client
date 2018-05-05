// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <winsock2.h>
#pragma comment (lib,"ws2_32.lib")
#include <atlbase.h>
#include <atlstr.h>
#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include "Setup_Socket.h"
#include "FTP_Command.h"
#include "CMenu.h"

#define SERVER_C_PORT 21
#define SERVER_D_PORT 20
#define PORT_RANGE 64512
#define ERROR_LIMIT 3


// TODO: reference additional headers your program requires here
