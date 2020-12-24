//
// FILE NAME: ZWaveLevi2Sh_DevInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/23/2014
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
//  This file provides the implementation for the TZWDevInfo class, which is a small
//  utility class used to stream in and manage the info from the ZWInfo files that
//  describe the capabilities of each Z-Wave device we support.
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
//   CLASS: TZWDevClass
//  PREFIX: zwdc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWDevClass: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TZWDevClass::eComp( const   TZWDevClass&    zwdc1
                    , const TZWDevClass&    zwdc2)
{
    if (zwdc1.m_eClass < zwdc2.m_eClass)
        return tCIDLib::ESortComps::FirstLess;
    else if (zwdc2.m_eClass < zwdc1.m_eClass)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


// ---------------------------------------------------------------------------
//  TZWDevClass: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWDevClass::TZWDevClass() :

    m_bAllZeroInst(kCIDLib::False)
    , m_bCanRead(kCIDLib::False)
    , m_bCanWrite(kCIDLib::False)
    , m_bPoll(kCIDLib::False)
    , m_bSecure(kCIDLib::False)
    , m_c4Flags(0)
    , m_c4InstId(0)
    , m_c4PollMins(0)
    , m_c4SetRespCls(0)
    , m_c4SetRespCmd(0)
    , m_eClass(tZWaveLevi2Sh::ECClasses::None)
    , m_eMultiType(tZWaveLevi2Sh::EMultiTypes::Single)
{
}

TZWDevClass::TZWDevClass(const TZWDevClass& zwdcSrc) :

    m_bAllZeroInst(zwdcSrc.m_bAllZeroInst)
    , m_bCanRead(zwdcSrc.m_bCanRead)
    , m_bCanWrite(zwdcSrc.m_bCanWrite)
    , m_bPoll(zwdcSrc.m_bPoll)
    , m_bSecure(zwdcSrc.m_bSecure)
    , m_c4Flags(zwdcSrc.m_c4Flags)
    , m_c4InstId(zwdcSrc.m_c4InstId)
    , m_c4PollMins(zwdcSrc.m_c4PollMins)
    , m_c4SetRespCls(zwdcSrc.m_c4SetRespCls)
    , m_c4SetRespCmd(zwdcSrc.m_c4SetRespCmd)
    , m_colOpts(zwdcSrc.m_colOpts)
    , m_eClass(zwdcSrc.m_eClass)
    , m_eMultiType(zwdcSrc.m_eMultiType)
    , m_strDescr(zwdcSrc.m_strDescr)
    , m_strName(zwdcSrc.m_strName)
{
}

TZWDevClass::~TZWDevClass()
{
}


// ---------------------------------------------------------------------------
//  TZWDevClass: Public operators
// ---------------------------------------------------------------------------
TZWDevClass& TZWDevClass::operator=(const TZWDevClass& zwdcSrc)
{
    if (&zwdcSrc != this)
    {
        m_bAllZeroInst  = zwdcSrc.m_bAllZeroInst;
        m_bCanRead      = zwdcSrc.m_bCanRead;
        m_bCanWrite     = zwdcSrc.m_bCanWrite;
        m_bPoll         = zwdcSrc.m_bPoll;
        m_bSecure       = zwdcSrc.m_bSecure;
        m_c4Flags       = zwdcSrc.m_c4Flags;
        m_c4InstId      = zwdcSrc.m_c4InstId;
        m_c4PollMins    = zwdcSrc.m_c4PollMins;
        m_c4SetRespCls  = zwdcSrc.m_c4SetRespCls;
        m_c4SetRespCmd  = zwdcSrc.m_c4SetRespCmd;
        m_colOpts       = zwdcSrc.m_colOpts;
        m_eClass        = zwdcSrc.m_eClass;
        m_eMultiType    = zwdcSrc.m_eMultiType;
        m_strDescr      = zwdcSrc.m_strDescr;
        m_strName       = zwdcSrc.m_strName;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TZWDevClass: Public, non-virtual methods
// ---------------------------------------------------------------------------
//
//  We can provide defaults for command class options. So we provide this helper that
//  takes a command class, and the name and standard default value for an option. We
//  will see if we have a default for it in our list. If so, we set it to that, else
//  we use the provided standard default.
//
//  The special case being if the passed option value provides a limited list of
//  values. We check to see if the default value is in that list. If not, then we take
//  the first one in the list.
//
tCIDLib::TVoid
TZWDevClass::QueryOptDef(       TZWOptionVal&   zwoptvTar
                        , const TString&        strKey
                        , const TString&        strDef) const
{
    const tCIDLib::TCard4 c4OptCnt = m_colOpts.c4ElemCount();
    for (tCIDLib::TCard4 c4OptInd = 0; c4OptInd < c4OptCnt; c4OptInd++)
    {
        const TZWOptionVal& zwoptvCur = m_colOpts[c4OptInd];
        if (zwoptvCur.m_strKey == strKey)
        {
            // We found it so store the option value and return
            zwoptvTar = zwoptvCur;
            return;
        }
    }

    //
    //  Never found it so take the proposed default, as long as it is within the
    //  list of allowed values (or allowed values list is empty.)
    //
    //  We set fixed to false, since this is called from the command class objects,
    //  which never set fixed options, else they'd not bother to have it as an option.
    //
    TString strVal;
    const tCIDLib::TCard4 c4AllowedCnt = zwoptvTar.m_colAllowedVals.c4ElemCount();
    if (c4AllowedCnt)
    {
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4AllowedCnt)
        {
            if (zwoptvTar.m_colAllowedVals[c4Index] == strDef)
                break;
            c4Index++;
        }

        // If in the list, we can take it, else take the first list value
        if (c4Index < c4AllowedCnt)
            strVal = strDef;
        else
            strVal = zwoptvTar.m_colAllowedVals[0];
    }
     else
    {
        // No allowed values, so take the default
        strVal = strDef;
    }

    zwoptvTar.Set(strKey, strDef, kCIDLib::False, TString::strEmpty());
}


tCIDLib::TVoid TZWDevClass::Reset(const tZWaveLevi2Sh::ECClasses eClass)
{
    m_bAllZeroInst = kCIDLib::False;
    m_bCanRead = kCIDLib::True;
    m_bCanWrite = kCIDLib::True;
    m_bPoll = kCIDLib::False;
    m_bSecure = kCIDLib::False;
    m_c4Flags = 0;
    m_c4InstId = 0;
    m_c4PollMins = 0;
    m_c4SetRespCls = 0;
    m_c4SetRespCmd = 0;
    m_colOpts.RemoveAll();
    m_eClass = eClass;
    m_eMultiType = tZWaveLevi2Sh::EMultiTypes::Single;
    m_strDescr.Clear();
    m_strName.Clear();
}





// ---------------------------------------------------------------------------
//   CLASS: TZWDevInfo
//  PREFIX: zwdi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWDevInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TZWDevInfo::strKey(const TZWDevInfo& zwdiSrc)
{
    return zwdiSrc.m_strFileName;
}


// ---------------------------------------------------------------------------
//  TZWDevInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWDevInfo::TZWDevInfo()
{
    // Call reset to get to default state
    Reset();
}

TZWDevInfo::TZWDevInfo(const TZWDevInfo& zwdiSrc) :

    m_bAES128(zwdiSrc.m_bAES128)
    , m_bBeam(zwdiSrc.m_bBeam)
    , m_bLoaded(zwdiSrc.m_bLoaded)
    , m_bNetSec(zwdiSrc.m_bNetSec)
    , m_colAutoCfg(zwdiSrc.m_colAutoCfg)
    , m_colClasses(zwdiSrc.m_colClasses)
    , m_eBaseClass(zwdiSrc.m_eBaseClass)
    , m_eGenType(zwdiSrc.m_eGenType)
    , m_eSpecType(zwdiSrc.m_eSpecType)
    , m_fcolGroups(zwdiSrc.m_fcolGroups)
    , m_strDevName(zwdiSrc.m_strDevName)
    , m_strFileName(zwdiSrc.m_strFileName)
    , m_strMake(zwdiSrc.m_strMake)
    , m_strModel(zwdiSrc.m_strModel)
{
}

TZWDevInfo::~TZWDevInfo()
{
}


// ---------------------------------------------------------------------------
//  TZWDevInfo: Public operators
// ---------------------------------------------------------------------------
TZWDevInfo& TZWDevInfo::operator=(const TZWDevInfo& zwdiSrc)
{
    if (this != &zwdiSrc)
    {
        m_bAES128     = zwdiSrc.m_bAES128;
        m_bBeam       = zwdiSrc.m_bBeam;
        m_bLoaded     = zwdiSrc.m_bLoaded;
        m_bNetSec     = zwdiSrc.m_bNetSec;
        m_colAutoCfg  = zwdiSrc.m_colAutoCfg;
        m_colClasses  = zwdiSrc.m_colClasses;
        m_eBaseClass  = zwdiSrc.m_eBaseClass;
        m_eGenType    = zwdiSrc.m_eGenType;
        m_eSpecType   = zwdiSrc.m_eSpecType;
        m_fcolGroups  = zwdiSrc.m_fcolGroups;
        m_strDevName  = zwdiSrc.m_strDevName;
        m_strFileName = zwdiSrc.m_strFileName;
        m_strMake     = zwdiSrc.m_strMake;
        m_strModel    = zwdiSrc.m_strModel;
    }
    return *this;
}



// ---------------------------------------------------------------------------
//  TZWDevInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We get the root element of the ZWInfo file. Most of the content is in the form
//  of attributes on the root, so there's not a lot to do.
//
tCIDLib::TVoid
TZWDevInfo::ParseFromXML(const  TString&            strFileName
                        , const TXMLTreeElement&    xtnodeRoot)
{
    m_strFileName = strFileName;

    m_bAES128   = xtnodeRoot.xtattrNamed(L"AES128").bValueAs();
    m_bBeam     = xtnodeRoot.xtattrNamed(L"Beam").bValueAs();
    m_bNetSec   = xtnodeRoot.xtattrNamed(L"NetSec").bValueAs();

    m_strDevName= xtnodeRoot.xtattrNamed(L"Name").strValue();
    m_strMake   = xtnodeRoot.xtattrNamed(L"Make").strValue();
    m_strModel  = xtnodeRoot.xtattrNamed(L"Model").strValue();

    m_eGenType  = tZWaveLevi2Sh::eAltXlatEGenTypes
    (
        xtnodeRoot.xtattrNamed(L"GType").strValue()
    );
    m_eSpecType = tZWaveLevi2Sh::eAltXlatESpecTypes
    (
        xtnodeRoot.xtattrNamed(L"SType").strValue()
    );

    m_eBaseClass = tZWaveLevi2Sh::eAltXlatECClasses
    (
        xtnodeRoot.xtattrNamed(L"BCls").strValue()
    );

    // Get the sub-element that contains the list of classes
    tCIDLib::TCard4 c4At;
    const TXMLTreeElement& xtnodeClasses = xtnodeRoot.xtnodeFindElement
    (
        L"ClsList", 0, c4At
    );

    // If there is a Groups element process that
    m_fcolGroups.RemoveAll();
    const TXMLTreeElement* pxtnodeGrps = xtnodeRoot.pxtnodeFindElement(L"GrpList", 0, c4At);
    if (pxtnodeGrps)
    {
        const tCIDLib::TCard4 c4GrpCnt = pxtnodeGrps->c4ChildCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4GrpCnt; c4Index++)
        {
            const TXMLTreeElement& xtnodeGrp = pxtnodeGrps->xtnodeChildAtAsElement(c4Index);
            m_fcolGroups.c4AddElement(xtnodeGrp.xtattrNamed(L"N").c4ValueAs());
        }
    }

    //
    //  Iterate its children. Each one indicates a class, and one or more instances
    //  of that class.
    //
    m_colClasses.RemoveAll();
    const tCIDLib::TCard4 c4ClsCnt = xtnodeClasses.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ClsCnt; c4Index++)
        ParseClass(xtnodeClasses.xtnodeChildAtAsElement(c4Index));

    // If there's auto-config info, then let's load that up
    m_colAutoCfg.RemoveAll();
    const TXMLTreeElement* pxtnodeAutoCfg = xtnodeRoot.pxtnodeFindElement(L"AutoCfg", 0, c4At);
    if (pxtnodeAutoCfg)
    {
        TKeyValues kvalsCur;
        TString strCParm;

        // We just loop through the children and store them away
        const tCIDLib::TCard4 c4ACCnt = pxtnodeAutoCfg->c4ChildCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ACCnt; c4Index++)
        {
            const TXMLTreeElement& xtnodeAC = pxtnodeAutoCfg->xtnodeChildAtAsElement(c4Index);

            // All of them have the number and description attributes
            const TString& strNum = xtnodeAC.xtattrNamed(L"N").strValue();
            const TString& strDescr = xtnodeAC.xtattrNamed(L"D").strValue();

            // The key is the same as the element name, which indicates the operation type
            kvalsCur.strKey(xtnodeAC.strQName());
            kvalsCur.strVal1(strDescr);
            kvalsCur.strVal2(strNum);

            // If a config param, then we get the value to store
            if (xtnodeAC.strQName() == L"CfgParm")
            {
                strCParm = xtnodeAC.xtattrNamed(L"V").strValue();
                strCParm.Append(L'/');
                strCParm.Append(xtnodeAC.xtattrNamed(L"C").strValue());
                kvalsCur.strVal3(strCParm);
            }
             else if (xtnodeAC.strQName() == L"SceneCfg")
            {
                // Store the scene number
                kvalsCur.strVal3(xtnodeAC.xtattrNamed(L"S").strValue());
            }
             else if (xtnodeAC.strQName() == L"Group")
            {
                kvalsCur.strVal3(TString::strEmpty());
            }
            m_colAutoCfg.objAdd(kvalsCur);
        }
    }


    //
    //  Sort the list so that, when the unit class iterates them and creates the
    //  command class handlers, they are always created in the same order. That makes
    //  equality checks a lot easier.
    //
    m_colClasses.Sort(&TZWDevClass::eComp);

    // Mark us as loaded
    m_bLoaded = kCIDLib::True;
}


// Resets us back to default values
tCIDLib::TVoid TZWDevInfo::Reset()
{
    m_bAES128   = kCIDLib::False;
    m_bBeam     = kCIDLib::False;
    m_bLoaded   = kCIDLib::False;
    m_bNetSec   = kCIDLib::False;
    m_colAutoCfg.RemoveAll();
    m_colClasses.RemoveAll();
    m_eBaseClass = tZWaveLevi2Sh::ECClasses::None;
    m_eGenType  = tZWaveLevi2Sh::EGenTypes::None;
    m_eSpecType = tZWaveLevi2Sh::ESpecTypes::None;
    m_fcolGroups.RemoveAll();
    m_strDevName.Clear();
    m_strFileName.Clear();
    m_strMake.Clear();
    m_strModel.Clear();
}




// ---------------------------------------------------------------------------
//  TZWDevInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  For each command class element, this is called to parse the info and add another
//  class object to our list.
//
tCIDLib::TVoid TZWDevInfo::ParseClass(const TXMLTreeElement& xtnodeClass)
{
    // Translate the class and multi-type to our enum types
    const tZWaveLevi2Sh::ECClasses eClass = tZWaveLevi2Sh::eAltXlatECClasses
    (
        xtnodeClass.xtattrNamed(L"C").strValue()
    );

    const tZWaveLevi2Sh::EMultiTypes eMultiType = tZWaveLevi2Sh::eAltXlatEMultiTypes
    (
        xtnodeClass.xtattrNamed(L"MT").strValue()
    );


    // Get some flags that set this on all instances
    const tCIDLib::TBoolean bAllZInst = xtnodeClass.xtattrNamed(L"AZI").bValueAs();
    const tCIDLib::TBoolean bSecure = xtnodeClass.xtattrNamed(L"Sec").bValueAs();
    const tCIDLib::TBoolean bPoll = xtnodeClass.xtattrNamed(L"P").bValueAs();
    const tCIDLib::TBoolean bCanRead = xtnodeClass.xtattrNamed(L"RD").bValueAs();
    const tCIDLib::TBoolean bCanWrite = xtnodeClass.xtattrNamed(L"WR").bValueAs();
    const tCIDLib::TCard4   c4PollMins = xtnodeClass.xtattrNamed(L"PM").c4ValueAs();
    const tCIDLib::TCard4   c4Flags = xtnodeClass.xtattrNamed(L"FL").c4ValueAs();

    tCIDLib::TCard4 c4SetRespCls = 0;
    tCIDLib::TCard4 c4SetRespCmd = 0;
    const TXMLTreeAttr& xtattrSR = xtnodeClass.xtattrNamed(L"SR");
    if (!xtattrSR.strValue().bIsEmpty())
    {
        TString strSetCls = xtattrSR.strValue();
        TString strSetCmd;
        if (!strSetCls.bSplit(strSetCmd, L'/')
        ||  !strSetCls.bToCard4(c4SetRespCls)
        ||  !strSetCmd.bToCard4(c4SetRespCmd))
        {
            facZWaveLevi2Sh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW2ShErrs::errcDevI_BadSetRep
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , xtnodeClass.xtattrNamed(L"C").strValue()
                , m_strFileName
            );
        }
    }

    TZWDevClass zwdcCur;
    zwdcCur.Reset(eClass);
    zwdcCur.m_bAllZeroInst = bAllZInst;
    zwdcCur.m_bPoll = bPoll;
    zwdcCur.m_bCanRead = bCanRead;
    zwdcCur.m_bCanWrite= bCanWrite;
    zwdcCur.m_bSecure = bSecure;
    zwdcCur.m_c4Flags = c4Flags;
    zwdcCur.m_c4PollMins = c4PollMins;
    zwdcCur.m_c4SetRespCls = c4SetRespCls;
    zwdcCur.m_c4SetRespCmd = c4SetRespCmd;

    // Make sure poll minutes is never less than 5 (if non-zero)
    if (zwdcCur.m_c4PollMins && (zwdcCur.m_c4PollMins< 5))
        zwdcCur.m_c4PollMins = 5;

    // If no explicit instances, just short circuit it
    const tCIDLib::TCard4 c4InstCnt = xtnodeClass.c4ChildCount();
    if (!c4InstCnt)
    {
        m_colClasses.objAdd(zwdcCur);
        return;
    }

    // There's more than one, or there's one there just to provide options
    for (tCIDLib::TCard4 c4InstInd = 0; c4InstInd < c4InstCnt; c4InstInd++)
    {
        const TXMLTreeElement& xtnodeInst = xtnodeClass.xtnodeChildAtAsElement(c4InstInd);

        // Add extra instance related stuff
        zwdcCur.m_c4InstId = xtnodeInst.xtattrNamed(L"I").c4ValueAs();
        zwdcCur.m_eMultiType = eMultiType;
        zwdcCur.m_strName = xtnodeInst.xtattrNamed(L"N").strValue();
        zwdcCur.m_strDescr = xtnodeInst.xtattrNamed(L"D").strValue();

        // See if there are any options
        const tCIDLib::TCard4 c4OptCnt = xtnodeInst.c4ChildCount();
        for (tCIDLib::TCard4 c4OptInd = 0; c4OptInd < c4OptCnt; c4OptInd++)
        {
            const TXMLTreeElement& xtnodeOpt = xtnodeInst.xtnodeChildAtAsElement(c4OptInd);

            zwdcCur.m_colOpts.objAdd
            (
                TZWOptionVal
                (
                    xtnodeOpt.xtattrNamed(L"N").strValue()
                    , xtnodeOpt.xtattrNamed(L"V").strValue()
                    , xtnodeOpt.xtattrNamed(L"F").bValueAs()
                    , xtnodeOpt.xtattrNamed(L"AV").strValue()
                )
            );
        }

        // And now add it
        m_colClasses.objAdd(zwdcCur);
    }
}


