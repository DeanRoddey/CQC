//
// FILE NAME: ZWaveUSB3C_SelUTypeDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/18/2018
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
//  This is the header for a small dialog that lets the user select the device
//  info file for a unit. We try to auto-id them, but can't always and many of
//  course we just won't have pre-fab handlers for so the user needs to select a
//  generic one.
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
//   CLASS: TZWUSB3SelUTypeDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWUSB3SelUTypeDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWUSB3SelUTypeDlg();

        TZWUSB3SelUTypeDlg(const TZWUSB3SelUTypeDlg&) = delete;

        ~TZWUSB3SelUTypeDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWUSB3SelUTypeDlg& operator=(const TZWUSB3SelUTypeDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
                    TZWaveUSB3CWnd&         wndDriver
            ,       tCIDLib::TCard8&        c8ManIds
            , const TString&                strUnitName
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
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c8ManIds
        //      ON the way in holds any previous value so we can pre-select those.
        //      On the way out, if accepted, holds the chosen value until we can get
        //      it into the caller's parameter.
        //
        //  m_pwndXXX
        //      Some controls we need to interact with, so we get typed pointers
        //
        //  m_pwndDriver
        //      We get a pointer to the driver so that we can send msgs. It's also
        //      our owner.
        //
        //  m_strTitle
        //      The original title text. We put the unit name into the title text, so
        //      we don't want to let popups pick that up automatically.
        //
        //  m_strUnitName
        //      The caller provides the name of the unit so that we can put it into
        //      the title bar. Otherwise we don't need to access the unit itself,
        //      so we don't get that.
        // -------------------------------------------------------------------
        tCIDLib::TCard8     m_c8ManIds;
        TPushButton*        m_pwndAccept;
        TPushButton*        m_pwndCancel;
        TZWaveUSB3CWnd*     m_pwndDriver;
        TStaticMultiText*   m_pwndInstruct;
        TListBox*           m_pwndMakes;
        TMultiColListBox*   m_pwndModels;
        TString             m_strTitle;
        TString             m_strUnitName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWUSB3SelUTypeDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



