//
// FILE NAME: CQCIntfEd_SearchDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/09/2010
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
//  This is the header for the CQCIntfEd_SearchDlg.cpp file, which implements a search
//  and replace dialog specialized for template content.
//
//  operation.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//
#pragma once



// ---------------------------------------------------------------------------
//  CLASS: TIntfSearchDlg
// PREFIX: dlgb
// ---------------------------------------------------------------------------
class TIntfSearchDlg : public TDlgBox
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TIntfSearchDlg();

        ~TIntfSearchDlg();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
                    TWindow&                wndOwner
            ,       tCIDLib::TBoolean&      bRegEx
            ,       tCIDLib::TBoolean&      bFullMatch
            ,       TString&                strFindStr
            ,       TString&                strRepStr
            ,       tCQCIntfEng::ERepFlags& eFlags
            , const tCIDLib::TBoolean       bTmplSelected
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCQCIntfEng::ERepFlags eQueryFlags();

        tCIDLib::TVoid LoadLastVals();

        tCIDLib::TVoid UpdateRecallList
        (
            const   TString&                strNewVal
            ,       tCIDLib::TStrList&      colList
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bTmplSelected
        //      We get a flag indicating the special case of just the template
        //      being selected for search. In that case, we enable the Children
        //      option, else we disable it.
        //
        //  m_pwndXXX
        //      We store a few typed pointers to our widgets for convenience.
        //      We don't own them, we just reference them.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bTmplSelected;
        TPushButton*            m_pwndCancel;
        TComboBox*              m_pwndFindList;
        TCheckBox*              m_pwndActParms;
        TCheckBox*              m_pwndChildren;
        TCheckBox*              m_pwndExpressions;
        TCheckBox*              m_pwndFldName;
        TCheckBox*              m_pwndFiltParms;
        TCheckBox*              m_pwndFullMatch;
        TCheckBox*              m_pwndImgName;
        TCheckBox*              m_pwndMoniker;
        TCheckBox*              m_pwndText;
        TCheckBox*              m_pwndTemplate;
        TCheckBox*              m_pwndRegEx;
        TPushButton*            m_pwndReplace;
        TComboBox*              m_pwndRepList;
        TCheckBox*              m_pwndURL;
        TCheckBox*              m_pwndVarName;

        tCIDLib::TBoolean       m_bFullMatch;
        tCIDLib::TBoolean       m_bRegEx;
        TString                 m_strFindStr;
        TString                 m_strRepStr;
        tCQCIntfEng::ERepFlags  m_eFlags;


        // -------------------------------------------------------------------
        //  Private, static methods
        //
        //  We save the last entered values so we can put them back as the
        //  defaults. We save lists for the strings, so we can provide a 'last
        //  X values' type recall list.
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean        s_bLastFull;
        static tCIDLib::TBoolean        s_bLastRegEx;
        static tCIDLib::TStrList        s_colFindList;
        static tCIDLib::TStrList        s_colRepList;
        static tCQCIntfEng::ERepFlags   s_eLastFlags;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfSearchDlg,TDlgBox)
};

