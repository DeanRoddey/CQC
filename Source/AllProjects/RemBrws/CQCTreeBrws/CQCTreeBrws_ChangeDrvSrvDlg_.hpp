//
// FILE NAME: CQCTreeBrws_ChangeDrvSrvDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/02/2016
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
//  This is the header for the CQCTreeBrws_ChangeDrvSrvDlg.cpp file, which implements
//  the TChangeDrvSrvDlg class. This dialog allows the user to change the host for which
//  drivers are configured, for when they change the host name or move the CQC install
//  to a new machine. They click on the current server name, and we let them select a
//  new server. If they commit, we just return the selected host, and the caller will
//  do the work.
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
//   CLASS: TChangeDrvSrvDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TChangeDrvSrvDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TChangeDrvSrvDlg();

        TChangeDrvSrvDlg(const TChangeDrvSrvDlg&) = delete;

        ~TChangeDrvSrvDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TChangeDrvSrvDlg& operator=(const TChangeDrvSrvDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            , const tCIDLib::TStrList&      colList
            , const TString&                strOrgHost
            ,       TString&                strSelected
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
        //  m_pcolList
        //      This is the list of servers the caller wants us to select between.
        //
        //  m_pwndXXX
        //      We get typed pointers to controls we need to interact with
        //
        //  m_strOrgHost
        //      The original host from which the driver's will be moved, so that we can
        //      put into the instruction text.
        //
        //  m_strSelected
        //      The host selected until we can get it back to the caller's parameters.
        // -------------------------------------------------------------------
        const tCIDLib::TStrList*    m_pcolHosts;
        TPushButton*                m_pwndCancel;
        TPushButton*                m_pwndChange;
        TMultiColListBox*           m_pwndList;
        TString                     m_strOrgHost;
        TString                     m_strSelected;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TChangeDrvSrvDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


