// PerfHelper.cpp : Defines the entry point for the application.
//


#include "stdafx.h"

//#include <direct.h>

#include <memory>
#include <functional>

#include "PerfHelper.h"
#include "TestExecuter.h"
#include "ProcessInfo.h"
#include "Loger.h"
#include "Window.h"

using namespace std;

const int nMaxNum = 255;
struct ScopeGuard
{
    explicit ScopeGuard(function<void()> onExitScope) : onExitScope_(onExitScope) {}
    ~ScopeGuard() { onExitScope_(); }
private:
    function<void()> onExitScope_;
};

int _tmain(int argc, TCHAR* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    SecureZeroMemory(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);

    // 创建日志对象，用来记录整个过程中的各类消息
    LogerMgr logerMgr;
    try
    {
        logerMgr.AddLoger(make_shared<CStdOutLoger>(), LogerMgr::LT_MESSAGE);
        logerMgr.AddLoger(make_shared<CFileLoger>(_T(".\\warning.log")), LogerMgr::LT_WARNING);
        logerMgr.AddLoger(make_shared<CFileLoger>(_T(".\\error.log")), LogerMgr::LT_ERROR);
    }
    catch (const bad_alloc&)
    {
    }

    logerMgr.LogMessage(_T("启动客户端（%s）\n"), argv[1]);

    if (FALSE == CreateProcess(NULL, argv[1], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        logerMgr.LogError(_T("客户端启动失败\n"));
        return EXIT_FAILURE;
    }

    WaitForInputIdle(pi.hProcess, INFINITE);
    Sleep(1000);

    try
    {
        ProcessInfo processInfo(pi);
        CTestExecuterMgr mgr(processInfo);

        try
        {
            mgr.AddTest(std::make_shared<CStartupTestExecuter>(mgr));
            mgr.AddTest(std::make_shared<CPageSwitchTestExecuter>(mgr));
            mgr.AddTest(std::make_shared<CKPagePageDownTestExecuter>(mgr));
            mgr.AddTest(std::make_shared<CMinutePagePageDownTestExecuter>(mgr));
        }
        catch (const bad_alloc&)
        {
            logerMgr.LogWarning(_T("Caught exception: bad_alloc!\n"));
        }

        mgr.RunTest(logerMgr);
    }
    catch (const runtime_error& e)
    {
#ifdef _UNICODE
        const char* reason = e.what();
        size_t lenWithNull = strlen(reason) + 1;
        unique_ptr<TCHAR[]> spReason(new TCHAR[sizeof(TCHAR) * lenWithNull]);
        memset(spReason.get(), 0, sizeof(TCHAR) * lenWithNull);

        for (size_t i = 0; i < lenWithNull - 1; ++i)
        {
            spReason[i] = reason[i];
        }
#else
        LPCTSTR spReason = e.what();
#endif
        logerMgr.LogError(_T("Caught exception: runtime_error(%s)!\n"), spReason);
        return EXIT_FAILURE;
    }

    ///////////////////////////////////////////////////////////////////////
    // 测试用例已经跑结束了

    ///////////////////////////////////////////////////////////////////////
    // 创建和客户端通信的server端pipe
    LPCTSTR lpszPipeName = _T("\\\\.\\pipe\\hxperftest");
    HANDLE hPipe = CreateNamedPipe(lpszPipeName,                                    // pipe name
        PIPE_ACCESS_DUPLEX,                              // open mode
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, // pipe mode
        2,                                               // pipe instance count
        128,                                             // output buffer size
        128,                                             // input buffer size
        1000,                                            // time-out value
        NULL);                                           // SECURITY_ATTRIBUTES

    if (INVALID_HANDLE_VALUE == hPipe)
    {
        logerMgr.LogError(_T("Unable to create the pipe\n"));
        return EXIT_FAILURE;
    }

    ScopeGuard autoClosePipe([&hPipe] { CloseHandle(hPipe); });

    ///////////////////////////////////////////////////////////////////////
    // 注册消息，查找客户端通信窗口，并发送测试结束消息
    LPCTSTR szMsgStop = _T("4F3D8A93-84B9-400E-9173-C695D497CEAD");
    UINT uRegMsgStop = RegisterWindowMessage(szMsgStop);
    if (!uRegMsgStop)
    {
        logerMgr.LogError(_T("RegisterWindowMessage failed!\n"));
        return EXIT_FAILURE;
    }

    // 获取日志隐藏窗口句柄
    HWND hPerfMsgDlg = CWndSearchAssistant::Search<CWndSearchAssistant::TEXT_MODE>(_T("class='15e4a3e8-0587-4997-a692-565c0ad41b85'"));
    if (!IsWindow(hPerfMsgDlg))
    {
        logerMgr.LogError(_T("Communication window not found\n"));
        return EXIT_FAILURE;
    }

    PostMessage(hPerfMsgDlg, uRegMsgStop, 0, 0);
    ///////////////////////////////////////////////////////////////////////

    // 等待客户端
    // 等待返回消息，开启解析
    string logFile;
    if (ConnectNamedPipe(hPipe, NULL) || (GetLastError() == ERROR_PIPE_CONNECTED))
    {
        DWORD cbBytesRead;  
        char chRequest[nMaxNum];  
        if (!ReadFile (hPipe, chRequest, nMaxNum, &cbBytesRead, NULL))
        {
            logerMgr.LogError(_T("ReadFile failed!\n"));
            return EXIT_FAILURE;
        }
        chRequest[cbBytesRead] = '\0';  
        logFile.assign(chRequest);

        if (logFile.length() <= 0)
        {
            logerMgr.LogError(_T("pipe protocol match failed!\n"));
            return EXIT_FAILURE;
        }

        logerMgr.LogMessage("Connection finished!\n");
    }
    else
    {
        logerMgr.LogError(_T("ConnectNamedPipe returns: %u\n"), GetLastError());
        return EXIT_FAILURE;
    }

    ScopeGuard autoDisconnectPipe([&hPipe] { DisconnectNamedPipe(hPipe); });
    logerMgr.LogMessage(_T("日志保存结束\n"));

    ///////////////////////////////////////////////////////////////////////////

    // 准备传递给日志分析程序的命令行参数
    TCHAR lpszCurrentWorkingFolder[FILENAME_MAX];
    _tgetcwd(lpszCurrentWorkingFolder, FILENAME_MAX);
    basic_string<TCHAR> logFilePath(lpszCurrentWorkingFolder);
    if (_T('\\') != logFilePath.back())
    {
        logFilePath += _T('\\');
    }

    // 窄字符转宽字符
    for_each(begin(logFile), end(logFile), [&logFilePath](char ch)
    {
        logFilePath.push_back(ch);
    });

    basic_string<TCHAR> toolCmdLine = basic_string<TCHAR>(_T("LogParser.exe -in:")) + logFilePath + _T(" -out:") + logFilePath + _T(".xlsx -mode:0");
    unique_ptr<TCHAR> cmdLine(new TCHAR[toolCmdLine.size() + 1]);
    memset(cmdLine.get(), 0, sizeof(TCHAR) * (toolCmdLine.size() + 1));
    memcpy(cmdLine.get(), toolCmdLine.c_str(), sizeof(TCHAR) * (toolCmdLine.size() + 1));

    // 打开解析程序
    PROCESS_INFORMATION piAny;
    STARTUPINFO siAny;
    SecureZeroMemory(&siAny, sizeof(siAny));
    siAny.cb = sizeof(STARTUPINFO);
    if (CreateProcess(NULL, cmdLine.get(), NULL, NULL, FALSE, 0, NULL, NULL, &siAny, &piAny))
    {
        Sleep(1000);
    }
    else
    {
        //logerMgr.LogError(_T("打开解析程序失败!\n"));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

