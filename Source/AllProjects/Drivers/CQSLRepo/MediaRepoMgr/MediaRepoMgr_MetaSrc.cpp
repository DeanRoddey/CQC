//
// FILE NAME: MediaRepoMgr_MetaSrc.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/01/2011
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
//  This file edits the base classes for the metadata source interface.
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
#include    "MediaRepoMgr.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TRepoMetaMatch,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TRepoMetaMatch
//  PREFIX: rmm
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TRepoMetaMatch: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TRepoMetaMatch::eSortByQuality( const   TRepoMetaMatch& rmm1
                                , const TRepoMetaMatch& rmm2)
{
    if (rmm1.m_c4Quality < rmm2.m_c4Quality)
        return tCIDLib::ESortComps::FirstLess;
    else if (rmm1.m_c4Quality > rmm2.m_c4Quality)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


// ---------------------------------------------------------------------------
//  TRepoMetaMatch: Constructors and Destructor
// ---------------------------------------------------------------------------
TRepoMetaMatch::TRepoMetaMatch()
{
}

TRepoMetaMatch::~TRepoMetaMatch()
{
}


// ---------------------------------------------------------------------------
//  TRepoMetaMatch: Public methods
// ---------------------------------------------------------------------------

// Provide access to the values we store
tCIDLib::TCard4 TRepoMetaMatch::c4Quality() const
{
    return m_c4Quality;
}

tCIDLib::TCard4 TRepoMetaMatch::c4Year() const
{
    return m_c4Year;
}

const TString& TRepoMetaMatch::strBarcode() const
{
    return m_strBarcode;
}

const TString& TRepoMetaMatch::strEdition() const
{
    return m_strEdition;
}

const TString& TRepoMetaMatch::strLoadID() const
{
    return m_strLoadID;
}

const TString& TRepoMetaMatch::strMediaType() const
{
    return m_strMediaType;
}

const TString& TRepoMetaMatch::strName() const
{
    return m_strName;
}


// Set up the data for this match object
tCIDLib::TVoid
TRepoMetaMatch::Set(const   TString&        strLoadID
                    , const TString&        strName
                    , const TString&        strEdition
                    , const tCIDLib::TCard4 c4Year
                    , const TString&        strBarcode
                    , const TString&        strMediaType
                    , const tCIDLib::TCard4 c4Quality)
{
    m_c4Quality     = c4Quality;
    m_c4Year        = c4Year;
    m_strBarcode    = strBarcode;
    m_strEdition    = strEdition;
    m_strLoadID     = strLoadID;
    m_strMediaType  = strMediaType;
    m_strName       = strName;
}





// ---------------------------------------------------------------------------
//   CLASS: TRepoMetaSrc
//  PREFIX: rms
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TRepoMetaSrc: Destructor
// ---------------------------------------------------------------------------
TRepoMetaSrc::~TRepoMetaSrc()
{
}


// ---------------------------------------------------------------------------
//  TRepoMetaSrc: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TString& TRepoMetaSrc::strBarcodeType() const
{
    return m_strBarcodeType;
}

const TString& TRepoMetaSrc::strSrcName() const
{
    return m_strSrcName;
}


// ---------------------------------------------------------------------------
//  TRepoMetaSrc: Hidden Constructors
// ---------------------------------------------------------------------------
TRepoMetaSrc::TRepoMetaSrc(const TString& strSrcName, const TString& strBarcodeType) :

    m_strBarcodeType(strBarcodeType)
    , m_strSrcName(strSrcName)
{
}


// ---------------------------------------------------------------------------
//  TRepoMetaSrc: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper that will get the value of a given attribute out if it exists, and
//  returns a status as to whether it was found or not.
//
tCIDLib::TBoolean
TRepoMetaSrc::bGetAttr( const   TXMLTreeElement&    xtnodeSrc
                        , const TString&            strAttrName
                        ,       TString&            strToFill)
{
    tCIDLib::TCard4 c4At;
    if (!xtnodeSrc.bAttrExists(strAttrName, c4At))
        return kCIDLib::False;

    const TXMLTreeAttr& xtattrTar = xtnodeSrc.xtattrAt(c4At);
    strToFill = xtattrTar.strValue();
    return kCIDLib::True;
}


//
//  A helper to convert the value of an attribute of the passed node to a cardinal
//  cardinal value. If the attribute doesn't exist or cannot be converted, it can
//  throw or return a default.
//
tCIDLib::TCard4
TRepoMetaSrc::c4GetAttrAsCard(  const   TXMLTreeElement&    xtnodeSrc
                                , const TString&            strAttrName
                                , const tCIDLib::TCard4     c4Default)
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        c4Ret = xtnodeSrc.xtattrNamed(strAttrName).c4ValueAs();
    }

    catch(...)
    {
        if (c4Default == kCIDLib::c4MaxCard)
        {
            facMediaRepoMgr.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMRMgrErrs::errcMM_AttrToCard2
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , xtnodeSrc.strQName()
                , strAttrName
            );
        }
        c4Ret = c4Default;
    }
    return c4Ret;
}


//
//  A helper to get the text content of a child of the passed element and convert
//  it to a cardinal value. If the text element isn't there or the conversion fails,
//  it throws.
//
tCIDLib::TCard4
TRepoMetaSrc::c4GetChildTextAsCard( const   TXMLTreeElement&    xtnodeSrc
                                    , const TString&            strChildName
                                    , const tCIDLib::TCard4     c4Default)
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        tCIDLib::TCard4 c4Ind;
        const TXMLTreeElement& xtnodeChild
        (
            xtnodeSrc.xtnodeFindElement(strChildName, 0, c4Ind)
        );
        const TXMLTreeText& xtnodeMsg = xtnodeChild.xtnodeChildAtAsText(0);
        c4Ret = xtnodeMsg.strText().c4Val();
    }

    catch(...)
    {
        if (c4Default == kCIDLib::c4MaxCard)
        {
            facMediaRepoMgr.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMRMgrErrs::errcMM_TextToCard2
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , xtnodeSrc.strQName()
                , strChildName
            );
        }
        c4Ret = c4Default;
    }
    return c4Ret;
}



//
//  A helper to get the text content of the passed element and convert it to a
//  cardinal value. If the text element isn't there or the conversion fails, it
//  throw.
//
tCIDLib::TCard4
TRepoMetaSrc::c4GetTextAsCard(  const   TXMLTreeElement&    xtnodeSrc
                                , const tCIDLib::TCard4     c4Default)
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        const TXMLTreeText& xtnodeMsg = xtnodeSrc.xtnodeChildAtAsText(0);
        c4Ret = xtnodeMsg.strText().c4Val();
    }

    catch(...)
    {
        if (c4Default == kCIDLib::c4MaxCard)
        {
            facMediaRepoMgr.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMRMgrErrs::errcMM_TextToCard
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , xtnodeSrc.strQName()
            );
        }
        c4Ret = c4Default;
    }
    return c4Ret;
}


//
//  A little helper to check that an element has the expected name. If not, it
//  throws.
//
tCIDLib::TVoid
TRepoMetaSrc::CheckElement( const   TXMLTreeElement&    xtnodeSrc
                            , const TString&            strExpName)
{
    if (xtnodeSrc.strQName() != strExpName)
    {
        facMediaRepoMgr.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMRMgrErrs::errcMM_ExpElement
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strExpName
            , xtnodeSrc.strQName()
        );
    }
}


//
//  A helper that will get the value of a given attribute out and return it. We
//  have another that will validate the name of the node as well.
//
tCIDLib::TVoid
TRepoMetaSrc::GetAttr(  const   TXMLTreeElement&    xtnodeSrc
                        , const TString&            strAttrName
                        ,       TString&            strToFill)
{
    const TXMLTreeAttr& xtattrTar = xtnodeSrc.xtattrNamed(strAttrName);
    strToFill = xtattrTar.strValue();
}

tCIDLib::TVoid
TRepoMetaSrc::GetAttr(  const   TXMLTreeElement&    xtnodeSrc
                        , const TString&            strElemName
                        , const TString&            strAttrName
                        ,       TString&            strToFill)
{
    CheckElement(xtnodeSrc, strElemName);

    const TXMLTreeAttr& xtattrTar = xtnodeSrc.xtattrNamed(strAttrName);
    strToFill = xtattrTar.strValue();
}


//
//  A helper to get the child text content of the named child of the passed
//  element.
//
tCIDLib::TVoid
TRepoMetaSrc::GetChildText( const   TXMLTreeElement&    xtnodeSrc
                            , const TString&            strChildName
                            ,       TString&            strToFill
                            , const tCIDLib::TBoolean   bThrowIfNot)
{
    try
    {
        tCIDLib::TCard4 c4Ind;
        const TXMLTreeElement& xtnodeChild
        (
            xtnodeSrc.xtnodeFindElement(strChildName, 0, c4Ind)
        );
        const TXMLTreeText& xtnodeMsg = xtnodeChild.xtnodeChildAtAsText(0);
        strToFill = xtnodeMsg.strText();
    }

    catch(...)
    {
        if (bThrowIfNot)
        {
            facMediaRepoMgr.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMRMgrErrs::errcMM_ElemText2
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , xtnodeSrc.strQName()
                , strChildName
            );
        }
        strToFill.Clear();
    }
}


//
//  A helper to get the child text content of the passed element.
//
tCIDLib::TVoid
TRepoMetaSrc::GetText(  const   TXMLTreeElement&    xtnodeSrc
                        ,       TString&            strToFill
                        , const tCIDLib::TBoolean   bThrowIfNot)
{
    try
    {
        const TXMLTreeText& xtnodeMsg = xtnodeSrc.xtnodeChildAtAsText(0);
        strToFill = xtnodeMsg.strText();
    }

    catch(...)
    {
        if (bThrowIfNot)
        {
            facMediaRepoMgr.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMRMgrErrs::errcMM_ElemText
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , xtnodeSrc.strQName()
            );
        }
        strToFill.Clear();
    }
}



