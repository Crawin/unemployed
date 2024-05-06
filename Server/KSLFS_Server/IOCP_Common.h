#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <unordered_map>
#include <DirectXMath.h>
#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "MSWSock.lib")
#include "Packets.h"
#include <filesystem>
#include <fstream>
#include <DirectXCollision.h>
#include <functional>
#include <concurrent_unordered_map.h>
#include <atomic>
#include <memory>

void print_error(const char* msg, int err_no);