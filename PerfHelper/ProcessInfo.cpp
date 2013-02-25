#include "StdAfx.h"

#include <cassert>
#include <stdexcept>

#include "ProcessInfo.h"
#include "TestExecuter.h"
#include "Keyboard.h"

using namespace std;

ProcessInfo::ProcessInfo(const PROCESS_INFORMATION& pi)
    : m_pi(pi)
{
}

ProcessInfo::~ProcessInfo(void)
{
}

DWORD ProcessInfo::GetTargetProcessID() const
{
    return m_pi.dwProcessId;
}

HANDLE ProcessInfo::GetTargetProcessHandle() const
{
    return m_pi.hProcess;
}

DWORD ProcessInfo::GetTargetProcessMainThreadID() const
{
    return m_pi.dwThreadId;
}

HANDLE ProcessInfo::GetTargetProcessMainThreadHandle() const
{
    return m_pi.hThread;
}

