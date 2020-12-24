//
// FILE NAME: CQCKit_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/06/2000
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
//  This file defines facility class for this facility. This facility is a bit of a
//  grab bag, because it's the most fundamental facility in CQC, and so it provides
//  stuff that is almost universally needed by CQC applications. All non-GUI stuff
//  of course.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCfgServerClient;
class TPNGImage;


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCKit
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCKITEXPORT TFacCQCKit : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TEventQ = TRefQueue<TCQCEvent>;
        using TEvDupLIFO =  TBag<TMD5Hash>;
        using TEvDupKeyed =  THashSet<TMD5Hash,TMD5KeyOps>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCKit();

        TFacCQCKit(const TFacCQCKit&) = delete;
        TFacCQCKit(TFacCQCKit&&) = delete;

        ~TFacCQCKit() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCKit& operator=(const TFacCQCKit&) = delete;
        TFacCQCKit& operator=(TFacCQCKit&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckBoolVal
        (
            const   TString&                strToTest
        )   const;

        tCIDLib::TBoolean bCheckBoolVal
        (
            const   TString&                strToTest
            ,       tCIDLib::TBoolean&      bRes
        )   const;

        tCIDLib::TBoolean bCheckFldExists
        (
            const   TString&                strMoniker
            , const TString&                strField
        )   const;

        tCIDLib::TBoolean bCheckFldExists
        (
            const   TString&                strField
        )   const;

        tCIDLib::TBoolean bCheckFldAccess
        (
            const   tCQCKit::EReqAccess     eRequested
            , const tCQCKit::EFldAccess     eFldAccess
        );

        tCIDLib::TBoolean bCheckNumRangeFld
        (
            const   TCQCFldDef&             flddCheck
            , const tCIDLib::TFloat8        f8MinRange
            , const tCIDLib::TFloat8        f8MaxRange
            , const tCIDLib::TBoolean       bAllowFloat
            ,       tCIDLib::TFloat8&       f8RealMin
            ,       tCIDLib::TFloat8&       f8RealMax
        );

        tCIDLib::TBoolean bCheckNumRangeFld
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const TString&                strLimits
            , const tCIDLib::TFloat8        f8MinRange
            , const tCIDLib::TFloat8        f8MaxRange
            , const tCIDLib::TBoolean       bAllowFloat
            ,       tCIDLib::TFloat8&       f8RealMin
            ,       tCIDLib::TFloat8&       f8RealMax
        );

        tCIDLib::TBoolean bCheckYesNoVal
        (
            const   TString&                strToTest
        )   const;

        tCIDLib::TBoolean bDevMode() const;

        tCIDLib::TBoolean bDoCmdPrep
        (
                    TCQCCmdCfg&             ccfgSrc
            , const TCQCCmdRTVSrc&          crtsVals
            , const TStdVarsTar* const      pctarGVars
            , const TStdVarsTar* const      pctarLVars
            ,       TString&                strTmp
        );

        tCIDLib::TBoolean bDoEnvLogin
        (
                    TCQCSecToken&           sectToFill
            ,       TCQCUserAccount&        uaccToFill
            ,       tCQCKit::ELoginRes&     eRes
        );

        tCIDLib::TBoolean bDoLogin
        (
            const  TString&                 strUserName
            , const TString&                strPassword
            ,       TCQCSecToken&           sectToFill
            ,       TCQCUserAccount&        uaccToFill
            ,       tCQCKit::ELoginRes&     eRes
        );

        tCIDLib::TBoolean bDoSearchNReplace
        (
            const   TString&                strFind
            , const TString&                strRepStr
            ,       TString&                strTarget
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        tCIDLib::TBoolean bDrvIsConfigured
        (
            const   TString&                strMoniker
            ,       TString&                strHost
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bDrvIsLoaded
        (
            const   TString&                strMoniker
        )   const;

        tCIDLib::TBoolean bDrvIsLoaded
        (
            const   TString&                strMoniker
            ,       TString&                strHost
        )   const;

        tCIDLib::TBoolean bExpandResPath
        (
            const   TString&                strBasePath
            , const TString&                strNewPath
            ,       TString&                strExpPath
        )   const;

        tCIDLib::TBoolean bFindAllDrivers
        (
                    tCIDLib::TStrCollect&   colMonikers
        )   const;

        tCIDLib::TBoolean bFindAllDrivers
        (
            const   TString&                strOnHost
            ,       tCIDLib::TStrCollect&   colMonikers
        )   const;

        tCIDLib::TBoolean bFindAllDrivers
        (
                    tCIDLib::TStrCollect&   colMonikers
            ,       tCIDLib::TStrCollect&   colMake
            ,       tCIDLib::TStrCollect&   colModel
            ,       tCQCKit::TDevCatList&   fcolCats
        )   const;

        tCIDLib::TBoolean bFindDrivers
        (
            const   TString&                strMonikerFilter
            , const TString&                strMakeFilter
            , const TString&                strModelFilter
            , const tCQCKit::TDevCatSet&    btsDevCats
            ,       tCQCKit::TDrvObjCfgCol& colToFill
        )   const;

        tCIDLib::TBoolean bFindDrivers
        (
            const   TString&                strMonikerFilter
            , const TString&                strMakeFilter
            , const TString&                strModelFilter
            , const tCQCKit::TDevCatSet&    btsDevCats
            ,       tCIDLib::TStrCollect&   colMonikers
        )   const;

        tCIDLib::TBoolean bFindDrivers
        (
            const   TString&                strMonikerFilter
            , const TString&                strMakeFilter
            , const TString&                strModelFilter
            , const tCQCKit::TDevCatSet&    btsDevCats
            ,       tCIDLib::TStrCollect&   colMonikers
            ,       tCIDLib::TStrCollect&   colMakes
            ,       tCIDLib::TStrCollect&   colModels
        )   const;

        tCIDLib::TBoolean bFindRepoDrivers
        (
                    tCIDLib::TStrCollect&   colToFill
            , const TString&                strModelFilter
        );

        tCIDLib::TBoolean bFindHelpWS
        (
                    TString&                strHost
            ,       tCIDLib::TIPPortNum&    ippnWS
            ,       tCIDLib::TBoolean&      bSecure
        );

        tCIDLib::TBoolean bGetLocationInfo
        (
            const   tCIDLib::ECSSides       eSide
            ,       tCIDLib::TFloat8&       f8Lat
            ,       tCIDLib::TFloat8&       f8Long
            , const tCIDLib::TBoolean       bThrowIfNot
            , const TCQCSecToken&           sectToUse
        );

        tCIDLib::TBoolean bIsValidBaseFldName
        (
            const   TString&                strToCheck
        );

        tCIDLib::TBoolean bIsValidFldName
        (
            const   TString&                strToCheck
            , const tCIDLib::TBoolean       bInternal
        )   const;

        tCIDLib::TBoolean bIsValidFldName
        (
            const   TString&                strToCheck
            , const tCIDLib::TBoolean       bInternal
            ,       tCQCKit::EDevClasses&   eDevClass
        )   const;

        tCIDLib::TBoolean bIsValidFldNameChar
        (
            const   tCIDLib::TCh            chNew
            ,       tCIDLib::TCard4         c4Index
        )   const;

        tCIDLib::TBoolean bIsValidMoniker
        (
            const   TString&                strToCheck
        );

        tCIDLib::TBoolean bIsValidRemTypeRelPath
        (
            const   TString&                strToCheck
            ,       TString&                strErrMsg
            , const tCIDLib::TBoolean       bRelativeOK
        )   const;

        tCIDLib::TBoolean bLoadEnvInfo
        (
                    TString&                strFailReason
            , const tCIDLib::TBoolean       bIsClientApp
        );

        tCIDLib::TBoolean bMakeRelResPath
        (
            const   TString&                strBasePath
            , const TString&                strTarPath
            ,       TString&                strNewPath
        )   const;

        tCIDLib::TBoolean bParseFldName
        (
            const   TString&                strToParse
            ,       TString&                strMoniker
            ,       TString&                strFldName
        )   const;

        tCIDLib::TBoolean bParseFldName
        (
            const   TString&                strToParse
            ,       TString&                strMoniker
            ,       tCQCKit::EDevClasses&   eDevClass
            ,       TString&                strFldName
            , const tCIDLib::TBoolean       bNoMon = kCIDLib::False
        )   const;

        tCIDLib::TBoolean bParseFldName
        (
            const   TString&                strToParse
            ,       TString&                strMoniker
            ,       tCQCKit::EDevClasses&   eDevClass
            ,       TString&                strSubUnit
            ,       TString&                strSuffix
            , const tCIDLib::TBoolean       bNoMon = kCIDLib::False
        )   const;

        tCIDLib::TBoolean bParseFldName
        (
            const   TString&                strToParse
            ,       tCQCKit::EDevClasses&   eDevClass
            ,       TString&                strSubUnit
            ,       TString&                strSuffix
        )   const;

        tCIDLib::TBoolean bQueryFldDef
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       TCQCFldDef&             flddToFill
            , const tCIDLib::TBoolean       bQuickTest = kCIDLib::False
        )   noexcept;

        tCIDLib::TBoolean bReplaceTokens
        (
            const   tCIDLib::TKVPCollect&   colList
            ,       TString&                strToUpdate
        );

        tCIDLib::TBoolean bSysEdit() const;

        tCIDLib::TCard8 c8MakeVersion
        (
            const   tCIDLib::TCard4         c4Major
            , const tCIDLib::TCard4         c4Minor
            , const tCIDLib::TCard4         c4Revision
        );

        tCQCKit::ECmdPrepRes eStdTokenReplace
        (
            const   TString&                strPattern
            , const TCQCCmdRTVSrc* const    pcrtsToUse
            , const TStdVarsTar* const      pctarGVars
            , const TStdVarsTar* const      pctarLVars
            ,       TString&                strResult
            , const tCQCKit::ETokRepFlags   eFlags
        );

        tCIDLib::TVoid ExecApp
        (
            const   TString&                strApp
            , const TString&                strInitPath
            , const TString&                strParams
            , const tCIDLib::EExtProcShows  eShow
            , const TString&                strUserName
        );

        tCIDLib::TVoid FormatVersionStr
        (
                    TString&                strToFill
            , const tCIDLib::TCard8         c8Ver
        );

        tCIDLib::TVoid FreeApps();

        tCIDLib::TIPPortNum ippnEvents() const;

        tCIDLib::TIPPortNum ippnEvents
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TVoid Make50Path
        (
                    TString&                strToConvert
        );

        tCIDLib::TVoid GetNextEventId
        (
                    TMD5Hash&               mhashToFill
        );

        tCQCKit::TInstSrvProxy orbcInstSrvProxy
        (
            const   tCIDLib::TCard4         c4WaitUpTo = 0
            , const tCIDLib::TBoolean       bQuickTest = kCIDLib::False
        )   const;

        tCQCKit::TCQCSrvProxy orbcCQCSrvAdminProxy
        (
            const   TString&                strDriverMoniker
            , const tCIDLib::TCard4         c4WaitUpTo = 0
            , const tCIDLib::TBoolean       bQuickTest = kCIDLib::False
        )   const;

        tCQCKit::TCQCSrvProxy orbcCQCSrvAdminTestProxy
        (
            const   TString&                strHostName
            , const TString&                strProgName
            , const tCIDLib::TBoolean       bQuickTest = kCIDLib::False
        )   const;

        tCQCKit::TSecuritySrvProxy orbcSecuritySrvProxy
        (
            const   tCIDLib::TCard4         c4WaitUpTo = 0
            , const tCIDLib::TBoolean       bQuickTest = kCIDLib::False
        )   const;

        [[nodiscard]] TCQCFldValue* pfvMakeNewFor
        (
            const   tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FldId
            , const tCQCKit::EFldTypes      eType
        );

        [[nodiscard]] TCQCSrvAdminClientProxy* porbcCQCSrvAdminProxy
        (
            const   TString&                strHostName
            , const tCIDLib::TCard4         c4WaitUpTo = 0
            , const tCIDLib::TBoolean       bQuickTest = kCIDLib::False
        )   const;

        tCIDLib::TVoid PackageImg
        (
            const  TString&                 strSrcFile
            , const TPoint&                 pntTransAt
            , const tCIDLib::TBoolean       bTrans
            ,       TBinOutStream&          strmImg
            ,       TBinOutStream&          strmThumb
            ,       tCIDImage::EPixFmts&    eFmt
            ,       tCIDImage::EBitDepths&  eDepth
            ,       TSize&                  szImg
            ,       TSize&                  szThumb
        );

        tCIDLib::TVoid PackageImg
        (
            const   TPNGImage&              imgToPack
            ,       TBinOutStream&          strmImg
            ,       TBinOutStream&          strmThumb
            ,       TSize&                  szThumb
        );

        tCIDLib::TVoid ParseFldName
        (
            const   TString&                strToParse
            ,       TString&                strMoniker
            ,       TString&                strFldName
        )   const;

        tCIDLib::TVoid PrepRemBrwsPath
        (
            const   TString&                strSrc
            ,       TString&                strPrepped
        );

        tCIDLib::TVoid PrepRemBrwsPath
        (
                    TString&                strToPrep
        );

        tCIDLib::TVoid QueryAppAttachInfo
        (
                    TString&                strToFill
        );

        tCIDLib::TVoid QueueStdEventTrig
        (
            const   tCQCKit::EStdDrvEvs     eEvent
            , const TString&                strMoniker
            , const TString&                strFld
            , const TString&                strVal1
            , const TString&                strVal2
            , const TString&                strVal3
            , const TString&                strVal4
        );

        tCIDLib::TVoid QueryLiveDrvCfg
        (
            const   TString&                strDriverMoniker
            ,       TCQCDriverObjCfg&       cqcdcToFill
        )   const;

        const TString& strBoolEnterExit
        (
            const   tCIDLib::TBoolean       bValue
        )   const;

        const TString& strBoolLockedUnlocked
        (
            const   tCIDLib::TBoolean       bValue
        )   const;

        const TString& strBoolOffOn
        (
            const   tCIDLib::TBoolean       bValue
        )   const;

        const TString& strBoolStartEnd
        (
            const   tCIDLib::TBoolean       bValue
        )   const;

        const TString& strBoolVal
        (
            const   tCIDLib::TBoolean       bValue
        )   const;

        const TString& strBoolYesNo
        (
            const   tCIDLib::TBoolean       bValue
        )   const;

        const TString& strBinDir() const;

        const TString& strClientDataDir() const
        {
            return m_strClientDataDir;
        }

        const TString& strClientDir() const
        {
            return m_strClientDir;
        }

        const TString& strDataDir() const
        {
            return m_strDataDir;
        }

        const TString& strDevClassName
        (
            const   tCQCKit::EDevClasses    eClass
        )   const;

        const TString& strGenDriverLibName() const
        {
            return m_strGenDriverLibName;
        }

        const TString& strLocalDataDir() const
        {
            return m_strLocalDataDir;
        }

        const TString& strLocalLogDir() const
        {
            return m_strLocalLogDir;
        }

        const TString& strMacroRootClassScope() const
        {
            return m_strMacroRootClassScope;
        }

        const TString& strMacroRootPath() const
        {
            return m_strMacroRootPath;
        }

        TString strMakeFldName
        (
            const   TString&                strBase
            , const tCQCKit::EDevClasses    eClass
            , const tCIDLib::TBoolean       bV2Type
        );

        const TString& strOrgDataDir() const
        {
            return m_strOrgDataDir;
        }

        const TString& strProtoDriverLibName() const
        {
            return m_strProtoDriverLibName;
        }

        const TString& strRepositoryDir() const
        {
            return m_strRepositoryDir;
        }

        const TString& strServerDataDir() const
        {
            return m_strServerDataDir;
        }

        const TString& strServerDir() const
        {
            return m_strServerDir;
        }

        const TString& strUsersDir() const
        {
            return m_strUsersDir;
        }

        tCIDLib::TVoid SendCQCSrvCfgChange
        (
            const   TString&                strCfgType
            ,       tCIDLib::TBoolean&      bFailures
        );

        tCIDLib::TVoid SendEMail
        (
            const   TString&                strAccount
            , const TString&                strAddress
            , const TString&                strSubject
            , const TString&                strMessage
            , const tCIDLib::TBoolean       bIsHTML
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid SendEMail
        (
            const   TString&                strAccount
            , const TString&                strAddress
            , const TString&                strSubject
            ,       THeapBuf&&              mbufAttach
            , const tCIDLib::TCard4         c4Bytes
            , const TString&                strContType
            , const TString&                strFileName
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid SetAppAttachInfo
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetClientLocInfo
        (
            const   tCIDLib::TFloat8&       f8Lat
            , const tCIDLib::TFloat8&       f8Long
        );

        tCIDLib::TVoid StartEventProcessing
        (
            const   tCQCKit::EEvProcTypes   eType
            , const TCQCSecToken&           sectToUse
        );

        tCIDLib::TVoid StopEventProcessing();

        tCIDLib::TVoid TestPrivileges
        (
            const   TCQCSecToken&           sectUser
            , const tCQCKit::EUserRoles     eMinRole
            ,       tCQCKit::TSecuritySrvProxy& orbcSS
        );

        tCIDLib::TVoid TestPrivileges
        (
            const   TCQCSecToken&           sectUser
            , const tCQCKit::EUserRoles     eMinRole
        );

        tCIDLib::TVoid TestPrivileges
        (
            const   tCQCKit::EUserRoles     eTestRole
            , const tCQCKit::EUserRoles     eMinRole
        );

        tCIDLib::TVoid TestPrivileges
        (
            const   tCQCKit::EUserRoles     eTestRole
            , const tCQCKit::EUserRoles     eMinRole
            , const TString&                strUserName
        );

        tCIDLib::TVoid ValidateDrvCfg
        (
            const   tCIDLib::ECSSides       eSide
            , const TCQCDriverCfg&          cqcdcToCheck
            , const TString&                strLibName
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bReplacePathTokens
        (
                    TString&                strToUpdate
            , const TString&                strUserName
        );

        tCIDLib::TBoolean bELOHelper
        (
                    TCQCSecToken&           sectToFill
            ,       TCQCUserAccount&        uaccToFill
            ,       TString&                strID
            , const tCIDLib::TBoolean       bType
            ,       tCQCKit::ELoginRes&     eRes
        );

        tCIDLib::EExitCodes eEvReceiveThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::EExitCodes eEvSendThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCQCKit::ECmdPrepRes eStdTokenRepHelper
        (
            const   TString&                strPattern
            , const TCQCCmdRTVSrc* const    pcrtsToUse
            , const TStdVarsTar* const      pctarGVars
            , const TStdVarsTar* const      pctarLVars
            ,       TString&                strResult
            , const tCQCKit::ETokRepFlags   eFlags
            , const tCIDLib::TCard4         c4Depth
        );

        tCIDLib::TVoid FormatTokenVals
        (
                    TString&                strOut
            , const TString&                strRepVal
            , const TString&                strTPat
            , const tCIDLib::TCard4         c4TPatOfs
            , const TString&                strSep
        );

        tCIDLib::TVoid SaveFile
        (
            const   TString&                strFileName
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Size
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDevMode
        //      We check for the CQC_INDEVENV environment variable on startup and set this
        //      flag, so that we can do some things differently when running from the
        //      development environment. In particular the file access ORB object will
        //      adjust some paths to point them to their development environment locations,
        //      whereas in the real environment everything is under the CQCData directory.
        //
        //  m_bSecureMasterWS
        //      Indicates whether the master server web server supports secure connections
        //      or not. We'll use the secure connection if available.
        //
        //  m_bSysEdit
        //      This is set via an environmental variable, and it is used by various chunks
        //      of code to control whether it allows editing of system resources, or only
        //      user resources. This is for us internally since we have to be able to edit
        //      system resources in place.
        //
        //  m_c4EvSeq
        //      A sequence number that we run up each time someone calls our NextEvId()
        //      method. It goes up to 255 then wraps, and we put it into the low byte
        //      of the m_mhashEvId value to create a unique id for each event we send
        //      out (at least for long enough a time to handle duplicate packet rejection.
        //
        //  m_colEvList
        //  m_colEvLIFO
        //      We have to reject duplicate events, so we keep two lists, one is a bag
        //      that we can use like a deque to keep up with the list in time (i.e. we
        //      can throw out the oldest one when it's full). And one is a hash set so
        //      that we can quickly look for dups.
        //
        //      Note that these require no synchronization because they are used purely
        //      within the event receiving thread.
        //
        //  m_colEvSQ
        //      A by ref queue that is used to hold outgoing event triggers until they
        //      can be sent. This guy is thread safe, and since it is only written to
        //      by sending threads and read by the our transmission thread, there's no
        //      need for any explicit sync.
        //
        //  m_eEvProcType
        //      When the event processing engine is started up they tell us what types
        //      of event processing they want, either sending or receiving or both.
        //      This lets us discard events that are not going to be dealt with and not
        //      let their queues grow.
        //
        //  m_f8ClientLat
        //  m_f8ClientLong
        //      Applications can store local client side lat/long info for access by
        //      other code, such as actions. This can change over time for clients that
        //      sense their location dynically. We init these to known invalid values
        //      so we'll know if they have been set yet.
        //
        //  m_ippnEvents
        //      The port number that event listening and sending are done on. It's
        //      set to the default if not overridden.
        //
        //  m_ippnMasterWS
        //      We cache the HTTP port of the master server's web server when bFindHelpWS
        //      is called. After that we just return this cached version.
        //
        //  m_mhashEvId
        //      We have to uniquely identify events so that receivers can filter out
        //      dups (they are sent as UDP packets and dups can happen.) This is a
        //      unique MD5 hash that is generated for each app as it loads. The last
        //      two bytes are replaced by a sequence number that is run up as each event
        //      goes out. When that wraps around, we generate a new hash and start over
        //      again. That gets us a lot of uniqueness guarantee without constantly
        //      generating hashes. 64K variations is vastly larger than the duplicate
        //      rejection lists above.
        //
        //  m_mtxSync
        //      We have to do a little locking in here
        //
        //  m_pstopEvTrigs
        //      If incoming event trigger processing is enabled, then we publish this
        //      topic and start posting events to it. Anyone can subscribe to this
        //      topic and respond to events.
        //
        //  m_sciOutTrigDropped
        //      We maintain a stat for how many outgoing event triggers we had to drop
        //      because our queue was full.
        //
        //  m_strBinDir
        //      The directory where the binaries are installed.
        //
        //  m_strBoolXX
        //      There are some places where we want to display some standard formatted
        //      representations of boolean states. These avoid a lot of redundancy. We
        //      have methods to get the them.
        //
        //  m_strClientDir
        //      Relative to the m_strDataDir, this is for client side related files,
        //      and will have some subdirectories under it to hold the different types
        //      of files required.
        //
        //  m_strDataDir
        //      The base CQC data directory that the current process should use. We get
        //      the base data dir from the CQC_DATA environment var, or the /DataDir=
        //      command line param (which overrides the env var.) Then we build up some
        //      variants by appending things to that. For client programs, unless we are
        //      in development mode (as driven by an environment var CQC_INDEVENV), this
        //      is actually pointed into a per system id CQC directory in the common
        //      app data directory.
        //
        //  m_strGenDriverLibName
        //      The name of the generic driver server library, which we don't want to
        //      hard code and it's used a couple of places.
        //
        //  m_strLocalData
        //      Some files are just copied by the installer onto every machine and
        //      accessed locally. They are in here. This stuff is all read only, so
        //      clients and servers can access them.
        //
        //  m_strLocalLogPath
        //      The path into which local logs (which are used if programs cannot get to
        //      the log server, or the few little command line apps that don't use the
        //      log server.) This is under the global app data path.
        //
        //  m_strMacroRootPath
        //      The location where all files read/written by macros will be relative to.
        //
        //  m_strMacroRootClassScope
        //      The root class path of all classes that we let the user created or
        //      modify. It will be '/MEng/User' by default, but the /SysMacros parameter
        //      will be seen by the LoadEnvInfo() method above, which all the CQC apps
        //      will call, and will change it to '/MEng' to allow them to modify system
        //      macros.
        //
        //  m_strMasterWSHost
        //      We cache the master server's web server host name when the bFindHelpWS
        //      method is called. After that we just return the cached version.
        //
        //  m_strOrgDataDir
        //      We change the CQCData directory on the client, to point to the
        //      local ProgrmData directory. But in some cases we still need the
        //      orignal path, such as in the driver install wizard where we are
        //      really picking paths for server side drivers. So we store it here.
        //
        //  m_strProtoDriverLibName
        //      The name of the protocol driver server library, which we don't want to
        //      hard code and it's used a couple of places.
        //
        //  m_strRepositoryDir
        //      The CQCData\Repository path, where the main and any private server side
        //      repos are stored.
        //
        //  m_strServerDataDir
        //  m_strServerDir
        //      Relative to the m_strDataDir, this is for server side related files,
        //      and will have some subdirectories under it to hold the different types
        //      of files required.
        //
        //  m_strSharedDir
        //      Relative to m_strDataDir, this is for files shared by both client and
        //      server side programs, such as shared libraries.
        //
        //  m_strUsersDir
        //      Where client side per-user data is put. Not used by servers.
        //
        //  m_thrEvRecieve
        //  m_thrEvSend
        //      We need threads to manage event processing. One waits for events to
        //      arrive on a datagram socket and queues them up to be read by the
        //      containing application. The other sends out events as they are queued
        //      up by the containing application.
        //
        //      These use the m_colEvRQ and m_colEvWQ collections respectively to queue
        //      up their events
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bDevMode;
        tCIDLib::TBoolean       m_bSecureMasterWS;
        tCIDLib::TBoolean       m_bSysEdit;
        tCIDLib::TCard4         m_c4EvSeq;
        TEvDupKeyed             m_colEvList;
        TEvDupLIFO              m_colEvLIFO;
        TEventQ                 m_colEvSQ;
        tCQCKit::EEvProcTypes   m_eEvProcType;
        tCIDLib::TFloat8        m_f8ClientLat;
        tCIDLib::TFloat8        m_f8ClientLong;
        tCIDLib::TIPPortNum     m_ippnEvents;
        tCIDLib::TIPPortNum     m_ippnMasterWS;
        TMD5Hash                m_mhashEvId;
        TMutex                  m_mtxSync;
        TPubSubTopic            m_pstopEvTrigs;
        TString                 m_strBinDir;
        const TString           m_strBoolEnd;
        const TString           m_strBoolEnter;
        const TString           m_strBoolExit;
        const TString           m_strBoolFalse;
        const TString           m_strBoolLocked;
        const TString           m_strBoolNo;
        const TString           m_strBoolOff;
        const TString           m_strBoolOn;
        const TString           m_strBoolStart;
        const TString           m_strBoolTrue;
        const TString           m_strBoolUnlocked;
        const TString           m_strBoolYes;
        TString                 m_strClientDataDir;
        TString                 m_strClientDir;
        TString                 m_strDataDir;
        TString                 m_strGenDriverLibName;
        TString                 m_strLocalDataDir;
        TString                 m_strLocalLogDir;
        TString                 m_strMacroRootPath;
        TString                 m_strMacroRootClassScope;
        TString                 m_strMasterWSHost;
        TString                 m_strOrgDataDir;
        TString                 m_strProtoDriverLibName;
        TString                 m_strRepositoryDir;
        TString                 m_strServerDataDir;
        TString                 m_strServerDir;
        TString                 m_strUsersDir;
        TThread                 m_thrEvReceive;
        TThread                 m_thrEvSend;


        // -------------------------------------------------------------------
        //  Private, static data members
        // -------------------------------------------------------------------
        static TStatsCacheItem  m_sciOutTrigsDropped;
        static TStatsCacheItem  m_sciTrigsReceived;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCKit,TFacility)
};

#pragma CIDLIB_POPPACK


