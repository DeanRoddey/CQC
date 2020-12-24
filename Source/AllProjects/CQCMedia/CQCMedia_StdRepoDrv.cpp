//
// FILE NAME: CQCMedia_StdRepoDrv.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/14/2006
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
//  This is the implementation file for the standard repository driver base
//  class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMedia_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCStdMediaRepoDrv,TCQCServerBase)


// ---------------------------------------------------------------------------
//  CLASS: TCQCStdMediaRepoDrv
// PREFIX: sdrv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCStdMediaRepoDrv: Destructor
// ---------------------------------------------------------------------------
TCQCStdMediaRepoDrv::~TCQCStdMediaRepoDrv()
{
}


// ---------------------------------------------------------------------------
//  TCQCStdMediaRepoDrv: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We support a number of backdoor commands to query data. These are overrides
//  of those methods. Derived repo drivers can override further still to
//  provide more queries or to intercept incoming queries or the returned
//  information and modify them.
//
tCIDLib::TBoolean
TCQCStdMediaRepoDrv::bQueryData(const   TString&            strQType
                                , const TString&            strQData
                                ,       tCIDLib::TCard4&    c4OutBytes
                                ,       THeapBuf&           mbufToFill)
{
    //
    //  Make sure the out bytes get initalized to zero just in case, so
    //  that the ORB doesn't try to stream back some crazy random data.
    //
    c4OutBytes = 0;

    //
    //  We handle the query for metadata dumps here instead of passing it to
    //  the engine. We have the serial number info here.
    //
    if ((strQType == kCQCMedia::strQuery_BinMediaDump)
    ||  (strQType == kCQCMedia::strQuery_XMLMediaDump))
    {
        const TMediaDB& mdbDump = m_psrdbEng->mdbInfo();

        // If the serial number is the same, we return nothing
        if (strQData == m_strDBSerialNum)
        {
            c4OutBytes = 0;
            return kCIDLib::False;
        }

        // Get a dump of the requested type
        if (strQType == kCQCMedia::strQuery_BinMediaDump)
        {
            c4OutBytes = mdbDump.c4BuildBinDump
            (
                mbufToFill
                , strMoniker()
                , m_strDBSerialNum
                , m_psrdbEng->eMediaTypes()
            );
        }
         else
        {
            c4OutBytes = mdbDump.c4BuildXMLDump
            (
                mbufToFill
                , strMoniker()
                , m_strDBSerialNum
                , m_psrdbEng->eMediaTypes()
            );
        }
        return kCIDLib::True;
    }

    return m_psrdbEng->bQueryData(strQType, strQData, c4OutBytes, mbufToFill);
}


tCIDLib::TBoolean
TCQCStdMediaRepoDrv::bQueryData2(const  TString&            strQType
                                ,       tCIDLib::TCard4&    c4IOBytes
                                ,       THeapBuf&           mbufIO)
{
    return m_psrdbEng->bQueryData2(strQType, c4IOBytes, mbufIO);
}


tCIDLib::TBoolean
TCQCStdMediaRepoDrv::bQueryTextVal( const   TString&    strQType
                                    , const TString&    strQData
                                    ,       TString&    strToFill)
{
    // We handle this one ourself
    if (strQType == kCQCMedia::strQuery_DBSerialNumber)
    {
        strToFill = m_strDBSerialNum;
        return kCIDLib::True;
    }
    return m_psrdbEng->bQueryTextVal(strQType, strQData, strToFill);
}


tCIDLib::TCard4 TCQCStdMediaRepoDrv::c4QueryVal(const TString& strValId)
{
    // Else pass to the engine
    return m_psrdbEng->c4QueryVal(strValId);
}



//
//  We don't currently support any outgoing setting/sending backdoor commands,
//  but some of our derived classes might. So we provide some dummy
//  implementations so that they can call through to us in case we should
//  add any later.
//
//  The buffer is in/out, so the engine should zero the bytes if it's not
//  returning anything!
//
tCIDLib::TBoolean
TCQCStdMediaRepoDrv::bSendData( const   TString&            strSendType
                                ,       TString&            strDataName
                                ,       tCIDLib::TCard4&    c4Bytes
                                ,       THeapBuf&           mbufToSend)
{
    return m_psrdbEng->bSendData(strSendType, strDataName, c4Bytes, mbufToSend);
}


tCIDLib::TCard4
TCQCStdMediaRepoDrv::c4SendCmd(const TString& strCmdId, const TString& strData)
{
    //
    //  Unless overridden, we assume the repo cannot accept this type of dynamic
    //  update, and return 2 to indicate this. Repos that can support this command
    //  should override this method and handle this command.
    //
    if (strCmdId == kCQCMedia::strCmd_SetUserRating)
        return 2;

    return m_psrdbEng->c4SendCmd(strCmdId, strData);
}


// ---------------------------------------------------------------------------
//  TCQCStdMediaRepoDrv: Hidden constructors
// ---------------------------------------------------------------------------

//
//  The actual repo driver calls through to here, passing us his driver config
//  data, and the types of media he supports. He also provides a derivative
//  of the standard repo engine. We don't own it he does.
//
TCQCStdMediaRepoDrv::
TCQCStdMediaRepoDrv(const   TCQCDriverObjCfg&           cqcdcToLoad
                    ,       TCQCStdMediaRepoEng* const  psrdbEng) :

    TCQCServerBase(cqcdcToLoad)
    , m_psrdbEng(psrdbEng)
{
}

// ---------------------------------------------------------------------------
//  TCQCStdMediaRepoDrv: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Get the currently set serial number
const TString& TCQCStdMediaRepoDrv::strDBSerialNum() const
{
    return m_strDBSerialNum;
}


// Set a new serial number
tCIDLib::TVoid
TCQCStdMediaRepoDrv::SetDBSerialNum(const TString& strToSet)
{
    m_strDBSerialNum = strToSet;
}


tCIDLib::TVoid TCQCStdMediaRepoDrv::ThrowLoading()
{
    facCQCMedia().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kMedErrs::errcDB_LoadingBusy
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Already
    );
}


