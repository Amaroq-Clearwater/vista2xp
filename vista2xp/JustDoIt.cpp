// JustDoIt.cpp
// This file is public domain software.
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>.
#include "targetver.h"
#include "ExeImage.hpp"
#include <shlwapi.h>
#include <strsafe.h>
#include "resource.h"

extern "C"
HRESULT JustDoIt(HWND hwnd, LPCTSTR pszV2XKRE32, LPCTSTR pszFile)
{
    LPCTSTR pszTitle = PathFindFileName(pszFile);
    if (lstrcmpi(pszTitle, TEXT("v2xker32.dll")) == 0)
    {
        return S_FALSE;
    }

    TCHAR szText[128];
    codereverse::ExeImage image;
    if (!image.load(pszFile))
    {
        LoadString(NULL, IDS_CANTLOAD, szText, ARRAYSIZE(szText));
        MessageBox(hwnd, szText, NULL, MB_ICONERROR);
        return E_FAIL;
    }

    if (image.is_64bit())
    {
        LoadString(NULL, IDS_NOTSUP64BIT, szText, ARRAYSIZE(szText));
        MessageBox(hwnd, szText, TEXT("Warning"), MB_ICONWARNING);
        return S_FALSE;
    }

    std::vector<const char *> names;
    if (!image.get_import_dll_names(names))
    {
        return S_OK;
    }

    bool found = false;
    for (DWORD i = 0; i < names.size(); ++i)
    {
        if (lstrcmpiA(names[i], "kernel32.dll") == 0)
        {
            found = true;
            StringCbCopyA(const_cast<char *>(names[i]), 13, "v2xker32.dll");
        }
        else if (lstrcmpiA(names[i], "kernel32") == 0)
        {
            found = true;
            StringCbCopyA(const_cast<char *>(names[i]), 9, "v2xker32");
        }
    }

    TCHAR szPath[MAX_PATH];
    StringCbCopy(szPath, sizeof(szPath), pszFile);
    LPTSTR pch = PathFindFileName(szPath);
    if (pch == NULL)
        return E_FAIL;

    if (found)
    {
        // cut off file title
        *pch = 0;

        // create backup
        PathAppend(szPath, TEXT("Vista2XP-Backup"));
        CreateDirectory(szPath, NULL);
        PathAppend(szPath, pszTitle);
        CopyFile(pszFile, szPath, TRUE);

        if (!image.do_reverse_map() || !image.save(pszFile))
        {
            LoadString(NULL, IDS_CANTWRITE, szText, ARRAYSIZE(szText));
            MessageBox(hwnd, szText, NULL, MB_ICONERROR);
            return E_FAIL;
        }
    }

    // cut off file title
    *pch = 0;

    PathAppend(szPath, TEXT("v2xker32.dll"));
    if (PathFileExists(szPath) || CopyFile(pszV2XKRE32, szPath, FALSE))
        return S_OK;

    return S_FALSE;
}
