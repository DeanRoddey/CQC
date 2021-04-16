//
// FILE NAME: CQCInst_SecurityPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/02/2021
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
//  This is the header for the info panel derived class that shows the user
//  a reminder about setting up security on the registry entry used by CQC
//  to store some info used by background servers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstSecurityPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstSecurityPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstSecurityPanel() = delete;

        TInstSecurityPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstSecurityPanel(const TInstSecurityPanel&) = delete;
        TInstSecurityPanel(TInstSecurityPanel&&) = delete;

        ~TInstSecurityPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstSecurityPanel& operator=(const TInstSecurityPanel&) = delete;
        TInstSecurityPanel& operator=(TInstSecurityPanel&&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TBoolean bPanelIsVisible() const final;


    private :
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
        RTTIDefs(TInstSecurityPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


