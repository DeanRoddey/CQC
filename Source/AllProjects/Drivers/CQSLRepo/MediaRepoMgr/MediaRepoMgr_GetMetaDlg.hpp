//
// FILE NAME: MediaRepoMgr_GetMetaDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/22/2006
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
//  This is the header for MediaRepoMgr_GetMetaDlg.cpp, which implements a
//  dialog that is popped up when we get a media change event and see new
//  media arriving in a removable drive. We pop up a dialog that asks the user
//  what they want to do.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGetMetaDataDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TGetMetaDataDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGetMetaDataDlg();

        TGetMetaDataDlg(const TGetMetaDataDlg&) = delete;

        ~TGetMetaDataDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGetMetaDataDlg& operator=(const TGetMetaDataDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            , const TString&                strVolume
            ,       TMediaCollect&          mcolToFill
            ,       tCQCMedia::TItemList&   colItems
            ,       tCIDLib::TStrList&      colCatNames
            ,       TJPEGImage&             imgArt
            , const TKrnlRemMedia::TCDTOCInfo& TOCData
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        enum class EStatus
        {
            WaitPlayer
            , WaitScrape
            , Done
            , Cancel
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetHTMLPage
        (
                    THTTPClient&            httpcGet
            , const TURL&                   urlSrc
            ,       tCIDLib::TCard4&        c4Len
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::EExitCodes eScrapeThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid WaitThreadEnd();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bGotXXX
        //      Flags we use during the scraping phase, to avoid scraping for
        //      stuff we've already gotten.
        //
        //  m_enctEnd
        //      The point at which we give up and timeout.
        //
        //  m_eStatus
        //      This is used by the timer to keep up with where we are and
        //      want it should be doing.
        //
        //  m_pcolCatNames
        //      A pointer to the caller's category name collection that we'll
        //      put the cat names into.
        //
        //  m_pcolItems
        //      A pointer to the caller's item collection that we'll put the
        //      track items into.
        //
        //  m_pimgToFill
        //      A pointer to the caller's cover art image object. We fill it
        //      with the cover art for the selected item.
        //
        //  m_pmcolToFill
        //      A pointer to the caller's media collection object. We fill it
        //      with the gotten information.
        //
        //  m_pwndXXX
        //      Typed pointers to children that we want to interact with
        //      directly.
        //
        //  m_strBody
        //      This is used to transcode the HTML text into for parsing.
        //
        //  m_strInfoURL
        //      The WMP stuff can give us back a 'more info' URL, which we'll
        //      hit during the scraping phase.
        //
        //  m_strVolume
        //      The incoming volume to scan is stored here for later access.
        //
        //  m_thrScrape
        //      The WMP part of the gathering process occurs in the GUI thread.
        //      Then we do some screen scraping and that needs to happen in
        //      a background thread so that it doesn't hang up the GUI and so
        //      it can be cancelled via the Cancel button.
        //
        //  m_tmidUpdate
        //      We start a timer monitor the info left for us by the bgn thread.
        //
        //  m_TOCData
        //      If it was CD media, we'll get the CD table of contents info
        //      which we save away in case we need it.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bGotImage;
        tCIDLib::TEncodedTime       m_enctEnd;
        EStatus                     m_eStatus;
        tCIDLib::TStrList*          m_pcolCatNames;
        tCQCMedia::TItemList*       m_pcolItems;
        TJPEGImage*                 m_pimgToFill;
        TMediaCollect*              m_pmcolToFill;
        TPushButton*                m_pwndCancel;
        TString                     m_strInfoURL;
        TString                     m_strURL;
        TString                     m_strVolume;
        TThread                     m_thrScrape;
        tCIDCtrls::TTimerId         m_tmidUpdate;
        TKrnlRemMedia::TCDTOCInfo   m_TOCData;


        // -------------------------------------------------------------------
        //  Private, static data members
        //
        //  s_pwmpmsData
        //      For now, we use a fixed WMP metadata source. Later we will
        //      change this so that the manager passes in one or more that
        //      have been configured.
        // -------------------------------------------------------------------
        static TCQCWMPMetaSrc*  s_pwmpmsData;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TGetMetaDataDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK

