//
// FILE NAME: CQCTreeBrws_TSelSrvDrvDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/14/2001
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
//  This is the header for the CQCTreeBrws_SelSrvDrvDlg.cpp file, which implements
//  the TSelSrvDrvDlg class. This dialog is shown to allow the user to select a
//  device driver to load. We get that information from the installation server,
//  which provides us with the info parse in from all of the available driver manifest
//  files.
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
//   CLASS: TSelSrvDrvDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TSelSrvDrvDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TVector<TCQCDriverCfg>      TCfgList;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TSelSrvDrvDlg();

        TSelSrvDrvDlg(const TSelSrvDrvDlg&) = delete;

        ~TSelSrvDrvDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TSelSrvDrvDlg& operator=(const TSelSrvDrvDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TCQCDriverCfg&          cqcdcToFill
            , const TString&                strHost
            , const TCQCSecToken&           sectUser
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

        tCIDLib::TVoid FilterDriverList();

        tCIDLib::TVoid LoadDevClasses
        (
            const   TCQCDriverCfg&          cqcdcSrc
        );

        tCIDLib::TVoid LoadFilters();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4SelIndex
        //      If they select one, we store the index here so that we can get to
        //      the selected entry when we get back out and need to copy the info
        //      to the caller's parameter.
        //
        //  m_colDriverCfgs
        //      This is the list of driver configuration objects that we get
        //      from the installation server and display in our list box.
        //
        //  m_pwndXXX
        //      Since we have to interact with these controls a lot, we get
        //      convenience, typed, pointers to them. We don't own these, we
        //      just are looking at them a convenient way.
        //
        //  m_sectUser
        //
        //
        //  m_strHost
        //      We are given the target host for which the driver is to be loaded,
        //      so that we can put it into the title bar.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4SelIndex;
        TCfgList            m_colDriverCfg;
        TPushButton*        m_pwndCancelButton;
        TMultiColListBox*   m_pwndCatList;
        TStaticMultiText*   m_pwndDevClasses;
        TMultiColListBox*   m_pwndDrivers;
        TMultiColListBox*   m_pwndMakeList;
        TPushButton*        m_pwndSelectButton;
        TCQCSecToken        m_sectUser;
        TString             m_strHost;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TSelSrvDrvDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


