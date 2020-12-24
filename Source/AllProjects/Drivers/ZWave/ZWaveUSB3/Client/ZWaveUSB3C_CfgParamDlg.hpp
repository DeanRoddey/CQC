//
// FILE NAME: ZWaveUSB3C_CfgParamDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/17/2018
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
//  This is the header for a small dialog that lets the user manually manage the
//  configuration parameters of a unit.
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
//   CLASS: TZWUSB3CfgParamDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWUSB3CfgParamDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWUSB3CfgParamDlg();

        TZWUSB3CfgParamDlg(const TZWUSB3CfgParamDlg&) = delete;

        ~TZWUSB3CfgParamDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWUSB3CfgParamDlg& operator=(const TZWUSB3CfgParamDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
                    TZWaveUSB3CWnd&         wndDriver
            , const TZWUnitInfo&            unitiTar
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
        tCIDLib::TBoolean bCheckAwake() const;

        tCIDLib::TCard4 c4GetBytes() const;

        tCIDLib::TBoolean bGetParamNum
        (
                    tCIDLib::TCard1&        c1ToFill
        )   const;

        tCIDLib::TBoolean bGetValue
        (
                    tCIDLib::TInt4&         i4ToFill
        )   const;

        tCIDLib::ERadices eGetRadix() const;

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid QueryValue();

        tCIDLib::TVoid SetValue();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eRadix
        //      The last radix selected in the radix combo box. We need to store it
        //      separately because, if the user selects a new one, we have to convert
        //      the current value back to a number using the old radix, then convert
        //      to the new.
        //
        //  m_punitiTar
        //      A pointer to the unit we are managing parameters for. We don't need to
        //      modify it.
        //
        //  m_pwndXXX
        //      Some controls we need to interact with, so we get typed pointers
        //
        //  m_pwndDriver
        //      We get a pointer to the driver so that we can send msgs. It's also
        //      our owner.
        //
        //  m_strTitleText
        //      The original title text. We put the unit name into the title text, so
        //      we don't want to let popups pick that up automatically.
        // -------------------------------------------------------------------
        tCIDLib::ERadices   m_eRadix;
        const TZWUnitInfo*  m_punitiTar;
        TPushButton*        m_pwndClose;
        TZWaveUSB3CWnd*     m_pwndDriver;
        TComboBox*          m_pwndBytes;
        TComboBox*          m_pwndParamNum;
        TCheckBox*          m_pwndIsAwake;
        TPushButton*        m_pwndQueryValue;
        TComboBox*          m_pwndRadix;
        TPushButton*        m_pwndSetValue;
        TEntryField*        m_pwndValue;
        TString             m_strTitleText;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWUSB3CfgParamDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



