#pragma once

#include "StdAfx.h"
#include <afxtempl.h> 

class CResource {

public:
    CResource(const CString& id) : id(id) {

    }

    CString id;
};

class CAccelerator : public CResource {

public:
    CAccelerator(const CString& id, const CString& event, bool shift, bool control) : CResource(id), event(event), shift(shift), control(control) {

    }

    CString event;
    bool shift;
    bool control;
};

class CResourceFile {

public:

    CResourceFile();

    void Clear();
    void ReadResourceIDs(const CString& szPath);
    CMap<CString, LPCTSTR, CResource*, CResource*> idMap;

private:

    struct SectionInfo {
        LPCTSTR	name;
        UINT	flags;
    };

    // Section types
    enum SectionType {
        NONE,
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
    };

    // Section flags
    enum SectionFlags {
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

    void AddResource(const CString& id, const SectionType sectionType);
    void AddResource2(const CString& id, CResource* resource);

};

