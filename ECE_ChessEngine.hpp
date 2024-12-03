#include <string>
#include <iostream>
#include <string>
#include <windows.h>
#include <regex>
#include "chessCommon.h"

bool InitializeEngine();

bool sendMove(const std::string& strMove);

bool getResponseMove(std::string& strMove);

std::string ReadFromEngine();