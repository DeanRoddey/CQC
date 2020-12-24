//
// FILE NAME: MediaRepoMgr_SelRepoDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/17/2006
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
//  This is the header for the MediaRepoMGr_SelRepoDlg.cpp file, which
//  implements a small dialog box that is used to allow the user to select a
//  media repository to manage. If they select one, we try to get a lease on
//  the driver, to query repo settings, and then download the repo's current
//  metadata database. If all that does well, we have selected a new repo and
//  have the data to edit, which we store on the facility class for everyone
//  to have access to.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TMediaRepoSelRepoDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TMediaRepoSelRepoDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMediaRepoSelRepoDlg();

        TMediaRepoSelRepoDlg(const TMediaRepoSelRepoDlg&) = delete;

        ~TMediaRepoSelRepoDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TMediaRepoSelRepoDlg& operator=(const TMediaRepoSelRepoDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strLeaseId
            ,       tCQCMedia::EMediaTypes& eInitType
            ,       TMediaDB&               mdbToFill
            ,       TString&                strRepoMoniker
            ,       TString&                strRepoPath
            ,       TString&                strDBSerialNum
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

        tCIDLib::TVoid LoadList();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eInitType
        //      We return the default media type reported by the selected repo. We
        //      store it here until we can get it back to the caller's parm.
        //
        //  m_pmdbToFill
        //      Pointers to the caller' DB object we fill in if a new repo is
        //      selected.
        //
        //  m_pmdbEdit
        //      A pointer to the caller's database that we'll fill in if we can
        //      successfully connect to the selected driver.
        //
        //  m_pwndXXXX
        //      Some typed pointers to our widgets. We don't own them, we just
        //      want to avoid lots of downcasting when we use them.
        //
        //  m_strCurMoniker
        //      We store the moniker of any current repo that the caller has the lease
        //      for, so that we can release it.
        //
        //  m_strDBSerialNum
        //      We store the DB serial number of the new database here until we can
        //      get it back to the caller's parmeter.
        //
        //  m_strLeaseId
        //      We get a copy of the lease id from our owner window for later use
        //      during processing.
        //
        //  m_strRepoMoniker
        //  m_strRepoPath
        //      We store the moniker of the selected repo and repo path here until
        //      we can get back to the caller's out params.
        // -------------------------------------------------------------------
        tCQCMedia::EMediaTypes  m_eInitType;
        TMediaDB*               m_pmdbToFill;
        TPushButton*            m_pwndCancel;
        TPushButton*            m_pwndReload;
        TComboBox*              m_pwndRepoList;
        TPushButton*            m_pwndSelect;
        TString                 m_strCurMoniker;
        TString                 m_strDBSerialNum;
        TString                 m_strLeaseId;
        TString                 m_strRepoMoniker;
        TString                 m_strRepoPath;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TMediaRepoSelRepoDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK

