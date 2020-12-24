//
// FILE NAME: ZWaveLevi2Sh_CmdQ.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/13/2014
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
//  This file provides the implementation for the TZWQCmd class, which is used to
//  queue up commands for later sending, for wakeup type devices.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWQCmd,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TZWQCmd
//  PREFIX: zwopt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWQCmd: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWQCmd::TZWQCmd() :

    m_c4InstId(0)
    , m_eCmdClass(tZWaveLevi2Sh::ECClasses::None)
{
}

TZWQCmd::TZWQCmd(const  tCIDLib::TCard4             c4InstId
                , const tZWaveLevi2Sh::ECClasses    eCmdClass
                , const tCIDLib::TCardList          fcolBytes) :

    m_c4InstId(c4InstId)
    , m_eCmdClass(eCmdClass)
    , m_fcolBytes(fcolBytes)
{
}

TZWQCmd::TZWQCmd(const TZWQCmd& zwqcSrc) :

    m_c4InstId(zwqcSrc.m_c4InstId)
    , m_eCmdClass(zwqcSrc.m_eCmdClass)
    , m_fcolBytes(zwqcSrc.m_fcolBytes)
{
}

TZWQCmd::~TZWQCmd()
{
}


TZWQCmd& TZWQCmd:: operator=(const TZWQCmd& zwqcSrc)
{
    if (&zwqcSrc != this)
    {
        m_c4InstId  = zwqcSrc.m_c4InstId;
        m_eCmdClass = zwqcSrc.m_eCmdClass;
        m_fcolBytes = zwqcSrc.m_fcolBytes;
    }
    return *this;
}


