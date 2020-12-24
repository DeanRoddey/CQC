//
// FILE NAME: HAIOmniTCPC_AllTrigsDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/14/2013
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
//  This file implements the dialog that allows the user to edit an item.
//  It's independent of the type of the item being edited. The item classes
//  provide polymorphic methods that let us do what we need to do.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TAllTrigsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TAllTrigsDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAllTrigsDlg();

        TAllTrigsDlg(const TAllTrigsDlg&) = delete;

        ~TAllTrigsDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TAllTrigsDlg& operator=(const TAllTrigsDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Run
        (
            const   TWindow&                wndOwner
            ,       TOmniTCPDrvCfg&         dcfgCurrent
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


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pdcfgCurrent
        //      A pointer to the current config, so that we can update it
        //      as required.
        //
        //  m_pwndXXX
        //      Typed points to some of our widgets
        // -------------------------------------------------------------------
        TOmniTCPDrvCfg* m_pdcfgCurrent;
        TPushButton*    m_pwndClose;
        TListBox*       m_pwndTypes;
        TPushButton*    m_pwndClrAll;
        TPushButton*    m_pwndSetAll;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TAllTrigsDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



