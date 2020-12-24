//
// FILE NAME: CQCIGKit_CmdReplaceDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/07/2010
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
//  This is the header for the CQCIGKit_CmdReplaceDlg.cpp file, which
//  implements a dialog that gets the info required to doa  search and replace
//  operation in the action command editor.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TActCmdReplaceDlg
// PREFIX: dlgb
// ---------------------------------------------------------------------------
class TActCmdReplaceDlg : public TDlgBox
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TActCmdReplaceDlg();

        ~TActCmdReplaceDlg();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
                    TWindow&                wndOwner
            ,       tCIDLib::TBoolean&      bRegEx
            ,       tCIDLib::TBoolean&      bFullMatch
            ,       tCIDLib::TBoolean&      bThisEvent
            ,       TString&                strFindStr
            ,       TString&                strRepStr
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bRegEx
        //  m_bFullMatch
        //  m_bThisEvent
        //      Store the incoming values till we can get them loaded and then
        //      to get them back to teh caller as well.
        //
        //  m_pwndXXX
        //      We store a few typed pointers to our widgets for convenience.
        //      We don't own them, we just reference them.
        //
        //  m_strFindStr
        //  m_strRepStr
        //      Storage for the incoming find info until we can get it loaded
        //      and to get the entered values back out to be reused next time.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bRegEx;
        tCIDLib::TBoolean       m_bFullMatch;
        tCIDLib::TBoolean       m_bThisEvent;
        TRadioButton*           m_pwndAllEvents;
        TPushButton*            m_pwndCancel;
        TEntryField*            m_pwndFindStr;
        TCheckBox*              m_pwndFullMatch;
        TCheckBox*              m_pwndRegEx;
        TPushButton*            m_pwndReplace;
        TEntryField*            m_pwndRepStr;
        TRadioButton*           m_pwndThisEvent;
        TString                 m_strFindStr;
        TString                 m_strRepStr;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TActCmdReplaceDlg,TDlgBox)
};


#pragma CIDLIB_POPPACK



