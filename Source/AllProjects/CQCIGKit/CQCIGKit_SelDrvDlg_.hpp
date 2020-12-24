//
// FILE NAME: CQCIGKit_SelDrvDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/15/2015
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
//  This is the header for the CQCIGKit_SelDrvDlgDlg.cpp file, which allows the user
//  to select a driver moniker. There is another dialog for selecting a driver.field
//  combination. This is just the driver.
//
//  We let the list sort, which is no a problem since we are dealing purely with
//  the text in the list. No need to go back to the original data.
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
//   CLASS: TCQCSelDrvDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TCQCSelDrvDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSelDrvDlg();

        ~TCQCSelDrvDlg();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       TString&                strDriver
            , const tCQCKit::EDevCats       eDevCatFilt = tCQCKit::EDevCats::Count
            , const TString&                strMakeFilt = TString::strEmpty()
            , const TString&                strModelFilt = TString::strEmpty()
        );

        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            , const TCQCFldCache&           cfcFldInfo
            ,       TString&                strDriver
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
        TCQCSelDrvDlg(const TCQCSelDrvDlg&);
        tCIDLib::TVoid operator=(const TCQCSelDrvDlg&);


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
        //  m_colDrivers
        //      We get a filtered list of driver monikers before we invoke the dialog,
        //      sorted. This is then loaded for the user to select from.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to
        //      interact with on a typed basis.
        //
        //  m_strDriver
        //      We store the incoming value (if any) for initial selection, and
        //      we store the user selection here until we can get it back into the
        //      caller's parm.
        // -------------------------------------------------------------------
        tCIDLib::TStrList   m_colDrivers;
        TPushButton*        m_pwndCancelButton;
        TMultiColListBox*   m_pwndDrivers;
        TPushButton*        m_pwndSelectButton;
        TString             m_strDriver;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSelDrvDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



