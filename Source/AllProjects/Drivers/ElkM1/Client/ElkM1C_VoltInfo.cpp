//
// FILE NAME: ElkM1C_VoltInfo.cpp
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
#include    "ElkM1C_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TVoltInfo,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TVoltInfo
//  PREFIX: vi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TVoltInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TVoltInfo::TVoltInfo() :

    m_bUsed(kCIDLib::False)
    , m_f8AVal(0)
    , m_f8BVal(1)
    , m_f8CVal(0)
    , m_f8HighLim(10)
    , m_f8LowLim(0)
{
}

TVoltInfo::TVoltInfo(const TVoltInfo& viToCopy) :

    m_bUsed(viToCopy.m_bUsed)
    , m_f8AVal(viToCopy.m_f8AVal)
    , m_f8BVal(viToCopy.m_f8BVal)
    , m_f8CVal(viToCopy.m_f8CVal)
    , m_f8HighLim(viToCopy.m_f8HighLim)
    , m_f8LowLim(viToCopy.m_f8LowLim)
{
}


// ---------------------------------------------------------------------------
//  TVoltInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid TVoltInfo::operator=(const TVoltInfo& viToAssign)
{
    if (this != &viToAssign)
    {
        m_bUsed     = viToAssign.m_bUsed;
        m_f8AVal    = viToAssign.m_f8AVal;
        m_f8BVal    = viToAssign.m_f8BVal;
        m_f8CVal    = viToAssign.m_f8CVal;
        m_f8HighLim = viToAssign.m_f8HighLim;
        m_f8LowLim  = viToAssign.m_f8LowLim;
    }
}

tCIDLib::TBoolean TVoltInfo::operator==(const TVoltInfo& viToComp) const
{
    return (m_bUsed == viToComp.m_bUsed)
           && (m_f8AVal == viToComp.m_f8AVal)
           && (m_f8BVal == viToComp.m_f8BVal)
           && (m_f8CVal == viToComp.m_f8CVal)
           && (m_f8HighLim == viToComp.m_f8HighLim)
           && (m_f8LowLim == viToComp.m_f8LowLim);
}



// ---------------------------------------------------------------------------
//  TVoltInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TVoltInfo::bUsed() const
{
    return m_bUsed;
}

tCIDLib::TFloat8 TVoltInfo::f8AVal() const
{
    return m_f8AVal;
}

tCIDLib::TFloat8 TVoltInfo::f8BVal() const
{
    return m_f8BVal;
}

tCIDLib::TFloat8 TVoltInfo::f8CVal() const
{
    return m_f8CVal;
}

tCIDLib::TFloat8 TVoltInfo::f8HighLim() const
{
    return m_f8HighLim;
}

tCIDLib::TFloat8 TVoltInfo::f8LowLim() const
{
    return m_f8LowLim;
}


//
//  This has to be the same as what is expected by the server side driver
//  as well since it's used in the formatting out of the config string
//  we send.
//
tCIDLib::TVoid
TVoltInfo::FormatCfg(       TString&        strToFill
                    , const tCIDLib::TCard4 c4Index) const
{
    strToFill.Clear();
    strToFill.AppendFormatted(c4Index);
    strToFill.Append(L"=");
    strToFill.AppendFormatted(m_f8AVal);
    strToFill.Append(L' ');
    strToFill.AppendFormatted(m_f8BVal);
    strToFill.Append(L' ');
    strToFill.AppendFormatted(m_f8CVal);
    strToFill.Append(L' ');
    strToFill.AppendFormatted(m_f8LowLim);
    strToFill.Append(L' ');
    strToFill.AppendFormatted(m_f8HighLim);
}


// Formats the equation stuff for display
tCIDLib::TVoid TVoltInfo::FormatEQ(TString& strToFill) const
{
    strToFill = L"A=";
    strToFill.AppendFormatted(m_f8AVal);
    strToFill.Append(L", B=");
    strToFill.AppendFormatted(m_f8BVal);
    strToFill.Append(L", C=");
    strToFill.AppendFormatted(m_f8CVal);
}


// Formats the limits stuff for display
tCIDLib::TVoid TVoltInfo::FormatLims(TString& strToFill) const
{
    strToFill = L"Low=";
    strToFill.AppendFormatted(m_f8LowLim);
    strToFill.Append(L", High=");
    strToFill.AppendFormatted(m_f8HighLim);
}


// Reset us back to defaults
tCIDLib::TVoid TVoltInfo::Reset()
{
    m_bUsed = kCIDLib::False;
    m_f8AVal = 0;
    m_f8BVal = 1;
    m_f8CVal = 0;
    m_f8LowLim = 0;
    m_f8HighLim = 10;
}


// Let the outside world set us up
tCIDLib::TVoid TVoltInfo::Set(  const   tCIDLib::TBoolean   bUsed
                                , const tCIDLib::TFloat8    f8AVal
                                , const tCIDLib::TFloat8    f8BVal
                                , const tCIDLib::TFloat8    f8CVal
                                , const tCIDLib::TFloat8    f8LowLim
                                , const tCIDLib::TFloat8    f8HighLim)
{
    m_bUsed = bUsed;
    m_f8AVal = f8AVal;
    m_f8BVal = f8BVal;
    m_f8CVal = f8CVal;
    m_f8HighLim = f8HighLim;
    m_f8LowLim = f8LowLim;
}

