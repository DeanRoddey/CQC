//
// FILE NAME: CQCEventSrv_ProtoServerImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/18/2004
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
//  This is the header for the CQCEventSrv_ProtoServerImpl.cpp file, which
//  provides the implementation of the IDL generated server stub for the
//  protocol interface that we provide for the admin client to configure
//  us.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TEventServerImpl
//  PREFIX: acr
// ---------------------------------------------------------------------------
class TEventServerImpl : public TEventSrvServerBase
{
    public :
        // --------------------------------------------------------------------
        //  Constructors and Destructor
        // --------------------------------------------------------------------
        TEventServerImpl() = default;

        TEventServerImpl(const TEventServerImpl&) = delete;
        TEventServerImpl(TEventServerImpl&&) = delete;

        ~TEventServerImpl() = default;


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TEventServerImpl& operator=(const TEventServerImpl&) = delete;
        TEventServerImpl& operator=(TEventServerImpl&&) = delete;


        // --------------------------------------------------------------------
        //  Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid DumpEvents
        (
            const   TString&                strTarFile
        )   final;

        tCIDLib::TVoid AddEvent
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelPath
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid DeleteEvent
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelPath
            ,        tCIDLib::TCard4&       c4ChangeSerNum
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid PauseEvent
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelPath
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            , const tCIDLib::TBoolean       bNewState
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid RenameEvent
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelParPath
            , const TString&                strOldName
            , const TString&                strNewName
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid RenameScope
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelParPath
            , const TString&                strOldName
            , const TString&                strNewName
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid UpdateEvent
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelPath
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid Ping() final;

        tCIDLib::TVoid QueryPerEvInfo
        (
            const   TString&                strPath
            ,       tCQCKit::ESchTypes&     eType
            ,       tCIDLib::TEncodedTime&  enctStart
            ,       tCIDLib::TCard4&        c4Period
            ,       tCIDLib::TBoolean&      bPaused
        )   final;

        tCIDLib::TVoid QuerySchEvInfo
        (
            const   TString&                strPath
            ,       tCQCKit::ESchTypes&     eType
            ,       tCIDLib::TCard4&        c4Day
            ,       tCIDLib::TCard4&        c4Hour
            ,       tCIDLib::TCard4&        c4Minute
            ,       tCIDLib::TCard4&        c4DayMask
            ,       tCIDLib::TBoolean&      bPaused
        )   final;

        tCIDLib::TVoid QuerySunBasedEvInfo
        (
            const   TString&                strPath
            ,       tCIDLib::TInt4&         i4Offset
            ,       tCIDLib::TBoolean&      bPaused
        )   final;

        tCIDLib::TVoid ReloadList
        (
            const tCQCKit::EEvSrvTypes eToReload
        )   final;

        tCIDLib::TVoid SetLocInfo
        (
            const   tCIDLib::TFloat8        f8Lat
            , const tCIDLib::TFloat8        f8Long
            , const TCQCSecToken&           sectUser
        )   final;


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // --------------------------------------------------------------------
        // Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TEventServerImpl,TEventSrvServerBase)
};

#pragma CIDLIB_POPPACK


