//
// FILE NAME: CQCIGKit_MngVarsDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2006
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
//  This is the header for the CQCIGKit_MngVarsDlg.cpp file, which
//  implements a specialized dialog used for prompting the user for action
//  variable names. Normally we'd use the generic string getting dialogs
//  from CIDWUtils, but we have the special need that we also have to ask
//  them if the want a reference, i.e. %(LVar:FooBar) or just the name
//  itself, LVar:FooBar, to be generated. So we need to add a check box
//  for that.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TMngVarsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TMngVarsDlg : public TDlgBox, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMngVarsDlg();

        TMngVarsDlg(const TMngVarsDlg&) = delete;

        ~TMngVarsDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TMngVarsDlg& operator=(const TMngVarsDlg&) = delete;


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
        )   override;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       TCQCActVarList&         cavlToUse
            ,       TString&                strSelected
            ,       tCIDLib::TBoolean&      bByValue
            , const tCIDLib::TBoolean       bNameOnly = kCIDLib::False
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::TVoid RenumberItems();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bNameOnly
        //      The caller can tell us only to allow by name selection. We
        //      store that here for later use so we can disable the by value
        //      button.
        //
        //  m_pcavlToUse
        //      A pointer to the caller's variable name list to edit/use.
        //
        //  m_pwndXXXX
        //      Some typed pointers to our widgets. We don't own them, we just
        //      want to avoid lots of downcasting when we use them.
        //
        //  m_strVarName
        //      To get any incoming name in and the result back out.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bNameOnly;
        TCQCActVarList*     m_pcavlToUse;
        TPushButton*        m_pwndAdd;
        TPushButton*        m_pwndByName;
        TPushButton*        m_pwndByValue;
        TPushButton*        m_pwndClose;
        TPushButton*        m_pwndDelete;
        TMultiColListBox*   m_pwndVarList;
        TString             m_strVarName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TMngVarsDlg, TDlgBox)
};

#pragma CIDLIB_POPPACK




