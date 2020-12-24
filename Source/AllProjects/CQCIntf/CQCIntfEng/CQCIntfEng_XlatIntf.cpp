//
// FILE NAME: CQCIntfEng_XlatIntf.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/17/2013
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
//  This file implements a mixin interface that provides a text translation
//  capability to some widgets.
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
#include    "CQCIntfEng_.hpp"


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_XlatIntf
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 1;
    }
}


// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfXlatIntf
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MCQCIntfXlatIntf: Destructor
// ---------------------------------------------------------------------------
MCQCIntfXlatIntf::~MCQCIntfXlatIntf()
{
}


// ---------------------------------------------------------------------------
//  MCQCIntfXlatIntf: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
MCQCIntfXlatIntf::AddXlatItem(const  TString&    strKey
                                , const TString&    strValue)
{
    // Just put in a new k/v pair object the passed key and value
    m_colXlats.objAdd(TKeyValuePair(strKey, strValue));
}


// Return whether we have any xlat entries or not
tCIDLib::TBoolean MCQCIntfXlatIntf::bHaveXlats() const
{
    return !m_colXlats.bIsEmpty();
}


tCIDLib::TBoolean MCQCIntfXlatIntf::bRemoveXlatKey(const TString& strToRemove)
{
    return m_colXlats.bRemoveKeyIfExists(strToRemove);
}


tCIDLib::TBoolean
MCQCIntfXlatIntf::bSameXlats(const MCQCIntfXlatIntf& miwdgSrc) const
{
    // Check the special case ones first
    if ((m_strElseKey != miwdgSrc.m_strElseKey)
    ||  (m_strEmptyKey != miwdgSrc.m_strEmptyKey))
    {
        return kCIDLib::False;
    }
    return (m_colXlats == miwdgSrc.m_colXlats);
}


tCIDLib::TCard4
MCQCIntfXlatIntf::c4QueryXlatKeys(tCIDLib::TStrCollect& colToFill) const
{
    colToFill.RemoveAll();

    if (!m_colXlats.bIsEmpty())
    {
        tCIDLib::TKVHashSet::TCursor cursKeys(&m_colXlats);
        cursKeys.bReset();
        do
        {
            colToFill.objAdd(cursKeys.objRCur().strKey());
        }   while (cursKeys.bNext());
    }
    return colToFill.c4ElemCount();
}



//
//  Given a caption string, the fixed text, and a 'live value', we can setup
//  the text that should be displayed. The rules are:
//
//  1. If we have a caption
//      - If it has no %(f) in it, we display the caption as is
//      - If it has a %(f), we format the live value into it, translating
//          the live value if needed.
//
//  2. If no caption
//      - If not xlat for the live value, we display the live value as is
//      - Else we translate the live value and display that
//
//  In both cases, if we do translation, we will check for the two special
//  cases.
//
//  1. If the value is empty we see if we have a special empty value xlat
//  2. If we have translations but none of them match, we see if we have
//     the special else xlat and use that.
//
//  If neither of the special cases, we just display the caption or live
//  text as is.
//
//  They can also tell us if they are in error state. If so, we don't try
//  to do any of the above. We just set the text to the standard error
//  text value.
//
tCIDLib::TVoid
MCQCIntfXlatIntf::FmtXlatText(  const   TString&            strCaption
                                , const TString&            strValue
                                , const tCIDLib::TBoolean   bErrorState
                                ,       TString&            strToFill)
{
    //
    //  Decide what our live text is. It's either the incoming value or
    //  the standard error text.
    //
    const TString& strLT = bErrorState ? facCQCIntfEng().strErrText()
                                       : strValue;

    if (strCaption.bIsEmpty())
    {
        // No caption, so we just use the value, with possible xlat
        if (!bHaveXlats())
        {
            // No translations, so store the incoming value
            strToFill = strLT;
        }
         else
        {
            const TKeyValuePair* pkvalXlat = 0;
            if (strLT.bIsEmpty())
                pkvalXlat = pkvalEmptyXlatVal();
            else
                pkvalXlat = pkvalFindXlat(strLT);

            //  If we have a translate, use it, else take the live text
            if (pkvalXlat)
                strToFill = pkvalXlat->strValue();
            else
                strToFill = strLT;
        }
    }
     else
    {
        //
        //  We have a caption, so we may need to format the value string
        //  into it. If not, take the caption as is. We apply
        //  translation to the value string before formatting.
        //
        //  We have to be tolerant of badly formed replacement tokens here,
        //  since they are entering text and we are reacting on the fly, or
        //  they might have reason to enter something that looks like a token
        //  but isn't quite right.
        //
        tCIDLib::TBoolean bHaveToken = kCIDLib::False;
        try
        {
            bHaveToken = strCaption.bTokenExists(kCIDLib::chLatin_f);
        }

        catch(...)
        {
            // Just fall through, make sure we indicate no token
            bHaveToken = kCIDLib::False;
        }

        strToFill = strCaption;
        TString strTmp;
        if (bHaveToken)
        {
            //
            //  If no xalts, then take the live value, else xlat if possible.
            //  If not xlation available, then use the live value.
            //
            if (!bHaveXlats())
            {
                strTmp = strLT;
            }
             else
            {
                const TKeyValuePair* pkvalXlat = 0;
                if (strLT.bIsEmpty())
                    pkvalXlat = pkvalEmptyXlatVal();
                else
                    pkvalXlat = pkvalFindXlat(strLT);

                if (pkvalXlat)
                    strTmp = pkvalXlat->strValue();
                else
                    strTmp = strLT;
            }
            strToFill.eReplaceToken(strTmp, kCIDLib::chLatin_f);
        }
    }
}


TColCursor<TKeyValuePair>* MCQCIntfXlatIntf::pcursMakeNew() const
{
    return m_colXlats.pcursNew();
}


// Look for the special else value
const TKeyValuePair* MCQCIntfXlatIntf::pkvalElseXlatVal() const
{
    return m_colXlats.pobjFindByKey(m_strElseKey);
}


// Look for the special empty value
const TKeyValuePair* MCQCIntfXlatIntf::pkvalEmptyXlatVal() const
{
    return m_colXlats.pobjFindByKey(m_strEmptyKey);
}


//
//  Look up a key and return the key/value pair if found. If not found,
//  they can ask that we look for the else value and return that.
//
const TKeyValuePair*
MCQCIntfXlatIntf::pkvalFindXlat(const   TString&            strKey
                                , const tCIDLib::TBoolean   bUseElse) const
{
    // See if we can find the key
    const TKeyValuePair* pkvalRet = m_colXlats.pobjFindByKey(strKey);

    // If not found, and they want us to check for an else value, do that
    if (!pkvalRet && bUseElse)
        pkvalRet = m_colXlats.pobjFindByKey(m_strElseKey);

    return pkvalRet;
}


//
//  We just format out our content to binary format.
//
tCIDLib::TVoid
MCQCIntfXlatIntf::QueryXlatAttrs(tCIDMData::TAttrList& colAttrs, TAttrData& adatTmp) const
{
    colAttrs.objPlace
    (
        L"Translation:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    // Flatten out our data and set it
    TBinMBufOutStream strmOut(1024UL);
    WriteOutXlats(strmOut);
    strmOut.Flush();
    colAttrs.objPlace
    (
        L"Xlat Table"
        , kCQCIntfEng::strAttr_Xlats
        , strmOut.mbufData()
        , strmOut.c4CurPos()
        , TString::strEmpty()
        , tCIDMData::EAttrEdTypes::Visual
    );
}


// Read in our persistent data
tCIDLib::TVoid
MCQCIntfXlatIntf::ReadInXlats(TBinInStream& strmToReadFrom)
{
    // Our stuff shuold start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_XlatIntf::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString(L"MCQCIntfXlatIntf")
        );
    }

    strmToReadFrom  >> m_colXlats;

    // And it should all end with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
}


// Remove all keys
tCIDLib::TVoid MCQCIntfXlatIntf::Reset()
{
    m_colXlats.RemoveAll();
}


tCIDLib::TVoid
MCQCIntfXlatIntf::SetXlatAttr(          TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    if (adatNew.strId() == kCQCIntfEng::strAttr_Xlats)
    {
        // It's binary so stream ourself back in from the buffer
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        ReadInXlats(strmSrc);
    }
}


//
//  This is called when we need to update an item. In this case the key could
//  have changed, so we we have to find it by the old key.
//
//  Since we use a hash set, if the key has changed, we have to remove the
//  old one and put a new one back in.
//
tCIDLib::TVoid
MCQCIntfXlatIntf::UpdateXlatItem(const  TString&    strOldKey
                                , const TString&    strNewKey
                                , const TString&    strValue)
{
    if (strOldKey == strNewKey)
    {
        // Just find it and update the value
        TKeyValuePair* pkvalUp = m_colXlats.pobjFindByKey(strOldKey);
        if (pkvalUp)
            pkvalUp->strValue(strValue);
    }
     else
    {
        // Remove the old key and add a new one
        m_colXlats.bRemoveKey(strOldKey, kCIDLib::False);
        m_colXlats.objAdd(TKeyValuePair(strNewKey, strValue));
    }
}


// Write out our persistent info
tCIDLib::TVoid
MCQCIntfXlatIntf::WriteOutXlats(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCIntfEng_XlatIntf::c2FmtVersion
                    << m_colXlats
                    << tCIDLib::EStreamMarkers::Frame;

}


//
//  Just used to upgrade in 4.2.918. The translation stuff used to be in the
//  dynamic text base class. It will call here to let us read in just the
//  list.
//
tCIDLib::TVoid MCQCIntfXlatIntf::RefactorRead(TBinInStream& strmToReadFrom)
{
    strmToReadFrom >> m_colXlats;
}



// ---------------------------------------------------------------------------
//  MCQCIntfXlatIntf: Hidden constructors operators
// ---------------------------------------------------------------------------
MCQCIntfXlatIntf::MCQCIntfXlatIntf() :

    m_colXlats(29, TStringKeyOps(), &TKeyValuePair::strExtractKey)
    , m_strElseKey(L"<<Else>>")
    , m_strEmptyKey(L"<<Empty>>")
{
}

MCQCIntfXlatIntf::MCQCIntfXlatIntf(const MCQCIntfXlatIntf& miwdgSrc) :

    m_colXlats(miwdgSrc.m_colXlats)
    , m_strElseKey(L"<<Else>>")
    , m_strEmptyKey(L"<<Empty>>")
{
}

MCQCIntfXlatIntf& MCQCIntfXlatIntf::operator=(const MCQCIntfXlatIntf& miwdgSrc)
{
    if (this != &miwdgSrc)
    {
        // No need to copy the special keys. They are already set
        m_colXlats = miwdgSrc.m_colXlats;
    }
    return *this;
}


