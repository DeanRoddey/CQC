//
// FILE NAME: CQCAppCtrl_WndMapItem.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/15/2004
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
//  This file implements a class that represents a particular window within
//  a particular program being controlled.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTrayMon.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TAppCtrlWndRec,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TAppCtrlWndRec
//  PREFIX: acwr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppCtrlWndRec: Public static methods
// ---------------------------------------------------------------------------
const TString& TAppCtrlWndRec::strGetKey(const TAppCtrlWndRec& acwrSrc)
{
    return acwrSrc.m_strPath;
}


// ---------------------------------------------------------------------------
//  TAppCtrlWndRec: Constructors and Destructor
// ---------------------------------------------------------------------------
TAppCtrlWndRec::TAppCtrlWndRec() :

    m_hwndTarget(kCIDCtrls::hwndInvalid)
    , m_widTarget(kCIDCtrls::widInvalid)
{
}

TAppCtrlWndRec::TAppCtrlWndRec( const   TString&                strPath
                                , const TString&                strClass
                                , const tCIDCtrls::TWndId       widTarget
                                , const tCIDCtrls::TWndHandle   hwndTarget) :
    m_hwndTarget(hwndTarget)
    , m_strClass(strClass)
    , m_strPath(strPath)
    , m_widTarget(widTarget)
{
}

TAppCtrlWndRec::TAppCtrlWndRec(const TAppCtrlWndRec& acwrToCopy) :

    m_hwndTarget(acwrToCopy.m_hwndTarget)
    , m_strClass(acwrToCopy.m_strClass)
    , m_strPath(acwrToCopy.m_strPath)
    , m_widTarget(acwrToCopy.m_widTarget)
{
}

TAppCtrlWndRec::~TAppCtrlWndRec()
{
}


// ---------------------------------------------------------------------------
//  TAppCtrlWndRec: Public operators
// ---------------------------------------------------------------------------
TAppCtrlWndRec& TAppCtrlWndRec::operator=(const TAppCtrlWndRec& acwrToAssign)
{
    if (this != &acwrToAssign)
    {
        m_hwndTarget = acwrToAssign.m_hwndTarget;
        m_strClass   = acwrToAssign.m_strClass;
        m_strPath    = acwrToAssign.m_strPath;
        m_widTarget  = acwrToAssign.m_widTarget;
    }
    return *this;
}



// ---------------------------------------------------------------------------
//  TAppCtrlWndRec: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get or set the class
const TString& TAppCtrlWndRec::strClass() const
{
    return m_strClass;
}

const TString& TAppCtrlWndRec::strClass(const TString& strToSet)
{
    m_strClass = strToSet;
    return m_strClass;
}


// Get the path
const TString& TAppCtrlWndRec::strPath() const
{
    return m_strPath;
}


tCIDCtrls::TWndHandle TAppCtrlWndRec::hwndTarget() const
{
    return m_hwndTarget;
}


tCIDLib::TVoid
TAppCtrlWndRec::Set(const   tCIDCtrls::TWndId       widTarget
                    , const tCIDCtrls::TWndHandle   hwndTarget)
{
    m_hwndTarget = hwndTarget;
    m_widTarget  = widTarget;
}


tCIDCtrls::TWndId TAppCtrlWndRec::widTarget() const
{
    return m_widTarget;
}


