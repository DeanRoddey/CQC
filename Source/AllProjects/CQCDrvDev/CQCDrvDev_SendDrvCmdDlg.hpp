//
// FILE NAME: CQCDrvDev_SendDrvCmdDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/01/2004
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
//  This is the header for the file file CQCDrvDev_SendDrvCmdDlg.cpp file,
//  which implements a dialog that lets the user cause calls to the ClientCmd()
//  method of the CML driver. This is a 'backdoor' command that allows clients
//  to talk to a driver outside of the standard field API.
//
//  We get a numeric command id and a string of text to send with the command.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


class TCQCDrvDevFrameWnd;

// ---------------------------------------------------------------------------
//  CLASS: TCQCSendDrvCmdDlg
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TCQCSendDrvCmdDlg : public TDlgBox
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCSendDrvCmdDlg();

        ~TCQCSendDrvCmdDlg();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
            const   TWindow&                wndOwner
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
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

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidUpdate
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pdcmdWait
        //      The command we queued up asynchronously on the driver, which
        //      our timer callback will check until it's completed.
        //
        //  m_pwndXXX
        //      Typed pointers to widgets that we want to interact with enough
        //      to want to avoid casting them all the time.
        //
        //  m_tmidUpdate
        //      We start a timer to wait for the sent command to complete.
        // -------------------------------------------------------------------
        TCQCServerBase::TDrvCmd*    m_pdcmdWait;
        TPushButton*                m_pwndClose;
        TEntryField*                m_pwndCmdId;
        TMultiEdit*                 m_pwndData;
        TPushButton*                m_pwndSend;
        tCIDCtrls::TTimerId         m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSendDrvCmdDlg,TDlgBox)
};

