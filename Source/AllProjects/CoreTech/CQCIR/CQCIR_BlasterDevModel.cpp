//
// FILE NAME: CQCIR_BlasterDevModel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2002
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
//  This class implements the IR device object, which holds the info for
//  a device, and a list of the commands it supports.
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
#include    "CQCIR_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TIRBlasterDevModel,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIR_BlasterDevModel
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        //
        //  Version 2:
        //      Added in CQC version 1.1.1, to support the addition of a per-
        //      device repeat count.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 2;


        // -----------------------------------------------------------------------
        //  The hash modulus we use for our command map
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4       c4Modulus = 29;


        // -----------------------------------------------------------------------
        //  The default repeat count we set until the user should choose to change
        //  it.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4       c4DefRepeatCount = 2;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TIRBlasterDevModel
//  PREFIX: irbdm
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIRBlasterDevModel: Public, static methods
// ---------------------------------------------------------------------------
const TString&
TIRBlasterDevModel::strKey(const TIRBlasterDevModel& irbdmSrc)
{
    return irbdmSrc.m_strModel;
}


//
//  In order to allow the data server to quickly build up an in-memory key
//  collection, we provide this method to read just the header info from a
//  streamed device. It returns the format version found.
//
tCIDLib::TVoid
TIRBlasterDevModel::StreamHeader(TBinInStream&          strmSrc
                                , tCIDLib::TCard4&      c4RepeatCount
                                , TString&              strDevModel
                                , TString&              strDevMake
                                , TString&              strDescr
                                , tCQCKit::EDevCats&    eCategory)
{
    // Make sure we get the start obj marker
    strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmSrc >> c2FmtVersion;
    if (c2FmtVersion > CQCIR_BlasterDevModel::c2FmtVersion)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    //
    //  If we are at version 2 or greater, then read in the repeat count.
    //  Else, we need to default it.
    //
    if (c2FmtVersion >= 2)
        strmSrc >> c4RepeatCount;
    else
        c4RepeatCount = CQCIR_BlasterDevModel::c4DefRepeatCount;

    // Get the other stuff now
    strmSrc >> strDevModel
            >> strDevMake
            >> strDescr
            >> eCategory;
}



// ---------------------------------------------------------------------------
//  TIRBlasterDevModel: Constructors and Destructor
// ---------------------------------------------------------------------------
TIRBlasterDevModel::TIRBlasterDevModel() :

    m_c4RepeatCount(CQCIR_BlasterDevModel::c4DefRepeatCount)
    , m_colCmdMap
      (
        CQCIR_BlasterDevModel::c4Modulus
        , TStringKeyOps()
        , &TIRBlasterCmd::strKey
      )
    , m_eCategory(tCQCKit::EDevCats::Miscellaneous)
{
}

TIRBlasterDevModel::TIRBlasterDevModel( const   TString&            strModel
                                        , const tCQCKit::EDevCats   eCategory) :

    m_c4RepeatCount(CQCIR_BlasterDevModel::c4DefRepeatCount)
    , m_colCmdMap
      (
        CQCIR_BlasterDevModel::c4Modulus
        , TStringKeyOps()
        , &TIRBlasterCmd::strKey
      )
    , m_eCategory(eCategory)
    , m_strModel(strModel)
{
}

TIRBlasterDevModel::TIRBlasterDevModel( const   TString&            strModel
                                        , const TString&            strMake
                                        , const TString&            strDescr
                                        , const tCQCKit::EDevCats   eCategory) :

    m_c4RepeatCount(CQCIR_BlasterDevModel::c4DefRepeatCount)
    , m_colCmdMap
      (
        CQCIR_BlasterDevModel::c4Modulus
        , TStringKeyOps()
        , &TIRBlasterCmd::strKey
      )
    , m_eCategory(eCategory)
    , m_strDescr(strDescr)
    , m_strMake(strMake)
    , m_strModel(strModel)
{
}

TIRBlasterDevModel::~TIRBlasterDevModel()
{
}


// ---------------------------------------------------------------------------
//  TIRBlasterDevModel: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIRBlasterDevModel::bCmdExists(const TString& strToFind) const
{
    return m_colCmdMap.bKeyExists(strToFind);
}


tCIDLib::TCard4 TIRBlasterDevModel::c4RepeatCount() const
{
    return m_c4RepeatCount;
}

tCIDLib::TCard4
TIRBlasterDevModel::c4RepeatCount(const tCIDLib::TCard4 c4ToSet)
{
    m_c4RepeatCount = c4ToSet;
    return m_c4RepeatCount;
}


TIRBlasterDevModel::TCmdCursor TIRBlasterDevModel::cursCmds() const
{
    return TCmdCursor(&m_colCmdMap);
}


tCQCKit::EDevCats TIRBlasterDevModel::eCategory() const
{
    return m_eCategory;
}

tCQCKit::EDevCats TIRBlasterDevModel::eCategory(const tCQCKit::EDevCats eToSet)
{
    m_eCategory = eToSet;
    return m_eCategory;
}


TIRBlasterCmd&
TIRBlasterDevModel::irbcAdd(const   TString&        strName
                            , const TMemBuf&        mbufData
                            , const tCIDLib::TCard4 c4DataBytes)
{
    TIRBlasterCmd& irbcNew = m_colCmdMap.objAdd(TIRBlasterCmd(strName));
    irbcNew.SetData(mbufData, c4DataBytes);
    return irbcNew;
}


TIRBlasterCmd&
TIRBlasterDevModel::irbcAddOrUpdate(const   TIRBlasterCmd&      irbcToAdd
                                    ,       tCIDLib::TBoolean&  bAdded)
{
    return m_colCmdMap.objAddOrUpdate(irbcToAdd, bAdded);
}


const TIRBlasterCmd&
TIRBlasterDevModel::irbcFromName(const TString& strCmdName) const
{
    const TIRBlasterCmd* pirbcRet = m_colCmdMap.pobjFindByKey(strCmdName);
    if (!pirbcRet)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcCfg_UnknownCmd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strCmdName
            , m_strModel
        );
    }
    return *pirbcRet;
}

TIRBlasterCmd& TIRBlasterDevModel::irbcFromName(const TString& strCmdName)
{
    TIRBlasterCmd* pirbcRet = m_colCmdMap.pobjFindByKey(strCmdName);
    if (!pirbcRet)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcCfg_UnknownCmd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strCmdName
            , m_strModel
        );
    }
    return *pirbcRet;
}


tCIDLib::TVoid
TIRBlasterDevModel::LoadFromFile(const  TString&            strSrcFile
                                , const TString&            strExpBlaster
                                , const tCIDLib::TBoolean   bAllowDups)
{
    //
    //  Create the parser and try to parse it. Allow any exceptions to
    //  propogate.
    //
    TXMLTreeParser xtprsModel;
    const tCIDLib::TBoolean bOk = xtprsModel.bParseRootEntity
    (
        strSrcFile
        , tCIDXML::EParseOpts::None
        , tCIDXML::EParseFlags::Tags
    );

    // If not ok, then return the first error that occured
    if (!bOk)
    {
        //
        //  Format the first error in the error list, into a string that
        //  we can pass as a replacement parameter to the real error we
        //  log.
        //
        const TXMLTreeParser::TErrInfo& erriFirst
                                        = xtprsModel.colErrors()[0];

        TTextStringOutStream strmOut(512);
        strmOut << L"[" << erriFirst.strSystemId() << L"/"
                << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                << L"] " << erriFirst.strText() << kCIDLib::EndLn;

        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcImp_ParseFailed
            , strmOut.strData()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    //
    //  Ok, we now have the document with the validated system config
    //  data in it. So lets go through it and get the data out. We start
    //  by getting the root element node from the document.
    //
    const TXMLTreeElement& xtnodeRoot = xtprsModel.xtdocThis().xtnodeRoot();

    //
    //  The most fundamental thing is to make sure that this was exported
    //  from the same blaster type.
    //
    tCIDLib::TCard4 c4AttrInd;
    if (!xtnodeRoot.bAttrExists(L"CQCIR:Blaster", c4AttrInd))
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcImp_BadRootAttrs
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    const TString& strSrcDev = xtnodeRoot.xtattrAt(c4AttrInd).strValue();
    if (!strExpBlaster.bIsEmpty() && (strSrcDev != strExpBlaster))
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcImp_WrongBlaster
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strSrcDev
            , strExpBlaster
        );
    }

    // Ok, we got it so let's parse it in
    Parse(xtnodeRoot, bAllowDups);
}


tCIDLib::TVoid TIRBlasterDevModel::RemoveCmd(const TString& strCmdName)
{
    m_colCmdMap.bRemoveKey(strCmdName);
}


const TString& TIRBlasterDevModel::strDescription() const
{
    return m_strDescr;
}

const TString& TIRBlasterDevModel::strDescription(const TString& strNew)
{
    m_strDescr = strNew;
    return m_strDescr;
}


const TString& TIRBlasterDevModel::strMake() const
{
    return m_strMake;
}

const TString& TIRBlasterDevModel::strMake(const TString& strNew)
{
    m_strMake = strNew;
    return m_strMake;
}


const TString& TIRBlasterDevModel::strModel() const
{
    return m_strModel;
}

const TString& TIRBlasterDevModel::strModel(const TString& strNew)
{
    m_strModel = strNew;
    return m_strModel;
}


// ---------------------------------------------------------------------------
//  TIRBlasterDevModel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIRBlasterDevModel::StreamFrom(TBinInStream& strmToReadFrom)
{
    //
    //  We have a static method that will stream in the header stuff, which is
    //  done so that the data server can quickly scan the directory and build
    //  up an in-memory key collection.
    //
    StreamHeader
    (
        strmToReadFrom
        , m_c4RepeatCount
        , m_strModel
        , m_strMake
        , m_strDescr
        , m_eCategory
    );

    //
    //  Now pull in the cmds, if any. Flush our map, and check for the
    //  frame marker we should see. Then load up the command map collection.
    //
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
    m_colCmdMap.RemoveAll();
    strmToReadFrom >> m_colCmdMap;

    // Make sure we get the end obj marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TIRBlasterDevModel::StreamTo(TBinOutStream& strmToWriteTo) const
{
    //
    //  We keep the header info separate from the command info, so that the
    //  data server can scan the files and only read in the header part to
    //  find out the info it needs to provide the server side device model
    //  cache.
    //
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIR_BlasterDevModel::c2FmtVersion
                    << m_c4RepeatCount
                    << m_strModel
                    << m_strMake
                    << m_strDescr
                    << m_eCategory
                    << tCIDLib::EStreamMarkers::Frame
                    << m_colCmdMap
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TIRBlasterDevModel: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TIRBlasterDevModel::Parse(  const   TXMLTreeElement&    xtnodeRoot
                            , const tCIDLib::TBoolean   bAllowDups)
{
    // Make sure the root element is ok
    if (xtnodeRoot.strQName() != L"CQCIR:ExportFile")
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcImp_BadRootElem
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , xtnodeRoot.strQName()
        );
    }

    // Get out the four attributes that we expect
    tCIDLib::TCard4 c4CatInd = 0;
    tCIDLib::TCard4 c4DescrInd = 0;
    tCIDLib::TCard4 c4MakeInd = 0;
    tCIDLib::TCard4 c4ModelInd = 0;
    if (!xtnodeRoot.bAttrExists(L"CQCIR:Category", c4CatInd)
    ||  !xtnodeRoot.bAttrExists(L"CQCIR:Description", c4DescrInd)
    ||  !xtnodeRoot.bAttrExists(L"CQCIR:Make", c4MakeInd)
    ||  !xtnodeRoot.bAttrExists(L"CQCIR:Model", c4ModelInd))
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcImp_BadRootAttrs
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // They are present, so set the model object with them
    m_strMake = xtnodeRoot.xtattrAt(c4MakeInd).strValue();
    m_strModel = xtnodeRoot.xtattrAt(c4ModelInd).strValue();
    m_strDescr = xtnodeRoot.xtattrAt(c4DescrInd).strValue();
    m_eCategory = tCQCKit::eXlatEDevCats(xtnodeRoot.xtattrAt(c4CatInd).strValue());
    if (m_eCategory >= tCQCKit::EDevCats::Max)
        m_eCategory = tCQCKit::EDevCats::Miscellaneous;

    //
    //  We can optionally have a repeat count. If not, then we will default
    //  it.
    //
    tCIDLib::TCard4 c4RepeatInd;
    if (xtnodeRoot.bAttrExists(L"CQCIR:RepeatCount", c4RepeatInd))
        m_c4RepeatCount = xtnodeRoot.xtattrAt(c4RepeatInd).c4ValueAs();
    else
        m_c4RepeatCount = CQCIR_BlasterDevModel::c4DefRepeatCount;


    // Next we have to see the 0th child which is the CQCIR:Commands element
    const TXMLTreeElement& xtnodeCmds = xtnodeRoot.xtnodeChildAtAsElement(0);
    if (xtnodeCmds.strQName() != L"CQCIR:Commands")
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcImp_BadCmdsElem
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , xtnodeCmds.strQName()
        );
    }

    //
    //  And now let's loop through any command elements, which are all of the
    //  children of the root node.
    //
    const tCIDLib::TCard4 c4Count = xtnodeCmds.c4ChildCount();
    tCIDLib::TBoolean bAdded;
    TIRBlasterCmd irbcNew;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TXMLTreeElement& xtnodeCur = xtnodeCmds.xtnodeChildAtAsElement(c4Index);
        if (xtnodeCur.strQName() != L"CQCIR:Cmd")
        {
            facCQCIR().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIRErrs::errcImp_BadCmdElem
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , xtnodeCur.strQName()
            );
        }

        // It has to be a CQCIR:Cmd element
        tCIDLib::TCard4 c4CmdInd = 0;
        tCIDLib::TCard4 c4DataInd = 0;
        if (!xtnodeCur.bAttrExists(L"CQCIR:CmdName", c4CmdInd)
        ||  !xtnodeCur.bAttrExists(L"CQCIR:Data", c4DataInd))
        {
            facCQCIR().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIRErrs::errcImp_BadCmdAttrs
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        // Looks reasonable, so set it and try to add it
        irbcNew.Set
        (
            xtnodeCur.xtattrAt(c4CmdInd).strValue()
            , xtnodeCur.xtattrAt(c4DataInd).strValue()
        );
        irbcAddOrUpdate(irbcNew, bAdded);

        // If it wasn't added, it's duplicate, so reject
        if (!bAdded && !bAllowDups)
        {
            facCQCIR().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIRErrs::errcImp_DuplicateCmd
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , irbcNew.strName()
            );
        }
    }
}


