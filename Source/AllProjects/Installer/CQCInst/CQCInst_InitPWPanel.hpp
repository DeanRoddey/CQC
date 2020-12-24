//
// FILE NAME: CQCInst_InitPWPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/12/2004
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
//  This is the header for the info panel derived class that gets an initial
//  admin password from the user for a new install. If it's not a new install
//  they don't see this one.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstInitPWPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstInitPWPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstInitPWPanel() = delete;

        TInstInitPWPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstInitPWPanel(const TInstInitPWPanel&) = delete;
        TInstInitPWPanel(TInstInitPWPanel&&) = delete;

        ~TInstInitPWPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TInstInitPWPanel& operator=(const TInstInitPWPanel&) = delete;
        TInstInitPWPanel& operator=(TInstInitPWPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPanelIsVisible() const final;

        tCIDLib::TBoolean bValidate
        (
                    tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   final;


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


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bPWSetDone
        //      Once they get a good set of the password, we allow them to
        //      move forward. This flag is checked by the validation method.
        //
        //  m_pwndXXX
        //      Some typed pointers to important widgets that we have to
        //      interact with enough to want to avoid type casting. We don't
        //      own them.
        //
        //  m_strCurPW
        //      We don't make them move forward after setting the password,
        //      and they could change their mind and do it again. So we have
        //      to remember the last password they set. We start it as Welcome,
        //      and then just set it to whatever the last successful password
        //      set was.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bPWSetDone;
        TEntryField*        m_pwndPassword;
        TPushButton*        m_pwndSetPW;
        TString             m_strCurPW;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstInitPWPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


