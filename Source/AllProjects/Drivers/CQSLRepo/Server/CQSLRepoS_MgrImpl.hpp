//
// FILE NAME: CQSLRepoS_MgrImpl.hpp
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
//  This is our implementation of the repository management interface. We
//  just forward commands to our driver object.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQSLRepoMgrImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TCQSLRepoMgrImpl : public TCQSLRepoMgrServerBase
{
    public :
        // --------------------------------------------------------------------
        // Constructors Destructor
        // --------------------------------------------------------------------
        TCQSLRepoMgrImpl() = delete;

        TCQSLRepoMgrImpl
        (
                    TCQSLRepoSDriver* const pdrvImpl
        );

        TCQSLRepoMgrImpl(const TCQSLRepoMgrImpl&) = delete;
        TCQSLRepoMgrImpl(TCQSLRepoMgrImpl&&) = delete;

        ~TCQSLRepoMgrImpl();


        // --------------------------------------------------------------------
        // Public operators
        // --------------------------------------------------------------------
        TCQSLRepoMgrImpl& operator=(const TCQSLRepoMgrImpl&) = delete;
        TCQSLRepoMgrImpl& operator=(TCQSLRepoMgrImpl&&) = delete;


        // --------------------------------------------------------------------
        // Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bGetLease
        (
            const   TString&                strLeaseId
            , const tCIDLib::TBoolean       bOverride
        )   final;

        tCIDLib::TBoolean bQueryArtById
        (
            const   tCQCMedia::EMediaTypes  eMediaType
            , const tCQCMedia::ERArtTypes   eArtType
            , const tCIDLib::TCard4         c4Id
            ,       tCIDLib::TCard4&        c4ArtSz
            ,       THeapBuf&               mbufArt
            , const tCIDLib::TBoolean       bThrowIfNot
        )   final;

        tCIDLib::TBoolean bQueryDB
        (
                    tCIDLib::TCard4&        c4CompSz
            ,       THeapBuf&               mbufComp
            , const TString&                strDBSerNum
            ,       TString&                strRepoPath
        )   final;

        tCIDLib::TBoolean bStartUpload
        (
            const   TString&                strLeaseId
            , const tCIDLib::TCard4         c4DBSize
            , const tCIDLib::TCard4         c4ImgCount
            , const tCIDLib::TCard4         c4TrackCount
            ,       TString&                strHostUpload
            ,       tCIDLib::TIPPortNum&    ippnUpload
            ,       tCIDLib::TCard4&        c4Cookie
        )   final;

        tCIDLib::TVoid DropLease
        (
            const   TString&                strLeaseId
        )   final;

        tCIDLib::TVoid QueryChangers
        (
                    tCIDLib::TStrList&      colMonikersToFill
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // --------------------------------------------------------------------
        //  Private data members.
        //
        //  m_pdrvImpl
        //      We get a pointer to the driver we provide a front end to. We
        //      just delegate calls to it.
        // --------------------------------------------------------------------
        TCQSLRepoSDriver*   m_pdrvImpl;


        // --------------------------------------------------------------------
        // Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TCQSLRepoMgrImpl,TCQSLRepoMgrServerBase)
};

#pragma CIDLIB_POPPACK

