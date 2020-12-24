//
// FILE NAME: ElkM1V2C_VoltInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/29/2008
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
//  This file implements the class that we use to track the info for a
//  voltage.
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
#include    "ElkM1V2C_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TVoltInfo,TItemInfo)



// ---------------------------------------------------------------------------
//   CLASS: TVoltInfo
//  PREFIX: vi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TVoltInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TVoltInfo::TVoltInfo() :

    TItemInfo(tElkM1V2C::EItemTypes::Volt)
    , m_f8AVal(0)
    , m_f8BVal(1)
    , m_f8CVal(0)
    , m_f8HighLim(10)
    , m_f8LowLim(0)
{
}

TVoltInfo::TVoltInfo(const TVoltInfo& iiSrc) :

    TItemInfo(iiSrc)
    , m_f8AVal(iiSrc.m_f8AVal)
    , m_f8BVal(iiSrc.m_f8BVal)
    , m_f8CVal(iiSrc.m_f8CVal)
    , m_f8HighLim(iiSrc.m_f8HighLim)
    , m_f8LowLim(iiSrc.m_f8LowLim)
{
}


// ---------------------------------------------------------------------------
//  TVoltInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid TVoltInfo::operator=(const TVoltInfo& iiSrc)
{
    if (this != &iiSrc)
    {
        TParent::operator=(iiSrc);

        m_f8AVal    = iiSrc.m_f8AVal;
        m_f8BVal    = iiSrc.m_f8BVal;
        m_f8CVal    = iiSrc.m_f8CVal;
        m_f8HighLim = iiSrc.m_f8HighLim;
        m_f8LowLim  = iiSrc.m_f8LowLim;
    }
}

tCIDLib::TBoolean TVoltInfo::operator==(const TVoltInfo& iiSrc) const
{
    if (!TParent::operator==(iiSrc))
        return kCIDLib::False;

    return
    (
        (m_f8AVal == iiSrc.m_f8AVal)
        && (m_f8BVal == iiSrc.m_f8BVal)
        && (m_f8CVal == iiSrc.m_f8CVal)
        && (m_f8HighLim == iiSrc.m_f8HighLim)
        && (m_f8LowLim == iiSrc.m_f8LowLim)
    );
}


// ---------------------------------------------------------------------------
//  TVoltInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TVoltInfo::bComp(const TItemInfo& iiToComp) const
{
    return operator==(static_cast<const TVoltInfo&>(iiToComp));
}


tCIDLib::TVoid TVoltInfo::FormatCfg(TTextOutStream& strmTar) const
{
    // Call our parent first
    TParent::FormatCfg(strmTar);

    // And format our stuff
    strmTar << L','
            << m_f8AVal
            << L','
            << m_f8BVal
            << L','
            << m_f8CVal
            << L','
            << m_f8LowLim
            << L','
            << m_f8HighLim;
}



tCIDLib::TVoid
TVoltInfo::FormatSettings(          TTextOutStream& strmTar
                            , const TStreamFmt&     sfmtName
                            , const TStreamFmt&     sfmtValue) const
{
    TParent::FormatSettings(strmTar, sfmtName, sfmtValue);

    strmTar << sfmtName << L"A Value = "
            << sfmtValue << m_f8AVal << kCIDLib::NewLn

            << sfmtName << L"B Value = "
            << sfmtValue << m_f8BVal << kCIDLib::NewLn

            << sfmtName << L"C Value = "
            << sfmtValue << m_f8CVal << kCIDLib::NewLn

            << sfmtName << L"Low Limit = "
            << sfmtValue << m_f8LowLim << kCIDLib::NewLn

            << sfmtName << L"High Limit = "
            << sfmtValue << m_f8HighLim << kCIDLib::NewLn;
}



tCIDLib::TVoid
TVoltInfo::ParseCfg(const tCIDLib::TCard4 c4Id, tCIDLib::TStrList& colTokens)
{
    // Call our parent
    TParent::ParseCfg(c4Id, colTokens);

    m_f8AVal    = colTokens[0].f8Val();
    m_f8BVal    = colTokens[1].f8Val();
    m_f8CVal    = colTokens[2].f8Val();
    m_f8LowLim = colTokens[3].f8Val();
    m_f8HighLim  = colTokens[4].f8Val();
}


