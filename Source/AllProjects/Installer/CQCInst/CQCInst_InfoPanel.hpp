//
// FILE NAME: CQCInst_InfoPanel.hpp
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
//  This is the header for the base info panel class. A set of classes are
//  derived from this panel to get information from the user in a wizardy
//  sort of way. Each one represents a page of information. Each of them
//  is given a pointer to the info object that holds that information and
//  they will initialize from it and store to it as requested.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstInfoPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstInfoPanel : public TGenericWnd
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstInfoPanel() = delete;

        TInstInfoPanel(const TInstInfoPanel&) = delete;
        TInstInfoPanel(TInstInfoPanel&&) = delete;

        ~TInstInfoPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TInstInfoPanel& operator=(const TInstInfoPanel&) = delete;
        TInstInfoPanel& operator=(TInstInfoPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bPanelIsVisible() const;

        virtual tCIDLib::TBoolean bValidate
        (
                    tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        );

        virtual tCQCInst::ENavFlags eNavFlags() const;

        virtual tCIDLib::TVoid Entered();

        virtual tCIDLib::TVoid ReactToChanges();

        virtual tCQCInst::EProcRes eProcess();

        virtual tCIDLib::TVoid ReportPortUsage
        (
                    tCQCInst::TPortCntList& fcolPortCnts
        );

        virtual tCIDLib::TVoid SaveContents();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidated() const;

        tCIDLib::TBoolean bValidated
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid CreateInfoPanel
        (
            const   TWindow&                wndInitParent
            , const TArea&                  areaContent
            , const tCIDCtrls::TWndId       widToUse
        );

        const TString& strName() const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TInstInfoPanel
        (
            const   TString&                strName
            ,       TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCInstallInfo& infoCur() const;

        TCQCInstallInfo& infoCur();

        tCIDLib::TVoid LoadDlgItems
        (
            const   tCIDLib::TResId         ridToLoad
        );

        const TMainFrameWnd& wndParent() const;

        TMainFrameWnd& wndParent();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bValidated
        //      This panel has been seen by the user and successfully validated
        //      and moved past.
        //
        //  m_pinfoCur
        //      Each panel gets a pointer to the install info, so that it can look
        //      up any info it needs or make changes as required. We provide a protected
        //      method to get access to this.
        //
        //  m_strName
        //      The name of the panel, for human consumption purposes.
        //
        //  m_pwndParent
        //      We get a pointer to the main frame window which we make available to
        //      the derived classes for their convenience.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bValidated;
        TCQCInstallInfo*        m_pinfoCur;
        TString                 m_strName;
        TMainFrameWnd*          m_pwndParent;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstInfoPanel,TGenericWnd)
};

#pragma CIDLIB_POPPACK

