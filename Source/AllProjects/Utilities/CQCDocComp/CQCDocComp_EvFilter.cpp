//
// FILE NAME: CQCDocComp_EvFilter.Cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/09/2016
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
//  The implementation of the event filter reference class
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
//   CLASS: TDocEvFilter
//  PREFIX: actc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDocEvFilter: Constructors and Destructor
// ---------------------------------------------------------------------------
TDocEvFilter::TDocEvFilter()
{
}

TDocEvFilter::~TDocEvFilter()
{
}


// ---------------------------------------------------------------------------
//  TDocEvFilter: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TDocEvFilter::bParseXML(const TXMLTreeElement& xtnodeCmd)
{
    // Get the stuff that is available as attributes on the fld node
    m_strName = xtnodeCmd.xtattrNamed(L"Name").strValue();

    // Do any parameters, which starts at child 1
    const TKeyValuePair kvalEmpty;
    const tCIDLib::TCard4 c4ChildCnt = xtnodeCmd.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 1; c4Index < c4ChildCnt; c4Index++)
    {
        const TXMLTreeElement& xtnodeP = xtnodeCmd.xtnodeChildAtAsElement(c4Index);
        CIDAssert(xtnodeP.strQName() == L"EvFiltParm", L"Expected an event filter parameter here");

        TKeyValuePair& kvalNew = m_colParms.objAdd(kvalEmpty);
        kvalNew.Set
        (
            xtnodeP.xtattrNamed(L"Name").strValue()
            , xtnodeP.xtattrNamed(L"Descr").strValue()
        );
    }

    // Process the overall action description text
    return TBasePage::bProcessHelpText(xtnodeCmd, L"EvFiltDescr", m_hnDescr, kCIDLib::False);
}


tCIDLib::TVoid
TDocEvFilter::GenerateOutput(TTextOutStream& strmTar, const TString& strKey) const
{
    strmTar << L"<span class=\"EmphCode\">"
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

    // Show the parameter names/short descriptions in a table
    strmTar << L"<blockquote>";
    if (c4PCnt)
    {
        strmTar << L"<table class=\"EvFiltParmsTable\">";
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
//   CLASS: TDocEvFilterRef
//  PREFIX: actt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDocEvFilterRef: Constructors and Destructor
// ---------------------------------------------------------------------------
TDocEvFilterRef::TDocEvFilterRef()
{
}

TDocEvFilterRef::~TDocEvFilterRef()
{
}


// ---------------------------------------------------------------------------
//  TDocEvFilterRef: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TDocEvFilterRef::bParseXML(const  TXMLTreeElement&    xtnodeTar
                        , const TString&            strName
                        , const TTopic&             topicPar)
{
    // Call our parent first
    if (!TBasePage::bParseXML(xtnodeTar, strName, topicPar))
        return kCIDLib::False;

    // Process the commands. It starts at child 2
    TDocEvFilter actcEmpty;
    const tCIDLib::TCard4 c4ChildCnt = xtnodeTar.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 2; c4Index < c4ChildCnt; c4Index++)
    {
        const TXMLTreeElement& xtnodeCmd = xtnodeTar.xtnodeChildAtAsElement(c4Index);
        CIDAssert(xtnodeCmd.strQName() == L"EvFilter", L"Expected an event filter here");

        TDocEvFilter& evfCur = m_colFilters.objAdd(actcEmpty);

        if (!evfCur.bParseXML(xtnodeCmd))
            return kCIDLib::False;
    }

    // Process the overall action description text
    return bProcessHelpText(xtnodeTar, L"EvFiltDescr", m_hnDescr, kCIDLib::False);
}


tCIDLib::TVoid TDocEvFilterRef::GenerateOutput(TTextOutStream& strmTar) const
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
    strmTar << L"<br/><span class=\"SecHdr\">Available Triggered Event Filters</span><blockquote>\n";

    const tCIDLib::TCard4 c4FCnt = m_colFilters.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FCnt; c4Index++)
    {
        const TDocEvFilter& evfCur = m_colFilters[c4Index];
        evfCur.GenerateOutput(strmTar, m_strPath);
    }
    strmTar << L"</blockquote>";
}
