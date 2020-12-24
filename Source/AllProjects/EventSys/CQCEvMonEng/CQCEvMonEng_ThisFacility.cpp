//
// FILE NAME: CQCEvMonEng_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/08/2014
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCEvMonEng_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCEvMonEng,TFacility)




// ---------------------------------------------------------------------------
//  CLASS: TCQCEvMonClassLoader
// PREFIX: mecl
// ---------------------------------------------------------------------------
class TCQCEvMonClassLoader : public MMEngExtClassLoader
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCEvMonClassLoader() {}

        TCQCEvMonClassLoader(const TCQCEvMonClassLoader&) = delete;
        TCQCEvMonClassLoader(TCQCEvMonClassLoader&&) = delete;

        ~TCQCEvMonClassLoader() {}


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCEvMonClassLoader& operator=(const TCQCEvMonClassLoader&) = delete;
        TCQCEvMonClassLoader& operator=(TCQCEvMonClassLoader&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] TMEngClassInfo* pmeciLoadClass
        (
                    TCIDMacroEngine&        meTarget
            , const TString&                strClassPath
        )   final;
};


// ---------------------------------------------------------------------------
//  TCQCRTClassLoader: Public, inherited methods
// ---------------------------------------------------------------------------
TMEngClassInfo*
TCQCEvMonClassLoader::pmeciLoadClass(       TCIDMacroEngine&    meTarget
                                    , const TString&            strClassPath)
{
    TMEngClassInfo* pmeciRet = nullptr;

    if (strClassPath == TEvMonBaseInfo::strPath())
        pmeciRet = new TEvMonBaseInfo(meTarget);
    return pmeciRet;
}




// ---------------------------------------------------------------------------
//   CLASS: TFacCQCEvMonEng
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCEvMonEng: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCEvMonEng::TFacCQCEvMonEng() :

    TFacility
    (
        L"CQCEvMonEng"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_bCMLInit(kCIDLib::False)
    , m_ppolleMon(nullptr)
    , m_sectUser()
{
}

TFacCQCEvMonEng::~TFacCQCEvMonEng()
{
}



// ---------------------------------------------------------------------------
//  TFacCQCEvMonEng: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TFacCQCEvMonEng::AddFields( const   tCIDLib::TStrList&          colToAdd
                            ,       tCQCEvMonEng::TPollList&    colToFill)
{
    CIDAssert(m_ppolleMon != nullptr, L"The event monitor polling engine hasn't been set");

    //
    //  Update our list of poll info fields. Any that we did use and now don't (and
    //  any other sessions don't) will eventually time out of the engine.
    //
    colToFill.RemoveAll();

    TString strMoniker;
    TString strFldName;

    TCQCFldCache cfcFields;
    tCIDLib::TCard4 c4Count = colToAdd.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (facCQCKit().bParseFldName(colToAdd[c4Index], strMoniker, strFldName))
        {
            TCQCFldPollInfo cfpiCur(strMoniker, strFldName);
            colToFill.objAdd(cfpiCur);
        }
         else
        {
        }
    }

    //
    //  Go back through the ones that we kept (no errors in the field name), and register
    //  them with the poll engine in the facility object.
    //
    c4Count = colToFill.c4ElemCount();
    if (c4Count)
    {
        TCQCFldCache cfcData;
        cfcData.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite));
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TCQCFldPollInfo& cfpiCur = colToFill[c4Index];
            if (!cfpiCur.bRegister(*m_ppolleMon, cfcData))
            {
            }
        }
    }
}


//
//  The web server will call this to set us up. It's also called from the admin
//  client which needs to have the appropriate CML classes available for the CML
//  IDE if they create an event monitor and need to test it.
//
//  The containing app has to provide us with a polling engine to use. We don't
//  adopt it. We assume this is a shared one that he is handing out to other
//  facilities as well.
//
//  It's assumed this is called during startup before there are any sync issues
//  to worry about.
//
tCIDLib::TVoid
TFacCQCEvMonEng::Initialize(TCQCPollEngine* ppolleToUse, const TCQCSecToken& sectUser)
{
    m_sectUser = sectUser;

    // If we've not initialized the CML stuff yet, do that
    if (!m_bCMLInit)
    {
        facCIDMacroEng().AddClassLoader(new TCQCEvMonClassLoader);
        m_bCMLInit = kCIDLib::True;
    }

    // And store the passed polling engine
    m_ppolleMon = ppolleToUse;
}


TCQCPollEngine& TFacCQCEvMonEng::polleThis()
{
    CIDAssert(m_ppolleMon != nullptr, L"The event monitor polling engine hasn't been set");
    return *m_ppolleMon;
}


tCIDLib::TVoid TFacCQCEvMonEng::Terminate()
{
    // Drop our polling engine reference, since we should not be using it now
    m_ppolleMon = nullptr;
    m_sectUser.Reset();
}


tCIDLib::TVoid
TFacCQCEvMonEng::WriteField(const TString& strFld, const TString& strValue)
{
    CIDAssert(m_ppolleMon != nullptr, L"The event monitor polling engine hasn't been set");
    m_ppolleMon->WriteField(strFld, strValue, m_sectUser);
}

tCIDLib::TVoid
TFacCQCEvMonEng::WriteField(const   TString&        strMoniker
                            , const TString&        strFld
                            , const TString&        strValue)

{
    CIDAssert(m_ppolleMon != nullptr, L"The event monitor polling engine hasn't been set");
    m_ppolleMon->WriteField(strMoniker, strFld, strValue, m_sectUser);
}


