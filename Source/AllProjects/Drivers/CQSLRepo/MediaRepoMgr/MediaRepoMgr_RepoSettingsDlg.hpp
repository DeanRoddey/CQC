//
// FILE NAME: MediaRepoMgr_RepoSettingsDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/11/2006
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
//  This is the header for MediaRepoMgr_REpoSettingsDlg.cpp file, which implements a
//  simple dialog to allow the user to set the repository settings that we allow them
//  to control.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)



// ---------------------------------------------------------------------------
//   CLASS: TRepoSettingsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TRepoSettingsDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TRepoSettingsDlg();

        TRepoSettingsDlg(const TRepoSettingsDlg&) = delete;

        ~TRepoSettingsDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TRepoSettingsDlg& operator=(const TRepoSettingsDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TMediaRepoSettings&     mrsToEdit
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        typedef tCIDLib::TStrList           TNameList;
        typedef TVector<TNameList>          TFmtList;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eComboHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid TestMMovies
        (
            const   TString&                strName
            , const TString&                strPassword
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colCodecNames
        //  m_colCodecFmts
        //      The list of codec names, and for each item in the name list the is
        //      a vector of formats available for that codec.
        //
        //  m_mrsToEdit
        //      A copy of the caller's settings that we edit. We'll copy it back to
        //      the caller's object if the user commits.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our child controls we want to
        //      interact with in a type specific way. We don't own them.
        // -------------------------------------------------------------------
        TNameList           m_colCodecNames;
        TFmtList            m_colCodecFmts;
        TMediaRepoSettings  m_mrsToEdit;
        TPushButton*        m_pwndCancel;
        TComboBox*          m_pwndCodecList;
        TComboBox*          m_pwndFormatList;
        TCheckBox*          m_pwndJitter;
        TEntryField*        m_pwndMMName;
        TEntryField*        m_pwndMMPassword;
        TCheckBox*          m_pwndRipEject;
        TPushButton*        m_pwndSave;
        TCheckBox*          m_pwndShowAdult;
        TCheckBox*          m_pwndShowEnglish;
        TCheckBox*          m_pwndUseMM;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TRepoSettingsDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK

