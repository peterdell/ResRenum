#pragma once

#include "StdAfx.h"
#include <afxtempl.h> 

class CResourceFile;

class CDefine {
public:
    CString	name;
    UINT	value;

    CString GetPrefix() const;
    CString GetSectionPrefix() const;
};


class CHeaderFile {

public:

    bool RenumberResourceHeader(LPCTSTR szSrcPath, LPCTSTR szDstPath, const CResourceFile& resourceFile);

private:
    enum {
        NEXT_SYMED = 101,
        NEXT_CONTROL = 1001,
        NEXT_COMMAND = 32771,
        NEXT_RESOURCE = 101,
        RESERVED_IDS = IDHELP,
    };

    static bool HasPrefix(const CString& name, LPCTSTR prefix);

    static int SortCompareFunc(const void* p1, const void* p2);


};