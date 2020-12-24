//
// FILE NAME: CQCIntfEng_CheckBox.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/23/2002
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
//  This is the header for the CQCIntfEng_CheckBox.cpp file, which implements
//  a derivative of the button base class. Check boxes can have a true or
//  false state, and can display different images and/or text based on the
//  current state.
//
//  Derived from this class are two simple derivatives. One of which associates
//  a check box with a field, and the other just lets the user set the state
//  via an action command, i.e. a 'static check box'. This allows for a lot of
//  flexibiltiy since sometimes check boxes are used for purely viewer local
//  operations, not field based stuff.
//
//  A check box implements the image list interface, which lets the user set
//  checked/unchecked images to display. And it provides a tab that lets the
//  user indicate true and false text, and the spatial relationship between
//  the text and the image, so that the image can be to the left and text
//  to the right, or both centered with the text over the image, or not to
//  display any text so just the images themselves indicate the state. It
//  can also optionally use the Fgn colors always for text, or use the Fgn
//  colors for False state and the Extra colors for True state, so that it
//  can change text color based on state.
//
//  The field based one also implements the boolean interface, which lets
//  the user enter expressions used to evaluate the value of the associated
//  field and decide if it's true of false (whatever that means.)
//
//  The field doesn't have to be writable. The check never writes anything
//  to the field itself. The user's logic does any such stuff as desired.
//  So all we have to be able to do is read the value.
//
//  There is a configuration tab for the common check box attributes and it
//  works through the common base class. The field related one uses the std
//  Field tab for field association.
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
//   CLASS: TCQCIntfCheckBoxRTV
//  PREFIX: crtvs
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfCheckBoxRTV : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfCheckBoxRTV() = delete;

        TCQCIntfCheckBoxRTV
        (
            const   TString&                strStateText
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfCheckBoxRTV(const TCQCIntfCheckBoxRTV&) = delete;

        ~TCQCIntfCheckBoxRTV();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfCheckBoxRTV& operator=(const TCQCIntfCheckBoxRTV&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue
        (
            const   TString&                strId
            ,       TString&                strToFill
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strStateText
        //      The text of the newly selected state that the check box is
        //      switching to because of being invoked. The check box passes it
        //      in to us so we can give it out when asked.
        // -------------------------------------------------------------------
        TString m_strStateText;
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfCheckBoxBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfCheckBoxBase

    : public TCQCIntfButtonBase
    , public MCQCIntfBooleanIntf
    , public MCQCIntfFontIntf
    , public MCQCIntfImgListIntf
    , public MCQCCmdSrcIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfCheckBoxBase();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSizeTo() const override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid Clicked
        (
            const   TPoint&                 pntAt
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid Invoke() override;

        [[nodiscard]] TCQCCmdRTVSrc* pcrtsMakeNew
        (
            const   TCQCUserCtx&            cuctxToUse
        )   const override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryContentSize
        (
                    TSize&                  szToFill
        )   override;

        tCIDLib::TVoid QueryReferencedImgs
        (
                    tCIDLib::TStrHashSet&   colToFill
            ,       tCIDLib::TStrHashSet&   colScopes
            , const tCIDLib::TBoolean       bIncludeSysImgs
            , const tCIDLib::TBoolean       bIncludeScopes
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid RefreshImages
        (
                    TDataSrvClient&         dsclToUse
        )   override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        TSize szDefaultSize() const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
        )   const override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bUseExtraClrs() const;

        tCIDLib::TBoolean bUseExtraClrs
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCQCIntfEng::EHTxtImgLayouts eHLayout() const;

        tCQCIntfEng::EHTxtImgLayouts eHLayout
        (
            const   tCQCIntfEng::EHTxtImgLayouts eToSet
        );

        tCIDLib::TVoid Set
        (
            const   tCQCIntfEng::EHTxtImgLayouts eLayout
            , const TString&                strFalseText
            , const TString&                strTrueText
        );

        const TString& strFalseText() const;

        const TString& strFalseText
        (
            const   TString&                strToSet
        );

        const TString& strTrueText() const;

        const TString& strTrueText
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfCheckBoxBase
        (
            const   tCQCIntfEng::ECapFlags  eCaps
            , const TString&                strTypeName
        );

        TCQCIntfCheckBoxBase
        (
            const   TCQCIntfCheckBoxBase&   iwdgToCopy
        );

        TCQCIntfCheckBoxBase& operator=
        (
            const   TCQCIntfCheckBoxBase&   iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   override;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsChecked() const;

        tCIDLib::TBoolean bWriteable() const;

        tCIDLib::TBoolean bWriteable
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCQCIntfEng::EBoolStates eCheckState() const;

        tCIDLib::TVoid SetState
        (
            const   tCQCIntfEng::EBoolStates eToSet
            , const tCIDLib::TBoolean       bNoRedraw
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DrawBox
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bUseExtraClrs
        //      This lets us optionally use different text colors for false
        //      and true state. This was added in 4.1.901 and uses the recently
        //      added 'extra' colors. It defaults to false for backwards
        //      compabilitty. This is persisted.
        //
        //  m_bWriteable
        //      Indicates that the target value we are assigned to is writeable.
        //      The derived class tells us this.
        //
        //  m_eCurState
        //      Our current state, which is either true or false or in error
        //      (meaning not a field error, but the value of the field does
        //      not translate to either a true or false value according to the
        //      configured boolean expressions.) this is runtime only.
        //
        //  m_eHLayout
        //      The horizontal layout option for the image and text. This is persisted.
        //
        //  m_strFalseText
        //  m_strTrueText
        //      The text for the true and false states. They don't have to
        //      be provided, and are just displayed if text is entered. These
        //      are persisted.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean               m_bUseExtraClrs;
        tCIDLib::TBoolean               m_bWriteable;
        tCQCIntfEng::EBoolStates        m_eCurState;
        tCQCIntfEng::EHTxtImgLayouts    m_eHLayout;
        TString                         m_strFalseText;
        TString                         m_strTrueText;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfCheckBoxBase,TCQCIntfButtonBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFieldCheckBox
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFieldCheckBox

    : public TCQCIntfCheckBoxBase, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFieldCheckBox();

        TCQCIntfFieldCheckBox
        (
            const   TCQCIntfFieldCheckBox&  iwdgToCopy
        );

        ~TCQCIntfFieldCheckBox();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfFieldCheckBox& operator=
        (
            const   TCQCIntfFieldCheckBox&  iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             fldTest
            , const TString&                strMake
            , const TString&                strModel
            , const tCIDLib::TBoolean       bStore
        )   override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid RegisterFields
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        )   override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;



    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FieldChanged
        (
                    TCQCFldPollInfo&        cfpiAssoc
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;

        tCIDLib::TVoid QueryBoolAttrLims
        (
                    tCQCKit::EFldTypes&     eType
            ,       TString&                strLimits
        )   const override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid ValueUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bVarsUpdate
            , const TStdVarsTar&            ctarGlobalVars
            , const TGUIRegion&             grgnClip
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfFieldCheckBox,TCQCIntfCheckBoxBase)
        DefPolyDup(TCQCIntfFieldCheckBox)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticCheckBox
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfStaticCheckBox : public TCQCIntfCheckBoxBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStaticCheckBox();

        TCQCIntfStaticCheckBox
        (
            const   TCQCIntfStaticCheckBox& iwdgToCopy
        );

        ~TCQCIntfStaticCheckBox();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfStaticCheckBox& operator=
        (
            const   TCQCIntfStaticCheckBox& iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid Invoke() override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAutoToggle
        //      Often the user just wants something that will toggle when the user clicks
        //      and they just get the final state at the end, so this greatly simplifies
        //      some types of interfaces. Defaults to false which means that they have to
        //      handle toggling themselves.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bAutoToggle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfStaticCheckBox,TCQCIntfCheckBoxBase)
        DefPolyDup(TCQCIntfStaticCheckBox)
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarCheckBox
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfVarCheckBox

    : public TCQCIntfCheckBoxBase, public MCQCIntfVarIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfVarCheckBox();

        TCQCIntfVarCheckBox
        (
            const   TCQCIntfVarCheckBox&    iwdgToCopy
        );

        ~TCQCIntfVarCheckBox();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfVarCheckBox& operator=
        (
            const   TCQCIntfVarCheckBox&    iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptVar
        (
            const   TCQCActVar&             varToTest
        )   override;

        tCIDLib::TVoid QueryBoolAttrLims
        (
                    tCQCKit::EFldTypes&     eType
            ,       TString&                strLimits
        )   const override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid ValueUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bVarsUpdate
            , const TStdVarsTar&            ctarGlobalVars
            , const TGUIRegion&             grgnClip
        )   override;

        tCIDLib::TVoid VarInError
        (
            const   tCIDLib::TBoolean       bNoRedraw
        )   override;

        tCIDLib::TVoid VarValChanged
        (
            const   TCQCActVar&             varNew
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfVarCheckBox,TCQCIntfCheckBoxBase)
        DefPolyDup(TCQCIntfVarCheckBox)
};


#pragma CIDLIB_POPPACK


