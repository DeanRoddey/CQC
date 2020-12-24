//
// FILE NAME: CQCDocComp_DevDriver.Cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/05/2017
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
//  The implementation of the device driver definition page class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CQCDocComp.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TDevDriverPage
//  PREFIX: pg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDevDriverPage: Public static methods
// ---------------------------------------------------------------------------

// A comparison functor to compare by driver make
tCIDLib::ESortComps
TDevDriverPage::eSortByMake(const   TDevDriverPage& pg1
                            , const TDevDriverPage& pg2)
{
    return pg1.m_strMake.eCompare(pg2.m_strMake);
}


// ---------------------------------------------------------------------------
//  TDevDriverPage: Constructors and Destructor
// ---------------------------------------------------------------------------
TDevDriverPage::TDevDriverPage() :

    m_bHaveSemType(kCIDLib::False)
    , m_bV2Compat(kCIDLib::False)
{
}

TDevDriverPage::~TDevDriverPage()
{
}


// ---------------------------------------------------------------------------
//  TDevDriverPage: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDevDriverPage::bParseXML(  const   TXMLTreeElement&    xtnodeDrv
                            , const TString&            strName
                            , const TTopic&             topicPar)
{
    // Call our parent first
    if (!TBasePage::bParseXML(xtnodeDrv, strName, topicPar))
        return kCIDLib::False;

    m_bV2Compat = xtnodeDrv.xtattrNamed(s_strAttr_V2Compat).bValueAs();
    m_strConnType = xtnodeDrv.xtattrNamed(s_strAttr_ConnType).strValue();
    m_strCQCVersion = xtnodeDrv.xtattrNamed(s_strAttr_CQCVersion).strValue();
    m_strMake = xtnodeDrv.xtattrNamed(s_strAttr_Make).strValue();
    m_strModel = xtnodeDrv.xtattrNamed(s_strAttr_Model).strValue();
    m_strVersion = xtnodeDrv.xtattrNamed(s_strAttr_Version).strValue();

    // See if we have any device classes, and validate them
    {
        const TString strDCs = xtnodeDrv.xtattrNamed(L"DrvCls").strValue();
        TStringTokenizer stokDCs(&strDCs, kCIDLib::szWhitespace);
        TString strTok;
        while (stokDCs.bGetNextToken(strTok))
        {
            strTok.StripWhitespace();
            tCQCKit::EDevClasses eClass = tCQCKit::eXlatEDevClasses(strTok);
            if (eClass == tCQCKit::EDevClasses::Count)
            {
                facCQCDocComp.strmErr()
                    << L"'" << strTok
                    << L"' is not a valid device class"
                    << kCIDLib::NewEndLn;
                return kCIDLib::False;
            }
            m_fcolDevClasses.c4AddElement(eClass);
        }

        // If we got any, make sure V2 compat is set
        if (!m_fcolDevClasses.bIsEmpty() && !m_bV2Compat)
        {
            facCQCDocComp.strmErr()
                << L"Device classes were defined but the V2 compatible flag was not set"
                << kCIDLib::NewEndLn;
            return kCIDLib::False;
        }

        // And the opposite
        if (m_bV2Compat && m_fcolDevClasses.bIsEmpty())
        {
            facCQCDocComp.strmErr()
                << L"Not device classes were defined but the V2 compatible flag was set"
                << kCIDLib::NewEndLn;
            return kCIDLib::False;
        }
    }

    // We have to have an overview description block
    if (!bProcessHelpText(xtnodeDrv, L"DrvDescr", m_hnDesc, kCIDLib::False))
        return kCIDLib::False;

    // We have some optional sections
    bProcessHelpText(xtnodeDrv, s_strElem_DrvConnDets, m_hnConnDets, kCIDLib::True);
    bProcessHelpText(xtnodeDrv, s_strElem_DrvDCComments, m_hnDCComments, kCIDLib::True);
    bProcessHelpText(xtnodeDrv, s_strElem_DrvQuirks, m_hnQuirks, kCIDLib::True);
    bProcessHelpText(xtnodeDrv, s_strElem_DrvEvTriggers, m_hnEvTriggers, kCIDLib::True);

    // Process the field list, if this one defines any
    tCIDLib::TCard4 c4At;
    const TXMLTreeElement& xtnodeList = xtnodeDrv.xtnodeFindElement(L"FieldList", 0, c4At);
    const tCIDLib::TCard4 c4FldCnt = xtnodeList.c4ChildCount();
    if (c4FldCnt)
    {
        TDocFldDef dfdEmpty;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FldCnt; c4Index++)
        {
            const TXMLTreeNode& xtnodeCur = xtnodeList.xtnodeChildAt(c4Index);

            // Shouldn't happen but skip anything other than elements
            if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
                continue;

            // And it should be a field element
            const TXMLTreeElement& xtnodeFld = static_cast<const TXMLTreeElement&>(xtnodeCur);
            if (xtnodeFld.strQName() != L"Field")
            {
                facCQCDocComp.strmErr()
                    << L"Expected 'Field' element, but got '" << xtnodeFld.strQName() << L"'"
                    << kCIDLib::NewEndLn;
                return kCIDLib::False;
            }

            TDocFldDef& dfdNew = m_colFldList.objAdd(dfdEmpty);
            if (!dfdNew.bParseXML(xtnodeFld))
                return kCIDLib::False;

            // Remember if any have semantic types. If none, we'll leave the column out later
            m_bHaveSemType |= dfdNew.bHasSemType();
        }
    }

    // We can have optional commentary after the fields
    bProcessHelpText(xtnodeDrv, L"DrvFldComments", m_hnFldComments, kCIDLib::True);

    return kCIDLib::True;
}


tCIDLib::TVoid TDevDriverPage::GenerateOutput(TTextOutStream& strmTar) const
{
    strmTar << L"<!DOCTYPE html>\n";

    // Do the basic driver info table at the top
    strmTar << L"<blockquote><table class='DrvInfoTable'>"
            << L"<tr><td>Make/Model</td><td>"
            << m_strMake << L"/" << m_strModel
            << L"</td></tr>";

    // If there's a device version, do that
    if (!m_strVersion.bIsEmpty())
    {
        strmTar << L"<tr><td>Device Version</td><td>"
                << m_strVersion
                << L"</td></tr>";
    }

    // If not other type, then display the connection type
    if (!m_strConnType.bCompareI(s_strType_Other))
    {
        strmTar << L"<tr><td>Connection Type</td><td>";

        // Expand some that have to be a single word for XML syntax purposes
        if (m_strConnType == s_strConn_SerialOrIP)
            strmTar << L"Serial or IP";
        else
            strmTar <<  m_strConnType;

        strmTar << L"</td></tr>";
    }

    strmTar << L"<tr><td>V2 Compat</td><td>"
            << facCQCKit().strBoolYesNo(m_bV2Compat)
            << L"</td></tr>";

    // If there's a CQC version as of, display that
    if (!m_strCQCVersion.bIsEmpty())
    {
        strmTar << L"<tr><td>As of CQC</td><td>"
                << m_strCQCVersion << L"</td></tr>";
    }

    strmTar << L"</table></blockquote><Br/>\n";

    // Output the overview text, which has to be there
    strmTar << L"<span class='SecHdr'>Overview</span><blockquote>\n";
    OutputNodes(strmTar, m_hnDesc, m_strPath);
    strmTar << L"</blockquote>\n";

    // If V2 compatible, output the device class stuff
    if (m_bV2Compat)
    {
        strmTar << L"<span class='SecHdr'>Device Classes</span><blockquote>\n"
                << L"This driver is V2 compatible. So some (or even all) of its functionality is "
                   L"standardized and defined by one or more device classes it implements. "
                   L"See these links for the details of a given device class.\n";

        strmTar << L"<ul>\n";
        const tCIDLib::TCard4 c4Count = m_fcolDevClasses.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmTar << L"<li><a onclick=\"javascript:loadDynDiv('"
                    << L"/Reference/DevClasses', '/"
                    << tCQCKit::strXlatEDevClasses(m_fcolDevClasses[c4Index])
                    << L"');\" href='javascript:void(0)'"
                    << L">"
                    << tCQCKit::strAltXlat2EDevClasses(m_fcolDevClasses[c4Index])
                    << L"</a></li>\n";
        }
        strmTar << L"</ul>\n";

        // If any DC comments, output those
        if (!m_hnDCComments.bIsEmpty())
            OutputNodes(strmTar, m_hnDCComments, m_strPath);

        strmTar << L"</blockquote>\n";
    }

    if (!m_hnQuirks.bIsEmpty())
    {
        strmTar << L"<span class='SecHdr'>Quirks/Limitations</span><blockquote>\n";
        OutputNodes(strmTar, m_hnQuirks, m_strPath);
        strmTar << L"</blockquote>\n";
    }

    if (!m_hnConnDets.bIsEmpty())
    {
        strmTar << L"<span class='SecHdr'>Connection Details</span><blockquote>\n";
        OutputNodes(strmTar, m_hnConnDets, m_strPath);
        strmTar << L"</blockquote>\n";
    }

    if (!m_hnEvTriggers.bIsEmpty())
    {
        strmTar << L"<span class='SecHdr'>Event Triggers</span><blockquote>\n";
        OutputNodes(strmTar, m_hnEvTriggers, m_strPath);
        strmTar << L"</blockquote>\n";
    }

    // Spit out all of the fields
    const tCIDLib::TCard4 c4Count = m_colFldList.c4ElemCount();
    if (c4Count)
    {
        strmTar << L"<span class='SecHdr'>Fields</span>"
                << L"<blockquote>";

        // Display a different text depending on if its V2 or not
        strmTar << L"<p>";
        if (m_bV2Compat)
            strmTar << L"Driver specific fields, in addition to to those implemented by the device classes listed above";
        else
            strmTar << L"The non-V2/driver specific fields defined by this driver.";
        strmTar << L"</p>";

        // Start the field table now
        strmTar << L"<table class='FldDefTable'>";

        // The header row
        strmTar << L"<tr><td>Name</td><td>Type</td><td>R/W</td>";
        if (m_bHaveSemType)
            strmTar << L"<td>SemType</td>";

        strmTar << L"<td>Description</td></tr>";


        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TDocFldDef& dfdCur = m_colFldList[c4Index];

            strmTar << L"<tr><td>"
                    << dfdCur.m_strName
                    << L"</td><td>"
                    << dfdCur.m_strType
                    << L"</td>";

            strmTar << L"<td>";
            if (dfdCur.m_strAccess == s_strAccess_Read)
                strmTar << L"R";
            else if (dfdCur.m_strAccess == s_strAccess_Write)
                strmTar << L"W";
            else if (dfdCur.m_strAccess == s_strAccess_ReadWrite)
                strmTar << L"R/W";
            else if (dfdCur.m_strAccess == s_strAccess_CanReadMustWrite)
                strmTar << L"(R)/W";
            else if (dfdCur.m_strAccess == s_strAccess_CanWriteMustRead)
                strmTar << L"R/(W)";
            else
                strmTar << L"???";
            strmTar << L"</td>";

            if (m_bHaveSemType)
            {
                strmTar << L"<td>";
                if (dfdCur.m_eSemType == tCQCKit::EFldSTypes::Count)
                    strmTar << L"[See Below]";
                else
                    strmTar << tCQCKit::strXlatEFldSTypes(dfdCur.m_eSemType);
                strmTar << L"</td>";
            }

            // Spit out the description column
            strmTar << L"<td>";
            OutputNodes(strmTar, dfdCur.m_hnDescr, m_strPath);
            strmTar << L"</td>";

            // And close off the description and row for this round
            strmTar << L"</tr>\n";
        }
        strmTar << L"</table><br/></blockquote>\n";
    }

    // And any trailing field commentary if we have any
    if (!m_hnFldComments.bIsEmpty())
    {
        strmTar << L"<span class='SecHdr'>Field Comments</span><blockquote>\n";
        OutputNodes(strmTar, m_hnFldComments, m_strPath);
        strmTar << L"</blockquote>\n";
    }
}


tCIDLib::TVoid TDevDriverPage::SetLinkInfo(const TString& strDir, const TString& strFile)
{
    m_strDir = strDir;
    m_strFile = strFile;
}



// ---------------------------------------------------------------------------
//  TDevDriverPage: Private, static data members
// ---------------------------------------------------------------------------
const TString   TDevDriverPage::s_strAccess_CanReadMustWrite(L"CRW");
const TString   TDevDriverPage::s_strAccess_CanWriteMustRead(L"RCW");
const TString   TDevDriverPage::s_strAccess_Read(L"R");
const TString   TDevDriverPage::s_strAccess_ReadWrite(L"RW");
const TString   TDevDriverPage::s_strAccess_Write(L"W");

const TString   TDevDriverPage::s_strAttr_ConnType(L"ConnType");
const TString   TDevDriverPage::s_strAttr_CQCVersion(L"CQCVersion");
const TString   TDevDriverPage::s_strAttr_Make(L"Make");
const TString   TDevDriverPage::s_strAttr_Model(L"Model");
const TString   TDevDriverPage::s_strAttr_V2Compat(L"V2Compat");
const TString   TDevDriverPage::s_strAttr_Version(L"Version");

const TString   TDevDriverPage::s_strElem_DrvConnDets(L"DrvConnDets");
const TString   TDevDriverPage::s_strElem_DrvDCComments(L"DrvDCComments");
const TString   TDevDriverPage::s_strElem_DrvQuirks(L"DrvQuirks");
const TString   TDevDriverPage::s_strElem_DrvEvTriggers(L"DrvEvTriggers");

const TString   TDevDriverPage::s_strConn_SerialOrIP(L"SerialOrIP");

const TString   TDevDriverPage::s_strType_Other(L"Other");
