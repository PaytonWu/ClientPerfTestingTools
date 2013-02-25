#include "StdAfx.h"

#include <vector>
#include <algorithm>

#include "Keyboard.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// CKeyboard
void Keyboard::SendString(const basic_string<TCHAR>& str, DWORD dwMiliseconds/* = 20*/)
{
    vector<INPUT> vInputs;
    for_each(str.begin(), str.end(), [&vInputs](TCHAR ch) {
        INPUT inputs[6];
        SecureZeroMemory(inputs, sizeof(inputs));

        for (int i = 0; i < _countof(inputs); ++i)
        {
            inputs[i].type = INPUT_KEYBOARD;
            inputs[i].ki.dwFlags = KEYEVENTF_SCANCODE;
        }

        switch (ch)
        {
        case 'f':
            inputs[0].ki.wScan = 0x21;
            inputs[1].ki.wScan = 0xa1;
            inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;

            vInputs.push_back(inputs[0]);
            vInputs.push_back(inputs[1]);
            break;

        case 'c':
            inputs[0].ki.wScan = 0x2e;
            inputs[1].ki.wScan = 0xae;
            inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;

            vInputs.push_back(inputs[0]);
            vInputs.push_back(inputs[1]);
            break;

        case 'g':
            inputs[0].ki.wScan = 0x22;
            inputs[1].ki.wScan = 0xa2;
            inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;

            vInputs.push_back(inputs[0]);
            vInputs.push_back(inputs[1]);
            break;

        case '_':
            inputs[0].ki.wScan = 0x2a;
            inputs[1].ki.wScan = 0x0c;
            inputs[2].ki.wScan = 0x8c;
            inputs[3].ki.wScan = 0xaa;

            inputs[2].ki.dwFlags |= KEYEVENTF_KEYUP;
            inputs[3].ki.dwFlags |= KEYEVENTF_KEYUP;

            vInputs.push_back(inputs[0]);
            vInputs.push_back(inputs[1]);
            vInputs.push_back(inputs[2]);
            vInputs.push_back(inputs[3]);
            break;

        case 'd':
            inputs[0].ki.wScan = 0x20;
            inputs[1].ki.wScan = 0xa0;
            inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;

            vInputs.push_back(inputs[0]);
            vInputs.push_back(inputs[1]);
            break;

        case 'e':
            inputs[0].ki.wScan = 0x12;
            inputs[1].ki.wScan = 0x92;
            inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;

            vInputs.push_back(inputs[0]);
            vInputs.push_back(inputs[1]);
            break;

        case 'v':
            inputs[0].ki.wScan = 0x2f;
            inputs[1].ki.wScan = 0xaf;
            inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;

            vInputs.push_back(inputs[0]);
            vInputs.push_back(inputs[1]);
            break;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            inputs[0].ki.wScan = 0x02 + ch - '1';
            inputs[1].ki.wScan = 0x82 + ch - '1';
            inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;

            vInputs.push_back(inputs[0]);
            vInputs.push_back(inputs[1]);
            break;

        case '0':
            inputs[0].ki.wScan = 0x0b;
            inputs[1].ki.wScan = 0x8b;
            inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;

            vInputs.push_back(inputs[0]);
            vInputs.push_back(inputs[1]);

            break;
        }
    });

    for_each(vInputs.begin(), vInputs.end(), [dwMiliseconds](INPUT& input) {
        Sleep(dwMiliseconds);
        SendInput(1, &input, sizeof(INPUT));
    });

    Sleep(500);
}

void Keyboard::SendKey(DWORD dwVk, DWORD dwMiliseconds/* = 20*/)
{
    INPUT inputs[2];
    SecureZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = static_cast<WORD>(dwVk);
    inputs[1] = inputs[0];
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    Sleep(dwMiliseconds);
    SendInput(1, &inputs[0], sizeof(INPUT));
    Sleep(dwMiliseconds);
    SendInput(1, &inputs[1], sizeof(INPUT));

    Sleep(500);
}
