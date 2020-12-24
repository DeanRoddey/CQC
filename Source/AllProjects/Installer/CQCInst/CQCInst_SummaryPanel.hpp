//
// FILE NAME: CQCInst_SummaryPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/30/2004
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
//  a summary of install options to the user before committing to the install.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstSummaryPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstSummaryPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstSummaryPanel() = delete;

        TInstSummaryPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstSummaryPanel(const TInstSummaryPanel&) = delete;
        TInstSummaryPanel(TInstSummaryPanel&&) = delete;

        ~TInstSummaryPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstSummaryPanel& operator=(const TInstSummaryPanel&) = delete;
        TInstSummaryPanel& operator=(TInstSummaryPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Entered() final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadSummary();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        TMultiEdit*     m_pwndText;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstSummaryPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


