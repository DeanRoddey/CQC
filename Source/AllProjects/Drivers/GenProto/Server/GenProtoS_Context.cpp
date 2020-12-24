//
// FILE NAME: GenProtoS_Context.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This file implements the context class which is passed into code that
//  parses and builds messages, so that they can access various bits of
//  information that is available. Doing it this way make them all just
//  have a single interface without worrying about where the info actually
//  comes from.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GenProtoS_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGenProtoCtx,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoCtx
//  PREFIX: ctx
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoCtx: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoCtx::TGenProtoCtx( const   tGenProtoS::ESpecNodes  eAccess
                            , const TString&                strName) :

    m_c4DataBytes(0)
    , m_evalWrite(tGenProtoS::ETypes::Card1)
    , m_eAccess(eAccess)
    , m_mbufData(1024UL)
    , m_psdrvThis(nullptr)
    , m_ptcvtDevice(new TUSASCIIConverter)
    , m_strName(strName)
{
    //
    //  Make sure that the send/reply buffers are never both accessible,
    //  because we currently only have one actual buffer internally.
    //
    #if CID_DEBUG_ON
    if (tCIDLib::bAllBitsOn(eAccess, tGenProtoS::ESpecNodes::SendReplyBuf))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcDebug_BothCtxBuffers
            , tCIDLib::ESeverities::ProcFatal
            , tCIDLib::EErrClasses::Internal
            , m_strName
        );
    }
    #endif
}

TGenProtoCtx::~TGenProtoCtx()
{
    delete m_ptcvtDevice;
}


// ---------------------------------------------------------------------------
//  TGenProtoCtx: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard1 TGenProtoCtx::c1InputByte() const
{
    CheckAccess(tGenProtoS::ESpecNodes::InputByte, L"input byte");
    return m_c1InputByte;
}

tCIDLib::TCard1 TGenProtoCtx::c1InputByte(const tCIDLib::TCard1 c1ToSet)
{
    m_c1InputByte = c1ToSet;
    return m_c1InputByte;
}


tCIDLib::TCard4 TGenProtoCtx::c4ReplyBytes() const
{
    CheckAccess(tGenProtoS::ESpecNodes::ReplyBuf, L"reply buffer");
    return m_c4DataBytes;
}

tCIDLib::TCard4 TGenProtoCtx::c4ReplyBytes(const tCIDLib::TCard4 c4ToSet)
{
    CheckAccess(tGenProtoS::ESpecNodes::ReplyBuf, L"reply buffer");
    m_c4DataBytes = c4ToSet;
    return m_c4DataBytes;
}


tCIDLib::TCard4 TGenProtoCtx::c4SendBytes() const
{
    CheckAccess(tGenProtoS::ESpecNodes::SendBuf, L"send buffer");
    return m_c4DataBytes;
}

tCIDLib::TCard4 TGenProtoCtx::c4SendBytes(const tCIDLib::TCard4 c4ToSet)
{
    CheckAccess(tGenProtoS::ESpecNodes::SendBuf, L"send buffer");
    m_c4DataBytes = c4ToSet;
    return m_c4DataBytes;
}


tCIDLib::TVoid
TGenProtoCtx::ChangeEncoding(const TString& strEncodingToSet)
{
    delete m_ptcvtDevice;
    m_ptcvtDevice = 0;
    m_ptcvtDevice = facCIDEncode().ptcvtMake(strEncodingToSet);
}


const TGenProtoExprVal& TGenProtoCtx::evalWrite() const
{
    CheckAccess(tGenProtoS::ESpecNodes::WriteVal, L"write value");
    return m_evalWrite;
}

TGenProtoExprVal& TGenProtoCtx::evalWrite()
{
    CheckAccess(tGenProtoS::ESpecNodes::WriteVal, L"write value");
    return m_evalWrite;
}

TGenProtoExprVal& TGenProtoCtx::evalWrite(const TGenProtoExprVal& evalToSet)
{
    CheckAccess(tGenProtoS::ESpecNodes::WriteVal, L"write value");
    m_evalWrite = evalToSet;
    return m_evalWrite;
}


const TGenProtoInfo& TGenProtoCtx::gpinfoProto() const
{
    return m_gpinfoProto;
}

const TGenProtoInfo&
TGenProtoCtx::gpinfoProto(const TGenProtoInfo& gpinfoToSet)
{
    m_gpinfoProto = gpinfoToSet;
    return m_gpinfoProto;
}



const TMemBuf& TGenProtoCtx::mbufReply() const
{
    CheckAccess(tGenProtoS::ESpecNodes::ReplyBuf, L"reply buffer");
    return m_mbufData;
}


TMemBuf& TGenProtoCtx::mbufReply()
{
    CheckAccess(tGenProtoS::ESpecNodes::ReplyBuf, L"reply buffer");
    return m_mbufData;
}


const TMemBuf& TGenProtoCtx::mbufSend() const
{
    CheckAccess(tGenProtoS::ESpecNodes::SendBuf, L"send buffer");
    return m_mbufData;
}


TMemBuf& TGenProtoCtx::mbufSend()
{
    CheckAccess(tGenProtoS::ESpecNodes::SendBuf, L"send buffer");
    return m_mbufData;
}


TGenProtoSDriver* TGenProtoCtx::psdrvThis()
{
    return m_psdrvThis;
}


const TString& TGenProtoCtx::strProtoFile() const
{
    return m_strProtoFile;
}

const TString& TGenProtoCtx::strProtoFile(const TString& strToSet)
{
    m_strProtoFile = strToSet;
    return m_strProtoFile;
}


tCIDLib::TVoid TGenProtoCtx::SetDriver(TGenProtoSDriver* const psdrvToSet)
{
    m_psdrvThis = psdrvToSet;
}


TTextConverter& TGenProtoCtx::tcvtDevice()
{
    return *m_ptcvtDevice;
}


// ---------------------------------------------------------------------------
//  TGenProtoCtx: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoCtx::CheckAccess(  const   tGenProtoS::ESpecNodes  eToCheck
                            , const TString&                strMember) const
{
    // If none of the check bits are on in our access member, it's bad
    if (!tCIDLib::bAnyBitsOn(m_eAccess, eToCheck))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcRunT_ContextAccess
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotSupported
            , m_strName
            , strMember
        );
    }
}


