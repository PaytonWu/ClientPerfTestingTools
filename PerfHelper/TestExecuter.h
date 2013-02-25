#pragma once

#include <memory>
#include <vector>

class LogerMgr; 
class ITestExecuter
{
public:
    ITestExecuter(void);
    virtual ~ITestExecuter(void);

    virtual void RunTest(LogerMgr& logerMgr) = 0;
};

class ProcessInfo;
class CTestExecuter;
class Keyboard;
class CTestExecuterMgr : public ITestExecuter
{
public:
    CTestExecuterMgr(const ProcessInfo& pi);
    ~CTestExecuterMgr();

    virtual void RunTest(LogerMgr& logerMgr);
    void AddTest(const std::shared_ptr<CTestExecuter>& spTest);

    // HANDLE GetTargetProcessHandle() const;

private:
    std::vector<std::shared_ptr<CTestExecuter>> m_spTests;
    const ProcessInfo& m_refProcessInfo;

    CTestExecuterMgr& operator =(const CTestExecuterMgr& rhs);
    CTestExecuterMgr& operator =(CTestExecuterMgr&& rhs);

    friend class CTestExecuter;
};


class CTestExecuter : public ITestExecuter
{
public:
    CTestExecuter(const CTestExecuterMgr& refTestExeMgr);

    void SendKeyboardString(const std::basic_string<TCHAR>& str) const;
    void SendKeyboardCode(DWORD dwVk) const;

protected:
    const CTestExecuterMgr& m_refTestExeMgr;

private:
    CTestExecuter& operator =(const CTestExecuter& rhs);
    CTestExecuter& operator =(CTestExecuter&& rhs);
};


class CStartupTestExecuter : public CTestExecuter
{
public:
    CStartupTestExecuter(const CTestExecuterMgr& refTestExeMgr);

    virtual void RunTest(LogerMgr& logerMgr);

protected:
    void ByPassWarningMsgBox(LogerMgr& logerMgr);
    void ByPassLoginDlg(LogerMgr& logerMgr);

    HWND FindWindowInRetryMode(const std::basic_string<TCHAR>& searchCondition, unsigned int retryCount = 50) const;
};


class CPageSwitchTestExecuter : public CTestExecuter
{
public:
    CPageSwitchTestExecuter(const CTestExecuterMgr& refTestExeMgr);

    virtual void RunTest(LogerMgr& logerMgr);

protected:
};

class CKPagePageDownTestExecuter : public CTestExecuter
{
public:
    CKPagePageDownTestExecuter(const CTestExecuterMgr& refTestExeMgr);

    virtual void RunTest(LogerMgr& logerMgr);
};

class CMinutePagePageDownTestExecuter : public CTestExecuter
{
public:
    CMinutePagePageDownTestExecuter(const CTestExecuterMgr& refTestExeMgr);

    virtual void RunTest(LogerMgr& logerMgr);
};

