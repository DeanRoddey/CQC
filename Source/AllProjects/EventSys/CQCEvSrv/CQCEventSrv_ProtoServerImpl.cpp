//
// FILE NAME: CQCEventSrv_ProtoServerImpl.cpp
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
//  This implements the IDL generated server stub for the protocol interface
//  that the admin clients uses to talk to this server. Everything here just
//  delegates over to the facility object, so we are basically just a front
//  end interface that can remain the same while the underlying stuff changes.
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
#include    "CQCEventSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TEventServerImpl,TEventSrvServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TEventServerImpl
//  PREFIX: acr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEventServerImpl: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TEventServerImpl::DumpEvents(const TString& strTarFile)
{
    facCQCEventSrv.DumpEvents(strTarFile);
}


tCIDLib::TVoid
TEventServerImpl::AddEvent( const   tCQCKit::EEvSrvTypes    eEvType
                            , const TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4ChangeSerNum
                            , const TCQCSecToken&           sectUser)
{
    facCQCEventSrv.AddEvent(eEvType, strRelPath, c4ChangeSerNum, sectUser);
}


tCIDLib::TVoid
TEventServerImpl::DeleteEvent(  const   tCQCKit::EEvSrvTypes    eEvType
                                , const TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4ChangeSerNum
                                , const TCQCSecToken&           sectUser)
{
    facCQCEventSrv.DeleteEvent(eEvType, strRelPath, c4ChangeSerNum, sectUser);
}


tCIDLib::TVoid
TEventServerImpl::PauseEvent(const  tCQCKit::EEvSrvTypes    eEvType
                            , const TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4ChangeSerNum
                            , const tCIDLib::TBoolean       bPaused
                            , const TCQCSecToken&           sectUser)
{
    facCQCEventSrv.PauseEvent(eEvType, strRelPath, c4ChangeSerNum, bPaused, sectUser);
}


tCIDLib::TVoid
TEventServerImpl::RenameEvent(  const   tCQCKit::EEvSrvTypes    eEvType
                                , const TString&                strRelParPath
                                , const TString&                strOldName
                                , const TString&                strNewName
                                ,       tCIDLib::TCard4&        c4ChangeSerNum
                                , const TCQCSecToken&           sectUser)
{
    facCQCEventSrv.RenameEvent
    (
        eEvType, strRelParPath, strOldName, strNewName, c4ChangeSerNum, sectUser
    );
}


tCIDLib::TVoid
TEventServerImpl::RenameScope(  const   tCQCKit::EEvSrvTypes    eEvType
                                , const TString&                strRelParPath
                                , const TString&                strOldName
                                , const TString&                strNewName
                                ,       tCIDLib::TCard4&        c4ChangeSerNum
                                , const TCQCSecToken&           sectUser)
{
    facCQCEventSrv.RenameScope
    (
        eEvType, strRelParPath, strOldName, strNewName, c4ChangeSerNum, sectUser
    );
}


tCIDLib::TVoid
TEventServerImpl::UpdateEvent(  const   tCQCKit::EEvSrvTypes    eEvType
                                , const TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4ChangeSerNum
                                , const TCQCSecToken&           sectUser)
{
    facCQCEventSrv.UpdateEvent(eEvType, strRelPath, c4ChangeSerNum, sectUser);
}


tCIDLib::TVoid TEventServerImpl::Ping()
{
    // Nothing to do, just being here is all that matters for this one
}


tCIDLib::TVoid TEventServerImpl
::QueryPerEvInfo(const  TString&                strPath
                ,       tCQCKit::ESchTypes&     eType
                ,       tCIDLib::TEncodedTime&  enctStart
                ,       tCIDLib::TCard4&        c4Period
                ,       tCIDLib::TBoolean&      bPaused)
{
    facCQCEventSrv.QueryPerEvInfo(strPath, eType, enctStart, c4Period, bPaused);
}


tCIDLib::TVoid TEventServerImpl
::QuerySchEvInfo(const  TString&            strPath
                ,       tCQCKit::ESchTypes& eType
                ,       tCIDLib::TCard4&    c4Day
                ,       tCIDLib::TCard4&    c4Hour
                ,       tCIDLib::TCard4&    c4Minute
                ,       tCIDLib::TCard4&    c4DayMask
                ,       tCIDLib::TBoolean&  bPaused)
{
    facCQCEventSrv.QuerySchEvInfo
    (
        strPath, eType, c4Day, c4Hour, c4Minute, c4DayMask, bPaused
    );
}


tCIDLib::TVoid
TEventServerImpl::QuerySunBasedEvInfo(  const   TString&            strPath
                                        ,       tCIDLib::TInt4&     i4Offset
                                        ,       tCIDLib::TBoolean&  bPaused)
{
    facCQCEventSrv.QuerySunBasedEvInfo(strPath, i4Offset, bPaused);
}


tCIDLib::TVoid TEventServerImpl::ReloadList(const tCQCKit::EEvSrvTypes eToReload)
{
    facCQCEventSrv.ReloadList(eToReload);
}


tCIDLib::TVoid
TEventServerImpl::SetLocInfo(const  tCIDLib::TFloat8    f8Lat
                            , const tCIDLib::TFloat8    f8Long
                            , const TCQCSecToken&       sectUser)
{
    facCQCEventSrv.SetLocInfo(f8Lat, f8Long, sectUser);
}


// ---------------------------------------------------------------------------
//  TEventServerImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TEventServerImpl::Initialize()
{
    // Nothing to do at this time
}


tCIDLib::TVoid TEventServerImpl::Terminate()
{
    // Nothing to do at this time
}


