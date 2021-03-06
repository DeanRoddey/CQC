// ----------------------------------------------------------------------------
//  FILE: CQSLRepoS_CQSLRepoServerBase.cpp
//  DATE: Fri, Feb 12 21:14:16 2021 -0500
//    ID: 664424407F8B705F-F0C14316C78994B2
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------------
#include "CQSLRepoS_.hpp"

// ----------------------------------------------------------------------------
// Magic macros
// ----------------------------------------------------------------------------
RTTIDecls(TCQSLRepoMgrServerBase,TOrbServerBase)


// ----------------------------------------------------------------------------
// Public, static data
// ----------------------------------------------------------------------------
const TString TCQSLRepoMgrServerBase::strInterfaceId(L"664424407F8B705F-F0C14316C78994B2");
const TString TCQSLRepoMgrServerBase::strBindingName(L"MgrIntf");
const TString TCQSLRepoMgrServerBase::strBindingScope(L"/CQC/CQSLRepo/%(m)");


// ----------------------------------------------------------------------------
// TCQSLRepoMgrServerBase: Destructor
// ----------------------------------------------------------------------------
TCQSLRepoMgrServerBase::~TCQSLRepoMgrServerBase()
{
}

// ----------------------------------------------------------------------------
// TCQSLRepoMgrServerBase: Protected Constructors
// ----------------------------------------------------------------------------
TCQSLRepoMgrServerBase::TCQSLRepoMgrServerBase() :
    TOrbServerBase(strInterfaceId, L"TCQSLRepoMgrClientProxy" )
{
}

TCQSLRepoMgrServerBase::TCQSLRepoMgrServerBase(const TOrbObjId& ooidThis) :
    TOrbServerBase(ooidThis)
{
}

// ----------------------------------------------------------------------------
// TCQSLRepoMgrServerBase: Public, non-virtual methods
// ----------------------------------------------------------------------------
tCIDLib::TVoid TCQSLRepoMgrServerBase::Dispatch(const TString& strMethodName, TOrbCmd& orbcToDispatch)
{
    if (strMethodName == L"bGetLease")
    {
        TString strLeaseId;
        orbcToDispatch.strmIn() >> strLeaseId;
        tCIDLib::TBoolean bOverride;
        orbcToDispatch.strmIn() >> bOverride;
        tCIDLib::TBoolean retVal = bGetLease
        (
            strLeaseId
          , bOverride
        );
        orbcToDispatch.strmOut().Reset();
        orbcToDispatch.strmOut() << retVal;
    }
     else if (strMethodName == L"bQueryArtById")
    {
        tCQCMedia::EMediaTypes eMediaType;
        orbcToDispatch.strmIn() >> eMediaType;
        tCQCMedia::ERArtTypes eArtType;
        orbcToDispatch.strmIn() >> eArtType;
        tCIDLib::TCard4 c4Id;
        orbcToDispatch.strmIn() >> c4Id;
        tCIDLib::TCard4 c4BufSz_mbufArt = 0;
        THeapBuf mbufArt;
        tCIDLib::TBoolean bThrowIfNot;
        orbcToDispatch.strmIn() >> bThrowIfNot;
        tCIDLib::TBoolean retVal = bQueryArtById
        (
            eMediaType
          , eArtType
          , c4Id
          , c4BufSz_mbufArt
          , mbufArt
          , bThrowIfNot
        );
        orbcToDispatch.strmOut().Reset();
        orbcToDispatch.strmOut() << retVal;
        orbcToDispatch.strmOut() << c4BufSz_mbufArt;
        orbcToDispatch.strmOut().c4WriteBuffer(mbufArt, c4BufSz_mbufArt);
    }
     else if (strMethodName == L"bQueryDB")
    {
        tCIDLib::TCard4 c4BufSz_mbufComp = 0;
        THeapBuf mbufComp;
        TString strDBSerNum;
        orbcToDispatch.strmIn() >> strDBSerNum;
        TString strRepoPath;
        tCIDLib::TBoolean retVal = bQueryDB
        (
            c4BufSz_mbufComp
          , mbufComp
          , strDBSerNum
          , strRepoPath
        );
        orbcToDispatch.strmOut().Reset();
        orbcToDispatch.strmOut() << retVal;
        orbcToDispatch.strmOut() << c4BufSz_mbufComp;
        orbcToDispatch.strmOut().c4WriteBuffer(mbufComp, c4BufSz_mbufComp);
        orbcToDispatch.strmOut() << strRepoPath;
    }
     else if (strMethodName == L"bStartUpload")
    {
        TString strLeaseId;
        orbcToDispatch.strmIn() >> strLeaseId;
        tCIDLib::TCard4 c4DBSize;
        orbcToDispatch.strmIn() >> c4DBSize;
        tCIDLib::TCard4 c4ImgCount;
        orbcToDispatch.strmIn() >> c4ImgCount;
        tCIDLib::TCard4 c4TrackCount;
        orbcToDispatch.strmIn() >> c4TrackCount;
        TString strHostUpload;
        tCIDLib::TCard4 ippnUpload = {};
        tCIDLib::TCard4 c4Cookie = {};
        tCIDLib::TBoolean retVal = bStartUpload
        (
            strLeaseId
          , c4DBSize
          , c4ImgCount
          , c4TrackCount
          , strHostUpload
          , ippnUpload
          , c4Cookie
        );
        orbcToDispatch.strmOut().Reset();
        orbcToDispatch.strmOut() << retVal;
        orbcToDispatch.strmOut() << strHostUpload;
        orbcToDispatch.strmOut() << ippnUpload;
        orbcToDispatch.strmOut() << c4Cookie;
    }
     else if (strMethodName == L"DropLease")
    {
        TString strLeaseId;
        orbcToDispatch.strmIn() >> strLeaseId;
        DropLease
        (
            strLeaseId
        );
        orbcToDispatch.strmOut().Reset();
    }
     else if (strMethodName == L"QueryChangers")
    {
        TVector<TString> colMonikersToFill;
        QueryChangers
        (
            colMonikersToFill
        );
        orbcToDispatch.strmOut().Reset();
        orbcToDispatch.strmOut() << colMonikersToFill;
    }
     else
    {
         TParent::Dispatch(strMethodName, orbcToDispatch);
    }
}

