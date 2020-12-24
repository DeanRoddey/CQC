//
// FILE NAME: IRClient_ReceiverTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/02/2003
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
//  This file implements the tab that gets inserted into the main driver
//  window's tabbed control, if the device supports receiver functionality.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TIRReceiverTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TIRReceiverTab : public TTabWindow
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TIRReceiverTab() = delete;

        TIRReceiverTab
        (
                    TIRClientWnd* const     pwndDriver
        );

        TIRReceiverTab(const TIRReceiverTab&) = delete;

        ~TIRReceiverTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIRReceiverTab& operator=(const TIRReceiverTab&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset();

        tCIDLib::TVoid SetConnected
        (
            const   TString&                strFirmware
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        typedef TVector<TCQCStdKeyedCmdSrc> TEventList;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetIREvent
        (
                    TString&                strToFill
            , const TString&                strRetrainKey
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadEvents
        (
            const   TString&                strSelect
        );

        tCIDLib::TVoid NewOrEdit
        (
            const   tCIDLib::TBoolean       bNew
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colEvents
        //      Our current list of events. This represents what is displayed
        //      in the list box.
        //
        //  m_pwndDriver
        //      A pointer to the driver window that we work in terms of.
        //      Primarily we need to get access it's driver client proxy.
        //
        //  m_pwndXXX
        //      Some typed pointers to our widgets. We don't own them, our
        //      parent class does, but we want to have some typed pointers so
        //      that we avoid lots of downcasting. They are set up during
        //      startup.
        // -------------------------------------------------------------------
        TEventList          m_colEvents;
        TPushButton*        m_pwndDeleteAll;
        TPushButton*        m_pwndDelete;
        TIRClientWnd*       m_pwndDriver;
        TPushButton*        m_pwndEdit;
        TMultiColListBox*   m_pwndList;
        TPushButton*        m_pwndNew;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIRReceiverTab, TTabWindow)
};

#pragma CIDLIB_POPPACK



