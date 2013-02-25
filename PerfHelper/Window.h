#pragma once

#include <map>
#include <string>
#include <regex>

struct SearchCriterion
{
    SearchCriterion() : className(), windowTitle() {}
    SearchCriterion(const SearchCriterion& rhs);
    SearchCriterion(SearchCriterion&& rhs);

    std::basic_string<TCHAR> className;
    std::basic_string<TCHAR> windowTitle;
};

class CWndSearchAssistant
{
public:
    enum SearchMode
    {
        TEXT_MODE,      // 纯文本搜索模式，使用FindWindowEx
        REGEX_MODE,     // 正则表达式模式，在EnumWindow中做模式匹配
    };

    // 一个简单的帮助类，利用模板的静态分支选择
private:
    template <SearchMode SM>
    struct Enum2Type
    {
        enum { value = SM };
    };

public:
    // search string format:
    //  class='className',title='windowTitle';class='className'
    template <SearchMode SM>
    static HWND Search(const std::basic_string<TCHAR>& strSearchString, HWND hParent = NULL)
    {
        return DoSearch(strSearchString, hParent, Enum2Type<SM>());
    }

protected:
    // 针对不同查找模式的Search实现
    static HWND DoSearch(const std::basic_string<TCHAR>& strSearchString, HWND hParent, Enum2Type<TEXT_MODE>);
    static HWND DoSearch(const std::basic_string<TCHAR>& strSearchString, HWND hParent, Enum2Type<REGEX_MODE>);

    // 拆解分析查找字串的帮助函数
    static std::vector<std::basic_string<TCHAR>> Split(const std::basic_string<TCHAR>& str, TCHAR chDelimiter);
    static std::vector<SearchCriterion> GenerateSearchCriteriaFromSearchString(const std::basic_string<TCHAR>& str);

    // 模板型的查找函数以及针对特定查找模式的SearchWindow实现
    template <SearchMode SM>
    static HWND SearchWindow(HWND hParent, const std::vector<SearchCriterion>& vSc)
    {
        return DoSearchWindow(hParent, vSc, Enum2Type<SM>());
    }

    static HWND DoSearchWindow(HWND hParent, const std::vector<SearchCriterion>& vSc, Enum2Type<TEXT_MODE>);
    static HWND DoSearchWindow(HWND hParent, const std::vector<SearchCriterion>& vSc, Enum2Type<REGEX_MODE>);

    // callback function for EnumChildWindows
    static BOOL CALLBACK EnumChildWindowProc(HWND hChild, LPARAM lParam);
};


