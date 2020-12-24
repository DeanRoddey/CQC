//
// FILE NAME: CQCIGKit_SelFldDlg_.hpp
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
//  This is the header for the CQCIGKit_SelFldDlgDlg.cpp file, which allows
//  the user to select a driver/field value.
//
//  We let the lists sort, which is no a problem since we are dealing purely with
//  the text in the lists. No need to go back to the original data.
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
//   CLASS: TCQCSelFldDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TCQCSelFldDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSelFldDlg();

        ~TCQCSelFldDlg();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       TCQCFldCache&           cfcFldInfo
            ,       TString&                strDriver
            ,       TString&                strField
        );

        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       TCQCFldCache&           cfcFldInfo
            ,       TString&                strDriver
            ,       TString&                strField
            ,       tCIDLib::TBoolean&      bByRef
            , const tCIDLib::TBoolean       bNameOnly
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TCQCSelFldDlg(const TCQCSelFldDlg&);
        tCIDLib::TVoid operator=(const TCQCSelFldDlg&);


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

        tCIDLib::TVoid LoadFldsFor
        (
            const   TString&                strMon
            , const TString&                strFldSel
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bNameOnly
        //      When this is being used to get a field for an action parameter, we want
        //      to offer them the option of inserting the name or the dereferenced value.
        //      If not, we hide that check box and indicate by name.
        //
        //  m_flddRet
        //      We put the field definition of the selected field into this until we
        //      can get it back to the caller.
        //
        //  m_pcfcFldInfo
        //      The caller passes us an already set up field cache that has been
        //      set up to display the drivers/fields he wants to be available for
        //      selection.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to
        //      interact with on a typed basis.
        //
        //  m_strDriver
        //  m_strField
        //      We store the incoming values (if any) for initial selection, and
        //      we store the user selection here until we can get it back into the
        //      caller's parms.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bNameOnly;
        TCQCFldDef          m_flddRet;
        TCQCFldCache*       m_pcfcFldInfo;
        TPushButton*        m_pwndByName;
        TPushButton*        m_pwndByValue;
        TPushButton*        m_pwndCancel;
        TMultiColListBox*   m_pwndDrivers;
        TMultiColListBox*   m_pwndFields;
        TString             m_strDriver;
        TString             m_strField;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSelFldDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



