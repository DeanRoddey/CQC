//
// FILE NAME: CQCIntfEng_WebImg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/01/2003
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
//  This is the header for the CQCIntfEng_WebImg.cpp file, which implements a widget
//  that loads images from web servers and displays them. So you set a URL on the
//  widget and it loads that referenced image. Due to the slowness of such image
//  retrieval we have to do it via a bgn thread, which greatly complicates this
//  widget type.
//
//  Unless told not to do it or we are auto-refresh, we push these images through
//  the local image cache, the web image part of the cache. We maintain a time
//  stamp from the server and only go back to the server if the URL changes or we
//  can't find the image in the cache or the time stamp has expired (though it may
//  still come back and tell us it's not changed.) If auto-refreshing, we just
//  always go to the web server.
//
//  We have three versions. A static version that just takes a URL as configured
//  or dynamically at runtime via command. Then we have field and variable versions
//  that get the URL from a field or variable. If the URL is empty, then nothing
//  is done and the image is cleared.
//
//  All are derived from a common class that does all the work, with the derived
//  classes just handling how the URL is gotten and set on the base class.
//
//  We use a thread to do the downloading in the background. In auto-refresh mode it
//  constantly updates at the refresh rate. If not, then it will loop once each time
//  the URL changes then wait around or a new URL.
//
//  If we are in RIVA mode, and auto refresh is enabled, we will force it to not
//  be any faster than once every five seconds.
//
// CAVEATS/GOTCHAS:
//
//  1)  In version 4.1.912 we refactored. There used to be a single class,
//      TCQCIntfWebImg, which was a statically configured URL. We changed this
//      to be a base class that does all the work and static, field, and variable
//      based derivatives that just set the image on the base clas.
//
//      The old class was left around, stripped down to just streaming in
//      code, so that it can be ready in by the container class. We added some
//      special case code in the container to watch for the old class and
//      to actually create one of the new static ones and pass it the info.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWebImg
//
//  This has been refactored away. We just keep this highly stripped down
//  version around so that we can read in previously stored versions and
//  convert them to the new static web image class.
// ---------------------------------------------------------------------------
class TCQCIntfWebImg : public TCQCIntfWidget
{
    public :
        TCQCIntfWebImg();
        ~TCQCIntfWebImg();
        TSize szDefaultSize() const;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;
        tCIDLib::TVoid Update(TGraphDrawDev&, const TArea&, TGUIRegion&) override;

        tCIDLib::TBoolean   m_bNoCache;
        tCIDLib::TCard1     m_c1ConstOpacity;
        tCIDGraphDev::EPlacement m_ePlacement;
        TString             m_strURL;

    protected :
        tCIDLib::TVoid CopyFrom(const TCQCIntfWidget&);
        tCIDLib::TVoid StreamFrom(TBinInStream&);

    private :
        RTTIDefs(TCQCIntfWebImg,TCQCIntfWidget)
        DefPolyDup(TCQCIntfWebImg)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWebImgBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfWebImgBase : public TCQCIntfWidget
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfWebImgBase();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSizeTo() const override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TBoolean bValidateParm
        (
            const   TCQCCmd&                cmdSrc
            , const TCQCCmdCfg&             ccfgTar
            , const tCIDLib::TCard4         c4Index
            , const TString&                strValue
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid PostInit() override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        TSize szDefaultSize() const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
        )   const override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bNoCache() const;

        tCIDLib::TBoolean bNoCache
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard1 c1ConstOpacity() const;

        tCIDLib::TCard1 c1ConstOpacity
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        tCIDLib::TCard4 c4RefreshMSs() const;

        tCIDLib::TCard4 c4RefreshMSs
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDGraphDev::EPlacement ePlacement() const;

        tCIDGraphDev::EPlacement ePlacement
        (
            const   tCIDGraphDev::EPlacement eToSet
        );

        const TKeyValuePair& kvalHdrAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid SetHdrAt
        (
            const   TString&                strKey
            , const TString&                strValue
            , const tCIDLib::TCard4         c4At
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfWebImgBase
        (
            const   tCQCIntfEng::ECapFlags  eCaps
            , const TString&                strTypeName
        );

        TCQCIntfWebImgBase
        (
            const   TCQCIntfWebImgBase&     iwdgSrc
        );

        TCQCIntfWebImgBase& operator=
        (
            const   TCQCIntfWebImgBase&     iwdgSrc
        );


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActiveUpdate
        (
            const tCIDLib::TBoolean         bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   override;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid QueryLiveURL
        (
                    TString&                strToFill
        )   const;

        tCIDLib::TVoid SetLiveURL
        (
            const   TString&                strToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bStopDownload();

        tCIDLib::EExitCodes eDownloader
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bNewImage
        //      The background thread sets this when there's a new image for the GUI
        //      thread to display. This is protected by the sync mutex.
        //
        //  m_bNoCache
        //      A configuration option. If set, it prevents us from paying any attention
        //      to the web server's caching stuff, so we always go back to the server
        //      to get an image.
        //
        //  m_bmpImg
        //      When the background thread indicates new data is availble (m_bNewImage)
        //      our active update callback will use the m_imgData info to update this
        //      bitmap with the latest image data, and redraw.
        //
        //  m_c1ConstOpacity
        //      Any constant opacity the user wants to apply.
        //
        //  m_c4RefreshMSs
        //      We can be set to auto-refresh. If this is non-zero, then the
        //      background thread is started and never stops. It just keeps getting
        //      more images and loading them up for the foreground thread to
        //      grab.
        //
        //  m_c4NewSerialNum
        //      Since we don't draw from the image cache, we need to insure that our
        //      local bitmap has the right serial number (and user data) set on it,
        //      so that RIVA clients will see the right serial number when we draw
        //      the m_bmpImage. So it stores this and the m_imgData value when it
        //      gest a new image. Our active update callback will update our bitmap
        //      and set this new serial number on it.
        //
        //  m_ePlacement
        //      The standard window placement options we offer for images.
        //      This is configured by the user.
        //
        //  m_imgData
        //      When the background thread gets new data, it stores the info here,
        //      and sets m_bNewImage to indicate new image data is availble.
        //
        //  m_kvalHdr1
        //  m_kvalHdr2
        //      We support up to two key=value pairs that we will send as ingoing
        //      header lines.
        //
        //  m_mtxSync
        //      A mutex to sync access to the m_bmpImg, m_c4NewSerialNum, m_strLiveURL
        //      and m_bNewImage members, the stuff shared between the bgn thread and
        //      our GUI thread callbacks.
        //
        //  m_strLiveURL
        //      The URL of the image we are currently working with. This is whatever
        //      is set on us by the derived class, so it's a runtime only thing. As
        //      opposed to the configured URL on the static derivative.
        //
        //      This is protected by the sync mutex.
        //
        //  m_thrDownloader
        //      We have to do the access of the image in the background since
        //      we can't stop the GUI thread. So this thread does that work.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bNewImage;
        tCIDLib::TBoolean           m_bNoCache;
        TBitmap                     m_bmpEmpty;
        TBitmap                     m_bmpImg;
        tCIDLib::TCard1             m_c1ConstOpacity;
        tCIDLib::TCard4             m_c4NewSerialNum;
        tCIDLib::TCard4             m_c4RefreshMSs;
        tCIDGraphDev::EPlacement    m_ePlacement;
        TBitmapImage                m_imgData;
        TKeyValuePair               m_kvalHdr1;
        TKeyValuePair               m_kvalHdr2;
        TMutex                      m_mtxSync;
        TString                     m_strLiveURL;
        TThread                     m_thrDownloader;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfWebImgBase,TCQCIntfWidget)
        DefPolyDup(TCQCIntfWebImgBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldWebImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFldWebImg

    : public TCQCIntfWebImgBase, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFldWebImg();

        TCQCIntfFldWebImg
        (
            const   TCQCIntfWebImg&         iwdgOld
        );

        TCQCIntfFldWebImg
        (
            const   TCQCIntfFldWebImg&      iwdgSrc
        );

        ~TCQCIntfFldWebImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfFldWebImg& operator=
        (
            const   TCQCIntfFldWebImg&      iwdgSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             fldTest
            , const TString&                strMake
            , const TString&                strModel
            , const tCIDLib::TBoolean       bStore
        )   override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid RegisterFields
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        )   override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FieldChanged
        (
                    TCQCFldPollInfo&        cfpiAssoc
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid ValueUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bVarsUpdate
            , const TStdVarsTar&            ctarGlobalVars
            , const TGUIRegion&             grgnClip
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfFldWebImg,TCQCIntfWebImgBase)
        DefPolyDup(TCQCIntfFldWebImg)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticWebImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfStaticWebImg : public TCQCIntfWebImgBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStaticWebImg();

        TCQCIntfStaticWebImg
        (
            const   TCQCIntfWebImg&         iwdgOld
        );

        TCQCIntfStaticWebImg
        (
            const   TCQCIntfStaticWebImg&   iwdgToCopy
        );

        ~TCQCIntfStaticWebImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfStaticWebImg& operator=
        (
            const   TCQCIntfStaticWebImg&   iwdgSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TBoolean bValidateParm
        (
            const   TCQCCmd&                cmdSrc
            , const TCQCCmdCfg&             ccfgTar
            , const tCIDLib::TCard4         c4Index
            , const TString&                strValue
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid PostInit() override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strURL() const;

        const TString& strURL
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strURL
        //      The statically configured URL of the image we are to load.
        // -------------------------------------------------------------------
        TString m_strURL;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfStaticWebImg,TCQCIntfWebImgBase)
        DefPolyDup(TCQCIntfStaticWebImg)
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarWebImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfVarWebImg

    : public TCQCIntfWebImgBase, public MCQCIntfVarIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfVarWebImg();

        TCQCIntfVarWebImg
        (
            const   TCQCIntfWebImg&         iwdgOld
        );

        TCQCIntfVarWebImg
        (
            const   TCQCIntfVarWebImg&      iwdgSrc
        );

        ~TCQCIntfVarWebImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfVarWebImg& operator=
        (
            const   TCQCIntfVarWebImg&      iwdgSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptVar
        (
            const   TCQCActVar&             varToTest
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid ValueUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bVarsUpdate
            , const TStdVarsTar&            ctarGlobalVars
            , const TGUIRegion&             grgnClip
        )   override;

        tCIDLib::TVoid VarInError
        (
            const   tCIDLib::TBoolean       bNoRedraw
        )   override;

        tCIDLib::TVoid VarValChanged
        (
            const   TCQCActVar&             varNew
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfVarWebImg,TCQCIntfWebImgBase)
        DefPolyDup(TCQCIntfVarWebImg)
};

#pragma CIDLIB_POPPACK




