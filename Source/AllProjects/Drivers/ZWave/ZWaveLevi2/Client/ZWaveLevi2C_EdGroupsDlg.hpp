//
// FILE NAME: ZWaveLevi2C_EdGroupsDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/21/2014
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
//  This is the header for the ZWaveLevi2C_EdGroupsDlg.cpp file, which implements
//  the TZWaveEdGroupsDlg class. This class handles the editing of the group
//  info. In this driver's case, it's just the ability to give a name to one
//  of the 255 possible groups stored in the Leviton.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveEdGroupsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWaveEdGroupsDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveEdGroupsDlg();

        TZWaveEdGroupsDlg(const TZWaveEdGroupsDlg&) = delete;

        ~TZWaveEdGroupsDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWaveEdGroupsDlg& operator=(const TZWaveEdGroupsDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strMoniker
            ,       TZWaveDrvConfig&        dcfgEdit
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
        tCIDLib::TVoid DeleteAllGroups();

        tCIDLib::TVoid DeleteGroup();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditGroup();

        tCIDLib::TVoid FmtTextAt
        (
            const   tCIDLib::TCard4         c4At
            ,       TString&                strToFill
        );

        tCIDLib::TVoid LoadGrpList();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colEdit
        //      The caller passes us a copy of the configuration. We use that
        //      to get out our own copy of the current group names, for a
        //      temporary copy we can edit without commitment until the user
        //      decides to save or cancel.
        //
        //  m_pwndXXXX
        //      Since we have to interact with these controls a lot, we get
        //      convenience, typed, pointers to them. We don't own these, we
        //      just are looking at them in a convenient way.
        //
        //  m_strMoniker
        //      The moniker of our driver, which the caller provides for us,
        //      so that we can create a client proxy and upload the changes
        //      made.
        // -------------------------------------------------------------------
        tCIDLib::TStrList   m_colEdit;
        TPushButton*        m_pwndCancel;
        TPushButton*        m_pwndDelete;
        TPushButton*        m_pwndDeleteAll;
        TListBox*           m_pwndList;
        TPushButton*        m_pwndSave;
        TString             m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveEdGroupsDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


