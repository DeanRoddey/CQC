//
// FILE NAME: HAIOmniTCPS2_Item.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/06/2014
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
#include    "HAIOmniTCP2Sh_.hpp"



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
namespace HAIOmniTCP2Sh_Item
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
THAIOmniItem::QueryDefName( const   tHAIOmniTCP2Sh::EItemTypes  eType
                            , const tCIDLib::TCard2             c2ItemNum
                            ,       TString&                    strName
                            , const tCIDLib::TBoolean           bBrackets)
{
    if (bBrackets)
        strName = L"[";
    else
        strName.Clear();

    switch(eType)
    {
        case tHAIOmniTCP2Sh::EItemTypes::Area :
            strName.Append(L"A");
            break;

        case tHAIOmniTCP2Sh::EItemTypes::Button :
            strName.Append(L"B");
            break;

        case tHAIOmniTCP2Sh::EItemTypes::Code :
            strName.Append(L"C");
            break;

        case tHAIOmniTCP2Sh::EItemTypes::Msg :
            strName.Append(L"M");
            break;

        case tHAIOmniTCP2Sh::EItemTypes::Thermo :
            strName.Append(L"T");
            break;

        case tHAIOmniTCP2Sh::EItemTypes::Unit :
            strName.Append(L"U");
            break;

        case tHAIOmniTCP2Sh::EItemTypes::Zone :
            strName.Append(L"Z");
            break;
    };

    // And append the item number now
    strName.AppendFormatted(c2ItemNum);

    if (bBrackets)
        strName.Append(L"]");
}


// ---------------------------------------------------------------------------
//  THAIOmniItem: Destructor
// ---------------------------------------------------------------------------
THAIOmniItem::~THAIOmniItem()
{
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
           && (m_c2ItemNum == itemSrc.m_c2ItemNum)
           && (m_strName == itemSrc.m_strName);
}


// If not overridden say no
tCIDLib::TBoolean THAIOmniItem::bHasSubType(TString&) const
{
    return kCIDLib::False;
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


// If not overridden, nothing to do
tCIDLib::TVoid THAIOmniItem::Finalize(TCQCServerBase&)
{
}


tCIDLib::TVoid THAIOmniItem::SetDefName()
{
    //
    //  Call the static helper, passing along our info and have him fill in
    //  our name member.
    //
    QueryDefName(m_eType, m_c2ItemNum, m_strName);
}


// Default is to do nothing unless overridden
tCIDLib::TVoid THAIOmniItem::SetSubType(const TString&)
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


tCIDLib::TCard2 THAIOmniItem::c2ItemNum() const
{
    return m_c2ItemNum;
}

tHAIOmniTCP2Sh::EItemTypes THAIOmniItem::eType() const
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
THAIOmniItem::THAIOmniItem(const tHAIOmniTCP2Sh::EItemTypes eType) :

    m_bSendTrig(kCIDLib::True)
    , m_c2ItemNum(0)
    , m_eType(eType)
{
    // Set the default name until set otherwise
    SetDefName();
}

THAIOmniItem::THAIOmniItem( const   TString&                    strName
                            , const tHAIOmniTCP2Sh::EItemTypes  eType
                            , const tCIDLib::TCard2             c2ItemNum) :

    m_bSendTrig(kCIDLib::True)
    , m_c2ItemNum(c2ItemNum)
    , m_eType(eType)
    , m_strName(strName)
{
}

THAIOmniItem::THAIOmniItem(const THAIOmniItem& itemSrc) :

    m_bSendTrig(itemSrc.m_bSendTrig)
    , m_c2ItemNum(itemSrc.m_c2ItemNum)
    , m_eType(itemSrc.m_eType)
    , m_strName(itemSrc.m_strName)
{
}

THAIOmniItem& THAIOmniItem::operator=(const THAIOmniItem& itemSrc)
{
    if (this != &itemSrc)
    {
        m_bSendTrig     = itemSrc.m_bSendTrig;
        m_c2ItemNum     = itemSrc.m_c2ItemNum;
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
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniTCP2Sh_Item::c1FmtVerItem))
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
                    >> m_c2ItemNum
                    >> m_eType
                    >> m_strName;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid THAIOmniItem::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << HAIOmniTCP2Sh_Item::c1FmtVerItem
                    << m_bSendTrig
                    << m_c2ItemNum
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

    THAIOmniItem(tHAIOmniTCP2Sh::EItemTypes::Area)
    , m_c4FldId_Alarms(kCIDLib::c4MaxCard)
    , m_c4FldId_Arming(kCIDLib::c4MaxCard)
    , m_c4FldId_ArmMode(kCIDLib::c4MaxCard)
    , m_c4FldId_InAlarm(kCIDLib::c4MaxCard)
{
}

THAIOmniArea::THAIOmniArea( const   TString&        strName
                            , const tCIDLib::TCard2 c2ItemNum) :

    THAIOmniItem(strName, tHAIOmniTCP2Sh::EItemTypes::Area, c2ItemNum)
    , m_c4FldId_InAlarm(kCIDLib::c4MaxCard)
    , m_c4FldId_Alarms(kCIDLib::c4MaxCard)
    , m_c4FldId_ArmMode(kCIDLib::c4MaxCard)
{
}

THAIOmniArea::THAIOmniArea(const THAIOmniArea& itemSrc) :

    THAIOmniItem(itemSrc)
    , m_c4FldId_Alarms(itemSrc.m_c4FldId_Alarms)
    , m_c4FldId_Arming(itemSrc.m_c4FldId_Arming)
    , m_c4FldId_ArmMode(itemSrc.m_c4FldId_ArmMode)
    , m_c4FldId_InAlarm(itemSrc.m_c4FldId_InAlarm)
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
        m_c4FldId_Alarms    = itemSrc.m_c4FldId_Alarms;
        m_c4FldId_Arming    = itemSrc.m_c4FldId_Arming;
        m_c4FldId_ArmMode   = itemSrc.m_c4FldId_ArmMode;
        m_c4FldId_InAlarm   = itemSrc.m_c4FldId_InAlarm;
    }
    return *this;
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
    return  (c4FldId == m_c4FldId_Alarms)
            || (c4FldId == m_c4FldId_Arming)
            || (c4FldId == m_c4FldId_ArmMode)
            || (c4FldId == m_c4FldId_InAlarm);
}


tCIDLib::TCard4
THAIOmniArea::c4QueryFldDefs(tCQCKit::TFldDefList& colAddTo
                            , TCQCFldDef&
                            , const tCIDLib::TBoolean) const
{
    // Just pass it on to the class helper since we only do V2 compliant fields here
    TString strArmLims(128UL);
    tHAIOmniTCP2Sh::FormatEArmModes(strArmLims, L"Enum: ", kCIDLib::chComma);

    return TCQCDevClass::c4LoadSecAreaFlds(colAddTo, strName(), strArmLims);
}


// Polymorphically dup us
THAIOmniItem* THAIOmniArea::pitemDup() const
{
    return new THAIOmniArea(*this);
}


tCIDLib::TVoid
THAIOmniArea::QueryFirstLastId(tCIDLib::TCard4& c4First, tCIDLib::TCard4& c4Last) const
{
    tCIDLib::TCardList fcolIds(4UL);
    fcolIds.c4AddElement(m_c4FldId_Alarms);
    fcolIds.c4AddElement(m_c4FldId_Arming);
    fcolIds.c4AddElement(m_c4FldId_ArmMode);
    fcolIds.c4AddElement(m_c4FldId_InAlarm);

    fcolIds.Sort(tCIDLib::eComp<tCIDLib::TCard4>);

    c4First = fcolIds[0];
    c4Last = fcolIds[fcolIds.c4ElemCount() - 1];
}


tCIDLib::TVoid THAIOmniArea::StoreFldIds(const TCQCServerBase& sdrvOwner)
{
    m_c4FldId_Alarms  = sdrvOwner.flddFind(tCQCKit::EDevClasses::Security, L"Area", strName(), L"Alarms").c4Id();
    m_c4FldId_Arming = sdrvOwner.flddFind(tCQCKit::EDevClasses::Security, L"Area", strName(), L"Arming").c4Id();
    m_c4FldId_ArmMode = sdrvOwner.flddFind(tCQCKit::EDevClasses::Security, L"Area", strName(), L"ArmMode").c4Id();
    m_c4FldId_InAlarm = sdrvOwner.flddFind(tCQCKit::EDevClasses::Security, L"Area", strName(), L"InAlarm").c4Id();
}


// ---------------------------------------------------------------------------
//  THAIOmniArea: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarms() const
{
    return m_c4FldId_Alarms;
}

tCIDLib::TCard4 THAIOmniArea::c4FldIdArming() const
{
    return m_c4FldId_Arming;
}

tCIDLib::TCard4 THAIOmniArea::c4FldIdArmMode() const
{
    return m_c4FldId_ArmMode;
}

tCIDLib::TCard4 THAIOmniArea::c4FldIdInAlarm() const
{
    return m_c4FldId_InAlarm;
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
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniTCP2Sh_Item::c1FmtVerArea))
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
    m_c4FldId_InAlarm   = kCIDLib::c4MaxCard;
    m_c4FldId_Alarms = kCIDLib::c4MaxCard;
    m_c4FldId_Arming = kCIDLib::c4MaxCard;
    m_c4FldId_ArmMode = kCIDLib::c4MaxCard;
}


tCIDLib::TVoid THAIOmniArea::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << HAIOmniTCP2Sh_Item::c1FmtVerArea;

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

    THAIOmniItem(tHAIOmniTCP2Sh::EItemTypes::Thermo)
    , m_c4FldId_CurTemp(kCIDLib::c4MaxCard)
    , m_c4FldId_FanMode(kCIDLib::c4MaxCard)
    , m_c4FldId_FanOpMode(kCIDLib::c4MaxCard)
    , m_c4FldId_HighSP(kCIDLib::c4MaxCard)
    , m_c4FldId_HoldStatus(kCIDLib::c4MaxCard)
    , m_c4FldId_LowSP(kCIDLib::c4MaxCard)
    , m_c4FldId_Mode(kCIDLib::c4MaxCard)
    , m_c4FldId_OpMode(kCIDLib::c4MaxCard)
    , m_eThermoType(tHAIOmniTCP2Sh::EThermoTypes::Unused)
{
}

THAIOmniThermo::THAIOmniThermo( const   TString&                    strName
                                , const tCIDLib::TCard2             c2ItemNum
                                , const tHAIOmniTCP2Sh::EThermoTypes eType) :

    THAIOmniItem(strName, tHAIOmniTCP2Sh::EItemTypes::Thermo, c2ItemNum)
    , m_c4FldId_CurTemp(kCIDLib::c4MaxCard)
    , m_c4FldId_FanMode(kCIDLib::c4MaxCard)
    , m_c4FldId_FanOpMode(kCIDLib::c4MaxCard)
    , m_c4FldId_HighSP(kCIDLib::c4MaxCard)
    , m_c4FldId_HoldStatus(kCIDLib::c4MaxCard)
    , m_c4FldId_LowSP(kCIDLib::c4MaxCard)
    , m_c4FldId_Mode(kCIDLib::c4MaxCard)
    , m_c4FldId_OpMode(kCIDLib::c4MaxCard)
    , m_eThermoType(eType)
{
}

THAIOmniThermo::THAIOmniThermo(const THAIOmniThermo& itemSrc) :

    THAIOmniItem(itemSrc)
    , m_c4FldId_CurTemp(itemSrc.m_c4FldId_CurTemp)
    , m_c4FldId_FanMode(itemSrc.m_c4FldId_FanMode)
    , m_c4FldId_FanOpMode(itemSrc.m_c4FldId_FanMode)
    , m_c4FldId_HighSP(itemSrc.m_c4FldId_HighSP)
    , m_c4FldId_HoldStatus(itemSrc.m_c4FldId_HoldStatus)
    , m_c4FldId_LowSP(itemSrc.m_c4FldId_LowSP)
    , m_c4FldId_Mode(itemSrc.m_c4FldId_Mode)
    , m_c4FldId_OpMode(itemSrc.m_c4FldId_Mode)
    , m_eThermoType(itemSrc.m_eThermoType)
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

        m_c4FldId_CurTemp    = itemSrc.m_c4FldId_CurTemp;
        m_c4FldId_FanMode    = itemSrc.m_c4FldId_FanMode;
        m_c4FldId_FanOpMode  = itemSrc.m_c4FldId_FanOpMode;
        m_c4FldId_HighSP     = itemSrc.m_c4FldId_HighSP;
        m_c4FldId_HoldStatus = itemSrc.m_c4FldId_HoldStatus;
        m_c4FldId_LowSP      = itemSrc.m_c4FldId_LowSP;
        m_c4FldId_Mode       = itemSrc.m_c4FldId_Mode;
        m_c4FldId_OpMode     = itemSrc.m_c4FldId_OpMode;
        m_eThermoType        = itemSrc.m_eThermoType;
    }
    return *this;
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

    // Make sure they are the same thermo type
    const THAIOmniThermo& itemSrcThermo = static_cast<const THAIOmniThermo&>(itemSrc);
    return (m_eThermoType == itemSrcThermo.m_eThermoType);
}


tCIDLib::TBoolean
THAIOmniThermo::bOwnsField(const tCIDLib::TCard4 c4FldId) const
{
    return ((c4FldId == m_c4FldId_CurTemp)
           || (c4FldId == m_c4FldId_FanMode)
           || (c4FldId == m_c4FldId_FanOpMode)
           || (c4FldId == m_c4FldId_HighSP)
           || (c4FldId == m_c4FldId_HoldStatus)
           || (c4FldId == m_c4FldId_LowSP)
           || (c4FldId == m_c4FldId_Mode)
           || (c4FldId == m_c4FldId_OpMode));
}


tCIDLib::TCard4
THAIOmniThermo::c4QueryFldDefs(         tCQCKit::TFldDefList&   colAddTo
                                ,       TCQCFldDef&             flddTmp
                                , const tCIDLib::TBoolean       bCelsius) const
{
    // Set up the various limit strings we need
    TString strFanLim;
    tHAIOmniTCP2Sh::FormatEFanModes(strFanLim, L"Enum: ", kCIDLib::chComma);

    TString strModeLim;
    tHAIOmniTCP2Sh::FormatEThermoModes(strModeLim, L"Enum: ", kCIDLib::chComma);

    TString strTempLim;
    TString strSPLim;
    if (bCelsius)
    {
        strSPLim = L"Range: -18, 50";
        strTempLim = L"Range: -40, 87";
    }
     else
    {
        strSPLim = L"Range: 0, 122";
        strTempLim = L"Range: -40, 189";
    }

    // Do the non-V2 hold status
    TString strFldName = strName();
    strFldName.Append(L"_Hold");
    flddTmp.Set(strFldName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::ReadWrite);
    colAddTo.objAdd(flddTmp);

    // And the helper will do the V2 ones
    return TCQCDevClass::c4LoadThermoFlds
    (
        colAddTo
        , strName()
        , strTempLim
        , strSPLim
        , strSPLim
        , strFanLim
        , TString::strEmpty()
        , strModeLim
        , TString::strEmpty()
    );
}


//
//  This is called after connection and fields are initialized. For us, it gives us a
//  chance to the mode setting fields initialized.
//
tCIDLib::TVoid THAIOmniThermo::Finalize(TCQCServerBase& sdrvOwn)
{
    // Store the current fan mode as the operating fan mode
    tCIDLib::TCard4 c4SerialNum= 0;
    TString strVal;
    if (sdrvOwn.bReadStringFld(m_c4FldId_FanOpMode, c4SerialNum, strVal, kCIDLib::False))
        sdrvOwn.bStoreStringFld(m_c4FldId_FanMode, strVal, kCIDLib::True);
}


// Polymorphically dup us
THAIOmniItem* THAIOmniThermo::pitemDup() const
{
    return new THAIOmniThermo(*this);
}


tCIDLib::TVoid
THAIOmniThermo::QueryFirstLastId(tCIDLib::TCard4& c4First, tCIDLib::TCard4& c4Last) const
{
    tCIDLib::TCardList fcolIds(8UL);

    fcolIds.c4AddElement(m_c4FldId_CurTemp);
    fcolIds.c4AddElement(m_c4FldId_FanMode);
    fcolIds.c4AddElement(m_c4FldId_FanOpMode);
    fcolIds.c4AddElement(m_c4FldId_HoldStatus);
    fcolIds.c4AddElement(m_c4FldId_HighSP);
    fcolIds.c4AddElement(m_c4FldId_LowSP);
    fcolIds.c4AddElement(m_c4FldId_Mode);
    fcolIds.c4AddElement(m_c4FldId_OpMode);

    // Remove any that never got set and are still maxcard
    while (fcolIds.bDeleteIfPresent(kCIDLib::c4MaxCard)) {}

    fcolIds.Sort(tCIDLib::eComp<tCIDLib::TCard4>);

    c4First = fcolIds[0];
    c4Last = fcolIds[fcolIds.c4ElemCount() - 1];
}


tCIDLib::TVoid THAIOmniThermo::StoreFldIds(const TCQCServerBase& sdrvOwn)
{
    // Do the non-V2 ones
    TString strFldName = strName();
    strFldName.Append(L"_Hold");
    m_c4FldId_HoldStatus = sdrvOwn.pflddFind(strFldName, kCIDLib::True)->c4Id();

    // And the V2 ones
    m_c4FldId_CurTemp   = sdrvOwn.flddFind(tCQCKit::EDevClasses::Thermostat, strName(), L"CurrentTemp").c4Id();
    m_c4FldId_FanMode   = sdrvOwn.flddFind(tCQCKit::EDevClasses::Thermostat, strName(), L"FanMode").c4Id();
    m_c4FldId_FanOpMode = sdrvOwn.flddFind(tCQCKit::EDevClasses::Thermostat, strName(), L"FanOpMode").c4Id();
    m_c4FldId_HighSP    = sdrvOwn.flddFind(tCQCKit::EDevClasses::Thermostat, strName(), L"HighSetPnt").c4Id();
    m_c4FldId_LowSP     = sdrvOwn.flddFind(tCQCKit::EDevClasses::Thermostat, strName(), L"LowSetPnt").c4Id();
    m_c4FldId_Mode      = sdrvOwn.flddFind(tCQCKit::EDevClasses::Thermostat, strName(), L"CurMode").c4Id();
    m_c4FldId_OpMode    = sdrvOwn.flddFind(tCQCKit::EDevClasses::Thermostat, strName(), L"OpMode").c4Id();
}


// ---------------------------------------------------------------------------
//  THAIOmniThermo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 THAIOmniThermo::c4FldIdCurTemp() const
{
    return m_c4FldId_CurTemp;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdFanMode() const
{
    return m_c4FldId_FanMode;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdFanOpMode() const
{
    return m_c4FldId_FanOpMode;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdHighSP() const
{
    return m_c4FldId_HighSP;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdHoldStatus() const
{
    return m_c4FldId_HoldStatus;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdLowSP() const
{
    return m_c4FldId_LowSP;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdMode() const
{
    return m_c4FldId_Mode;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdOpMode() const
{
    return m_c4FldId_OpMode;
}


tHAIOmniTCP2Sh::EThermoTypes THAIOmniThermo::eThermoType() const
{
    return m_eThermoType;
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
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniTCP2Sh_Item::c1FmtVerThermo))
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

    // Stream in our stuff
    strmToReadFrom >> m_eThermoType;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Init runtime stuff
    m_c4FldId_CurTemp    = kCIDLib::c4MaxCard;
    m_c4FldId_FanMode    = kCIDLib::c4MaxCard;
    m_c4FldId_FanOpMode  = kCIDLib::c4MaxCard;
    m_c4FldId_HighSP     = kCIDLib::c4MaxCard;
    m_c4FldId_HoldStatus = kCIDLib::c4MaxCard;
    m_c4FldId_LowSP      = kCIDLib::c4MaxCard;
    m_c4FldId_Mode       = kCIDLib::c4MaxCard;
    m_c4FldId_OpMode     = kCIDLib::c4MaxCard;
}


tCIDLib::TVoid THAIOmniThermo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << HAIOmniTCP2Sh_Item::c1FmtVerThermo;

    // Stream our parent
    TParent::StreamTo(strmToWriteTo);

    // Stream out our stuff and and end marker
    strmToWriteTo   << m_eThermoType
                    << tCIDLib::EStreamMarkers::EndObject;
}





// ---------------------------------------------------------------------------
//   CLASS: THAIOmniUnit
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniUnit::THAIOmniUnit() :

    THAIOmniItem(tHAIOmniTCP2Sh::EItemTypes::Unit)
    , m_c4FldId_Value(kCIDLib::c4MaxCard)
    , m_eUnitType(tHAIOmniTCP2Sh::EUnitTypes::Unused)
{
}

THAIOmniUnit::THAIOmniUnit( const   TString&                    strName
                            , const tCIDLib::TCard2             c2ItemNum
                            , const tHAIOmniTCP2Sh::EUnitTypes  eUnitType) :

    THAIOmniItem(strName, tHAIOmniTCP2Sh::EItemTypes::Unit, c2ItemNum)
    , m_c4FldId_Value(kCIDLib::c4MaxCard)
    , m_eUnitType(eUnitType)
{
}

THAIOmniUnit::THAIOmniUnit(const THAIOmniUnit& itemSrc) :

    THAIOmniItem(itemSrc)
    , m_c4FldId_Value(itemSrc.m_c4FldId_Value)
    , m_eUnitType(itemSrc.m_eUnitType)
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


// ---------------------------------------------------------------------------
//  THAIOmniUnit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THAIOmniUnit::bCanSendTrig() const
{
    // If we are a binary or flag, we can
    return (m_eUnitType == tHAIOmniTCP2Sh::EUnitTypes::Switch)
           || (m_eUnitType == tHAIOmniTCP2Sh::EUnitTypes::Flag);
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


tCIDLib::TBoolean THAIOmniUnit::bHasSubType(TString& strToFill) const
{
    if (m_eUnitType == tHAIOmniTCP2Sh::EUnitTypes::Count)
        strToFill.Clear();
    else
        strToFill = tHAIOmniTCP2Sh::strXlatEUnitTypes(m_eUnitType);
    return kCIDLib::True;
}


tCIDLib::TBoolean
THAIOmniUnit::bOwnsField(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_Value);
}


tCIDLib::TCard4
THAIOmniUnit::c4QueryFldDefs(       tCQCKit::TFldDefList&   colAddTo
                            ,       TCQCFldDef&             flddTmp
                            , const tCIDLib::TBoolean       ) const
{
    tCIDLib::TCard4 c4Ret;
    if (m_eUnitType == tHAIOmniTCP2Sh::EUnitTypes::Flag)
    {
        flddTmp.Set
        (
            TStrCat(L"Flag_", strName())
            , tCQCKit::EFldTypes::Card
            , tCQCKit::EFldAccess::ReadWrite
            , tCQCKit::EFldSTypes::Generic
            , L"Range: 0, 255"
        );
        colAddTo.objAdd(flddTmp);
        c4Ret = 1;
    }
     else
    {
        c4Ret = TCQCDevClass::c4LoadLightFlds
        (
            colAddTo
            , strName()
            , m_eUnitType == tHAIOmniTCP2Sh::EUnitTypes::Dimmer
            , m_eUnitType == tHAIOmniTCP2Sh::EUnitTypes::Switch
        );
    }
    return c4Ret;
}



tCIDLib::TCard4
THAIOmniUnit::c4QueryTypes( tCIDLib::TStrCollect&   colToFill
                            , tCIDLib::TCard4&      c4CurIndex)
{
    colToFill.RemoveAll();

    colToFill.objAdd(tHAIOmniTCP2Sh::strXlatEUnitTypes(tHAIOmniTCP2Sh::EUnitTypes::Dimmer));
    colToFill.objAdd(tHAIOmniTCP2Sh::strXlatEUnitTypes(tHAIOmniTCP2Sh::EUnitTypes::Flag));
    colToFill.objAdd(tHAIOmniTCP2Sh::strXlatEUnitTypes(tHAIOmniTCP2Sh::EUnitTypes::Switch));

    if ((m_eUnitType > tHAIOmniTCP2Sh::EUnitTypes::Unused)
    &&  (m_eUnitType < tHAIOmniTCP2Sh::EUnitTypes::Count))
    {
        c4CurIndex = tCIDLib::TCard4(m_eUnitType) - 1;
    }
     else
    {
        c4CurIndex = kCIDLib::c4MaxCard;
    }
    return colToFill.c4ElemCount();
}


// Polymorphically dup us
THAIOmniItem* THAIOmniUnit::pitemDup() const
{
    return new THAIOmniUnit(*this);
}


tCIDLib::TVoid
THAIOmniUnit::QueryFirstLastId(tCIDLib::TCard4& c4First, tCIDLib::TCard4& c4Last) const
{
    c4First = m_c4FldId_Value;
    c4Last = m_c4FldId_Value;
}


tCIDLib::TVoid THAIOmniUnit::StoreFldIds(const TCQCServerBase& sdrvOwner)
{
    tCIDLib::TCard4 c4Id = kCIDLib::c4MaxCard;
    switch(m_eUnitType)
    {
        case tHAIOmniTCP2Sh::EUnitTypes::Dimmer :
            m_c4FldId_Value = sdrvOwner.flddFind(tCQCKit::EDevClasses::Lighting, L"Dim", strName(), L"").c4Id();
            break;

        case tHAIOmniTCP2Sh::EUnitTypes::Flag :
        {
            TString strFldName = L"Flag_";
            strFldName.Append(strName());
            m_c4FldId_Value = sdrvOwner.pflddFind(strFldName, kCIDLib::True)->c4Id();
            break;
        }

        case tHAIOmniTCP2Sh::EUnitTypes::Switch :
            m_c4FldId_Value = sdrvOwner.flddFind(tCQCKit::EDevClasses::Lighting, L"Sw", strName(), L"").c4Id();
            break;

        default :
            CIDAssert2(L"Unknown unit type when storing field ids");
            break;
    };
}


tCIDLib::TVoid THAIOmniUnit::SetSubType(const TString& strToSet)
{
    m_eUnitType = tHAIOmniTCP2Sh::eXlatEUnitTypes(strToSet);
}



// ---------------------------------------------------------------------------
//  THAIOmniUnit: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 THAIOmniUnit::c4FldIdValue() const
{
    return m_c4FldId_Value;
}


tHAIOmniTCP2Sh::EUnitTypes THAIOmniUnit::eUnitType() const
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
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniTCP2Sh_Item::c1FmtVerUnit))
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

    // Just in case, watch for an invalid value
    if (m_eUnitType >= tHAIOmniTCP2Sh::EUnitTypes::Count)
        m_eUnitType = tHAIOmniTCP2Sh::EUnitTypes::Switch;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Init any runtime stuff
    m_c4FldId_Value = kCIDLib::c4MaxCard;
}


tCIDLib::TVoid THAIOmniUnit::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << HAIOmniTCP2Sh_Item::c1FmtVerUnit;

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

    THAIOmniItem(tHAIOmniTCP2Sh::EItemTypes::Zone)
    , m_c4AreaNum(1)
    , m_c4FldId_Analog(kCIDLib::c4MaxCard)
    , m_c4FldId_Arming(kCIDLib::c4MaxCard)
    , m_c4FldId_CurVal(kCIDLib::c4MaxCard)
    , m_c4FldId_HighSP(kCIDLib::c4MaxCard)
    , m_c4FldId_Latched(kCIDLib::c4MaxCard)
    , m_c4FldId_LowSP(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_eZoneType(tHAIOmniTCP2Sh::EZoneTypes::Unused)
{
}

THAIOmniZone::THAIOmniZone( const   TString&                    strName
                            , const tCIDLib::TCard2             c2ItemNum
                            , const tHAIOmniTCP2Sh::EZoneTypes  eType
                            , const tCIDLib::TCard4             c4AreaNum) :

    THAIOmniItem(strName, tHAIOmniTCP2Sh::EItemTypes::Zone, c2ItemNum)
    , m_c4AreaNum(c4AreaNum)
    , m_c4FldId_Analog(kCIDLib::c4MaxCard)
    , m_c4FldId_Arming(kCIDLib::c4MaxCard)
    , m_c4FldId_CurVal(kCIDLib::c4MaxCard)
    , m_c4FldId_HighSP(kCIDLib::c4MaxCard)
    , m_c4FldId_Latched(kCIDLib::c4MaxCard)
    , m_c4FldId_LowSP(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_eZoneType(eType)
{
}

THAIOmniZone::THAIOmniZone(const THAIOmniZone& itemSrc) :

    THAIOmniItem(itemSrc)
    , m_c4AreaNum(itemSrc.m_c4AreaNum)
    , m_c4FldId_Analog(itemSrc.m_c4FldId_Analog)
    , m_c4FldId_Arming(itemSrc.m_c4FldId_Arming)
    , m_c4FldId_CurVal(itemSrc.m_c4FldId_CurVal)
    , m_c4FldId_HighSP(itemSrc.m_c4FldId_HighSP)
    , m_c4FldId_Latched(itemSrc.m_c4FldId_Latched)
    , m_c4FldId_LowSP(itemSrc.m_c4FldId_LowSP)
    , m_c4FldId_Status(itemSrc.m_c4FldId_Status)
    , m_eZoneType(itemSrc.m_eZoneType)
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
        m_c4AreaNum         = itemSrc.m_c4AreaNum;
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


// ---------------------------------------------------------------------------
//  THAIOmniZone: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THAIOmniZone::bAlarmType() const
{
    return
    (
        (m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Alarm)
        ||  (m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Fire)
        ||  (m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Intrusion)
        ||  (m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Water)
    );
}


tCIDLib::TBoolean THAIOmniZone::bCanSendTrig() const
{
    // If we are an alarm type or motion zone
    return bAlarmType() || (m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Motion);
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
    return
    (
        (m_eZoneType == itemSrcZone.m_eZoneType)
        && (m_c4AreaNum == itemSrcZone.m_c4AreaNum)
    );
}


tCIDLib::TBoolean THAIOmniZone::bHasSubType(TString& strToFill) const
{
    strToFill = tHAIOmniTCP2Sh::strXlatEZoneTypes(m_eZoneType);
    return kCIDLib::True;
}


tCIDLib::TBoolean
THAIOmniZone::bOwnsField(const tCIDLib::TCard4 c4FldId) const
{
    return ((c4FldId   == m_c4FldId_Analog)
           || (c4FldId == m_c4FldId_Arming)
           || (c4FldId == m_c4FldId_CurVal)
           || (c4FldId == m_c4FldId_HighSP)
           || (c4FldId == m_c4FldId_Latched)
           || (c4FldId == m_c4FldId_LowSP)
           || (c4FldId == m_c4FldId_Status));
}


tCIDLib::TVoid
THAIOmniZone::QueryFirstLastId(tCIDLib::TCard4& c4First, tCIDLib::TCard4& c4Last) const
{
    tCIDLib::TCardList fcolIds(8UL);

    fcolIds.c4AddElement(m_c4FldId_Analog);
    fcolIds.c4AddElement(m_c4FldId_Arming);
    fcolIds.c4AddElement(m_c4FldId_CurVal);
    fcolIds.c4AddElement(m_c4FldId_HighSP);
    fcolIds.c4AddElement(m_c4FldId_Latched);
    fcolIds.c4AddElement(m_c4FldId_LowSP);
    fcolIds.c4AddElement(m_c4FldId_Status);

    // Remove any that never got set and are still maxcard
    while (fcolIds.bDeleteIfPresent(kCIDLib::c4MaxCard)) {}

    fcolIds.Sort(tCIDLib::eComp<tCIDLib::TCard4>);

    c4First = fcolIds[0];
    c4Last = fcolIds[fcolIds.c4ElemCount() - 1];
}


tCIDLib::TCard4
THAIOmniZone::c4QueryTypes( tCIDLib::TStrCollect&   colToFill
                            , tCIDLib::TCard4&      c4CurIndex)
{
    colToFill.RemoveAll();
    colToFill.objAdd(tHAIOmniTCP2Sh::strXlatEZoneTypes(tHAIOmniTCP2Sh::EZoneTypes::Alarm));
    colToFill.objAdd(tHAIOmniTCP2Sh::strXlatEZoneTypes(tHAIOmniTCP2Sh::EZoneTypes::Fire));
    colToFill.objAdd(tHAIOmniTCP2Sh::strXlatEZoneTypes(tHAIOmniTCP2Sh::EZoneTypes::Humidity));
    colToFill.objAdd(tHAIOmniTCP2Sh::strXlatEZoneTypes(tHAIOmniTCP2Sh::EZoneTypes::Intrusion));
    colToFill.objAdd(tHAIOmniTCP2Sh::strXlatEZoneTypes(tHAIOmniTCP2Sh::EZoneTypes::Motion));
    colToFill.objAdd(tHAIOmniTCP2Sh::strXlatEZoneTypes(tHAIOmniTCP2Sh::EZoneTypes::Temp));
    colToFill.objAdd(tHAIOmniTCP2Sh::strXlatEZoneTypes(tHAIOmniTCP2Sh::EZoneTypes::Water));

    if ((m_eZoneType > tHAIOmniTCP2Sh::EZoneTypes::Unused)
    &&  (m_eZoneType < tHAIOmniTCP2Sh::EZoneTypes::Count))
    {
        c4CurIndex = tCIDLib::TCard4(m_eZoneType) - 1;
    }
     else
    {
        c4CurIndex = kCIDLib::c4MaxCard;
    }
    return colToFill.c4ElemCount();
}


// Polymorphically dup us
THAIOmniItem* THAIOmniZone::pitemDup() const
{
    return new THAIOmniZone(*this);
}


tCIDLib::TCard4
THAIOmniZone::c4QueryFldDefs(       tCQCKit::TFldDefList&   colAddTo
                            ,       TCQCFldDef&             flddTmp
                            , const tCIDLib::TBoolean       bCelsius) const
{
    const tCIDLib::TCard4 c4OrgCnt = colAddTo.c4ElemCount();

    TString strArmingLims(128UL);
    tHAIOmniTCP2Sh::FormatEZoneArmModes(strArmingLims, L"Enum: ", kCIDLib::chComma);

    TString strLatchLims(128UL);
    tHAIOmniTCP2Sh::FormatEZoneLatch(strLatchLims, L"Enum: ", kCIDLib::chComma);

    // The helper will do our V2 fields
    TCQCDevClass::c4LoadSecZoneFlds
    (
        colAddTo, strName(), m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Motion
    );

    //
    //  Add some non-V2 fields, based on type
    //
    if (bAlarmType())
    {
        // Do the analog loop value (input, not floating point
        TCQCDevClass::c4LoadAIOFlds
        (
            colAddTo
            , strName()
            , kCIDLib::True
            , kCIDLib::False
            , L"Range: 0, 255"
        );

        // Do the zone arming status
        TString strFldName = strName();
        strFldName.Append(L"_Arming");
        flddTmp.Set
        (
            strFldName
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
            , strArmingLims
        );
        colAddTo.objAdd(flddTmp);

        // Do the latched zone status
        strFldName = strName();
        strFldName.Append(L"_Latched");
        flddTmp.Set
        (
            strFldName
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
            , strLatchLims
        );
        colAddTo.objAdd(flddTmp);
    }
     else if ((m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Temp)
          ||  (m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Humidity))
    {
        TString strTempLim;
        TString strSPLim;
        if (bCelsius)
        {
            strTempLim = L"Range: -18, 50";
            strSPLim = L"Range: -18, 50";
        }
         else
        {
            strTempLim = L"Range: -40, 189";
            strSPLim = L"Range: 0, 122";
        }
        TString strPerLim(L"Range: 0, 100");
        TString strLimStr;

        // It's a temp/humidity zone
        tCQCKit::EFldSTypes eSType;
        tCQCKit::EFldTypes  eType;
        if (m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Humidity)
        {
            strLimStr = strPerLim;
            eSType = tCQCKit::EFldSTypes::HumSensor;
            eType = tCQCKit::EFldTypes::Card;
        }
         else
        {
            strLimStr = strTempLim;
            eSType = tCQCKit::EFldSTypes::TempSensor;
            eType = tCQCKit::EFldTypes::Int;
        }

        TString strFldName = strName();
        strFldName.Append(L"_CurVal");
        flddTmp.Set
        (
            strFldName
            , eType
            , tCQCKit::EFldAccess::Read
            , eSType
            , strLimStr
        );
        colAddTo.objAdd(flddTmp);

        if (m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Humidity)
            strLimStr = strPerLim;
        else
            strLimStr = strSPLim;

        strFldName = strName();
        strFldName.Append(L"_LowSP");
        flddTmp.Set
        (
            strFldName
            , tCQCKit::EFldTypes::Int
            , tCQCKit::EFldAccess::ReadWrite
            , tCQCKit::EFldSTypes::LowSetPnt
            , strLimStr
        );
        colAddTo.objAdd(flddTmp);

        strFldName = strName();
        strFldName.Append(L"_HighSP");
        flddTmp.Set
        (
            strFldName
            , tCQCKit::EFldTypes::Int
            , tCQCKit::EFldAccess::ReadWrite
            , tCQCKit::EFldSTypes::HighSetPnt
            , strLimStr
        );
        colAddTo.objAdd(flddTmp);
    }

    return (colAddTo.c4ElemCount() - c4OrgCnt);
}


tCIDLib::TVoid THAIOmniZone::SetSubType(const TString& strToSet)
{
    m_eZoneType = tHAIOmniTCP2Sh::eXlatEZoneTypes(strToSet);
}


tCIDLib::TVoid THAIOmniZone::StoreFldIds(const TCQCServerBase& sdrvOwner)
{
    TString strFldName;

    if (bAlarmType())
    {
        strFldName = strName();
        strFldName.Append(L"_Arming");
        m_c4FldId_Arming = sdrvOwner.pflddFind(strFldName, kCIDLib::True)->c4Id();

        strFldName = strName();
        strFldName.Append(L"_Latched");
        m_c4FldId_Latched = sdrvOwner.pflddFind(strFldName, kCIDLib::True)->c4Id();

        m_c4FldId_Status = sdrvOwner.flddFind(tCQCKit::EDevClasses::Security, L"Zone", strName(), L"Status").c4Id();
        m_c4FldId_Analog = sdrvOwner.flddFind(tCQCKit::EDevClasses::AIO, L"In", strName(), L"").c4Id();
    }
     else if ((m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Temp)
          ||  (m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Humidity))
    {
        m_c4FldId_Status = sdrvOwner.flddFind(tCQCKit::EDevClasses::Security, L"Zone", strName(), L"Status").c4Id();

        strFldName = strName();
        strFldName.Append(L"_CurVal");
        m_c4FldId_CurVal = sdrvOwner.pflddFind(strFldName, kCIDLib::True)->c4Id();

        strFldName = strName();
        strFldName.Append(L"_LowSP");
        m_c4FldId_LowSP = sdrvOwner.pflddFind(strFldName, kCIDLib::True)->c4Id();

        strFldName = strName();
        strFldName.Append(L"_HighSP");
        m_c4FldId_HighSP = sdrvOwner.pflddFind(strFldName, kCIDLib::True)->c4Id();
    }
     else if (m_eZoneType == tHAIOmniTCP2Sh::EZoneTypes::Motion)
    {
        strFldName = L"MOT#";
        strFldName.Append(strName());
        m_c4FldId_Status = sdrvOwner.pflddFind(strFldName, kCIDLib::True)->c4Id();
    }
     else
    {
        CIDAssert2(L"Unknown zone type when storing fld ids");
    }
}


// ---------------------------------------------------------------------------
//  THAIOmniZone: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Get/set the number of the area this zone belongs to. This is just the Omni area id
//  really, though it's a Card4 here.
//
tCIDLib::TCard4 THAIOmniZone::c4AreaNum() const
{
    return m_c4AreaNum;
}

tCIDLib::TCard4 THAIOmniZone::c4AreaNum(const tCIDLib::TCard4 c4ToSet)
{
    m_c4AreaNum = c4ToSet;
    return m_c4AreaNum;
}


// Get the ids of our various fields
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


tHAIOmniTCP2Sh::EZoneTypes THAIOmniZone::eZoneType() const
{
    return m_eZoneType;
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
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniTCP2Sh_Item::c1FmtVerZone))
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
    strmToReadFrom  >> m_eZoneType
                    >> m_c4AreaNum;

    // Make sure the zone type is valid, just in case
    if (m_eZoneType >= tHAIOmniTCP2Sh::EZoneTypes::Count)
        m_eZoneType = tHAIOmniTCP2Sh::EZoneTypes::Unused;

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
                    << HAIOmniTCP2Sh_Item::c1FmtVerZone;

    // Let our parent stream
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_eZoneType
                    << m_c4AreaNum
                    << tCIDLib::EStreamMarkers::EndObject;
}


