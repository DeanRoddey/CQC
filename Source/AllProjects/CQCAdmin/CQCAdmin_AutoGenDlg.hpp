//
// FILE NAME: CQCCAdmin_AutoGenDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/14/2014
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
//  This file implements the dialog that spits out the templates and images based
//  on the auto-generation information. We basically just provide the user with
//  a 'start' button, and a status display text widget. When they start us, we
//  kick off a background thread that does the work. It posts codes back to the
//  GUI thread to let the GUI thread update the status display.
//
// CAVEATS/GOTCHAS:
//
//  1)  Remember that if you remove any widgets, they might be in a group. We provide
//      a DeleteWidget private method to make sure that widgets get removed correctly
//      whether in a group or not. So if you aren't sure a widget is ungrouped, call
//      that method to remove it.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TAutoGenDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TAutoGenDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAutoGenDlg();

        TAutoGenDlg(const TAutoGenDlg&) = delete;
        TAutoGenDlg(TAutoGenDlg&&) = delete;

        ~TAutoGenDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAutoGenDlg& operator=(const TAutoGenDlg&) = delete;
        TAutoGenDlg& operator=(TAutoGenDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
            const   TWindow&                wndOwner
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TVoid Destroyed() final;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        using TTmplList = TVector<TCQCIntfTemplate>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4AddPrefabTmplsToList
        (
            const   tCIDLib::TCh* const     pszRelPath
        );

        tCIDLib::TVoid CalcScaleFactors
        (
            const   TSize&                  szTarRes
            ,       tCIDLib::TFloat8&       f8XScale
            ,       tCIDLib::TFloat8&       f8YScale
        );

        tCIDLib::TVoid DeleteWidget
        (
                    TCQCIntfTemplate&       iwdgParent
            , const TString&                strName
        );

        tCIDLib::TVoid DeleteWidget
        (
                    TCQCIntfTemplate&       iwdgParent
            ,       TCQCIntfWidget* const   piwdgToDelete
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::EExitCodes eGenThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        const TCQCIntfTemplate& iwdgFindTileTmpl
        (
            const   TString&                strUID
        )   const;

        tCIDLib::TVoid GenTemplates
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid GenTemplatesOneRes
        (
                    TThread&                thrThis
            , const TCQCSysCfgRmInfo&       scriCur
            ,       TTmplList&              colTmpls
            , const TString&                strResFmt
        );

        tCIDLib::TVoid ProcessHVACList
        (
                    TCQCIntfTemplate&       iwdgEdit
            , const TCQCSysCfgRmInfo&       scriCur
        );

        tCIDLib::TVoid ProcessLightList
        (
                    TCQCIntfTemplate&       iwdgEdit
            , const TCQCSysCfgRmInfo&       scriCur
        );

        tCIDLib::TVoid ProcessMain
        (
                    TCQCIntfTemplate&       iwdgEdit
            , const TCQCSysCfgRmInfo&       scriCur
        );

        tCIDLib::TVoid ProcessMainLeft
        (
                    TCQCIntfTemplate&       iwdgEdit
            , const TCQCSysCfgRmInfo&       scriCur
        );

        tCIDLib::TVoid ProcessNowPlaying
        (
                    TCQCIntfTemplate&       iwdgEdit
            , const TCQCSysCfgRmInfo&       scriCur
            , const tCIDLib::TBoolean       bMusic
        );

        tCIDLib::TVoid ProcessTiles
        (
                    TCQCIntfTemplate&       iwdgEdit
            , const TCQCSysCfgRmInfo&       scriCur
            , const TString&                strResFmt
        );

        tCIDLib::TVoid ProcessWeatherFC
        (
                    TCQCIntfTemplate&       iwdgEdit
            , const TCQCSysCfgRmInfo&       scriCur
        );

        tCIDLib::TVoid QUpMsg
        (
            const   tCIDLib::TCh* const     pszText
        );

        tCIDLib::TVoid QUpMsg
        (
            const   tCIDLib::TCh* const     pszText
            , const MFormattable&           fmtblToken1
        );

        tCIDLib::TVoid QUpMsg
        (
            const   tCIDLib::TCh* const     pszText
            , const MFormattable&           fmtblToken1
            , const MFormattable&           fmtblToken2
        );

        tCIDLib::TVoid RemoveGroup
        (
                    TCQCIntfTemplate&       iwdgEdit
            , const TString&                strGroupName
        );

        tCIDLib::TVoid ReplaceAudioSrc
        (
                    TCQCIntfWidget&         iwdgTar
            , const TSCMediaInfo&           scliInfo
            , const tCQCSysCfg::ERmFuncs    eFunc
        );

        tCIDLib::TVoid ReplaceMoniker
        (
                    TCQCIntfTemplate&       iwdgTar
            , const TString&                strSrc
            , const TString&                strTar
        );

        const TString& strFindTarPath
        (
            const   TString&                strSrcPath
        )   const;

        const TString& strFindXOverTarPath
        (
            const   TString&                strSrcPath
        )   const;

        tCIDLib::TVoid ScaleImages
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid ScaleImgsOneRes
        (
            const   TSize&                  szTarRes
            , const tCIDLib::TFloat8        f8XScale
            , const tCIDLib::TFloat8        f8YScale
            , const TObjArray<TPNGImage>&   objaImgs
            ,       TThread&                thrThis
        );

        tCIDLib::TVoid ScaleTemplates
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid UpdateGActCommand
        (
                    MCQCCmdSrcIntf&         mcsrcUpdate
            , const tCQCSysCfg::EGlobActs   eAct
            , const TCQCSysCfgRmInfo&       scriSrc
            , const TString&                strEvent
        );

        tCIDLib::TVoid UpdateTmplForRoom
        (
                    TThread&                thrThis
            , const tCIDLib::TCard4         c4TmplIndex
            ,       TCQCIntfTemplate&       iwdgTar
            , const TCQCSysCfgRmInfo&       scriCur
            , const TString&                strResFmt
        );



        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bSkipImages
        //      We allow them to skip the images. If they know they've not changed
        //      any images since the last time, this can vastly speed up the process by not
        //      re-scaling them again.
        //
        //  m_bVerbose
        //      We let the user check a check box to have us send more info to the output
        //      window.
        //
        //  m_c4PreTmplCnt
        //      The number of predefined templates. We remember this so that we know
        //      which templates in m_colScrTmpls and m_colSrcTPaths are for the predefined
        //      ones and which are for the use defined ones.
        //
        //  m_c4SingleImgCnt
        //      The number of slots in the m_colImgMap list that are the single image refs.
        //      Any after this are from scopes, which we want to scale, but we don't need
        //      to do search and replace on them, which saves a lot of generation time. We
        //      add all the scope images to the list so that we have a single list to scale.
        //
        //  m_c4TmplInd_XXX
        //      We remember the indices of specific templates in the template lists,
        //      because we want to do special processing on them later.
        //
        //  m_colImgMap
        //  m_colScopeMap
        //      A list of unique paths of images referenced by all of our source templates.
        //      The key is the source path. The target is the target path (with a replacement
        //      token for the resolution so that we can easily create the target path for
        //      each res.)
        //
        //      For those widgets that return a scope, we store those scopes in the same
        //      way, but we also add any of the images they contain to the image map as well,
        //      for scaling. We remember where the single images end, using m_c4SingleImgCnt
        //      We don't need to do search and replace on the ones from the scopes, we only
        //      need to search and replace the scopes.
        //
        //  m_colMsgQ
        //      The background thread posts msgs to this, and our timer handler grabs them
        //      out and dipslays them. It is set up thread safe of course.
        //
        //  m_colSrcTmpls
        //      We download the content of all of the source templates and store them here.
        //      They are in the same order as m_colSrcTPaths, so a given index gets the
        //      related values. m_c4PreTmplCnt of them are the data for the prefab templates,
        //      and any others are use defined ones.
        //
        //  m_colTileTmpls
        //      These are the tile templates defined for the extra overlays. These are not
        //      processed and uploaded. They are integrated into tiles on the tiles overlay.
        //      But we still need to download them and search them for image references. So
        //      we keep a separate list.
        //
        //  m_colTPaths
        //      The source and target paths of each template (prefab and any of the user
        //      defined ones.) As with the images, the target paths have a replacement token
        //      that will be replaced with each output res. And these also have a replacement
        //      token for the room as well. Also, as with m_colSrcTmpls, m_c4PreTmplCnt of
        //      these are the prefab tmpls and any beyond that are the user defined ones.
        //
        //  m_pdsclSrv
        //      We need a data server client in many places here, so we get one at the
        //      start and keep it around.
        //
        //  m_pwndXXX
        //      Since we have to interact with these controls a lot, we get convenience,
        //      typed, pointers to them. We don't own these, we just are looking at them
        //      a convenient way.
        //
        //  m_regxSaR
        //      We need a regex for some of the search/replace we do to update the templates.
        //
        //  m_scfgGen
        //      We download the room configuration here which is used to drive the process.
        //
        //  m_strQMsg
        //      A temp string for the timer to use for pulling msgs out of the msg queue.
        //
        //  m_strmImg
        //  m_strmThumb
        //      Output straems for upload/download. They are chunked streams so they can grow
        //      to the sized needed efficiently.
        //
        //  m_strRmListIds
        //  m_strRmListNames
        //      During some initial iteration of the configured rooms, we build up
        //      some enumerated limit strings that we set on variables in the OnPreload
        //      of the main template. These are used to present the list of rooms to
        //      the user for selection.
        //
        //  m_strSrcPath
        //      The top level path that contains the source templates that we use as source
        //      material for the autogen. This is a known thing so we can set it up and leave
        //      it around.
        //
        //  m_strTmpMsg
        //      A temp string when we need to build up a msg to queue and it needs to have
        //      info formatted out.
        //
        //  m_szSrcRes
        //      We know the resolution of the source templates, so we can store that on ctor
        //      and keep it around. This will be used to calculate the scaling ratio (for each
        //      target res.)
        //
        //  m_tmidToDo
        //      We crank up a timer to grab stuff posted by the background thread and display
        //      it, and to watch for the process to complete or fail.
        //
        //  m_thrGenerate
        //      We start up this thread and point it at eGenThread to do the actual gen of
        //      the solution content.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bSkipImages;
        tCIDLib::TBoolean       m_bVerbose;
        tCIDLib::TCard4         m_c4PreTmplCnt;
        tCIDLib::TCard4         m_c4SingleImgCnt;
        tCIDLib::TCard4         m_c4TmplInd_HVACList;
        tCIDLib::TCard4         m_c4TmplInd_LightsList;
        tCIDLib::TCard4         m_c4TmplInd_Main;
        tCIDLib::TCard4         m_c4TmplInd_MainLeft;
        tCIDLib::TCard4         m_c4TmplInd_MovieNP;
        tCIDLib::TCard4         m_c4TmplInd_MusicNP;
        tCIDLib::TCard4         m_c4TmplInd_Tiles;
        tCIDLib::TCard4         m_c4TmplInd_WeatherFC;
        tCIDLib::TKVPList       m_colImgMap;
        tCIDLib::TKVPList       m_colScopeMap;
        tCIDLib::TKVPList       m_colTPaths;
        TDeque<TString>         m_colMsgQ;
        TTmplList               m_colSrcTmpls;
        TTmplList               m_colTileTmpls;
        TDataSrvClient*         m_pdsclSrv;
        TPushButton*            m_pwndCloseButton;
        TPushButton*            m_pwndGenButton;
        TCheckBox*              m_pwndSkipImgs;
        TTextOutWnd*            m_pwndStatus;
        TCheckBox*              m_pwndVerbose;
        TRegEx                  m_regxSaR;
        TCQCSysCfg              m_scfgGen;
        TString                 m_strQMsg;
        TBinMBufOutStream       m_strmImg;
        TBinMBufOutStream       m_strmThumb;
        TString                 m_strRmListIds;
        TString                 m_strRmListNames;
        const TString           m_strSrcPath;
        TString                 m_strTmpMsg;
        const TSize             m_szSrcRes;
        tCIDCtrls::TTimerId     m_tmidToDo;
        TThread                 m_thrGenerate;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TAutoGenDlg, TDlgBox)
};

#pragma CIDLIB_POPPACK

