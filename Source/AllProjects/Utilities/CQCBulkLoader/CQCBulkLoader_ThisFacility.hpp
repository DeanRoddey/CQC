//
// FILE NAME: CQCBulkLoader_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2010
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
//  This is the header for the CQCBulkLoader_ThisFacility.cpp file, which
//  implements the facility class for this program. This guy manages all
//  the data and functionality of the program.
//
// CAVEATS/GOTCHAS:
//
//  1)  This guy is obviously single threaded, so we avoid issues with
//      the nested parsing stuff by just declaring method scoped statics
//      where we need to have data available for repeated use in parsing.
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCBulkLoader
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCBulkLoader : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCBulkLoader();

        ~TFacCQCBulkLoader();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TFacCQCBulkLoader(const TFacCQCBulkLoader&);
        tCIDLib::TVoid operator=(const TFacCQCBulkLoader&);


        // -------------------------------------------------------------------
        //  Private class types
        //
        //  We use a simple enum to throw exceptions that unwind us out of
        //  the parsing or loading operations. This way we don't need to
        //  have a message file in order to define standard exception text
        //  and such.
        //
        //  And we need some collections of things. And we us hash sets of
        //  strings as dup key/moniker type checkers.
        // -------------------------------------------------------------------
        enum EErrors
        {
            EError_UnexpectedEOF
            , EError_Data
            , EError_Syntax
            , EError_System
            , EError_Internal
        };

        typedef TVector<TBLDrvInfo>         TDrvList;
        typedef tCIDLib::TStrHashSet        TDupList;
        typedef TVector<TBLEvScopeInfo>     TEvList;
        typedef TVector<TBLImgScopeInfo>    TImgList;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckKey
        (
            const   TString&                strToCheck
            ,       TString&                strValue
            , const tCIDLib::TBoolean       bEmptyOK = kCIDLib::False
        );

        tCIDLib::TBoolean bCheckLine
        (
            const   TString&                strToCheck
        );

        tCIDLib::TBoolean bLogin();

        tCIDLib::TBoolean bInitialize();

        tCIDLib::TBoolean bParseCfgFile();

        tCIDLib::TBoolean bReadLine
        (
                    TString&                strToFill
            , const tCIDLib::TBoolean       bEndOK = kCIDLib::False
        );

        tCIDLib::TBoolean bValidateCfg();

        tCIDLib::TBoolean bValidateDrivers();

        tCIDLib::TBoolean bValidateEvents();

        tCIDLib::TBoolean bValidateEventTree
        (
            const   TBLEvScopeInfo&         scpiParent
            ,       TPathStr&               pathSrcDir
            ,       TString&                strTarScope
            , const tCIDLib::TBoolean       bTrigEv
        );


        tCIDLib::TBoolean bValidateHosts();

        tCIDLib::TBoolean bValidateImages();

        tCIDLib::TBoolean bValidateImageTree
        (
            const   TBLImgScopeInfo&        scpiParent
            ,       TPathStr&               pathSrcDir
            ,       TString&                strTarScope
        );

        tCIDLib::TBoolean bValidatePrompt
        (
            const   TString&                strMoniker
            , const TBLPromptInfo&          piTest
            ,       TCQCDriverObjCfg&       cqcdcTar
        );

        tCIDLib::TCard4 c4CheckKey
        (
            const   TString&                strKey
        );

        tCIDLib::TVoid CheckKey
        (
            const   TString&                strToCheck
            ,       TString&                strValue
            , const tCIDLib::TBoolean       bEmptyOK = kCIDLib::False
        );

        tCIDLib::TVoid CheckLine
        (
            const   TString&                strToCheck
        );

        tCQCBulkLoader::EImgTrans eParseImageData
        (
            const   TString&                strImgData
            ,       TString&                strName
            ,       TPoint&                 pntTransClr
            ,       TRGBClr&                rgbTransClr
        );

        tCIDLib::TVoid EatToEnd();

        tCIDLib::TVoid LoadEvents();

        tCIDLib::TVoid LoadEventTree
        (
                    TDataSrvClient&         dsclTar
            , const TBLEvScopeInfo&         scpiParent
            ,       TPathStr&               pathSrcDir
            ,       TString&                strTarScope
        );

        tCIDLib::TVoid LoadFldExtracts();

        tCIDLib::TVoid LoadImages();

        tCIDLib::TVoid LoadImageTree
        (
                    TDataSrvClient&         dsclTar
            , const TBLImgScopeInfo&        scpiParent
            ,       TPathStr&               pathSrcDir
            ,       TString&                strTarScope
            ,       TBinMBufOutStream&      strImg
            ,       TBinMBufOutStream&      strThumb
        );

        const TCQCDCfgPrompt* pcqcdcpByKey
        (
            const   TString&                strMoniker
            , const TCQCDriverCfg&          cqcdcSrc
            , const TString&                strKey
        );

        tCIDLib::TVoid ParseDriverBlock
        (
            const   TString&                strMoniker
        );

        tCIDLib::TVoid ParseDriversBlock();

        tCIDLib::TVoid ParseEventsBlock();

        tCIDLib::TVoid ParseEventTree
        (
                    TBLEvScopeInfo&         scpiParent
        );

        tCIDLib::TVoid ParseIgnoreBlock();

        tCIDLib::TVoid ParseImageTree
        (
                    TBLImgScopeInfo&        scpiParent
        );

        tCIDLib::TVoid ParseImagesBlock();

        tCIDLib::TVoid ParseMainBlock();

        tCIDLib::TVoid ParseUnloadBlock();

        tCIDLib::TVoid Pushback
        (
            const   TString&                strToPush
        );

        tCIDLib::TVoid ShowUsage();

        tCIDLib::TVoid UpdateDrivers();

        tCIDLib::TVoid ValidateTarName
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bExtOK
        );

        tCIDLib::TVoid ValidateTarPath
        (
            const   TString&                strPath
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLoadFldExtracts
        //      As we are loading the configuration file, we remember if any
        //      of them referenced field extract fields to load (into var
        //      driver instances.) If so, then at the end we need to go through
        //      and load those.
        //
        //  m_c4LineNum
        //      bReadLine bumps up the line number as we read them, so that
        //      we can report the line number in errors.
        //
        //  m_c4VersionNum
        //      The version number we read out of the config file, which
        //      indicates the version of the configuration file syntax we
        //      should expect.
        //
        //  m_colDupDrivers
        //  m_colDupKeys
        //      We use some hash sets of strings to check for duplicates,
        //      which we know we cannot have. For the drivers it's the moniker
        //      and we reuse the keys one within each driver to check for
        //      duplicate prompt names.
        //
        //  m_colDrivers
        //      The drivers that we are configured to load.
        //
        //  m_colEvents
        //      The list of path/pattern pairs that defines the scheduled
        //      and triggered events to load.
        //
        //  m_colHosts
        //      A list of unique hosts names referenced by the drivers. We
        //      make sure that each of them is contactable during the
        //      validation phase.
        //
        //  m_colUnloadDrvs
        //      A list of drivers to unload.
        //
        //  m_cuctxToUse
        //      After the login, we set up a user context object, which we
        //      pass to anything that requires user credentials.
        //
        //  m_imgXXX
        //      We need image objects to use in the image loading process.
        //      It's either a PNG or we convert it to a PNG.
        //
        //  m_pathCfgFile
        //      We get the path to the configuration file on the command line.
        //
        //  m_pathCfgRel
        //      The path part of the configuration file we get on the command line (see
        //      m_pathCfgFile above.) Any relative local paths in the config file are
        //      made relative to this.
        //
        //  m_strmCfg
        //      The input text stream for the configuration file
        //
        //  m_strmOut
        //      For flexibility we store the standard output stream here for
        //      use.
        //
        //  m_strFmt_XXX
        //      Some strings that we access fairly often because they are
        //      strings that are part of the file markup.
        //
        //  m_strDescription
        //      The description line we read from the configuration file
        //      header block.
        //
        //  m_strPushback
        //      We support a single level line pushback to allow for a little
        //      peaking ahead to see if optional blocks are present. If not
        //      empty, bReadLine will return it then clear it.
        //
        //  m_strPassword
        //  m_strUserName
        //      We have to get a user name and password of a CQC admin account
        //      since we have to do things that require that level of access.
        //
        //  m_strUserPref
        //      We check that various things are in the User section of that
        //      particular type of content, so we set up the name for quick
        //      checking.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bLoadFldExtracts;
        tCIDLib::TCard4     m_c4LineNum;
        tCIDLib::TCard4     m_c4VersionNum;
        TDrvList            m_colDrivers;
        TDupList            m_colDupDrivers;
        TDupList            m_colDupKeys;
        TEvList             m_colEvents;
        TDupList            m_colHosts;
        TImgList            m_colImages;
        tCIDLib::TStrList   m_colUnloadDrvs;
        TCQCUserCtx         m_cuctxToUse;
        TBitmapImage        m_imgBitmap;
        TPNGImage           m_imgPNG;
        TJPEGImage          m_imgJPEG;
        TPNGImage           m_imgThumb;
        TPathStr            m_pathCfgFile;
        TPathStr            m_pathCfgRel;
        TTextFileInStream   m_strmCfg;
        TTextOutStream&     m_strmOut;
        TString             m_strDescription;
        const TString       m_strBlk_DriverEnd;
        const TString       m_strBlk_Drivers;
        const TString       m_strBlk_DriversEnd;
        const TString       m_strBlk_CfgInfo;
        const TString       m_strBlk_CfgInfoEnd;
        const TString       m_strBlk_Events;
        const TString       m_strBlk_EventsEnd;
        const TString       m_strBlk_EventScope;
        const TString       m_strBlk_EventScopeEnd;
        const TString       m_strBlk_Images;
        const TString       m_strBlk_ImagesEnd;
        const TString       m_strBlk_ImageScope;
        const TString       m_strBlk_ImageScopeEnd;
        const TString       m_strBlk_Ignore;
        const TString       m_strBlk_IgnoreEnd;
        const TString       m_strBlk_Prompt;
        const TString       m_strBlk_PromptEnd;
        const TString       m_strBlk_Prompts;
        const TString       m_strBlk_PromptsEnd;
        const TString       m_strBlk_SubKeyEnd;
        const TString       m_strBlk_Unload;
        const TString       m_strBlk_UnloadEnd;
        const TString       m_strKey_ConnInfo;
        const TString       m_strKey_Descr;
        const TString       m_strKey_Driver;
        const TString       m_strKey_Event;
        const TString       m_strKey_EventEnd;
        const TString       m_strKey_FldExport;
        const TString       m_strKey_Host;
        const TString       m_strKey_Image;
        const TString       m_strKey_Key;
        const TString       m_strKey_Make;
        const TString       m_strKey_Model;
        const TString       m_strKey_Name;
        const TString       m_strKey_Pattern;
        const TString       m_strKey_Prompt;
        const TString       m_strKey_SrcDir;
        const TString       m_strKey_SubKey;
        const TString       m_strKey_TarName;
        const TString       m_strKey_TarScope;
        const TString       m_strKey_Type;
        const TString       m_strKey_Value;
        const TString       m_strKey_Version;
        TString             m_strPassword;
        TString             m_strPushback;
        TString             m_strUserName;
        TString             m_strUserPref;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCBulkLoader,TFacility)
};


