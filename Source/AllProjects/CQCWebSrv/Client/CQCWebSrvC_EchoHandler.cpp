//
// FILE NAME: CQCWebSrvC_EchoHandler.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/09/2005
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
//  This file implements the CMLBin URL handler, which calls out to a CML
//  macro to allow it to generate a page.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebSrvC_.hpp"
#include    "CQCWebSrvC_EchoHandler_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TWSEchoHandler,TWSURLHandler)


// ---------------------------------------------------------------------------
//   CLASS: TWSEchoCfgBlock
//  PREFIX: wsecb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWSEchoCfgBlock: Constructors and Destructor
// ---------------------------------------------------------------------------
TWSEchoCfgBlock::TWSEchoCfgBlock() :

    m_bIsMacro(kCIDLib::False)
{
}


TWSEchoCfgBlock::TWSEchoCfgBlock(const TWSEchoCfgBlock& wsecbSrc) :

    m_bIsMacro(wsecbSrc.m_bIsMacro)
    , m_colPhrases(wsecbSrc.m_colPhrases)
    , m_strPath(wsecbSrc.m_strPath)
{
}

TWSEchoCfgBlock::~TWSEchoCfgBlock()
{
}


// ---------------------------------------------------------------------------
//  Public operators
// ---------------------------------------------------------------------------
TWSEchoCfgBlock&
TWSEchoCfgBlock::operator=(const TWSEchoCfgBlock& wsecbSrc)
{
    if (&wsecbSrc != this)
    {
        m_bIsMacro      = wsecbSrc.m_bIsMacro;
        m_colPhrases    = wsecbSrc.m_colPhrases;
        m_strPath       = wsecbSrc.m_strPath;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TWSEchoCfgBlock::AddPhraseList(const tCIDLib::TStrList& colToAdd)
{
    m_colPhrases.objAdd(colToAdd);
}


//
//  Return true if we match the passed phrase, and if so it returns the preferred
//  form of that phrase, to report to the caller's handler.
//
tCIDLib::TBoolean
TWSEchoCfgBlock::bMatches(const TString& strCheck, TString& strToReport) const
{
    const tCIDLib::TCard4 c4ListCnt = m_colPhrases.c4ElemCount();
    for (tCIDLib::TCard4 c4ListInd = 0; c4ListInd < c4ListCnt; c4ListInd++)
    {
        const tCIDLib::TStrList& colSubList = m_colPhrases[c4ListInd];
        const tCIDLib::TCard4 c4SubCnt = colSubList.c4ElemCount();
        for (tCIDLib::TCard4 c4SubInd = 0; c4SubInd < c4SubCnt; c4SubInd++)
        {
            if (strCheck.bCompareI(colSubList[c4SubInd]))
            {
                strToReport = colSubList[0];
                return kCIDLib::True;
            }
        }
    }
    return kCIDLib::False;
}


// Returns whether we are configured for a macro or an action
tCIDLib::TBoolean TWSEchoCfgBlock::bIsMacro() const
{
    return m_bIsMacro;
}


// Provide access to our action/macro path
const TString& TWSEchoCfgBlock::strPath() const
{
    return m_strPath;
}


//
//  Reset us for another block during parseing, ready to add new keys.
//
tCIDLib::TVoid
TWSEchoCfgBlock::Set(const tCIDLib::TBoolean bIsMacro, const TString& strPath)
{
    // Clean any keys out and set us up to accept new ones
    m_colPhrases.RemoveAll();

    m_bIsMacro = bIsMacro;
    m_strPath = strPath;
}




// ---------------------------------------------------------------------------
//   CLASS: TWSEchoCfg
//  PREFIX: wsec
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWSEchoCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TWSEchoCfg::TWSEchoCfg() :

    m_bMapLoaded(kCIDLib::False)
    , m_c4LineNum(0)
{
}

TWSEchoCfg::~TWSEchoCfg()
{
}


// ---------------------------------------------------------------------------
//  TWSEchoCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Checks to see if the passed key maps to one the keys we got from the file
tCIDLib::TBoolean TWSEchoCfg::bCheckKey(const TString& strToCheck) const
{
    TLocker lockrSync(&m_mtxSync);

    const tCIDLib::TCard4 c4Count = m_colKeys.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colKeys[c4Index].bCompareI(strToCheck))
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Returns our loaded flag. We don't need to lock. If it's set, then it's loaded
//  and nothing to do. If it's not, then the caller will call bLoadMaps(). If
//  two happen to call, one will get blocked until it's loaded.
//
tCIDLib::TBoolean TWSEchoCfg::bLoaded() const
{
    return m_bMapLoaded;
}


//
//  The echo config handler calls this to load the config if it has not been
//  already. It can force us to reload if it needs to (because the user asked
//  that it be reloaded.)
//
tCIDLib::TBoolean TWSEchoCfg::bLoadMaps(const tCIDLib::TBoolean bForce)
{
    TLocker lockrSync(&m_mtxSync);

    // If already loaded, and they aren't forcing it, then we are already good
    if (m_bMapLoaded && !bForce)
        return kCIDLib::False;

    try
    {
        // Reset us to stat loading again
        Reset();

        // Figure out the path
        TPathStr pathFile(facCQCKit().strServerDataDir());
        pathFile.AddLevel(L"Echo");
        pathFile.AddLevel(L"EchoCfg.Txt");

        if (!TFileSys::bExists(pathFile))
        {
            facCQCWebSrvC().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcEcho_NoConfigFile
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }

        TTextFileInStream strmSrc
        (
            pathFile
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Read
        );

        // First should be the main echo config block
        TString strKeys;
        CheckLine(strmSrc, L"EchoConfig");
        CheckKey(strmSrc, L"MyKey", strKeys, kCIDLib::False);
        CheckLine(strmSrc, L"EndEchoConfig");

        // Parse out the list of keys, which will just be a comma separated list
        {
            TStringTokenizer stokKeys(&strKeys, L",");
            TString strCurKey;
            while (stokKeys.bGetNextToken(strCurKey))
            {
                strCurKey.StripWhitespace();
                m_colKeys.objAdd(strCurKey);
            }
        }

        // Next we should see the maps all in a row, all present even if empty
        tCIDLib::TStrList colTmp;
        LoadMap(strmSrc, m_colRunMap, colTmp, L"Run");
        LoadMap(strmSrc, m_colSetMap, colTmp, L"Set");
        LoadMap(strmSrc, m_colQueryMap, colTmp, L"Query");

        // We have loaded it
        m_bMapLoaded = kCIDLib::True;
    }

    catch(TError& errToCatch)
    {
        // Make sure we are reset
        Reset();

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }

    catch(...)
    {
        // Make sure we are reset
        Reset();

        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"A system exception occurred while parsing the Echo config file"
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean
TWSEchoCfg::bMapPhrase( const   TString&            strIntentType
                        , const TString&            strPhraseWeGot
                        ,       TString&            strPreferredPhrase
                        ,       TString&            strPath
                        ,       tCIDLib::TBoolean&  bIsMacro) const
{
    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    // Figure out which map to get it from
    const TCfgMap* pcolMap = nullptr;
    if (strIntentType == L"CQCQueryIntent")
    {
        pcolMap = &m_colQueryMap;
    }
     else if ((strIntentType == L"CQCRunIntent")
          ||  (strIntentType == L"CQCRunWithIntent"))
    {
        pcolMap = &m_colRunMap;
    }
     else if ((strIntentType == L"CQCSetToIntent")
          ||  (strIntentType == L"CQCSetIntent")
          ||  (strIntentType == L"CQCSetOffIntent")
          ||  (strIntentType == L"CQCSetOnIntent")
          ||  (strIntentType == L"CQCSetDnIntent")
          ||  (strIntentType == L"CQCSetUpIntent"))
    {
        pcolMap = &m_colSetMap;
    }
     else
    {
        facCQCWebSrvC().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcEcho_UnknownIntentType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strIntentType
        );

        // Won't happen but makes the analyzer happy
        return kCIDLib::False;
    }

    // Let's search the one we found
    const tCIDLib::TCard4 c4Count = pcolMap->c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TWSEchoCfgBlock& wsecbCur = pcolMap->objAt(c4Index);
        if (wsecbCur.bMatches(strPhraseWeGot, strPreferredPhrase))
        {
            strPath = wsecbCur.strPath();
            bIsMacro = wsecbCur.bIsMacro();
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}



// ---------------------------------------------------------------------------
//  TWSEchoCfg: Private, non-virtual methods
//
//  We don't need to lock in any of these. We assume the public method locked
//  across all these calls.
// ---------------------------------------------------------------------------

//
//  A fault tolerant test to see if the current line has a given value. If not, it
//  will be pushed back to be gotten next time.
//
tCIDLib::TBoolean
TWSEchoCfg::bCheckLine(         TTextInStream&      strmSrc
                        , const TString&            strToCheck)
{
    // If we hit the end of file, then obviously not
    TString strLine;
    if (!bReadLine(strmSrc, strLine, kCIDLib::False))
        return kCIDLib::False;

    // We got something, see if it's what we want
    if (strLine != strToCheck)
    {
        CIDAssert(m_c4LineNum != 0, L"Line number underflow during push back");
        m_strPushback = strLine;
        m_c4LineNum--;
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Read the next line, skipping empty lines or comments
tCIDLib::TBoolean
TWSEchoCfg::bReadLine(          TTextInStream&      strmSrc
                        ,       TString&            strToFill
                        , const tCIDLib::TBoolean   bEndOK)
{
    static const TString strCommentPref(L"//");

    // If we have a pushback line, give that first
    if (!m_strPushback.bIsEmpty())
    {
        m_c4LineNum++;
        strToFill = m_strPushback;
        m_strPushback.Clear();
        return kCIDLib::True;
    }

    while (!strmSrc.bEndOfStream())
    {
        strmSrc.c4GetLine(strToFill);
        m_c4LineNum++;

        // Get a line of text. Skip it if its empty
        strToFill.StripWhitespace();
        if (strToFill.bIsEmpty())
            continue;

        // If it starts with a //, skip it as a comment
        if (strToFill.bStartsWith(strCommentPref))
            continue;

        // Must be something useful, so break out
        break;
    }

    // If we got nothing, and it's not ok to hit the end, that's bad
    if (strToFill.bIsEmpty() && !bEndOK)
    {
        if (!bEndOK)
        {
            facCQCWebSrvC().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcEcho_UnexpectedEOF
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotAllRead
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Check a key=value type line that it starts with the indicted key and
//  return the value we got. The caller indicates if an empty value is
//  acceptable or not. If not we throw.
//
tCIDLib::TVoid
TWSEchoCfg::CheckKey(       TTextInStream&      strmSrc
                    , const TString&            strToCheck
                    ,       TString&            strValue
                    , const tCIDLib::TBoolean   bEmptyOk)
{
    TString strLine;
    bReadLine(strmSrc, strLine, kCIDLib::False);

    // Break the two parts out
    if (!strLine.bSplit(strValue, kCIDLib::chEquals)
    ||  (strLine != strToCheck))
    {
        facCQCWebSrvC().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcEcho_ExpectedCfg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllRead
            , strToCheck
            , TCardinal(m_c4LineNum)
        );
    }

    strValue.StripWhitespace();
    if (!bEmptyOk && strValue.bIsEmpty())
    {
        facCQCWebSrvC().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcEcho_ExpectedCfg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllRead
            , TString(L"non-empty value for key ") + strToCheck
            , TCardinal(m_c4LineNum)
        );
    }
}


//
//  Similar to above but we have two possible values that the key can equal.
//  We return both the value and which of the keys we found.
//
tCIDLib::TVoid
TWSEchoCfg::CheckKey2(          TTextInStream&      strmSrc
                        , const TString&            strToCheck1
                        , const TString&            strToCheck2
                        ,       TString&            strKey
                        ,       TString&            strValue
                        , const tCIDLib::TBoolean   bEmptyOk)
{
    bReadLine(strmSrc, strKey, kCIDLib::False);

    // Break the two parts out
    if (!strKey.bSplit(strValue, kCIDLib::chEquals)
    ||  ((strKey != strToCheck1) && (strKey != strToCheck2)))
    {
        TString strExp(strToCheck1);
        strExp.Append(L" OR ");
        strExp.Append(strToCheck2);
        facCQCWebSrvC().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcEcho_ExpectedCfg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllRead
            , strExp
            , TCardinal(m_c4LineNum)
        );
    }

    strValue.StripWhitespace();
    if (!bEmptyOk && strValue.bIsEmpty())
    {
        facCQCWebSrvC().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcEcho_ExpectedCfg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllRead
            , TString(L"non-empty value for key ") + strKey
            , TCardinal(m_c4LineNum)
        );
    }
}


// Get the next line and sees if it equals the passed check value
tCIDLib::TVoid
TWSEchoCfg::CheckLine(TTextInStream& strmSrc, const TString& strToCheck)
{
    TString strLine;
    bReadLine(strmSrc, strLine, kCIDLib::False);
    if (strLine != strToCheck)
    {
        facCQCWebSrvC().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcEcho_ExpectedCfg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllRead
            , strToCheck
            , TCardinal(m_c4LineNum)
        );
    }
}


//
//  Loads a single map. We get called here with each of the possible mapping
//  collections. The code is the same for all of them.
//
tCIDLib::TVoid
TWSEchoCfg::LoadMap(        TTextInStream&      strmSrc
                    ,       TCfgMap&            colToFill
                    ,       tCIDLib::TStrList&  colTmpList
                    , const TString&            strMapName)
{
    TString strStartLine(strMapName);
    strStartLine.Append(L"Map");
    TString strEndLine(L"End");
    strEndLine.Append(strStartLine);

    TString strCurType;
    TString strCurPhrase;
    TString strCurList;
    TString strCurPath;
    TString strBlockEnd;
    TWSEchoCfgBlock wsecbCur;
    TStringTokenizer stokLine;

    CheckLine(strmSrc, strStartLine);
    while (kCIDLib::True)
    {
        // If we see the end, done with this block
        if (bCheckLine(strmSrc, strEndLine))
            break;

        // Else has to be an action or macro, followed by the target path
        CheckKey2(strmSrc, L"Action", L"Macro", strCurType, strCurPath, kCIDLib::False);

        // Make sure the path is well formed, adjust for 5.x changes, etc...
        facCQCKit().PrepRemBrwsPath(strCurPath);

        // Set up our local cfg block for this new block
        wsecbCur.Set(strCurType.bCompareI(L"Macro"), strCurPath);

        // Loop till we hit the end of this block, adding phrase lists to it
        strBlockEnd = L"End";
        strBlockEnd.Append(strCurType);
        while (kCIDLib::True)
        {
            if (bCheckLine(strmSrc, strBlockEnd))
                break;

            //
            //  Get the next line and tokenize it on commas to get all of the
            //  related phrases, adding them to the temp string list.
            //
            colTmpList.RemoveAll();
            bReadLine(strmSrc, strCurList, kCIDLib::False);

            stokLine.Reset(&strCurList, L",");
            while (stokLine.bGetNextToken(strCurPhrase))
            {
                strCurPhrase.StripWhitespace();
                colTmpList.objAdd(strCurPhrase);
            }

            // And now add this phrase list to the config block
            wsecbCur.AddPhraseList(colTmpList);
        }

        // Add this block to the map
        colToFill.objAdd(wsecbCur);
    }
}


// Reset us to empty
tCIDLib::TVoid TWSEchoCfg::Reset()
{
    m_c4LineNum = 0;
    m_strPushback.Clear();

    m_colQueryMap.RemoveAll();
    m_colRunMap.RemoveAll();
    m_colSetMap.RemoveAll();
    m_bMapLoaded = kCIDLib::False;
}





// ---------------------------------------------------------------------------
//   CLASS: TWSEchoHandler
//  PREFIX: urlh
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TWSEchoHandler: Constructors and Destructor
// ---------------------------------------------------------------------------
TWSEchoHandler::TWSEchoHandler() :

    m_ctarGlobalVars(tCIDLib::EMTStates::Unsafe, kCIDLib::False)
{
}

TWSEchoHandler::~TWSEchoHandler()
{
}


// ---------------------------------------------------------------------------
// TWSEchoHandler: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4
TWSEchoHandler::c4ProcessURL(const  TURL&               urlReq
                            , const TString&            strType
                            , const tCIDLib::TKVPList&
                            ,       tCIDLib::TKVPList&
                            , const tCIDLib::TKVPList&
                            , const TString&            strEncoding
                            , const TString&
                            ,       THeapBuf&           mbufData
                            ,       tCIDLib::TCard4&    c4ContLen
                            ,       TString&            strContType
                            ,       TString&            strRepText)
{
    try
    {
        // Make sure the incoming is JSON
        if (!strContType.bCompareI(L"application/json"))
        {
            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The Echo msg was not JSON content"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );

            c4ContLen = c4MakeReply
            (
                L"CQC rejected the message I sent as invalid. Please contact your "
                L"CQC administrator."
                , mbufData
            );
            return kCIDNet::c4HTTPStatus_BadRequest;
        }

        // One way or another the return will be JSON, utf-8 encoded
        strContType = L"application/json; charset=utf-8";

        //
        //  If we haven't loaded our map file yet, do so now. Do a non-forced
        //  load so that, if another thread beats us to it, we won't turn
        //  around and reload it again.
        //
        if (!s_wsecMaps.bLoaded())
        {
            if (!s_wsecMaps.bLoadMaps(kCIDLib::False))
            {
                c4ContLen = c4MakeReply
                (
                    L"The CQC configuration file could not be loaded. Please contact "
                    L"your CQC administrator."
                    , mbufData
                );
                return kCIDNet::c4HTTPStatus_SrvError;
            }
        }

        //
        //  The body text is JSON, so let's parse in the JSON object. Since we
        //  can resuse this buffer, it also means we can give away the contents
        //  to the stream for efficiency. If there's no error, it never needs to
        //  get reallocated.
        //
        TTextMBufInStream strmSrc
        (
            tCIDLib::ForceMove(mbufData)
            , c4ContLen
            , facCIDEncode().ptcvtMake(strEncoding)
        );
        TJSONValue* pjprsnNew = m_jprsComm.pjprsnParse(strmSrc);

        // If not a container, that's an error
        if (!pjprsnNew->bIsContType())
        {
            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The Echo msg was not a JSON container"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );

            c4ContLen = c4MakeReply
            (
                L"CQC rejected the message I sent as invalid. Please contact your "
                L"CQC administrator."
                , mbufData
            );
            return kCIDNet::c4HTTPStatus_BadRequest;
        }
        TJSONCont* pjprsnMsg = static_cast<TJSONCont*>(pjprsnNew);

        // We just need to get out the small number of values sent
        TString strEchoKey;
        TString strEchoIntent;
        TString strEchoPhrase;

        if (!pjprsnMsg->bQueryVal(L"MyKey", strEchoKey)
        ||  !pjprsnMsg->bQueryVal(L"Type", strEchoIntent)
        ||  !pjprsnMsg->bQueryVal(L"Name", strEchoPhrase))
        {
            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"A required Echo msg value was not provided"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );

            c4ContLen = c4MakeReply
            (
                L"CQC rejected the message I sent as invalid. Please contact your "
                L"CQC administrator."
                , mbufData
            );
            return kCIDNet::c4HTTPStatus_BadRequest;
        }

        //
        //  The value is optional, at least here. It may be required by the handler
        //  who will send back an error if not provided.
        //
        TString strEchoValue;
        pjprsnMsg->bQueryVal(L"Value", strEchoValue);

        // Log the info we got, for now it's unconditional
        {
            TString strLogInfo;
            strLogInfo = L"Key=";
            strLogInfo.Append(strEchoKey);
            strLogInfo.Append(L", Intent=");
            strLogInfo.Append(strEchoIntent);
            strLogInfo.Append(L", Phrase=");
            strLogInfo.Append(strEchoPhrase);
            strLogInfo.Append(L", Value=");
            strLogInfo.Append(strEchoValue);

            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , strLogInfo
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        // Validate the key
        if (!s_wsecMaps.bCheckKey(strEchoKey))
        {
            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Rejected Echo request due to bad key"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );

            c4ContLen = c4MakeReply
            (
                L"CQC rejected the account key that I sent, something must be configured"
                L" incorrectly."
                , mbufData
            );
            return kCIDNet::c4HTTPStatus_SrvError;
        }

        //
        //  Watch for the special command to reload the map file. We handle this
        //  one ourself, it's no in a map file.
        //
        if ((strEchoIntent == L"CQCRunIntent")
        &&  (strEchoPhrase.bCompareI(L"ECHO CONFIGURATION")
        ||   strEchoPhrase.bCompareI(L"CONFIGURATION")))
        {
            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Reloading configuration file upon user request"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );

            // Do a forced reload of the maps
            if (s_wsecMaps.bLoadMaps(kCIDLib::True))
            {
                strRepText = L"The CQC Echo configuration file has been reloaded";
            }
             else
            {
                strRepText = L"The CQC Echo configuration file reloaded failed. Only "
                             L"this reload command will function until the file is "
                             L"successfully loaded";
            }
            c4ContLen = c4MakeReply(strRepText, mbufData);
        }
         else
        {
            // See if this one is one we understand
            tCIDLib::TBoolean   bIsMacro;
            TString             strPath;
            TString             strReportPhrase;

            tCIDLib::TBoolean bMapOK = s_wsecMaps.bMapPhrase
            (
                strEchoIntent, strEchoPhrase, strReportPhrase, strPath, bIsMacro
            );

            if (bMapOK)
            {
                // It is, so run the op and get back the reply text
                TString strRepText;
                const tCIDLib::TBoolean bRes = bDoEchoOp
                (
                    strEchoIntent
                    , strReportPhrase
                    , strPath
                    , bIsMacro
                    , strEchoValue
                    , strEchoKey
                    , strRepText
                );

                if (!bRes)
                {
                    strRepText = L"The CQC handler for this command failed. "
                                 L"Check with your CQC administrator.";

                    c4ContLen = c4MakeReply(strRepText, mbufData);
                    return kCIDNet::c4HTTPStatus_SrvError;
                }

                //
                //  It worked so build up teh reply and fall through to the bottom
                //  with the success reply code.
                //
                c4ContLen = c4MakeReply(strRepText, mbufData);
            }
             else
            {
                // Dunno how to handle this one so return the appropriate error
                TString strMsg;
                if (strEchoIntent == L"CQCQueryIntent")
                {
                    strMsg = L"CQC doesn't know how to answer that question, ";
                             L"please try again";
                }
                 else if ((strEchoIntent == L"CQCRunIntent")
                      ||  (strEchoIntent == L"CQCRunWithIntent"))
                {
                    strMsg = L"CQC doesn't know how to run '";
                    strMsg.Append(strEchoPhrase);
                    strMsg.Append(L"'. Please try again");
                }
                 else if ((strEchoIntent == L"CQCSetToIntent")
                      ||  (strEchoIntent == L"CQCSetIntent")
                      ||  (strEchoIntent == L"CQCSetOffIntent")
                      ||  (strEchoIntent == L"CQCSetOnIntent")
                      ||  (strEchoIntent == L"CQCSetDnIntent")
                      ||  (strEchoIntent == L"CQCSetUpIntent"))
                {
                    strMsg = L"CQC doesn't know how to set '";
                    strMsg.Append(strEchoPhrase);
                    strMsg.Append(L"'. Please try again");
                }
                 else
                {
                    strMsg = L"CQC rejected the message because it did not understand the Echo "
                             L"intent type. Something must be configured incorrectly.";
                }

                facCQCWebSrvC().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , strMsg
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );

                c4ContLen = c4MakeReply(strMsg, mbufData);
                return kCIDNet::c4HTTPStatus_BadRequest;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        // Build up an appropriate JSON error response
        c4ContLen = c4MakeReply
        (
            L"An error occurred while CQC was processing the request", mbufData
        );
        return 500;
    }

    return kCIDNet::c4HTTPStatus_OK;
}



// ---------------------------------------------------------------------------
// TWSEchoHandler: Private, static data
// ---------------------------------------------------------------------------
TWSEchoCfg  TWSEchoHandler::s_wsecMaps;



// ---------------------------------------------------------------------------
// TWSEchoHandler: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when we seem to have all of the needed and apparently legal info
//  to invoke a handler. The 'report phrase' is the one that the config file indicated
//  was the one to send to the handler (out of the possible optional phrases for that
//  particular line in the config file.)
//
tCIDLib::TBoolean
TWSEchoHandler::bDoEchoOp(  const   TString&            strIntentType
                            , const TString&            strReportPhrase
                            , const TString&            strPath
                            , const tCIDLib::TBoolean   bIsMacro
                            , const TString&            strEchoValue
                            , const TString&            strEchoKey
                            ,       TString&            strRepText)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    if (bIsMacro)
    {
        TCIDMacroEngine meToUse;
        try
        {
            //
            //  Set up our macro engine and the handlers we need and get them plugged
            //  into the macro engine.
            //
            TCQCMEngErrHandler          meehLogger;
            TMEngFixedBaseFileResolver  mefrData(facCQCKit().strMacroRootPath());
            TMacroEngParser             meprsToUse;
            meToUse.SetErrHandler(&meehLogger);
            meToUse.SetFileResolver(&mefrData);

            // Build up the class path, based on the file style path we get
            TString strClassPath(L"MEng.");
            strClassPath.AppendSubStr(strPath, 1);

            // We could get either type of slash
            strClassPath.bReplaceChar(kCIDLib::chBackSlash, kCIDLib::chPeriod);
            strClassPath.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chPeriod);

            // Try to download and parse the macro. If this fails, we return an error page
            TMEngClassInfo* pmeciTar;
            {
                TCQCMEngClassMgr    mecmCQC(facCQCWebSrvC().sectUser());
                TCQCPrsErrHandler   meehParse;
                if (!meprsToUse.bParse(strClassPath, pmeciTar, &meToUse, &meehParse, &mecmCQC))
                {
                    facCQCWebSrvC().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"The Echo handler CML macro could not be parsed"
                        , strClassPath
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::CantDo
                    );
                    return kCIDLib::False;
                }
            }

            //
            //  Looks like it worked, so create the value object and invoke the
            //  default ctor.
            //
            TMEngClassVal* pmecvTarget = pmeciTar->pmecvMakeStorage
            (
                L"$Main$", meToUse, tCIDMacroEng::EConstTypes::NonConst
            );
            TJanitor<TMEngClassVal> janClass(pmecvTarget);

            // Call it's default constructor
            if (!meToUse.bInvokeDefCtor(*pmecvTarget, &facCQCWebSrvC().cuctxToUse()))
            {
                facCQCWebSrvC().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The Echo handler CML macro's default constructor failed"
                    , strClassPath
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::CantDo
                );
                return kCIDLib::False;
            }

            //
            //  Search this class for a legal entry point. It must have particular name
            //  and form.
            //
            TMEngClassInfo& meciTarget = meToUse.meciFind(pmecvTarget->c2ClassId());
            const tCIDLib::TCard2 c2MethodId = meToUse.c2FindEntryPoint(meciTarget, 0);

            // Make sure it has the four params we need
            const TMEngMethodInfo& methFind = meciTarget.methiFind(c2MethodId);
            if ((methFind.c4ParmCount() < 4)
            ||  (methFind.c4ParmCount() > 5)
            ||  (methFind.mepiFind(0).c2ClassId() != tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::String))
            ||  (methFind.mepiFind(1).c2ClassId() != tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::String))
            ||  (methFind.mepiFind(2).c2ClassId() != tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::String))
            ||  (methFind.mepiFind(3).c2ClassId() != tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::String))
            ||  (methFind.mepiFind(0).eDir() != tCIDMacroEng::EParmDirs::In)
            ||  (methFind.mepiFind(1).eDir() != tCIDMacroEng::EParmDirs::In)
            ||  (methFind.mepiFind(2).eDir() != tCIDMacroEng::EParmDirs::In)
            ||  (methFind.mepiFind(3).eDir() != tCIDMacroEng::EParmDirs::Out))
            {
                facCQCWebSrvC().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The Echo handler CML macro's parameters were incorrect"
                    , strClassPath
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::CantDo
                );
                return kCIDLib::False;
            }

            // If five, then the last one has to be an input string
            if (methFind.c4ParmCount() == 5)
            {
                if ((methFind.mepiFind(4).c2ClassId() != tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::String))
                ||  (methFind.mepiFind(4).eDir() != tCIDMacroEng::EParmDirs::In))
                {
                    facCQCWebSrvC().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"The Echo handler CML macro's parameters were incorrect"
                        , strClassPath
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::CantDo
                    );
                    return kCIDLib::False;
                }
            }

            //
            //  Create four parameters, they are:
            //
            //  1. The incoming intent name
            //  2. The incoming phrase to report to the handler
            //  3. The incoming intent value, could be empty for some
            //  4. An output string for reply text
            //
            //  If they provide the extra, optional, parameter, provide the key
            //  5. The echo key from the AWS side
            //
            TCIDMacroEngine::TParmList colParms(tCIDLib::EAdoptOpts::Adopt);

            const TMEngClassInfo& meciString = meToUse.meciFind(tCIDMacroEng::EIntrinsics::String);

            TMEngStringVal* pmecvIntent= static_cast<TMEngStringVal*>
            (
                meciString.pmecvMakeStorage(L"Intent", meToUse, tCIDMacroEng::EConstTypes::Const)
            );
            pmecvIntent->strValue(strIntentType);
            colParms.Add(pmecvIntent);

            TMEngStringVal* pmecvKey = static_cast<TMEngStringVal*>
            (
                meciString.pmecvMakeStorage(L"Phrase", meToUse, tCIDMacroEng::EConstTypes::Const)
            );
            pmecvKey->strValue(strReportPhrase);
            colParms.Add(pmecvKey);

            TMEngStringVal* pmecvParms = static_cast<TMEngStringVal*>
            (
                meciString.pmecvMakeStorage(L"Parms", meToUse, tCIDMacroEng::EConstTypes::Const)
            );
            pmecvParms->strValue(strEchoValue);
            colParms.Add(pmecvParms);

            TMEngStringVal* pmecvRepText = static_cast<TMEngStringVal*>
            (
                meciString.pmecvMakeStorage(L"RepText", meToUse, tCIDMacroEng::EConstTypes::NonConst)
            );
            colParms.Add(pmecvRepText);

            if (methFind.c4ParmCount() == 5)
            {
                TMEngStringVal* pmecvKey = static_cast<TMEngStringVal*>
                (
                    meciString.pmecvMakeStorage(L"EchoKey", meToUse, tCIDMacroEng::EConstTypes::Const)
                );
                pmecvKey->strValue(strEchoKey);
                colParms.Add(pmecvKey);
            }

            // And invoke the macro, passing along the configured parameters
            const tCIDLib::TInt4 i4Res = meToUse.i4Run
            (
                *pmecvTarget, colParms, &facCQCWebSrvC().cuctxToUse()
            );

            // If the reply text isn't empty, give it back
            strRepText = pmecvRepText->strValue();
        }

        // Handle a CML level exception here since the caller won't understand them
        catch(const TExceptException&)
        {
            TString strClass;
            TString strText;
            tCIDLib::TCard4 c4Line;
            meToUse.bGetLastExceptInfo(strClass, strText, c4Line);

            TString strMsg(L"[", 1024);
            strMsg.Append(strClass);
            strMsg.Append(kCIDLib::chComma);
            strMsg.AppendFormatted(c4Line);
            strMsg.Append(L"] - ");
            strMsg.Append(strText);

            // We rethrow it as a regular CIDLib exception
            TLogEvent logevToLog
            (
                facCQCWebSrvC().strName()
                , strClass
                , c4Line
                , strMsg
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
            );
            logevToLog.strProcess(L"CQCWebSrv.exe");
            logevToLog.strHostName(TSysInfo::strIPHostName());

            TModule::LogEventObj(logevToLog);
            bRet = kCIDLib::False;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            bRet = kCIDLib::False;
        }
    }
     else
    {
        // Try to download the indicated global action
        try
        {
            TCQCStdCmdSrc csrcRun;
            {
                TDataSrvClient dsclLoad;
                tCIDLib::TCard4 c4SerialNum = 0;
                tCIDLib::TEncodedTime enctLastChange;
                tCIDLib::TKVPFList colMeta;
                dsclLoad.bReadGlobalAction
                (
                    strPath
                    , c4SerialNum
                    , enctLastChange
                    , csrcRun
                    , colMeta
                    , facCQCWebSrvC().cuctxToUse().sectUser()
                );
            }

            //
            //  Set up the parameters. We pass in:
            //
            //  1. The intent type
            //  2. They key that was triggered
            //  3. The value, if relevant, provided
            //  4. The key from the AWS side
            //
            TString strParms(L"\"");
            strParms.Append(strIntentType);
            strParms.Append(L"\", \"");
            strParms.Append(strReportPhrase);
            strParms.Append(L"\", \"");
            strParms.Append(strEchoValue);
            strParms.Append(L"\", \"");
            strParms.Append(strEchoKey);
            strParms.Append(L"\"");

            m_ctarGlobalVars.DeleteAllVars();
            TCQCStdActEngine acteToUse(facCQCWebSrvC().cuctxToUse());
            const tCQCKit::ECmdRes eRes = acteToUse.eInvokeOps
            (
                csrcRun
                , kCQCKit::strEvId_OnTrigger
                , m_ctarGlobalVars
                , acteToUse.colExtra()
                , nullptr
                , strParms
            );

            //
            //  If it worked, then let's see if they provided any reply text. If
            //  not, we'll provide a default. If it failed, we provide the text.
            //
            if (eRes < tCQCKit::ECmdRes::Except)
            {
                if (!m_ctarGlobalVars.bVarValue(L"GVar:EchoReply", strRepText))
                    strRepText = L"OK, CQC has done that";
            }
             else
            {
                facCQCWebSrvC().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Unsuccessful return status from action. Path=%(1)"
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , strPath
                );
                bRet = kCIDLib::False;
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            bRet = kCIDLib::False;
        }
    }

    return bRet;
}


//
//  All replies are created via this helper We return the number of bytes we put
//  into the provided memory buffer.
//
tCIDLib::TCard4
TWSEchoHandler::c4MakeReply(const   TString&            strText
                            ,       TMemBuf&            mbufToFill)
{
    TTextMBufOutStream strmTar
    (
        &mbufToFill, tCIDLib::EAdoptOpts::NoAdopt, new TUTF8Converter
    );

    strmTar << L"{\n" << L"\"Reply\" : \"" << strText
            << L"\"\n}\n" << kCIDLib::FlushIt;

    return strmTar.c4CurSize();
}


