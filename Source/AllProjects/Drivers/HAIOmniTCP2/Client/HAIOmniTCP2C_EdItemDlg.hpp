//
// FILE NAME: HAIOmniTCP2C_EdItemDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/09/2014
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
//   CLASS: TEdItemDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdItemDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdItemDlg();

        TEdItemDlg(const TEdItemDlg&) = delete;

        ~TEdItemDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEdItemDlg& operator=(const TEdItemDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tHAIOmniTCP2C::EEditRes eRunDlg
        (
            const   TWindow&                wndOwner
            ,       THAIOmniItem&           itemEdit
            , const TOmniTCPDrvCfg&         dcfgCurrent
            , const tCIDLib::TBoolean       bCanDelete
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
        tCIDLib::TBoolean bValidate();

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
        //  m_bCanDelete
        //      The caller tells us if we are editing an existing item and
        //      therefore we can enable the delete button.
        //
        //  m_pdcfgCurrent
        //      A pointer to the current config, so that we can make sure that
        //      the new name is not a dup.
        //
        //  m_pitemEdit
        //      A pointer to the caller's incoming item that we are editing.
        //
        //  m_pwndXXX
        //      Typed points to some of our widgets
        //
        //  m_strOrgName
        //      The original name we got incoming, if any. This way, when we
        //      do the dup name check, we can be sure we don't reject the
        //      original (unchanged) name.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bCanDelete;
        const TOmniTCPDrvCfg*   m_pdcfgCurrent;
        THAIOmniItem*           m_pitemEdit;
        TComboBox*              m_pwndAreas;
        TPushButton*            m_pwndCancelButton;
        TComboBox*              m_pwndTypes;
        TPushButton*            m_pwndDelButton;
        TEntryField*            m_pwndName;
        TPushButton*            m_pwndSaveButton;
        TCheckBox*              m_pwndSendTrig;
        TString                 m_strOrgName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdItemDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


