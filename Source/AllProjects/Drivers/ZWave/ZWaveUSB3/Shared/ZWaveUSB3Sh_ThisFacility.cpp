//
// FILE NAME: ZWaveUSB3Sh_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/31/2017
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
//  This is the implementation for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3Sh_.hpp"


// ---------------------------------------------------------------------------
//  Localtypes and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSB3Sh_ThisFacility
{
    //
    //  A map for individual devices. We use a unique id in the form of a 64 bit
    //  value. The low 16 is the product id. The next is the type, and the next is
    //  the manufacturer. The top word isn't used.
    //
    //  We have a compiler that compiles the files and copies them to the
    //  appropriate place and which creates an index file. That is used both by
    //  us to see if we have a map for a given id and find the file to load, and by
    //  the client to present a list of available types to the user.
    //
    //  The main driver class will force this to load during his initialization, so
    //  it always should be available. It is read only once loaded, however we can be
    //  asked to reload it. Though we currently know it only to be accessed by the driver
    //  thread (or the replication thread during which time the main driver thread will
    //  not do anything) we synchronize it anyway.
    //
    //  For performance reasons we also create a list of unique makes since the client
    //  driver needs that when the user wants to select a make/model for a unit.
    //
    //
    //  We then have a cache of actual files we've loaded. They won't have some of the
    //  info, but the caller can get the other info (node id, generic, specific types
    //  mostly) by discovery and does so. As with the index this also probably doesn't
    //  need sync, but we do so anyway.
    //
    //  Both are sorted on the manids so they can be binary searched.
    //
    class TDevMapItem
    {
        public :
            TDevMapItem();
            TDevMapItem(const   tCIDLib::TCard8             c8ManIds
                        , const TString&                    strName
                        , const TString&                    strInstruct
                        , const TString&                    strFile
                        , const TString                     strMake
                        , const TString                     strModel
                        , const TString                     strCppClass) :

                m_c8ManIds(c8ManIds)
                , m_strCppClass(strCppClass)
                , m_strFile(strFile)
                , m_strInstruct(strInstruct)
                , m_strMake(strMake)
                , m_strModel(strModel)
                , m_strName(strName)
            {
            }
            ~TDevMapItem() {}

            tCIDLib::TCard8             m_c8ManIds;
            TString                     m_strCppClass;
            TString                     m_strFile;
            TString                     m_strInstruct;
            TString                     m_strMake;
            TString                     m_strModel;
            TString                     m_strName;
    };

    typedef TVector<TZWDevInfo>         TDevInfoCache;

    tCIDLib::TBoolean       bDevMapLoaded = kCIDLib::False;
    TVector<TDevMapItem>    colDevInfoMap;
    tCIDLib::TStrList       colUniqueMakes;
    TDevInfoCache           colDevInfoCache(32);

    // What we return if we can't map an id to a manufacturer or product name
    const TString   strUnknown(L"Unknown");


    //
    //  A verion simple class to provide a mapping table for command class cmd ids
    //  to text. Trying to memorize the ids is a waste of time so we need to, for the
    //  CCs we understand, be able to display text commands.
    //
    //  We combine the class id and the cmd id into a Card4 and use that as a key, to
    //  map to a string. We insert the ids sorted, so a binary lookup quickly finds
    //  the name if it's present. 0 is an 'unknown' value.
    //
    //  We fault this in since it's typically only needed if tracing or verbose logging
    //  is enabled.
    //
    //  The text is assumed to be just literal text values that we don't need to clean
    //  up.
    //
    class TCCmdIdItem
    {
        public :
            TCCmdIdItem() :
                m_c4Id(0)
                , m_pszText(nullptr)
            {
            }

            TCCmdIdItem(const   tCIDLib::TCard1     c1Class
                        , const tCIDLib::TCard1     c1Cmd
                        , const tCIDLib::TCh* const pszText) :

                m_c4Id((tCIDLib::TCard4(c1Class) << 16) | c1Cmd)
                , m_pszText(pszText)
            {
            }

            ~TCCmdIdItem() {}

            tCIDLib::TCard4         m_c4Id;
            const tCIDLib::TCh*     m_pszText;
    };

    tCIDLib::TBoolean       bCCCmdIdMapLoaded = kCIDLib::False;
    TVector<TCCmdIdItem>    colCCmdIdMap;
}

tCIDLib::ESortComps
eCompDevMapItem(const   ZWaveUSB3Sh_ThisFacility::TDevMapItem& item1
                , const ZWaveUSB3Sh_ThisFacility::TDevMapItem& item2)
{
    if (item1.m_c8ManIds < item2.m_c8ManIds)
        return tCIDLib::ESortComps::FirstLess;
    else if (item1.m_c8ManIds > item2.m_c8ManIds)
        return tCIDLib::ESortComps::FirstGreater;

    return tCIDLib::ESortComps::Equal;
}

tCIDLib::ESortComps
eKeyCompCCmdIdItem( const   tCIDLib::TCard4&                        c4Key
                    , const ZWaveUSB3Sh_ThisFacility::TCCmdIdItem&  item2)
{
    if (c4Key < item2.m_c4Id)
        return tCIDLib::ESortComps::FirstLess;
    else if (c4Key > item2.m_c4Id)
        return tCIDLib::ESortComps::FirstGreater;

    return tCIDLib::ESortComps::Equal;
}


// ---------------------------------------------------------------------------
//   CLASS: TFacZWaveUSB3Sh
//  PREFIX: fac
// ---------------------------------------------------------------------------

// -------------------------------------------------------------------
//  Constructors and Destructor
// -------------------------------------------------------------------
TFacZWaveUSB3Sh::TFacZWaveUSB3Sh() :

    TFacility
    (
        L"ZWaveUSB3Sh"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_c1CtrlId(kCIDLib::c1MaxCard)
    , m_ckeyNetwork(16)
    , m_enctLastMsg(0)
    , m_eTraceLevel(tCQCKit::EVerboseLvls::Off)
    , m_eVerboseLevel(tCQCKit::EVerboseLvls::Off)
    , m_pstrmTrace(nullptr)
{
    // Set the default key that we will use if we aren't in the network yet
    THeapBuf mbufKey(16, 16);
    mbufKey.Set(0);
    m_ckeyNetwork.Set(mbufKey);

    // Initialize our encrypters based on this key initially
    SecuritySetup();
}

TFacZWaveUSB3Sh::~TFacZWaveUSB3Sh()
{
    // If not alreayd cleaned up
    try
    {
        if (m_pstrmTrace)
            m_pstrmTrace->Flush();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
//  TFacZWaveUSB3Sh: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Check the passed buffer to make sure it has a legal (non-inclusion) network key.
//
tCIDLib::TBoolean TFacZWaveUSB3Sh::bLegalNetworkKey(const TMemBuf& mbufToCheck) const
{
    if (mbufToCheck.c4Size() != 16)
        return kCIDLib::False;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < 16; c4Index++)
    {
        if (mbufToCheck[c4Index] != 0)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Indicates if the passed encoded man ids represent a generic device handler, which is
//  one that we didn't map ourself the user selected it. So we never override that, and
//  of course we know that we any ids we find ourself won't match it.
//
//  For our generic ones the manufacturer and product ids are both 0xFFFF.
//
tCIDLib::TBoolean TFacZWaveUSB3Sh::bIsGenericManId(const tCIDLib::TCard8 c8ToCheck) const
{
    return (c8ToCheck & 0xFFFFFFFF0000) == 0xFFFFFFFF0000;
}


// Returns true if tracing is enabled and medium or low verbosity or higher is set
tCIDLib::TBoolean TFacZWaveUSB3Sh::bHighTrace() const
{
    return (m_eTraceLevel >= tCQCKit::EVerboseLvls::High);
}

tCIDLib::TBoolean TFacZWaveUSB3Sh::bLowTrace() const
{
    return (m_eTraceLevel >= tCQCKit::EVerboseLvls::Low);
}

tCIDLib::TBoolean TFacZWaveUSB3Sh::bMediumTrace() const
{
    return (m_eTraceLevel >= tCQCKit::EVerboseLvls::Medium);
}


//
//  We will try to find the passed man id in our device info map. If so, we'll give
//  the caller a copy of our information if we find the passed ids.
//
//  We have an issue where with locking. The files come from the installation server,
//  and we don't want to lock the cache across a remote call. So we first check the
//  cache, and, if not found, we unlock and try to load it. If we get it, we re-lock
//  and insert it into the cache if someone else didn't beat us to it.
//
tCIDLib::TBoolean
TFacZWaveUSB3Sh::bQueryDevInfo( const   tCIDLib::TCard8 c8ToFind
                                ,       TZWDevInfo&     deviToFill
                                ,       TString&        strErr) const
{
    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    // Let's see if we have this guy in our cache
    tCIDLib::TCard4 c4At = c4FindCacheIndex(c8ToFind);

    // If so, take it now
    if (c4At != kCIDLib::c4MaxCard)
    {
        deviToFill = ZWaveUSB3Sh_ThisFacility::colDevInfoCache[c4At];
        return kCIDLib::True;
    }

    //
    //  Not there, so see if it's in our map index. If it is, try to load the file
    //  associated with the man ids.
    //
    c4At = c4FindMapIndex(c8ToFind);
    if (c4At != kCIDLib::c4MaxCard)
    {
        // Get the file name, a copy of it in this case since we are going to unlock!
        TString strFile = ZWaveUSB3Sh_ThisFacility::colDevInfoMap[c4At].m_strFile;

        // Now let's unlock and try to download the file
        lockrSync.Release();

        THeapBuf mbufFile;
        tCIDLib::TCard4 c4FileSz = 0;
        try
        {
            tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
            if (!orbcIS->bQueryZWDIFile(strFile, c4FileSz, mbufFile))
            {
                strErr = L"The associated device info file could not be downloaded from the "
                         L"Master Server.";
                return kCIDLib::False;
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            strErr = L"An error occurred while downloading the device info file. See the logs";
            return kCIDLib::False;
        }

        // New add the extension to the file name for any logging and such below
        strFile.Append(L".zwdi");

        // Create an entity src ref to feed to the XML parser
        tCIDXML::TEntitySrcRef esrRoot
        (
            new TMemBufEntitySrc(strFile, mbufFile, c4FileSz)
        );

        //
        //  We need to set our already downloaded DTD on the parser since he can't
        //  load it directly. The public ID is what is generated into the XML files
        //  which is how the parser links them up.
        //
        TXMLTreeParser xtprsToUse;
        xtprsToUse.AddMapping(m_esrDTD);

        // And now we should be able to parse the file
        try
        {
            if (!xtprsToUse.bParseRootEntity(esrRoot
                                            , tCIDXML::EParseOpts::None
                                            , tCIDXML::EParseFlags::TagsNText))
            {
                const TXMLTreeParser::TErrInfo& erriFirst = xtprsToUse.colErrors()[0];
                TTextStringOutStream strmOut(512);
                strmOut << L"[" << erriFirst.strSystemId() << L"/"
                        << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                        << L"] " << erriFirst.strText() << kCIDLib::FlushIt;

                facZWaveUSB3Sh().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kZW3ShErrs::errcCfg_ParseDI
                    , strmOut.strData()
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }

            // Let the passed object parse himself out
            deviToFill.ParseFrom(xtprsToUse.xtdocThis().xtnodeRoot());

            // We got it, so we need to reaquire the lock and add it to the cache
            lockrSync.Lock();

            //
            //  But we also need to add it to our cache, sorted by the man ids. Put
            //  the resulting insertion index into the return value. He gives us back
            //  the index. First make sure someone didn't beat us to it while we were
            //  unlocked.
            //
            if (c4FindCacheIndex(c8ToFind) == kCIDLib::c4MaxCard)
            {
                ZWaveUSB3Sh_ThisFacility::colDevInfoCache.InsertSorted
                (
                    deviToFill, &TZWDevInfo::eCompByManIds, c4At
                );
            }
        }

        catch(TError& errToCatch)
        {
            if (!xtprsToUse.colErrors().bIsEmpty())
            {
                const TXMLTreeParser::TErrInfo& erriFirst = xtprsToUse.colErrors()[0];
                TTextStringOutStream strmOut(512);
                strmOut << L"[" << erriFirst.strSystemId() << L"/"
                        << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                        << L"] " << erriFirst.strText() << kCIDLib::FlushIt;

                facZWaveUSB3Sh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kZW3ShErrs::errcCfg_ParseDI
                    , strmOut.strData()
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }

            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            strErr = L"The device info file could not be parsed.";
            return kCIDLib::False;
        }
    }

    if (c4At == kCIDLib::c4MaxCard)
    {
        strErr = L"No device info file matched that passed manufacturer ids";
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  Query some basic info by man ids from our device info map. This stuff we store
//  in the map itself, so we don't need to load any device info files to get this.
//
tCIDLib::TBoolean
TFacZWaveUSB3Sh::bQueryDevInfo( const   tCIDLib::TCard8             c8ManIds
                                ,       TString&                    strMake
                                ,       TString&                    strModel
                                ,       TString&                    strCppClass) const
{
    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    // Let's see if we have this guy in our map
    tCIDLib::TCard4 c4At = c4FindMapIndex(c8ManIds);
    if (c4At != kCIDLib::c4MaxCard)
    {
        const ZWaveUSB3Sh_ThisFacility::TDevMapItem& itemInfo
        (
            ZWaveUSB3Sh_ThisFacility::colDevInfoMap[c4At]
        );

        strMake = itemInfo.m_strMake;
        strModel = itemInfo.m_strModel;
        strCppClass = itemInfo.m_strCppClass;
    }
    return (c4At != kCIDLib::c4MaxCard);
}


// For the client interface to get a list of makes and models for a specific make
tCIDLib::TBoolean TFacZWaveUSB3Sh::bQueryMakes(tCIDLib::TStrList& colToFill) const
{
    colToFill.RemoveAll();

    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    // Make sure it's loaded. This could be the first time its been accessed
    if (!ZWaveUSB3Sh_ThisFacility::bDevMapLoaded)
    {
        LoadDevInfoMap();
        if (!ZWaveUSB3Sh_ThisFacility::bDevMapLoaded)
            return kCIDLib::False;
    }

    // We already have a cached list of unique makes
    colToFill = ZWaveUSB3Sh_ThisFacility::colUniqueMakes;

    return kCIDLib::True;
}


//
//  For the key/values list, the key is the model description. Value 1 is the literal
//  model (which is the actual second part of the make/model key.) Value 2 is the
//  man ids formatted to text (in hex.) Value 3 is the instruction text.
tCIDLib::TBoolean
TFacZWaveUSB3Sh::bQueryModels(  const   TString&                strMake
                                ,       tCIDLib::TKValsList&    colToFill) const
{
    colToFill.RemoveAll();

    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    // Make sure it's loaded. This could be the first time its been accessed
    if (!ZWaveUSB3Sh_ThisFacility::bDevMapLoaded)
    {
        LoadDevInfoMap();
        if (!ZWaveUSB3Sh_ThisFacility::bDevMapLoaded)
            return kCIDLib::False;
    }

    // Find all of the items that match the passed make
    const TVector<ZWaveUSB3Sh_ThisFacility::TDevMapItem>& colMap
    (
        ZWaveUSB3Sh_ThisFacility::colDevInfoMap
    );
    TString strManIds;
    const tCIDLib::TCard4 c4Count = colMap.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const ZWaveUSB3Sh_ThisFacility::TDevMapItem& itemCur = colMap[c4Index];
        if (itemCur.m_strMake == strMake)
        {
            strManIds.SetFormatted(itemCur.m_c8ManIds, tCIDLib::ERadices::Hex);
            colToFill.objAdd
            (
                TKeyValues
                (
                    itemCur.m_strName
                    , itemCur.m_strModel
                    , strManIds
                    , itemCur.m_strInstruct
                )
            );
        }
    }
    return kCIDLib::True;
}


//
//  Returns the notes for the indicated man ids (device info file) if we have it
//  in our list.
//
tCIDLib::TBoolean
TFacZWaveUSB3Sh::bQueryModelNotes(const tCIDLib::TCard8 c8ManIds, TString& strToFill) const
{
    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    // Make sure it's loaded. This could be the first time its been accessed
    if (!ZWaveUSB3Sh_ThisFacility::bDevMapLoaded)
    {
        LoadDevInfoMap();
        if (!ZWaveUSB3Sh_ThisFacility::bDevMapLoaded)
            return kCIDLib::False;
    }

    // Let's see if we have this guy in our map
    tCIDLib::TCard4 c4At = c4FindMapIndex(c8ManIds);
    if (c4At == kCIDLib::c4MaxCard)
        strToFill.Clear();
    else
        strToFill = ZWaveUSB3Sh_ThisFacility::colDevInfoMap[c4At].m_strInstruct;

    return (c4At != kCIDLib::c4MaxCard) && !strToFill.bIsEmpty();
}


// If the level is not off, then tracing is enabled
tCIDLib::TBoolean TFacZWaveUSB3Sh::bTraceEnabled() const
{
    return (m_eTraceLevel != tCQCKit::EVerboseLvls::Off);
}


//
//  Some helpers for the very common operation of sending out command class
//  messages.
//
tCIDLib::TVoid
TFacZWaveUSB3Sh::BuildCCMsg(        TZWOutMsg&              zwomToFill
                            , const TZWUnitInfo&            unitiTar
                            , const tCIDLib::TCard1         c1ClassId
                            , const tCIDLib::TCard1         c1CmdId
                            , const tCIDLib::TCard1         c1ReplyCmdId
                            , const tZWaveUSB3Sh::EMsgPrios ePriority
                            , const tCIDLib::TBoolean       bReceiptAck) const
{
    zwomToFill.Reset(unitiTar.c1Id(), c1ClassId, c1CmdId, c1ReplyCmdId, 2, ePriority);
    zwomToFill.AppendTransOpts(bReceiptAck);
    zwomToFill.AppendCallback();
    zwomToFill.Finalize(unitiTar);
}

tCIDLib::TVoid
TFacZWaveUSB3Sh::BuildCCMsg(        TZWOutMsg&              zwomToFill
                            , const TZWUnitInfo&            unitiTar
                            , const tCIDLib::TCard1         c1ClassId
                            , const tCIDLib::TCard1         c1CmdId
                            , const tCIDLib::TCard1         c1ReplyCmdId
                            , const tCIDLib::TCard1         c1Data1
                            , const tZWaveUSB3Sh::EMsgPrios ePriority
                            , const tCIDLib::TBoolean       bReceiptAck) const
{
    zwomToFill.Reset(unitiTar.c1Id(), c1ClassId, c1CmdId, c1ReplyCmdId, 3, ePriority);
    zwomToFill += c1Data1;
    zwomToFill.AppendTransOpts(bReceiptAck);
    zwomToFill.AppendCallback();
    zwomToFill.Finalize(unitiTar);
}

tCIDLib::TVoid
TFacZWaveUSB3Sh::BuildCCMsg(        TZWOutMsg&              zwomToFill
                            , const TZWUnitInfo&            unitiTar
                            , const tCIDLib::TCard1         c1ClassId
                            , const tCIDLib::TCard1         c1CmdId
                            , const tCIDLib::TCard1         c1ReplyCmdId
                            , const tCIDLib::TCard1         c1Data1
                            , const tCIDLib::TCard1         c1Data2
                            , const tZWaveUSB3Sh::EMsgPrios ePriority
                            , const tCIDLib::TBoolean       bReceiptAck) const
{
    zwomToFill.Reset(unitiTar.c1Id(), c1ClassId, c1CmdId, c1ReplyCmdId, 4, ePriority);
    zwomToFill += c1Data1;
    zwomToFill += c1Data2;
    zwomToFill.AppendTransOpts(bReceiptAck);
    zwomToFill.AppendCallback();
    zwomToFill.Finalize(unitiTar);
}

tCIDLib::TVoid
TFacZWaveUSB3Sh::BuildCCMsg(        TZWOutMsg&              zwomToFill
                            , const TZWUnitInfo&            unitiTar
                            , const tCIDLib::TCard1         c1ClassId
                            , const tCIDLib::TCard1         c1CmdId
                            , const tCIDLib::TCard1         c1ReplyCmdId
                            , const tCIDLib::TCard1         c1Data1
                            , const tCIDLib::TCard1         c1Data2
                            , const tCIDLib::TCard1         c1Data3
                            , const tZWaveUSB3Sh::EMsgPrios ePriority
                            , const tCIDLib::TBoolean       bReceiptAck) const
{
    zwomToFill.Reset(unitiTar.c1Id(), c1ClassId, c1CmdId, c1ReplyCmdId, 5, ePriority);
    zwomToFill += c1Data1;
    zwomToFill += c1Data2;
    zwomToFill += c1Data3;

    zwomToFill.AppendTransOpts(bReceiptAck);
    zwomToFill.AppendCallback();
    zwomToFill.Finalize(unitiTar);
}

tCIDLib::TVoid
TFacZWaveUSB3Sh::BuildCCMsg(        TZWOutMsg&              zwomToFill
                            , const TZWUnitInfo&            unitiTar
                            , const tCIDLib::TCard1         c1ClassId
                            , const tCIDLib::TCard1         c1CmdId
                            , const tCIDLib::TCard1         c1ReplyCmdId
                            , const tCIDLib::TCard1         c1Data1
                            , const tCIDLib::TCard1         c1Data2
                            , const tCIDLib::TCard1         c1Data3
                            , const tCIDLib::TCard1         c1Data4
                            , const tZWaveUSB3Sh::EMsgPrios ePriority
                            , const tCIDLib::TBoolean       bReceiptAck) const
{
    zwomToFill.Reset(unitiTar.c1Id(), c1ClassId, c1CmdId, c1ReplyCmdId, 6, ePriority);
    zwomToFill += c1Data1;
    zwomToFill += c1Data2;
    zwomToFill += c1Data3;
    zwomToFill += c1Data4;

    zwomToFill.AppendTransOpts(bReceiptAck);
    zwomToFill.AppendCallback();
    zwomToFill.Finalize(unitiTar);
}


// Make our Z-Stick controller id available to everyone
tCIDLib::TCard1 TFacZWaveUSB3Sh::c1CtrlId() const
{
    return m_c1CtrlId;
}


//
//  A multi-level switch dimmer level is 0 to 99, but we deal in percents for dimmers
//  so we have to adjust it. Why in hell they didn't use 0 to 100 boggles the mind,
//  since they are so close and percents are so commonly used.
//
tCIDLib::TCard1 TFacZWaveUSB3Sh::c1PercentToDim(const tCIDLib::TCard4 c4ToMap)
{
    // If not a valid percent, return max card
    if (c4ToMap > 100)
        return 0xFF;

    tCIDLib::TFloat8 f8Val = (tCIDLib::TFloat8(c4ToMap) / 100.0) * 99.0;
    TMathLib::Round(f8Val);

    tCIDLib::TCard1 c1Ret = tCIDLib::TCard1(f8Val);
    if (c1Ret > 99)
        c1Ret = 99;
    return c1Ret;
}


// Convert a Z-Wave dimmer level to a percent
tCIDLib::TCard4 TFacZWaveUSB3Sh::c4DimToPercent(const tCIDLib::TCard1 c1ToMap)
{
    //  We have to treat 0xFF as 100%
    if (c1ToMap == 0xFF)
        return 100;

    // If not valid otherwise, return max card
    if (c1ToMap > 0x63)
        return kCIDLib::c4MaxCard;

    // Otherwise, map it
    tCIDLib::TFloat8 f8Val = (tCIDLib::TFloat8(c1ToMap) / 99.0) * 100.0;
    TMathLib::Round(f8Val);
    tCIDLib::TCard4 c4Ret = tCIDLib::TCard4(f8Val);
    if (c4Ret > 100)
        c4Ret = 100;
    return c4Ret;
}


//
//  Calculates an authentication code for a secure message. Used for outgoing msgs
//  to create it, and for incoming to verify it.
//
tCIDLib::TVoid
TFacZWaveUSB3Sh::CalculateMAC(  const   tCIDLib::TCard1         c1SecCmdId
                                , const tCIDLib::TCard1* const  pc1EncData
                                , const tCIDLib::TCard4         c4Len
                                , const tCIDLib::TCard1         c1Sender
                                , const tCIDLib::TCard1         c1Receiver
                                , const TMemBuf&                mbufIV
                                ,       TMemBuf&                mbufToFill)
{
    THeapBuf mbufData(64, 256);
    THeapBuf mbufTmpAuth(16, 16);
    mbufData.Set(0);
    mbufTmpAuth.Set(0);

    tCIDLib::TCard4 c4Cnt = 0;
    mbufData.PutCard1(c1SecCmdId, c4Cnt++);
    mbufData.PutCard1(c1Sender, c4Cnt++);
    mbufData.PutCard1(c1Receiver, c4Cnt++);

    // Put the encrypted bytes length and the bytes themselves into the buffer
    mbufData.PutCard1(tCIDLib::TCard1(c4Len), c4Cnt++);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Len; c4Index++)
        mbufData.PutCard1(pc1EncData[c4Index], c4Cnt++);

    //
    //  Zero pad to a 16 byte boundary. The buffer was zero filled, so they are already
    //  there. So we just need to adjust the count.
    //
    {
        const tCIDLib::TCard4 c4PadCnt = (c4Cnt % 16);
        if (c4PadCnt)
            c4Cnt += (16 - c4PadCnt);
    }


    // Lock for the rest of it so the key can't change
    TLocker lockrSync(&m_mtxSync);

    //
    //  Encrypt the IV to create the temp auth value. It's set to ECB mode, which is
    //  what we want here.
    //
    m_crypAuth.c4Encrypt(mbufIV, mbufTmpAuth, 16);

    THeapBuf mbufEncPack(16, 16);
    mbufEncPack.Set(0);

    THeapBuf mbufTmp(16, 16);
    tCIDLib::TCard4 c4BlockInd = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Cnt; c4Index++)
    {
        mbufEncPack.PutCard1(mbufData[c4Index], c4BlockInd);
        c4BlockInd++;

        // Encrypt each block as we get it
        if (c4BlockInd == 16)
        {
            for (tCIDLib::TCard4 c4AuthInd = 0; c4AuthInd < 16; c4AuthInd++)
                mbufTmpAuth.PutCard1(mbufEncPack[c4AuthInd] ^ mbufTmpAuth[c4AuthInd], c4AuthInd);

            // Reset for the next block
            mbufEncPack.Set(0, 0);
            c4BlockInd = 0;

            // We encrypt it back into itself
            mbufTmp.CopyIn(mbufTmpAuth, 16);
            m_crypAuth.c4Encrypt(mbufTmp, mbufTmpAuth, 16);
        }
    }

    // And we keep the first 8 bytes as the MAC
    mbufToFill.CopyIn(mbufTmpAuth, 8);
}


tCIDLib::TVoid
TFacZWaveUSB3Sh::Decrypt(const  TMemBuf&        mbufCypher
                        , const tCIDLib::TCard4 c4Count
                        ,       TMemBuf&        mbufPlain
                        , const TMemBuf&        mbufIV)
{
    //
    //  Figure out how many blocks that would have been, since the decrypter always
    //  expects to get full blocks.
    //
    tCIDLib::TCard4 c4Blocks = c4Count / 16;
    if (c4Count % 16)
        c4Blocks++;

    // Lock for the rest of it so the key can't change
    TLocker lockrSync(&m_mtxSync);

    m_crypEncrypt.c4Decrypt(mbufCypher, mbufPlain, c4Blocks * 16, mbufIV.pc1Data());
}


// Flushes the trace debugging stream if tracing is enabled
tCIDLib::TVoid TFacZWaveUSB3Sh::FlushMsgTrace()
{
    TLocker lockrSync(&m_mtxSync);
    if (m_pstrmTrace)
        m_pstrmTrace->Flush();
}


//
//  This is called to load up our device info map, which we do by parsing the device
//  info index XML file and pulling out the basic info we need. We just need the
//  man ids, file name, and some other basic info. We use the file name to later
//  get to the full info if it is needed.
//
//  We load it into a temp, then lock and store the data. The data is downloaded
//  from the installation server.
//
tCIDLib::TVoid TFacZWaveUSB3Sh::LoadDevInfoMap() const
{
    tCIDLib::TBoolean bStoreTmpMap = kCIDLib::False;
    TVector<ZWaveUSB3Sh_ThisFacility::TDevMapItem> colTmpMap;
    tCIDLib::TStrList colTmpUMakes;
    TPathStr pathFile;
    try
    {
        // Get an installation server proxy and use that to download the index file
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        THeapBuf mbufIndex;
        tCIDLib::TCard4 c4IndexSz = 0;
        THeapBuf mbufDTD;
        tCIDLib::TCard4 c4DTDSz = 0;
        if (!orbcIS->bQueryZWDIIndex(c4IndexSz, mbufIndex, c4DTDSz, mbufDTD))
        {
            // If we can't download it, then we failed
            facZWaveUSB3Sh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW3ShErrs::errcCfg_DownloadZWDI
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        //
        //  Set up the entity source for the DTD. We keep this around since we
        //  need to use it every time we parse device info files in the future.
        //  We have to be sure to set the public id since we are going to add it
        //  as a mapping to the parser.
        //
        m_esrDTD.SetPointer
        (
            new TMemBufEntitySrc
            (
                L"ZWaveDevInfo.DTD"
                , L"urn:charmedquark.com:CQC-ZWDevInfo.DTD"
                , mbufDTD
                , c4DTDSz
            )
        );

        //
        //  And now parse the index. WE need to create an entiry source ref for the
        //  memory buffer we just downloaded, to feed it ot the parser.
        //
        TXMLTreeParser xtprsToUse;
        try
        {
            tCIDXML::TEntitySrcRef esrRoot
            (
                new TMemBufEntitySrc(L"ZWU3DevInfoIndex.xml", mbufIndex, c4IndexSz)
            );

            if (!xtprsToUse.bParseRootEntity(esrRoot
                                            , tCIDXML::EParseOpts::None
                                            , tCIDXML::EParseFlags::TagsNText))
            {
                const TXMLTreeParser::TErrInfo& erriFirst = xtprsToUse.colErrors()[0];
                TTextStringOutStream strmOut(512);
                strmOut << L"[" << erriFirst.strSystemId() << L"/"
                        << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                        << L"] " << erriFirst.strText() << kCIDLib::FlushIt;

                facZWaveUSB3Sh().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kZW3ShErrs::errcCfg_ParseDI
                    , strmOut.strData()
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facZWaveUSB3Sh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW3ShErrs::errcCfg_ParseDI
                , pathFile
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        //
        //  The stuff we want is a level down. So we have to iterate the root and, for
        //  each Make entry we need to then iterate the Dev entries under that.
        //
        tCIDLib::TStrHashSet colUnique(109, TStringKeyOps(kCIDLib::False));
        TStringTokenizer stokParse;
        TString strTmp;
        TString strInstruct;
        const TXMLTreeElement& xtnodeRoot = xtprsToUse.xtdocThis().xtnodeRoot();
        const tCIDLib::TCard4 c4MakeCnt = xtnodeRoot.c4ChildCount();
        for (tCIDLib::TCard4 c4MIndex = 0; c4MIndex < c4MakeCnt; c4MIndex++)
        {
            // Skip non-elements
            if (xtnodeRoot.eChildTypeAt(c4MIndex) != tCIDXML::ENodeTypes::Element)
                continue;

            const TXMLTreeElement& xtnodeMake = xtnodeRoot.xtnodeChildAtAsElement(c4MIndex);
            const TXMLTreeAttr& xtattrMake = xtnodeMake.xtattrNamed(L"Name");

            const tCIDLib::TCard4 c4DevCnt = xtnodeMake.c4ChildCount();
            for (tCIDLib::TCard4 c4DIndex = 0; c4DIndex < c4DevCnt; c4DIndex++)
            {
                if (xtnodeMake.eChildTypeAt(c4DIndex) != tCIDXML::ENodeTypes::Element)
                    continue;

                const TXMLTreeElement& xtnodeDev = xtnodeMake.xtnodeChildAtAsElement(c4DIndex);
                const TXMLTreeAttr& xtattrCppClass = xtnodeDev.xtattrNamed(L"CppClass");
                const TXMLTreeAttr& xtattrFile = xtnodeDev.xtattrNamed(L"File");
                const TXMLTreeAttr& xtattrManIds = xtnodeDev.xtattrNamed(L"ManIds");
                const TXMLTreeAttr& xtattrModel = xtnodeDev.xtattrNamed(L"Model");
                const TXMLTreeAttr& xtattrName = xtnodeDev.xtattrNamed(L"Name");
                const tCIDLib::TCard8 c8ManIds = xtattrManIds.strValue().c8Val();

                // Get the instruction element if available
                tCIDLib::TCard4 c4At;
                const TXMLTreeElement* pxtnodeInstruct = xtnodeDev.pxtnodeFindElement
                (
                    L"Instruct", 0, c4At
                );
                if (pxtnodeInstruct)
                {
                    strInstruct = pxtnodeInstruct->xtnodeChildAtAsText(0).strText();

                    // Replace any new lines with spaces
                    tCIDLib::TCard4 c1At = 0;
                    strInstruct.bReplaceSubStr(L"\r\n", L" ", c4At, kCIDLib::True);
                    c4At = 0;
                    strInstruct.bReplaceSubStr(L"\n", L" ", c4At, kCIDLib::True);

                    // Replace any multi-space values with a single space
                    strInstruct.Strip(L" ", tCIDLib::EStripModes::Full);

                    // Now replace any literal \n strings with newlines
                    c4At = 0;
                    TString strRepWith(L"\r\n");
                    strInstruct.bReplaceSubStr
                    (
                        L"\\n", strRepWith, c4At, kCIDLib::True
                    );
                }
                 else
                {
                    strInstruct.Clear();
                }

                // It has to be sorted by man ids since we binary search on that
                colTmpMap.InsertSorted
                (
                    ZWaveUSB3Sh_ThisFacility::TDevMapItem
                    (
                        c8ManIds
                        , xtattrName.strValue()
                        , strInstruct
                        , xtattrFile.strValue()
                        , xtattrMake.strValue()
                        , xtattrModel.strValue()
                        , xtattrCppClass.strValue()
                    )
                    , &eCompDevMapItem
                    , c4At
                );

                // If not already seen, add this make to our list of makes
                if (!colUnique.bHasElement(xtattrMake.strValue()))
                {
                    colUnique.objAdd(xtattrMake.strValue());
                    colTmpUMakes.objAdd(xtattrMake.strValue());
                }
            }
        }

        // It seems to have worked, so let's store it
        bStoreTmpMap = kCIDLib::True;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facZWaveUSB3Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCfg_ParseDI
            , pathFile
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
        );
        return;
    }

    // Store it if asked to and set the loaded flag
    TLocker lockrSync(&m_mtxSync);
    if (bStoreTmpMap)
    {
        ZWaveUSB3Sh_ThisFacility::colUniqueMakes = colTmpUMakes;
        ZWaveUSB3Sh_ThisFacility::colDevInfoMap = colTmpMap;
    }
    ZWaveUSB3Sh_ThisFacility::bDevMapLoaded = kCIDLib::True;
}



// Log an incoming message to the trace if it is enabled.
tCIDLib::TVoid
TFacZWaveUSB3Sh::LogInMsg(const tCIDLib::TCh* const pszPrefix, const TZWInMsg& zwimMsg)
{
    if (!facZWaveUSB3Sh().bTraceEnabled())
        return;

    // Make sure this is a single chunk of output
    TLocker lockrSync(&m_mtxSync);

    TTextOutStream& strmOut = *m_pstrmTrace;
    tCIDLib::TCard1 c1Cur;

    TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
    tmCur.strDefaultFormat(TTime::str24HHMMSS());
    tCIDLib::TCard4 c4Indent = 0;
    if (pszPrefix)
    {
        strmOut << tmCur << L" - [MSG] - " << pszPrefix << L"\n{\n";
        c4Indent = 4;
    }
     else
    {
        strmOut << tmCur << L" - ";
    }

    {
        TStreamIndentJan janIndent(m_pstrmTrace, c4Indent);

        //
        //  Show the direction and whether it is a request or response and the
        //  function id.
        //
        strmOut << L"[ZW->DR] - ";
        if (zwimMsg.m_eType == tZWaveUSB3Sh::EMsgTypes::Request)
            strmOut << L"REQ,Msg:";
        else if (zwimMsg.m_eType == tZWaveUSB3Sh::EMsgTypes::Response)
            strmOut << L"RES,Msg:";
        else if (zwimMsg.m_eType == tZWaveUSB3Sh::EMsgTypes::TransAck)
            strmOut << L"TRANSACK,Msg:";
        else if (zwimMsg.m_eType == tZWaveUSB3Sh::EMsgTypes::Ack)
            strmOut << L"ACK";
        else if (zwimMsg.m_eType == tZWaveUSB3Sh::EMsgTypes::Can)
            strmOut << L"CAN";
        else if (zwimMsg.m_eType == tZWaveUSB3Sh::EMsgTypes::Nak)
            strmOut << L"NAK";
        else
            strmOut << L"???,Msg:";

        // If no msg id, it's one of the special ack/nak/can ones so we are done
        if (!zwimMsg.m_c1MsgId)
        {
            strmOut << L"\n";
            return;
        }

        LogMsgId(zwimMsg.m_c1MsgId);
        if (zwimMsg.m_bSecure)
            strmOut << L",Secure";

        tmCur = zwimMsg.enctReceived();
        strmOut << L",Recvd: " << tmCur;

        if (zwimMsg.bWasEncapsulated())
        {
            strmOut << L", Src/Tar EP: " << zwimMsg.c1SrcEPId()
                    << L"/" << zwimMsg.c1TarEPId();
        }

        strmOut << L") ";

        const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
        if (m_enctLastMsg != 0)
        {
            tCIDLib::TEncodedTime enctOfs(enctNow - m_enctLastMsg);
            enctOfs /= kCIDLib::enctOneMilliSec;
            strmOut << enctOfs << L"MSs";
        }
        m_enctLastMsg = enctNow;

        strmOut << L"\n{";

        // If it was original encrypted, display the original info
        if (zwimMsg.m_bSecure)
        {
            TStreamIndentJan janIndent(m_pstrmTrace, 4);

            const TZWInMsg& zwimOrg = *zwimMsg.m_pzwimOrg;
            strmOut << L"\nEncrypted Data:\n{"
                    << L"\n    SrcId: " << TCardinal(zwimOrg[1], tCIDLib::ERadices::Hex)
                    << L"\n    Bytes: ";
            for (tCIDLib::TCard4 c4Index = 0; c4Index < zwimOrg.m_c1PLLength; c4Index++)
            {
                c1Cur = zwimOrg[c4Index];
                if (c1Cur < 0x10)
                    strmOut << L"0";
                strmOut << TCardinal(c1Cur, tCIDLib::ERadices::Hex) << kCIDLib::chSpace;
            }
            strmOut << L"\n}\n";
        }

        //
        //  For some common ones we break the info out and display it. For less
        //  common ones we just display the raw payload bytes.
        //
        if (zwimMsg.m_eType == tZWaveUSB3Sh::EMsgTypes::TransAck)
        {
            strmOut << L"\n   Callback Id: " << zwimMsg[0]
                    << L"\n        Status: ";

            switch(zwimMsg[1])
            {
                case TRANSMIT_COMPLETE_OK :
                    strmOut << L"Success";
                    break;

                case TRANSMIT_COMPLETE_NO_ACK :
                    strmOut << L"No ack received";
                    break;

                case TRANSMIT_COMPLETE_FAIL :
                    strmOut << L"Network jammed";
                    break;

                case TRANSMIT_ROUTING_NOT_IDLE :
                    strmOut << L"Routing not idle";
                    break;

                case TRANSMIT_COMPLETE_NOROUTE :
                    strmOut << L"No route found";
                    break;

                default :
                    strmOut << TCardinal(zwimMsg[1]);
                    break;
            };
        }
         else if (zwimMsg.m_c1MsgId == FUNC_ID_APPLICATION_COMMAND_HANDLER)
        {
            c1Cur = zwimMsg[0];
            strmOut << L"\n    Flags:";

            if (c1Cur & RECEIVE_STATUS_ROUTED_BUSY)
                strmOut << L" Busy";

            strmOut << L"\n    Type:";
            const tCIDLib::TCard1 c1Type = c1Cur & RECEIVE_STATUS_TYPE_MASK;
            if (c1Type == RECEIVE_STATUS_TYPE_BROAD)
                strmOut << L" Broad";
            else if (c1Type == RECEIVE_STATUS_TYPE_MULTI)
                strmOut << L" Multi";
            else if (c1Type == RECEIVE_STATUS_TYPE_SINGLE)
                strmOut << L" Single";
            else
                strmOut << L" ???";

            strmOut << L"\n    SrcId: " << TCardinal(zwimMsg[1], tCIDLib::ERadices::Hex);

            const tCIDLib::TCard1 c1ClassId = zwimMsg[3];
            const tCIDLib::TCard1 c1CmdId = zwimMsg[4];
            FormatCCIds(strmOut, c1ClassId, c1CmdId);

            // Handle some specially, else display as raw bytes
            if ((c1ClassId == COMMAND_CLASS_MANUFACTURER_SPECIFIC)
            &&  (c1CmdId == MANUFACTURER_SPECIFIC_REPORT))
            {
                tCIDLib::TCard2 c2Id;
                c2Id = zwimMsg[5];
                c2Id <<= 8;
                c2Id |= zwimMsg[6];
                strmOut << L"\n    ManId: " << TCardinal(c2Id, tCIDLib::ERadices::Hex);

                c2Id = zwimMsg[7];
                c2Id <<= 8;
                c2Id |= zwimMsg[8];
                strmOut << L"\n    TypeId: " << TCardinal(c2Id, tCIDLib::ERadices::Hex);

                c2Id = zwimMsg[9];
                c2Id <<= 8;
                c2Id |= zwimMsg[10];
                strmOut << L"\n    ProdId: " << TCardinal(c2Id, tCIDLib::ERadices::Hex);

                tCIDLib::TCard8 c8Ids = zwimMsg[5];
                c8Ids <<= 8;
                c8Ids |= zwimMsg[6];
                c8Ids <<= 8;
                c8Ids |= zwimMsg[7];
                c8Ids <<= 8;
                c8Ids |= zwimMsg[8];
                c8Ids <<= 8;
                c8Ids |= zwimMsg[9];
                c8Ids <<= 8;
                c8Ids |= zwimMsg[10];

                strmOut << L"\n    Maps To: " << facZWaveUSB3Sh().strFindDevName(c8Ids);
            }
             else if ((c1ClassId == COMMAND_CLASS_SENSOR_MULTILEVEL)
                  &&  (c1CmdId == SENSOR_MULTILEVEL_REPORT))
            {
                const tCIDLib::TCard4 c4Bytes(tCIDLib::TCard1(zwimMsg.c1CCByteAt(3) & 0x7));
                const tCIDLib::TCard4 c4Prec(tCIDLib::TCard1(zwimMsg.c1CCByteAt(3) >> 5) & 0x7);
                strmOut << L"\n    Type: " << zwimMsg.c1CCByteAt(2)
                        << L"\n    Prec: " << c4Prec
                        << L"\n   Bytes: " << c4Bytes;

                tCIDLib::TCard4 c4Val = 0;
                if (c4Bytes == 1)
                {
                    c4Val = zwimMsg.c1CCByteAt(4);
                }
                 else if (c4Bytes == 2)
                {
                    c4Val = zwimMsg.c1CCByteAt(4);
                    c4Val <<= 8;
                    c4Val |= zwimMsg.c1CCByteAt(4);
                }
                 else if (c4Bytes == 4)
                {
                    c4Val = zwimMsg.c1CCByteAt(4);
                    c4Val <<= 8;
                    c4Val |= zwimMsg.c1CCByteAt(4);
                    c4Val <<= 8;
                    c4Val |= zwimMsg.c1CCByteAt(4);
                    c4Val <<= 8;
                    c4Val |= zwimMsg.c1CCByteAt(4);
                }
                strmOut << L"\n     Value: " << c4Val;
            }
             else if ((c1ClassId == COMMAND_CLASS_NODE_NAMING)
                  &&  ((c1CmdId == NODE_NAMING_NODE_NAME_SET)
                  ||   (c1CmdId == NODE_NAMING_NODE_NAME_REPORT)))
            {
                // Get the name out
                TString strName;
                if (TZWUSB3CCNaming::bExtractNodeName(zwimMsg, strName))
                    strmOut << L"\n    Name: " << strName;
                else
                    strmOut << L"\n    Name: [Unable to extract]";
            }
             else if (c1ClassId == COMMAND_CLASS_SECURITY)
            {
                switch(zwimMsg[4])
                {
                    case NETWORK_KEY_SET :
                        break;

                    case SECURITY_COMMANDS_SUPPORTED_GET :
                        break;

                    case SECURITY_MESSAGE_ENCAPSULATION :
                        break;

                    case SECURITY_MESSAGE_ENCAPSULATION_NONCE_GET :
                        break;

                    case SECURITY_NONCE_GET :
                        break;

                    case SECURITY_SCHEME_GET :
                        break;

                    case SECURITY_SCHEME_INHERIT :
                        break;

                    case SECURITY_NONCE_REPORT :
                        break;

                    default :
                        break;
                };
            }
             else if (c1ClassId == COMMAND_CLASS_VERSION)
            {
                if (c1CmdId == VERSION_COMMAND_CLASS_REPORT)
                {
                    tZWaveUSB3Sh::ECClasses eClass = tZWaveUSB3Sh::eAltNumECClasses(zwimMsg[5]);
                    strmOut << L"\n    ";
                    if (eClass == tZWaveUSB3Sh::ECClasses::None)
                        strmOut << TCardinal(zwimMsg[5], tCIDLib::ERadices::Hex);
                    else
                        strmOut << eClass;
                    strmOut << L"  Version=" << zwimMsg[6];
                }
            }
        }
         else if (zwimMsg.m_c1MsgId == FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION)
        {
            strmOut << L"\n    Src Id: " << zwimMsg[1]
                    << L"\n    Listen: ";
            if (zwimMsg[0] & 0x1)
                strmOut << L"Yes";
            else
                strmOut << L"No";

            tZWaveUSB3Sh::EGenTypes eGenType = tZWaveUSB3Sh::eAltNumEGenTypes(zwimMsg[4]);
            strmOut << L"\n    GenType: ";
            if (eGenType == tZWaveUSB3Sh::EGenTypes::Count)
                strmOut << TCardinal(zwimMsg[4], tCIDLib::ERadices::Hex);
            else
                strmOut << eGenType;
            strmOut << L"\n    SpecType: " << TCardinal(zwimMsg[5], tCIDLib::ERadices::Hex);
        }
         else if (zwimMsg.m_c1MsgId == FUNC_ID_ZW_APPLICATION_UPDATE)
        {
            c1Cur = zwimMsg[0];
            strmOut << L"\n    Type: ";
            if (c1Cur == UPDATE_STATE_ADD_DONE)
            {
                strmOut << L"Add Done";
            }
             else if (c1Cur == UPDATE_STATE_DELETE_DONE)
            {
                strmOut << L"Delete Done";
            }
             else if (c1Cur == UPDATE_STATE_NODE_INFO_RECEIVED)
            {
                strmOut << L"Node Info"
                        << L"\n    SrcId: " << TCardinal(zwimMsg[1], tCIDLib::ERadices::Hex);

                tZWaveUSB3Sh::EGenTypes eGenType = tZWaveUSB3Sh::eAltNumEGenTypes(zwimMsg[4]);
                strmOut << L"\n    GenType: ";
                if (eGenType == tZWaveUSB3Sh::EGenTypes::Count)
                    strmOut << TCardinal(zwimMsg[4], tCIDLib::ERadices::Hex);
                else
                    strmOut << eGenType;

                strmOut << L"\n    SpecType: "
                        << TCardinal(zwimMsg[5], tCIDLib::ERadices::Hex);

                // The rest are the command classes supported
                strmOut << L"\n    Classes:";
                for (tCIDLib::TCard4 c4Index = 6; c4Index < zwimMsg.m_c1PLLength; c4Index++)
                {
                    const tCIDLib::TCard1 c1CC = zwimMsg[c4Index];
                    tZWaveUSB3Sh::ECClasses eClass = tZWaveUSB3Sh::eAltNumECClasses(c1CC);

                    strmOut << kCIDLib::chSpace;
                    if (eClass == tZWaveUSB3Sh::ECClasses::Count)
                        strmOut << TCardinal(c1CC, tCIDLib::ERadices::Hex);
                    else
                        strmOut << eClass;
                }
            }
             else if (c1Cur == UPDATE_STATE_SUC_ID)
            {
                strmOut << L"SUC Id";
            }
             else if (c1Cur == UPDATE_STATE_NODE_INFO_REQ_FAILED)
            {
                strmOut << L"Failed";
            }
             else
            {
                strmOut << L"Unknown (" << TCardinal(c1Cur, tCIDLib::ERadices::Hex) << L")";
            }
        }
         else if (zwimMsg.m_c1MsgId == FUNC_ID_SERIAL_API_GET_CAPABILITIES)
        {
            tCIDLib::TCard2 c2Id;
            c2Id = zwimMsg[2];
            c2Id <<= 8;
            c2Id |= zwimMsg[3];
            strmOut << L"\n    ManId: " << TCardinal(c2Id, tCIDLib::ERadices::Hex);

            c2Id = zwimMsg[4];
            c2Id <<= 8;
            c2Id |= zwimMsg[5];
            strmOut << L"\n    TypeId: " << TCardinal(c2Id, tCIDLib::ERadices::Hex);

            c2Id = zwimMsg[6];
            c2Id <<= 8;
            c2Id |= zwimMsg[7];
            strmOut << L"\n    ProdId: " << TCardinal(c2Id, tCIDLib::ERadices::Hex);
        }
         else if (zwimMsg.m_c1MsgId == FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO)
        {
            strmOut << L"\n       Listen: ";
            if (zwimMsg[0] & NODEINFO_LISTENING_SUPPORT)
                strmOut << L"Yes";
            else
                strmOut << L"No";

            strmOut << L"\n      Beaming: ";
            if (zwimMsg[1] & 0x10)
                strmOut << L"Yes";
            else
                strmOut << L"No";

            strmOut << L"\n  Freq Listen: ";
            if (zwimMsg[1] & 0x60)
                strmOut << L"Yes";
            else
                strmOut << L"No";

            strmOut << L"\n     Security: ";
            if (zwimMsg[1] & 0x1)
                strmOut << L"Yes";
            else
                strmOut << L"No";

            tZWaveUSB3Sh::EDevTypes eDevType = tZWaveUSB3Sh::eAltNumEDevTypes(zwimMsg[3]);
            strmOut << L"\n      DevType: ";
            if (eDevType == tZWaveUSB3Sh::EDevTypes::Count)
                strmOut << TCardinal(zwimMsg[3], tCIDLib::ERadices::Hex);
            else
                strmOut << eDevType;

            tZWaveUSB3Sh::EGenTypes eGenType = tZWaveUSB3Sh::eAltNumEGenTypes(zwimMsg[4]);
            strmOut << L"\n      GenType: " ;
            if (eGenType == tZWaveUSB3Sh::EGenTypes::Count)
                strmOut << TCardinal(zwimMsg[4], tCIDLib::ERadices::Hex);
            else
                strmOut << eGenType;

            strmOut << L"\n     SpecType: " << TCardinal(zwimMsg[5], tCIDLib::ERadices::Hex);
        }

        // Display the raw bytes
        strmOut << L"\n    Bytes: ";
        for (tCIDLib::TCard4 c4Index = 0; c4Index < zwimMsg.m_c1PLLength; c4Index++)
        {
            c1Cur = zwimMsg[c4Index];
            if (c1Cur < 0x10)
                strmOut << L"0";
            strmOut << TCardinal(c1Cur, tCIDLib::ERadices::Hex) << kCIDLib::chSpace;
        }
        strmOut << L"\n}\n";
    }

    if (pszPrefix)
        strmOut << L"}\n";
}


// Log an outgoing message to the trace if it is enabled
tCIDLib::TVoid
TFacZWaveUSB3Sh::LogOutMsg(const tCIDLib::TCh* const pszPrefix, const TZWOutMsg& zwomLog)
{
    if (!facZWaveUSB3Sh().bTraceEnabled())
        return;

    // Make sure this is a single chunk of output
    TLocker lockrSync(&m_mtxSync);

    // We want to log the unencrypted data if it is encrypted
    const TZWOutMsg& zwomPlain = zwomLog.pzwomOrg() ? *zwomLog.pzwomOrg() : zwomLog;

    //
    //  Show the direction and whether it is a request or response and the
    //  function id. We'll translate the ids for some common functions to help
    //  with debugging.
    //
    TTextOutStream& strmTar = *m_pstrmTrace;

    TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
    tmCur.strDefaultFormat(TTime::str24HHMMSS());

    tCIDLib::TCard4 c4Indent = 0;
    strmTar << tmCur << L" - ";;
    if (pszPrefix)
    {
        strmTar << pszPrefix << L"\n{\n";
        c4Indent = 4;
    }

    // Indent most of the rest by the amount set up above
    {
        TStreamIndentJan janIndent(m_pstrmTrace, c4Indent);

        strmTar << L"[DR->ZW] - (";
        if (zwomPlain[1] == REQUEST)
            strmTar << L"REQ,Msg: ";
        else
            strmTar << L"RES,Msg: ";

        const tCIDLib::TCard1 c1MsgId = zwomPlain[2];
        LogMsgId(c1MsgId);
        if (zwomPlain.bSecure())
            strmTar << L",Secure";
        strmTar << L",AckId=" << zwomPlain.c4AckId();

        if (zwomPlain.bNeedsCallback())
            strmTar << L",CBId=" << zwomPlain.c1CallbackId();

        strmTar << L") ";

        const tCIDLib::TCard1 c1Len = zwomPlain[0];
        const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
        if (m_enctLastMsg != 0)
        {
            tCIDLib::TEncodedTime enctOfs(enctNow - m_enctLastMsg);
            enctOfs /= kCIDLib::enctOneMilliSec;
            strmTar << enctOfs << L"MSs";
        }
        m_enctLastMsg = enctNow;

        strmTar << kCIDLib::NewLn;

        // If we don't have more bytes, then we are done
        if (c1Len <= 3)
        {
            strmTar << L"}\n";
            return;
        }

        // We more to log so let's do it
        strmTar << L"{";

        // If this is encrypted, then show the encrypted data
        if (zwomLog.pzwomOrg())
        {
            TStreamIndentJan janIndent(m_pstrmTrace, 4);
            strmTar << L"\nEncrypted Data\n{"
                    << L"\n    TarId: " << TCardinal(zwomLog[3], tCIDLib::ERadices::Hex)
                    << L"\n    Bytes: ";
            const tCIDLib::TCard4 c4Count = zwomLog[0];
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const tCIDLib::TCard1 c1Cur = zwomLog[c4Index];
                if (c1Cur < 0x10)
                    strmTar << L"0";
                strmTar << TCardinal(c1Cur, tCIDLib::ERadices::Hex) << kCIDLib::chSpace;
            }
            strmTar << L"\n}\n";
        }

        if (c1MsgId == FUNC_ID_ZW_SEND_DATA)
        {
            strmTar << L"\n    TarId: " << TCardinal(zwomPlain[3], tCIDLib::ERadices::Hex);

            tCIDLib::TCard1 c1Class = zwomPlain.c1ClassId();
            tZWaveUSB3Sh::ECClasses eClass = tZWaveUSB3Sh::eAltNumECClasses(c1Class);

            //
            //  If it's an encapsulated msg, show the encapsulated stuff, else show the
            //  actual message content.
            //
            tCIDLib::TCard1 c1Cmd = 0;
            if ((eClass == tZWaveUSB3Sh::ECClasses::MultiCh)
            &&  (zwomPlain[6] == MULTI_CHANNEL_CMD_ENCAP_V2))
            {
                strmTar << L"\n   MultiChannel Msg:\n   ------------------"
                        << L"\n     SrcEP: " << zwomPlain[7]
                        << L"\n     TarEP: " << zwomPlain[8];

                c1Class = zwomPlain[9];
                if (c1Class != COMMAND_CLASS_NO_OPERATION)
                    c1Cmd = zwomPlain[10];
                FormatCCIds(strmTar, c1Class, c1Cmd);
            }
             else
            {
                if (c1Class != COMMAND_CLASS_NO_OPERATION)
                    c1Cmd = zwomPlain[6];
                FormatCCIds(strmTar, c1Class, c1Cmd);
            }
        }


        //
        //  Now show the remaining payload bytes. The buffer doesn't include the SOF
        //  or check sum, so the count byte is the full buffer size.
        //
        strmTar << L"\n    Bytes: ";
        for (tCIDLib::TCard1 c1Index = 0; c1Index < c1Len; c1Index++)
        {
            const tCIDLib::TCard1 c1Cur = zwomPlain[c1Index];
            if (c1Cur < 0x10)
                *(m_pstrmTrace) << L"0";
            strmTar << TCardinal(c1Cur, tCIDLib::ERadices::Hex) << kCIDLib::chSpace;
        }
        strmTar << L"\n}\n";
    }

    if (pszPrefix)
        strmTar << L"}\n";
}


// Same as LogTraceMsg below, but put out an error prefix
tCIDLib::TVoid TFacZWaveUSB3Sh::LogTraceErr(const tCIDLib::TCh* const pszMsg) const
{
    if (m_pstrmTrace)
    {
        TLocker lockrSync(&m_mtxSync);
        if (m_pstrmTrace)
        {
            TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
            tmCur.strDefaultFormat(TTime::str24HHMMSS());
            *(m_pstrmTrace) << tmCur << L" - "
                            << L"[ERROR]- " << pszMsg << kCIDLib::NewLn;
        }
    }
}

tCIDLib::TVoid TFacZWaveUSB3Sh::LogTraceErr(const TString& strMsg) const
{
    if (m_pstrmTrace)
    {
        TLocker lockrSync(&m_mtxSync);
        if (m_pstrmTrace)
        {
            TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
            tmCur.strDefaultFormat(TTime::str24HHMMSS());
            *(m_pstrmTrace) << tmCur << L" - "
                            << L"[ERROR]- " << strMsg << kCIDLib::NewLn;
        }
    }
}

tCIDLib::TVoid
TFacZWaveUSB3Sh::LogTraceErr(const  tCIDLib::TCh* const     pszMsg
                            , const MFormattable&           mfmtblToken1
                            , const MFormattable&           mfmtblToken2
                            , const MFormattable&           mfmtblToken3) const
{
    if (m_pstrmTrace)
    {
        TString strMsg(pszMsg, 92UL);
        if (!MFormattable::bIsNullObject(mfmtblToken1))
            strMsg.eReplaceToken(mfmtblToken1, kCIDLib::chDigit1);

        if (!MFormattable::bIsNullObject(mfmtblToken2))
            strMsg.eReplaceToken(mfmtblToken2, kCIDLib::chDigit2);

        if (!MFormattable::bIsNullObject(mfmtblToken3))
            strMsg.eReplaceToken(mfmtblToken3, kCIDLib::chDigit3);

        LogTraceErr(strMsg);
    }
}

tCIDLib::TVoid
TFacZWaveUSB3Sh::LogTraceErr(const  TString&        strMsgText
                            , const MFormattable&   mfmtblToken1
                            , const MFormattable&   mfmtblToken2
                            , const MFormattable&   mfmtblToken3) const
{
    if (m_pstrmTrace)
    {
        TString strMsg(strMsgText, 92UL);
        if (!MFormattable::bIsNullObject(mfmtblToken1))
            strMsg.eReplaceToken(mfmtblToken1, kCIDLib::chDigit1);

        if (!MFormattable::bIsNullObject(mfmtblToken2))
            strMsg.eReplaceToken(mfmtblToken2, kCIDLib::chDigit2);

        if (!MFormattable::bIsNullObject(mfmtblToken3))
            strMsg.eReplaceToken(mfmtblToken3, kCIDLib::chDigit3);

        LogTraceErr(strMsg);
    }
}


//
//  These log msgs with a leading time stamp. We have versions that push out a new line
//  separator first.
//
tCIDLib::TVoid TFacZWaveUSB3Sh::LogTraceMsg(const tCIDLib::TCh* const pszMsg) const
{
    if (m_pstrmTrace)
    {
        TLocker lockrSync(&m_mtxSync);
        if (m_pstrmTrace)
        {
            TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
            tmCur.strDefaultFormat(TTime::str24HHMMSS());
            *(m_pstrmTrace) << tmCur << L" - "
                            << L"[TRACE]- " << pszMsg << kCIDLib::NewLn;
        }
    }
}

tCIDLib::TVoid TFacZWaveUSB3Sh::LogTraceMsgNL(const tCIDLib::TCh* const pszMsg) const
{
    if (m_pstrmTrace)
    {
        TLocker lockrSync(&m_mtxSync);
        if (m_pstrmTrace)
        {
            TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
            tmCur.strDefaultFormat(TTime::str24HHMMSS());
            *(m_pstrmTrace) << L"\n\n***************************\n" << tmCur << L" - "
                            << L"[TRACE]- " << pszMsg << kCIDLib::NewLn;
        }
    }
}

tCIDLib::TVoid TFacZWaveUSB3Sh::LogTraceMsg(const TString& strMsg) const
{
    if (m_pstrmTrace)
    {
        TLocker lockrSync(&m_mtxSync);
        if (m_pstrmTrace)
        {
            TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
            tmCur.strDefaultFormat(TTime::str24HHMMSS());
            *(m_pstrmTrace) << tmCur << L" - "
                            << L"[TRACE]- " << strMsg << kCIDLib::NewLn;
        }
    }
}

tCIDLib::TVoid TFacZWaveUSB3Sh::LogTraceMsgNL(const TString& strMsg) const
{
    if (m_pstrmTrace)
    {
        TLocker lockrSync(&m_mtxSync);
        if (m_pstrmTrace)
        {
            TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
            tmCur.strDefaultFormat(TTime::str24HHMMSS());
            *m_pstrmTrace   << L"\n\n***************************\n" << tmCur << L" - "
                            << L"[TRACE]- " << strMsg << kCIDLib::NewLn;
        }
    }
}

tCIDLib::TVoid
TFacZWaveUSB3Sh::LogTraceMsg(const  tCIDLib::TCh* const     pszMsg
                            , const MFormattable&           mfmtblToken1
                            , const MFormattable&           mfmtblToken2
                            , const MFormattable&           mfmtblToken3
                            , const MFormattable&           mfmtblToken4) const
{
    if (m_pstrmTrace)
    {
        TString strMsg(pszMsg, 92UL);
        if (!MFormattable::bIsNullObject(mfmtblToken1))
            strMsg.eReplaceToken(mfmtblToken1, kCIDLib::chDigit1);

        if (!MFormattable::bIsNullObject(mfmtblToken2))
            strMsg.eReplaceToken(mfmtblToken2, kCIDLib::chDigit2);

        if (!MFormattable::bIsNullObject(mfmtblToken3))
            strMsg.eReplaceToken(mfmtblToken3, kCIDLib::chDigit3);

        if (!MFormattable::bIsNullObject(mfmtblToken4))
            strMsg.eReplaceToken(mfmtblToken4, kCIDLib::chDigit4);

        LogTraceMsg(strMsg);
    }
}

tCIDLib::TVoid
TFacZWaveUSB3Sh::LogTraceMsgNL( const   tCIDLib::TCh* const     pszMsg
                                , const MFormattable&           mfmtblToken1
                                , const MFormattable&           mfmtblToken2
                                , const MFormattable&           mfmtblToken3) const
{
    if (m_pstrmTrace)
    {
        TLocker lockrSync(&m_mtxSync);
        if (m_pstrmTrace)
        {
            TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
            tmCur.strDefaultFormat(TTime::str24HHMMSS());
            *m_pstrmTrace   << L"\n\n***************************\n" << tmCur << L" - "
                            << L"[TRACE]- ";
            LogTraceMsg(pszMsg, mfmtblToken1, mfmtblToken2, mfmtblToken3);
            *m_pstrmTrace << kCIDLib::NewLn;
        }
    }
}


tCIDLib::TVoid
TFacZWaveUSB3Sh::Encrypt(const  TMemBuf&        mbufPlain
                        , const tCIDLib::TCard4 c4Count
                        ,       TMemBuf&        mbufCypher
                        , const TMemBuf&        mbufIV)
{
    TLocker lockrSync(&m_mtxSync);
    m_crypEncrypt.c4Encrypt(mbufPlain, mbufCypher, c4Count, mbufIV.pc1Data());
}


//
//  Used by the trace formatting and sometimes debugging stuff.
//
tCIDLib::TVoid
TFacZWaveUSB3Sh::FormatCCIds(       TTextOutStream& strmOut
                            , const tCIDLib::TCard1 c1ClassId
                            , const tCIDLib::TCard1 c1CmdId)
{
    strmOut << L"\n    Class: ";
    tZWaveUSB3Sh::ECClasses eClass = tZWaveUSB3Sh::eAltNumECClasses(c1ClassId);
    if (eClass == tZWaveUSB3Sh::ECClasses::Count)
    {
        strmOut << TCardinal(c1ClassId, tCIDLib::ERadices::Hex)
                << L"\n     Cmd: " << TCardinal(c1CmdId, tCIDLib::ERadices::Hex);
    }
     else
    {
        const tCIDLib::TCh* pszCmdName = pszXlatCCCmdId(c1ClassId, c1CmdId);
        strmOut << eClass
                << L"\n     Cmd: ";

        if (pszCmdName)
            strmOut << pszCmdName;
        else
            strmOut << TCardinal(c1CmdId, tCIDLib::ERadices::Hex);
    }
}


//
//  Z-Wave is way too complicated, so to help reduce some of that, we need to display
//  human consumable content in the trace file and logs, particularly wrt to the cmd
//  classes and their commands. This one handles translating from a class/cmd id pair
//  to a human readable command name, for at least a core set of classes that we really
//  deal with a lot.
//
//  We fault in a table upon first use
//
const tCIDLib::TCh*
TFacZWaveUSB3Sh::pszXlatCCCmdId(const   tCIDLib::TCard1 c1ClassId
                                , const tCIDLib::TCard1 c1CmdId) const
{
    // If the map has not been faulted in, then do that
    if (!ZWaveUSB3Sh_ThisFacility::bCCCmdIdMapLoaded)
    {
        // Lock, check again and load if no one beat us to it
        TLocker lockrSync(&m_mtxSync);
        if (!ZWaveUSB3Sh_ThisFacility::bCCCmdIdMapLoaded)
        {
            // OK, let's do it. Get a short cut name for the collection we are filling
            TVector<ZWaveUSB3Sh_ThisFacility::TCCmdIdItem>& colMap
            (
                ZWaveUSB3Sh_ThisFacility::colCCmdIdMap
            );

            // ADD THESE IN SORTED ORDER!

            // Add an unknown value at the 0 key, just in case
            colMap.objPlace(0, 0, L"Unknown");

            // Basic
            colMap.objPlace(0x20, 1, L"Set");
            colMap.objPlace(0x20, 2, L"Get");
            colMap.objPlace(0x20, 3, L"Report");

            // Binary switch
            colMap.objPlace(0x25, 1, L"Set");
            colMap.objPlace(0x25, 2, L"Get");
            colMap.objPlace(0x25, 3, L"Report");

            // Binary sensor
            colMap.objPlace(0x30, 2, L"Get");
            colMap.objPlace(0x30, 3, L"Report");

            // Meter
            colMap.objPlace(0x32, 1, L"Get");
            colMap.objPlace(0x32, 2, L"Report");

            // Color switch
            colMap.objPlace(0x33, 1, L"Supported Get");
            colMap.objPlace(0x33, 2, L"Supported Report");
            colMap.objPlace(0x33, 3, L"Get");
            colMap.objPlace(0x33, 4, L"Report");
            colMap.objPlace(0x33, 5, L"Set");
            colMap.objPlace(0x33, 6, L"Start Change");
            colMap.objPlace(0x33, 7, L"Stop Change");

            // Multi-channel
            colMap.objPlace(0x60, 4, L"Inst. Get");
            colMap.objPlace(0x60, 5, L"Inst. Report");
            colMap.objPlace(0x60, 6, L"Inst. Encap");
            colMap.objPlace(0x60, 7, L"MCh. Get");
            colMap.objPlace(0x60, 8, L"MCh. EP Report");
            colMap.objPlace(0x60, 9, L"MCh. Cap Get");
            colMap.objPlace(0x60, 0xA, L"MCh. Cap Report");
            colMap.objPlace(0x60, 0xB, L"MCh. EP Find");
            colMap.objPlace(0x60, 0xC, L"MCh. EP Find Report");
            colMap.objPlace(0x60, 0xD, L"MCh Encap");

            // Doorlock
            colMap.objPlace(0x62, 1, L"Set");
            colMap.objPlace(0x62, 2, L"Get");
            colMap.objPlace(0x62, 3, L"Report");
            colMap.objPlace(0x62, 4, L"Config Set");
            colMap.objPlace(0x62, 5, L"Config Get");
            colMap.objPlace(0x62, 6, L"Config Report");

            // Entry control
            colMap.objPlace(0x6F, 1, L"Notification");
            colMap.objPlace(0x6F, 2, L"Ctrl Key Sup. Get");
            colMap.objPlace(0x6F, 3, L"Ctrl Key Sup. Report");
            colMap.objPlace(0x6F, 4, L"Event Sup. Get");
            colMap.objPlace(0x6F, 5, L"Event Sup. Report");
            colMap.objPlace(0x6F, 6, L"Config Set");
            colMap.objPlace(0x6F, 7, L"Config Get");
            colMap.objPlace(0x6F, 8, L"Config Report");

            // Config
            colMap.objPlace(0x70, 4, L"Set");
            colMap.objPlace(0x70, 5, L"Get");
            colMap.objPlace(0x70, 6, L"Report");

            // Man specific
            colMap.objPlace(0x72, 4, L"Get");
            colMap.objPlace(0x72, 5, L"Report");

            // Lock
            colMap.objPlace(0x76, 1, L"Set");
            colMap.objPlace(0x76, 2, L"Get");
            colMap.objPlace(0x76, 3, L"Report");

            // Node Naming
            colMap.objPlace(0x77, 1, L"Set");
            colMap.objPlace(0x77, 2, L"Get");
            colMap.objPlace(0x77, 3, L"Report");
            colMap.objPlace(0x77, 4, L"Loc Set");
            colMap.objPlace(0x77, 5, L"Loc Get");
            colMap.objPlace(0x77, 6, L"Loc Report");

            // Battery
            colMap.objPlace(0x80, 2, L"Get");
            colMap.objPlace(0x80, 3, L"Report");

            // Wakeup
            colMap.objPlace(0x84, 4, L"Interval Set");
            colMap.objPlace(0x84, 5, L"Interval Get");
            colMap.objPlace(0x84, 6, L"Interval Report");
            colMap.objPlace(0x84, 7, L"WUP Notification");
            colMap.objPlace(0x84, 8, L"No more WUP Info");

            // Association
            colMap.objPlace(0x85, 1, L"Set");
            colMap.objPlace(0x85, 2, L"Get");
            colMap.objPlace(0x85, 3, L"Report");
            colMap.objPlace(0x85, 4, L"Remove");
            colMap.objPlace(0x85, 5, L"Groupings Get");
            colMap.objPlace(0x85, 6, L"Groupings Report");

            // Version
            colMap.objPlace(0x86, 0x13, L"Get");
            colMap.objPlace(0x86, 0x14, L"Report");

            // Security
            colMap.objPlace(0x98, 2, L"Get Sup. Cmds");
            colMap.objPlace(0x98, 3, L"Sup Cmds Report");
            colMap.objPlace(0x98, 4, L"Scheme Get");
            colMap.objPlace(0x98, 5, L"Scheme Report");
            colMap.objPlace(0x98, 6, L"Key Set");
            colMap.objPlace(0x98, 7, L"Key Verify");
            colMap.objPlace(0x98, 8, L"Scheme Inherit");
            colMap.objPlace(0x98, 0x40, L"Nonce Get");
            colMap.objPlace(0x98, 0x80, L"Nonce Report");
            colMap.objPlace(0x98, 0x81, L"Msg Encap");
            colMap.objPlace(0x98, 0xC1, L"Msg Encap/Nonce");
        }

        // Mark us loaded now
        ZWaveUSB3Sh_ThisFacility::bCCCmdIdMapLoaded = kCIDLib::True;
    }

    // Create the unique key
    tCIDLib::TCard4 c4IdKey((tCIDLib::TCard4(c1ClassId) << 16) | c1CmdId);

    // Do a binary lookup
    tCIDLib::TCard4 c4At;
    ZWaveUSB3Sh_ThisFacility::TCCmdIdItem* pitemFind
    (
        ZWaveUSB3Sh_ThisFacility::colCCmdIdMap.pobjKeyedBinarySearch
        (
            c4IdKey, &eKeyCompCCmdIdItem, c4At
        )
    );

    // If not found, return a null ptr
    if (!pitemFind)
        return nullptr;

    // Else we can return the mapped cmd name
    return pitemFind->m_pszText;
}


//
//  We make this easy by remembering our current trace level, setting it to off,
//  then setting it back.
//
tCIDLib::TVoid TFacZWaveUSB3Sh::ResetMsgTrace()
{
    const tCQCKit::EVerboseLvls eLevel = m_eTraceLevel;
    SetTraceLevel(tCQCKit::EVerboseLvls::Off);
    SetTraceLevel(eLevel);
}


TString TFacZWaveUSB3Sh::strFindDevName(const tCIDLib::TCard8 c8ManIds) const
{
    // Lock for this
    TLocker lockrSync(&m_mtxSync);

    // See if we have this particular set of ids in our map
    const tCIDLib::TCard4 c4Index = c4FindMapIndex(c8ManIds);
    if (c4Index != kCIDLib::c4MaxCard)
    {
        const ZWaveUSB3Sh_ThisFacility::TDevMapItem& curItem
                                        = ZWaveUSB3Sh_ThisFacility::colDevInfoMap[c4Index];

        TString strRet = curItem.m_strMake;
        strRet.Append(kCIDLib::chSpace);
        strRet.Append(curItem.m_strModel);
        return strRet;
    }
    return ZWaveUSB3Sh_ThisFacility::strUnknown;
}


//
//  Sometimes device info may be updated. We don't want to make the user cycle the
//  driver to get the new info, but if we have already read it it will be in the
//  cache and never read again. So this allows ut to be told to reload it.
//
//  Actually what it does is see if it is in the cache and, if so, just remove it
//  from the cache so the next access will cause it to reload.
//
tCIDLib::TVoid TFacZWaveUSB3Sh::ReloadDevInfo(const tCIDLib::TCard8 c8ManIds)
{
    // Lock for this
    TLocker lockrSync(&m_mtxSync);

    // Let's see if we have this guy in our cache
    tCIDLib::TCard4 c4At = c4FindCacheIndex(c8ManIds);
    if (c4At != kCIDLib::c4MaxCard)
        ZWaveUSB3Sh_ThisFacility::colDevInfoCache.RemoveAt(c4At);
}


//
//  This forces us to reload the device info map. Normally this is loaded once and is
//  just kept around. But, in some case, they may need to force it to reload, perhaps
//  changes had to be made that affected the info in the map.
//
//  What we do is jsut clear the loaded flag. So the next access will cause it to be
//  reloaded.
//
tCIDLib::TVoid TFacZWaveUSB3Sh::ReloadDevInfoMap() const
{
    ZWaveUSB3Sh_ThisFacility::bDevMapLoaded = kCIDLib::False;
}


//
//  Sends the passed manufacture ids to the indicated target node id. We send this at
//  special command priority to insure it gets out quickly, since it's important to get
//  this to the master during startup and replication.
//
//  We assume this is not sent to non-listeners, so it doesn't go through any unit
//  object, it's just a lower level send.
//
tCIDLib::TVoid
TFacZWaveUSB3Sh::SendManIds(        MZWaveCtrlIntf&     mzwciIO
                            , const tCIDLib::TCard1     c1TarId
                            , const tCIDLib::TCard8     c8ToSend
                            , const tCIDLib::TBoolean   bSecure)
{
    TZWOutMsg zwomOut;
    zwomOut.Reset
    (
        c1TarId
        , COMMAND_CLASS_MANUFACTURER_SPECIFIC
        , MANUFACTURER_SPECIFIC_REPORT
        , 0
        , 8
        , tZWaveUSB3Sh::EMsgPrios::SpecialCmd
    );
    zwomOut += tCIDLib::TCard1(c8ToSend >> 40);
    zwomOut += tCIDLib::TCard1(c8ToSend >> 32);
    zwomOut += tCIDLib::TCard1(c8ToSend >> 24);
    zwomOut += tCIDLib::TCard1(c8ToSend >> 16);
    zwomOut += tCIDLib::TCard1(c8ToSend >> 8);
    zwomOut += tCIDLib::TCard1(c8ToSend);
    zwomOut.AppendTransOpts();

    // If it's the broadcast address, don't ask for callbacks
    if (c1TarId == NODE_BROADCAST)
        zwomOut.AppendNullCallback();
    else
        zwomOut.AppendCallback();
    zwomOut.Finalize(kCIDLib::False, bSecure);
    mzwciIO.USB3SendCCMsg(zwomOut);
}


//
//  The driver tells us the controller id as soon as it's figured out, which is early
//  in the startup process if we are in the network or after a successful inclusion gets
//  us a new id.
//
tCIDLib::TVoid TFacZWaveUSB3Sh::SetCtrlId(const tCIDLib::TCard1 c1ToSet)
{
    m_c1CtrlId = c1ToSet;
}


//
//  When we start an inclusion, we force the key back to the default if it is not already.
//  If we were already in the network we'll have the real key, and that won't work for the
//  bootstrapping process that occurs during inclusion. We'll get the key back again if
//  the inclusion works. If not, the driver will set the one we had before back./
//
tCIDLib::TVoid TFacZWaveUSB3Sh::SetDefNetworkKey()
{
    THeapBuf mbufKey(16, 16);
    mbufKey.Set(0);
    SetNetworkKey(mbufKey);
}


//
//  This is called during driver startup if we are in the network where were have
//  the actual network key, or if we were just added to the network and get a
//  new key. It can be called with a default key to reset us if we are removed from
//  the network.
//
//  If called during startup, there are no sync issues. Otherwise it only gets called
//  by the Z-Stick object's I/O thread, so still no sync issues.
//
tCIDLib::TVoid TFacZWaveUSB3Sh::SetNetworkKey(const TMemBuf& mbufToSet)
{
    CIDAssert(mbufToSet.c4Size() == 16, L"The network key must be 16 bytes");

    // Set the new network key and then re-gen our own keys
    {
        TLocker lockrSync(&m_mtxSync);
        m_ckeyNetwork.Set(mbufToSet);
        SecuritySetup();
    }

    if (bLowTrace())
    {
        TString strMsg(L"New network key set: ", 64UL);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < 16; c4Index++)
        {
            const tCIDLib::TCard1 c1Cur = mbufToSet[c4Index];
            if (c1Cur < 0x10)
                strMsg.Append(kCIDLib::chDigit0);
            strMsg.AppendFormatted(c1Cur, tCIDLib::ERadices::Hex);
            strMsg.Append(kCIDLib::chSpace);
        }
        LogTraceMsg(strMsg);
    }
}


// Sets the verbosity level for a trace log logging
tCIDLib::TVoid TFacZWaveUSB3Sh::SetTraceLevel(const tCQCKit::EVerboseLvls eToSet)
{
    // If the new level is off and we have a trace stream, then kill it
    if (m_pstrmTrace && (eToSet == tCQCKit::EVerboseLvls::Off))
    {
        // Make sure we don't pull it out from under someone's feet
        TLocker lockrSync(&m_mtxSync);
        m_pstrmTrace->Flush();
        m_pstrmTrace->Close();
        delete m_pstrmTrace;
        m_pstrmTrace = nullptr;
    }

    // Store the new level
    m_eTraceLevel = eToSet;

    // And now, if not turning off and not already started, let's start it up
    if ((eToSet != tCQCKit::EVerboseLvls::Off) && !m_pstrmTrace)
    {
        try
        {
            //
            //  Build up the path. We'll put it in the same path that the server
            //  driver puts his config.
            //
            TPathStr pathCfg = facCQCKit().strServerDataDir();
            pathCfg.AddLevel(L"ZWaveUSB3S");
            pathCfg.AddLevel(L"Trace.Txt");

            TTextFileOutStream* pstrmNew = new TTextFileOutStream;
            pstrmNew->Open
            (
                pathCfg
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SafeStream
                , tCIDLib::EAccessModes::Write
            );
            TJanitor<TTextFileOutStream> janStream(pstrmNew);

            // Put out some initial info
            TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);

            *(pstrmNew) << L"Z-Wave USB3 Trace\n"
                        << L",  Time: " << tmCur
                        << L"\n----------------------------------------------------"
                        << kCIDLib::DNewLn;

            // Now store the pointer to make it available, no need to sync for this
            m_pstrmTrace = janStream.pobjOrphan();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            facZWaveUSB3Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Failed to start Z-Wave trace"
                , tCIDLib::ESeverities::Status
            );
        }

        m_enctLastMsg = 0;
    }
}


// The driver tells us when its logging verbosity level changes
tCIDLib::TVoid TFacZWaveUSB3Sh::SetVerboseLevel(const tCQCKit::EVerboseLvls eToSet)
{
    m_eVerboseLevel = eToSet;
}


// ---------------------------------------------------------------------------
//  TFacZWaveUSB3Sh: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We have two lookup methods here. One finds an encoded set of ids in our cache of
//  already loaded device info files. THe other finds the same in our in-memory map.
//  Both are sorted by the man ids so we can binary search them.
//
//  WE ASSUME that the caller has locked and will remain so until he has finished
//  using the returned index.
//
tCIDLib::TCard4
TFacZWaveUSB3Sh::c4FindCacheIndex(const  tCIDLib::TCard8 c8FindId) const
{
    tCIDLib::TInt4 i4End = tCIDLib::TInt4
    (
        ZWaveUSB3Sh_ThisFacility::colDevInfoCache.c4ElemCount() - 1
    );
    tCIDLib::TInt4 i4Begin = 0;

    tCIDLib::TCard4 c4Index = kCIDLib::c4MaxCard;
    while (i4Begin <= i4End)
    {
        // Divide the current range
        tCIDLib::TInt4 i4MidPoint = (i4Begin + i4End) / 2;

        // Check this guy, and return it if this is the one
        const tCIDLib::TCard8 c8CurId
                = ZWaveUSB3Sh_ThisFacility::colDevInfoCache[i4MidPoint].c8ManIds();
        if (c8CurId == c8FindId)
        {
            c4Index = tCIDLib::TCard4(i4MidPoint);
            break;
        }

        // Didn't find it, so see which way to go and adjust begin/end
        if (c8FindId < c8CurId)
            i4End = i4MidPoint - 1;
        else
            i4Begin = i4MidPoint + 1;
    }

    return c4Index;
}

tCIDLib::TCard4
TFacZWaveUSB3Sh::c4FindMapIndex(const  tCIDLib::TCard8 c8FindId) const
{
    // They are sorted by id, so we do a simple binary search
    tCIDLib::TInt4 i4End = tCIDLib::TInt4
    (
        ZWaveUSB3Sh_ThisFacility::colDevInfoMap.c4ElemCount() - 1
    );
    tCIDLib::TInt4 i4Begin = 0;

    tCIDLib::TCard4 c4Index = kCIDLib::c4MaxCard;
    while (i4Begin <= i4End)
    {
        // Divide the current range
        tCIDLib::TInt4 i4MidPoint = (i4Begin + i4End) / 2;

        // Check this guy, and return it if this is the one
        const tCIDLib::TCard8 c8CurId = ZWaveUSB3Sh_ThisFacility::colDevInfoMap[i4MidPoint].m_c8ManIds;
        if (c8CurId == c8FindId)
        {
            c4Index = tCIDLib::TCard4(i4MidPoint);
            break;
        }

        // Didn't find it, so see which way to go and adjust begin/end
        if (c8FindId < c8CurId)
            i4End = i4MidPoint - 1;
        else
            i4Begin = i4MidPoint + 1;
    }

    return c4Index;
}


// We translate some common function ids to make reading the log easier
tCIDLib::TVoid TFacZWaveUSB3Sh::LogMsgId(const tCIDLib::TCard1 c1MsgId)
{
    switch(c1MsgId)
    {
        case FUNC_ID_APPLICATION_COMMAND_HANDLER :
            *(m_pstrmTrace) << L"AppCmd";
            break;

        case FUNC_ID_SERIAL_API_GET_INIT_DATA :
            *(m_pstrmTrace) << L"GetInitData";
            break;

        case FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION :
            *(m_pstrmTrace) << L"NodeInfo";
            break;

        case FUNC_ID_SERIAL_API_SET_TIMEOUTS :
            *(m_pstrmTrace) << L"SetTimeouts";
            break;

        case FUNC_ID_ZW_SEND_DATA :
            *(m_pstrmTrace) << L"SendData";
            break;

        case FUNC_ID_ZW_SEND_DATA_MULTI :
            *(m_pstrmTrace) << L"SendMData";
            break;

        case FUNC_ID_ZW_GET_VERSION :
            *(m_pstrmTrace) << L"GetVersion";
            break;

        case FUNC_ID_ZW_REQUEST_NODE_INFO :
            *(m_pstrmTrace) << L"ReqNodeInfo";
            break;

        case FUNC_ID_SERIAL_API_GET_CAPABILITIES :
            *(m_pstrmTrace) << L"GetCaps";
            break;

        case FUNC_ID_MEMORY_GET_ID :
            *(m_pstrmTrace) << L"MemGetId";
            break;

        case FUNC_ID_ZW_SET_LEARN_MODE :
            *(m_pstrmTrace) << L"SetLearnMode";
            break;

        case FUNC_ID_ZW_APPLICATION_UPDATE :
            *(m_pstrmTrace) << L"AppUpdate";
            break;

        case FUNC_ID_ZW_REPLICATION_SEND_DATA :
            *(m_pstrmTrace) << L"RepSendData";
            break;

        case FUNC_ID_ZW_REPLICATION_COMMAND_COMPLETE :
            *(m_pstrmTrace) << L"RepCmdComplete";
            break;

        case FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO :
            *(m_pstrmTrace) << L"GetNodeProtoInfo";
            break;

        case FUNC_ID_ZW_SEND_NODE_INFORMATION :
            *(m_pstrmTrace) << L"SendNodeInfo";
            break;

        default :
            *(m_pstrmTrace) << L"???";
            break;
    };

    *(m_pstrmTrace) << L"/0x" << TCardinal(c1MsgId, tCIDLib::ERadices::Hex);
}


//
//  WE have one encrypter for encryption and one for authentication. We have to
//  set them up based on the current network key. That changes depending on whether
//  we are currently in the network (and have obtained a real key) or still need to
//  join the network and therefore have to use a temp key.
//
//  See SetNetworkKey() above for more info.
//
tCIDLib::TVoid TFacZWaveUSB3Sh::SecuritySetup()
{
    static const tCIDLib::TCard1 ac1EncryptPW[16] =
    {
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA
      , 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA
    };
    static const tCIDLib::TCard1 ac1AuthPW[16] =
    {
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55
      , 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55
    };


    // Lock for the rest of it so the key can't change
    TLocker lockrSync(&m_mtxSync);

    // Set up both encrypters with the current network key
    m_crypAuth.SetNewKey(m_ckeyNetwork);
    m_crypEncrypt.SetNewKey(m_ckeyNetwork);

    // Set both to ECB mode
    m_crypAuth.eMode(tCIDCrypto::EBlockModes::ECB);
    m_crypEncrypt.eMode(tCIDCrypto::EBlockModes::ECB);

    // Now use this set up to encrypt our two passwords above
    THeapBuf mbufAuth(16, 16);
    THeapBuf mbufEncrypt(16, 16);

    m_crypAuth.c4Encrypt(ac1AuthPW, mbufAuth, 16);
    m_crypEncrypt.c4Encrypt(ac1EncryptPW, mbufEncrypt, 16);

    // Now create keys out of these and set them on their respective encrypters
    TCryptoKey ckeyAuth(mbufAuth, 16);
    TCryptoKey ckeyEncrypt(mbufEncrypt, 16);

    m_crypAuth.SetNewKey(ckeyAuth);
    m_crypEncrypt.SetNewKey(ckeyEncrypt);

    // Authentication uses ECB mode, and encrypt/decrypt uses OFB
    m_crypAuth.eMode(tCIDCrypto::EBlockModes::ECB);
    m_crypEncrypt.eMode(tCIDCrypto::EBlockModes::OFB);
}

