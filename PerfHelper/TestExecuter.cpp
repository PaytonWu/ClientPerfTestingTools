#include "StdAfx.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>

#include "TestExecuter.h"
#include "Keyboard.h"
#include "ProcessInfo.h"
#include "Window.h"
#include "Loger.h"

using namespace std;

ITestExecuter::ITestExecuter(void)
{
}


ITestExecuter::~ITestExecuter(void)
{
}

///////////////////////////////////////////////////////////////////////////////
// CTestExecuterMgr
CTestExecuterMgr::CTestExecuterMgr(const ProcessInfo& pi)
    : m_refProcessInfo(pi)
{
    if (!AttachThreadInput(GetCurrentThreadId(), m_refProcessInfo.GetTargetProcessMainThreadID(), TRUE))
    {
        // DWORD dwErr = GetLastError();
        throw runtime_error("AttachThreadInput failed!");
    }
}

CTestExecuterMgr::~CTestExecuterMgr()
{
    AttachThreadInput(GetCurrentProcessId(), m_refProcessInfo.GetTargetProcessMainThreadID(), FALSE);
}

void CTestExecuterMgr::RunTest(LogerMgr& logerMgr)
{
    for_each(m_spTests.begin(), m_spTests.end(), [this, &logerMgr](const shared_ptr<CTestExecuter>& ele) {
        ele->RunTest(logerMgr);
    });
}

void CTestExecuterMgr::AddTest(const shared_ptr<CTestExecuter>& spTest)
{
    m_spTests.push_back(spTest);
}


///////////////////////////////////////////////////////////////////////////////
// CTestExecuter
CTestExecuter::CTestExecuter(const CTestExecuterMgr& refTestExeMgr)
    : m_refTestExeMgr(refTestExeMgr)
{
}

void CTestExecuter::SendKeyboardString(const std::basic_string<TCHAR>& str) const
{
    Keyboard::SendString(str);
}

void CTestExecuter::SendKeyboardCode(DWORD dwVk) const
{
    Keyboard::SendKey(dwVk);
}


///////////////////////////////////////////////////////////////////////////////
// 登陆相关性能测试
CStartupTestExecuter::CStartupTestExecuter(const CTestExecuterMgr& refTestExeMgr)
    : CTestExecuter(refTestExeMgr)
{
}

HWND CStartupTestExecuter::FindWindowInRetryMode(const basic_string<TCHAR>& searchCondition, unsigned int retryCount/* = 50*/) const
{
    HWND hWnd = NULL;
    DWORD dwSleepTime = 100;

    for (unsigned i = 0; i < retryCount; ++i)
    {
        hWnd = CWndSearchAssistant::Search<CWndSearchAssistant::TEXT_MODE>(searchCondition, NULL);
        if (IsWindow(hWnd))
        {
            break;
        }

        Sleep(dwSleepTime);
    }

    return hWnd;
}

void CStartupTestExecuter::ByPassWarningMsgBox(LogerMgr& logerMgr)
{
    HWND hWarningDlg = FindWindowInRetryMode(_T("class='#32770',title='同花顺'"));
    if (NULL == hWarningDlg)
    {
        logerMgr.LogWarning(_T("警告对话框没有找到！\n"));
        return;
    }

    SendKeyboardCode(VK_RETURN);
}

void CStartupTestExecuter::ByPassLoginDlg(LogerMgr& logerMgr)
{
    HWND hLoginDlg = FindWindowInRetryMode(_T("class='#32770', title='登录到全部行情主站'"));
    if (!IsWindow(hLoginDlg))
    {
        logerMgr.LogError(_T("登陆对话框没有找到\n"));
        return;
    }

    // 账号
    HWND hChild = CWndSearchAssistant::Search<CWndSearchAssistant::TEXT_MODE>(_T("class='ComboBox'; class='edit'"), hLoginDlg);

    if (!IsWindow(hChild))
    {
        logerMgr.LogError(_T("行情登陆对话框没有找到\n"));
    }

    if (GetFocus() != hChild)
    {
        logerMgr.LogWarning(_T("账号输入框默认没有获得焦点！\n"));
    }

    SendKeyboardString(_T("fcg_dev"));

    // 密码
    hChild = CWndSearchAssistant::Search<CWndSearchAssistant::TEXT_MODE>(_T("class='edit'"), hLoginDlg);

    if (!IsWindow(hChild))
    {
        logerMgr.LogError(_T("密码输入框没有找到!\n"));
    }

    SetFocus(hChild);

    if (GetFocus() != hChild)
    {
        logerMgr.LogWarning(_T("密码输入框没有焦点！\n"));
    }

    SendKeyboardString(_T("987654321"));

    // 登陆按钮
    hChild = CWndSearchAssistant::Search<CWndSearchAssistant::TEXT_MODE>(_T("class='button', title='登录'"), hLoginDlg);
    if (!IsWindow(hChild))
    {
        hChild = CWndSearchAssistant::Search<CWndSearchAssistant::TEXT_MODE>(_T("class='button', title='登   录'"), hLoginDlg);
    }

    if (!IsWindow(hChild))
    {
        logerMgr.LogError(_T("登陆按钮没有找到!\n"));
    }

    SetFocus(hChild);

    if (GetFocus() != hChild)
    {
        logerMgr.LogWarning(_T("登陆按钮没有键盘焦点！\n"));
    }

    SendKeyboardCode(VK_RETURN);

    // hChild = FindWindowInRetryMode(_T(""), 500);
}

void CStartupTestExecuter::RunTest(LogerMgr& logerMgr)
{
    ByPassWarningMsgBox(logerMgr);
    ByPassLoginDlg(logerMgr);
}

///////////////////////////////////////////////////////////////////////////////
// CPageSwitchTestExecuter
CPageSwitchTestExecuter::CPageSwitchTestExecuter(const CTestExecuterMgr& refTestExeMgr)
    : CTestExecuter(refTestExeMgr)
{
}

void CPageSwitchTestExecuter::RunTest(LogerMgr& logerMgr)
{
    Sleep(10000);
    logerMgr.LogMessage(_T("键盘精灵：80页面\n"));

    // 80
    SendKeyboardString(_T("80"));
    SendKeyboardCode(VK_RETURN);

    Sleep(1000);

    logerMgr.LogMessage(_T("键盘精灵：81页面\n"));
    SendKeyboardString(_T("81"));
    SendKeyboardCode(VK_RETURN);

    logerMgr.LogMessage(_T("回退\n"));
    SendKeyboardCode(VK_ESCAPE);
    SendKeyboardCode(VK_ESCAPE);

    Sleep(1000);
}

///////////////////////////////////////////////////////////////////////////////
// CKPagePageDownTestExecuter
CKPagePageDownTestExecuter::CKPagePageDownTestExecuter(const CTestExecuterMgr& refTestExeMgr)
    : CTestExecuter(refTestExeMgr)
{
}

void CKPagePageDownTestExecuter::RunTest(LogerMgr& logerMgr)
{
    Sleep(30000);

    logerMgr.LogMessage(_T("键盘精灵：1页面\n"));
    SendKeyboardString(_T("1"));
    SendKeyboardCode(VK_RETURN);
    Sleep(1000);

    logerMgr.LogMessage(_T("F5\n"));
    SendKeyboardCode(VK_F5);
    Sleep(1000);

    for (int i = 0; i < 10; ++i)
    {
        logerMgr.LogMessage(_T("PageDown (第%d次)\n"), i);
        SendKeyboardCode(VK_NEXT);
        Sleep(3000);
    }

    // ESCs
    logerMgr.LogMessage(_T("回退\n"));
    SendKeyboardCode(VK_ESCAPE);
    SendKeyboardCode(VK_ESCAPE);
}

///////////////////////////////////////////////////////////////////////////////
// CMinutePagePageDownTestExecuter
CMinutePagePageDownTestExecuter::CMinutePagePageDownTestExecuter(const CTestExecuterMgr& refTestExeMgr)
    : CTestExecuter(refTestExeMgr)
{
}

void CMinutePagePageDownTestExecuter::RunTest(LogerMgr& logerMgr)
{
    // 3页面
    logerMgr.LogMessage(_T("键盘精灵：3页面\n"));
    SendKeyboardString(_T("3"));
    SendKeyboardCode(VK_RETURN);

    // 回车
    logerMgr.LogMessage(_T("进入分时\n"));
    SendKeyboardCode(VK_RETURN);

    // 翻页
    for (int i = 0; i < 20; ++i)
    {
        logerMgr.LogMessage(_T("PageDown (第%d次)\n"), i);
        SendKeyboardCode(VK_NEXT);
        Sleep(3000);
    }

    // ESCs
    logerMgr.LogMessage(_T("回退\n"));
    SendKeyboardCode(VK_ESCAPE);
    SendKeyboardCode(VK_ESCAPE);
}
