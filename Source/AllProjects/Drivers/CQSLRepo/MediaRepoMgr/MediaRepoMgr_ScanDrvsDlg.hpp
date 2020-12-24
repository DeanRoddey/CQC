//
// FILE NAME: MediaRepoMgr_ScanDrvsDlg.hpp
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
//  This is the header for MediaRepoMgr_ScanDrvsDlg.cpp, which implements a
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
//   CLASS: TScanDrvsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TScanDrvsDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TScanDrvsDlg();

        TScanDrvsDlg(const TScanDrvsDlg&) = delete;

        ~TScanDrvsDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TScanDrvsDlg& operator=(const TScanDrvsDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TString&                strVolToFill
            ,       TKrnlRemMedia::EMediaTypes& eMType
            ,       tCIDLib::TCard4&        c4ItemCnts
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private dat atypes
        // -------------------------------------------------------------------
        typedef TFundVector<TKrnlRemMedia::EMediaTypes>     TMTypeList;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eComboHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Selected
        //      This is used to store the selection index until it can be
        //      used to fill in the caller's parms.
        //
        //  m_colVolumes
        //      During the scan for available media, we load up this list with
        //      the volumes.
        //
        //  m_fcolItemCnts
        //      For music media we store the track count from the TOC for
        //      each volume we put into the list, so that we can tell the
        //      caller this info. We return zero if not CD media.
        //
        //  m_fcolTypes
        //      We store the media types of each volume we load into the
        //      list. This is so we can return the type, and so that we can
        //      show the type when they select a drive.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls. We don't own them, we
        //      just point to them.
        //
        //  m_strTypeXXX
        //      The message that we show to indicate the media type of the
        //      selected drive.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4Selected;
        tCIDLib::TStrList   m_colVolumes;
        tCIDLib::TCardList  m_fcolItemCnts;
        TMTypeList          m_fcolTypes;
        TPushButton*        m_pwndCancel;
        TComboBox*          m_pwndList;
        TPushButton*        m_pwndProcess;
        const TString       m_strBDType;
        const TString       m_strCDType;
        const TString       m_strDVDType;
        const TString       m_strUnknown;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TScanDrvsDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK

