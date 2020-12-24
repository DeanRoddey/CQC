//
// FILE NAME: MediaRepoMgr_UploadDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/07/2006
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
//  This is the header for MediaRepoMgr_UploadDlg.cpp, which implements a
//  dialog that is popped up while we upload newly ripped music data to
//  the server. We spin off a thread that does the upload and keeps us
//  informed of the progress.
//
//  For movies, there's just meta data, so that's just done at the point where
//  we get the metadata because it can be done quickly. This could take a
//  few minutes in some cases.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TUploadDataDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TUploadDataDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TUploadDataDlg();

        TUploadDataDlg(const TUploadDataDlg&) = delete;

        ~TUploadDataDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TUploadDataDlg& operator=(const TUploadDataDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       TString&                strError
            , const TMediaDB&               mdbChanges
            , const TString&                strLeaseId
            , const TString&                strRepoMoniker
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCancelRequest();

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidUpdate
        );


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        enum class EStatus
        {
            Initializing
            , SendingDB
            , SendingImgs
            , SendingItems
            , Cleanup
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

        tCIDLib::EExitCodes eUploadThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid SendPacket
        (
                    TClientStreamSocket&    sockSrv
            , const tCIDLib::TCard4         c4Cmd
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4DBytes
            , const tCIDLib::TCard4         c4Parm1 = 0
            , const tCIDLib::TCard4         c4Parm2 = 0
            , const tCIDLib::TCard4         c4Parm3 = 0
        );

        tCIDLib::TVoid WaitPacket
        (
                    TClientStreamSocket&    sockSrv
            ,       tCQSLRepoUpl::THdr&     hdrToUse
            ,       TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Expected
        );

        tCIDLib::TVoid UploadMediaFile
        (
                    TClientStreamSocket&    sockTar
            , const TPathStr&               pathFilePath
        );

        tCIDLib::TVoid UploadBuffer
        (
                    TClientStreamSocket&    sockTar
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Size
            , const tCIDLib::TCard4         c4FileType
            , const tCQCMedia::EMediaTypes  eMType
            , const tCIDLib::TCard2         c2Id
        );

        tCIDLib::TVoid WaitThreadEnd();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bCancel
        //      This is set by our cancel button to make the upload thread
        //      given up.
        //
        //  m_c4Cookie
        //      The server gives the caller a cookie and he passes it to us.
        //      We have to pass it back in to the initial upload start packet
        //      we send, to verify that we are the one who started the upload
        //      process.
        //
        //  m_c4CurFileNum
        //      When doing the file lists, this is the current number of the one in
        //      the list we are uploading, for use by the 'x of y' display.
        //
        //  m_c4DBSize
        //      The number of bytes in the m_mbufDBData buffer.
        //
        //  m_c4FileCnt
        //      When doing the lists of files, we set this to the number of them, so
        //      that the 'x of y' display can use it.
        //
        //  m_c4LastFileNum
        //  m_c4LastPercent
        //      These are used by the window timer, to remember the last values we
        //      saw the thread set, so that we know when it changes, and that we need
        //      to update the dipslay.
        //
        //  m_colImgs
        //      On entry we go through the database and find all of the images that
        //      have had image data cached on them. These are the ones we need to
        //      upload.
        //
        //  m_colRipFiles
        //      On entry we find all of the ripped files we need to upload and get
        //      them into this list. We have to report the number of files we are
        //      going to upload, when we ask the server to let us start it.
        //
        //  m_eRepStatus
        //  m_eStatus
        //      The worker thread updates m_eStatus to indicate it's status.
        //      The timer uses m_eRepStatus to know if the thread has reported
        //      a new status, so that it only updates the display on a change.
        //
        //  m_ipepSrv
        //      The caller provides us with the end point to use to connect
        //      to the server for the upload.
        //
        //  m_mbufChunk
        //      This is a buffer used to chunk files as we upload. It's set up for
        //      the chunk size and will send up to this much per round.
        //
        //  m_mbufDBData
        //      The flattened media database data. The size of the data is in
        //      m_c4DBSize. We don't bother to compress it for this upload, since
        //      we are doing this via a separate socket.
        //
        //  m_pwndXXX
        //      Typed pointers to important child controls, so that we can
        //      interact with them easily.
        //
        //  m_strCurFileMsg
        //      We have an 'x of y' status message, that we update each time
        //      the file number changes. To avoid loading the resource over
        //      and over, we load it once and reuse it.
        //
        //  m_strError
        //      The caller provides us a string to put an error message back
        //      into if we fail.
        //
        //  m_strTmpFmt
        //      Just a temporary string for formatting purposes in the timer
        //      callback.
        //
        //  m_tmidUpdate
        //      We start up a timer to monitor the progress of the bgn thread.
        //
        //  m_thrUpload
        //      The thread we start up to do the actual work. It leaves status
        //      msgs in m_eStatus and the timer handler loads that info into
        //      the status display window.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bCancel;
        tCIDLib::TCard4         m_c4Cookie;
        tCIDLib::TCard4         m_c4CurFileNum;
        tCIDLib::TCard4         m_c4CurPercent;
        tCIDLib::TCard4         m_c4DBSize;
        tCIDLib::TCard4         m_c4FileCnt;
        tCIDLib::TCard4         m_c4LastFileNum;
        tCIDLib::TCard4         m_c4LastPercent;
        tCQCMedia::TImgIdList   m_colImgs;
        tCIDLib::TStrList       m_colRipFiles;
        EStatus                 m_eRepStatus;
        EStatus                 m_eStatus;
        tCIDLib::TIPPortNum     m_ippnHost;
        THeapBuf                m_mbufChunk;
        THeapBuf                m_mbufDBData;
        TString*                m_pstrError;
        TPushButton*            m_pwndCancel;
        TProgressBar*           m_pwndProgress;
        TStaticText*            m_pwndStatus;
        TString                 m_strCurFileMsg;
        TString                 m_strHost;
        TString                 m_strTmpFmt;
        tCIDCtrls::TTimerId     m_tmidUpdate;
        TThread                 m_thrUpload;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TUploadDataDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



