//
// FILE NAME: CQCKit_DevClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/27/2014
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
//  This is the implementation for the device class class.
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
#include    "CQCKit_.hpp"



// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCDevClass,TObject)
RTTIDecls(TCQCV2Validator,TObject)


// ---------------------------------------------------------------------------
//  CLASS: TCQCDevClFldCfg
// PREFIX: dvclsfc
//
//  For each field we can keep up with one or more possible configuration for that
//  field.
// ---------------------------------------------------------------------------
class TCQCDevClFldCfg
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDevClFldCfg();

        TCQCDevClFldCfg(const TCQCDevClFldCfg&) = default;
        TCQCDevClFldCfg(TCQCDevClFldCfg&&) = default;

        ~TCQCDevClFldCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDevClFldCfg& operator=(const TCQCDevClFldCfg&) = default;
        TCQCDevClFldCfg& operator=(TCQCDevClFldCfg&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ParseFrom
        (
            const   TXMLTreeElement&        xtnodeCfg
        );


        tCIDLib::TBoolean   m_bLimitOpt;
        tCQCKit::EReqAccess m_eAccess;
        tCQCKit::EFldSTypes m_eSemType;
        tCQCKit::EFldTypes  m_eType;
        TString             m_strLimits;
        TString             m_strLimType;
};


// -------------------------------------------------------------------
//  TCQCDevClFldCfg: Constructors and Destructor
// -------------------------------------------------------------------
TCQCDevClFldCfg::TCQCDevClFldCfg() :

    m_bLimitOpt(kCIDLib::False)
    , m_eAccess(tCQCKit::EReqAccess::None)
    , m_eSemType(tCQCKit::EFldSTypes::Generic)
    , m_eType(tCQCKit::EFldTypes::Count)
{
}

TCQCDevClFldCfg::~TCQCDevClFldCfg()
{
}


// -------------------------------------------------------------------
//  TCQCDevClFldCfg: Public, non-virtual methods
// -------------------------------------------------------------------
tCIDLib::TVoid TCQCDevClFldCfg::ParseFrom(const TXMLTreeElement& xtnodeCfg)
{
    // Some we just store directly
    m_strLimits = xtnodeCfg.xtattrNamed(L"CQCDC:Limits").strValue();;
    m_strLimType = xtnodeCfg.xtattrNamed(L"CQCDC:LimitType").strValue();;

    // Convert where necessary to binary form
    const TString& strAccess = xtnodeCfg.xtattrNamed(L"CQCDC:Access").strValue();;
    const TString& strSemType = xtnodeCfg.xtattrNamed(L"CQCDC:SemType").strValue();;
    const TString& strType = xtnodeCfg.xtattrNamed(L"CQCDC:Type").strValue();

    m_bLimitOpt = facCQCKit().bCheckYesNoVal
    (
        xtnodeCfg.xtattrNamed(L"CQCDC:LimitOpt").strValue()
    );
    m_eAccess = tCQCKit::eAltXlatEReqAccess(strAccess);
    m_eSemType = tCQCKit::eAltXlatEFldSTypes(strSemType);
    m_eType = tCQCKit::eAltXlatEFldTypes(strType);
}






// ---------------------------------------------------------------------------
//  CLASS: TCQCDevClFld
// PREFIX: dvclsf
//
//  The name (or pattern) for this field, plus a list of possible configuration
//  that the field can conform to.
// ---------------------------------------------------------------------------
class TCQCDevClFld
{
    public  :
        // -------------------------------------------------------------------
        //  The list of possible configurations this field can be.
        // -------------------------------------------------------------------
        using TTypeList = TVector<TCQCDevClFldCfg>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDevClFld();

        TCQCDevClFld
        (
            const   TCQCDevClFld&           dvclsfSrc
        );

        ~TCQCDevClFld();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDevClFld& operator=
        (
            const   TCQCDevClFld&           dvclsfSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ParseFrom
        (
            const   TXMLTreeElement&        xtnodeCfg
            , const TCQCDevClass::ENameTypes eNameType
        );


        // -------------------------------------------------------------------
        //  Public data members
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bRegEx;
        TTypeList           m_colConfigs;
        TString             m_strName;
        TRegEx              m_regxPat;
};


// -------------------------------------------------------------------
//  Constructors and Destructor
// -------------------------------------------------------------------
TCQCDevClFld::TCQCDevClFld() :

    m_bRegEx(kCIDLib::False)
{
}

TCQCDevClFld::TCQCDevClFld(const TCQCDevClFld& dvclsfSrc) :

    m_bRegEx(dvclsfSrc.m_bRegEx)
    , m_colConfigs(dvclsfSrc.m_colConfigs)
    , m_strName(dvclsfSrc.m_strName)
{
    // If a reg expression, then set it
    if (m_bRegEx)
        m_regxPat.SetExpression(dvclsfSrc.m_strName);
}

TCQCDevClFld::~TCQCDevClFld()
{
}


// -------------------------------------------------------------------
//  Public operators
// -------------------------------------------------------------------
TCQCDevClFld& TCQCDevClFld::operator=(const TCQCDevClFld& dvclsfSrc)
{
    if (&dvclsfSrc != this)
    {
        m_bRegEx     = dvclsfSrc.m_bRegEx;
        m_colConfigs = dvclsfSrc.m_colConfigs;
        m_strName    = dvclsfSrc.m_strName;

        // If a reg expression, then set it
        if (m_bRegEx)
            m_regxPat.SetExpression(dvclsfSrc.m_strName);
    }
    return *this;
}


// -------------------------------------------------------------------
//  Public, non-virtual methods
// -------------------------------------------------------------------
tCIDLib::TVoid
TCQCDevClFld::ParseFrom(const   TXMLTreeElement&            xtnodeFld
                        , const TCQCDevClass::ENameTypes    eNameType)
{
    // Store the name oriented suff
    m_bRegEx = (eNameType == TCQCDevClass::ENameTypes::Pattern);
    m_strName = xtnodeFld.xtattrNamed(L"CQCDC:Name").strValue();

    // If a regular expression, then set it
    if (m_bRegEx)
        m_regxPat.SetExpression(m_strName);

    m_colConfigs.RemoveAll();
    TCQCDevClFldCfg dvclsfcCur;

    const tCIDLib::TCard4 c4CfgCnt = xtnodeFld.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CfgCnt; c4Index++)
    {
        const TXMLTreeElement& xtnodeCfg = xtnodeFld.xtnodeChildAtAsElement(c4Index);
        dvclsfcCur.ParseFrom(xtnodeCfg);
        m_colConfigs.objAdd(dvclsfcCur);
    }
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCDevClass
// PREFIX: dvcls
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDevClass: Public data
// ---------------------------------------------------------------------------

//
//  The simple DTD for the device class XML format
//
const TString TCQCDevClass::strDTDText
(
    L"<?xml encoding='$NativeWideChar$'?>\n"

    L"<!ELEMENT CQCDC:GenInfo EMPTY>\n"
    L"<!ATTLIST CQCDC:GenInfo\n"
    L"          CQCDC:Base CDATA 'CQCDevice'\n"
    L"          CQCDC:Class CDATA #REQUIRED\n"
    L"          CQCDC:Prefix CDATA #REQUIRED\n"
    L"          CQCDC:Descr CDATA #REQUIRED\n"
    L"          CQCDC:MUnit (Yes | No) #REQUIRED\n"
    L"          CQCDC:NameType (Fixed | Free | Pattern) #REQUIRED\n"
    L"          CQCDC:Power (Yes | No) #REQUIRED>\n"

    L"<!ELEMENT CQCDC:FieldCfg EMPTY>\n"
    L"<!ATTLIST CQCDC:FieldCfg\n"
    L"          CQCDC:Type (Boolean | Card | Float | Int | String | Time | StringList) #REQUIRED\n"
    L"          CQCDC:SemType CDATA 'Generic'\n"
    L"          CQCDC:Access (ReadOnly | WriteOnly | MReadCWrite | CReadMWrite\n"
    L"                          | ReadOrWrite | ReadAndWrite) #REQUIRED\n"
    L"          CQCDC:Limits CDATA ''\n"
    L"          CQCDC:LimitType CDATA ''\n"
    L"          CQCDC:LimitOpt (Yes | No) 'No'>\n"

    L"<!ELEMENT CQCDC:Field (CQCDC:FieldCfg+)>\n"
    L"<!ATTLIST CQCDC:Field\n"
    L"          CQCDC:Name CDATA #REQUIRED>\n"

    L"<!ELEMENT CQCDC:Fields (CQCDC:Field+)>\n"


    L"<!ELEMENT CQCDC:DevClass (CQCDC:GenInfo, CQCDC:Fields)>\n"
    L"<!ATTLIST CQCDC:DevClass\n"
    L"          CQCDC:Version CDATA #REQUIRED>\n"
);


//
//  These methods load up the field defs for specific device classes. We can only
//  support those that have fixed field names or some reasonable pattern that we
//  can create.
//
tCIDLib::TCard4
TCQCDevClass::c4LoadAIOFlds(        tCQCKit::TFldDefList&   colToFill
                            , const TString&                strIOName
                            , const tCIDLib::TBoolean       bInput
                            , const tCIDLib::TBoolean       bFloat
                            , const TString&                strLim)
{
    TCQCFldDef  flddNew;
    TString     strName(64UL);

    strName = L"AIO#";
    if (bInput)
        strName.Append(L"In_");
    else
        strName.Append(L"Out_");
    strName.Append(strIOName);

    flddNew.Set
    (
        strName
        , bFloat ? tCQCKit::EFldTypes::Float : tCQCKit::EFldTypes::Card
        , bInput ? tCQCKit::EFldAccess::Read : tCQCKit::EFldAccess::Write
        , tCQCKit::EFldSTypes::AnalogIO
        , strLim
    );
    colToFill.objAdd(flddNew);

    return 1;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadAudioFlds(          tCQCKit::TFldDefList&   colToFill
                                , const TString&                strSubUnit)
{
    const tCIDLib::TBoolean bSub = !strSubUnit.bIsEmpty();
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TString                 strName(64UL);

    strName = L"AUD#";
    if (bSub)
    {
        strName.Append(strSubUnit);
        strName.Append(kCIDLib::chTilde);
    }
    const tCIDLib::TCard4 c4PrefLen = strName.c4Length();

    strName.CapAt(c4PrefLen);
    strName.Append(L"AdjustVolume");
    colToFill.objPlace
    (
        strName
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
        , tCQCKit::EFldSTypes::VolumeAdj
    );

    strName.CapAt(c4PrefLen);
    strName.Append(L"Mute");
    colToFill.objPlace
    (
        strName
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::Mute
    );

    strName.CapAt(c4PrefLen);
    strName.Append(L"Volume");
    colToFill.objPlace
    (
        strName
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::Volume
        , L"Range: 0, 100"
    );

    return colToFill.c4ElemCount() - c4StartCnt;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadAVProcFlds(         tCQCKit::TFldDefList&   colToFill
                                , const TString&                strSubUnit
                                , const TString&                strCurLim
                                , const TString&                strSetLim
                                , const tCIDLib::TBoolean       bSetWriteAlways)
{
    const tCIDLib::TBoolean bSub = !strSubUnit.bIsEmpty();
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TCQCFldDef              flddNew;
    TString                 strName(64UL);

    strName = L"AVPRC#";
    if (bSub)
    {
        strName.Append(strSubUnit);
        strName.Append(kCIDLib::chTilde);
    }
    const tCIDLib::TCard4 c4PrefLen = strName.c4Length();

    strName.CapAt(c4PrefLen);
    strName.Append(L"CurAudioMode");
    flddNew.Set
    (
        strName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , strCurLim
    );
    colToFill.objAdd(flddNew);


    strName.CapAt(c4PrefLen);
    strName.Append(L"SetAudioMode");
    flddNew.Set
    (
        strName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::ReadWrite
        , strSetLim
    );
    if (bSetWriteAlways)
        flddNew.bAlwaysWrite(kCIDLib::True);
    colToFill.objAdd(flddNew);

    return colToFill.c4ElemCount() - c4StartCnt;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadDevInfoFlds(        tCQCKit::TFldDefList&   colToFill
                                , const TString&                strSubUnit)
{
    const tCIDLib::TBoolean bSub = !strSubUnit.bIsEmpty();
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TCQCFldDef              flddNew;
    TString                 strName(64UL);

    strName = L"DEVI#";
    if (bSub)
    {
        strName.Append(strSubUnit);
        strName.Append(kCIDLib::chTilde);
    }
    const tCIDLib::TCard4 c4PrefLen = strName.c4Length();

    strName.CapAt(c4PrefLen);
    strName.Append(L"Firmware");
    flddNew.Set
    (
        strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read
    );
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"Model");
    flddNew.Set
    (
        strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read
    );
    colToFill.objAdd(flddNew);

    return colToFill.c4ElemCount() - c4StartCnt;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadDIOFlds(        tCQCKit::TFldDefList&   colToFill
                            , const TString&                strIOName
                            , const tCIDLib::TBoolean       bInput)
{
    TCQCFldDef  flddNew;
    TString     strName(64UL);

    strName = L"DIO#";
    if (bInput)
        strName.Append(L"In_");
    else
        strName.Append(L"Out_");
    strName.Append(strIOName);

    flddNew.Set
    (
        strName
        , tCQCKit::EFldTypes::Boolean
        , bInput ? tCQCKit::EFldAccess::Read : tCQCKit::EFldAccess::Write
        , tCQCKit::EFldSTypes::DigitalIO
    );
    colToFill.objAdd(flddNew);

    return 1;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadLightFlds(          tCQCKit::TFldDefList&   colToFill
                                , const TString&                strLightName
                                , const tCIDLib::TBoolean       bDimmer
                                , const tCIDLib::TBoolean       bSwitch)
{
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TCQCFldDef              flddNew;
    TString                 strName(64UL);

    if (bDimmer)
    {
        strName = L"LGHT#";
        strName.Append(L"Dim_");
        strName.Append(strLightName);

        flddNew.Set
        (
            strName
            , tCQCKit::EFldTypes::Card
            , tCQCKit::EFldAccess::ReadWrite
            , tCQCKit::EFldSTypes::Dimmer
            , L"Range: 0, 100"
        );
        colToFill.objAdd(flddNew);
    }

    if (bSwitch)
    {
        strName = L"LGHT#";
        strName.Append(L"Sw_");
        strName.Append(strLightName);

        flddNew.Set
        (
            strName
            , tCQCKit::EFldTypes::Boolean
            , tCQCKit::EFldAccess::ReadWrite
            , tCQCKit::EFldSTypes::LightSwitch
        );
        colToFill.objAdd(flddNew);
    }

    return colToFill.c4ElemCount() - c4StartCnt;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadMTransFlds(         tCQCKit::TFldDefList&   colToFill
                                , const TString&                strSubUnit)
{
    const tCIDLib::TBoolean bSub = !strSubUnit.bIsEmpty();
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TCQCFldDef              flddNew;
    TString                 strName(64UL);

    strName = L"MTRANS#";
    if (bSub)
    {
        strName.Append(strSubUnit);
        strName.Append(kCIDLib::chTilde);
    }
    const tCIDLib::TCard4 c4PrefLen = strName.c4Length();

    strName.CapAt(c4PrefLen);
    strName.Append(L"MediaState");
    flddNew.Set
    (
        strName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::MediaState
        , L"Enum: Undefined, Buffering, Loading, Playing, Paused, Stopped"
    );
    colToFill.objAdd(flddNew);


    strName.CapAt(c4PrefLen);
    strName.Append(L"Transport");
    flddNew.Set
    (
        strName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , tCQCKit::EFldSTypes::MediaTransport
        , L"Enum: Pause, Play, Stop, Next, Previous, FF, Rewind"
    );
    colToFill.objAdd(flddNew);

    return colToFill.c4ElemCount() - c4StartCnt;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadNowPlayingFlds(         tCQCKit::TFldDefList&   colToFill
                                    , const TString&                strSubUnit)
{
    const tCIDLib::TBoolean bSub = !strSubUnit.bIsEmpty();
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TCQCFldDef              flddNew;
    TString                 strName(64UL);

    strName = L"NWPLY#";
    if (bSub)
    {
        strName.Append(strSubUnit);
        strName.Append(kCIDLib::chTilde);
    }
    const tCIDLib::TCard4 c4PrefLen = strName.c4Length();

    strName.CapAt(c4PrefLen);
    strName.Append(L"AspectRatio");
    flddNew.Set(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"Artist");
    flddNew.Set(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"Cast");
    flddNew.Set(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"CollectName");
    flddNew.Set(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"Description");
    flddNew.Set(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"ImgSerialNum");
    flddNew.Set(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"ItemName");
    flddNew.Set(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"PBPercent");
    flddNew.Set(strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read, L"Range: 0, 100");
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"Rating");
    flddNew.Set(strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read, L"Range: 0, 10");
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"Year");
    flddNew.Set(strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read);
    colToFill.objAdd(flddNew);

    return colToFill.c4ElemCount() - c4StartCnt;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadPowerFlds(          tCQCKit::TFldDefList&   colToFill
                                , const TString&                strSubUnit)
{
    const tCIDLib::TBoolean bSub = !strSubUnit.bIsEmpty();
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TCQCFldDef              flddNew;
    TString                 strName(64UL);
    TString                 strLims(64UL);

    strName = L"PWR#";
    if (bSub)
    {
        strName.Append(strSubUnit);
        strName.Append(kCIDLib::chTilde);
    }
    const tCIDLib::TCard4 c4PrefLen = strName.c4Length();

    strName.CapAt(c4PrefLen);
    strName.Append(L"Power");
    flddNew.Set
    (
        strName
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
        , tCQCKit::EFldSTypes::Power
    );
    colToFill.objAdd(flddNew);


    strLims = L"Enum: ";
    tCIDLib::ForEachE<tCQCKit::EPowerStatus>
    (
        [&strLims](const auto eStatus)
        {
            if (eStatus > tCQCKit::EPowerStatus::Min)
                strLims.Append(L", ");
            strLims.Append(tCQCKit::strXlatEPowerStatus(eStatus));
        }
    );

    strName.CapAt(c4PrefLen);
    strName.Append(L"Status");
    flddNew.Set
    (
        strName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::PowerState
        , strLims
    );
    colToFill.objAdd(flddNew);

    return colToFill.c4ElemCount() - c4StartCnt;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadProjFlds(       tCQCKit::TFldDefList&   colToFill
                            , const TString&                strSubUnit
                            , const TString&                strARLim
                            , const tCIDLib::TBoolean       bARWriteAlways)
{
    const tCIDLib::TBoolean bSub = !strSubUnit.bIsEmpty();
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TCQCFldDef              flddNew;
    TString                 strName(64UL);


    strName = L"PROJ#";
    if (bSub)
    {
        strName.Append(strSubUnit);
        strName.Append(kCIDLib::chTilde);
    }
    const tCIDLib::TCard4 c4PrefLen = strName.c4Length();

    strName.CapAt(c4PrefLen);
    strName.Append(L"AspectRatio");

    flddNew.Set
    (
        strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::ReadWrite, strARLim
    );
    if (bARWriteAlways)
        flddNew.bAlwaysWrite(kCIDLib::True);
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"OpAspectRatio");
    flddNew.Set(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colToFill.objAdd(flddNew);

    return colToFill.c4ElemCount() - c4StartCnt;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadSecAreaFlds(        tCQCKit::TFldDefList&   colToFill
                                , const TString&                strAreaName
                                , const TString&                strArmLim)
{
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TCQCFldDef              flddNew;
    TString                 strName(64UL);


    strName = L"SEC#Area_";
    strName.Append(strAreaName);
    strName.Append(kCIDLib::chUnderscore);
    const tCIDLib::TCard4 c4PrefLen = strName.c4Length();


    strName.CapAt(c4PrefLen);
    strName.Append(L"InAlarm");
    flddNew.Set
    (
        strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read
    );
    colToFill.objAdd(flddNew);


    strName.CapAt(c4PrefLen);
    strName.Append(L"Arming");
    flddNew.Set
    (
        strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read
    );
    colToFill.objAdd(flddNew);


    strName.CapAt(c4PrefLen);
    strName.Append(L"Alarms");
    flddNew.Set
    (
        strName, tCQCKit::EFldTypes::StringList, tCQCKit::EFldAccess::Read
    );
    colToFill.objAdd(flddNew);


    strName.CapAt(c4PrefLen);
    strName.Append(L"ArmMode");
    flddNew.Set
    (
        strName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , strArmLim
    );
    colToFill.objAdd(flddNew);

    return colToFill.c4ElemCount() - c4StartCnt;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadSecZoneFlds(        tCQCKit::TFldDefList&   colToFill
                                , const TString&                strZoneName
                                , const tCIDLib::TBoolean       bIsMotion)
{
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TCQCFldDef              flddNew;
    TString                 strName(64UL);

    if (bIsMotion)
    {
        strName = L"MOT#";
        strName.Append(strZoneName);

        flddNew.Set
        (
            strName
            , tCQCKit::EFldTypes::Boolean
            , tCQCKit::EFldAccess::Read
            , tCQCKit::EFldSTypes::MotionSensor
        );
    }
     else
    {
        TString strLims;
        tCQCKit::FormatEZoneStates(strLims, L"Enum: ", kCIDLib::chComma);

        strName = L"SEC#Zone_";
        strName.Append(strZoneName);
        strName.Append(L"_Status");

        flddNew.Set
        (
            strName
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
            , tCQCKit::EFldSTypes::SecZoneStat
            , strLims
        );
    }
    colToFill.objAdd(flddNew);

    return colToFill.c4ElemCount() - c4StartCnt;
}


//
//  The current source limit can be empty if they don't want to set one. If both are
//  provided they must be the same type (Enum or Range.) The settable limit must be
//  provided.
//
tCIDLib::TCard4
TCQCDevClass::c4LoadSwitcherFlds(       tCQCKit::TFldDefList&   colToFill
                                , const TString&                strSubUnit
                                , const TString&                strCurSrcLim
                                , const TString&                strSetSrcLim
                                , const tCIDLib::TBoolean       bSetWriteAlways)
{
    const tCIDLib::TBoolean bSub = !strSubUnit.bIsEmpty();
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TCQCFldDef              flddNew;
    TString                 strName(64UL);
    TString                 strCurLims(strCurSrcLim);
    TString                 strSetLims(strSetSrcLim);

    strName = L"SWTCH#";
    if (bSub)
    {
        strName.Append(strSubUnit);
        strName.Append(kCIDLib::chTilde);
    }
    const tCIDLib::TCard4 c4PrefLen = strName.c4Length();


    // Figure out the data type based on enum
    tCQCKit::EFldTypes eType = tCQCKit::EFldTypes::Count;
    strSetLims.StripWhitespace();
    if (strSetLims.bStartsWithI(L"Enum"))
        eType = tCQCKit::EFldTypes::String;
    else if (strSetLims.bStartsWithI(L"Range"))
        eType = tCQCKit::EFldTypes::Card;
    else
    {
        strSetLims.bCapAtChar(kCIDLib::chColon);
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFld_WrongLimitType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
            , strName
            , strSetLims
        );
    }

    strName.CapAt(c4PrefLen);
    strName.Append(L"Source");
    flddNew.Set(strName, eType, tCQCKit::EFldAccess::ReadWrite, strSetLims);
    if (bSetWriteAlways)
        flddNew.bAlwaysWrite(kCIDLib::True);
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"OpSource");
    flddNew.Set
    (
        strName
        , eType
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::SourceInp
        , strCurLims
    );
    colToFill.objAdd(flddNew);

    return colToFill.c4ElemCount() - c4StartCnt;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadThermoFlds(         tCQCKit::TFldDefList&   colToFill
                                , const TString&                strSubUnit
                                , const TString&                strTempLim
                                , const TString&                strHSPLim
                                , const TString&                strLSPLim
                                , const TString&                strFanLim
                                , const TString&                strFanOpLim
                                , const TString&                strModeLim
                                , const TString&                strOpModeLim)
{
    const tCIDLib::TBoolean bSub = !strSubUnit.bIsEmpty();
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TCQCFldDef              flddNew;
    TString                 strName(64UL);


    strName = L"THERM#";
    if (bSub)
    {
        strName.Append(strSubUnit);
        strName.Append(kCIDLib::chTilde);
    }
    const tCIDLib::TCard4 c4PrefLen = strName.c4Length();


    //
    //  DO NOT CHANGE the order! Drivers may be dependent on each thermo's fields
    //  being contiguous and in this order.
    //
    strName.CapAt(c4PrefLen);
    strName.Append(L"CurrentTemp");
    flddNew.Set
    (
        strName
        , tCQCKit::EFldTypes::Int
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::CurTemp
        , strTempLim
    );
    colToFill.objAdd(flddNew);


    strName.CapAt(c4PrefLen);
    strName.Append(L"HighSetPnt");
    flddNew.Set
    (
        strName
        , tCQCKit::EFldTypes::Int
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::HighSetPnt
        , strHSPLim
    );
    colToFill.objAdd(flddNew);


    strName.CapAt(c4PrefLen);
    strName.Append(L"LowSetPnt");
    flddNew.Set
    (
        strName
        , tCQCKit::EFldTypes::Int
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::LowSetPnt
        , strLSPLim
    );
    colToFill.objAdd(flddNew);


    strName.CapAt(c4PrefLen);
    strName.Append(L"FanMode");
    flddNew.Set
    (
        strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::ReadWrite, strFanLim
    );
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"FanOpMode");
    flddNew.Set
    (
        strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read, strFanOpLim
    );
    colToFill.objAdd(flddNew);


    strName.CapAt(c4PrefLen);
    strName.Append(L"CurMode");
    flddNew.Set
    (
        strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::ReadWrite, strModeLim
    );
    colToFill.objAdd(flddNew);

    strName.CapAt(c4PrefLen);
    strName.Append(L"OpMode");
    flddNew.Set
    (
        strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read, strOpModeLim
    );
    colToFill.objAdd(flddNew);

    return colToFill.c4ElemCount() - c4StartCnt;
}


tCIDLib::TCard4
TCQCDevClass::c4LoadTunerFlds(          tCQCKit::TFldDefList&   colToFill
                                , const TString&                strSubUnit
                                , const TString&                strPreLim
                                , const tCIDLib::TBoolean       bWritePreset)
{
    const tCIDLib::TBoolean bSub = !strSubUnit.bIsEmpty();
    const tCIDLib::TCard4   c4StartCnt = colToFill.c4ElemCount();
    TCQCFldDef              flddNew;
    TString                 strName(64UL);


    strName = L"TUNR#";
    if (bSub)
    {
        strName.Append(strSubUnit);
        strName.Append(kCIDLib::chTilde);
    }
    const tCIDLib::TCard4 c4PrefLen = strName.c4Length();

    strName.CapAt(c4PrefLen);
    strName.Append(L"CurPreset");
    flddNew.Set
    (
        strName
        , tCQCKit::EFldTypes::String
        , bWritePreset ? tCQCKit::EFldAccess::ReadWrite : tCQCKit::EFldAccess::Read
        , strPreLim
    );
    colToFill.objAdd(flddNew);


    strName.CapAt(c4PrefLen);
    strName.Append(L"CurFrequency");
    flddNew.Set
    (
        strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read
    );
    colToFill.objAdd(flddNew);


    strName.CapAt(c4PrefLen);
    strName.Append(L"NextPrevPreset");
    flddNew.Set
    (
        strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Write
    );
    colToFill.objAdd(flddNew);

    return colToFill.c4ElemCount() - c4StartCnt;
}


// ---------------------------------------------------------------------------
//  TCQCDevClass: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDevClass::TCQCDevClass() :

    m_bMultiUnit(kCIDLib::False)
    , m_bNeedsPower(kCIDLib::False)
    , m_c4FmtVersion(0)
    , m_colOurFlds(tCIDLib::EAdoptOpts::NoAdopt, 109, TStringKeyOps(), &TCQCFldDef::strKey)
    , m_eDevClass(tCQCKit::EDevClasses::None)
    , m_eNameType(ENameTypes::Count)
    , m_pcolFldList(new TFldList(tCIDLib::EAdoptOpts::Adopt))
{
}

TCQCDevClass::TCQCDevClass(const TCQCDevClass& dvclsSrc) :

    m_bMultiUnit(dvclsSrc.m_bMultiUnit)
    , m_bNeedsPower(dvclsSrc.m_bNeedsPower)
    , m_c4FmtVersion(dvclsSrc.m_c4FmtVersion)
    , m_colOurFlds(tCIDLib::EAdoptOpts::NoAdopt, 109, TStringKeyOps(), &TCQCFldDef::strKey)
    , m_eDevClass(dvclsSrc.m_eDevClass)
    , m_eNameType(dvclsSrc.m_eNameType)
    , m_pcolFldList
      (
        new TFldList
        (
            tCIDLib::EAdoptOpts::Adopt, dvclsSrc.m_pcolFldList->c4ElemCount()
        )
      )
    , m_strClassDesc(dvclsSrc.m_strClassDesc)
    , m_strClassName(dvclsSrc.m_strClassName)
    , m_strClassPref(dvclsSrc.m_strClassPref)
{
    tCIDLib::TCard4 c4Count(dvclsSrc.m_pcolFldList->c4ElemCount());
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_pcolFldList->Add(new TCQCDevClFld(*dvclsSrc.m_pcolFldList->pobjAt(c4Index)));
}

TCQCDevClass::~TCQCDevClass()
{
    delete m_pcolFldList;
}


// ---------------------------------------------------------------------------
//  TCQCDevClass: Public operators
// ---------------------------------------------------------------------------
TCQCDevClass& TCQCDevClass::operator=(const TCQCDevClass& dvclsSrc)
{
    if (&dvclsSrc != this)
    {
        m_bMultiUnit    = dvclsSrc.m_bMultiUnit;
        m_bNeedsPower   = dvclsSrc.m_bNeedsPower;
        m_c4FmtVersion  = dvclsSrc.m_c4FmtVersion;
        m_eDevClass     = dvclsSrc.m_eDevClass;
        m_eNameType     = dvclsSrc.m_eNameType;
        m_strClassDesc  = dvclsSrc.m_strClassDesc;
        m_strClassName  = dvclsSrc.m_strClassName;
        m_strClassPref  = dvclsSrc.m_strClassPref;

        m_pcolFldList->RemoveAll();
        tCIDLib::TCard4 c4Count(dvclsSrc.m_pcolFldList->c4ElemCount());
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            m_pcolFldList->Add(new TCQCDevClFld(*dvclsSrc.m_pcolFldList->pobjAt(c4Index)));
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCDevClass: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCDevClass::bMultiUnit() const
{
    return m_bMultiUnit;
}


tCIDLib::TBoolean TCQCDevClass::bNeedsPower() const
{
    return m_bNeedsPower;
}


tCIDLib::TBoolean
TCQCDevClass::bParseFrom(const  tCQCKit::EDevClasses    eClass
                        , const TMemBuf&                mbufSrc
                        , const tCIDLib::TCard4         c4Bytes)
{
    TXMLEntitySrc* pxsrcConfig = new TMemBufEntitySrc(L"DevClass", mbufSrc, c4Bytes);
    tCIDXML::TEntitySrcRef esrConfig(pxsrcConfig);
    TXMLTreeParser xtprsDevCls;

    //
    //  Add an entity mapping so that the public URN that is used in the
    //  manifest files get mapped to our hard coded internal DTD text.
    //
    xtprsDevCls.AddMapping
    (
        new TMemBufEntitySrc
        (
            L"http://www.charmedquark.com/CQC/CQCDevClass.DTD"
            , L"urn:charmedquark.com:CQC-DeviceClass.DTD"
            , TCQCDevClass::strDTDText
        )
    );

    //
    //  Ok, we have a file, so lets try to use the XML tree parser to get
    //  a representation of the file into memory. Tell it that we only
    //  want to see non-ignorable chars and tag information. That will
    //  save us a lot of filtering out comments and whitspace.
    //
    const tCIDLib::TBoolean bOk = xtprsDevCls.bParseRootEntity
    (
        esrConfig
        , tCIDXML::EParseOpts::Validate
        , tCIDXML::EParseFlags::TagsNText
    );

    if (!bOk)
    {
        //
        //  Format the first error in the error list, into a string that
        //  we can pass as a replacement parameter to the real error we
        //  log.
        //
        const TXMLTreeParser::TErrInfo& erriFirst = xtprsDevCls.colErrors()[0];

        TTextStringOutStream strmOut(512);
        strmOut << L"[" << erriFirst.strSystemId() << L"/"
                << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                << L"] " << erriFirst.strText() << kCIDLib::FlushIt;

        //
        //  Throw an exception that indicates that the file was not valid
        //  XML. Pass along the first error that occured.
        //
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDevCls_InvalidXML
            , strmOut.strData()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TString(tCQCKit::strAltXlatEDevClasses(eClass))
        );
    }

    // Pull the info out and store it
    m_pcolFldList->RemoveAll();
    const TXMLTreeElement& xtnodeRoot = xtprsDevCls.xtdocThis().xtnodeRoot();
    try
    {
        const TString& strVer = xtnodeRoot.xtattrNamed(L"CQCDC:Version").strValue();
        m_c4FmtVersion = strVer.c4Val();

        // Get the general info element
        const TXMLTreeElement& xtnodeGen = xtnodeRoot.xtnodeChildAtAsElement(0);
        m_strClassDesc = xtnodeGen.xtattrNamed(L"CQCDC:Descr").strValue();
        m_strClassName = xtnodeGen.xtattrNamed(L"CQCDC:Class").strValue();
        m_strClassPref = xtnodeGen.xtattrNamed(L"CQCDC:Prefix").strValue();

        const TString& strMUnit = xtnodeGen.xtattrNamed(L"CQCDC:MUnit").strValue();
        m_bMultiUnit = facCQCKit().bCheckYesNoVal(strMUnit);

        const TString& strNeedsPwr = xtnodeGen.xtattrNamed(L"CQCDC:Power").strValue();
        m_bNeedsPower = facCQCKit().bCheckYesNoVal(strNeedsPwr);

        // Set up the name type
        const TString& strNameType = xtnodeGen.xtattrNamed(L"CQCDC:NameType").strValue();
        if (strNameType == L"Fixed")
            m_eNameType = ENameTypes::Fixed;
        else if (strNameType == L"Free")
            m_eNameType = ENameTypes::Free;
        else if (strNameType == L"Pattern")
            m_eNameType = ENameTypes::Pattern;

        CIDAssert(m_eNameType != ENameTypes::Count, L"Invalid name type in XML");


        //
        //  And let's process all of the fields and add them to our list. Not that,
        //  if in free naming mode, there can only be one field, that they all must
        //  match.
        //
        const TXMLTreeElement& xtnodeCfgs = xtnodeRoot.xtnodeChildAtAsElement(1);
        const tCIDLib::TCard4 c4FldCnt = xtnodeCfgs.c4ChildCount();

        if ((m_eNameType == TCQCDevClass::ENameTypes::Free) && (c4FldCnt > 1))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcDevCls_FreeOneFld
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , m_strClassName
            );
        }

        TCQCDevClFld dvclsfCur;
        m_pcolFldList->CheckExpansion(c4FldCnt);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FldCnt; c4Index++)
        {
            const TXMLTreeElement& xtnodeCfg = xtnodeCfgs.xtnodeChildAtAsElement(c4Index);

            //
            //  Set it up and add to the list. Tell him whether his name should be
            //  treated a a regular expression or not.
            //
            dvclsfCur.ParseFrom(xtnodeCfg, m_eNameType);
            m_pcolFldList->Add(new TCQCDevClFld(dvclsfCur));
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCKit().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }

    // It worked so store the device class
    m_eDevClass = eClass;
    return kCIDLib::True;
}


//
//  Given a list of fields currently set on a driver, we will validate all of the
//  fields that claim to be of our device class (based on prefix.)
//
tCIDLib::TBoolean
TCQCDevClass::bValidateFlds(tCQCKit::TFldDefList&   colFields
                            , tCIDLib::TStrCollect& colErrors) const
{
    //
    //  Go through the fields and find all of the fields that are relevant to our
    //  device class. In the process we insure that they alls tart with valid class
    //  prefixes, since we parse them out and convert the prefix to the class enum.
    //
    //  We also remember any unique sub-units we saw, and remember any device
    //  classes we saw represented.
    //

    tCIDLib::TStrHashSet    colSubs(109, TStringKeyOps());
    TString                 strError;

    m_colOurFlds.RemoveAll();
    {
        tCQCKit::EDevClasses eClass;
        TString strPref;
        TString strSuffix;
        TString strSub;

        const tCIDLib::TCard4 c4Count = colFields.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TCQCFldDef& flddCur = colFields[c4Index];

            // Parse the name and see if it is a V2 field and our device class
            if (facCQCKit().bParseFldName(flddCur.strName(), eClass, strSub, strSuffix)
            &&  (eClass == m_eDevClass))
            {
                // It's one of our guys
                m_colOurFlds.Add(&flddCur);

                // If we haven't seen this (non-empty) sub-unit yet, then store it
                if (!strSub.bIsEmpty() && !colSubs.bHasElement(strSub))
                    colSubs.objAdd(strSub);
            }
        }
    }

    // If any sub-units were found, then we have to be all multi-unit
    const tCIDLib::TBoolean bMulti = !colSubs.bIsEmpty();
    const tCIDLib::TCard4 c4FldCount = m_colOurFlds.c4ElemCount();

    // Check that they are all either sub-unit or not
    {
        tCIDLib::TBoolean bFail = kCIDLib::False;

        // If we got any sub-units then they all have to be
        tCIDLib::TCard4 c4Pos;
        TFldDefSet::TCursor cursFlds(&m_colOurFlds);
        if (cursFlds.bReset())
        {
            do
            {
                const TString& strCurName = cursFlds.objRCur().strName();
                if (strCurName.bFirstOccurrence(kCIDLib::chTilde, c4Pos) != bMulti)
                {
                    strError = m_strClassPref;
                    strError.Append(L" - All fields must be multi-unit or not");
                    colErrors.objAdd(strError);
                    bFail = kCIDLib::True;
                    break;
                }
            }   while(cursFlds.bNext());
        }

        // If we failed any here, then don't bother going further
        if (bFail)
            return kCIDLib::False;
    }

    //
    //  If multi-unit, then make sure that we have all of the defined fields for
    //  each sub-unit. Else check them all without sub-unit naming.
    //
    if (bMulti)
    {
        tCIDLib::TStrHashSet::TCursor cursSubs(&colSubs);
        if (cursSubs.bReset())
        {
            do
            {
                const TString& strCurSub = cursSubs.objRCur();
                bCheckSubUnit(strCurSub, colErrors);
            }   while(cursSubs.bNext());
        }
    }
     else
    {
        bCheckSubUnit(TString::strEmpty(), colErrors);
    }

    return kCIDLib::True;
}


tCIDLib::TCard4 TCQCDevClass::c4FmtVersion() const
{
    return m_c4FmtVersion;
}


tCQCKit::EDevClasses TCQCDevClass::eDevClass() const
{
    return m_eDevClass;
}


const TString& TCQCDevClass::strClassDesc() const
{
    return m_strClassDesc;
}


const TString& TCQCDevClass::strClassName() const
{
    return m_strClassName;
}


const TString& TCQCDevClass::strClassPref() const
{
    return m_strClassPref;
}


// ---------------------------------------------------------------------------
//  TCQCDevClass: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This handles the per-field changing not related to name, so the config part.
//  We return through each configuration to see if one matches.
//
tCIDLib::TBoolean
TCQCDevClass::bCheckField(  const   TCQCDevClFld&           dvclsfCur
                            , const TCQCFldDef&             flddTest
                            ,       tCIDLib::TStrCollect&   colErrors
                            ,       TString&                strError
                            , const tCIDLib::TBoolean       bReportFail) const
{
    // Get the count of configs and test them all
    const tCIDLib::TCard4 c4CfgCnt = dvclsfCur.m_colConfigs.c4ElemCount();
    tCIDLib::TCard4 c4CfgInd = 0;

    for (; c4CfgInd < c4CfgCnt; c4CfgInd++)
    {
        const TCQCDevClFldCfg& dvclsfcCur = dvclsfCur.m_colConfigs[c4CfgInd];

        // Looks reasonable. Check the access
        if (!facCQCKit().bCheckFldAccess(dvclsfcCur.m_eAccess, flddTest.eAccess()))
            continue;

        // Check the field type
        if (dvclsfcCur.m_eType != flddTest.eType())
            continue;

        // Check the semantic field type
        if (dvclsfcCur.m_eSemType != flddTest.eSemType())
            continue;

        //
        //  If there is a limit type set, check it. If not just testing the type,
        //  then see if we need to check the literal limits. We never do both.
        //
        if (!dvclsfcCur.m_strLimType.bIsEmpty())
        {
            if (flddTest.strLimits().bIsEmpty())
            {
                // The field doesn't have one, if not optional, it then we failed
                 if (!dvclsfcCur.m_bLimitOpt)
                    continue;
            }
             else if (!flddTest.strLimits().bStartsWithI(dvclsfcCur.m_strLimType))
            {
                // They didn'tmatch, so we failed
                continue;
            }
        }
         else
        {
            // Not checking the type, so check the limits
            if (dvclsfcCur.m_strLimits.bIsEmpty())
            {
                // Class doesn't define a limit. If the field does, then we failed
                if (!flddTest.strLimits().bIsEmpty())
                    continue;
            }
             else
            {
                if (flddTest.strLimits().bIsEmpty())
                {
                    // Device has limits but the field doesn't. Fail if not optional
                     if (!dvclsfcCur.m_bLimitOpt)
                        continue;
                }
                 else
                {
                    // They both have them, so parse them and compare
                    TCQCFldLimit* pfldlFld = TCQCFldLimit::pfldlMakeNew(flddTest);
                    TJanitor<TCQCFldLimit> janFld(pfldlFld);
                    TCQCFldLimit* pfldlCls = TCQCFldLimit::pfldlMakeNew
                    (
                        flddTest.strName(), dvclsfcCur.m_strLimits, flddTest.eType()
                    );
                    TJanitor<TCQCFldLimit> janCls(pfldlCls);

                    // If not equal, then a failure
                    if (!pfldlCls->bSameLimits(*pfldlFld))
                        continue;
                }
            }
        }

        // We got a full match so break out
        break;
    }

    // If we found a match, return true and no error
    if (c4CfgInd < c4CfgCnt)
        return kCIDLib::True;

    // Sometimes they are just checking and don't want to issue an error
    if (bReportFail)
    {
        strError = m_strClassPref;
        strError.Append(L" - Field '");
        strError.Append(flddTest.strName());
        strError.Append(L"' did not match any available field configuration");
        colErrors.objAdd(strError);
    }

    return kCIDLib::False;
}


//
//  For each sub-unit (or empty if none), we check all of the fields to see if they
//  are all present and have the required attributes.
//
tCIDLib::TBoolean
TCQCDevClass::bCheckSubUnit(const   TString&                strSubToCheck
                            ,       tCIDLib::TStrCollect&   colErrors) const
{
    TString strError;

    //
    //  We have different scenarios here, based on the field naming scheme.
    //
    const tCIDLib::TCard4 c4ClsFldCnt = m_pcolFldList->c4ElemCount();

    if (m_eNameType == ENameTypes::Free)
    {
        const TCQCDevClFld& dvclsfCur = *m_pcolFldList->pobjAt(0);

        //
        //  We don't check the name, just the other stuff, for each field. Each
        //  one must match one of the configurations of the one available field
        //  description.
        //
        TFldDefSet::TNCCursor cursDrvFlds(&m_colOurFlds);
        if (cursDrvFlds.bReset())
        {
            do
            {
                bCheckField
                (
                    dvclsfCur
                    , cursDrvFlds.objRCur()
                    , colErrors
                    , strError
                    , kCIDLib::True
                );

                // Mark this one as having been looked at
                cursDrvFlds.objWCur().bFlag(kCIDLib::True);
            }   while (cursDrvFlds.bNext());
        }
    }
     else if (m_eNameType == ENameTypes::Pattern)
    {
        //
        //  Go through all of the driver's fields for our class. We check that the
        //  match one of the device fields.
        //
        TFldDefSet::TNCCursor cursDrvFlds(&m_colOurFlds);
        if (cursDrvFlds.bReset())
        {
            tCQCKit::EDevClasses eDummy;
            TString strSub;
            TString strSuffix;

            do
            {
                TCQCFldDef& flddCur = cursDrvFlds.objWCur();
                const TString& strCurName = flddCur.strName();

                // Mark this one as having been looked at
                flddCur.bFlag(kCIDLib::True);

                //
                //  Iterate all of the fields until we find one whose reg expression
                //  matches our field name.
                //
                //  NOTE that we are only matching the suffix part of the name here,
                //  so we have to parse out the parts of the field name. The names have
                //  already been parsed before, so we know they are valid.
                //
                if (!facCQCKit().bParseFldName(flddCur.strName(), eDummy, strSub, strSuffix))
                {
                    CIDAssert2(L"Already parsed field name failed to parse")
                }

                // And just for funxies
                CIDAssert(strSub == strSubToCheck, L"Sub-unit mismatch during validation");

                tCIDLib::TCard4 c4ClsFldInd = 0;
                for (; c4ClsFldInd < c4ClsFldCnt; c4ClsFldInd++)
                {
                    const TCQCDevClFld& dvclsfCur = *m_pcolFldList->pobjAt(c4ClsFldInd);
                    if (dvclsfCur.m_regxPat.bFullyMatches(strSuffix, kCIDLib::True))
                        break;
                }

                if (c4ClsFldInd == c4ClsFldCnt)
                {
                    // No match, so an error
                    strError = m_strClassPref;
                    strError.Append(L" - No match for field name '");
                    strError.Append(flddCur.strName());
                    strError.Append(L"'");
                    colErrors.objAdd(strError);
                }
                 else
                {
                    // Check the rest of the stuff against any configs for this field
                    bCheckField
                    (
                        *m_pcolFldList->pobjAt(c4ClsFldInd)
                        , cursDrvFlds.objRCur()
                        , colErrors
                        , strError
                        , kCIDLib::True
                    );
                }
            }   while (cursDrvFlds.bNext());
        }

    }
     else if (m_eNameType == ENameTypes::Fixed)
    {
        //
        //  In this case, we first go through all of the class field definitions, and
        //  every one of them must be present by name. If found, we then check that it
        //  matches the specification for that named field in the device class.
        //
        TString strExpName;
        for (tCIDLib::TCard4 c4ClsFldInd = 0; c4ClsFldInd < c4ClsFldCnt; c4ClsFldInd++)
        {
            const TCQCDevClFld& dvclsfCur = *m_pcolFldList->pobjAt(c4ClsFldInd);

            // Build up the expected name
            strExpName = m_strClassPref;
            strExpName.Append(kCIDLib::chPoundSign);
            if (!strSubToCheck.bIsEmpty())
            {
                strExpName.Append(strSubToCheck);
                strExpName.Append(kCIDLib::chTilde);
            }
            strExpName.Append(dvclsfCur.m_strName);

            // Look up this field. If it doesn't exist, then obviously this one fails
            TCQCFldDef* pflddCur = m_colOurFlds.pobjFindByKey(strExpName, kCIDLib::False);
            if (!pflddCur)
            {
                strError = m_strClassPref;
                strError.Append(L" - Missing field. Name=");
                strError.Append(strExpName);
                colErrors.objAdd(strError);

                // No need checking this one further
                continue;
            }

            // Check the non-name stuff
            bCheckField(dvclsfCur, *pflddCur, colErrors, strError, kCIDLib::True);

            // Mark this one as having been processed
            pflddCur->bFlag(kCIDLib::True);
        }
    }
     else
    {
        CIDAssert2(L"Unknown device class field naming type")
    }
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCV2Validator
// PREFIX: cv2v
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCV2Validator: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCV2Validator::TCQCV2Validator()
{
}

TCQCV2Validator::~TCQCV2Validator()
{
}


// ---------------------------------------------------------------------------
//  TCQCV2Validator: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCV2Validator::bDoV2Validation(const  TCQCDriverCfg&          cqcdcTest
                                , const tCQCKit::TFldDefList&   colFields
                                ,       tCIDLib::TStrCollect&   colErrors)
{
    TString strErr;

    // Check some obvious stuff that would prevent us from doing anything useful
    if (cqcdcTest.c4ArchVersion() < 2)
    {
        strErr = L"[Driver] - This is not a V2 driver";
        colErrors.objAdd(strErr);
        return kCIDLib::False;
    }

    // Get the list of device classes this guy implements
    if (cqcdcTest.fcolDevClasses().bIsEmpty())
    {
        strErr = L"[Driver] - The driver is V2 but doesn't implement any classes";
        colErrors.objAdd(strErr);
        return kCIDLib::False;
    }

    if (colFields.c4ElemCount() <= tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Count))
    {
        strErr = L"[Driver] - No fields are registered yet";
        colErrors.objAdd(strErr);
        return kCIDLib::False;
    }


    //
    //  Do some global checks before we start doing per-device class checks. One
    //  thing we want to do is to go through all of the fields and find all of the
    //  device classes it would appear the driver implements, based on prefixes
    //  used. Then we can check to see if the manifest indicates all of those
    //  classes.
    //
    //  Whlie doing this, we also store all of the V2 fields. These are the only ones
    //  that we need to deal with here.
    //
    tCQCKit::TFldDefList    colV2Fields;
    {
        tCQCKit::TDevClassList  fcolClassesFnd;
        {
            const tCIDLib::TCard4   c4Count = colFields.c4ElemCount();
            tCQCKit::EDevClasses    eFldClass;
            TString                 strSub;
            TString                 strSuffix;

            const tCQCKit::TDevClassList& fcolSrcClasses = cqcdcTest.fcolDevClasses();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TCQCFldDef& flddCur = colFields[c4Index];
                const TString& strCurName = flddCur.strName();

                if (!facCQCKit().bParseFldName(strCurName, eFldClass, strSub, strSuffix))
                {
                    strErr = L"Already parsed field name failed to parse. Name=";
                    strErr.Append(flddCur.strName());
                    CIDAssert2(strErr)
                }

                // If we don't have this one yet, then add it to our temp list
                if ((eFldClass != tCQCKit::EDevClasses::None)
                &&  !fcolClassesFnd.bElementPresent(eFldClass))
                {
                    fcolClassesFnd.c4AddElement(eFldClass);
                }

                if (eFldClass != tCQCKit::EDevClasses::None)
                    colV2Fields.objAdd(flddCur);
            }

            //
            //  OK, now for every one we found, make sure it is in the list from the
            //  driver configuration.
            //
            const tCIDLib::TCard4 c4FndCnt = fcolClassesFnd.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FndCnt; c4Index++)
            {
                if (!fcolSrcClasses.bElementPresent(fcolClassesFnd[c4Index]))
                {
                    strErr = L"Driver has fields for class ";
                    strErr.Append(tCQCKit::strXlatEDevClasses(fcolClassesFnd[c4Index]));
                    strErr.Append(L" but the manifest doesn't list it");
                    colErrors.objAdd(strErr);
                }
            }
        }
    }

    // Remember how many source fields (the V2 ones we care about)
    const tCIDLib::TCard4 c4SrcFldCnt = colV2Fields.c4ElemCount();

    // From here forward we use the list of dev classes in the driver config
    const tCQCKit::TDevClassList& fcolSrcClasses = cqcdcTest.fcolDevClasses();

    //
    //  Do a quick check to see if we have all of the device classes this driver
    //  implements. This way we can avoid having to create an installation server
    //  proxy every time we are invoked.
    //
    tCIDLib::TBoolean bGotAllClasses = kCIDLib::True;
    {
        const tCIDLib::TCard4 c4Count = fcolSrcClasses.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (c4FindDevClass(fcolSrcClasses[c4Index]) == kCIDLib::c4MaxCard)
            {
                // We are missing at least one, so downloads are required
                bGotAllClasses = kCIDLib::False;
                break;
            }
        }
    }

    // If we haven't loaded all of the device classes yet, then do so
    if (!bGotAllClasses)
    {
        tCIDLib::TCard4 c4Bytes;
        TPathStr        pathClsFile;
        try
        {
            THeapBuf mbufTmp(16 * 1024, 64 * 1024);
            TCQCDevClass dvclsCur;

            const tCIDLib::TCard4 c4Count = fcolSrcClasses.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                // If we don't have this device class, then download it
                const tCQCKit::EDevClasses eClass = fcolSrcClasses[c4Index];

                const tCIDLib::TCard4 c4ClsInd = c4FindDevClass(eClass);
                if (c4ClsInd == kCIDLib::c4MaxCard)
                {
                    // Don't have it yet, so let's pull it down
                    pathClsFile = facCQCKit().strLocalDataDir();
                    pathClsFile.AddLevel(L"DevClasses");
                    pathClsFile.AddLevel(L"CQCDevCls_");
                    pathClsFile.Append(tCQCKit::strAltXlatEDevClasses(eClass));
                    pathClsFile.AppendExt(L"xml");

                    try
                    {
                        TBinaryFile flSrc(pathClsFile);
                        flSrc.Open
                        (
                            tCIDLib::EAccessModes::Read
                            , tCIDLib::ECreateActs::OpenIfExists
                            , tCIDLib::EFilePerms::Default
                            , tCIDLib::EFileFlags::SequentialScan
                        );

                        c4Bytes = tCIDLib::TCard4(flSrc.c8CurSize());
                        flSrc.c4ReadBuffer(mbufTmp, c4Bytes, tCIDLib::EAllData::FailIfNotAll);

                        // Add it to our list
                        if (dvclsCur.bParseFrom(eClass, mbufTmp, c4Bytes))
                        {
                            m_colDevClasses.objAdd(dvclsCur);
                        }
                         else
                        {
                            // Something went awry, so warn the user
                            strErr = L"[Internal] - ";
                            strErr.Append(L"Could not query or parse device class ");
                            strErr.Append(tCQCKit::strXlatEDevClasses(eClass));
                            colErrors.objAdd(strErr);
                        }
                    }

                    catch(const TError& errToCatch)
                    {
                        strErr = L"[Internal] - ";
                        strErr.Append(L"Error parsing device class ");
                        strErr.Append(tCQCKit::strXlatEDevClasses(eClass));
                        strErr.Append(L". Err=");
                        strErr.Append(errToCatch.strErrText());
                        colErrors.objAdd(strErr);
                    }
                }
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            strErr = L"An error occurred while loading classes. Error=";
            strErr.Append(errToCatch.strErrText());
            colErrors.objAdd(strErr);
            return kCIDLib::False;
        }
    }

    //
    //  OK, let's let any of the implemented device classes validate the field list.
    //  As we go, we remember if any of them require Power. We'll check that at the
    //  end.
    //
    tCIDLib::TBoolean bNeedsPower = kCIDLib::False;
    const tCIDLib::TCard4 c4ClsCnt = fcolSrcClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ClsCnt; c4Index++)
    {
        // Get the current device class to process
        const tCQCKit::EDevClasses eCurClass = fcolSrcClasses[c4Index];

        // Find this class definition in the passed list
        tCIDLib::TCard4 c4ClsDefInd = c4FindDevClass(eCurClass);

        // If not found, put out an error and try the next one
        if (c4ClsDefInd == kCIDLib::c4MaxCard)
        {
            strErr = L"Could not find device class ";
            strErr.Append(tCQCKit::strXlatEDevClasses(eCurClass));
            colErrors.objAdd(strErr);
            continue;
        }

        //
        //  Don't really care about the return, just the errors into the list. We
        //  pass in the V2 fields only here.
        //
        const TCQCDevClass& dvclsCur = m_colDevClasses[c4ClsDefInd];
        dvclsCur.bValidateFlds(colV2Fields, colErrors);

        bNeedsPower |= dvclsCur.bNeedsPower();
    }

    //
    //  For device classes that are fixed field names, the validation involves
    //  looking for a field that matches each defined field for the devic class
    //  defintion. But, that can still leave fields that were defined, but aren't
    //  correct. They could just be bogus, or they could be badly spelled versions
    //  of a correct field.
    //
    //  All of the fields that are actually processed are marked, and the non-fixed
    //  types also mark theirs. So any unmarked fields at this point are bogus.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SrcFldCnt; c4Index++)
    {
        if (!colV2Fields[c4Index].bFlag())
        {
            strErr= L"[OTHER] - Field '";
            strErr.Append(colV2Fields[c4Index].strName());
            strErr.Append(L"' was not processed by any class.");
            colErrors.objAdd(strErr);
        }
    }

    // If power was needed and not implemented, then complain
    if (bNeedsPower && !fcolSrcClasses.bElementPresent(tCQCKit::EDevClasses::Power))
    {
        strErr = L"The Power device class is required but was not implemented";
        colErrors.objAdd(strErr);
    }

    // If no errors, then we passed
    return colErrors.bIsEmpty();
}


// ---------------------------------------------------------------------------
//  TCQCV2Validator: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Look up a class in our list of so far downloaded classes
tCIDLib::TCard4
TCQCV2Validator::c4FindDevClass(const tCQCKit::EDevClasses eClass) const
{
    const tCIDLib::TCard4 c4Count = m_colDevClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colDevClasses[c4Index].eDevClass() == eClass)
            return c4Index;
    }
    return kCIDLib::c4MaxCard;
}

