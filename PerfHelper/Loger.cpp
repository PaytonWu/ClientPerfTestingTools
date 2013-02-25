#include "StdAfx.h"

#include <cstdarg>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <locale>

#include "Loger.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// LogerMgr class.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor.

LogerMgr::LogerMgr(void)
{
}

LogerMgr::~LogerMgr(void)
{
}


///////////////////////////////////////////////////////////////////////////////
// static variables.

const basic_string<TCHAR> LogerMgr::messagePrefix(_T("[Message]: "));
const basic_string<TCHAR> LogerMgr::warningPrefix(_T("[Warning]: "));
const basic_string<TCHAR> LogerMgr::errorPrefix(_T("[Error]: "));
#ifdef _UNICODE
const string LogerMgr::messagePrefixA("[Message]: ");
const string LogerMgr::warningPrefixA("[Warning]: ");
const string LogerMgr::errorPrefixA("[Error]: ");
#endif

///////////////////////////////////////////////////////////////////////////////
// protected and private member functions.

// find the corresponding loger container by the loger type.
LogerMgr::LogerContainer& LogerMgr::_FindLogerContainerByType(LogerMgr::LOGER_TYPE eLogerType)
{
    auto logerContainerIterator = m_mapType2LogerContainer.find(eLogerType);
    bool bFound = logerContainerIterator != m_mapType2LogerContainer.end();

    if (!bFound)
    {
        auto ret = m_mapType2LogerContainer.insert(make_pair(eLogerType, LogerContainer()));
        logerContainerIterator = ret.first;
    }

    return logerContainerIterator->second;
}

LogerMgr::LogerContainer LogerMgr::_FindLogerContainerByType(LogerMgr::LOGER_TYPE eLogerType) const
{
    auto logerContainerIterator = m_mapType2LogerContainer.find(eLogerType);
    bool bFound = logerContainerIterator != m_mapType2LogerContainer.end();

    return bFound ? logerContainerIterator->second : LogerContainer();
}

// let logers in the specified container prints the string with designated prefix.
void LogerMgr::_LogStringWithPrefix(const LogerContainer& logerContainer,
                                    const basic_string<TCHAR>& message,
                                    const basic_string<TCHAR>& prefix) const
{
    for_each(logerContainer.begin(), logerContainer.end(), [&](const shared_ptr<ILoger>& spLoger) {
        spLoger->LogMessage(prefix + message);
    });
}

#ifdef _UNICODE
void LogerMgr::_LogStringWithPrefix(const LogerContainer& logerContainer,
                                    const string& message,
                                    const string& prefix) const
{
    for_each(logerContainer.begin(), logerContainer.end(), [&](const shared_ptr<ILoger>& spLoger) {
        spLoger->LogMessage(prefix + message);
    });
}
#endif  //  _UNICODE

// build the string from input format and va_list.  The result is stored in a buffer managed by smart pointer.
unique_ptr<TCHAR[]> LogerMgr::_ConstructMessageString(LPCTSTR lpszFmt, va_list argList) const
{
    int nLen = _vsctprintf(lpszFmt, argList);
    int nLenWithNull = nLen + 1;

    unique_ptr<TCHAR[]> spBuf(new TCHAR[nLenWithNull]);
    memset(spBuf.get(), 0, sizeof(TCHAR) * nLenWithNull);

    int nPrinted = _vstprintf(spBuf.get(),
#ifdef _UNICODE
        nLenWithNull,
#endif
        lpszFmt, argList);
    assert(nPrinted == nLen);

    return spBuf;
}

#ifdef _UNICODE
unique_ptr<char[]> LogerMgr::_ConstructMessageString(LPCSTR lpszFmt, va_list argList) const
{
    int nLen = _vscprintf(lpszFmt, argList);
    int nLenWithNull = nLen + 1;

    unique_ptr<char[]> spBuf(new char[nLenWithNull]);
    memset(spBuf.get(), 0, nLenWithNull);

    int nPrinted = vsprintf(spBuf.get(), lpszFmt, argList);
    assert(nPrinted == nLen);

    return spBuf;
}
#endif

// send messages to LT_MESSAGE type loger container.
void LogerMgr::_RedirectToMessageLogerContainer(const basic_string<TCHAR>& message, const basic_string<TCHAR>& prefix) const
{
    LogerContainer messageLogerCont = _FindLogerContainerByType(LT_MESSAGE);
    _LogStringWithPrefix(messageLogerCont, message, prefix);
}

#ifdef _UNICODE
void LogerMgr::_RedirectToMessageLogerContainer(const string& message, const string& prefix) const
{
    LogerContainer messageLogerCont = _FindLogerContainerByType(LT_MESSAGE);
    _LogStringWithPrefix(messageLogerCont, message, prefix);
}
#endif

///////////////////////////////////////////////////////////////////////////////
// public member functions.

// add a loger and specify the corresponding loger type.
// there are two overrides.
void LogerMgr::AddLoger(const shared_ptr<ILoger>& spLoger, LogerMgr::LOGER_TYPE eLogerType)
{
    auto& logerContainer = _FindLogerContainerByType(eLogerType);
    logerContainer.push_back(spLoger);
}

void LogerMgr::AddLoger(shared_ptr<ILoger>&& spLoger, LogerMgr::LOGER_TYPE eLogerType)
{
    auto& logerContainer = _FindLogerContainerByType(eLogerType);
    logerContainer.push_back(spLoger);
}


// log normal message to LT_MESSAGE type loger.
// two overrides.
void LogerMgr::LogMessage(LPCTSTR lpszFmt, ...) const
{
    LogerType2LogerContainerMap::const_iterator iter = m_mapType2LogerContainer.find(LT_MESSAGE);
    if (m_mapType2LogerContainer.end() == iter)
    {
        return;
    }

    va_list argList;
    va_start(argList, lpszFmt);
    unique_ptr<TCHAR[]> spBuf = _ConstructMessageString(lpszFmt, argList);
    va_end(argList);

    _LogStringWithPrefix(iter->second, spBuf.get(), messagePrefix);
}

void LogerMgr::LogMessage(const basic_string<TCHAR>& message) const
{
    LogerType2LogerContainerMap::const_iterator iter = m_mapType2LogerContainer.find(LT_MESSAGE);
    if (m_mapType2LogerContainer.end() == iter)
    {
        return;
    }

    _LogStringWithPrefix(iter->second, message, messagePrefix);
}

// log warning message to LT_WARNING and LT_MESSAGE type loger.
void LogerMgr::LogWarning(LPCTSTR lpszFmt, ...) const
{
    LogerType2LogerContainerMap::const_iterator iter = m_mapType2LogerContainer.find(LT_WARNING);
    if (m_mapType2LogerContainer.end() == iter)
    {
        return;
    }

    va_list argList;
    va_start(argList, lpszFmt);
    unique_ptr<TCHAR[]> spBuf = _ConstructMessageString(lpszFmt, argList);
    va_end(argList);

    _LogStringWithPrefix(iter->second, spBuf.get(), warningPrefix);

    _RedirectToMessageLogerContainer(spBuf.get(), warningPrefix);
}

void LogerMgr::LogWarning(const basic_string<TCHAR>& warning) const
{
    LogerType2LogerContainerMap::const_iterator iter = m_mapType2LogerContainer.find(LT_WARNING);
    if (m_mapType2LogerContainer.end() == iter)
    {
        return;
    }

    _LogStringWithPrefix(iter->second, warning, warningPrefix);

    _RedirectToMessageLogerContainer(warning, warningPrefix);
}

void LogerMgr::LogError(LPCTSTR lpszFmt, ...) const
{
    LogerType2LogerContainerMap::const_iterator iter = m_mapType2LogerContainer.find(LT_ERROR);
    if (m_mapType2LogerContainer.end() == iter)
    {
        return;
    }

    va_list argList;
    va_start(argList, lpszFmt);
    unique_ptr<TCHAR[]> spBuf = _ConstructMessageString(lpszFmt, argList);
    va_end(argList);

    _LogStringWithPrefix(iter->second, spBuf.get(), errorPrefix);

    _RedirectToMessageLogerContainer(spBuf.get(), errorPrefix);
}

void LogerMgr::LogError(const basic_string<TCHAR>& errors) const
{
    LogerType2LogerContainerMap::const_iterator iter = m_mapType2LogerContainer.find(LT_ERROR);
    if (m_mapType2LogerContainer.end() == iter)
    {
        return;
    }

    _LogStringWithPrefix(iter->second, errors, errorPrefix);

    _RedirectToMessageLogerContainer(errors, errorPrefix);
}

#ifdef _UNICODE
void LogerMgr::LogMessage(LPCSTR lpszFmt, ...) const
{
    LogerType2LogerContainerMap::const_iterator iter = m_mapType2LogerContainer.find(LT_MESSAGE);
    if (m_mapType2LogerContainer.end() == iter)
    {
        return;
    }

    va_list argList;
    va_start(argList, lpszFmt);
    unique_ptr<char[]> spBuf = _ConstructMessageString(lpszFmt, argList);
    va_end(argList);

    _LogStringWithPrefix(iter->second, spBuf.get(), messagePrefixA);
}

void LogerMgr::LogMessage(const string& message) const
{
    LogerType2LogerContainerMap::const_iterator iter = m_mapType2LogerContainer.find(LT_MESSAGE);
    if (m_mapType2LogerContainer.end() == iter)
    {
        return;
    }

    _LogStringWithPrefix(iter->second, message, messagePrefixA);
}

void LogerMgr::LogWarning(LPCSTR lpszFmt, ...) const
{
    LogerType2LogerContainerMap::const_iterator iter = m_mapType2LogerContainer.find(LT_WARNING);
    if (m_mapType2LogerContainer.end() == iter)
    {
        return;
    }

    va_list argList;
    va_start(argList, lpszFmt);
    unique_ptr<char[]> spBuf = _ConstructMessageString(lpszFmt, argList);
    va_end(argList);

    _LogStringWithPrefix(iter->second, spBuf.get(), warningPrefixA);

    _RedirectToMessageLogerContainer(spBuf.get(), warningPrefixA);
}

void LogerMgr::LogWarning(const string& warning) const
{
    LogerType2LogerContainerMap::const_iterator iter = m_mapType2LogerContainer.find(LT_WARNING);
    if (m_mapType2LogerContainer.end() == iter)
    {
        return;
    }

    _LogStringWithPrefix(iter->second, warning, warningPrefixA);

    _RedirectToMessageLogerContainer(warning, warningPrefixA);
}

void LogerMgr::LogError(LPCSTR lpszFmt, ...) const
{
    LogerType2LogerContainerMap::const_iterator iter = m_mapType2LogerContainer.find(LT_ERROR);
    if (m_mapType2LogerContainer.end() == iter)
    {
        return;
    }

    va_list argList;
    va_start(argList, lpszFmt);
    unique_ptr<char[]> spBuf = _ConstructMessageString(lpszFmt, argList);
    va_end(argList);

    _LogStringWithPrefix(iter->second, spBuf.get(), errorPrefixA);

    _RedirectToMessageLogerContainer(spBuf.get(), errorPrefixA);
}

void LogerMgr::LogError(const string& errors) const
{
    LogerType2LogerContainerMap::const_iterator iter = m_mapType2LogerContainer.find(LT_ERROR);
    if (m_mapType2LogerContainer.end() == iter)
    {
        return;
    }

    _LogStringWithPrefix(iter->second, errors, errorPrefixA);

    _RedirectToMessageLogerContainer(errors, errorPrefixA);
}
#endif


// ILoger
ILoger::~ILoger()
{
}


///////////////////////////////////////////////////////////////////////////////
// CStdOutLoger
CStdOutLoger::CStdOutLoger()
#ifdef _UNICODE
    : std_out(wcout)
#else
    : std_out(cout)
#endif
{
    std_out.imbue(locale("Chinese_China"));
}

void CStdOutLoger::LogMessage(LPCTSTR lpszFmt, ...)
{
    va_list args;
    va_start(args, lpszFmt);

    int nLen = _vsctprintf(lpszFmt, args);
    int nLenWithNull = nLen + 1;

    unique_ptr<TCHAR[]> spBuf(new TCHAR[nLenWithNull]);
    memset(spBuf.get(), 0, sizeof(TCHAR) * nLenWithNull);

    if (_vstprintf(spBuf.get(), nLenWithNull, lpszFmt, args) == nLen)
    {
        LogMessage(basic_string<TCHAR>(spBuf.get()));
    }
}

void CStdOutLoger::LogMessage(const basic_string<TCHAR>& strMsg)
{
    std_out << strMsg;
}

#ifdef _UNICODE
// multibyte char version.
void CStdOutLoger::LogMessage(LPCSTR lpszFmt, ...)
{
    va_list vaList;
    va_start(vaList, lpszFmt);

    int nLen = _vscprintf(lpszFmt, vaList);
    int nLenWithNull = nLen + 1;

    unique_ptr<char[]> spBuf(new char[nLenWithNull]);
    memset(spBuf.get(), 0, nLenWithNull);

    int nPrinted = vsprintf(spBuf.get(), lpszFmt, vaList);
    assert(nPrinted == nLen);
    LogMessage(string(spBuf.get()));
}

void CStdOutLoger::LogMessage(const string& strMsg)
{
    int nLen = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, strMsg.c_str(), -1, NULL, 0);
    unique_ptr<TCHAR[]> spBuf(new TCHAR[nLen]);
    memset(spBuf.get(), 0, sizeof(TCHAR) * nLen);

    MultiByteToWideChar(CP_ACP, MB_COMPOSITE, strMsg.c_str(), -1, spBuf.get(), nLen);

    LogMessage(spBuf.get());
}
#endif  // _UNICODE

// CFileLoger
CFileLoger::CFileLoger(const basic_string<TCHAR>& strFile)
    : m_log(strFile, ios_base::out)
{
    m_log.imbue(locale("Chinese_China"));
}

void CFileLoger::LogMessage(LPCTSTR lpszFmt, ...)
{
    va_list args;
    va_start(args, lpszFmt);
    int nLen = _vsctprintf(lpszFmt, args) + 1;

    unique_ptr<TCHAR[]> spBuf(new TCHAR[nLen]);
    memset(spBuf.get(), 0, sizeof(TCHAR) * nLen);

    if (_vstprintf(spBuf.get(), nLen, lpszFmt, args))
    {
        LogMessage(basic_string<TCHAR>(spBuf.get()));
    }
}

void CFileLoger::LogMessage(const basic_string<TCHAR>& strMsg)
{
    m_log << strMsg;
}

#ifdef _UNICODE
void CFileLoger::LogMessage(LPCSTR lpszFmt, ...)
{
    va_list args;
    va_start(args, lpszFmt);
    int nLen = _vscprintf(lpszFmt, args) + 1;

    unique_ptr<char[]> spBuf(new char[nLen]);
    memset(spBuf.get(), 0, sizeof(TCHAR) * nLen);

    int nPrinted = vsprintf(spBuf.get(), lpszFmt, args);
    assert(nPrinted == nLen);

    LogMessage(string(spBuf.get()));
}

void CFileLoger::LogMessage(const string& strMsg)
{
    int nLen = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, strMsg.c_str(), -1, NULL, 0);
    unique_ptr<TCHAR[]> spBuf(new TCHAR[nLen]);

    MultiByteToWideChar(CP_ACP, MB_COMPOSITE, strMsg.c_str(), -1, spBuf.get(), nLen);

    LogMessage(basic_string<TCHAR>(spBuf.get()));
}
#endif
