//
// FILE NAME: CQCIGKit_GetCmdDelOpt_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/23/2006
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
//  This is the header for the CQCGKit_GetCmdDelOptDlg.cpp file, which implements a
//  small dailog box that is presented to the user when they press the delete button in
//  the action editing dialog box and the current cmd step is a conditional cmd. This
//  gives them some options as to how to handle that deletion.
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
//   CLASS: TGetCmdDelOptDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TGetCmdDelOptDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGetCmdDelOptDlg();

        TGetCmdDelOptDlg(const TGetCmdDelOptDlg&) = delete;

        ~TGetCmdDelOptDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGetCmdDelOptDlg& operator=(const TGetCmdDelOptDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCIGKit::ECmdDelOpts eRunDlg
        (
            const   TWindow&                wndOwner
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


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eRet
        //      To store the selection until we can return it.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we
        //      need to interact with on a typed basis.
        // -------------------------------------------------------------------
        tCQCIGKit::ECmdDelOpts  m_eRet;
        TPushButton*            m_pwndCancel;
        TPushButton*            m_pwndDelete;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TGetCmdDelOptDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


