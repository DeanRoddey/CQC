//
// FILE NAME: CQCIntfEd_EditXlatsDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/21/2015
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
//  This is the header for the CQCIntfEd_EditXlatsDlg.cpp file, which allows the user
//  to edit the attributes of the Xlats mixin class.
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
//   CLASS: TEditXlatsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEditXlatsDlg : public TDlgBox, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEditXlatsDlg();

        ~TEditXlatsDlg();


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


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       TIEXlatsWrap&           iewXlats
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TEditXlatsDlg(const TEditXlatsDlg&);
        tCIDLib::TVoid operator=(const TEditXlatsDlg&);


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

        tCIDLib::TVoid GatherInfo
        (
                    MCQCIntfXlatIntf&       iewTar
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4NextId
        //      For in place editing purposes, we assign a unique id to each item we
        //      put into the list box.
        //
        //  m_iewEdit
        //      We have to have somewhere to put the edited data until we can get it
        //      back to the caller's parameter.
        //
        //  m_piewOrg
        //      A pointer to the incoming data so that we can get the incoming data
        //      loaded up.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to
        //      interact with on a typed basis.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4NextId;
        TIEXlatsWrap        m_iewEdit;
        TIEXlatsWrap*       m_piewOrg;
        TPushButton*        m_pwndAddButton;
        TPushButton*        m_pwndCancelButton;
        TMultiColListBox*   m_pwndList;
        TPushButton*        m_pwndRemoveButton;
        TPushButton*        m_pwndRemoveAllButton;
        TPushButton*        m_pwndSaveButton;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEditXlatsDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



