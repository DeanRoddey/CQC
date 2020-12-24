//
// FILE NAME: MediaRepoMgr_EdCastDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/23/2006
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
//  This is the header for MediaRepoMgr_EdCastDlg.cpp, which implements a
//  small dialog for editing the casts of movies. It's called from the main
//  collection metadata editing dialog.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TEdCastDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdCastDlg : public TDlgBox, public MIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdCastDlg();

        TEdCastDlg(const TEdCastDlg&) = delete;

        ~TEdCastDlg();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TEdCastDlg& operator=(const TEdCastDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       TString&                strCast
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
        //  m_pwndXXXX
        //      Typed pointers to some of our children.
        //
        //  m_strCast
        //      The incoming cast is copied here for loading up into the list
        //      box.
        // -------------------------------------------------------------------
        TPushButton*        m_pwndAdd;
        TPushButton*        m_pwndCancel;
        TPushButton*        m_pwndDelete;
        TMultiColListBox*   m_pwndList;
        TPushButton*        m_pwndMoveDn;
        TPushButton*        m_pwndMoveUp;
        TPushButton*        m_pwndSave;
        TString             m_strCast;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdCastDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



