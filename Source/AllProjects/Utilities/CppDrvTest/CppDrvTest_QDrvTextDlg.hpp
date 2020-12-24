//
// FILE NAME: CppDrvTest_QDrvTextDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/25/2012
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
//  This is the header for the file file CppDrvTest_QDrvTextDlg.cpp file,
//  which implements a dialog that lets the user fake text query backdoor
//  commands to the driver and get back the text for evaluation.
//
//  We get the value id and data name strings that the query driver text
//  call takes, and send them off when the user pressed Send. We get the
//  return info and display it.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


class TCppDrvTestMainFrame;

// ---------------------------------------------------------------------------
//  CLASS: TQueryDrvTextDlg
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TQueryDrvTextDlg : public TDlgBox
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TQueryDrvTextDlg();

        ~TQueryDrvTextDlg();


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

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   override;


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
        //  m_pdcmdWait
        //      The command we queued up asynchronously on the driver, which
        //      our timer callback will check until it's completed.
        //
        //  m_pwndXXX
        //      Typed pointers to widgets that we want to interact with enough
        //      to want to avoid casting them all the time.
        // -------------------------------------------------------------------
        TCQCServerBase::TDrvCmd*    m_pdcmdWait;
        TPushButton*                m_pwndClose;
        TEntryField*                m_pwndDataName;
        TEntryField*                m_pwndValId;
        TStaticMultiText*           m_pwndOutput;
        TPushButton*                m_pwndSend;
        tCIDCtrls::TTimerId         m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TQueryDrvTextDlg,TDlgBox)
};


