//
// FILE NAME: CQCInst_SuccessPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/03/2004
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
//  This is the header for the info panel derived class that handles showing
//  the final success/failure info.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstSuccessPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstSuccessPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstSuccessPanel() = delete;

        TInstSuccessPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstSuccessPanel(const TInstSuccessPanel&) = delete;
        TInstSuccessPanel(TInstSuccessPanel&&) = delete;

        ~TInstSuccessPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstSuccessPanel& operator=(const TInstSuccessPanel&) = delete;
        TInstSuccessPanel& operator=(TInstSuccessPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCQCInst::ENavFlags eNavFlags() const final;

        tCIDLib::TVoid Entered() final;

        tCIDLib::TVoid ReactToChanges() final;


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


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaContent
        //      We lazily fault in our controls so that we can show either
        //      success or failure content. So we have to save this until we
        //      need it.
        //
        //  m_bStatus
        //      This is set on us before we are loaded, to tell us what the
        //      success/failure status was. We use this to know which dialog
        //      description to load.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        TArea               m_areaContent;
        tCIDLib::TBoolean   m_bStatus;
        TStaticImg*         m_pwndImg;
        TPushButton*        m_pwndSaveWarn;
        TStaticMultiText*   m_pwndText;
        TMultiColListBox*   m_pwndWarnList;
        TStaticText*        m_pwndWarnRes;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstSuccessPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK




