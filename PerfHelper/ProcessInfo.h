// ProcessInfo保存进程信息

#pragma once

#include <memory>

class ProcessInfo
{
public:
    ProcessInfo(const PROCESS_INFORMATION& pi);
    ~ProcessInfo(void);

    DWORD  GetTargetProcessID() const;
    HANDLE GetTargetProcessHandle() const;
    DWORD  GetTargetProcessMainThreadID() const;
    HANDLE GetTargetProcessMainThreadHandle() const;

private:
    PROCESS_INFORMATION m_pi;

    ProcessInfo& operator =(const ProcessInfo& rhs);
    ProcessInfo& operator =(ProcessInfo&& rhs);
};

