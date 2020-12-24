//
// FILE NAME: CQCIntfEd_UserLimitsDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/20/2016
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
//  This is the header for the CQCIntfEd_UserLimitsDlg.cpp file, which is used to set
//  per-user level limits, based on a field or variable's range. The viewer will then limit
//  the widget's controllable range based on the current user's level.
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
//   CLASS: TUserLimitsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TUserLimitsDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TUserLimitsDlg();

        TUserLimitsDlg(const TUserLimitsDlg&) = delete;

        ~TUserLimitsDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TUserLimitsDlg& operator=(const TUserLimitsDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       tCQCIntfEng::TUserLims& fcolVals
            , const tCIDLib::TInt4          i4MinLimit
            , const tCIDLib::TInt4          i4MaxLimit
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate() const;

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eListHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDCtrls::EEvResponses eSliderHandler
        (
                    TSliderChangeInfo&      wnotEvent
        );

        tCIDLib::TVoid SetLimitSliders();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fcolOrg
        //  m_fcolVals
        //      We store the incoming values here, and a copy so that we can check for
        //      changes. It has one entry per user level. In each one, the low word is
        //      the minimum value and the high word is the max. They are Int2 values. The
        //      changes are put into the Vals one until we can get them back to the user.
        //
        //  m_i4MinLimit
        //  m_i4MaxLimit
        //      The absolute limits that we can allow, so we store the incoming values until
        //      we can use them to set up the sliders.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to interact
        //      with on a typed basis.
        // -------------------------------------------------------------------
        tCQCIntfEng::TUserLims  m_fcolOrg;
        tCQCIntfEng::TUserLims  m_fcolVals;
        tCIDLib::TInt4          m_i4MinLimit;
        tCIDLib::TInt4          m_i4MaxLimit;
        TPushButton*            m_pwndCancelButton;
        TStaticText*            m_pwndDispMax;
        TStaticText*            m_pwndDispMin;
        TComboBox*              m_pwndRoles;
        TPushButton*            m_pwndResetButton;
        TSlider*                m_pwndSetMax;
        TSlider*                m_pwndSetMin;
        TPushButton*            m_pwndSaveButton;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TUserLimitsDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK
