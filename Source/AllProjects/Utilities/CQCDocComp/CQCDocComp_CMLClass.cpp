//
// FILE NAME: CQCDocComp_CMLClass.Cpp
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
//  The implementation of the CML class specific page class.
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
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCDocComp_CMLClass
{
    namespace
    {
        //
        //  Some constants that we use for the method groups. During parsing we set these
        //  flags. We have an output generator that will look for methods with a particular
        //  flag value and output those.
        //
        constexpr tCIDLib::TCard4 c4Const       = 0x01;
        constexpr tCIDLib::TCard4 c4Ctor        = 0x02;
        constexpr tCIDLib::TCard4 c4Final       = 0x04;
        constexpr tCIDLib::TCard4 c4Required    = 0x08;
        constexpr tCIDLib::TCard4 c4Override    = 0x10;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCMLSubType
//  PREFIX: subt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCMLSubType: Constructors and Destructor
// ---------------------------------------------------------------------------
TCMLSubType::TCMLSubType()
{
}

TCMLSubType::~TCMLSubType()
{
}



// ---------------------------------------------------------------------------
//   CLASS: TCMLMethodVar
//  PREFIX: methv
// ---------------------------------------------------------------------------
TCMLMethodVar::TCMLMethodVar()
{
}

TCMLMethodVar::~TCMLMethodVar()
{
}


// ---------------------------------------------------------------------------
//   CLASS: TCMLMethod
//  PREFIX: meth
// ---------------------------------------------------------------------------
TCMLMethod::TCMLMethod()
{
}

TCMLMethod::~TCMLMethod()
{
}


// ---------------------------------------------------------------------------
//   CLASS: TCMLMethodGrp
//  PREFIX: methg
// ---------------------------------------------------------------------------
TCMLMethodGrp::TCMLMethodGrp() :

    m_c4Flags(0)
{
}

TCMLMethodGrp::~TCMLMethodGrp()
{
}




// ---------------------------------------------------------------------------
//   CLASS: TCMLClassPage
//  PREFIX: pg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCMLClassPage: Constructors and Destructor
// ---------------------------------------------------------------------------
TCMLClassPage::TCMLClassPage()
{
}

TCMLClassPage::~TCMLClassPage()
{
}


// ---------------------------------------------------------------------------
//  TCMLClassPage: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCMLClassPage::bParseXML(const  TXMLTreeElement&    xtnodeCls
                        , const TString&            strName
                        , const TTopic&             topicPar)
{
    // Call our parent first
    if (!TBasePage::bParseXML(xtnodeCls, strName, topicPar))
        return kCIDLib::False;

    m_bCopyable = xtnodeCls.xtattrNamed(L"Copyable").bValueAs();
    m_bFinal = xtnodeCls.xtattrNamed(L"Final").bValueAs();

    // Get our dotted path, and create the slashed and parent versions
    m_strDottedPath = xtnodeCls.xtattrNamed(L"ClsPath").strValue();

    m_strSlashedPath = L"/";
    m_strSlashedPath.Append(m_strDottedPath);
    m_strSlashedPath.bReplaceChar(L'.', L'/');

    // Get the parent class
    m_strParentPath = xtnodeCls.xtattrNamed(L"ParPath").strValue();

    // Get the overall class description text
    if (!bProcessHelpText(xtnodeCls, L"ClassDescr", m_hnDesc, kCIDLib::False))
        return kCIDLib::False;

    // Iterate any literal nodes
    tCIDLib::TCard4 c4At;
    const TXMLTreeElement* pxtnodeCur = xtnodeCls.pxtnodeFindElement(L"Literals", 0, c4At);
    if (pxtnodeCur && !bParseLiterals(*pxtnodeCur))
        return kCIDLib::False;

    // Do any enumerated types
    pxtnodeCur = xtnodeCls.pxtnodeFindElement(L"Enums", 0, c4At);
    if (pxtnodeCur && !bParseEnums(*pxtnodeCur))
        return kCIDLib::False;

    // Iterate any nested types
    pxtnodeCur = xtnodeCls.pxtnodeFindElement(L"Types", 0, c4At);
    if (pxtnodeCur && !bParseNestedTypes(*pxtnodeCur))
        return kCIDLib::False;

    //
    //  Do all the method groups. These will be the rest of the childen of the class
    //  after everything else. So find the first method group and move forward from
    //  there.
    //
    pxtnodeCur = xtnodeCls.pxtnodeFindElement(L"MethodGrp", 0, c4At);
    if (pxtnodeCur)
    {
        const tCIDLib::TCard4 c4ChildCnt = xtnodeCls.c4ChildCount();
        TCMLMethodGrp methgEmpty;
        while (c4At < c4ChildCnt)
        {
            const TXMLTreeElement& xtnodeMGrp = xtnodeCls.xtnodeChildAtAsElement(c4At);
            CIDAssert(xtnodeMGrp.strQName() == L"MethodGrp", L"Expected a method group here");

            // Add a new group
            TCMLMethodGrp& methgNew = m_colMethodGrps.objAdd(methgEmpty);

            // Get the attribute flags out and store them
            if (xtnodeMGrp.xtattrNamed(L"Const").bValueAs())
                methgNew.m_c4Flags |= CQCDocComp_CMLClass::c4Const;

            if (xtnodeMGrp.xtattrNamed(L"Ctor").bValueAs())
                methgNew.m_c4Flags |= CQCDocComp_CMLClass::c4Ctor;

            if (xtnodeMGrp.xtattrNamed(L"Final").bValueAs())
                methgNew.m_c4Flags |= CQCDocComp_CMLClass::c4Final;

            if (xtnodeMGrp.xtattrNamed(L"Req").bValueAs())
                methgNew.m_c4Flags |= CQCDocComp_CMLClass::c4Required;

            if (xtnodeMGrp.xtattrNamed(L"Over").bValueAs())
                methgNew.m_c4Flags |= CQCDocComp_CMLClass::c4Override;

            // Parse the rest of this group
            if (!bParseMethodGrp(xtnodeMGrp, methgNew))
                return kCIDLib::False;

            c4At++;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TCMLClassPage::GenerateOutput(TTextOutStream& strmTar) const
{
    strmTar << L"<!DOCTYPE html>\n";

    // Put out the class path in page header style
    strmTar << L"<span class='PageHdr'>"
            << m_strSlashedPath
            << L"</span><p/>";

    // Do the little key-value table at the top
    strmTar << L"<blockquote><table class='CMLClsHdrTable'>"
            << L"<tr><td>ClassPath:</td><td>"
            << m_strDottedPath
            << L"</td></tr>";

    strmTar << L"<tr><td>Parent ClassPath:</td><td>"
            << m_strParentPath
            << L"</td></tr>";

    strmTar << L"<tr><td>Copyable:</td><td>"
            << (m_bCopyable ? L"Yes" : L"No")
            << L"</td></tr>";

    strmTar << L"<tr><td>Final:</td><td>"
            << (m_bFinal ? L"Yes" : L"No")
            << L"</td></tr>";
    strmTar<< L"</table></blockquote><Br/>";

    // Spit out the main class docs
    strmTar << L"<blockquote>";
    OutputNodes(strmTar, m_hnDesc, m_strPath);
    strmTar << L"</blockquote>";

    tCIDLib::TCard4 c4Count = m_colLiterals.c4ElemCount();
    if (c4Count)
    {
        strmTar << L"<span class='SecHdr'>Literal Values</span><blockquote>\n";
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCMLSubType& stCur = m_colLiterals[c4Index];

            // There can be multiple literal values with one descriptions
            strmTar << L"<pre class=\"EmphCode\">";
            const tCIDLib::TCard4 c4VCnt = stCur.m_colValues.c4ElemCount();
            for (tCIDLib::TCard4 c4VInd = 0; c4VInd < c4VCnt; c4VInd++)
            {
                const TKeyValues& kvalsCur = stCur.m_colValues[c4VInd];
                strmTar << kvalsCur.strVal1() << kCIDLib::chSpace
                        << kvalsCur.strKey() << kCIDLib::chOpenParen
                        << kvalsCur.strVal2() << kCIDLib::chCloseParen
                        << L"<br/>";
            }
            strmTar << L"</pre><blockquote>";
            OutputNodes(strmTar, stCur.m_hnDescr, m_strPath);
            strmTar << L"</blockquote>";
        }
        strmTar << L"</blockquote>\n";
    }

    // Do any enums
    if (m_colEnums.c4ElemCount())
    {
        strmTar << L"<span class='SecHdr'>Enumerated Types</span><blockquote>\n";

        c4Count = m_colEnums.c4ElemCount();
        if (c4Count)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                strmTar << L"<pre>";
                const TCMLSubType& stCur = m_colEnums[c4Index];
                strmTar << L"<span class=\"DeemphCode\">Enum=</span><span class=\"EmphCode\">"
                        << stCur.m_strName
                        << L"</span><br/>";

                //
                //  Output the individual values. We done one pass first to find the longest
                //  name.
                //
                const tCIDLib::TCard4 c4VCnt = stCur.m_colValues.c4ElemCount();
                tCIDLib::TCard4 c4NameWidth = 0;
                for (tCIDLib::TCard4 c4VInd = 0; c4VInd < c4VCnt; c4VInd++)
                {
                    const tCIDLib::TCard4 c4CurLen = stCur.m_colValues[c4VInd].strKey().c4Length();
                    if (c4CurLen > c4NameWidth)
                        c4NameWidth = c4CurLen;
                }

                // Set up a stream formatter for the name side
                TStreamFmt strmfName(c4NameWidth, 0, tCIDLib::EHJustify::Left);
                for (tCIDLib::TCard4 c4VInd = 0; c4VInd < c4VCnt; c4VInd++)
                {
                    strmTar << L"    "
                            << L"<span class=\"EmphCode\">"
                            << strmfName << stCur.m_colValues[c4VInd].strKey() << TTextOutStream::strmfDefault()
                            << L"</span><span class=\"DeemphCode\">"
                            << L" : \"" << stCur.m_colValues[c4VInd].strVal1()
                            << L"\"</span>;<br/>";
                }

                strmTar << L"EndEnum;</pre><blockquote>";
                OutputNodes(strmTar, stCur.m_hnDescr, m_strPath);
                strmTar << L"</blockquote>";
            }
        }
        strmTar << L"</blockquote>\n";
    }

    if (m_colTypes.c4ElemCount())
    {
        strmTar << L"<span class='SecHdr'>Nested Types</span><blockquote>\n";

        c4Count = m_colTypes.c4ElemCount();
        if (c4Count)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TCMLSubType& stCur = m_colTypes[c4Index];
                strmTar << L"<pre class=\"EmphCode\">" << stCur.m_strType << L"   "
                        << stCur.m_strName << L"<br/>";

                strmTar << L"</pre><blockquote>";
                OutputNodes(strmTar, stCur.m_hnDescr, m_strPath);
                strmTar << L"</blockquote>";
            }
        }
        strmTar << L"</blockquote>";
    }

    // Output any method groups we got
    c4Count = m_colMethodGrps.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCMLMethodGrp& methgCur = m_colMethodGrps[c4Index];

        // Figure out the title, which is driven by the flags
        TString strTitle;
        if (methgCur.m_c4Flags & CQCDocComp_CMLClass::c4Ctor)
        {
            strTitle = L"Constructors";
        }
         else
        {
            //
            //  There's no need to do final if required is set. If it's required it inherently
            //  is non-final.
            //
            if (!(methgCur.m_c4Flags & CQCDocComp_CMLClass::c4Required))
            {
                if (methgCur.m_c4Flags & CQCDocComp_CMLClass::c4Final)
                    strTitle.Append(L"Final");
                else
                    strTitle.Append(L"Non-Final");
            }

            if (!strTitle.bIsEmpty())
                strTitle.Append(L", ");
            if (methgCur.m_c4Flags & CQCDocComp_CMLClass::c4Const)
                strTitle.Append(L"Const");
            else
                strTitle.Append(L"Non-Const");

            // Required and override are mutually exclusive
            if (methgCur.m_c4Flags & CQCDocComp_CMLClass::c4Required)
            {
                if (!strTitle.bIsEmpty())
                    strTitle.Append(L", ");
                strTitle.Append(L"Required");
            }
             else if (methgCur.m_c4Flags & CQCDocComp_CMLClass::c4Override)
            {
                if (!strTitle.bIsEmpty())
                    strTitle.Append(L", ");
                strTitle.Append(L"Override");
            }
        }

        OutputMethodGrp(strmTar, strTitle, methgCur);
    }
}



// ---------------------------------------------------------------------------
//  TCMLClassPage: Private, non-virtual methods
// ---------------------------------------------------------------------------

// A helper to parse the enums section
tCIDLib::TBoolean TCMLClassPage::bParseEnums(const TXMLTreeElement& xtnodeEnums)
{
    TCMLSubType stEmpty;
    TString strVal;
    const tCIDLib::TCard4 c4Count = xtnodeEnums.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TXMLTreeElement& xtnodeEnum = xtnodeEnums.xtnodeChildAtAsElement(c4Index);
        CIDAssert(xtnodeEnum.strQName() == L"Enum", L"Expected an enum here");

        TCMLSubType& stNew = m_colEnums.objAdd(stEmpty);
        stNew.m_strName = xtnodeEnum.xtattrNamed(L"Name").strValue();

        // The first child should be the enum documentation node
        if (!bProcessHelpText(xtnodeEnum.xtnodeChildAtAsElement(0), stNew.m_hnDescr))
            return kCIDLib::False;

        // And the rest should be the values.
        const tCIDLib::TCard4 c4ChildCnt = xtnodeEnum.c4ChildCount();
        for (tCIDLib::TCard4 c4VInd = 1; c4VInd < c4ChildCnt; c4VInd++)
        {
            // These we store as key value pairs
            const TXMLTreeElement& xtnodeVal = xtnodeEnum.xtnodeChildAtAsElement(c4VInd);

            // The value is optional. If not provided we use the name value
            const TString strName = xtnodeVal.xtattrNamed(L"Name").strValue();
            if (!xtnodeVal.bAttrExists(L"Value", strVal))
                strVal = strName;
            stNew.m_colValues.objAdd(TKeyValues(strName, strVal));
        }
    }
    return kCIDLib::True;
}



// A helper to parse the literals section
tCIDLib::TBoolean TCMLClassPage::bParseLiterals(const TXMLTreeElement& xtnodeLits)
{
    TCMLSubType stEmpty;
    const tCIDLib::TCard4 c4Count = xtnodeLits.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TXMLTreeElement& xtnodeLit = xtnodeLits.xtnodeChildAtAsElement(c4Index);
        CIDAssert(xtnodeLit.strQName() == L"Literal", L"Expected a literal here");

        // It should have two children, the first of which is the values list
        const TXMLTreeElement& xtnodeVals = xtnodeLit.xtnodeChildAtAsElement(0);
        CIDAssert(xtnodeVals.strQName() == L"LiteralVals", L"Expected literal values list");

        //
        //  We can have multiple values within each set of literals, with a shared
        //  description, so let's go through those.
        //
        TCMLSubType& stNew = m_colLiterals.objAdd(stEmpty);
        const tCIDLib::TCard4 c4VCnt = xtnodeVals.c4ChildCount();
        for (tCIDLib::TCard4 c4VInd = 0; c4VInd < c4VCnt; c4VInd++)
        {
            const TXMLTreeElement& xtnodeVal = xtnodeVals.xtnodeChildAtAsElement(c4VInd);
            CIDAssert(xtnodeVal.strQName() == L"LiteralVal", L"Expected literal value node");

            stNew.m_colValues.objAdd
            (
                TKeyValues
                (
                    xtnodeVal.xtattrNamed(L"Name").strValue()
                    , xtnodeVal.xtattrNamed(L"Type").strValue()
                    , xtnodeVal.xtattrNamed(L"Value").strValue()
                )
            );
        }

        // And the second child should be the description for these literals
        if (!bProcessHelpText(xtnodeLit.xtnodeChildAtAsElement(1), stNew.m_hnDescr))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCMLClassPage::bParseMethodGrp(const TXMLTreeElement& xtnodeMGrp, TCMLMethodGrp& methgToFill)
{
    const tCIDLib::TCard4 c4MCount = xtnodeMGrp.c4ChildCount();
    tCIDLib::TCard4 c4MIndex = 0;

    // The first child can be a method group description
    if (xtnodeMGrp.xtnodeChildAtAsElement(0).strQName() == L"MethodDescr")
    {
        if (!bProcessHelpText(xtnodeMGrp.xtnodeChildAtAsElement(0), methgToFill.m_hnDescr))
            return kCIDLib::False;

        // And now start on the next one for the methods
        c4MIndex++;
    }

    const TCMLMethod    methEmpty;
    const TCMLMethodVar methvEmpty;
    const TKeyValues    kvalsEmpty;

    while (c4MIndex < c4MCount)
    {
        const TXMLTreeElement& xtnodeMethod = xtnodeMGrp.xtnodeChildAtAsElement(c4MIndex);

        // First child should be the method description
        CIDAssert
        (
            xtnodeMethod.strQName() == L"Method"
            , TString(TStrCat(L"Expected a Method here but got ", xtnodeMethod.strQName()))
        );

        TCMLMethod& methNew = methgToFill.m_colMethods.objAdd(methEmpty);

        // Get the method description text, which is the 0th node
        if (!bProcessHelpText(xtnodeMethod.xtnodeChildAtAsElement(0), methNew.m_hnDescr))
            return kCIDLib::False;

        // Now get the method variations info, which are the rest of the methods child nodes
        const tCIDLib::TCard4 c4VCnt = xtnodeMethod.c4ChildCount();
        for (tCIDLib::TCard4 c4VInd = 1; c4VInd < c4VCnt; c4VInd++)
        {
            const TXMLTreeElement& xtnodeMV = xtnodeMethod.xtnodeChildAtAsElement(c4VInd);
            CIDAssert(xtnodeMV.strQName() == L"MethodVar", L"Expected a method variation here");

            TCMLMethodVar& methvNew = methNew.m_colVariants.objAdd(methvEmpty);

            // Get the name and return type
            methvNew.m_strName = xtnodeMV.xtattrNamed(L"Name").strValue();
            methvNew.m_strRetType = xtnodeMV.xtattrNamed(L"RetType").strValue();

            // And get out the parameters
            const tCIDLib::TCard4 c4PVCnt = xtnodeMV.c4ChildCount();
            for (tCIDLib::TCard4 c4PInd = 0; c4PInd < c4PVCnt; c4PInd++)
            {
                const TXMLTreeElement& xtnodeParm = xtnodeMV.xtnodeChildAtAsElement(c4PInd);
                TKeyValues& kvalsParm = methvNew.m_colParms.objAdd(kvalsEmpty);
                kvalsParm.strKey(xtnodeParm.xtattrNamed(L"Name").strValue());
                kvalsParm.strVal1(xtnodeParm.xtattrNamed(L"Dir").strValue());
                kvalsParm.strVal2(xtnodeParm.xtattrNamed(L"Type").strValue());
            }
        }

        c4MIndex++;
    }
    return kCIDLib::True;
}



// A helper to parse the nested types section
tCIDLib::TBoolean TCMLClassPage::bParseNestedTypes(const TXMLTreeElement& xtnodeTypes)
{
    TCMLSubType stEmpty;
    const tCIDLib::TCard4 c4Count = xtnodeTypes.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TXMLTreeElement& xtnodeType = xtnodeTypes.xtnodeChildAtAsElement(c4Index);
        CIDAssert(xtnodeType.strQName() == L"Type", L"Expected a nested type here");

        TCMLSubType& stNew = m_colTypes.objAdd(stEmpty);
        stNew.m_strName = xtnodeType.xtattrNamed(L"Name").strValue();
        stNew.m_strType = xtnodeType.xtattrNamed(L"Type").strValue();

        // This element's content is the description text
        if (!bProcessHelpText(xtnodeType, stNew.m_hnDescr))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Output any methods we parsed
tCIDLib::TVoid
TCMLClassPage::OutputMethodGrp(         TTextOutStream& strmTar
                                , const TString&        strSecTitle
                                , const TCMLMethodGrp&  methgSrc) const
{
    strmTar << L"<span class='SecHdr'>"
            << strSecTitle
            << L"</span><blockquote>\n";

    // If we have any documentation, put it out
    if (!methgSrc.m_hnDescr.bIsEmpty())
    {
        strmTar << L"<p>";
        OutputNodes(strmTar, methgSrc.m_hnDescr, m_strPath);
        strmTar << L"</p>";
    }

    // And now do the methods
    TStreamFmt strmfPDir(7, kCIDLib::chSpace, tCIDLib::EHJustify::Left);
    const tCIDLib::TCard4 c4Count = methgSrc.m_colMethods.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCMLMethod& methCur = methgSrc.m_colMethods[c4Index];
        const tCIDLib::TCard4 c4VCnt = methCur.m_colVariants.c4ElemCount();

        strmTar << L"<pre>";
        for (tCIDLib::TCard4 c4VInd = 0; c4VInd < c4VCnt; c4VInd++)
        {
            const TCMLMethodVar& methvCur = methCur.m_colVariants[c4VInd];
            const tCIDLib::TKValsList& colParms = methvCur.m_colParms;
            const tCIDLib::TCard4 c4PCnt = colParms.c4ElemCount();

            if ((c4PCnt > 1) && c4VInd)
                strmTar << L"<br/>";

            strmTar << L"<span class=\"EmphCode\">" << methvCur.m_strName
                    << L"</span>";

            if (c4PCnt > 1)
                strmTar << L"<br/>(<br/>    ";
            else
                strmTar << kCIDLib::chOpenParen;

            //
            //  And go through the parameters. If there are more than 1, then we
            //  will wrap them around.
            //
            for (tCIDLib::TCard4 c4PInd = 0; c4PInd < c4PCnt; c4PInd++)
            {
                const TKeyValues& kvalsCur = colParms[c4PInd];
                if (c4PInd > 0)
                {
                    if (c4PCnt > 1)
                        strmTar << L"<Br/>    ";
                }

                m_strTmpFmt.Clear();
                if (c4PInd)
                    m_strTmpFmt.Append(L", ");
                m_strTmpFmt.Append(L"[");
                m_strTmpFmt.Append(kvalsCur.strVal1());
                m_strTmpFmt.Append(L"]");

                strmTar << L"<span class=\"EmphCode\">";

                if (c4PCnt > 1)
                    strmfPDir.c4Width(7);
                else
                    strmfPDir.c4Width(0);

                strmTar << strmfPDir
                        << m_strTmpFmt
                        << TTextOutStream::strmfDefault()
                        << L"</span>"
                        << L"<span class=\"DeemphCode\"> "
                        << kvalsCur.strVal2() << L"</span> "
                        << L"<span class=\"EmphCode\">" << kvalsCur.strKey() << L"</span>";
            }
            if (c4PCnt > 1)
                strmTar << L"<br/>";
            strmTar << L")";

            if (!methvCur.m_strRetType.bIsEmpty())
            {
                if (c4PCnt > 1)
                    strmTar << L"  ";
                strmTar << L" <span class=\"EmphCode\">Returns</span> <span class=\"DeemphCode\">"
                        << methvCur.m_strRetType << L"</span>";
            }

            strmTar << L";<br/>";

            if ((c4PCnt > 1) && (c4VInd + 1 < c4VCnt))
                strmTar << L"<br/>";
        }
        strmTar << L"</pre><blockquote>";
        OutputNodes(strmTar, methCur.m_hnDescr, m_strPath);
        strmTar << L"</blockquote>";
    }
    strmTar << L"</blockquote>\n";
}
