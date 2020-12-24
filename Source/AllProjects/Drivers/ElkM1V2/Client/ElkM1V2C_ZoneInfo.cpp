//
// FILE NAME: ElkM1V2C_ZoneInfo.cpp
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
//  This file implements the class that we use to track the info for a
//  zone.
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
RTTIDecls(TZoneInfo,TItemInfo)



// ---------------------------------------------------------------------------
//   CLASS: TZoneInfo
//  PREFIX: vi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZoneInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TZoneInfo::TZoneInfo() :

    TItemInfo(tElkM1V2C::EItemTypes::Zone)
    , m_bSendTrigs(kCIDLib::False)
    , m_eType(tElkM1V2C::EZoneTypes::Security)
{
}

TZoneInfo::TZoneInfo(const TZoneInfo& iiSrc) :

    TItemInfo(iiSrc)
    , m_bSendTrigs(iiSrc.m_bSendTrigs)
    , m_eType(iiSrc.m_eType)
{
}


// ---------------------------------------------------------------------------
//  TZoneInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZoneInfo::operator=(const TZoneInfo& iiSrc)
{
    if (this != &iiSrc)
    {
        TParent::operator=(iiSrc);
        m_bSendTrigs = iiSrc.m_bSendTrigs;
        m_eType = iiSrc.m_eType;
    }
}

tCIDLib::TBoolean TZoneInfo::operator==(const TZoneInfo& iiSrc) const
{
    if (!TParent::operator==(iiSrc))
        return kCIDLib::False;

    return
    (
        (m_bSendTrigs == iiSrc.m_bSendTrigs)
        && (m_eType == iiSrc.m_eType)
    );
}


// ---------------------------------------------------------------------------
//  TZoneInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZoneInfo::bComp(const TItemInfo& iiToComp) const
{
    return operator==(static_cast<const TZoneInfo&>(iiToComp));
}


tCIDLib::TVoid TZoneInfo::FormatCfg(TTextOutStream& strmTar) const
{
    // Call our parent first
    TParent::FormatCfg(strmTar);

    // And format our stuff
    strmTar << L',';

    if (m_eType == tElkM1V2C::EZoneTypes::Motion)
        strmTar << L"Mot";
    else
        strmTar << L"Sec";

    strmTar << L','
            << (m_bSendTrigs ? L"Trig" : L"NoTrig");
}


tCIDLib::TVoid
TZoneInfo::FormatSettings(          TTextOutStream& strmTar
                            , const TStreamFmt&     sfmtName
                            , const TStreamFmt&     sfmtValue) const
{
    TParent::FormatSettings(strmTar, sfmtName, sfmtValue);

    strmTar << sfmtName << L"Triggers = "
            << sfmtValue << facCQCKit().strBoolYesNo(m_bSendTrigs)
            << kCIDLib::NewLn

            << sfmtName << L"Type = "
            << sfmtValue;

    switch(m_eType)
    {
        case tElkM1V2C::EZoneTypes::Motion :
            strmTar << L"Motion";
            break;

        case tElkM1V2C::EZoneTypes::Security :
            strmTar << L"Security";
            break;

        default :
            strmTar << L"???";
            break;
    };

    strmTar << kCIDLib::NewLn;
}


tCIDLib::TVoid
TZoneInfo::ParseCfg(const tCIDLib::TCard4 c4Id, tCIDLib::TStrList& colTokens)
{
    // Call our parent
    TParent::ParseCfg(c4Id, colTokens);

    if (colTokens[0].bCompareI(L"Mot"))
        m_eType =  tElkM1V2C::EZoneTypes::Motion;
    else
        m_eType = tElkM1V2C::EZoneTypes::Security;

    m_bSendTrigs = colTokens[1] == L"Trig";
}


