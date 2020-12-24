//
// FILE NAME: MediaRepoMgr_RipCDDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/04/2006
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
//  This is the header for MediaRepoMgr_RipCDDlg.cpp, which implements a
//  dialog that kicks off a CD rip, displays the progress, and allows the
//  user to cancel it if desired.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

class TMainFrameWnd;

// ---------------------------------------------------------------------------
//   CLASS: TRipCDDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TRipCDDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TRipCDDlg();

        TRipCDDlg(const TRipCDDlg&) = delete;

        ~TRipCDDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TRipCDDlg& operator=(const TRipCDDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
                    TMainFrameWnd&          wndOwner
            , const TString&                strVolume
            ,       TString&                strError
            ,       TMediaDB&               mdbEdit
            , const tCQCMedia::EMediaTypes  eMType
            , const tCIDLib::TCard2         c2ColId
            , const TString&                strCatName
            ,       tCIDLib::TStrList&      colFiles
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCancelRequest() override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidUpdate
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        enum class EStatus
        {
            WaitStart
            , Initializing
            , Start
            , Working
            , Failed
            , Cancelled
            , Done
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::EExitCodes eWorkerThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid WaitThreadEnd();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bBreak
        //      We point the ripper at this, and the worker thread watches it
        //      as well in it's outer loop. So when the user pressed cancel,
        //      we set this to force the thread down.
        //
        //  m_c4CurItem
        //  m_c4ItemCnt
        //      The worker thread keeps the current track updated with the
        //      track number it is currently ripping. And it sets the track
        //      count during the initialization phase, based on info it got
        //      from the TOC.
        //
        //  m_c4LastPercent
        //  m_c4LastItem
        //      We pull percent done (per track) out of the ripper object as
        //      it rips. This is used to keep up with the last percent we
        //      saw, so that we only have to update it when it's changed. And
        //      we need to keep with the last track number we saw, again so
        //      that we only need to update when it changes.
        //
        //  m_daerWorker
        //      The DAE ripper object that our worker thread uses to do the
        //      ripping work.
        //
        //  m_eLastStatus
        //  m_eStatus
        //      The ripping thread leaves status info here. We have one for
        //      him to update, and one for us to keep the last value we saw,
        //      so that we know it's changed.
        //
        //  m_mcolCD
        //      A copy of the incoming parent CD collection, so we can get metadata
        //      out of it to embed in the ripped files.
        //
        //  m_mrsRip
        //      We get a copy of the repo settings, so we can access codec and other
        //      rip oriented settings.
        //
        //  m_pmdbEdit
        //      A pointer to the caller's database. The passed collection must be
        //      in the database, as well as all its items.
        //
        //  m_pcolFiles
        //      A pointer to the caller's vector of strings. We fill this with the
        //      paths to the ripped files, in the same order as the incoming items.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our child controls we want to
        //      interact with in a type specific way. We don't own them.
        //
        //  m_strCatName
        //      The caller can pass us a primary category to embed into the
        //      ripped files.
        //
        //  m_strCurItemMsg
        //      We have an 'x of y' status message, that we update each time
        //      the trck changes. To avoid loading the resource over and over,
        //      we load it once and reuse it.
        //
        //  m_strTmpFmt
        //      Used in the timer for temporary formatting purposes.
        //
        //  m_strVolume
        //      Indicates the volume that we are to rip from.
        //
        //  m_tmidUpdate
        //      We start a timer to monitor the bgn thread.
        //
        //  m_thrWorker
        //      The thread we start up to do the actual work.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bBreak;
        tCIDLib::TCard4         m_c4CurItem;
        tCIDLib::TCard4         m_c4LastPercent;
        tCIDLib::TCard4         m_c4LastItem;
        tCIDLib::TCard4         m_c4ItemCnt;
        TCIDDAERipper           m_daerWorker;
        EStatus                 m_eLastStatus;
        EStatus                 m_eStatus;
        TMediaRepoSettings      m_mrsRip;
        tCIDLib::TStrList*      m_pcolFiles;
        TMediaDB*               m_pmdbEdit;
        TMediaCollect           m_mcolCD;
        TString*                m_pstrError;
        TPushButton*            m_pwndCancel;
        TStaticText*            m_pwndCurName;
        TStaticText*            m_pwndCurItem;
        TProgressBar*           m_pwndProgress;
        TString                 m_strCatName;
        TString                 m_strCurItemMsg;
        TString                 m_strTmpFmt;
        TString                 m_strVolume;
        tCIDCtrls::TTimerId     m_tmidUpdate;
        TThread                 m_thrWorker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TRipCDDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



