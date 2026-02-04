// Copyleft 2001 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda

        revision history:
        rev		date	comments
        00      16apr01 initial version
        01      30sep04 sort resource names
        02      21oct16	refactor to use CString and support Unicode

        renumber resource IDs in an MFC project

*/

// ResRenum.cpp : Defines the entry point for the console application.
//

#include "StdAfx.h"
#include "ResRenum.h"
#include "ResourceFile.h"
#include "HeaderFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;




int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    UNREFERENCED_PARAMETER(envp);
    int nRetCode = 0;
    // initialize MFC and print and error on failure
    if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) {
        printf("Fatal Error: MFC initialization failed\n");
        nRetCode = 1;
    }
    else {
        if (argc > 1) {
            TRY{
                CResourceFile resourceFile;
                if (argc > 2) {
                    resourceFile.ReadResourceIDs(argv[2]);
                }
                CHeaderFile headerFile;
                headerFile.RenumberResourceHeader(argv[1], CString(argv[1]) + ".new", resourceFile);
            }
                CATCH(CException, e) {
                TCHAR	szMsg[MAX_PATH];
                e->GetErrorMessage(szMsg, _countof(szMsg));
                _tprintf(_T("%s\n"), szMsg);
                nRetCode = 1;
            }
            END_CATCH
        }
        else {
            _tprintf(_T("not enough arguments\n"));
            nRetCode = 1;
        }
    }
    return nRetCode;
}
