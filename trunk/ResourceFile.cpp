#include "StdAfx.h"
#include "ResourceFile.h"


void CResourceFile::AddResource(const CString& id, const SectionType sectionType)
{
    idMap.SetAt(id, nullptr);
}

void CResourceFile::AddResource2(const CString& id, CResource* resource)
{
    idMap.SetAt(id, resource);
}

void CResourceFile::Clear() {
    auto entry = idMap.PGetFirstAssoc();
    while (entry != nullptr)
    {
        if (entry->value) {
            delete entry->value;
        }
        entry = idMap.PGetNextAssoc(entry);
    }
    idMap.RemoveAll();
}


CResourceFile::CResourceFile() {

}


void CResourceFile::ReadResourceIDs(const CString& filePath)
{

    static const SectionInfo SecInfo[] = {
        {_T("NONE"),	        SF_NONE},
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
    auto currentSectionType = SectionType::NONE;
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
                CString	event(Tokenize(line, _T(" "), iPos));
                if (event.IsEmpty()) {
                    break;
                }
                CString	id(Tokenize(line, _T(" ,"), iPos));
                if (id.IsEmpty()) {
                    break;
                }

                auto shift = false;
                auto control = false;
                while (iPos < line.GetLength()) {
                    CString	option(Tokenize(line, _T(" ,"), iPos));
                    if (option == "SHIFT") {
                        shift = true;
                    }
                    if (option == "CONTROL") {
                        control = true;
                    }
                }
                AddResource2(id, new CAccelerator(id, event, shift, control));
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
                    AddResource(sID, currentSectionType);
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
                    AddResource(sID, currentSectionType);
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
                AddResource(sID, currentSectionType);
            }
            break;
            case RS_STRINGTABLE:
            {
                int	iPos = 0;
                CString	sID(Tokenize(line, _T(" "), iPos));
                if (sID.IsEmpty() || sID[0] == '"') {
                    break;
                }
                AddResource(sID, currentSectionType);
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
                AddResource(sID, currentSectionType);
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
            int sectionType;
            bool found = false;
            for (sectionType = (int)RS_ACCELERATORS; sectionType <= (int)RS_VERSIONINFO; sectionType++) {
                if (sTag == SecInfo[sectionType].name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                // if unknown section
                continue;
            }
            if (SecInfo[sectionType].flags & SF_SINGLE) {	// if single line
                AddResource(sID, currentSectionType);
            }
            else {	// proper section with begin/end pair
                if (SecInfo[sectionType].flags & SF_NAMED) {
                    AddResource(sID, currentSectionType);
                }
                currentSectionType = (SectionType)sectionType;
                parseState = ParseState::FIND_BEGIN;	// find start of section
            }
        }
    }
}