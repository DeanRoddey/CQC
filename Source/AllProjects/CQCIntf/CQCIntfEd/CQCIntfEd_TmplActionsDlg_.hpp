//
// FILE NAME: CQCIntfEd_TmplActionsDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/23/2015
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
//  This is the header for the CQCIntfEd_TmplActionsDlg.cpp file, which allows the
//  user to edit the named actions associated with a template.
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
//   CLASS: TTmplActionsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TTmplActionsDlg : public TDlgBox, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTmplActionsDlg();

        ~TTmplActionsDlg();


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
            ,       TAttrData&              adatTar
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
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            , const TString&                strTmplBasePath
            ,       tCQCIntfEng::TActList&  colActions
            ,       tCQCKit::TCmdTarList&   colExtraTars
            , const TCQCUserCtx&            cuctxToUse
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
        TTmplActionsDlg(const TTmplActionsDlg&);
        tCIDLib::TVoid operator=(const TTmplActionsDlg&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ListToColInd
        (
            const   tCIDLib::TCard4         c4RowId
        )   const;

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eListHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditAction();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4NextId
        //      For mapping from action to list index we assign a temporary id to each new
        //      action added to the list and set that as the list row id for that new item.
        //
        //  m_colEdit
        //      We make a copy of the actions to edit, and copy them back if the user saves.
        //
        //  m_pcolExtraTars
        //      The caller passes us any extra command targets he wants us to present when
        //      when editing the actions.
        //
        //  m_pcolOrgActs
        //      A pointer to the original actions so that we can test for changes.
        //
        //  m_pcuctxToUse
        //      The caller has to provide us with a user context so that we can do
        //      various things when the user edits any actions.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to interact
        //      with on a typed basis.
        //
        //  m_strTmplBasePath
        //      The caller gives us the path of the template that is being edited. We
        //      have to set them when editing the actions, to support relative template
        //      paths.
        // -------------------------------------------------------------------
        tCIDLib::TCard4                 m_c4NextId;
        tCQCIntfEng::TActList           m_colEdit;
        tCQCKit::TCmdTarList*           m_pcolExtraTars;
        const tCQCIntfEng::TActList*    m_pcolOrgActs;
        const TCQCUserCtx*              m_pcuctxToUse;
        TStaticMultiText*               m_pwndActText;
        TPushButton*                    m_pwndAdd;
        TPushButton*                    m_pwndCancel;
        TPushButton*                    m_pwndDel;
        TPushButton*                    m_pwndEdit;
        TMultiColListBox*               m_pwndList;
        TPushButton*                    m_pwndSave;
        TString                         m_strTmplBasePath;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TTmplActionsDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



