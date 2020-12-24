//
// FILE NAME: CQCTrayMon_ListChangeEvent.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/15/2004
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
//  This file implements the TAppListChangeEv class. Objects of this class
//  are put into a queue managed by the main frame window, by the protocol
//  server object, to tell the window about changes that it should display.
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
#include    "CQCTrayMon.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TAppListChangeEv,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TAppListChangeEv
//  PREFIX: ace
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppListChangeEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TAppListChangeEv::TAppListChangeEv() :

    m_eEvent(tCQCTrayMon::EListEvs::None)
{
}

TAppListChangeEv::TAppListChangeEv( const   TString&                strMoniker
                                    , const TString&                strTitle
                                    , const tCQCTrayMon::EListEvs   eEvent) :
    m_eEvent(eEvent)
    , m_strMoniker(strMoniker)
    , m_strTitle(strTitle)
{
}

TAppListChangeEv::TAppListChangeEv( const   TString&                strMoniker
                                    , const tCQCTrayMon::EListEvs   eEvent) :
    m_eEvent(eEvent)
    , m_strMoniker(strMoniker)
{
}

TAppListChangeEv::TAppListChangeEv(const TString& strStatusMsg) :

    m_eEvent(tCQCTrayMon::EListEvs::Status)
    , m_strTitle(strStatusMsg)
{
}

TAppListChangeEv::TAppListChangeEv(const TAppListChangeEv& aceToCopy) :

    m_eEvent(aceToCopy.m_eEvent)
    , m_strMoniker(aceToCopy.m_strMoniker)
    , m_strTitle(aceToCopy.m_strTitle)
{
}

TAppListChangeEv::~TAppListChangeEv()
{
}


// ---------------------------------------------------------------------------
//  TAppListChangeEv: Public operators
// ---------------------------------------------------------------------------
TAppListChangeEv&
TAppListChangeEv::operator=(const TAppListChangeEv& aceToAssign)
{
    if (this != &aceToAssign)
    {
        m_eEvent     = aceToAssign.m_eEvent;
        m_strMoniker = aceToAssign.m_strMoniker;
        m_strTitle   = aceToAssign.m_strTitle;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TAppListChangeEv: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCTrayMon::EListEvs TAppListChangeEv::eEvent() const
{
    return m_eEvent;
}


const TString& TAppListChangeEv::strMoniker() const
{
    return m_strMoniker;
}


const TString& TAppListChangeEv::strMsgText() const
{
    //
    //  We use the title member for passing the event text in status
    //  message events.
    //
    return m_strTitle;
}


const TString& TAppListChangeEv::strTitle() const
{
    return m_strTitle;
}


