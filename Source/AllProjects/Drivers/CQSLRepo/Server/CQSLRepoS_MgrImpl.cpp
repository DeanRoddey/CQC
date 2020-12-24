//
// FILE NAME: CQSLRepoS_MgrImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/02/2006
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
//  This file provides the implementation of the repository management
//  interface. We just delegate everything to driver object.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQSLRepoS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQSLRepoMgrImpl,TCQSLRepoMgrServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TCQSLRepoMgrImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLRepoMgrImpl: Constructors Destructor
// ---------------------------------------------------------------------------
TCQSLRepoMgrImpl::TCQSLRepoMgrImpl(TCQSLRepoSDriver* const pdrvImpl) :

    m_pdrvImpl(pdrvImpl)
{
}

TCQSLRepoMgrImpl::~TCQSLRepoMgrImpl()
{
}

// ---------------------------------------------------------------------------
//  TCQSLRepoMgrImpl: Public, inherited methods
// ---------------------------------------------------------------------------
//
//  Called to get a lease on the interface. Used to prevent multiple
//  clients from connecting at once.
//
tCIDLib::TBoolean
TCQSLRepoMgrImpl::bGetLease(const   TString&            strLeaseId
                            , const tCIDLib::TBoolean   bOverride)
{
    return m_pdrvImpl->bGetLease(strLeaseId, bOverride);
}


// Queries cover art data by numeric id
tCIDLib::TBoolean
TCQSLRepoMgrImpl::bQueryArtById(const   tCQCMedia::EMediaTypes  eMediaType
                                , const tCQCMedia::ERArtTypes   eArtType
                                , const tCIDLib::TCard4         c4Id
                                ,       tCIDLib::TCard4&        c4ArtSz
                                ,       THeapBuf&               mbufArt
                                , const tCIDLib::TBoolean       bThrowIfNot)
{
    const tCIDLib::TCard2 c2Id = tCIDLib::TCard2(c4Id);
    const tCIDLib::TBoolean bRes = m_pdrvImpl->bQueryArtById
    (
        eMediaType, eArtType, c2Id, c4ArtSz, mbufArt
    );

    if (!bRes && bThrowIfNot)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownObjId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4Id)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Image)
            , tCQCMedia::strXlatEMediaTypes(eMediaType)
        );
    }
    return bRes;
}


tCIDLib::TBoolean
TCQSLRepoMgrImpl::bQueryDB(         tCIDLib::TCard4&  c4CompSz
                            ,       THeapBuf&         mbufComp
                            , const TString&          strDBSerNum
                            ,       TString&          strRepoPath)
{
    // Just in case it should fail to set the return size
    c4CompSz = 0;
    return m_pdrvImpl->bQueryDB(c4CompSz, mbufComp, strDBSerNum, strRepoPath);
}


tCIDLib::TBoolean TCQSLRepoMgrImpl::
bStartUpload(const  TString&                strLeaseId
            , const tCIDLib::TCard4         c4DBSize
            , const tCIDLib::TCard4         c4ImgCount
            , const tCIDLib::TCard4         c4TrackCount
            ,       TString&                strHostUpload
            ,       tCIDLib::TIPPortNum&    ippnUpload
            ,       tCIDLib::TCard4&        c4Cookie)
{
    return m_pdrvImpl->bStartUpload
    (
        strLeaseId
        , c4DBSize
        , c4ImgCount
        , c4TrackCount
        , strHostUpload
        , ippnUpload
        , c4Cookie
    );
}


tCIDLib::TVoid TCQSLRepoMgrImpl::DropLease(const TString& strLeaseId)
{
    m_pdrvImpl->DropLease(strLeaseId);
}


// Returns a list of referenced changer monikers
tCIDLib::TVoid
TCQSLRepoMgrImpl::QueryChangers(TVector<TString>& colMonikersToFill)
{
    return m_pdrvImpl->QueryChangers(colMonikersToFill);
}


// ---------------------------------------------------------------------------
//  TCQSLRepoMgrImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQSLRepoMgrImpl::Initialize()
{
}


tCIDLib::TVoid TCQSLRepoMgrImpl::Terminate()
{
}

