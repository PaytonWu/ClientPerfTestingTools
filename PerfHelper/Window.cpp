#include "StdAfx.h"

// windows headers
#include <Windows.h>

// c/c++ headers
#include <cassert>
#include <algorithm>
#include <sstream>
#include <vector>
#include <regex>
#include <iostream>
#include <memory>

#include "Window.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// SearchCriteria
SearchCriterion::SearchCriterion(const SearchCriterion& rhs)
    : className(rhs.className), windowTitle(rhs.windowTitle)
{
}

SearchCriterion::SearchCriterion(SearchCriterion&& rhs)
    : className(std::move(rhs.className))
    , windowTitle(std::move(rhs.windowTitle))
{
}

///////////////////////////////////////////////////////////////////////////////
// CWndSearchAssistant
//
HWND CWndSearchAssistant::DoSearch(const basic_string<TCHAR>& strSearchString, HWND hParent, Enum2Type<TEXT_MODE>)
{
    vector<SearchCriterion> vSc = GenerateSearchCriteriaFromSearchString(strSearchString);
    return SearchWindow<TEXT_MODE>(hParent, vSc);
}

HWND CWndSearchAssistant::DoSearch(const basic_string<TCHAR>& strSearchString, HWND hParent, Enum2Type<REGEX_MODE>)
{
    vector<SearchCriterion> vSc = GenerateSearchCriteriaFromSearchString(strSearchString);
    return SearchWindow<REGEX_MODE>(hParent, vSc);
}

vector<basic_string<TCHAR>> CWndSearchAssistant::Split(const basic_string<TCHAR>& str, TCHAR chDelimiter)
{
    basic_istringstream<TCHAR> inputstream(str);
    basic_string<TCHAR> token;
    vector<basic_string<TCHAR>> vTokens;

    while (std::getline(inputstream, token, chDelimiter))
    {
        vTokens.push_back(token);
    }

    return vTokens;
}

vector<SearchCriterion> CWndSearchAssistant::GenerateSearchCriteriaFromSearchString(const basic_string<TCHAR>& strSearchString)
{
    basic_istringstream<TCHAR> iputstream(strSearchString);
    vector<basic_string<TCHAR>>&& tokens = Split(strSearchString, _T(';'));

    vector<SearchCriterion> vSearchCriteria;

    for_each(tokens.begin(), tokens.end(), [&vSearchCriteria](const basic_string<TCHAR>& token)
    {
        SearchCriterion sc;
        vector<basic_string<TCHAR>>&& vCriteria = Split(token, _T(','));

        static const basic_regex<TCHAR> regexClass(_T("\\s*class\\s*=\\s*'(.+)'\\s*"), basic_regex<TCHAR>::icase);
        static const basic_regex<TCHAR> regexTitle(_T("\\s*title\\s*=\\s*'(.+)'\\s*"), basic_regex<TCHAR>::icase);

        for_each(vCriteria.begin(), vCriteria.end(), [&sc](const basic_string<TCHAR>& prop)
        {
            match_results<basic_string<TCHAR>::const_iterator> propValue; 
            if (regex_search(prop, propValue, regexClass))
            {
                sc.className = propValue[1];
            }
            else if (regex_search(prop, propValue, regexTitle))
            {
                sc.windowTitle = propValue[1];
            }
        });

        if (!sc.className.empty() || !sc.windowTitle.empty())
        {
            vSearchCriteria.push_back(sc);
        }
    });

    return vSearchCriteria;
}

// 查找窗口的相关函数
HWND CWndSearchAssistant::DoSearchWindow(HWND hParent, const vector<SearchCriterion>& vSc, Enum2Type<TEXT_MODE>)
{
    for (auto i = vSc.begin(); i != vSc.end(); ++i)
    {
        hParent = FindWindowEx(hParent, NULL, i->className.c_str(), i->windowTitle.c_str());
    }

    return hParent;
}

struct enumParam
{
    enumParam(HWND hWnd, const SearchCriterion& refSc) : hChild(hWnd), sc(refSc) {}
    HWND hChild;
    const SearchCriterion& sc;

private:
    enumParam& operator =(const enumParam& rhs);
    enumParam& operator =(enumParam&& rhs);
};

HWND CWndSearchAssistant::DoSearchWindow(HWND hParent, const vector<SearchCriterion>& vSc, Enum2Type<REGEX_MODE>)
{
    HWND hRet = hParent;
    for_each(vSc.begin(), vSc.end(), [&hRet](const SearchCriterion& sc) {
        enumParam lParam(NULL, sc);

        EnumChildWindows(hRet, EnumChildWindowProc, LPARAM(&lParam));
        hRet = lParam.hChild;

        if (!IsWindow(hRet))
        {
            return;
        }
    });

    return hRet;
}

// 判断当前子窗口是否符合传入的衡量标准（从lParam里解析出来）
// 如果符合，返回FALSE，退出EnumChildWindows。
BOOL CWndSearchAssistant::EnumChildWindowProc(HWND hChild, LPARAM lParam)
{
    enumParam* plParam = (enumParam*)lParam;
    assert(NULL != plParam);

    bool bClassMatched = plParam->sc.className.empty();
    bool bTitleMatched = plParam->sc.windowTitle.empty();

    try
    {
        // try to match class.
        if (!bClassMatched)
        {
            int nMaxCount = MAX_PATH;
            unique_ptr<TCHAR[]> spClassNameBuf(new TCHAR[nMaxCount]);

            int nCopiedChars = GetClassName(hChild, spClassNameBuf.get(), nMaxCount);
            while (nCopiedChars == nMaxCount - 1)
            {
                nMaxCount *= 2;
                spClassNameBuf.reset(new TCHAR[nMaxCount]);
                nCopiedChars = GetClassName(hChild, spClassNameBuf.get(), nMaxCount);
            }

            if (0 != nCopiedChars)
            {
                const basic_regex<TCHAR> regexClass(plParam->sc.className, basic_regex<TCHAR>::icase);
                if (regex_search(spClassNameBuf.get(), regexClass))
                {
                    bClassMatched = true;
                }
            }
        }

        // try to match title.
        if (!bTitleMatched)
        {
            int nMaxCount = MAX_PATH;
            unique_ptr<TCHAR[]> spTitleBuf(new TCHAR[nMaxCount]);

            int nCopiedChars = SendMessage(hChild, WM_GETTEXT, static_cast<WPARAM>(nMaxCount), LPARAM(spTitleBuf.get()));
            while (nCopiedChars == nMaxCount - 1)
            {
                nMaxCount *= 2;
                spTitleBuf.reset(new TCHAR[nMaxCount]);

                nCopiedChars = SendMessage(hChild, WM_GETTEXT, static_cast<WPARAM>(nMaxCount), LPARAM(spTitleBuf.get()));
            }

            const basic_regex<TCHAR> regexTitle(plParam->sc.windowTitle, basic_regex<TCHAR>::icase);
            if (regex_search(spTitleBuf.get(), regexTitle))
            {
                bTitleMatched = true;
            }
        }
    }
    catch (bad_alloc&)
    {
        return TRUE;
    }

    BOOL bRet = !(bTitleMatched && bClassMatched);
    if (!bRet)
    {
        plParam->hChild = hChild;
    }

    return bRet;
}
