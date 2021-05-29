//
// FILE NAME: CQCKit_VarDrvCfg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/05/2003
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
//  This file implements the TVarDrvCfg class.
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
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TVarDrvCfg,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCDriver_VarDrvCfg
    {
        // -----------------------------------------------------------------------
        //  Our current and previous format versions.
        //
        //  Version 2 -
        //      Not ultimately used anymore. We used to sort the list and we bumped the version
        //      to deal with a sorting issue, but we don't sort it anymore, so it's not even
        //      used but they will all be at 2 by now so we have to keep it.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion     = 2;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TVarDrvCfg
//  PREFIX: cfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TVarDrvCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TVarDrvCfg::TVarDrvCfg() :

    m_colFlds(64)
{
}

TVarDrvCfg::~TVarDrvCfg()
{
}


// ---------------------------------------------------------------------------
//  TVarDrvCfg: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TVarDrvCfg::operator==(const TVarDrvCfg& cfgToComp) const
{
    // If not the same fields, then not equal
    const tCIDLib::TCard4 c4Count = m_colFlds.c4ElemCount();
    if (c4Count != cfgToComp.c4Count())
        return kCIDLib::False;

    // Compare the field def objects
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colFlds[c4Index] != cfgToComp.flddAt(c4Index))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TVarDrvCfg::operator!=(const TVarDrvCfg& cfgToComp) const
{
    return !operator==(cfgToComp);
}


// ---------------------------------------------------------------------------
//  TVarDrvCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We don't allow two fields with the same name, and to avoid further
//  confusion do the check non-case sensitive so that they cannot have
//  'AField' and 'afield'.
//
tCIDLib::TBoolean
TVarDrvCfg::bFieldExists(const TString& strToFind) const
{
    const tCIDLib::TCard4 c4Count = m_colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colFlds[c4Index].strName().bCompareI(strToFind))
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  We add the field to our list and return the index at which we put it, which is just
//  at the the end of the list.
//
tCIDLib::TCard4 TVarDrvCfg::c4AddField(const TCQCFldDef& flddToAdd)
{
    m_colFlds.CheckIsFull(kCQCKit::c4MaxDrvFields, L"Variable driver field list");

    const tCIDLib::TCard4 c4RetVal = m_colFlds.c4ElemCount();
    m_colFlds.objAdd(flddToAdd);
    return c4RetVal;
}


tCIDLib::TCard4 TVarDrvCfg::c4Count() const
{
    return m_colFlds.c4ElemCount();
}


const TVarDrvCfg::TFldInfoList& TVarDrvCfg::colFields() const
{
    return m_colFlds;
}

TVarDrvCfg::TFldInfoList& TVarDrvCfg::colFields()
{
    return m_colFlds;
}


TVarDrvCfg::TFldCursor TVarDrvCfg::cursFields() const
{
    return TFldCursor(&m_colFlds);
}


const TCQCFldDef& TVarDrvCfg::flddAt(const tCIDLib::TCard4 c4At) const
{
    return m_colFlds[c4At];
}

TCQCFldDef& TVarDrvCfg::flddAt(const tCIDLib::TCard4 c4At)
{
    return m_colFlds[c4At];
}


TCQCFldDef* TVarDrvCfg::pflddFind(const TString& strToFind)
{
    const tCIDLib::TCard4 c4Count = m_colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCFldDef& flddCur = m_colFlds[c4Index];

        if (flddCur.strName() == strToFind)
            return &flddCur;
    }
    return 0;
}


//
//  This will fill ourself in based on the contents of the passed file,
//  which is assumed to be an output file from the CQCExtractDrvFlds
//  utility.
//
tCIDLib::TVoid
TVarDrvCfg::ParseFromXML(const TString& strSrcFile)
{
    // A simple inline DTD we'll force the parser to use
    static TString strDTD
    (
        L"<?xml encoding='$NativeWideChar$'?>\n"
        L"<!ELEMENT CQCFldDef (#PCDATA)>\n"
        L"<!ATTLIST CQCFldDef Type CDATA #REQUIRED\n"
        L"                    SType CDATA 'Generic'\n"
        L"                    Name CDATA #REQUIRED\n"
        L"                    Access CDATA #REQUIRED\n"
        L"                    Private (Yes|No) #REQUIRED\n"
        L"                    AlwaysWrite (Yes|No) #REQUIRED\n"
        L"                    QueuedWrite (Yes|No) #REQUIRED\n"
        L"                    FldValue CDATA ''>\n"

        L"<!ELEMENT CQCFldDump (CQCFldDef*)>\n"
        L"<!ATTLIST CQCFldDump  Make CDATA #REQUIRED\n"
        L"                      Model CDATA #REQUIRED\n"
        L"                      Moniker CDATA #REQUIRED\n"
        L"                      FldCnt CDATA #REQUIRED>\n"
    );

    //
    //  Try to parse this guy. Set up a simple inline DTD we can use
    //  for the parsing.
    //
    tCIDXML::TEntitySrcRef esrToParse(new TFileEntitySrc(strSrcFile));
    TXMLTreeParser xtprsConfig;
    try
    {
        //
        //  Add an entity mapping so that the public URN that is used in the
        //  manifest files get mapped to our hard coded internal DTD text.
        //
        xtprsConfig.AddMapping
        (
            new TMemBufEntitySrc
            (
                L"http://www.charmedquark.com/CQC/CQCFldDumpV1.DTD"
                , L"urn:charmedquark.com:CQC-CQCFldDumpV1.DTD"
                , strDTD
            )
        );

        //
        //  Ok, we have a file, so lets try to use the XML tree parser to get
        //  a representation of the file into memory. Tell it that we only
        //  want to see non-ignorable chars and tag information. That will
        //  save us a lot of filtering out comments and whitspace.
        //
        const tCIDLib::TBoolean bOk = xtprsConfig.bParseRootEntity
        (
            esrToParse
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
            const TXMLTreeParser::TErrInfo& erriFirst
                                            = xtprsConfig.colErrors()[0];

            TTextStringOutStream strmOut(512);
            strmOut << L"[" << erriFirst.strSystemId() << L"/"
                    << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                    << L"] " << erriFirst.strText() << kCIDLib::EndLn;

            //
            //  Throw an exception that indicates that the file was not valid
            //  XML. Pass along the first error that occured.
            //
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_InvalidFldDump
                , strmOut.strData()
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , strSrcFile
            );
        }

        //
        //  Ok, we now have the document, so get out the root node and
        //  call a helper to traverse the data and pull out the data.
        //
        const TXMLTreeElement& xtnodeRoot = xtprsConfig.xtdocThis().xtnodeRoot();
        ParseFromNode(xtnodeRoot);
    }

    catch(TError& errToCatch)
    {
        if (facCQCKit().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// Removes the field at the indicated index
tCIDLib::TVoid TVarDrvCfg::RemoveAt(const tCIDLib::TCard4 c4At)
{
    // Just delegate to the collection
    m_colFlds.RemoveAt(c4At);
}


// ---------------------------------------------------------------------------
//  TVarDrvCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TVarDrvCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCDriver_VarDrvCfg::c2FmtVersion))
    {
        // We are throwing a CIDLib error here, not one of ours!
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

    strmToReadFrom  >> m_colFlds;

    // And the end object marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

}


tCIDLib::TVoid TVarDrvCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCDriver_VarDrvCfg::c2FmtVersion
                    << m_colFlds
                    << tCIDLib::EStreamMarkers::Frame;
}


// ---------------------------------------------------------------------------
//  TVarDrvCfg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TVarDrvCfg::ParseFromNode(const TXMLTreeElement& xtnodeRoot)
{
    // Flush our current contents out if any
    m_colFlds.RemoveAll();

    // Loop through the child nodes, each of which is a field
    TString strLimits;
    TString strTmp;
    const tCIDLib::TCard4 c4ChildCount = xtnodeRoot.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
    {
        // Get the field element at this index
        const TXMLTreeElement& xtnodeFld
        (
            xtnodeRoot.xtnodeChildAtAsElement(c4Index)
        );

        const TString& strName = xtnodeFld.xtattrNamed(L"Name").strValue();

        // And get the info out so that we can create a field def from it
        const tCIDLib::TBoolean bAlwaysWrite
        (
            xtnodeFld.xtattrNamed(L"AlwaysWrite").strValue() == L"Yes"
        );

        const tCIDLib::TBoolean bPrivate
        (
            xtnodeFld.xtattrNamed(L"Private").strValue() == L"Yes"
        );

        const tCIDLib::TBoolean bQueuedWrite
        (
            xtnodeFld.xtattrNamed(L"QueuedWrite").strValue() == L"Yes"
        );

        // The access may still have the old style enum prefix so remove if so
        strTmp = xtnodeFld.xtattrNamed(L"Access").strValue();
        if (strTmp.bStartsWith(L"EFldAccess_"))
            strTmp.Cut(0, 11);
        tCQCKit::EFldAccess eAccess =(tCQCKit::eXlatEFldAccess(strTmp));

        // The field type might also have the old enum prefix style
        strTmp = xtnodeFld.xtattrNamed(L"Type").strValue();
        if (strTmp.bStartsWith(L"EFldType_"))
            strTmp.Cut(0, 9);

        const tCQCKit::EFldTypes eType(tCQCKit::eXlatEFldTypes(strTmp));
        if (eType == tCQCKit::EFldTypes::Count)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_BadType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , strName
                , xtnodeFld.xtattrNamed(L"Type").strValue()
            );
        }

        // And again perhaps with the old style enum prefix
        strTmp = xtnodeFld.xtattrNamed(L"SType").strValue();
        if (strTmp.bStartsWith(L"EFldSType_"))
            strTmp.Cut(0, 10);

        const tCQCKit::EFldSTypes eSemType(tCQCKit::eXlatEFldSTypes(strTmp));
        if (eSemType == tCQCKit::EFldSTypes::Count)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_BadSemType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , strName
                , xtnodeFld.xtattrNamed(L"SType").strValue()
            );
        }

        // And there could be a child node with limit text
        if (xtnodeFld.c4ChildCount())
        {
            strLimits = xtnodeFld.xtnodeChildAtAsText(0).strText();
            strLimits.StripWhitespace();
        }
         else
        {
            strLimits.Clear();
        }

        // If it's read only, make it read/write
        if (eAccess == tCQCKit::EFldAccess::Read)
            eAccess = tCQCKit::EFldAccess::ReadWrite;

        // Ok, start setting up a new field def object
        TCQCFldDef flddCur(strName, eType, eAccess, eSemType, strLimits);
        flddCur.bAlwaysWrite(bAlwaysWrite);
        flddCur.bPrivate(bPrivate);
        flddCur.bQueuedWrite(bQueuedWrite);

        // If values are were dumped, them store it as the default value
        tCIDLib::TCard4 c4AttrInd;
        if (xtnodeFld.bAttrExists(L"FldValue", c4AttrInd))
            flddCur.strExtraCfg(xtnodeFld.xtattrAt(c4AttrInd).strValue());

        // And store it in the list
        m_colFlds.objAdd(flddCur);
    }
}


