#pragma once

#include <string>
#include <memory>
#include <ostream>
#include <fstream>
#include <map>
#include <vector>
#include <cstdarg>

class ILoger;
class LogerMgr
{
public:
    enum LOGER_TYPE { LT_MESSAGE, LT_WARNING, LT_ERROR };

private:
    typedef std::vector<std::shared_ptr<ILoger>> LogerContainer;
    typedef std::map<LOGER_TYPE, LogerContainer> LogerType2LogerContainerMap;

public:
    LogerMgr(void);
    virtual ~LogerMgr(void);

    void AddLoger(const std::shared_ptr<ILoger>& spLoger, LOGER_TYPE eLogerType);
    void AddLoger(std::shared_ptr<ILoger>&& spLoger, LOGER_TYPE eLogerType);

    void LogMessage(LPCTSTR lpszFmt, ...) const;
    void LogMessage(const std::basic_string<TCHAR>& message) const;

    void LogWarning(LPCTSTR lpszFmt, ...) const;
    void LogWarning(const std::basic_string<TCHAR>& warning) const;

    void LogError(LPCTSTR lpszFmt, ...) const;
    void LogError(const std::basic_string<TCHAR>& errors) const;

#ifdef _UNICODE
    void LogMessage(LPCSTR lpszFmt, ...) const;
    void LogMessage(const std::string& message) const;

    void LogWarning(LPCSTR lpszFmt, ...) const;
    void LogWarning(const std::string& warning) const;

    void LogError(LPCSTR lpszFmt, ...) const;
    void LogError(const std::string& errors) const;
#endif

protected:
    LogerContainer& _FindLogerContainerByType(LOGER_TYPE eLogerType);
    LogerContainer _FindLogerContainerByType(LOGER_TYPE eLogerType) const;

    void _LogStringWithPrefix(const LogerContainer& logerContainer, const std::basic_string<TCHAR>& strMessage, const std::basic_string<TCHAR>& prefix) const;
    std::unique_ptr<TCHAR[]> _ConstructMessageString(LPCTSTR lpszFmt, std::va_list argList) const;
    void _RedirectToMessageLogerContainer(const std::basic_string<TCHAR>& message, const std::basic_string<TCHAR>& prefix) const;
#ifdef _UNICODE
    void _LogStringWithPrefix(const LogerContainer& logerContainer, const std::string& strMessage, const std::string& prefix) const;
    std::unique_ptr<char[]> _ConstructMessageString(LPCSTR lpszFmt, std::va_list argList) const;
    void _RedirectToMessageLogerContainer(const std::string& message, const std::string& prefix) const;
#endif  //  _UNICODE

private:
    LogerType2LogerContainerMap m_mapType2LogerContainer;

    static const std::basic_string<TCHAR> messagePrefix;
    static const std::basic_string<TCHAR> warningPrefix;
    static const std::basic_string<TCHAR> errorPrefix;
#ifdef _UNICODE
    static const std::string messagePrefixA;
    static const std::string warningPrefixA;
    static const std::string errorPrefixA;
#endif  // _UNICODE
};

class ILoger
{
public:
    virtual ~ILoger(void);

public:
    virtual void LogMessage(LPCTSTR lpszFmt, ...) = 0;
    virtual void LogMessage(const std::basic_string<TCHAR>& strMsg) = 0;

#ifdef _UNICODE
    virtual void LogMessage(LPCSTR lpszFmt, ...) = 0;
    virtual void LogMessage(const std::string& strMsg) = 0;
#endif
};

class CStdOutLoger : public ILoger
{
public:
    CStdOutLoger();

    virtual void LogMessage(LPCTSTR lpszFmt, ...);
    virtual void LogMessage(const std::basic_string<TCHAR>& strMsg);

#ifdef _UNICODE
    virtual void LogMessage(LPCSTR lpszFmt, ...);
    virtual void LogMessage(const std::string& strMsg);
#endif

private:
    std::basic_ostream<TCHAR>& std_out;

    // used to deprecate warning...
    CStdOutLoger(const CStdOutLoger&);
    CStdOutLoger& operator =(const CStdOutLoger&);
};

class CFileLoger : public ILoger
{
public:
    CFileLoger(const std::basic_string<TCHAR>& strFile);

    virtual void LogMessage(LPCTSTR lpszFmt, ...);
    virtual void LogMessage(const std::basic_string<TCHAR>& strMsg);

#ifdef _UNICODE
    virtual void LogMessage(LPCSTR lpszFmt, ...);
    virtual void LogMessage(const std::string& strMsg);
#endif

private:
    std::basic_fstream<TCHAR> m_log;
};
