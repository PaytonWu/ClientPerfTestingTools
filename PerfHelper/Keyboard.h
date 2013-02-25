#pragma once

#include <string>

// 
class Keyboard
{
public:
    static void SendString(const std::basic_string<TCHAR>& str, DWORD dwMiliseconds = 20);
    static void SendKey(DWORD dwVk, DWORD dwMiliseconds = 20);
};

