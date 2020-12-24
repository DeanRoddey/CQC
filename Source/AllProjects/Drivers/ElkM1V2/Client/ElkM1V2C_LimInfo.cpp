//
// FILE NAME: ElkM1V2C_LimInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/06/2014
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
//  This file implements the class that we use to track the info for those types
//  that have limit values.
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
RTTIDecls(TLimInfo,TItemInfo)



// ---------------------------------------------------------------------------
//   CLASS: TLimInfo
//  PREFIX: vi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLimInfo: Destructor
// ---------------------------------------------------------------------------
TLimInfo::~TLimInfo()
{
}


// ---------------------------------------------------------------------------
//  TLimInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TLimInfo::operator==(const TLimInfo& iiSrc) const
{
    if (!TParent::operator==(iiSrc))
        return kCIDLib::False;

    //
    //  Don't need to compare the signed thing. That's always the same for a
    //  given type.
    //
    return
    (
        (m_i4HighLim == iiSrc.m_i4HighLim)
        && (m_i4LowLim == iiSrc.m_i4LowLim)
    );
}


// ---------------------------------------------------------------------------
//  TLimInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TLimInfo::bComp(const TItemInfo& iiToComp) const
{
    return operator==(static_cast<const TLimInfo&>(iiToComp));
}


tCIDLib::TVoid TLimInfo::FormatCfg(TTextOutStream& strmTar) const
{
    // Call our parent first
    TParent::FormatCfg(strmTar);

    // And format our stuff
    strmTar << L','
            << m_i4LowLim
            << L','
            << m_i4HighLim;
}


tCIDLib::TVoid
TLimInfo::FormatSettings(       TTextOutStream& strmTar
                            , const TStreamFmt&     sfmtName
                            , const TStreamFmt&     sfmtValue) const
{
    TParent::FormatSettings(strmTar, sfmtName, sfmtValue);

    strmTar << sfmtName << L"Low Limit = "
            << sfmtValue << m_i4LowLim << kCIDLib::NewLn

            << sfmtName << L"High Limit = "
            << sfmtValue << m_i4HighLim << kCIDLib::NewLn;
}


tCIDLib::TVoid
TLimInfo::ParseCfg(const tCIDLib::TCard4 c4Id, tCIDLib::TStrList& colTokens)
{
    // Call our parent
    TParent::ParseCfg(c4Id, colTokens);

    if (m_bSignedLims)
    {
        m_i4LowLim = colTokens[0].i4Val();
        m_i4HighLim = colTokens[1].i4Val();
    }
     else
    {
        m_i4LowLim = colTokens[0].c4Val();
        m_i4HighLim = colTokens[1].c4Val();
    }
}



// ---------------------------------------------------------------------------
//  TLimInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TLimInfo::bSignedLims() const
{
    return m_bSignedLims;
}



// ---------------------------------------------------------------------------
//  TLimInfo: Hidden constructors and operators
// ---------------------------------------------------------------------------

TLimInfo::TLimInfo( const   tElkM1V2C::EItemTypes   eType
                    , const tCIDLib::TBoolean       bSignedLims
                    , const tCIDLib::TInt4          i4DefLow
                    , const tCIDLib::TInt4          i4DefHigh) :

    TItemInfo(eType)
    , m_bSignedLims(bSignedLims)
    , m_i4HighLim(i4DefHigh)
    , m_i4LowLim(i4DefLow)
{
}

TLimInfo::TLimInfo(const TLimInfo& iiSrc) :

    TItemInfo(iiSrc)
    , m_bSignedLims(iiSrc.m_bSignedLims)
    , m_i4HighLim(iiSrc.m_i4HighLim)
    , m_i4LowLim(iiSrc.m_i4LowLim)
{
}

tCIDLib::TVoid TLimInfo::operator=(const TLimInfo& iiSrc)
{
    if (this != &iiSrc)
    {
        TParent::operator=(iiSrc);

        m_bSignedLims   = iiSrc.m_bSignedLims;
        m_i4HighLim     = iiSrc.m_i4HighLim;
        m_i4LowLim      = iiSrc.m_i4LowLim;
    }
}

