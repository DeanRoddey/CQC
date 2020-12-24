//
// FILE NAME: CQCMEng_ClassManager.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/24/2001
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
//  This file implements a derivative of the CIDLib Macro Engine's class
//  manager interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMEng_.hpp"
#include    "CQCMEng_CQCUtilsClass_.hpp"
#include    "CQCMEng_CRCHelpersClass_.hpp"
#include    "CQCMEng_DevSenseClass_.hpp"
#include    "CQCMEng_EventClass_.hpp"
#include    "CQCMEng_LoggerClass_.hpp"
#include    "CQCMEng_FieldDefClass_.hpp"
#include    "CQCMEng_SimpleFldClientClass_.hpp"
#include    "CQCMEng_V2HelperClass_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCMEngClassMgr,TObject)



// ---------------------------------------------------------------------------
//  CLASS: TCQCMEngClassMgr
// PREFIX: mecm
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCMEngClassMgr: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCMEngClassMgr::TCQCMEngClassMgr(const TCQCSecToken& sectUser) :

    m_sectUser(sectUser)
{
}

TCQCMEngClassMgr::~TCQCMEngClassMgr()
{
}


// ---------------------------------------------------------------------------
//  TCQCMEngClassMgr: Protected, virtual methods
// ---------------------------------------------------------------------------
const TCQCSecToken& TCQCMEngClassMgr::sectUser() const
{
    return m_sectUser;
}


tCIDLib::TVoid TCQCMEngClassMgr::Shutdown()
{
}


// ---------------------------------------------------------------------------
//  TCQCMEngClassMgr: Protected, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCMEngClassMgr::bCheckIfExists(const TString& strToCheck)
{
    // Convert the class path
    TString strRelPath;
    facCQCRemBrws().CMLClassPathToRelPath(strToCheck, strRelPath);

    TDataSrvClient dsclRead;
    return dsclRead.bFileExists(strRelPath, tCQCRemBrws::EDTypes::Macro);
}


tCIDLib::EOpenRes
TCQCMEngClassMgr::eDoSelect(TString&, const tCIDMacroEng::EResModes)
{
    // We don't support selection in this one, so return cancel
    return tCIDLib::EOpenRes::Cancel;
}


tCIDLib::TVoid TCQCMEngClassMgr::DoStore(const  TString&    strClassPath
                                        , const TString&    strText)
{
    // Convert the class path
    TString strRelPath;
    facCQCRemBrws().CMLClassPathToRelPath(strClassPath, strRelPath);


    TDataSrvClient dsclRead;

    // First make sure the parent scope exists
    TString strScope(strRelPath);
    facCQCRemBrws().bRemoveLastPathItem(strScope);
    dsclRead.MakePath(strScope, tCQCRemBrws::EDTypes::Macro, m_sectUser);

    // And try to write the file
    tCIDLib::TKVPFList colExtraMeta;
    tCIDLib::TCard4 c4NewSerialNum;
    tCIDLib::TEncodedTime enctLastChange;
    dsclRead.WriteMacro
    (
        strRelPath
        , c4NewSerialNum
        , enctLastChange
        , strText
        , kCQCRemBrws::c4Flag_OverwriteOK
        , colExtraMeta
        , m_sectUser
    );
}


tCIDLib::TVoid TCQCMEngClassMgr::DoUndoWriteMode(const TString&)
{
    //
    //  We really don't have any thing to do here at this time, since the
    //  macro respository doesn't support check out semantics.
    //
}


//
//  Note that we ignore the mode for now, since the macro repository does not
//  have checkout semantics.
//
TTextInStream*
TCQCMEngClassMgr::pstrmDoLoad(  const   TString&                strClassPath
                                , const tCIDMacroEng::EResModes    )
{
    // Convert the class path
    TString strRelPath;
    facCQCRemBrws().CMLClassPathToRelPath(strClassPath, strRelPath);


    //
    //  Try to load this guy up. We'll need a string to use. If it works,
    //  we give the string to the stream we return. Put a janitor on it for
    //  the meantime. Here, since we are always used just to load classes
    //  for invocation, we say we want to get either encrypted or non-encrypted
    //  macros.
    //
    TString* pstrLoad = new TString;
    TJanitor<TString> janLoad(pstrLoad);
    {
        //
        //  We use a data server client object to get the file. We just pass zero
        //  as the serial number since aren't doing any local caching here, not
        //  yet anyway. So we don't care about the return value.
        //
        TDataSrvClient          dsclRead;
        tCIDLib::TCard4         c4SerNum = 0;
        tCIDLib::TEncodedTime   enctLastChange;
        tCIDLib::TKVPFList      colMeta;
        dsclRead.bReadMacro
        (
            strRelPath
            , c4SerNum
            , enctLastChange
            , *pstrLoad
            , kCIDLib::True
            , colMeta
            , m_sectUser
        );
    }

    //
    //  Ok, we got it, so create a string based text input stream and give
    //  it our string and give it back.
    //
    return new TTextStringInStream(janLoad.pobjOrphan(), tCIDLib::EAdoptOpts::Adopt);
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCRTClassLoader
// PREFIX: mecl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCRTClassLoader: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCRTClassLoader::TCQCRTClassLoader(const TCQCSecToken& sectUser) :

    m_sectUser(sectUser)
{
}

TCQCRTClassLoader::~TCQCRTClassLoader()
{
}


// ---------------------------------------------------------------------------
//  TCQCRTClassLoader: Public, inherited methods
// ---------------------------------------------------------------------------
TMEngClassInfo*
TCQCRTClassLoader::pmeciLoadClass(          TCIDMacroEngine&    meTarget
                                    , const TString&            strClassPath)
{
    TMEngClassInfo* pmeciRet = 0;

    if (strClassPath == TCQCUtilsInfo::strClassPath())
        pmeciRet = new TCQCUtilsInfo(meTarget, m_sectUser);
    else if (strClassPath == TCQCDevSenseInfo::strClassPath())
        pmeciRet = new TCQCDevSenseInfo(meTarget);
    else if (strClassPath == TEventInfo::strClassPath())
        pmeciRet = new TEventInfo(meTarget);
    else if (strClassPath == TEventHandlerInfo::strClassPath())
        pmeciRet = new TEventHandlerInfo(meTarget);
    else if (strClassPath == TCQCFldDefInfo::strClassPath())
        pmeciRet = new TCQCFldDefInfo(meTarget);
    else if (strClassPath == TCRCHelpersInfo::strClassPath())
        pmeciRet = new TCRCHelpersInfo(meTarget);
    else if (strClassPath == TCQCLoggerInfo::strClassPath())
        pmeciRet = new TCQCLoggerInfo(meTarget);
    else if (strClassPath == TSimpleFldInfo::strClassPath())
        pmeciRet = new TSimpleFldInfo(meTarget, m_sectUser);
    else if (strClassPath == TV2HelperInfo::strClassPath())
        pmeciRet = new TV2HelperInfo(meTarget);
    return pmeciRet;
}

