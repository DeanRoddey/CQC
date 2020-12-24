//
// FILE NAME: MediaRepoMgr_EdDescrDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/03/2006
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
//  This is the header for the MediaRepoMgr_EdDescrDlg.cpp file, which
//  implements a small dialog box that allows the user to edit the title
//  description text.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TMediaRepoEdDescrDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TMediaRepoEdDescrDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMediaRepoEdDescrDlg();

        TMediaRepoEdDescrDlg(const TMediaRepoEdDescrDlg&) = delete;

        ~TMediaRepoEdDescrDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TMediaRepoEdDescrDlg& operator=(const TMediaRepoEdDescrDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       TString&                strText
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
        //  m_pwndXXXX
        //      Some typed pointers to our widgets. We don't own them, we just
        //      want to avoid lots of downcasting when we use them.
        //
        //  m_pstrText
        //      A pointer to the caller's incoming string, so that we can load
        //      the text up and store it back if they save.
        // -------------------------------------------------------------------
        TString*        m_pstrText;
        TPushButton*    m_pwndCancel;
        TMultiEdit*     m_pwndText;
        TPushButton*    m_pwndSave;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TMediaRepoEdDescrDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


