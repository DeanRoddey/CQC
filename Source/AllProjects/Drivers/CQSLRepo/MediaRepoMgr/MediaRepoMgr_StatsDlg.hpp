//
// FILE NAME: MediaRepoMgr_StatsDlg.hpp
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
//  This is the header for MediaRepoMgr_StatsDlg.cpp, which implements a
//  simple dialog to display some media repository statistics.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TStatsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TStatsDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TStatsDlg();

        TStatsDlg(const TStatsDlg&) = delete;

        ~TStatsDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TStatsDlg& operator=(const TStatsDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
            const   TWindow&                wndOwner
            , const TMediaDB&               mdbSrc
            , const TString&                strRepoPath
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
        //  m_pmdbSrc
        //      A pointer to the incoming database that we get the stats from
        //
        //  m_pwndXXX
        //      Typed pointers to some of our child controls we want to
        //      interact with in a type specific way. We don't own them.
        //
        //  m_strRepoPath
        //      The caller provides us with the repo path.
        // -------------------------------------------------------------------
        const TMediaDB* m_pmdbSrc;
        TPushButton*    m_pwndClose;
        TString         m_strRepoPath;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TStatsDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


