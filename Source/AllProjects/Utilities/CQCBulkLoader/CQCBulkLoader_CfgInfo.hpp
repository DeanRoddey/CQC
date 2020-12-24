//
// FILE NAME: CQCBulkLoader_CfgInfo.hpp
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
//  This is the header for the CQCBulkLoader_CfgInfo.cpp file, which implements
//  some class that we use to store the configuration file info. It's all
//  parsed into memory first, then checking is done, then we do the actual
//  operation.
//
//  We have classes here to contain a list of things to load. We have driver
//  info (and the prompts they contain), event info, and image/scope info
//  hold a hierarchical list of images to load.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//   CLASS: TBLPromptInfo
//  PREFIX: pi
// ---------------------------------------------------------------------------
class TBLPromptInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBLPromptInfo();

        TBLPromptInfo
        (
            const   TString&                strKey
        );

        ~TBLPromptInfo();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddSubKey
        (
            const   TString&                strKey
            , const TString&                strValue
        );


        // -------------------------------------------------------------------
        //  Public data members
        // -------------------------------------------------------------------
        TVector<TKeyValuePair>  m_colSubKeys;
        TString                 m_strKey;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TBLPromptInfo,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TBLDrvInfo
//  PREFIX: drvi
// ---------------------------------------------------------------------------
class TBLDrvInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBLDrvInfo();

        TBLDrvInfo
        (
            const   TString&                strMoniker
            , const TString&                strMake
            , const TString&                strModel
            , const TString&                strHost
            , const TString&                strConnInfo
            , const TString&                strFldExport
        );

        ~TBLDrvInfo();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddPrompt
        (
            const   TBLPromptInfo&          piToAdd
        );

        tCIDLib::TCard4 c4PromptCount() const;

        const TCQCDriverObjCfg& cqcdcInfo() const;

        TCQCDriverObjCfg& cqcdcSetManifest
        (
            const   TCQCDriverCfg&          cqcdcToSet
        );

        const TBLPromptInfo& piAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TString& strConnInfo() const;

        const TString& strFldExport() const;

        const TString& strHost() const;

        const TString& strMake() const;

        const TString& strModel() const;

        const TString& strMoniker() const;



    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colPrompts
        //      The raw prompt info until we can get it into the driver cfg
        //      object.
        //
        //  m_cqcdcInfo
        //      We load the manifest info and then put the configured prompt
        //      values into the driver config object. This lets us test the
        //      prompt values and make use of all of the smarts already in
        //      this class.
        //
        //  m_strConnInfo
        //      The connection info. This will ultimately go into the driver
        //      config object, but until then we need to store it in the
        //      original formatted text that we get from the config file.
        //
        //  m_strFldExport
        //      If it's a variables driver, then they can point us to a field
        //      extraction file that we can use to load up the fields of the
        //      variables driver.
        //
        //  m_strHost
        //      The host on which we are to load this driver. This tells us
        //      which CQCServer to talk to.
        //
        //  m_strMake
        //  m_strModel
        //      The make/model of the driver, which we need to find the
        //      right manifest file and validate the parms and such, and in
        //      the driver loading process.
        //
        //  m_strMoniker
        //      The driver moniker to use for this driver
        // -------------------------------------------------------------------
        TVector<TBLPromptInfo>  m_colPrompts;
        TCQCDriverObjCfg        m_cqcdcInfo;
        TString                 m_strConnInfo;
        TString                 m_strFldExport;
        TString                 m_strHost;
        TString                 m_strMake;
        TString                 m_strModel;
        TString                 m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TBLDrvInfo,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TBLEventInfo
//  PREFIX: evi
// ---------------------------------------------------------------------------
class TBLEventInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBLEventInfo();

        TBLEventInfo
        (
            const   TString&                strName
        );

        ~TBLEventInfo();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strName() const;


    private :
        // -------------------------------------------------------------------
        //  Private class members
        //
        //  m_strName
        //      The name of the original source file name (within the directory that this
        //      event file is in, so relative to the current scope being processed.) This
        //      will also be the target name.
        // -------------------------------------------------------------------
        TString     m_strName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TBLEventInfo,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TBLEvScopeInfo
//  PREFIX: scpi
// ---------------------------------------------------------------------------
class TBLEvScopeInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  We need collection of event info objects that represent the events that
        //  up the source directory (and target scope.)
        //
        //  And we need a list of sub-dirs to handle the hierarchical nature of the info.
        // -------------------------------------------------------------------
        typedef TVector<TBLEventInfo>       TEvList;
        typedef TVector<TBLEvScopeInfo>     TSubList;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBLEvScopeInfo();

        TBLEvScopeInfo
        (
            const   TBLEvScopeInfo&         scpiToCopy
        );

        TBLEvScopeInfo
        (
            const   TString&                strSrcDir
            , const TString&                strTarScope
            , const tCQCRemBrws::EDTypes    eType
        );

        ~TBLEvScopeInfo();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddEvent
        (
            const   TString&                strName
        );

        tCIDLib::TBoolean bIsSchedEv() const
        {
            return (m_eType == tCQCRemBrws::EDTypes::SchEvent);
        }

        tCIDLib::TCard4 c4EvCount() const;

        tCIDLib::TCard4 c4SubDirCount() const;

        tCQCRemBrws::EDTypes eType() const
        {
            return m_eType;
        }

        const TBLEventInfo& eviAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TBLEvScopeInfo& scpiAddSubDir
        (
            const   TString&                strName
        );

        const TBLEvScopeInfo& scpiAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TString& strName() const;

        const TString& strTarScope() const;


    private :
        // -------------------------------------------------------------------
        //  Private class members
        //
        //  m_colEvents
        //      The list of events from this source path that will be put into the
        //      target scope path.
        //
        //  m_colSubDirs
        //      A list of subdirectories under this one, if there are any.
        //
        //  m_eType
        //      The type of event this tree is for
        //
        //  m_strName
        //      The name for this subdirectory and the target scope name. The exception
        //      is the top level one which needs separate values so this is the full
        //      path to the starting directory.
        //
        //  m_strTarScope
        //      For the top level block, we need separate values for the source dir
        //      and the target scope so this is the target scope.
        // -------------------------------------------------------------------
        TEvList                 m_colEvents;
        TSubList                m_colSubDirs;
        tCQCRemBrws::EDTypes    m_eType;
        TString                 m_strName;
        TString                 m_strTarScope;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TBLEvScopeInfo,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TBLmgInfo
//  PREFIX: imgi
// ---------------------------------------------------------------------------
class TBLImageInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBLImageInfo();

        TBLImageInfo
        (
            const   tCQCBulkLoader::EImgTrans eTransType
            , const TString&                strName
            , const TPoint&                 pntTransClr
            , const TRGBClr&                rgbTransClr
        );

        ~TBLImageInfo();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCBulkLoader::EImgTrans eTransType() const;

        const TPoint& pntTransClr() const;

        const TRGBClr& rgbTransClr() const;

        const TString& strName() const;


    private :
        // -------------------------------------------------------------------
        //  Private class members
        //
        //  m_eTransType
        //      The type of transparency this image uses, if any.
        //
        //  m_pntTransClr
        //      If the transparency type indicates it's a point based color, we'll have to
        //      grab that point from the image at the time that we load it to get the
        //      transparency color.
        //
        //  m_rgbTransClr
        //      If the image is a color based transparency and they give a literal color,
        //      that is stored here.
        //
        //  m_strName
        //      The name for this one. This is the source sub-dir and the target scope.
        // -------------------------------------------------------------------
        tCQCBulkLoader::EImgTrans   m_eTransType;
        TPoint                      m_pntTransClr;
        TRGBClr                     m_rgbTransClr;
        TString                     m_strName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TBLImageInfo,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TBLmgScopeInfo
//  PREFIX: scpi
// ---------------------------------------------------------------------------
class TBLImgScopeInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  We need collection of images that represent the images that
        //  make up the source directory (and target scope) that this object
        //  will represent.
        //
        //  And we need a list of sub-dirs to handle the hierarchical nature
        //  of the info.
        // -------------------------------------------------------------------
        typedef TVector<TBLImageInfo>       TImgList;
        typedef TVector<TBLImgScopeInfo>    TSubList;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBLImgScopeInfo();

        TBLImgScopeInfo
        (
            const   TBLImgScopeInfo&        scpiToCopy
        );

        TBLImgScopeInfo
        (
            const   TString&                strSrcName
            , const TString&                strTarName
        );

        ~TBLImgScopeInfo();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddImage
        (
            const   tCQCBulkLoader::EImgTrans eTransType
            , const TString&                strName
            , const TPoint&                 pntTransClr
            , const TRGBClr&                rgbTransClr
        );

        TBLImgScopeInfo& scpiAddSubDir
        (
            const   TString&                strName
        );

        tCIDLib::TCard4 c4ImgCount() const;

        tCIDLib::TCard4 c4SubDirCount() const;

        const TBLImageInfo& imgiAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TBLImgScopeInfo& scpiAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TString& strName() const;

        const TString& strTarScope() const;


    private :
        // -------------------------------------------------------------------
        //  Private class members
        //
        //  m_colImages
        //      The list of images from this source path that will be put into the target
        //      scope path.
        //
        //  m_colSubDirs
        //      A list of subdirectories under this one, if there are any.
        //
        //  m_strName
        //      At the top level, this is the source directory of the start of the tree.
        //      The rest of the way down it's the name of the next sub-dir/scope.
        //
        //  m_strTarScope
        //      At the top level, this is the path to the starting point of the of output
        //      scope tree. The rest of the way down this is not used.
        // -------------------------------------------------------------------
        TImgList        m_colImages;
        TSubList        m_colSubDirs;
        TString         m_strName;
        TString         m_strTarScope;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TBLImgScopeInfo,TObject)
};


