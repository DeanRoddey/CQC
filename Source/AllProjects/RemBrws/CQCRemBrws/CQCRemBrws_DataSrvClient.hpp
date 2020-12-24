//
// FILE NAME: CQCRemBrws_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/27/2015
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
//  This file creates a wrapper around the data server access client proxy. That
//  interface is necessarily fairly generic, because ithas to deal with all kinds
//  of data server managed resources. All files are read/written as flattened
//  objects in a buffer, and all of them are read via a first/next scheme that
//  allows arbitrarily large files to be managed.
//
//  So this guy just makes it a lot easier for clients to use this interface, and
//  provides data type specific methods for reading/writing files.
//
//
//  NOTE that the data server interface works in terms of the system wide namespace,
//  and the data server provides the /Customize part of that name space. So internally
//  a type relative path like this:
//
//      /User/Overlay_Weather
//
//  becomes an 'hpath' or hierarchical path like this:
//
//      /Customize/Interfaces/User/Overlay_Weather
//
//  Since that's where it fits within the larger hierarchy of the data server. In
//  order to simplify things, since all clients really just work in terms of their
//  data specific part of the hiearchy, most of our methods take a data type and
//  assume that incoming paths are type relative, and they return type relative
//  paths as well.
//
//  There are a few that explicitly take HPath parameters, since they are not type
//  specific, such as copy/paste, and the ones that the client tree browser uses to
//  maintain the tree, and some related to file/directory management. If they aren't
//  dealing with specific types of files, they usually take full HPaths.
//
//  Many of the calls are just straight passthroughs to the client proxy object
//  other than the path swizzling mentioned above.
//
// CAVEATS/GOTCHAS:
//
//  1)  IF WE CHANGE the stored format in any way, we have to update the conversion
//      code in the facility class to match, so that it will bring forward old content
//      (pre-5.3.903) during ugprades to the correct new state!
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TDataSrvAccClientProxy;
class TPNGImage;


// ---------------------------------------------------------------------------
//   CLASS: TDataSrvClient
//  PREFIX: dscl
// ---------------------------------------------------------------------------
class CQCREMBRWSEXP TDataSrvClient : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bFindMetaVal
        (
            const   TString&                strFind
            , const tCIDLib::TKVPFList&     colList
            ,       TString&                strToFill
        );

        static const TString& strFindMetaVal
        (
            const   TString&                strSrc
            , const TString&                strFind
            , const tCIDLib::TKVPFList&     colList
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDataSrvClient
        (
            const   tCIDLib::TBoolean       bQuickTest = kCIDLib::False
        );

        TDataSrvClient(const TDataSrvClient&) = delete;
        TDataSrvClient(TDataSrvClient&&) = delete;

        ~TDataSrvClient();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDataSrvClient& operator=(const TDataSrvClient&) = delete;
        TDataSrvClient& operator=(TDataSrvClient&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFileExists
        (
            const   TString&                strRelToFind
            , const tCQCRemBrws::EDTypes    eType
        );

        tCIDLib::TBoolean bFileExists
        (
            const   TString&                strRelScope
            , const TString&                strToFind
            , const tCQCRemBrws::EDTypes    eType
        );

        tCIDLib::TBoolean bFindFile
        (
            const   TString&                strRelToFind
            , const tCQCRemBrws::EDTypes    eType
            ,       TFindBuf&               fndbToFill
        );

        tCIDLib::TBoolean bFindFile
        (
            const   TString&                strRelScope
            , const TString&                strToFind
            , const tCQCRemBrws::EDTypes    eType
            ,       TFindBuf&               fndbToFill
        );

        tCIDLib::TBoolean bFindScope
        (
            const   TString&                strRelToFind
            , const tCQCRemBrws::EDTypes    eType
            ,       TFindBuf&               fndbToFill
        );

        tCIDLib::TBoolean bFindScope
        (
            const   TString&                strRelScope
            , const TString&                strToFind
            , const tCQCRemBrws::EDTypes    eType
            ,       TFindBuf&               fndbToFill
        );

        tCIDLib::TBoolean bIsConnected() const;

        tCIDLib::TBoolean bIsScope
        (
            const   TString&                strRelScope
            , const tCQCRemBrws::EDTypes    eType
        );

        tCIDLib::TBoolean bQueryImageInfo
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4ImageSz
            ,       tCIDLib::TCard4&        c4ThumbSz
            ,       tCIDImage::EPixFmts&    eImgFmt
            ,       tCIDImage::EBitDepths&  eImgDepth
            ,       TSize&                  szImg
            ,       TSize&                  szThumb
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const tCIDLib::TBoolean       bThrowIfNotFnd
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bQueryItemMetaData
        (
            const   TString&                strRelPath
            , const tCQCRemBrws::EDTypes    eType
            ,       tCIDLib::TKVPFList&     colMeta
            ,       tCIDLib::TStrList&      colChunkIds
            ,       tCIDLib::TCardList&     fcolChunkSizes
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const tCIDLib::TBoolean       bThrowIfNot
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bQueryScopeNames
        (
                    tCIDLib::TCard4&        c4ScopeId
            , const TString&                strRelPath
            , const tCQCRemBrws::EDTypes    eType
            ,       tCIDLib::TStrList&      colToFill
            , const tCQCRemBrws::EQSFlags   eQSFlags
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bQueryTemplateInfo
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4TmplSz
            ,       tCQCKit::EUserRoles&    eMinRole
            ,       TString&                strNotes
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const tCIDLib::TBoolean       bThrowIfNotFnd
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bReadEventMon
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            ,       TCQCEvMonCfg&           emoncToFill
            ,       tCIDLib::TKVPFList&     colMeta
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache = kCIDLib::False
        );

        tCIDLib::TBoolean bReadFile
        (
            const   TString&                strRelPath
            , const tCQCRemBrws::EDTypes    eDType
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            ,       TChunkedFile&           chflToFill
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache = kCIDLib::False
        );

        tCIDLib::TBoolean bReadGlobalAction
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            ,       TCQCStdCmdSrc&          csrcToFill
            ,       tCIDLib::TKVPFList&     colMeta
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache = kCIDLib::False
        );

        tCIDLib::TBoolean bReadImage
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            ,       TPNGImage&              imgToFill
            ,       tCIDLib::TKVPFList&     colMeta
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache = kCIDLib::False
        );

        tCIDLib::TBoolean bReadImage
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            ,       TMemBuf&                mbufToFill
            ,       tCIDLib::TCard4&        c4Bytes
            ,       tCIDLib::TKVPFList&     colMeta
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache = kCIDLib::False
        );

        tCIDLib::TBoolean bReadImage
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            ,       TMemBuf&                mbufFull
            ,       tCIDLib::TCard4&        c4FullBytes
            ,       TMemBuf&                mbufThumb
            ,       tCIDLib::TCard4&        c4ThumbBytes
            ,       tCIDLib::TKVPFList&     colMeta
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache = kCIDLib::False
        );

        tCIDLib::TBoolean bReadThumb
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            ,       TPNGImage&              imgToFill
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache = kCIDLib::False
        );

        tCIDLib::TBoolean bReadThumb
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            ,       THeapBuf&               mbufToFill
            ,       tCIDLib::TCard4&        c4Bytes
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache = kCIDLib::False
        );

        tCIDLib::TBoolean bReadMacro
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bDoEncrypted
            ,       tCIDLib::TKVPFList&     colMeta
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache = kCIDLib::False
        );

        tCIDLib::TBoolean bReadSchedEvent
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            ,       TCQCSchEvent&           csrcToFill
            ,       tCIDLib::TKVPFList&     colMeta
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache = kCIDLib::False
        );

        tCIDLib::TBoolean bReadTemplate
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            ,       TMemBuf&                mbufToFill
            ,       tCIDLib::TCard4&        c4Bytes
            ,       tCIDLib::TKVPFList&     colMeta
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache = kCIDLib::False
        );

        tCIDLib::TBoolean bReadTrigEvent
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            ,       TCQCTrgEvent&           csrcToFill
            ,       tCIDLib::TKVPFList&     colMeta
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache = kCIDLib::False
        );

        tCIDLib::TBoolean bScopeExists
        (
            const   TString&                strRelToFind
            , const tCQCRemBrws::EDTypes    eType
        );

        tCIDLib::TBoolean bScopeExists
        (
            const   TString&                strRelScope
            , const TString&                strToFind
            , const tCQCRemBrws::EDTypes    eType
        );

        tCIDLib::TVoid DeleteFile
        (
            const   TString&                strRelPath
            , const tCQCRemBrws::EDTypes    eType
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bMustExist = kCIDLib::False
        );

        tCIDLib::TVoid DeleteScope
        (
            const   TString&                strRelPath
            , const tCQCRemBrws::EDTypes    eType
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bMustExist = kCIDLib::False
        );

        tCIDLib::ERenameRes eRename
        (
            const   TString&                strRelParScope
            , const TString&                strOldName
            , const TString&                strNewName
            , const tCQCRemBrws::EDTypes    eType
            , const tCIDLib::TBoolean       bIsScope
            ,       tCIDLib::TCard4&        c4ParScopeId
            ,       tCIDLib::TCard4&        c4TarItemId
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid GenerateFullReport
        (
                    TTextOutStream&         strmTar
            , const TString&                strDescr
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid GenerateReport
        (
                    TTextOutStream&         strmTar
            , const TString&                strDescr
            , const TString&                strRelScope
            , const tCQCRemBrws::EDTypes    eType
            , const tCIDLib::TBoolean       bRecurse
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid MakeNewScope
        (
            const   TString&                strRelParScope
            , const TString&                strNewSubScope
            , const tCQCRemBrws::EDTypes    eType
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid MakePath
        (
            const   TString&                strRelPath
            , const tCQCRemBrws::EDTypes    eType
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid PastePath
        (
            const   TString&                strHSrcPath
            , const TString&                strHTarScope
            , const tCIDLib::TBoolean       bSrcIsScope
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid Ping();

        tCIDLib::TVoid QueryBrowserItem
        (
            const   TString&                strHPath
            ,       tCIDLib::TCardList&     fcolPathIds
            ,       TDSBrowseItem&          dsbiToFill
            ,       tCIDLib::TBoolean&      bIsScope
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid QueryBrowserItem2
        (
            const   TString&                strHPath
            ,       tCIDLib::TCardList&     fcolPathIds
            ,       TDSBrowseItem&          dsbiTarget
            ,       TDSBrowseItem&          dsbiParScope
            ,       tCIDLib::TBoolean&      bIsScope
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid QueryScopeItems
        (
            const   TString&                strHPath
            ,       tCIDLib::TCardList&     fcolPathIds
            ,       TVector<TDSBrowseItem>& colItems
            , const tCIDLib::TBoolean       bItemsOnly
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid QueryTree
        (
            const   TString&                strStartPath
            , const tCQCRemBrws::EDTypes    eType
            ,       TString&                strXML
            , const tCIDLib::TBoolean       bSparse
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid QueryTree2
        (
            const   TString&                strStartPath
            , const tCQCRemBrws::EDTypes    eType
            ,       tCIDLib::TStrList&      colToFill
            , const tCIDLib::TBoolean       bSparse
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid SetEventPauseState
        (
            const   tCQCKit::EEvSrvTypes    eType
            , const TString&                strRelPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const tCIDLib::TBoolean       bPaused
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid SetPeriodicEvTime
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const tCIDLib::TEncodedTime   enctStartTime
            , const tCIDLib::TCard4         c4Period
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid SetScheduledEvTime
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const tCIDLib::TCard4         c4Day
            , const tCIDLib::TCard4         c4Hour
            , const tCIDLib::TCard4         c4Min
            , const tCIDLib::TCard4         c4Mask
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid SetSunBasedEvOffset
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const tCIDLib::TCard4         i4Offset
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid UploadFile
        (
            const   TString&                strRelPath
            , const tCQCRemBrws::EDTypes    eDType
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const TChunkedFile&           chflToSend
            ,       tCIDLib::TCard4&        c4Flags
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteEventMon
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const TCQCEvMonCfg&           emoncToSend
            , const tCIDLib::TCard4         c4Flags
            , const tCIDLib::TKVPFList&     colExtraMeta
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteGlobalAction
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const TCQCStdCmdSrc&          csrcToSend
            , const tCIDLib::TCard4         c4Flags
            , const tCIDLib::TKVPFList&     colExtraMeta
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteImage
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const TPNGImage&              imgFull
            , const TPNGImage&              imgThumb
            , const tCIDImage::EPixFmts     eImgFmt
            , const tCIDImage::EBitDepths   eImgDepth
            , const tCIDLib::TCard4         c4Flags
            , const tCIDLib::TKVPFList&     colExtraMeta
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteImage
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const TMemBuf&                mbufImg
            , const tCIDLib::TCard4         c4ImgBytes
            , const TMemBuf&                mbufThumb
            , const tCIDLib::TCard4         c4ThumbBytes
            , const tCIDImage::EPixFmts     eImgFmt
            , const tCIDImage::EBitDepths   eImgDepth
            , const TSize&                  szImg
            , const TSize&                  szThumb
            , const tCIDLib::TCard4         c4Flags
            , const tCIDLib::TKVPFList&     colExtraMeta
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteMacro
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const TString&                strToSend
            , const tCIDLib::TCard4         c4Flags
            , const tCIDLib::TKVPFList&     colExtraMeta
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteScheduledEvent
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const TCQCSchEvent&           csrcToSend
            , const tCIDLib::TCard4         c4Flags
            , const tCIDLib::TKVPFList&     colExtraMeta
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteTemplate
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Bytes
            , const tCQCKit::EUserRoles     eMinRole
            , const TString&                strNotes
            , const tCIDLib::TCard4         c4Flags
            , const tCIDLib::TKVPFList&     colExtraMeta
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteTriggeredEvent
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const TCQCTrgEvent&           csrcToSend
            , const tCIDLib::TCard4         c4Flags
            , const tCIDLib::TKVPFList&     colExtraMeta
            , const TCQCSecToken&           sectUser
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bReadRawData
        (
            const   TString&                strRelPath
            , const tCQCRemBrws::EDTypes    eType
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TKVPFList&     colMeta
            ,       TMemBuf&                mbufDataChunk
            ,       tCIDLib::TCard4&        c4DataBytes
            ,       TString&                strExtChunkId
            ,       TMemBuf&                mbufExtChunk
            ,       tCIDLib::TCard4&        c4ExtBytes
            ,       tCIDLib::TCard4&        c4Flags
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid CheckIsHPath
        (
            const   TString&                strToCheck
        )   const;

        tCIDLib::TVoid CheckReady
        (
            const   tCIDLib::TBoolean       bQuickCheck = kCIDLib::False
        );

        tCIDLib::TVoid WriteRawData
        (
            const   TString&                strRelPath
            , const tCQCRemBrws::EDTypes    eType
            ,       tCIDLib::TCard4&        c4NewSerialNum
            , const tCIDLib::TKVPFList&     colMeta
            , const TMemBuf&                mbufDataChunk
            , const tCIDLib::TCard4         c4DataSz
            , const TString&                strExtChunkId
            , const TMemBuf&                mbufExtChunk
            , const tCIDLib::TCard4         c4ExtSz
            , const tCIDLib::TCard4         c4Flags
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const TCQCSecToken&           sectUser
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_porbcDS
        //      We use a pointer to hold the data server access proxy, so that we
        //      can keep the header internal.
        // -------------------------------------------------------------------
        TDataSrvAccClientProxy* m_porbcDS;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDataSrvClient,TObject)
};

#pragma CIDLIB_POPPACK



