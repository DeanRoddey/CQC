//
// FILE NAME: CQCRemBrws_ImgMap.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/09/2008
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
//  This is the implementation for the image map class
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCRemBrws_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCImgMap,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TCQCImgMap
//  PREFIX: cimap
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCImgMap: Public, static methods
// ---------------------------------------------------------------------------

//
//  This method will parse a map file and fil in the passed map object
tCIDLib::TVoid
TCQCImgMap::ParseMapFile(       TTextInStream&  strmSrc
                        , const TString&        strFileName
                        ,       TCQCImgMap&     cimapToFill)
{
    tCIDLib::TCard4 c4LineNum = 0;
    TString         strCurLine;

    // The first line has to start with [CQCImgMap
    strmSrc >> strCurLine;
    c4LineNum++;
    if (!strCurLine.bStartsWith(L"[CQCImgMap"))
    {
        facCQCKit().ThrowErr
        (
            strFileName
            , c4LineNum
            , kKitErrs::errcImgMap_NoMarker
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // The next two lines must be the to and from scopes
    TString strTo;
    strmSrc >> strTo;
    c4LineNum++;
    if (!strTo.bStartsWith(L"To="))
    {
        facCQCKit().ThrowErr
        (
            strFileName
            , c4LineNum
            , kKitErrs::errcImgMap_NoToScope
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
    strTo.Cut(0, 3);
    strTo.StripWhitespace();

    TString strFrom;
    strmSrc >> strFrom;
    c4LineNum++;
    if (!strFrom.bStartsWith(L"From="))
    {
        facCQCKit().ThrowErr
        (
            strFileName
            , c4LineNum
            , kKitErrs::errcImgMap_NoFromScope
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
    strFrom.Cut(0, 5);
    strFrom.StripWhitespace();

    //
    //  OK, it all looks reasonable, so set up the target map object and
    //  and start parsing the mappings.
    //
    cimapToFill.m_strTarPath = strTo;
    cimapToFill.m_strSrcPath = strFrom;
    cimapToFill.m_colMap.RemoveAll();

    TString strKey;
    TString strValue;
    while (!strmSrc.bEndOfStream())
    {
        strmSrc >> strKey;
        c4LineNum++;

        // If it's an empty line, skip it
        strKey.StripWhitespace();
        if (strKey.bIsEmpty())
            continue;

        // Find the equal sign and split it there
        if (!strKey.bSplit(strValue, L'='))
        {
            facCQCKit().ThrowErr
            (
                strFileName
                , c4LineNum
                , kKitErrs::errcImgMap_BadMapping
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }
        cimapToFill.m_colMap.objPlace(strKey, strValue);
    }
}


const TString& TCQCImgMap::strKey(const TCQCImgMap& imapSrc)
{
    return imapSrc.m_strTarPath;
}


// ---------------------------------------------------------------------------
//  TCQCImgMap: Constructors and destructor
// ---------------------------------------------------------------------------
TCQCImgMap::TCQCImgMap() :

    m_colMap(109, TStringKeyOps(), &TKeyValuePair::strExtractKey)
{
}

TCQCImgMap::TCQCImgMap(const TString& strSrcPath, const TString& strTarPath) :

    m_colMap(109, TStringKeyOps(), &TKeyValuePair::strExtractKey)
    , m_strSrcPath(strSrcPath)
    , m_strTarPath(strTarPath)
{
}

TCQCImgMap::~TCQCImgMap()
{
}

