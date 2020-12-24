//
// FILE NAME: CQCIGKit_EdTimeToksDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/11/2016
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
//  This is the header for the CQCIGKit_EdTimeToksDlg.cpp file. It provides a helper dialog
//  to build up a string of text that includes time tokens. The time system defines a list
//  of tokens that represent the various aspects of a time stamp, and those can be replaced
//  with their represented value to display various formatted times.
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
//   CLASS: TEdTimeToksDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdTimeToksDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdTimeToksDlg();

        TEdTimeToksDlg(const TEdTimeToksDlg&) = delete;

        ~TEdTimeToksDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TEdTimeToksDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       TString&                strToEdit
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eEFHandler
        (
                    TEFChangeInfo&          wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid InsertToken
        (
            const   tCIDLib::TCard4         c4Index
        );

        tCIDLib::TVoid ShowPrefabMenu();

        tCIDLib::TVoid UpdateSample();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndXXX
        //      We look up some of the widgets that we want to directly interact with in a
        //      typed way.
        //
        //  m_strToEdit
        //      We make a copy of the incoming value and put the selected value back here
        //      to get back to the caller.
        // -------------------------------------------------------------------
        TPushButton*        m_pwndCancel;
        TPushButton*        m_pwndInsert;
        TMultiColListBox*   m_pwndList;
        TPushButton*        m_pwndPrefab;
        TEntryField*        m_pwndValue;
        TStaticText*        m_pwndSample;
        TPushButton*        m_pwndSave;
        TString             m_strToEdit;


        // -------------------------------------------------------------------
        //  Private, static data members
        //
        //  s_areaLast
        //      The last area at which we were positioned by the user. Initially it is
        //      empty.
        // -------------------------------------------------------------------
        static TArea        s_areaLast;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdTimeToksDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK

