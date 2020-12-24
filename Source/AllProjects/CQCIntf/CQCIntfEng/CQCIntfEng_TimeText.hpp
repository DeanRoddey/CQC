//
// FILE NAME: CQCIntfEng_TimeText.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/17/2007
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
//  This is the header for the CQCIntfEng_TimeText.cpp file, which
//  implements a couple widgets designed to show time/date values in a
//  flexible way. This widget can be used in two different ways. It can be
//  set up to get it's time value from a field, or from the current time on
//  the local system.
//
//  Because the widgets are dynamically allocated by name, we have to create
//  two trivial derivatives that just call the base class ctor with a flag
//  that indicates whether it's a field mode or local mode time.
//
// CAVEATS/GOTCHAS:
//
//  1)  These guys support a 'hide if zero' option, which overrides any
//      the widget visbility state and will hide the widget if it goes to
//      zero value. So it's another layer over the basic widget visbility
//      stuff, but it only ever overrides a hide, it never forces it to show
//      if it's non-zero and the widget is marked hidden.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfTimeTextTab;


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTimeTextBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfTimeTextBase

    : public TCQCIntfWidget, public MCQCIntfFontIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfTimeTextBase();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        TSize szDefaultSize() const override;

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


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHideIfZero() const;

        tCIDLib::TBoolean bHideIfZero
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TInt4 i4Offset() const;

        tCIDLib::TInt4 i4Offset
        (
            const   tCIDLib::TInt4          i4ToSet
        );

        const TString& strFormat() const;

        const TString& strFormat
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfTimeTextBase
        (
            const   tCQCIntfEng::ECapFlags  eCapflags
            , const TString&                strTypeName
        );

        TCQCIntfTimeTextBase
        (
            const   TCQCIntfTimeTextBase&   iwdgToCopy
        );

        TCQCIntfTimeTextBase& operator=
        (
            const   TCQCIntfTimeTextBase&   iwdgToAssign
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
        tCIDLib::TVoid SetTime
        (
            const   tCIDLib::TEncodedTime   enctToSet
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bInError
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DrawText
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TArea&                  areaDraw
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bHideIfZero
        //      This allows them to tell us to just show nothing if the raw
        //      time stamp is zero, which is commonly desired if this is being
        //      used to show an elapsed media time. It'll get zeroed when the
        //      media ends, and they won't want to show anything.
        //
        //  m_i4Offset
        //      They can indicate an offset that we should apply to the time
        //      value that we get. We can add or substract up to 12 hours
        //      from the indicated time. It can be used to show a time of
        //      another time zone from the current, or to adjust an incoming
        //      field based time to the local.
        //
        //  m_strTmpFmt
        //      Used internally for formatting the value
        //
        //  m_strValue
        //      The formatted out value. When we get a new value, we set it
        //      on the time object below, then format it out to this field so
        //      that we don't have to reformat every time we are asked to
        //      redraw.
        //
        //  m_tmCurTime
        //      We keep a time object around to do the formatting with. The
        //      derived classes provide us with a TEncodedTime time stamp and
        //      we set it on this object and use it to format out the value.
        //
        //      We persist the time format info, but not the time object
        //      itself.
        //
        //      NOTE: We assume that this always has the most recently set
        //      time value in it (which would be set in the SetTime() method,
        //      and we use this if the m_bHideIfZero flag is set, to know if
        //      we should hide it because thetime value is zero. This will
        //      override the visbility state in this one case and hide it even
        //      if it's marked visible.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bHideIfZero;
        tCIDLib::TInt4      m_i4Offset;
        TString             m_strTmpFmt;
        TString             m_strValue;
        TTime               m_tmCurTime;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfTimeTextBase,TCQCIntfWidget)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFieldTimeText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFieldTimeText

    : public TCQCIntfTimeTextBase, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFieldTimeText();

        TCQCIntfFieldTimeText
        (
            const TArea&                    areaInit
            , const TPoint&                 pntParent
        );

        TCQCIntfFieldTimeText
        (
            const   TCQCIntfFieldTimeText&  iwdgToCopy
        );


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TCQCIntfFieldTimeText& operator=
        (
            const   TCQCIntfFieldTimeText&  iwdgToAssign
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
        RTTIDefs(TCQCIntfFieldTimeText, TCQCIntfTimeTextBase)
        DefPolyDup(TCQCIntfFieldTimeText)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfLocalTimeText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfLocalTimeText : public TCQCIntfTimeTextBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfLocalTimeText();

        TCQCIntfLocalTimeText
        (
            const TArea&                    areaInit
            , const TPoint&                 pntParent
        );

        TCQCIntfLocalTimeText
        (
            const   TCQCIntfLocalTimeText&  iwdgToCopy
        );


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TCQCIntfLocalTimeText& operator=
        (
            const   TCQCIntfLocalTimeText&  iwdgToAssign
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


    protected :
        // -------------------------------------------------------------------
        //  Protected , inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActiveUpdate
        (
            const tCIDLib::TBoolean         bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
        )   override;

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
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfLocalTimeText, TCQCIntfTimeTextBase)
        DefPolyDup(TCQCIntfLocalTimeText)
};


#pragma CIDLIB_POPPACK

