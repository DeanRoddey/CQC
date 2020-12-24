//
// FILE NAME: CQCInst_PrereqPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/02/2017
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
//  This is the header for the info panel derived class that allows the user to install
//  any pre-requisite system features.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstPrereqPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstPrereqPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  A small class to hold the list of prereqs and their info
        // -------------------------------------------------------------------
        enum ETypes
        {
            EType_URL
            , EType_Installer
        };

        class TPRItem
        {
            public :
                TPRItem();

                TPRItem
                (
                    const   ETypes          eType
                    , const tCIDLib::TMsgId midDescr
                    , const tCIDLib::TMsgId midTitle
                    , const tCIDLib::TCh*   pszTarget
                );

                ~TPRItem();

                ETypes      m_eType;
                TString     m_strDescr;
                TString     m_strTitle;
                TString     m_strTarget;
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstPrereqPanel() = delete;

        TInstPrereqPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstPrereqPanel(const TInstPrereqPanel&) = delete;
        TInstPrereqPanel(TInstPrereqPanel&&) = delete;

        ~TInstPrereqPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstPrereqPanel& operator=(const TInstPrereqPanel&) = delete;
        TInstPrereqPanel& operator=(TInstPrereqPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   final;

        tCIDLib::TVoid SaveContents() final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eListHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid InvokeItem
        (
            const   TPRItem&                itemInvoke
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colItems
        //      The list of prereqs that we load up in the ctor and use to drive
        //      the rest of the tab functionality.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        TVector<TPRItem>    m_colItems;
        TPushButton*        m_pwndInstall;
        TMultiColListBox*   m_pwndList;
        TStaticMultiText*   m_pwndListText;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstPrereqPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK

