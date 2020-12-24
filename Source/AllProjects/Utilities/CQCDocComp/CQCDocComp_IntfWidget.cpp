//
// FILE NAME: CQCDocComp_IntfWdg.Cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/19/2016
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
//  The implementation of the interface widget reference docs class.
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
//   CLASS: TDocIntfWidget
//  PREFIX: actt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDocIntfWidget: Constructors and Destructor
// ---------------------------------------------------------------------------
TDocIntfWidget::TDocIntfWidget()
{
}

TDocIntfWidget::~TDocIntfWidget()
{
}


// ---------------------------------------------------------------------------
//  TDocIntfWidget: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TDocIntfWidget::bParseXML(  const   TXMLTreeElement&    xtnodeWdg
                            , const TString&            strName
                            , const TTopic&             topicPar)
{
    // Call our parent first
    if (!TBasePage::bParseXML(xtnodeWdg, strName, topicPar))
        return kCIDLib::False;

    // If we have any commands, process them
    tCIDLib::TCard4 c4At;
    const TXMLTreeElement* pxtnodeCur = xtnodeWdg.pxtnodeFindElement(L"IntfCmds", 0, c4At);
    if (pxtnodeCur)
    {
        if (!bParseCmds(*pxtnodeCur))
            return kCIDLib::False;
    }

    // If we have any events, process them
    c4At = 0;
    pxtnodeCur = xtnodeWdg.pxtnodeFindElement(L"IntfEvents", 0, c4At);
    if (pxtnodeCur)
    {
        if (!bParseItems(*pxtnodeCur, m_colEvents, m_hnEvents))
            return kCIDLib::False;
    }

    // If we have any RTVs, process them
    c4At = 0;
    pxtnodeCur = xtnodeWdg.pxtnodeFindElement(L"IntfRTVs", 0, c4At);
    if (pxtnodeCur)
    {
        if (!bParseItems(*pxtnodeCur, m_colRTVs, m_hnRTVs))
            return kCIDLib::False;
    }

    // Process the overall widget description text
    return bProcessHelpText(xtnodeWdg, L"WdgDescr", m_hnDescr, kCIDLib::False);
}


tCIDLib::TVoid TDocIntfWidget::GenerateOutput(TTextOutStream& strmTar) const
{
    strmTar << L"<!DOCTYPE html>\n"
               L"<span class='PageHdr'>";
    strmTar << m_strTitle;
    strmTar << L"</span><p></p>";

    // Spit out the main target docs
    strmTar << L"<blockquote>";
    OutputNodes(strmTar, m_hnDescr, m_strPath);
    strmTar << L"</blockquote>";

    // If any events do those
    {
        const tCIDLib::TCard4 c4EvCnt = m_colEvents.c4ElemCount();
        if (c4EvCnt)
        {
            strmTar << L"<p class=\"SecHdr\">Supported Events</p>";

            if (!m_hnEvents.bIsEmpty())
            {
                strmTar << L"<blockquote>";
                OutputNodes(strmTar, m_hnEvents, m_strPath);
                strmTar << L"</blockquote>";
            }

            strmTar << L"<ul>\n";
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4EvCnt; c4Index++)
            {
                const TKeyValuePair& kvalCur = m_colEvents[c4Index];
                strmTar << L"<li><span class=\"CmdParmName\">" << kvalCur.strKey() << L"</span>. "
                        << kvalCur.strValue() << L"</li>";
            }
            strmTar << L"</ul>";
        }
    }

    // If any commands do those
    {
        const tCIDLib::TCard4 c4CmdCnt = m_colCmds.c4ElemCount();
        if (c4CmdCnt)
        {
            strmTar << L"<p class=\"SecHdr\">Supported Commands</p>";
            if (!m_hnCmds.bIsEmpty())
            {
                strmTar << L"<blockquote>";
                OutputNodes(strmTar, m_hnCmds, m_strPath);
                strmTar << L"</blockquote>";
            }

            strmTar << L"<blockquote>\n";
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CmdCnt; c4Index++)
                m_colCmds[c4Index].GenerateOutput(strmTar, m_strPath);

            strmTar << L"</blockquote>";
        }

        strmTar << L"</blockquote>";
    }

    // If any RTVs do those
    {
        const tCIDLib::TCard4 c4RTVCnt = m_colRTVs.c4ElemCount();
        if (c4RTVCnt)
        {
            strmTar << L"<p class=\"SecHdr\">Provded Runtime Values</p>";
            if (!m_hnRTVs.bIsEmpty())
            {
                strmTar << L"<blockquote>";
                OutputNodes(strmTar, m_hnRTVs, m_strPath);
                strmTar << L"</blockquote>";
            }

            strmTar << L"<ul>\n";
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RTVCnt; c4Index++)
            {
                const TKeyValuePair& kvalCur = m_colRTVs[c4Index];
                strmTar << L"<li><span class=\"CmdParmName\">" << kvalCur.strKey() << L"</span>. "
                        << kvalCur.strValue() << L"</li>";
            }
            strmTar << L"</ul>";
        }
    }
}


// ---------------------------------------------------------------------------
//  TDocIntfWidget: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TDocIntfWidget::bParseCmds(const TXMLTreeElement& xtnodeCmds)
{
    // The first child can be a helper text
    tCIDLib::TCard4 c4Index = 0;
    const TXMLTreeElement& xtnodeFirst = xtnodeCmds.xtnodeChildAtAsElement(0);
    if (xtnodeFirst.strQName() == L"WdgDescr")
    {
        if (!bProcessHelpText(xtnodeFirst, m_hnCmds))
            return kCIDLib::False;
        c4Index++;
    }

    TDocActionCmd actcEmpty;
    const tCIDLib::TCard4 c4ChildCnt = xtnodeCmds.c4ChildCount();
    for (; c4Index < c4ChildCnt; c4Index++)
    {
        const TXMLTreeElement& xtnodeCmd = xtnodeCmds.xtnodeChildAtAsElement(c4Index);
        CIDAssert(xtnodeCmd.strQName() == L"CQCActCmd", L"Expected an action command here");
        TDocActionCmd& actcCur = m_colCmds.objAdd(actcEmpty);
        if (!actcCur.bParseXML(xtnodeCmd))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TDocIntfWidget::bParseItems(const   TXMLTreeElement&        xtnodePar
                            ,       tCIDLib::TKVPList&      colToFill
                            ,       THelpNode&              hnToFill)
{
    // The first child can be a helper text
    tCIDLib::TCard4 c4Index = 0;

    const TXMLTreeElement& xtnodeFirst = xtnodePar.xtnodeChildAtAsElement(0);
    if (xtnodeFirst.strQName() == L"WdgDescr")
    {
        if (!bProcessHelpText(xtnodeFirst, hnToFill))
            return kCIDLib::False;
        c4Index++;
    }

    TString strDescr;
    TKeyValuePair kvalEmpty;
    const tCIDLib::TCard4 c4ChildCnt = xtnodePar.c4ChildCount();
    for (; c4Index < c4ChildCnt; c4Index++)
    {
        const TXMLTreeElement& xtnodeCur = xtnodePar.xtnodeChildAtAsElement(c4Index);

        // Get the child text which is the description
        const TXMLTreeText& xtnodeDescr = xtnodeCur.xtnodeChildAtAsText(0);

        TKeyValuePair& kvalCur = colToFill.objAdd(kvalEmpty);
        kvalCur.Set
        (
            xtnodeCur.xtattrNamed(L"Name").strValue(), xtnodeDescr.strText()
        );
    }
    return kCIDLib::True;
}

