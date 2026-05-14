#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <conio.h>
#include "Packet.h"

#pragma comment(lib, "ws2_32")

#define SERVER_PORT 31000