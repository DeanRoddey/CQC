//
// FILE NAME: HAIOmniTCPS_Item.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/16/2008
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
//  This file implements the classes that model the named items we query
//  from the Omni and make available for control.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniTCPSh_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(THAIOmniItem,TObject)
RTTIDecls(THAIOmniArea,THAIOmniItem)
RTTIDecls(THAIOmniThermo,THAIOmniItem)
RTTIDecls(THAIOmniUnit,THAIOmniItem)
RTTIDecls(THAIOmniZone,THAIOmniItem)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace HAIOmniTCPSh_Item
{
    // -----------------------------------------------------------------------
    //  The persistent format of the base item class
    // -----------------------------------------------------------------------
    const   tCIDLib::TCard1     c1FmtVerItem = 1;

    // -----------------------------------------------------------------------
    //  The persistent format of the area class
    // -----------------------------------------------------------------------
    const   tCIDLib::TCard1     c1FmtVerArea = 1;

    // -----------------------------------------------------------------------
    //  The persistent format of the thermo class
    // -----------------------------------------------------------------------
    const   tCIDLib::TCard1     c1FmtVerThermo = 1;

    // -----------------------------------------------------------------------
    //  The persistent format of the unit class
    // -----------------------------------------------------------------------
    const   tCIDLib::TCard1     c1FmtVerUnit = 1;

    // -----------------------------------------------------------------------
    //  The persistent format of the Zone class
    // -----------------------------------------------------------------------
    const   tCIDLib::TCard1     c1FmtVerZone = 1;
}



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniItem
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniItem: Public, static methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
THAIOmniItem::QueryDefName( const   tHAIOmniTCPSh::EItemTypes   eType
                            , const tCIDLib::TCard4             c4ItemNum
                            ,       TString&                    strToFill
                            , const tCIDLib::TBoolean           bBrackets)
{
    if (bBrackets)
        strToFill = L"[";
    else
        strToFill.Clear();

    tCIDLib::TCard4 c4NumWidth;
    switch(eType)
    {
        case tHAIOmniTCPSh::EItem_Area :
            c4NumWidth = 1;
            strToFill.Append(L"Area");
            break;

        case tHAIOmniTCPSh::EItem_Button :
            c4NumWidth = 3;
            strToFill.Append(L"Butt");
            break;

        case tHAIOmniTCPSh::EItem_Code :
            c4NumWidth = 2;
            strToFill.Append(L"Code");
            break;

        case tHAIOmniTCPSh::EItem_Message :
            c4NumWidth = 3;
            strToFill.Append(L"Msg");
            break;

        case tHAIOmniTCPSh::EItem_Thermo :
            c4NumWidth = 2;
            strToFill.Append(L"Thermo");
            break;

        case tHAIOmniTCPSh::EItem_Unit :
            c4NumWidth = 3;
            strToFill.Append(L"Unit");
            break;

        case tHAIOmniTCPSh::EItem_Zone :
            c4NumWidth = 3;
            strToFill.Append(L"Zone");
            break;
    };

    // Figure out how many incoming digits we have
    tCIDLib::TCard4 c4InDigs;
    if (c4ItemNum < 10)
        c4InDigs = 1;
    else if (c4ItemNum < 100)
        c4InDigs = 2;
    else
        c4InDigs = 3;

    //
    //  Based on the required width of the number field in the field name,
    //  add some leading zeros if the incoming digits is less than what is
    //  required to fill up the width.
    //
    if (c4NumWidth == 2)
    {
        if (c4InDigs == 1)
            strToFill.Append(L"0");
    }
     else if (c4NumWidth == 3)
    {
        if (c4InDigs == 1)
            strToFill.Append(L"00");
        else if (c4InDigs == 2)
            strToFill.Append(L"0");
    }

    // And append the item number now
    strToFill.AppendFormatted(c4ItemNum);

    if (bBrackets)
        strToFill.Append(L"]");
}


// ---------------------------------------------------------------------------
//  THAIOmniItem: Destructor
// ---------------------------------------------------------------------------
THAIOmniItem::~THAIOmniItem()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniItem: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THAIOmniItem::operator==(const THAIOmniItem& itemSrc) const
{
    if (&itemSrc != this)
    {
        if ((m_bSendTrig != itemSrc.m_bSendTrig)
        ||  (m_c4ItemNum != itemSrc.m_c4ItemNum)
        ||  (m_eType != itemSrc.m_eType)
        ||  (m_strName != itemSrc.m_strName))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean THAIOmniItem::operator!=(const THAIOmniItem& itemSrc) const
{
    return !operator==(itemSrc);
}


// ---------------------------------------------------------------------------
//  THAIOmniItem: Public, virtual methods
// ---------------------------------------------------------------------------

// Say we cannot send triggers unless overidden
tCIDLib::TBoolean THAIOmniItem::bCanSendTrig() const
{
    return kCIDLib::False;
}


tCIDLib::TBoolean THAIOmniItem::bCompare(const THAIOmniItem& itemSrc) const
{
    return (m_bSendTrig == itemSrc.m_bSendTrig)
           && (m_c4ItemNum == itemSrc.m_c4ItemNum)
           && (m_strName == itemSrc.m_strName);
}


// Default is none, unless overridden
tCIDLib::TCard4
THAIOmniItem::c4QueryTypes(tCIDLib::TStrCollect&    colToFill
                            , tCIDLib::TCard4&      c4CurIndex)
{
    c4CurIndex = 0;
    colToFill.RemoveAll();
    return 0;
}


//
//  Format out the display text for this level. Some derived classes will
//  append to this.
//
tCIDLib::TVoid THAIOmniItem::QueryDisplayText(TString& strToFill) const
{
    strToFill = m_strName;
}


tCIDLib::TVoid THAIOmniItem::SetDefName()
{
    //
    //  Call the static helper, passing along our info and have him fill in
    //  our name member.
    //
    QueryDefName(m_eType, m_c4ItemNum, m_strName);
}


// Default is to do nothing unless overridden
tCIDLib::TVoid THAIOmniItem::SetType(const TString&)
{
}


// ---------------------------------------------------------------------------
//  THAIOmniItem: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THAIOmniItem::bSendTrig() const
{
    return m_bSendTrig;
}

tCIDLib::TBoolean
THAIOmniItem::bSendTrig(const tCIDLib::TBoolean bToSet)
{
    m_bSendTrig = bToSet;
    return m_bSendTrig;
}


tCIDLib::TCard4 THAIOmniItem::c4ItemNum() const
{
    return m_c4ItemNum;
}

tHAIOmniTCPSh::EItemTypes THAIOmniItem::eType() const
{
    return m_eType;
}

const TString& THAIOmniItem::strName() const
{
    return m_strName;
}

const TString& THAIOmniItem::strName(const TString& strName)
{
    m_strName = strName;
    return m_strName;
}


// ---------------------------------------------------------------------------
//  THAIOmniItem: Hidden constructors and operators
// ---------------------------------------------------------------------------
THAIOmniItem::THAIOmniItem(const tHAIOmniTCPSh::EItemTypes eType) :

    m_bSendTrig(kCIDLib::True)
    , m_c4ItemNum(0)
    , m_eType(eType)
{
    // Set the default name until set otherwise
    SetDefName();
}

THAIOmniItem::THAIOmniItem( const   TString&                    strName
                            , const tHAIOmniTCPSh::EItemTypes   eType
                            , const tCIDLib::TCard4             c4ItemNum) :

    m_bSendTrig(kCIDLib::True)
    , m_c4ItemNum(c4ItemNum)
    , m_eType(eType)
    , m_strName(strName)
{
    // If the incoming name is empty, set a default for now
    if (strName.bIsEmpty())
        SetDefName();
}

THAIOmniItem::THAIOmniItem(const THAIOmniItem& itemSrc) :

    m_bSendTrig(itemSrc.m_bSendTrig)
    , m_c4ItemNum(itemSrc.m_c4ItemNum)
    , m_eType(itemSrc.m_eType)
    , m_strName(itemSrc.m_strName)
{
}

THAIOmniItem& THAIOmniItem::operator=(const THAIOmniItem& itemSrc)
{
    if (this != &itemSrc)
    {
        m_bSendTrig     = itemSrc.m_bSendTrig;
        m_c4ItemNum     = itemSrc.m_c4ItemNum;
        m_eType         = itemSrc.m_eType;
        m_strName       = itemSrc.m_strName;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  THAIOmniItem: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid THAIOmniItem::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniTCPSh_Item::c1FmtVerItem))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_bSendTrig
                    >> m_c4ItemNum
                    >> m_eType
                    >> m_strName;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid THAIOmniItem::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << HAIOmniTCPSh_Item::c1FmtVerItem
                    << m_bSendTrig
                    << m_c4ItemNum
                    << m_eType
                    << m_strName
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  THAIOmniItem: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid THAIOmniItem::SetName(const TString& strToSet)
{
    m_strName = strToSet;
}




// ---------------------------------------------------------------------------
//   CLASS: THAIOmniArea
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniArea: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniArea::THAIOmniArea() :

    THAIOmniItem(tHAIOmniTCPSh::EItem_Area)
    , m_c4FldId_Alarmed(kCIDLib::c4MaxCard)
    , m_c4FldId_AlarmBits(kCIDLib::c4MaxCard)
    , m_c4FldId_AlarmList(kCIDLib::c4MaxCard)
    , m_c4FldId_ArmStatus(kCIDLib::c4MaxCard)
{
}

THAIOmniArea::THAIOmniArea( const   TString&        strName
                            , const tCIDLib::TCard4 c4ItemNum) :

    THAIOmniItem(strName, tHAIOmniTCPSh::EItem_Area, c4ItemNum)
    , m_c4FldId_Alarmed(kCIDLib::c4MaxCard)
    , m_c4FldId_AlarmBits(kCIDLib::c4MaxCard)
    , m_c4FldId_AlarmList(kCIDLib::c4MaxCard)
    , m_c4FldId_ArmStatus(kCIDLib::c4MaxCard)
{
}

THAIOmniArea::THAIOmniArea(const THAIOmniArea& itemSrc) :

    THAIOmniItem(itemSrc)
    , m_c4FldId_Alarmed(itemSrc.m_c4FldId_Alarmed)
    , m_c4FldId_AlarmBits(itemSrc.m_c4FldId_AlarmBits)
    , m_c4FldId_AlarmList(itemSrc.m_c4FldId_AlarmList)
    , m_c4FldId_ArmStatus(itemSrc.m_c4FldId_ArmStatus)
{
}

THAIOmniArea::~THAIOmniArea()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniArea: Public operators
// ---------------------------------------------------------------------------
THAIOmniArea& THAIOmniArea::operator=(const THAIOmniArea& itemSrc)
{
    if (this != &itemSrc)
    {
        TParent::operator=(itemSrc);
        m_c4FldId_Alarmed   = itemSrc.m_c4FldId_Alarmed;
        m_c4FldId_AlarmBits = itemSrc.m_c4FldId_AlarmBits;
        m_c4FldId_AlarmList = itemSrc.m_c4FldId_AlarmList;
        m_c4FldId_ArmStatus = itemSrc.m_c4FldId_ArmStatus;
    }
    return *this;
}


// Field ids don't matter, so for now just pass to our parent
tCIDLib::TBoolean THAIOmniArea::operator==(const THAIOmniArea& itemSrc) const
{
    return TParent::operator==(itemSrc);
}


tCIDLib::TBoolean THAIOmniArea::operator!=(const THAIOmniArea& itemSrc) const
{
    return TParent::operator!=(itemSrc);
}



// ---------------------------------------------------------------------------
//  THAIOmniArea: Public, inherited methods
// ---------------------------------------------------------------------------

// A polymorphnic comparator
tCIDLib::TBoolean THAIOmniArea::bCompare(const THAIOmniItem& itemSrc) const
{
    // Make sure it' an area. It should be but make sure
    if (!itemSrc.bIsA(THAIOmniArea::clsThis()))
        return kCIDLib::False;

    // Let our parent check his stuff
    if (!TParent::bCompare(itemSrc))
        return kCIDLib::False;

    // Otherwise, we don't have anything at this level to check
    return kCIDLib::True;
}

tCIDLib::TBoolean THAIOmniArea::bOwnsField(const tCIDLib::TCard4 c4FldId) const
{
    return  (c4FldId == m_c4FldId_Alarmed)
            || (c4FldId == m_c4FldId_AlarmBits)
            || (c4FldId == m_c4FldId_AlarmList)
            || (c4FldId == m_c4FldId_ArmStatus);
}


// Polymorphically dup us
THAIOmniItem* THAIOmniArea::pitemDup() const
{
    return new THAIOmniArea(*this);
}


// ---------------------------------------------------------------------------
//  THAIOmniArea: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarmed() const
{
    return m_c4FldId_Alarmed;
}

tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarmed(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId_Alarmed = c4ToSet;
    return m_c4FldId_Alarmed;
}


tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarmBits() const
{
    return m_c4FldId_AlarmBits;
}

tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarmBits(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId_AlarmBits = c4ToSet;
    return m_c4FldId_AlarmBits;
}


tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarmList() const
{
    return m_c4FldId_AlarmList;
}

tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarmList(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId_AlarmList = c4ToSet;
    return m_c4FldId_AlarmList;
}


tCIDLib::TCard4 THAIOmniArea::c4FldIdArmStatus() const
{
    return m_c4FldId_ArmStatus;
}

tCIDLib::TCard4 THAIOmniArea::c4FldIdArmStatus(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId_ArmStatus = c4ToSet;
    return m_c4FldId_ArmStatus;
}


// ---------------------------------------------------------------------------
//  THAIOmniArea: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid THAIOmniArea::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniTCPSh_Item::c1FmtVerArea))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsThis()
        );
    }

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // We don't have anything of our yet

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Init any runtime stuff
    m_c4FldId_Alarmed   = kCIDLib::c4MaxCard;
    m_c4FldId_AlarmBits = kCIDLib::c4MaxCard;
    m_c4FldId_AlarmList = kCIDLib::c4MaxCard;
    m_c4FldId_ArmStatus = kCIDLib::c4MaxCard;
}


tCIDLib::TVoid THAIOmniArea::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << HAIOmniTCPSh_Item::c1FmtVerArea;

    // Stream our parent
    TParent::StreamTo(strmToWriteTo);

    // We don't have any stuff of our own yet

    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}





// ---------------------------------------------------------------------------
//   CLASS: THAIOmniThermo
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniThermo: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniThermo::THAIOmniThermo() :

    THAIOmniItem(tHAIOmniTCPSh::EItem_Thermo)
    , m_c4FldId_CoolSP(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTemp(kCIDLib::c4MaxCard)
    , m_c4FldId_FanMode(kCIDLib::c4MaxCard)
    , m_c4FldId_HeatSP(kCIDLib::c4MaxCard)
    , m_c4FldId_HoldStatus(kCIDLib::c4MaxCard)
    , m_c4FldId_Mode(kCIDLib::c4MaxCard)
{
}

THAIOmniThermo::THAIOmniThermo( const   TString&        strName
                                , const tCIDLib::TCard4 c4ItemNum) :

    THAIOmniItem(strName, tHAIOmniTCPSh::EItem_Thermo, c4ItemNum)
    , m_c4FldId_CoolSP(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTemp(kCIDLib::c4MaxCard)
    , m_c4FldId_FanMode(kCIDLib::c4MaxCard)
    , m_c4FldId_HeatSP(kCIDLib::c4MaxCard)
    , m_c4FldId_HoldStatus(kCIDLib::c4MaxCard)
    , m_c4FldId_Mode(kCIDLib::c4MaxCard)
{
}

THAIOmniThermo::THAIOmniThermo(const THAIOmniThermo& itemSrc) :

    THAIOmniItem(itemSrc)
    , m_c4FldId_CoolSP(itemSrc.m_c4FldId_CoolSP)
    , m_c4FldId_CurTemp(itemSrc.m_c4FldId_CurTemp)
    , m_c4FldId_FanMode(itemSrc.m_c4FldId_FanMode)
    , m_c4FldId_HeatSP(itemSrc.m_c4FldId_HeatSP)
    , m_c4FldId_HoldStatus(itemSrc.m_c4FldId_HoldStatus)
    , m_c4FldId_Mode(itemSrc.m_c4FldId_Mode)
{
}

THAIOmniThermo::~THAIOmniThermo()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniThermo: Public operators
// ---------------------------------------------------------------------------
THAIOmniThermo&
THAIOmniThermo::operator=(const THAIOmniThermo& itemSrc)
{
    if (this != &itemSrc)
    {
        TParent::operator=(itemSrc);
        m_c4FldId_CoolSP     = itemSrc.m_c4FldId_CoolSP;
        m_c4FldId_CurTemp    = itemSrc.m_c4FldId_CurTemp;
        m_c4FldId_FanMode    = itemSrc.m_c4FldId_FanMode;
        m_c4FldId_HeatSP     = itemSrc.m_c4FldId_HeatSP;
        m_c4FldId_HoldStatus = itemSrc.m_c4FldId_HoldStatus;
        m_c4FldId_Mode       = itemSrc.m_c4FldId_Mode;
    }
    return *this;
}


// Field ids don't matter here
tCIDLib::TBoolean THAIOmniThermo::operator==(const THAIOmniThermo& itemSrc) const
{
    return TParent::operator==(itemSrc);
}

tCIDLib::TBoolean THAIOmniThermo::operator!=(const THAIOmniThermo& itemSrc) const
{
    return !TParent::operator==(itemSrc);
}


// ---------------------------------------------------------------------------
//  THAIOmniThermo: Public, inherited methods
// ---------------------------------------------------------------------------

// A polymorphnic comparator
tCIDLib::TBoolean THAIOmniThermo::bCompare(const THAIOmniItem& itemSrc) const
{
    // Make sure it' a thermo. It should be but make sure
    if (!itemSrc.bIsA(THAIOmniThermo::clsThis()))
        return kCIDLib::False;

    // Let our parent check his stuff
    if (!TParent::bCompare(itemSrc))
        return kCIDLib::False;

    // Otherwise, we don't have anything at this level to check
    return kCIDLib::True;
}


tCIDLib::TBoolean
THAIOmniThermo::bOwnsField(const tCIDLib::TCard4 c4FldId) const
{
    return ((c4FldId == m_c4FldId_CoolSP)
         || (c4FldId == m_c4FldId_CurTemp)
         || (c4FldId == m_c4FldId_FanMode)
         || (c4FldId == m_c4FldId_HeatSP)
         || (c4FldId == m_c4FldId_HoldStatus)
         || (c4FldId == m_c4FldId_Mode));
}


// Polymorphically dup us
THAIOmniItem* THAIOmniThermo::pitemDup() const
{
    return new THAIOmniThermo(*this);
}


// ---------------------------------------------------------------------------
//  THAIOmniThermo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 THAIOmniThermo::c4FldIdCoolSP() const
{
    return m_c4FldId_CoolSP;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdCurTemp() const
{
    return m_c4FldId_CurTemp;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdFanMode() const
{
    return m_c4FldId_FanMode;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdHeatSP() const
{
    return m_c4FldId_HeatSP;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdHoldStatus() const
{
    return m_c4FldId_HoldStatus;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdMode() const
{
    return m_c4FldId_Mode;
}


tCIDLib::TVoid
THAIOmniThermo::SetFieldIds(const   tCIDLib::TCard4 c4CoolSP
                            , const tCIDLib::TCard4 c4CurTemp
                            , const tCIDLib::TCard4 c4FanMode
                            , const tCIDLib::TCard4 c4HeatSP
                            , const tCIDLib::TCard4 c4HoldStatus
                            , const tCIDLib::TCard4 c4Mode)
{
    m_c4FldId_CoolSP     = c4CoolSP;
    m_c4FldId_CurTemp    = c4CurTemp;
    m_c4FldId_FanMode    = c4FanMode;
    m_c4FldId_HeatSP     = c4HeatSP;
    m_c4FldId_HoldStatus = c4HoldStatus;
    m_c4FldId_Mode       = c4Mode;
}


// ---------------------------------------------------------------------------
//  THAIOmniThermo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid THAIOmniThermo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniTCPSh_Item::c1FmtVerThermo))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsThis()
        );
    }

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // We don't have anything of our yet

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Init runtime stuff
    m_c4FldId_CoolSP     = kCIDLib::c4MaxCard;
    m_c4FldId_CurTemp    = kCIDLib::c4MaxCard;
    m_c4FldId_FanMode    = kCIDLib::c4MaxCard;
    m_c4FldId_HeatSP     = kCIDLib::c4MaxCard;
    m_c4FldId_HoldStatus = kCIDLib::c4MaxCard;
    m_c4FldId_Mode       = kCIDLib::c4MaxCard;
}


tCIDLib::TVoid THAIOmniThermo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << HAIOmniTCPSh_Item::c1FmtVerThermo;

    // Stream our parent
    TParent::StreamTo(strmToWriteTo);

    // We don't have any stuff of our own yet

    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: THAIOmniUnit
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniUnit::THAIOmniUnit() :

    THAIOmniItem(tHAIOmniTCPSh::EItem_Unit)
    , m_c4FldId_Value(kCIDLib::c4MaxCard)
    , m_eUnitType(tHAIOmniTCPSh::EUnit_Unused)
    , m_strType_Binary(L"Binary")
    , m_strType_Dimmer(L"Dimmer")
    , m_strType_Flag(L"Flag")
{
}

THAIOmniUnit::THAIOmniUnit( const   TString&                    strName
                            , const tCIDLib::TCard4             c4ItemNum
                            , const tHAIOmniTCPSh::EUnitTypes   eUnitType) :

    THAIOmniItem(strName, tHAIOmniTCPSh::EItem_Unit, c4ItemNum)
    , m_c4FldId_Value(kCIDLib::c4MaxCard)
    , m_eUnitType(eUnitType)
    , m_strType_Binary(L"Binary")
    , m_strType_Dimmer(L"Dimmer")
    , m_strType_Flag(L"Flag")
{
}

THAIOmniUnit::THAIOmniUnit(const THAIOmniUnit& itemSrc) :

    THAIOmniItem(itemSrc)
    , m_c4FldId_Value(itemSrc.m_c4FldId_Value)
    , m_eUnitType(itemSrc.m_eUnitType)
    , m_strType_Binary(itemSrc.m_strType_Binary)
    , m_strType_Dimmer(itemSrc.m_strType_Dimmer)
    , m_strType_Flag(itemSrc.m_strType_Flag)
{
}

THAIOmniUnit::~THAIOmniUnit()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniUnit: Public operators
// ---------------------------------------------------------------------------
THAIOmniUnit& THAIOmniUnit::operator=(const THAIOmniUnit& itemSrc)
{
    if (this != &itemSrc)
    {
        TParent::operator=(itemSrc);
        m_c4FldId_Value = itemSrc.m_c4FldId_Value;
        m_eUnitType     = itemSrc.m_eUnitType;
    }
    return *this;
}


// Field ids don't matter here
tCIDLib::TBoolean THAIOmniUnit::operator==(const THAIOmniUnit& itemSrc) const
{
    if (&itemSrc != this)
    {
        if (!TParent::operator==(itemSrc)
        ||  (m_eUnitType != itemSrc.m_eUnitType))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean THAIOmniUnit::operator!=(const THAIOmniUnit& itemSrc) const
{
    return !operator==(itemSrc);
}


// ---------------------------------------------------------------------------
//  THAIOmniUnit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THAIOmniUnit::bCanSendTrig() const
{
    // If we are a binary or flag, we can
    return (m_eUnitType == tHAIOmniTCPSh::EUnit_Binary)
           || (m_eUnitType == tHAIOmniTCPSh::EUnit_Flag);
}


// A polymorphnic comparator
tCIDLib::TBoolean THAIOmniUnit::bCompare(const THAIOmniItem& itemSrc) const
{
    // Make sure it' a unit. It should be but make sure
    if (!itemSrc.bIsA(THAIOmniUnit::clsThis()))
        return kCIDLib::False;

    // Let our parent check his stuff
    if (!TParent::bCompare(itemSrc))
        return kCIDLib::False;

    // Check our stuff
    const THAIOmniUnit& itemSrcUnit = static_cast<const THAIOmniUnit&>(itemSrc);
    return (m_eUnitType == itemSrcUnit.m_eUnitType);
}


tCIDLib::TBoolean
THAIOmniUnit::bOwnsField(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_Value);
}


tCIDLib::TCard4
THAIOmniUnit::c4QueryTypes( tCIDLib::TStrCollect&   colToFill
                            , tCIDLib::TCard4&      c4CurIndex)
{
    colToFill.RemoveAll();
    colToFill.objAdd(m_strType_Binary);
    colToFill.objAdd(m_strType_Dimmer);
    colToFill.objAdd(m_strType_Flag);

    if (m_eUnitType == tHAIOmniTCPSh::EUnit_Binary)
        c4CurIndex = 0;
    else if (m_eUnitType == tHAIOmniTCPSh::EUnit_Dimmer)
        c4CurIndex = 1;
    else if (m_eUnitType == tHAIOmniTCPSh::EUnit_Flag)
        c4CurIndex = 2;
    else
        c4CurIndex = kCIDLib::c4MaxCard;

    return colToFill.c4ElemCount();
}


// Polymorphically dup us
THAIOmniItem* THAIOmniUnit::pitemDup() const
{
    return new THAIOmniUnit(*this);
}


//
//  We add our unit specific type to it
//
tCIDLib::TVoid THAIOmniUnit::QueryDisplayText(TString& strToFill) const
{
    TParent::QueryDisplayText(strToFill);

    strToFill.Append(L" (");
    if (m_eUnitType == tHAIOmniTCPSh::EUnit_Binary)
        strToFill.Append(m_strType_Binary);
    else if (m_eUnitType == tHAIOmniTCPSh::EUnit_Dimmer)
        strToFill.Append(m_strType_Dimmer);
    else if (m_eUnitType == tHAIOmniTCPSh::EUnit_Flag)
        strToFill.Append(m_strType_Flag);
    strToFill.Append(L")");
}


tCIDLib::TVoid THAIOmniUnit::SetType(const TString& strToSet)
{
    if (strToSet == m_strType_Binary)
        m_eUnitType = tHAIOmniTCPSh::EUnit_Binary;
    else if (strToSet == m_strType_Dimmer)
        m_eUnitType = tHAIOmniTCPSh::EUnit_Dimmer;
    else if (strToSet == m_strType_Flag)
        m_eUnitType = tHAIOmniTCPSh::EUnit_Flag;
}



// ---------------------------------------------------------------------------
//  THAIOmniUnit: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 THAIOmniUnit::c4FldIdValue() const
{
    return m_c4FldId_Value;
}

tCIDLib::TCard4 THAIOmniUnit::c4FldIdValue(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId_Value = c4ToSet;
    return m_c4FldId_Value;
}


tHAIOmniTCPSh::EUnitTypes THAIOmniUnit::eUnitType() const
{
    return m_eUnitType;
}


// ---------------------------------------------------------------------------
//  THAIOmniUnit: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid THAIOmniUnit::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniTCPSh_Item::c1FmtVerUnit))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsThis()
        );
    }

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // Stream in our info, which currently is just unit type
    strmToReadFrom >> m_eUnitType;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Init any runtime stuff
    m_c4FldId_Value = kCIDLib::c4MaxCard;
}


tCIDLib::TVoid THAIOmniUnit::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << HAIOmniTCPSh_Item::c1FmtVerUnit;

    // Stream our parent
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_eUnitType
                    << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: THAIOmniZone
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniZone: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniZone::THAIOmniZone() :

    THAIOmniItem(tHAIOmniTCPSh::EItem_Zone)
    , m_c4FldId_Analog(kCIDLib::c4MaxCard)
    , m_c4FldId_Arming(kCIDLib::c4MaxCard)
    , m_c4FldId_CurVal(kCIDLib::c4MaxCard)
    , m_c4FldId_HighSP(kCIDLib::c4MaxCard)
    , m_c4FldId_Latched(kCIDLib::c4MaxCard)
    , m_c4FldId_LowSP(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_eZoneType(tHAIOmniTCPSh::EZone_Unused)
    , m_strType_Alarm(L"Alarm")
    , m_strType_Humidity(L"Humidity")
    , m_strType_Motion(L"Motion")
    , m_strType_Temp(L"Temperature")
{
}

THAIOmniZone::THAIOmniZone( const   TString&                    strName
                            , const tCIDLib::TCard4             c4ItemNum
                            , const tHAIOmniTCPSh::EZoneTypes   eType) :

    THAIOmniItem(strName, tHAIOmniTCPSh::EItem_Zone, c4ItemNum)
    , m_c4FldId_Analog(kCIDLib::c4MaxCard)
    , m_c4FldId_Arming(kCIDLib::c4MaxCard)
    , m_c4FldId_CurVal(kCIDLib::c4MaxCard)
    , m_c4FldId_HighSP(kCIDLib::c4MaxCard)
    , m_c4FldId_Latched(kCIDLib::c4MaxCard)
    , m_c4FldId_LowSP(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_eZoneType(eType)
    , m_strType_Alarm(L"Alarm")
    , m_strType_Humidity(L"Humidity")
    , m_strType_Motion(L"Motion")
    , m_strType_Temp(L"Temperature")
{
}

THAIOmniZone::THAIOmniZone(const THAIOmniZone& itemSrc) :

    THAIOmniItem(itemSrc)
    , m_c4FldId_Analog(itemSrc.m_c4FldId_Analog)
    , m_c4FldId_Arming(itemSrc.m_c4FldId_Arming)
    , m_c4FldId_CurVal(itemSrc.m_c4FldId_CurVal)
    , m_c4FldId_HighSP(itemSrc.m_c4FldId_HighSP)
    , m_c4FldId_Latched(itemSrc.m_c4FldId_Latched)
    , m_c4FldId_LowSP(itemSrc.m_c4FldId_LowSP)
    , m_c4FldId_Status(itemSrc.m_c4FldId_Status)
    , m_eZoneType(itemSrc.m_eZoneType)
    , m_strType_Alarm(itemSrc.m_strType_Alarm)
    , m_strType_Humidity(itemSrc.m_strType_Humidity)
    , m_strType_Motion(itemSrc.m_strType_Motion)
    , m_strType_Temp(itemSrc.m_strType_Temp)
{
}

THAIOmniZone::~THAIOmniZone()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniZone: Public operators
// ---------------------------------------------------------------------------
THAIOmniZone& THAIOmniZone::operator=(const THAIOmniZone& itemSrc)
{
    if (this != &itemSrc)
    {
        TParent::operator=(itemSrc);
        m_c4FldId_Analog    = itemSrc.m_c4FldId_Analog;
        m_c4FldId_Arming    = itemSrc.m_c4FldId_Arming;
        m_c4FldId_CurVal    = itemSrc.m_c4FldId_CurVal;
        m_c4FldId_HighSP    = itemSrc.m_c4FldId_HighSP;
        m_c4FldId_Latched   = itemSrc.m_c4FldId_Latched;
        m_c4FldId_LowSP     = itemSrc.m_c4FldId_LowSP;
        m_c4FldId_Status    = itemSrc.m_c4FldId_Status;
        m_eZoneType         = itemSrc.m_eZoneType;
    }
    return *this;
}


// Field ids don't matter here
tCIDLib::TBoolean THAIOmniZone::operator==(const THAIOmniZone& itemSrc) const
{
    if (&itemSrc != this)
    {
        if (!TParent::operator==(itemSrc)
        ||  (m_eZoneType != itemSrc.m_eZoneType))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean THAIOmniZone::operator!=(const THAIOmniZone& itemSrc) const
{
    return !operator==(itemSrc);
}


// ---------------------------------------------------------------------------
//  THAIOmniZone: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THAIOmniZone::bCanSendTrig() const
{
    // If we are an alarm or motion zone
    return (m_eZoneType == tHAIOmniTCPSh::EZone_Alarm)
           || (m_eZoneType == tHAIOmniTCPSh::EZone_Motion);
}


// A polymorphnic comparator
tCIDLib::TBoolean THAIOmniZone::bCompare(const THAIOmniItem& itemSrc) const
{
    // Make sure it's a zone. It should be but make sure
    if (!itemSrc.bIsA(THAIOmniZone::clsThis()))
        return kCIDLib::False;

    // Let our parent check his stuff
    if (!TParent::bCompare(itemSrc))
        return kCIDLib::False;

    // Check our stuff
    const THAIOmniZone& itemSrcZone = static_cast<const THAIOmniZone&>(itemSrc);
    return (m_eZoneType == itemSrcZone.m_eZoneType);
}


tCIDLib::TBoolean
THAIOmniZone::bOwnsField(const tCIDLib::TCard4 c4FldId) const
{
    return ((c4FldId == m_c4FldId_Analog)
         || (c4FldId == m_c4FldId_Arming)
         || (c4FldId == m_c4FldId_CurVal)
         || (c4FldId == m_c4FldId_HighSP)
         || (c4FldId == m_c4FldId_Latched)
         || (c4FldId == m_c4FldId_LowSP)
         || (c4FldId == m_c4FldId_Status));
}


tCIDLib::TCard4
THAIOmniZone::c4QueryTypes( tCIDLib::TStrCollect&   colToFill
                            , tCIDLib::TCard4&      c4CurIndex)
{
    colToFill.RemoveAll();
    colToFill.objAdd(m_strType_Alarm);
    colToFill.objAdd(m_strType_Humidity);
    colToFill.objAdd(m_strType_Motion);
    colToFill.objAdd(m_strType_Temp);

    if (m_eZoneType == tHAIOmniTCPSh::EZone_Alarm)
        c4CurIndex = 0;
    else if (m_eZoneType == tHAIOmniTCPSh::EZone_Humidity)
        c4CurIndex = 1;
    else if (m_eZoneType == tHAIOmniTCPSh::EZone_Motion)
        c4CurIndex = 2;
    else if (m_eZoneType == tHAIOmniTCPSh::EZone_Temp)
        c4CurIndex = 3;
    else
        c4CurIndex = kCIDLib::c4MaxCard;

    return colToFill.c4ElemCount();
}


// Polymorphically dup us
THAIOmniItem* THAIOmniZone::pitemDup() const
{
    return new THAIOmniZone(*this);
}


//
//  We add our unit specific type to it
//
tCIDLib::TVoid THAIOmniZone::QueryDisplayText(TString& strToFill) const
{
    TParent::QueryDisplayText(strToFill);

    strToFill.Append(L" (");
    if (m_eZoneType == tHAIOmniTCPSh::EZone_Alarm)
        strToFill.Append(m_strType_Alarm);
    else if (m_eZoneType == tHAIOmniTCPSh::EZone_Humidity)
        strToFill.Append(m_strType_Humidity);
    else if (m_eZoneType == tHAIOmniTCPSh::EZone_Motion)
        strToFill.Append(m_strType_Motion);
    else if (m_eZoneType == tHAIOmniTCPSh::EZone_Temp)
        strToFill.Append(m_strType_Temp);

    strToFill.Append(L")");
}


tCIDLib::TVoid THAIOmniZone::SetType(const TString& strToSet)
{
    if (strToSet == m_strType_Alarm)
        m_eZoneType = tHAIOmniTCPSh::EZone_Alarm;
    else if (strToSet == m_strType_Humidity)
        m_eZoneType = tHAIOmniTCPSh::EZone_Humidity;
    else if (strToSet == m_strType_Motion)
        m_eZoneType = tHAIOmniTCPSh::EZone_Motion;
    else if (strToSet == m_strType_Temp)
        m_eZoneType = tHAIOmniTCPSh::EZone_Temp;
}


// ---------------------------------------------------------------------------
//  THAIOmniZone: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 THAIOmniZone::c4FldIdAnalog() const
{
    return m_c4FldId_Analog;
}


tCIDLib::TCard4 THAIOmniZone::c4FldIdArming() const
{
    return m_c4FldId_Arming;
}


tCIDLib::TCard4 THAIOmniZone::c4FldIdCurVal() const
{
    return m_c4FldId_CurVal;
}


tCIDLib::TCard4 THAIOmniZone::c4FldIdHighSP() const
{
    return m_c4FldId_HighSP;
}


tCIDLib::TCard4 THAIOmniZone::c4FldIdLatched() const
{
    return m_c4FldId_Latched;
}


tCIDLib::TCard4 THAIOmniZone::c4FldIdLowSP() const
{
    return m_c4FldId_LowSP;
}


tCIDLib::TCard4 THAIOmniZone::c4FldIdStatus() const
{
    return m_c4FldId_Status;
}


tHAIOmniTCPSh::EZoneTypes THAIOmniZone::eZoneType() const
{
    return m_eZoneType;
}


tCIDLib::TVoid
THAIOmniZone::SetFieldIds(  const   tCIDLib::TCard4 c4Analog
                            , const tCIDLib::TCard4 c4Arming
                            , const tCIDLib::TCard4 c4CurVal
                            , const tCIDLib::TCard4 c4High
                            , const tCIDLib::TCard4 c4Latched
                            , const tCIDLib::TCard4 c4Low
                            , const tCIDLib::TCard4 c4Status)
{
    m_c4FldId_Analog    = c4Analog;
    m_c4FldId_Arming    = c4Arming;
    m_c4FldId_CurVal    = c4CurVal;
    m_c4FldId_HighSP    = c4High;
    m_c4FldId_Latched   = c4Latched;
    m_c4FldId_LowSP     = c4Low;
    m_c4FldId_Status    = c4Status;
}


// ---------------------------------------------------------------------------
//  THAIOmniZone: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid THAIOmniZone::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniTCPSh_Item::c1FmtVerZone))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsThis()
        );
    }

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // Stream in our info, which currently is just zone type
    strmToReadFrom >> m_eZoneType;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Default runtime stuff
    m_c4FldId_Analog  = kCIDLib::c4MaxCard;
    m_c4FldId_Arming  = kCIDLib::c4MaxCard;
    m_c4FldId_CurVal  = kCIDLib::c4MaxCard;
    m_c4FldId_HighSP  = kCIDLib::c4MaxCard;
    m_c4FldId_Latched = kCIDLib::c4MaxCard;
    m_c4FldId_LowSP   = kCIDLib::c4MaxCard;
    m_c4FldId_Status  = kCIDLib::c4MaxCard;
}


tCIDLib::TVoid THAIOmniZone::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << HAIOmniTCPSh_Item::c1FmtVerZone;

    // Let our parent stream
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_eZoneType
                    << tCIDLib::EStreamMarkers::EndObject;
}


