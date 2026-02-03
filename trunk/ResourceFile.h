#pragma once

#include "StdAfx.h"
#include <afxtempl.h> 

class CResourceFile {

public:

    CResourceFile();

    void ReadResourceIDs(const CString& szPath);
    CMap<CString, LPCTSTR, int, int> idMap;

private:

    struct SECTION_INFO {
        LPCTSTR	name;
        UINT	flags;
    };

    // resource sections
    enum {
        RS_ACCELERATORS,
        RS_BITMAP,
        RS_CURSOR,
        RS_DESIGNINFO,
        RS_DIALOG,
        RS_DIALOGEX,
        RS_DLGINIT,
        RS_ICON,
        RS_MENU,
        RS_STRINGTABLE,
        RS_TOOLBAR,
        RS_VERSIONINFO,
        SECTIONS
    };
    // section flags
    enum {
        SF_NONE = 0x00,
        SF_SINGLE = 0x01,		// single-line (no begin/end pair)
        SF_NAMED = 0x02,		// section identifier is meaningful
    };

    // parse states
    enum class ParseState {
        FIND_SECTION,		// searching for section header
        FIND_BEGIN,			// searching for section begin tag
        IN_SECTION,			// processing section contents
    };

    void AddIDStr(const CString& sID);
};



CResourceFile::CResourceFile() {

}

void CResourceFile::AddIDStr(const CString& sID)
{
    idMap.SetAt(sID, 0);
}

void CResourceFile::ReadResourceIDs(const CString& filePath)
{

    static const SECTION_INFO SecInfo[SECTIONS] = {
        {_T("ACCELERATORS"),	SF_NAMED},
        {_T("BITMAP"),			SF_SINGLE},
        {_T("CURSOR"),			SF_SINGLE},
        {_T("DESIGNINFO"),		SF_NONE},
        {_T("DIALOG"),			SF_NAMED},
        {_T("DIALOGEX"),		SF_NAMED},
        {_T("DLGINIT"),			SF_NONE},
        {_T("ICON"),			SF_SINGLE},
        {_T("MENU"),			SF_NAMED},
        {_T("STRINGTABLE"),		SF_NONE},
        {_T("TOOLBAR"),			SF_NAMED},
        {_T("VERSIONINFO"),		SF_NONE},
    };

    FILE* inputFilePtr;
    if (_tfopen_s(&inputFilePtr, filePath, _T("rt, ccs=UNICODE"))) {
        CFileException::ThrowErrno(errno, filePath);
    }
    CStdioFile inputFile(inputFilePtr);
    CString	line;
    ParseState parseState = ParseState::FIND_SECTION;
    int	currentSectionType = 0;
    while (inputFile.ReadString(line)) {
        switch (parseState) {
        case ParseState::FIND_BEGIN:
            if (line == _T("BEGIN")) {
                // if start of section found
                parseState = ParseState::IN_SECTION;
            }
            break;
        case ParseState::IN_SECTION:
            if (line == _T("END")) {	// if end of section reached
                parseState = ParseState::FIND_SECTION;
                break;
            }
            switch (currentSectionType) {
            case RS_ACCELERATORS:
            {
                int	iPos = 0;
                CString	sChar(Tokenize(line, _T(" "), iPos));
                if (sChar.IsEmpty()) {
                    break;
                }
                CString	sID(Tokenize(line, _T(" ,"), iPos));
                if (sID.IsEmpty()) {
                    break;
                }
                AddIDStr(sID);
            }
            break;
            case RS_DIALOG:
            case RS_DIALOGEX:
            {
                static const LPCTSTR szIDFirst[] = {
                    _T("EDITTEXT"),
                    _T("BEDIT"),
                    _T("HEDIT"),
                    _T("IEDIT"),
                    _T("LISTBOX"),
                    _T("COMBOBOX"),
                    _T("SCROLLBAR"),
                };
                // TODO This const is total nonsense
                const int nCtrlWidth = 20;
                CString	sCtrl(line.Left(nCtrlWidth));
                CString	sParam(line.Mid(nCtrlWidth));
                sCtrl.TrimRight();
                sCtrl.TrimLeft();
                if (sCtrl.IsEmpty()) {
                    break;
                }
                int	nIDFirsts = _countof(szIDFirst);
                int	iItem;
                for (iItem = 0; iItem < nIDFirsts; iItem++) {
                    if (sCtrl == szIDFirst[iItem])
                        break;
                }
                if (iItem < nIDFirsts) {	// if ID is first parameter
                    int	iPos = 0;
                    CString	sID(Tokenize(sParam, _T(" ,"), iPos));
                    if (sID.IsEmpty()) {
                        break;
                    }
                    AddIDStr(sID);
                }
                else {	// ID is second parameter; first is caption
                    int	iPos = 0;
                    if (sParam[0] == '"') {	// if first parameter is quoted
                        if (sParam.Left(3) == _T("\"\",")) {	// if empty string
                            sParam = sParam.Mid(3);
                        }
                        else {	// non-empty string; parse caption
                            CString	sText(Tokenize(sParam, _T("\""), iPos));
                            if (sText.IsEmpty())
                                break;
                        }
                    }
                    else {	// first parameter not quoted; happens with icons
                        CString	sText(Tokenize(sParam, _T(","), iPos));
                        if (sText.IsEmpty())
                            break;
                    }
                    CString	sID(Tokenize(sParam, _T(",\""), iPos));
                    if (sID.IsEmpty()) {	// if identifier not found
                        inputFile.ReadString(sParam);	// try next line
                        sParam.TrimLeft();
                        iPos = 0;
                        sID = Tokenize(sParam, _T(" ,"), iPos);
                        if (sID.IsEmpty())
                            break;
                    }
                    if (sID == _T("IDC_STATIC")) {
                        // ignore static controls
                        break;
                    }
                    AddIDStr(sID);
                }
            }
            break;
            case RS_MENU:
            {
                int	iPos = 0;
                CString	sItem(Tokenize(line, _T(" "), iPos));
                if (sItem != _T("MENUITEM")) {
                    break;
                }
                CString	sText(Tokenize(line, _T("\""), iPos));
                if (sText.IsEmpty()) {
                }
                break;
                CString	sID(Tokenize(line, _T(", "), iPos));
                if (sID.IsEmpty()) {
                    break;
                }
                AddIDStr(sID);
            }
            break;
            case RS_STRINGTABLE:
            {
                int	iPos = 0;
                CString	sID(Tokenize(line, _T(" "), iPos));
                if (sID.IsEmpty() || sID[0] == '"') {
                    break;
                }
                AddIDStr(sID);
            }
            break;
            case RS_TOOLBAR:
            {
                int	iPos = 0;
                CString	sItem(Tokenize(line, _T(" "), iPos));
                if (sItem.IsEmpty()) {
                    break;
                }
                CString	sID(Tokenize(line, _T(" "), iPos));
                if (sID.IsEmpty()) {
                    break;
                }
                AddIDStr(sID);
            }
            break;
            }
            break;
        default:	// searching for section header
            if (line.Left(2) == _T("//")) {
                // if comment line
                continue;
            }
            int	iPos = 0;
            CString	sID(Tokenize(line, _T(" "), iPos));
            if (sID.IsEmpty()) {
                continue;
            }
            if (sID == _T("STRINGTABLE")) {
                // if string table
                currentSectionType = RS_STRINGTABLE;
                parseState = ParseState::FIND_BEGIN;	// find start of section
                continue;
            }
            CString	sTag(Tokenize(line, _T(" "), iPos));
            if (sTag.IsEmpty()) {
                continue;
            }
            int	sectionType;
            for (sectionType = 0; sectionType < SECTIONS; sectionType++) {
                if (sTag == SecInfo[sectionType].name) {
                    break;
                }
            }
            if (sectionType >= SECTIONS) {
                // if unknown section
                continue;
            }
            if (SecInfo[sectionType].flags & SF_SINGLE) {	// if single line
                AddIDStr(sID);
            }
            else {	// proper section with begin/end pair
                if (SecInfo[sectionType].flags & SF_NAMED) {
                    AddIDStr(sID);
                }
                currentSectionType = sectionType;
                parseState = ParseState::FIND_BEGIN;	// find start of section
            }
        }
    }
}