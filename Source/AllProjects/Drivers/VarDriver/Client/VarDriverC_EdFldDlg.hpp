//
// FILE NAME: VarDriverC_EdFldDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/16/2003
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
//  This file implements the dialog that allows the user to edit the field list. We have
//  a bit of an issue here in that we want to allow the field list to be sorted, but we
//  need to be able to map back to the originals, even in the face of a name change and
//  deletion of existing fields (so we can't just store the original field index.) So we
//  assign pseudo field ids to the fields from a running counter and store that as the
//  row id. This provides us with an always valid mapping back from list index to field
//  def. The field ids are persisted, but these field defs are only for our own field
//  config purposes so it doesn't matter.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TEdFldListDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdFldListDlg : public TDlgBox, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdFldListDlg();

        TEdFldListDlg(const TEdFldListDlg&) = delete;

        ~TEdFldListDlg();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TEdFldListDlg& operator=(const TEdFldListDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIPETestCanEdit
        (
            const   tCIDCtrls::TWndId       widSrc
            , const tCIDLib::TCard4         c4ColInd
            ,       TAttrData&              adatFill
            ,       tCIDLib::TBoolean&      bValueSet
        )   override;

        tCIDLib::TBoolean bIPEValidate
        (
            const   TString&                strSrc
            ,       TAttrData&              adatSrc
            , const TString&                strNewVal
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard8&        c8UserId
        )   const override;

        tCIDLib::TVoid IPEValChanged
        (
            const   tCIDCtrls::TWndId       widSrc
            , const TString&                strSrc
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
            , const tCIDLib::TCard8         c8UserId
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       TVarDrvCfg&             cfgEdit
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

        tCIDLib::TCard4 c4FindCurFieldIndex() const;

        tCIDLib::TCard4 c4ListToFldIndex
        (
            const   tCIDLib::TCard4         c4ListInd
        )   const;

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eComboHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDCtrls::EEvResponses eMLEHandler
        (
                    TMLEChangeInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4NextId
        //      A running counter we use to assign pseudo ids to the fields for list
        //      to field mapping.
        //
        //  m_cfgEdit
        //  m_pcfgOrg
        //      We make our own copy to work with and copy back to the caller's
        //      buffer if they commit. And we store a pointer to the original
        //      so we can check for changes if they press cancel, and ask if
        //      they want to lose them.
        //
        //  m_pwndXXX
        //      Typed points to some of our widgets
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4NextId;
        TVarDrvCfg          m_cfgEdit;
        TVarDrvCfg*         m_pcfgOrg;
        TPushButton*        m_pwndAddButton;
        TPushButton*        m_pwndCancelButton;
        TComboBox*          m_pwndDataType;
        TPushButton*        m_pwndDelButton;
        TMultiColListBox*   m_pwndFldList;
        TMultiEdit*         m_pwndLimits;
        TCheckBox*          m_pwndPersist;
        TPushButton*        m_pwndSaveButton;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdFldListDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



