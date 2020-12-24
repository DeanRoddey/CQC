//
// FILE NAME: CQCDocComp_DevClass.Cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/24/2016
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
//  The implementation of the device class specific page class
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
//   CLASS: TDevClassPage
//  PREFIX: pg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDevClassPage: Constructors and Destructor
// ---------------------------------------------------------------------------
TDevClassPage::TDevClassPage() :

    m_bHaveSemType(kCIDLib::False)
    , m_bPowerReq(kCIDLib::False)
    , m_eSubUnit(tCQCDocComp::ESubUnit_Never)
{
}

TDevClassPage::~TDevClassPage()
{
}


// ---------------------------------------------------------------------------
//  TDevClassPage: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDevClassPage::bParseXML(const  TXMLTreeElement&    xtnodeDC
                        , const TString&            strName
                        , const TTopic&             topicPar)
{
    // Call our parent first
    if (!TBasePage::bParseXML(xtnodeDC, strName, topicPar))
        return kCIDLib::False;

    // Get attributes from the main device class element
    m_strPrefix = xtnodeDC.xtattrNamed(L"Prefix").strValue();

    const TString& strSubUnit = xtnodeDC.xtattrNamed(L"SubUnit").strValue();
    if (strSubUnit == L"Always")
        m_eSubUnit = tCQCDocComp::ESubUnit_Always;
    else if (strSubUnit == L"Never")
        m_eSubUnit = tCQCDocComp::ESubUnit_Never;
    else if (strSubUnit == L"Device")
        m_eSubUnit = tCQCDocComp::ESubUnit_Device;

    // The 'as of' is optional
    if (xtnodeDC.bAttrExists(L"AsOf", m_strAsOf))
    {
        // Make sure it's a valid version
        try
        {
            tCIDLib::TCard4   c4MajVer;
            tCIDLib::TCard4   c4MinVer;
            tCIDLib::TCard4   c4Revision;
            TModule::ParseVersionStr(m_strAsOf, c4MajVer, c4MinVer, c4Revision);
        }

        catch(const TError& errToCatch)
        {
            facCQCDocComp.strmErr() << errToCatch.strErrText();
            return kCIDLib::False;
        }
    }
    TString strPowerReq;
    if (xtnodeDC.bAttrExists(L"PowerReq", strPowerReq) && strPowerReq.bCompareI(L"Yes"))
        m_bPowerReq = kCIDLib::True;

    // Find our comments element and parse it out
    if (!bProcessHelpText(xtnodeDC, L"Comments", m_hnComments, kCIDLib::False))
        return kCIDLib::False;

    // These are optional
    if (!bProcessHelpText(xtnodeDC, L"FldComments", m_hnFldComments, kCIDLib::True))
        return kCIDLib::False;
    if (!bProcessHelpText(xtnodeDC, L"PowerMng", m_hnPowerMng, kCIDLib::True))
        return kCIDLib::False;
    if (!bProcessHelpText(xtnodeDC, L"Triggers", m_hnTriggers, kCIDLib::True))
        return kCIDLib::False;
    if (!bProcessHelpText(xtnodeDC, L"BDCmds", m_hnBDCmds, kCIDLib::True))
        return kCIDLib::False;

    // And process the field list
    tCIDLib::TCard4 c4At;
    const TXMLTreeElement& xtnodeList = xtnodeDC.xtnodeFindElement(L"FieldList", 0, c4At);
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
    return kCIDLib::True;
}


tCIDLib::TVoid TDevClassPage::GenerateOutput(TTextOutStream& strmTar) const
{
    // Put out the doctype and main page title
    strmTar << L"<!DOCTYPE html>\n"
               L"<span class='PageHdr'>";
    strmTar << m_strTitle;
    strmTar << L"</span><p></p>";

    //
    //  Output the various sections. First we do a little table that contains the
    //  prefix and a couple other bits of info.
    //
    strmTar << L"<blockquote><table class='DevClsHdrTable'>"
            << L"<tr><td>Prefix:</td><td>"
            << m_strPrefix
            << L"</td></tr>";

    strmTar << L"<tr><td>Sub-Unit:</td><td>";
    if (m_eSubUnit == tCQCDocComp::ESubUnit_Always)
        strmTar << L"Always uses sub-unit style";
    else if (m_eSubUnit == tCQCDocComp::ESubUnit_Never)
        strmTar << L"Never uses sub-unit style";
    else if (m_eSubUnit == tCQCDocComp::ESubUnit_Device)
        strmTar << L"Uses sub-unit style if needed for the device";
    else
        strmTar << L"[Unknown!!]";
    strmTar << L"</td></tr>";

    // Show the power required flag if true
    if (m_bPowerReq)
        strmTar << L"<tr><td>Power Required:</td><td>Yes</td></tr>";

    // If there is an 'as of' value, show it. It's optional
    if (!m_strAsOf.bIsEmpty())
        strmTar << L"<tr><td>As Of Version:</td><td>" << m_strAsOf << L"</td></tr>";

    strmTar<< L"</table></blockquote><Br/>";

    strmTar << L"<span class='SecHdr'>Comments</span><blockquote>\n";
    OutputNodes(strmTar, m_hnComments, m_strPath);
    strmTar << L"</blockquote>\n";

    // If any power management stuff, do that
    if (!m_hnPowerMng.bIsEmpty())
    {
        strmTar << L"<span class='SecHdr'>Power Management</span><blockquote>\n";
        OutputNodes(strmTar, m_hnPowerMng, m_strPath);
        strmTar << L"</blockquote>\n";
    }

    // If any triggers stuff, do that
    if (!m_hnTriggers.bIsEmpty())
    {
        strmTar << L"<span class='SecHdr'>Event Triggers</span><blockquote>\n";
        OutputNodes(strmTar, m_hnTriggers, m_strPath);
        strmTar << L"</blockquote>\n";
    }

    // Do the field list
    strmTar << L"<span class='SecHdr'>Fields</span>"
            << L"<blockquote>"
            << L"<span class='SmallNote'>V2 prefixes are not shown for brevity</span>\n"
            << L"<table class='FldDefTable'>";

    // The header row
    strmTar << L"<tr><td>Name</td><td>Type</td><td>R/W</td>";
    if (m_bHaveSemType)
        strmTar << L"<td>SemType</td>";

    strmTar << L"<td>Description</td></tr>";

    // Spit out all of the fields
    const tCIDLib::TCard4 c4Count = m_colFldList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TDocFldDef& dfdCur = m_colFldList[c4Index];

        strmTar << L"<tr><td>"
                << dfdCur.m_strName
                << L"</td><td>"
                << dfdCur.m_strType
                << L"</td>";

        strmTar << L"<td>";
        if (dfdCur.m_strAccess == L"R")
            strmTar << L"R";
        else if (dfdCur.m_strAccess == L"W")
            strmTar << L"W";
        else if (dfdCur.m_strAccess == L"RW")
            strmTar << L"R/W";
        else if (dfdCur.m_strAccess == L"CRW")
            strmTar << L"(R)/W";
        else if (dfdCur.m_strAccess == L"RCW")
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

    // If any field comments, then spit those out
    if (!m_hnFldComments.bIsEmpty())
    {
        strmTar << L"<blockquote>\n";
        OutputNodes(strmTar, m_hnFldComments, m_strPath);
        strmTar << L"</blockquote>\n";
    }

    // If any backdoor commands, then spit those out
    if (!m_hnBDCmds.bIsEmpty())
    {
        strmTar << L"<span class='SecHdr'>Backdoor Commands</span><blockquote>\n";
        OutputNodes(strmTar, m_hnBDCmds, m_strPath);
        strmTar << L"</blockquote>\n";
    }
}
