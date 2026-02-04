#include "StdAfx.h"
#include "HeaderFile.h"
#include "ResourceFile.h"


CString CDefine::GetPrefix() const {
    auto prefixIndex = name.Find('_');
    if (prefixIndex >= 0) {
        return name.Left(prefixIndex);
    }
    return "";
}

CString CDefine::GetSectionPrefix() const {
    auto prefixIndex = name.Find('_');
    if (prefixIndex >= 0) {
        auto sectionIndex = name.Find('_', prefixIndex + 1);
        if (sectionIndex >= 0) {
            return name.Mid(prefixIndex + 1, sectionIndex - prefixIndex - 1);
        }
    }
    return "";
}

// Resource ID types in standard order
enum {
    RIT_APP,
    RIT_BITMAP,
    RIT_ICON,
    RIT_DIALOG,
    RIT_MENU,
    RIT_STRING,
    RIT_STRING_ALT,
    RIT_CONTROL,
    RIT_COMMAND,
    RIT_CONTROL_ALT,
    RES_ID_TYPES = 10,
};

// Resource ID type prefixes
static const LPCTSTR szPrefix[RES_ID_TYPES] = {
    _T("IDR_"),
    _T("IDB_"),
    _T("IDI_"),
    _T("IDD_"),
    _T("IDM_"),
    _T("IDS_"),
    _T("IDP_"),
    _T("IDC_"),
    _T("ID_"),
    _T("IDW_"),
};


bool CHeaderFile::HasPrefix(const CString& name, LPCTSTR prefix)
{
    return !_tcsncmp(name, prefix, _tcslen(prefix));
}

int CHeaderFile::SortCompareFunc(const void* p1, const void* p2)
{
    const auto pDef1 = (CDefine*)p1;
    const auto pDef2 = (CDefine*)p2;
    // first ID without '_'
    if (!_tcschr(pDef1->name, '_')) {
        if (!_tcschr(pDef2->name, '_')) {
            return _tcscmp(pDef1->name, pDef2->name);
        }
        return -1;
    }
    if (!_tcschr(pDef2->name, '_')) {
        if (!_tcschr(pDef1->name, '_')) {
            return _tcscmp(pDef1->name, pDef2->name);
        }
        return 1;
    }
    // next extract IDR_MAINFRAME as first IDR_
    if (!_tcscmp(pDef1->name, _T("IDR_MAINFRAME"))) {
        return -1;
    }
    if (!_tcscmp(pDef2->name, _T("IDR_MAINFRAME"))) {
        return 1;
    }

    // next ID?_
    for (int iType = 0; iType < RES_ID_TYPES; iType++) {
        auto hasPrefix1 = HasPrefix(pDef1->name, szPrefix[iType]);
        auto hasPrefix2 = HasPrefix(pDef2->name, szPrefix[iType]);
        if (hasPrefix1 || hasPrefix2) {	// if either has prefix
            if (!hasPrefix1) {
                if (hasPrefix2)
                    return 1;
            }
            else {
                if (!hasPrefix2)
                    return -1;
            }
            return _tcscmp(pDef1->name, pDef2->name);
        }
    }
    return _tcscmp(pDef1->name, pDef2->name);
}

bool CHeaderFile::RenumberResourceHeader(LPCTSTR szSrcPath, LPCTSTR szDstPath, const CResourceFile& resourceFile)
{
    CString	sAppName;
    CArray<CDefine, CDefine&> aDef;
    int	iDef, iRes;
    int nDefs = 0;
    FILE* pfIn;
    if (_tfopen_s(&pfIn, szSrcPath, _T("rt, ccs=UNICODE"))) {
        CFileException::ThrowErrno(errno, szSrcPath);
    }
    CStdioFile fIn(pfIn);
    CString	line;
    while (fIn.ReadString(line)) {
        // get application name from resource.h file
        static const TCHAR USED_BY[] = _T("Used by ");
        int	iPos = line.Find(USED_BY);
        if (iPos >= 0) {
            iPos += _countof(USED_BY) - 1;
            sAppName = Tokenize(line, _T(" "), iPos);
            continue;
        }
        static const TCHAR DEFINE[] = _T("#define");
        iPos = line.Find(DEFINE);
        if (iPos >= 0) {
            iPos += _countof(DEFINE) - 1;
            CString	sName(Tokenize(line, _T(" "), iPos));
            if (sName.IsEmpty())
                continue;
            CString	sVal(Tokenize(line, _T(" "), iPos));
            if (sVal.IsEmpty()) {
                continue;
            }
            if (HasPrefix(sName, _T("_APS_"))) {
                // skip APS lines
                continue;
            }
            CResource* resource;
            if (resourceFile.idMap.GetCount() && !resourceFile.idMap.Lookup(sName, resource)) {
                // JAC! Do not delete because the detection of the IDs is flawed
                //  _tprintf(_T("deleted %s\n"), sName);
                //  continue;
            }
            for (iDef = 0; iDef < nDefs; iDef++) {	// find symbol name in list
                if (aDef[iDef].name == sName)
                    break;
            }
            if (iDef >= nDefs) {	// if not found
                CDefine def;
                def.name = sName;
                if (sVal.Find('x')) {
                    def.value = (int)_tcstol(sVal, NULL, 16);
                }
                else {
                    def.value = _ttoi(sVal);
                }
                aDef.Add(def);
                nDefs++;
            }
        }
    }
    fIn.Close();
    if (!nDefs) {
        // if no definitions found
        return false;
    }

    UINT	nNextSymed = NEXT_SYMED;
    UINT	nNextControl = NEXT_CONTROL;
    UINT	nNextCommand = NEXT_COMMAND;
    UINT	nNextResource = NEXT_RESOURCE;
    UINT	nSingleWordIDs = RESERVED_IDS + 1;

    // Sort define array by ID
    qsort(aDef.GetData(), nDefs, sizeof(CDefine), SortCompareFunc);

    // First renumber IDs without '_'
    for (iDef = 0; iDef < nDefs; iDef++) {
        if (aDef[iDef].GetPrefix().IsEmpty()) {
            break;
        }
        aDef[iDef].value = nSingleWordIDs;
        nSingleWordIDs++;
    }

    // Check resource start
    if (nNextResource < nSingleWordIDs) {
        nNextResource = nSingleWordIDs;
    }

    // renumber resources in first section
    for (iDef = 0; iDef < nDefs; iDef++) {
        for (iRes = RIT_APP; iRes <= RIT_MENU; iRes++)
            if (HasPrefix(aDef[iDef].name, szPrefix[iRes])) {
                aDef[iDef].value = nNextResource++;
            }
    }
    // start strings on next hundred
    nNextResource = (nNextResource / 100 + 1) * 100;
    // renumber string resources
    for (iDef = 0; iDef < nDefs; iDef++) {
        for (iRes = RIT_STRING; iRes <= RIT_STRING_ALT; iRes++) {
            if (HasPrefix(aDef[iDef].name, szPrefix[iRes]))
                aDef[iDef].value = nNextResource++;
        }
    }
    // start controls on next thousand
    if (nNextControl < nNextResource) {
        nNextControl = (nNextControl / 1000 + 1) * 1000;
    }
    // renumber controls
    for (iDef = 0; iDef < nDefs; iDef++) {
        if (HasPrefix(aDef[iDef].name, szPrefix[RIT_CONTROL]))
            aDef[iDef].value = nNextControl++;
    }
    // renumber commands
    for (iDef = 0; iDef < nDefs; iDef++) {
        if (HasPrefix(aDef[iDef].name, szPrefix[RIT_COMMAND]))
            aDef[iDef].value = nNextCommand++;
    }
    // Create new resource file
    CStdioFile	fOut(szDstPath, CFile::modeWrite | CFile::modeCreate);
    fOut.WriteString(
        _T("//{{NO_DEPENDENCIES}}\n")
        _T("// Microsoft Developer Studio generated include file.\n"));
    line.Format(_T("// Used by %s\n"), sAppName);
    fOut.WriteString(line);
    fOut.WriteString(_T("//\n"));
    CString lastPrefix;
    CString lastSectionPrefix;
    for (iDef = 0; iDef < nDefs; iDef++) {
        const CDefine& def = aDef[iDef];
        auto prefix = def.GetPrefix();
        if (!prefix.IsEmpty() && (prefix != lastPrefix)) {
            lastPrefix = prefix;
            fOut.WriteString("\n# Prefix " + lastPrefix + "\n\n");
        }
        else {
            if (lastPrefix == "ID") {
                auto sectionPrefix = def.GetSectionPrefix();
                if (!sectionPrefix.IsEmpty() && sectionPrefix != lastSectionPrefix) {
                    lastSectionPrefix = sectionPrefix;
                    fOut.WriteString("\n# Section " + lastSectionPrefix + "\n\n");
                }
            }
        }
        if (HasPrefix(def.name, szPrefix[RIT_CONTROL_ALT])) {
            line.Format(_T("#define %-31s 0x%X\n"), def.name, def.value);
        }
        else {
            line.Format(_T("#define %-31s %u\n"), def.name, def.value);
        }
        fOut.WriteString(line);
    }
    fOut.WriteString(
        _T("\n")
        _T("// Next default values for new objects\n")
        _T("//\n")
        _T("#ifdef APSTUDIO_INVOKED\n")
        _T("#ifndef APSTUDIO_READONLY_SYMBOLS\n"));
    line.Format(_T("#define %-31s %u\n"), _T("_APS_3D_CONTROLS"), 1);
    fOut.WriteString(line);
    line.Format(_T("#define %-31s %u\n"), _T("_APS_NEXT_RESOURCE_VALUE"), nNextResource);
    fOut.WriteString(line);
    line.Format(_T("#define %-31s %u\n"), _T("_APS_NEXT_COMMAND_VALUE"), nNextCommand);
    fOut.WriteString(line);
    line.Format(_T("#define %-31s %u\n"), _T("_APS_NEXT_CONTROL_VALUE"), nNextControl);
    fOut.WriteString(line);
    line.Format(_T("#define %-31s %u\n"), _T("_APS_NEXT_SYMED_VALUE"), nNextSymed);
    fOut.WriteString(line);
    fOut.WriteString(
        _T("#endif\n")
        _T("#endif\n"));
    return true;
}