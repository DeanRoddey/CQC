//
// FILE NAME: CQCIntfEd_EdTrigEvsDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/26/2016
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
//  This is the header for the CQCIntfEd_EdTrigEvsDlg.cpp file, which allows the user
//  to edit the triggered events for the passed template.
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
//  CLASS: TEdTrigEvsDlg
// PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdTrigEvsDlg : public TDlgBox
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TEdTrigEvsDlg();

        TEdTrigEvsDlg(const TEdTrigEvsDlg&) = delete;

        ~TEdTrigEvsDlg();


        // -------------------------------------------------------------------
        //  Public operators=
        // -------------------------------------------------------------------
        TEdTrigEvsDlg& operator=(const TEdTrigEvsDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndParent
            ,       TCQCIntfTemplate* const piwdgEdit
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

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditEvent
        (
            const   tCIDLib::TCard4         c4Index
        );

        tCIDLib::TVoid LoadEventList();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_piwdgOrg
        //  m_piwdgWorking
        //      This is a pointer to the template widget we are working on. We also make
        //      a copy that we can restore if they don't commit.
        //
        //  m_pwndXXX
        //      We get typed pointers to some of our child widgets, for convenience. We
        //      don't own them.
        // -------------------------------------------------------------------
        TCQCIntfTemplate        m_iwdgOrg;
        TCQCIntfTemplate*       m_piwdgWorking;
        TPushButton*            m_pwndAdd;
        TPushButton*            m_pwndCancel;
        TPushButton*            m_pwndRemove;
        TPushButton*            m_pwndRemoveAll;
        TPushButton*            m_pwndEdit;
        TListBox*               m_pwndList;
        TPushButton*            m_pwndSave;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdTrigEvsDlg, TDlgBox)
};

#pragma CIDLIB_POPPACK
