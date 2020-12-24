//
// FILE NAME: CQCAdmin_ScaleTmplsDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/28/2014
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
//  This is the header for the tempalte scaling dialog. This allows the user to
//  select a source template repo scope, and a target, and a new resolution for
//  the target. The templates in the source scope will be pre-scaled to fit the
//  target resolution (from a provided source resolution), and written to a sub-
//  scope under the target, which is named after the new resolution.
//
//  Once all the templates are scaled successfully, we hang onto them in memory,
//  then we will write them to the repo.
//
//  We can optionally do just the source scope, or recursively below the source,
//  creating the same tree at the target. We will remove either the target scope
//  contents or the whole target tree first, respectively.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TCQCScaleTmplsDlg
// PREFIX: dlg
// ---------------------------------------------------------------------------
class TScaleTmplsCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TScaleTmplsCfg();

        TScaleTmplsCfg(const TScaleTmplsCfg&) = default;
        TScaleTmplsCfg(TScaleTmplsCfg&&) = default;

        ~TScaleTmplsCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TScaleTmplsCfg& operator=(const TScaleTmplsCfg&) = default;
        TScaleTmplsCfg& operator=(TScaleTmplsCfg&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset();


        // -------------------------------------------------------------------
        //  Public data members
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bDoSubDirs;
        TVector<TSize>      m_colTarResList;
        TSize               m_szSrcRes;
        TString             m_strSrcScope;
        TString             m_strTarScope;


    protected :
        // -------------------------------------------------------------------
        //  Private, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&   strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&  strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TScaleTmplsCfg,TObject)
};




// ---------------------------------------------------------------------------
//  CLASS: TCQCScaleTmplsDlg
// PREFIX: dlg
// ---------------------------------------------------------------------------
class TCQCScaleTmplsDlg : public TDlgBox
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCScaleTmplsDlg();

        TCQCScaleTmplsDlg(const TCQCScaleTmplsDlg&) = delete;
        TCQCScaleTmplsDlg(TCQCScaleTmplsDlg&&) = delete;

        ~TCQCScaleTmplsDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCScaleTmplsDlg& operator=(const TCQCScaleTmplsDlg&) = delete;
        TCQCScaleTmplsDlg& operator=(TCQCScaleTmplsDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Run
        (
            const   TWindow&                wndOwner
        );

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        enum class EStatus
        {
            Processing
            , Success
            , Failure
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsUniqueTarRes
        (
            const   TSize&                  szCheck
        );

        tCIDLib::TBoolean bStopWorker
        (
            const   tCIDLib::TBoolean       bWaitOnly
        );

        tCIDLib::TBoolean bValidate();

        tCIDLib::TVoid CreatePath
        (
                    TDataSrvClient&         dsclToUse
            , const TString&                strPath
            , const tCIDLib::TBoolean       bImage
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::EExitCodes eScalingThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*
        );

        tCIDLib::TVoid LoadLastSettings();

        tCIDLib::TVoid LoadTemplate
        (
                    TDataSrvClient&         dsclToUse
            , const TString&                strPath
            ,       tCIDLib::TCard4&        c4CurSerialNum
            ,       tCIDLib::TKVPFList&     colMeta
            ,       TCQCIntfTemplate&       iwdgToFill
        );

        tCIDLib::TVoid ProcessScope
        (
                    TGraphDrawDev&          gdevTmp
            ,       TThread&                thrThis
            ,       TDataSrvClient&         dsclToUse
            , const TString&                strSrc
            , const TString&                strTar
        );

        tCIDLib::TVoid ScaleImage
        (
                    TDataSrvClient&         dsclToUse
            ,       TString&                pathSrc
            ,       TCQCIntfTemplate&       iwdgTar
            , const tCIDLib::TBoolean       bNeedRep
        );

        tCIDLib::TVoid ScaleImages
        (
                    TThread&                thrThis
            ,       TDataSrvClient&         dsclToUse
            ,       TCQCIntfTemplate&       iwdgTar
            , const tCIDLib::TBoolean       bNeedRep
        );

        tCIDLib::TVoid Select
        (
            const   tCIDLib::TBoolean       bSrc
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bCleanTar
        //      The user can indicate whether or not to clean the target directories
        //      first. This requires we re-process everything, but sometimes you want
        //      to do that in order to get rid of no longer used content. We leave this
        //      here for the processing thread.
        //
        //      This isnt' saved, they have to check it when desired.
        //
        //  m_cfgInfo
        //      Any previous settings are loaded into this object. During validation
        //      we update this data with currently entered info and use it during the
        //      processing. It's stored back out to the local data store upon
        //      sucessful generation.
        //
        //  m_colImgs
        //      We scale referenced images for each target resolution. To avoid doing
        //      it over and over, since they tend to be used many times. So we keep
        //      the source and target paths of each one we've done, so we can just
        //      look it up and update the image path.
        //
        //  m_colMsgQ
        //      A queue that the bgn thread can put status messages in, in order
        //      to update the GUI thread as to status.
        //
        //  m_colScopes
        //      Same as m_colImgs, but for the scope based image references. We
        //      definitely don't want to re-gen whole scopes of images, so we keep
        //      up with the ones we've done so far.
        //
        //  m_eStatus
        //      The background thread updates this as it goes, to let the GUI
        //      thread know when it's finished or failed.
        //
        //  m_f8XScale
        //  m_f8YScale
        //      The calculated horz/vertical scaling factors. This is set up for each
        //      target resolution as the bgn thread loops through them.
        //
        //  m_imgSrc
        //  m_imgTar
        //      Temporary images for downloading images and scaling to a new size.
        //
        //  m_iwdgCur
        //      The current template we are working on. Just to avoid having to
        //      allocate and delete one constantly.
        //
        //  m_mbufData
        //      A memory buffer for reading and writing templates. We don't use the helper
        //      in CQCIntfEng because it uses the cache and we don't want to risk that we
        //      get old content for this.
        //
        //  m_pwndXXX
        //      Typed pointers to the dialog widgets we interact with enough
        //      to want to avoid casting.
        //
        //  m_regxImgs
        //      During the search to update image references we use a regular
        //      epxression that lets us be independent of whether they have single
        //      or double slashes. Otherwise we could miss some. Some might also
        //      have mixed double and single. For each source path we search we
        //      create a new regular expression that will match it in any form.
        //
        //  m_strImgDir
        //      For each resolution we do, we scale the referenced images. They are
        //      put into a single directory under each per-res directory. This is
        //      the path we put them in.
        //
        //  m_strPrevVerMeta
        //      A string we use to store a previous version metadata value on the
        //      templates and images we scale.
        //
        //  m_strMsg
        //      For the timer, to pull messages out of the msg queue.
        //
        //  m_strMsgQPref
        //      For each target resolution round, the bgn thread sets up this prefix
        //      which will be appended to all of the queued up msgs.
        //
        //  m_strmImg
        //  m_strmThumb
        //      We keep a couple chunked memory streams around for reading and writing
        //      images.
        //
        //  m_szMinImg
        //      We have to be careful of scaling an image down to zero size. So we set this
        //      to 1x1 and do a TakeLarger to insure nothing gets that small.
        //
        //  m_thrWorker
        //      The worker thread we start up to do the actual work. It posts us
        //      status msgs to the queue and set the m_eStatus member to indicate
        //      its final status.
        //
        //  m_tmidUpdate
        //      We start a timer to grab messages queued by the bgn thread and display
        //      them.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bCleanTar;
        TScaleTmplsCfg          m_cfgInfo;
        tCIDLib::TStrHashSet    m_colScopes;
        tCIDLib::TKVHashSet     m_colImgs;
        TDeque<TString>         m_colMsgQ;
        EStatus                 m_eStatus;
        tCIDLib::TFloat8        m_f8XScale;
        tCIDLib::TFloat8        m_f8YScale;
        TPNGImage               m_imgSrc;
        TPNGImage               m_imgTar;
        TCQCIntfTemplate        m_iwdgCur;
        THeapBuf                m_mbufData;
        TPushButton*            m_pwndAddTarRes;
        TCheckBox*              m_pwndCleanTar;
        TPushButton*            m_pwndCloseButton;
        TPushButton*            m_pwndDelTarRes;
        TCheckBox*              m_pwndDoSubs;
        TPushButton*            m_pwndSelSrcScope;
        TPushButton*            m_pwndSelTarScope;
        TPushButton*            m_pwndScaleButton;
        TEntryField*            m_pwndSrcResX;
        TEntryField*            m_pwndSrcResY;
        TEntryField*            m_pwndSrcScope;
        TStaticText*            m_pwndStatus;
        TListBox*               m_pwndTarResList;
        TEntryField*            m_pwndTarScope;
        TRegEx                  m_regxImgs;
        TString                 m_strImgDir;
        const TString           m_strPrevVerMeta;
        TString                 m_strMsg;
        TString                 m_strMsgQPref;
        TBinMBufOutStream       m_strmImg;
        TBinMBufOutStream       m_strmThumb;
        const TSize             m_szMinImg;
        TThread                 m_thrWorker;
        tCIDCtrls::TTimerId     m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCScaleTmplsDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


