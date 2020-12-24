//
// FILE NAME: CQCHostMonC_VolInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/25/2001
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
//  This file implements the TCQCVolInfo class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCHostMonC.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCVolPollInfo,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TCQCVolPollInfo
//  PREFIX: vpi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCVolPollInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCVolPollInfo::TCQCVolPollInfo() :

    m_c4PercentUsed(0)
    , m_c4PercentUsed2(0)
    , m_c4TotalMB(0)
{
}

TCQCVolPollInfo::TCQCVolPollInfo(const TCQCVolPollInfo& vpiToCopy) :

    m_c4PercentUsed(vpiToCopy.m_c4PercentUsed)
    , m_c4PercentUsed2(vpiToCopy.m_c4PercentUsed2)
    , m_c4TotalMB(vpiToCopy.m_c4TotalMB)
    , m_strVolume(vpiToCopy.m_strVolume)
{
}

TCQCVolPollInfo::TCQCVolPollInfo(const TString& strInitValue) :

    m_c4PercentUsed(0)
    , m_c4PercentUsed2(0)
    , m_c4TotalMB(0)
{
    ParseValue(strInitValue);
}

TCQCVolPollInfo::~TCQCVolPollInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCVolPollInfo: Public operators
// ---------------------------------------------------------------------------
TCQCVolPollInfo& TCQCVolPollInfo::operator=(const TCQCVolPollInfo& vpiToAssign)
{
    if (this == &vpiToAssign)
        return *this;

    m_c4PercentUsed  = vpiToAssign.m_c4PercentUsed;
    m_c4PercentUsed2 = vpiToAssign.m_c4PercentUsed2;
    m_c4TotalMB      = vpiToAssign.m_c4TotalMB;
    m_strVolume      = vpiToAssign.m_strVolume;

    return *this;
}


tCIDLib::TBoolean
TCQCVolPollInfo::operator==(const TCQCVolPollInfo& vpiToCompare) const
{
    if (this == &vpiToCompare)
        return kCIDLib::True;

    // <TBD> Compare 2 version?

    if ((m_c4PercentUsed != vpiToCompare.m_c4PercentUsed)
    ||  (m_c4TotalMB != vpiToCompare.m_c4TotalMB))
    {
        return kCIDLib::False;
    }
    return (m_strVolume == vpiToCompare.m_strVolume);
}


// ---------------------------------------------------------------------------
//  TCQCVolPollInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCVolPollInfo::bChanged() const
{
    return m_c4PercentUsed != m_c4PercentUsed2;
}


tCIDLib::TCard4 TCQCVolPollInfo::c4PercentUsed() const
{
    return m_c4PercentUsed;
}

tCIDLib::TCard4 TCQCVolPollInfo::c4PercentUsed(const tCIDLib::TCard4 c4ToSet)
{
    m_c4PercentUsed = c4ToSet;
    return m_c4PercentUsed;
}

tCIDLib::TCard4 TCQCVolPollInfo::c4PercentUsed2() const
{
    return m_c4PercentUsed2;
}

tCIDLib::TCard4 TCQCVolPollInfo::c4PercentUsed2(const tCIDLib::TCard4 c4ToSet)
{
    m_c4PercentUsed2 = c4ToSet;
    return m_c4PercentUsed2;
}


tCIDLib::TCard4 TCQCVolPollInfo::c4TotalMB() const
{
    return m_c4TotalMB;
}


tCIDLib::TVoid TCQCVolPollInfo::ParseValue(const TString& strNewValue)
{
    // Get a tokenizer, with the separate being a space or comma
    TStringTokenizer stokParse(&strNewValue, L" ,");

    TString strTotal;
    TString strFree;
    if (!stokParse.bGetNextToken(m_strVolume)
    ||  !stokParse.bGetNextToken(strTotal)
    ||  !stokParse.bGetNextToken(strFree))
    {
        m_strVolume.Append(L" ???");
        m_c4TotalMB = 0;
        m_c4PercentUsed2 = 0;
    }
     else
    {
        // Convert our two values and calcuate the used amount
        m_c4TotalMB = strTotal.c4Val();
        tCIDLib::TCard4 c4FreeMB = strFree.c4Val();

        tCIDLib::TFloat8 f8TmpPer
        (
            (tCIDLib::TFloat8(c4FreeMB) / tCIDLib::TFloat8(m_c4TotalMB)) * 100.0
        );
        f8TmpPer = 100.0 - f8TmpPer;
        if (f8TmpPer < 0)
            f8TmpPer = 0;
        else if (f8TmpPer > 100)
            f8TmpPer = 100;
        m_c4PercentUsed2 = tCIDLib::TCard4(f8TmpPer);
    }
}


const TString& TCQCVolPollInfo::strVolume() const
{
    return m_strVolume;
}

const TString& TCQCVolPollInfo::strVolume(const TString& strToSet)
{
    m_strVolume = strToSet;
    return m_strVolume;
}


