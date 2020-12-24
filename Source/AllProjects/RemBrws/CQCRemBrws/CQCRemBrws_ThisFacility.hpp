//
// FILE NAME: CQCRemBrws_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
//  This file defines facility class for this facility. Some definitions for the
//  types and names used below:
//
//      Type            - The data type, macro, image, etc...
//      HPath           - A path in the full dataserver hierarchy, so
//                          /Customize/Macros/User/MyMacro
//      Path/RelPath
//                      - A type relative path, so it's part of the above HPath
//                          /User/MyMacro
//      Full/LocalPath  - Used on the data server side and is the full file system
//                          path that represents a given HPath/DSPath.
//
//  We provide a second cpp file that holds some temporary code that handles conversion
//  of data server files from the new chunked file format to the old ad hoc formats that
//  were used before. This is used by the installer to convert files during upgrade from
//  a pre-5.3.903 version.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCRemBrws
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCREMBRWSEXP TFacCQCRemBrws : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Public, static data members
        // -------------------------------------------------------------------
        static const TString strReportDTDURN;
        static const TString strReportDTD;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCRemBrws();

        TFacCQCRemBrws(const TFacCQCRemBrws&) = delete;
        TFacCQCRemBrws(TFacCQCRemBrws&&) = delete;

        ~TFacCQCRemBrws();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCRemBrws& operator=(const TFacCQCRemBrws&) = delete;
        TFacCQCRemBrws& operator=(TFacCQCRemBrws&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddPathComp
        (
                    TString&                strTar
            , const TString&                strToAppend
        );

        tCIDLib::TVoid AdjustFileName
        (
                    TPathStr&               pathCur
        );

        tCIDLib::TVoid AppendTypeExt
        (
                    TString&                strTar
            , const tCQCRemBrws::EDTypes    eType
        );

        tCIDLib::TBoolean bCheckIsSystemScope
        (
            const   TString&                strHPath
            ,       tCQCRemBrws::EDTypes&   eType
        )   const;

        tCIDLib::TBoolean bIsSystemScope
        (
            const   TString&                strHPath
            , const tCQCRemBrws::EDTypes    eType
        )   const;

        tCIDLib::TBoolean bRemoveLastPathItem
        (
                    TString&                strTar
        );

        tCIDLib::TVoid BasePathForType
        (
            const   tCQCRemBrws::EDTypes    eType
            ,       TString&                strHPathToFill
        )   const;

        tCIDLib::TVoid ChunkUpEvMonitor
        (
            const   TCQCEvMonCfg&           emoncSrc
            ,       TChunkedFile&           chflToFill
            , const tCIDLib::TCard4         c4InitSN = 1
        )   const;

        tCIDLib::TVoid ChunkUpGlobalAct
        (
            const   TCQCStdCmdSrc&          csrcSrc
            ,       TChunkedFile&           chflToFill
            , const tCIDLib::TCard4         c4InitSN = 1
        )   const;

        tCIDLib::TVoid ChunkUpImage
        (
            const   TPNGImage&              imgiSrc
            ,       TChunkedFile&           chflToFill
            , const tCIDLib::TCard4         c4InitSN = 1
        )   const;

        tCIDLib::TVoid ChunkUpMacro
        (
            const   TString&                strText
            ,       TChunkedFile&           chflToFill
            , const tCIDLib::TCard4         c4InitSN = 1
        )   const;

        tCIDLib::TVoid ChunkUpSchedEvent
        (
            const   TCQCSchEvent&           csrcSrc
            ,       TChunkedFile&           chflToFill
            , const tCIDLib::TCard4         c4InitSN = 1
        )   const;

        tCIDLib::TVoid ChunkUpTemplate
        (
            const   TMemBuf&                mbufTmpl
            , const tCIDLib::TCard4         c4Bytes
            ,       TChunkedFile&           chflToFill
            , const tCQCKit::EUserRoles     eMinRole
            , const TString&                strNotes
            , const tCIDLib::TCard4         c4InitSN = 1
        )   const;

        tCIDLib::TVoid ChunkUpTrigEvent
        (
            const   TCQCTrgEvent&           csrcSrc
            ,       TChunkedFile&           chflToFill
            , const tCIDLib::TCard4         c4InitSN = 1
        )   const;

        tCIDLib::TVoid CMLClassPathToHPath
        (
            const   TString&                strClassPath
            ,       TString&                strToFill
        )   const;

        tCIDLib::TVoid CMLClassPathToRelPath
        (
            const   TString&                strClassPath
            ,       TString&                strToFill
        )   const;

        tCIDLib::TVoid CreateRelPath
        (
            const   TString&                strHPath
            , const tCQCRemBrws::EDTypes    eType
            ,       TString&                strToFill
        );

        tCIDLib::TVoid CreateTypePaths
        (
            const   tCQCRemBrws::EDTypes    eType
            ,       TString&                strHPath
            ,       TString&                strLocalPath
        );

        tCIDLib::TVoid ConvertToChunkedFormat
        (
            const   tCQCRemBrws::EDTypes    eType
            , const TString&                strSource
            ,       TBinInStream&           strmSrc
            ,       TChunkedFile&           chflToFill
        );

        tCQCRemBrws::EDTypes eXlatPathType
        (
            const   TString&                strHPath
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        )   const;

        tCQCRemBrws::EDTypes eConvertPath
        (
            const   TString&                strHPath
            , const tCIDLib::TBoolean       bIsScope
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bGetFullPath
        )   const;

        tCQCRemBrws::EDTypes eConvertPath
        (
            const   TString&                strHPath
            , const tCIDLib::TBoolean       bIsScope
            ,       TString&                strRelPath
            ,       TString&                strLocalPath
        )   const;

        tCIDLib::TVoid HPathToCMLClassPath
        (
            const   TString&                strHPath
            ,       TString&                strToFill
        )   const;

        tCQCRemBrws::TDSrvClProxy orbcDataSrv();

        tCIDLib::TVoid QueryNamePart
        (
            const   TString&                strSrcPath
            ,       TString&                strToFill
        );

        tCIDLib::TVoid QueryParentPath
        (
            const   TString&                strPath
            ,       TString&                strToFill
        );

        tCIDLib::TVoid RelPathToCMLClassPath
        (
            const   TString&                strRelPath
            ,       TString&                strToFill
        )   const;

        const TString& strFlTypeDescr
        (
            const   tCQCRemBrws::EDTypes    eType
        )   const;

        const TString& strFlTypeExt
        (
            const   tCQCRemBrws::EDTypes    eType
        )   const;

        const TString& strFlTypePref
        (
            const   tCQCRemBrws::EDTypes    eType
        )   const;

        tCIDLib::TVoid SplitPath
        (
                    TString&                strPath
            ,       TString&                strName
        );

        tCIDLib::TVoid ToHPath
        (
            const   TString&                strRelPath
            , const tCQCRemBrws::EDTypes    eType
            ,       TString&                strToFill
        );

        tCIDLib::TVoid ToHPath
        (
            const   TString&                strRelScope
            , const TString&                strFile
            , const tCQCRemBrws::EDTypes    eType
            ,       TString&                strToFill
        );



    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        using TFTList = TObjArray<TString, tCQCRemBrws::EDTypes>;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_objaFTExts
        //      An array of strings to hold the file type extensions, indexed by the data
        //      type enum. Only those that actually have extensions are set, which is
        //      basically those managed by the data server.
        //
        //  m_objaFTPrefs
        //      An array of strings to hold the data type base paths, indexed by the data
        //      type enum. These are where they end up in the admin client's browser tree.
        //
        //  m_objFTSysPrefs
        //      We pre-build the base system scope for each file type, to make the
        //      bIsSystemScope() call much faster. These only really matter for the stuff
        //      managed by the data server, where there is a user/system distinction. For
        //      everything else it's all basically 'system' data.
        //
        //  m_strBadChars
        //      Characters we will replace with spaces in any files we get via drag
        //      and drop (where we can't control the original names.)
        // -------------------------------------------------------------------
        TFTList         m_objaFTExts;
        TFTList         m_objaFTPrefs;
        TFTList         m_objaFTSysPrefs;
        const TString   m_strBadChars;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCRemBrws,TFacility)
};

#pragma CIDLIB_POPPACK


