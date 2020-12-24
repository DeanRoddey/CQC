//
// FILE NAME: CQCGKit_ClassManager.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/25/2003
//
// COPYRIGHT: Charmed Quark Systems, Ltd @ 2020
//
//  This software is copyrighted by 'Charmed Quark Systems, Ltd' and 
//  the author (Dean Roddey.) It is licensed under the MIT Open Source 
//  license:
//
//  https://opensource.org/licenses/MIT
//
// DESCRIPTION:
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIGKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCInterClassMgr, TObject)



// ---------------------------------------------------------------------------
//  CLASS: TCQCInterClassMgr
// PREFIX: mecm
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCInterClassMgr: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCInterClassMgr::TCQCInterClassMgr(       TWindow* const          pwndOwner
                                    , const TString&                strSelDlgTitle
                                    , const TCQCUserCtx&            cuctxToUse
                                    , const tCIDLib::TBoolean       bReadOnly
                                    ,       MRemBrwsBrwsCB* const   prbbcApp) :
    m_bReadOnly(bReadOnly)
    , m_cuctxToUse(cuctxToUse)
    , m_prbbcApp(prbbcApp)
    , m_pwndOwner(pwndOwner)
    , m_strSelDlgTitle(strSelDlgTitle)
{
}

TCQCInterClassMgr::~TCQCInterClassMgr()
{
}


// ---------------------------------------------------------------------------
//  TCQCInterClassMgr: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCInterClassMgr::Shutdown()
{
}


// ---------------------------------------------------------------------------
//  TCQCInterClassMgr: Protected, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCInterClassMgr::bCheckIfExists(const TString& strToCheck)
{
    // Convert the class path to a relative data server path
    TString strActual;
    facCQCRemBrws().CMLClassPathToRelPath(strToCheck, strActual);

    // Try to write this guy back
    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
    TDataSrvClient dsclCheck;
    return dsclCheck.bFileExists(strActual, tCQCRemBrws::EDTypes::Macro);
}


tCIDLib::TVoid TCQCInterClassMgr::DoStore(  const   TString&    strClassPath
                                            , const TString&    strText)
{
    // Convert the class path to a relative data server path
    TString strRelPath;
    facCQCRemBrws().CMLClassPathToRelPath(strClassPath, strRelPath);

    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
    TDataSrvClient dsclStore;

    // First make sure the parent scope exists
    TString strScope(strRelPath);
    facCQCRemBrws().bRemoveLastPathItem(strScope);
    dsclStore.MakePath(strScope, tCQCRemBrws::EDTypes::Macro, m_cuctxToUse.sectUser());

    // And then write the file
    tCIDLib::TCard4         c4SerialNum = 0;
    tCIDLib::TEncodedTime   enctLastChange;
    tCIDLib::TKVPFList      colMeta;
    dsclStore.WriteMacro
    (
        strRelPath
        , c4SerialNum
        , enctLastChange
        , strText
        , kCQCRemBrws::c4Flag_OverwriteOK
        , colMeta
        , m_cuctxToUse.sectUser()
    );
}


tCIDLib::TVoid TCQCInterClassMgr::DoUndoWriteMode(const TString&)
{
    //
    //  We really don't have any thing to do here at this time, since the
    //  macro respository doesn't support check out semantics.
    //
}


//
//  We only allow the selection of existing files, since our tree browser dialog only
//  supports that. If they want a new file, they have to create it separately then open
//  from within the IDE for editing.
//
tCIDLib::EOpenRes
TCQCInterClassMgr::eDoSelect(       TString&                strToFill
                            , const tCIDMacroEng::EResModes    eMode)
{
    //
    //  Set up the file selection flags. If they are calling us in read/write mode
    //  then we set the edit flag, else not. That will limit them to the /User area
    //  unless system editing is enabled.
    //
    tCQCTreeBrws::EFSelFlags eFlags = tCQCTreeBrws::EFSelFlags::SelectItems;
    if (eMode == tCIDMacroEng::EResModes::ReadWrite)
    {
        eFlags = tCIDLib::eOREnumBits<tCQCTreeBrws::EFSelFlags>
        (
            eFlags, tCQCTreeBrws::EFSelFlags::Edit
        );
    }

    //
    //  If we are set up for non-read only mode, then set that flag. That will prevent
    //  them from getting a popup menu to make changes to the tree.
    //
    if (m_bReadOnly)
    {
        eFlags = tCIDLib::eOREnumBits<tCQCTreeBrws::EFSelFlags>
        (
            eFlags, tCQCTreeBrws::EFSelFlags::ReadOnly
        );
    }

    TString strSel;
    tCIDLib::TBoolean bRes = facCQCTreeBrws().bSelectFile
    (
        *m_pwndOwner
        , m_strSelDlgTitle
        , tCQCRemBrws::EDTypes::Macro
        , m_strLastClassPath
        , m_cuctxToUse
        , eFlags
        , strSel
        , m_prbbcApp
    );

    if (bRes)
    {
        // Make this path the new last path
        m_strLastClassPath = strSel;
        facCQCRemBrws().bRemoveLastPathItem(m_strLastClassPath);

        // Convert the relative data server style path back to a CML class path
        facCQCRemBrws().RelPathToCMLClassPath(strSel, strToFill);
        return tCIDLib::EOpenRes::ExistingFile;
    }
    return tCIDLib::EOpenRes::Cancel;
}


//
//  Note that we ignore the mode for now, since the macro repository does not
//  have checkout semantics.
//
TTextInStream*
TCQCInterClassMgr::pstrmDoLoad(const TString& strClassPath, const tCIDMacroEng::EResModes)
{
    //
    //  Try to load this guy up. We'll need a string to use. If it works,
    //  we give the string to the stream we return. Put a janitor on it for
    //  the meantime.
    //
    TString* pstrLoad = new TString(32 * 1024UL);
    TJanitor<TString> janLoad(pstrLoad);

    // Convert the class path to a relative data server path
    TString strActual;
    facCQCRemBrws().CMLClassPathToRelPath(strClassPath, strActual);

    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        TDataSrvClient          dsclLoad;
        tCIDLib::TCard4         c4SerialNum = 0;
        tCIDLib::TKVPFList      colMeta;
        tCIDLib::TEncodedTime   enctLastChange;
        const tCIDLib::TBoolean bRes = dsclLoad.bReadMacro
        (
            strActual
            , c4SerialNum
            , enctLastChange
            , *pstrLoad
            , kCIDLib::False
            , colMeta
            , m_cuctxToUse.sectUser()
        );
    }

    //
    //  Ok, we got it, so create a string based text input stream and give
    //  it our string and give it back.
    //
    return new TTextStringInStream(janLoad.pobjOrphan(), tCIDLib::EAdoptOpts::Adopt);
}

