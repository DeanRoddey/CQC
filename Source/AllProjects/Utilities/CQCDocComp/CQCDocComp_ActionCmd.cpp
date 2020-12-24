//
// FILE NAME: CQCDocComp_ActionCmd.Cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/1/2016
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
//  The implementation of the action command description class.
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
//   CLASS: TDocActionCmd
//  PREFIX: actc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDocActionCmd: Constructors and Destructor
// ---------------------------------------------------------------------------
TDocActionCmd::TDocActionCmd() :

    m_bRetStatus(kCIDLib::False)
{
}

TDocActionCmd::~TDocActionCmd()
{
}


// ---------------------------------------------------------------------------
//  TDocActionCmd: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TDocActionCmd::bParseXML(const TXMLTreeElement& xtnodeCmd)
{
    // Get the stuff that is available as attributes on the fld node
    m_strName = xtnodeCmd.xtattrNamed(L"Name").strValue();
    m_bRetStatus = xtnodeCmd.xtattrNamed(L"RetStatus").bValueAs();

    // Do any parameters, which starts at child 1
    const TKeyValuePair kvalEmpty;
    const tCIDLib::TCard4 c4ChildCnt = xtnodeCmd.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 1; c4Index < c4ChildCnt; c4Index++)
    {
        const TXMLTreeElement& xtnodeP = xtnodeCmd.xtnodeChildAtAsElement(c4Index);
        CIDAssert(xtnodeP.strQName() == L"CQCActParm", L"Expected an action parameter here");

        TKeyValuePair& kvalNew = m_colParms.objAdd(kvalEmpty);
        kvalNew.Set
        (
            xtnodeP.xtattrNamed(L"Name").strValue()
            , xtnodeP.xtattrNamed(L"Descr").strValue()
        );
    }

    // Process the overall action description text
    return TBasePage::bProcessHelpText(xtnodeCmd, L"ActDescr", m_hnDescr, kCIDLib::False);
}


tCIDLib::TVoid
TDocActionCmd::GenerateOutput(TTextOutStream& strmTar, const TString& strKey) const
{
    //
    //  Give it an id that matches the name, so that we can scroll to it later when
    //  doing a bookmark type link.
    //
    strmTar << L"<span class=\"CmdParmName\" id=\"" << m_strName << L"\">"
            << m_strName
            << L"</span><span class=\"DeemphCode\">(";

    const tCIDLib::TCard4 c4PCnt = m_colParms.c4ElemCount();
    for (tCIDLib::TCard4 c4PInd = 0; c4PInd < c4PCnt; c4PInd++)
    {
        const TKeyValuePair& kvalCur = m_colParms[c4PInd];

        if (c4PInd)
            strmTar << L", ";

        strmTar << kvalCur.strKey();
    }
    strmTar << L")</span>";

    // If it returns status, indicate that
    if (m_bRetStatus)
    {
        strmTar << L"<span class=\"EmphCode\">"
                << L"&nbsp;&nbsp;[*]</span>";
    }

    // Show the parameter names/short descriptions in a table
    strmTar << L"<blockquote>";
    if (c4PCnt)
    {
        strmTar << L"<table class=\"ActCmdParmsTable\">";
        for (tCIDLib::TCard4 c4PInd = 0; c4PInd < c4PCnt; c4PInd++)
        {
            const TKeyValuePair& kvalCur = m_colParms[c4PInd];
            strmTar << L"<tr><td>"
                    << kvalCur.strKey()
                    << L":&nbsp;</td><td>"
                    << kvalCur.strValue()
                    << L"</td></tr>";
        }
        strmTar << L"</table>";
    }

    // Spit out the description text
    TBasePage::OutputNodes(strmTar, m_hnDescr, strKey);
    strmTar << L"</blockquote>";
}




// ---------------------------------------------------------------------------
//   CLASS: TDocActionTar
//  PREFIX: actt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDocActionTar: Constructors and Destructor
// ---------------------------------------------------------------------------
TDocActionTar::TDocActionTar()
{
}

TDocActionTar::~TDocActionTar()
{
}


// ---------------------------------------------------------------------------
//  TDocActionTar: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TDocActionTar::bParseXML(const  TXMLTreeElement&    xtnodeTar
                        , const TString&            strName
                        , const TTopic&             topicPar)
{
    // Call our parent first
    if (!TBasePage::bParseXML(xtnodeTar, strName, topicPar))
        return kCIDLib::False;

    // Process the commands. It starts at child 2
    TDocActionCmd actcEmpty;
    const tCIDLib::TCard4 c4ChildCnt = xtnodeTar.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 2; c4Index < c4ChildCnt; c4Index++)
    {
        const TXMLTreeElement& xtnodeCmd = xtnodeTar.xtnodeChildAtAsElement(c4Index);
        CIDAssert(xtnodeCmd.strQName() == L"CQCActCmd", L"Expected an action command here");

        TDocActionCmd& actcCur = m_colCmds.objAdd(actcEmpty);

        if (!actcCur.bParseXML(xtnodeCmd))
            return kCIDLib::False;
    }

    // Process the overall action description text
    return bProcessHelpText(xtnodeTar, L"ActDescr", m_hnDescr, kCIDLib::False);
}


tCIDLib::TVoid TDocActionTar::GenerateOutput(TTextOutStream& strmTar) const
{
    strmTar << L"<!DOCTYPE html>\n"
               L"<span class='PageHdr'>";
    strmTar << m_strTitle;
    strmTar << L"</span><p></p>";

    // Spit out the main target docs
    strmTar << L"<blockquote>";
    OutputNodes(strmTar, m_hnDescr, m_strPath);
    strmTar << L"</blockquote>";

    // Now let'd do all of the action commands
    strmTar << L"<br/><span class=\"SecHdr\">Commands for this Target</span><blockquote>\n";

    const tCIDLib::TCard4 c4CmdCnt = m_colCmds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CmdCnt; c4Index++)
    {
        const TDocActionCmd& actcCur = m_colCmds[c4Index];
        actcCur.GenerateOutput(strmTar, m_strPath);
    }
    strmTar << L"</blockquote>";
}
