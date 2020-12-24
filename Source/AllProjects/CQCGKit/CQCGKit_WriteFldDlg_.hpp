//
// FILE NAME: CQCGKit_WriteFldDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/22/2002
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
//  This file implements a pair of dialog boxes with a common base. The base one is used
//  to allow the user to select a value interactively. This value selection is driven by a
//  field type and field limits string. However, we don't assume the source is really a
//  field. It could be a variable, or just some arbitrary value that needs to be gotten
//  from the user in some cases.
//
//  If the user selects a value, then we call a virtual with that value and the derived
//  class decides whether to keep it and exit or give an error and continue.
//

//  We provide one derived class that is a trivial wrapper around the base and is used
//  for arbitrary value selection based on a passed type and limit, which it just passes
//  on to the base class. If a selection is made it just accepts that and returns the value
//  to the caller.
//
//
//  We have another drived class that takes a field definition and moniker. It will get
//  the limit and type info out and pass that to the base class. When it gets called on the
//  virtual it will attempt to write the value to the field. If it succeeds, it exists and
//  returns the value, else it shows an error and continues.
//
//
//  The base dialog is a bit unusual in that it has to decide what the optimal interactive
//  means for value selection will be. It then loads up the appropriate dialog resource
//  and remembers which one it loaded. It has to deal with the fact that there will be
//  different child controls based on which value selection means was selected.
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
//  CLASS: TSelLimitValBaseDlg
// PREFIX: dlg
// ---------------------------------------------------------------------------
class TSelLimitValBaseDlg : public TDlgBox
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TSelLimitValBaseDlg(const TSelLimitValBaseDlg&) = delete;
        TSelLimitValBaseDlg(TSelLimitValBaseDlg&&) = delete;

        ~TSelLimitValBaseDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TSelLimitValBaseDlg& operator=(const TSelLimitValBaseDlg&) = delete;
        TSelLimitValBaseDlg& operator=(TSelLimitValBaseDlg&&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TSelLimitValBaseDlg();


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid ValueSelected
        (
            const   TString&                strValue
        )   = 0;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunIt
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const tCQCKit::EFldTypes      eType
            , const TString&                strLimits
            , const TPoint&                 pntOrg
            ,       TString&                strValue
            , const TString&                strInVal
            , const tCIDLib::TBoolean       bInValue
        );



    private :
        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eSliderHandler
        (
                    TSliderChangeInfo&      wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bInValue
        //      Indicates whether m_strInValue has a useful value.
        //
        //  m_bIsEnumType
        //      On the way in we remember if the incoming type/limit represents an enum
        //      type limit.
        //
        //  m_eType
        //      The incoming data type, which is expressed in terms of the standard field
        //      types even if it may not actually be a field providing the source values.
        //
        //  m_i4Min
        //      If it's a spin box, then the values that go in aren't meaningful
        //      and we just get which slot is selected, so we need to remember
        //      what the min of the range is in order to recreate the actual
        //      value without having to go back through getting the min/max
        //      values out of the limit object.
        //
        //  m_pfldlTarget
        //      We create a limits object, based on the provided limits string that
        //      we get.
        //
        //  m_pwndXXX
        //      Some pointers to some of our widgets. This dialog has a couple
        //      of flavors, according to the data type. So some of these will
        //      be used and others not for any one invocation. We don't own
        //      them, they are just convenience pointers.
        //
        //  m_ridLoaded
        //      The id of the dialog template that we loaded, based on the
        //      field type and its range.
        //
        //  m_strInValue
        //      Storage for any incoming initial value until we get it loaded
        //
        //  m_strLimits
        //      The limits string we got. If it's empty, we just get a string value and
        //      pass no judgement.
        //
        //  m_strTitle
        //      Since we are generic, this is the title to display
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bInValue;
        tCIDLib::TBoolean   m_bIsEnumType;
        tCQCKit::EFldTypes  m_eType;
        tCIDLib::TInt4      m_i4Min;
        TCQCFldLimit*       m_pfldlTarget;
        TListBox*           m_pwndBList;
        TCheckBox*          m_pwndCheck;
        TPushButton*        m_pwndCloseButton;
        TComboBox*          m_pwndList;
        TPushButton*        m_pwndSetButton;
        TNumSpinBox*        m_pwndNumSpin;
        TNumSpinBox*        m_pwndNumSpin2;
        TNumSpinBox*        m_pwndNumSpin3;
        TEnumSpinBox*       m_pwndTextSpin;
        TSlider*            m_pwndSlider;
        TMultiEdit*         m_pwndText;
        TStaticText*        m_pwndValue;
        tCIDLib::TResId     m_ridLoaded;
        TString             m_strInValue;
        TString             m_strLimits;
        TString             m_strTitle;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TSelLimitValBaseDlg, TDlgBox)
};


// ---------------------------------------------------------------------------
//  CLASS: TSelLimitValDlg
// PREFIX: dlg
// ---------------------------------------------------------------------------
class TSelLimitValDlg : public TSelLimitValBaseDlg
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TSelLimitValDlg();

        TSelLimitValDlg(const TSelLimitValDlg&) = delete;
        TSelLimitValDlg(TSelLimitValDlg&&) = delete;

        ~TSelLimitValDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TSelLimitValDlg& operator=(const TSelLimitValDlg&) = delete;
        TSelLimitValDlg& operator=(TSelLimitValDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const tCQCKit::EFldTypes      eType
            , const TString&                strLimits
            , const TPoint&                 pntOrg
            ,       TString&                strValue
            , const TString&                strOrgVal
        );

        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const tCQCKit::EFldTypes      eType
            , const TString&                strLimits
            , const TPoint&                 pntOrg
            ,       TString&                strValue
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ValueSelected
        (
            const   TString&                strValue
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strValue
        //      This is where we store the selected string until it can be gotten into
        //      the caller's out parameter.
        // -------------------------------------------------------------------
        TString             m_strValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TSelLimitValDlg, TSelLimitValBaseDlg)
};



// ---------------------------------------------------------------------------
//  CLASS: TWrtFldDlg
// PREFIX: dlg
// ---------------------------------------------------------------------------
class TWrtFldDlg : public TSelLimitValBaseDlg
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TWrtFldDlg() = delete;
        TWrtFldDlg
        (
            const   TCQCSecToken&           sectUser
        );

        TWrtFldDlg(const TWrtFldDlg&) = delete;
        TWrtFldDlg(TWrtFldDlg&&) = delete;

        ~TWrtFldDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TWrtFldDlg& operator=(const TWrtFldDlg&) = delete;
        TWrtFldDlg& operator=(TWrtFldDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const TCQCFldDef&             flddTarget
            , const TString&                strMoniker
            , const TPoint&                 pntOrg
            ,       TString&                strValue
        );

        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const TCQCFldDef&             flddTarget
            , const TString&                strMoniker
            , const TPoint&                 pntOrg
            ,       TString&                strValue
            , const TString&                strOrgVal
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ValueSelected
        (
            const   TString&                strValue
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_flddTarget
        //      The incoming field definition for the target field
        //
        //  m_sectUser
        //      We need a security token to do the field write, so the calling code has
        //      to provide one
        //
        //  m_strMoniker
        //      The incoming drive moniker of the target field
        //
        //  m_strValue
        //      This is where we store the selected string until it can be gotten into
        //      the caller's out parameter.
        // -------------------------------------------------------------------
        TCQCFldDef      m_flddTarget;
        TCQCSecToken    m_sectUser;
        TString         m_strMoniker;
        TString         m_strValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TWrtFldDlg, TSelLimitValBaseDlg)
};

#pragma CIDLIB_POPPACK



