//
// FILE NAME: CQCIntfEng_States.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/28/2004
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
//  This is the header for the CQCIntfEng_States.cpp file, which implements
//  a set of classes that define a set of 'states' for a template. Each
//  state looks at the value of one or more fields and sets itself to either
//  a true of false value. These states can be used by widgets in the
//  interface to adjust themselves in various supported ways (visible or not,
//  enabled or not, etc...)
//
//  Each state references one or more fields via one of the standard statement
//  types supported by the CQCKit value objects. So the value of the state
//  is a combination of the true/false evaluations of each of the statements
//  that it contains, combined via logical operators (AND, OR, NOT.)
//
//  TCQCIntfStateItem represents a single item in a state, i.e. a field
//  whose value is to be evaluated, the statement used to evaluate it, and
//  and any data associated with that statement (such as the reg ex or
//  any fixed values to compare against.)
//
//  TCQCIntfState represents a single state. It contains a set of state items
//  and will when asked evaluate them and set it's state to true or false.
//
//  TCQCIntfStateList represents all of the states an interface manages,
//  and it holds all of the state objects. It also provides a cache of
//  field value objects for all of the fields referenced by any states it
//  contains. This way, we don't have each state redundantly getting copies
//  of the same field value out of the field polling engine.
//
//  All of this stuff works in terms of the standard polling engine, which
//  is provided to us in those methods where it's needed. The polling engine
//  facility also provides a class that the state item uses to keep track
//  of its field. That class does the bulk of the work for us.
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
//  CLASS: TCQCIntfStateItem
// PREFIX: isti
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfStateItem : public TObject, public MStreamable
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStateItem();

        TCQCIntfStateItem
        (
            const   TString&                strDescription
            , const TString&                strDevice
            , const TString&                strField
        );

        TCQCIntfStateItem
        (
            const   TCQCExpression&         exprEval
        );

        TCQCIntfStateItem
        (
            const   TString&                strDevice
            , const TString&                strField
            , const TCQCExpression&         exprEval
        );

        TCQCIntfStateItem
        (
            const   TCQCIntfStateItem&      itemToCopy
        );

        ~TCQCIntfStateItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=
        (
            const   TCQCIntfStateItem&      itemToAssign
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCIntfStateItem&      itemToAssign
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCIntfStateItem&      itemToAssign
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bNegated() const;

        tCIDLib::TBoolean bNegated
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCQCIntfEng::EExprResults eEvaluate
        (
                    TCQCPollEngine&         polleToUse
            , const TStdVarsTar&            ctarGlobalVars
        );

        const TCQCExpression& exprEval() const;

        const TCQCFldDef& flddAssoc() const;

        tCIDLib::TVoid QueryAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const;

        tCIDLib::TVoid RegisterFields
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        );

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        const TString& strDescription() const;

        const TString& strDescription
        (
            const   TString&                strToSet
        );

        const TString& strMoniker() const;

        const TString& strField() const;

        tCIDLib::TVoid Set
        (
            const   TString&                strMoniker
            , const TString&                strField
            , const TCQCExpression&         exprEval
        );

        tCIDLib::TVoid SetAttr
        (
            const   TAttrData&              adatNew
            , const TAttrData&              adatOld
        );

        tCIDLib::TVoid SetField
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const;


        // Temporarily to support changes in 2.5. Can be removed after 3.0
        tCIDLib::TVoid Update
        (
            const   TCQCFldCache&           cfcSrc
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_cfpiAssoc
        //      This is the polling engine field info class for our associated
        //      class. It does most of the work for us of managing the field
        //      and keeping up with its value and registering it with the
        //      polling engine and so forth.
        //
        //      The moniker and field name of our associated field is
        //      stored in here. But this object is not persisted. We pull
        //      those values out when we stream ourself, and then put them
        //      back into the object when we are streamed back in. This avoids
        //      the need for redundant storage of that info.
        //
        //  m_eLastRes
        //      The last result we got. We store it because our containing
        //      state class have to re-evaluate every time we get a change
        //      notification. So all of the state items that weren't affected
        //      would otherwise have to be re-evaluated again otherwise.
        //
        //  m_exprEval
        //      The expression that we evaluate.
        // -------------------------------------------------------------------
        TCQCFldPollInfo             m_cfpiAssoc;
        tCQCIntfEng::EExprResults   m_eLastRes;
        TCQCExpression              m_exprEval;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfStateItem,TObject)
};



// ---------------------------------------------------------------------------
//  CLASS: TCQCIntfState
// PREFIX: ist
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfState : public TObject, public MStreamable
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfState();

        TCQCIntfState
        (
            const   TString&                strName
            , const TString&                strDescription
        );

        TCQCIntfState
        (
            const   TCQCIntfState&          istSrc
        );

        ~TCQCIntfState();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=
        (
            const   TCQCIntfState&          istSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCIntfState&          istSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCIntfState&          istSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bNegated() const;

        tCIDLib::TBoolean bNegated
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bEvaluate
        (
                    TCQCPollEngine&         polleToUse
            , const TStdVarsTar&            ctarGlobalVars
        );

        tCIDLib::TBoolean bValue() const;

        tCIDLib::TCard4 c4AddItem
        (
            const   TCQCIntfStateItem&      istiToAdd
        );

        tCIDLib::TCard4 c4ItemCount() const;

        tCIDLib::TVoid DeleteAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid DeleteExpr
        (
            const   TString&                strNameToDel
        );

        tCQCKit::ELogOps eLogOp() const;

        tCQCKit::ELogOps eLogOp
        (
            const   tCQCKit::ELogOps        eToSet
        );

        const TCQCIntfStateItem& istiAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TCQCIntfStateItem& istiAt
        (
            const   tCIDLib::TCard4         c4At
        );

        TCQCIntfStateItem* pistiFind
        (
            const   TString&                strNameToFind
        );

        tCIDLib::TVoid RegisterFields
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        );

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        const TString& strDescription() const;

        const TString& strDescription
        (
            const   TString&                strToSet
        );

        const TString& strName() const;

        const TString& strName
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const;


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
        //  A list of state items, which make up the states we manage.
        // -------------------------------------------------------------------
        using TItemList = TVector<TCQCIntfStateItem>;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bNegated
        //      Indicates whether the return should be the evaluated true
        //      false value, or it's negation.
        //
        //  m_bValue
        //      The last evaluated value of this state. This isn't persisted.
        //
        //  m_colItems
        //      The list of state items that make up our state. Each item
        //      presents the evaluation of an expression against a field
        //      value.
        //
        //  m_strDescription
        //      A short descriptive string to provide a reminder of what
        //      the state represents, for human consumption.
        //
        //  m_strName
        //      The name of the state. This is what the user sees in a list
        //      when he selects a state for editing or for use from an intf
        //      widget.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bNegated;
        tCIDLib::TBoolean   m_bValue;
        TItemList           m_colItems;
        tCQCKit::ELogOps    m_eLogOp;
        TString             m_strDescription;
        TString             m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfState,TObject)
};


// ---------------------------------------------------------------------------
//  CLASS: TCQCIntfStateList
// PREFIX: istl
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfStateList : public TObject, public MStreamable
{
    public  :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TIndexList = TFundVector<tCIDLib::TCard4>;


        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStateList();

        TCQCIntfStateList
        (
            const   TCQCIntfStateList&      istlSrc
        );

        ~TCQCIntfStateList();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfStateList& operator=
        (
            const   TCQCIntfStateList&      istlSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCIntfStateList&      istlSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCIntfStateList&      istlSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bEvaluate
        (
                    TCQCPollEngine&         polleToUse
            , const TStdVarsTar&            ctarGlobalVars
        );

        tCIDLib::TBoolean bFindState
        (
            const   TString&                strToFind
            ,       tCIDLib::TCard4&        c4StateInd
        )   const;

        tCIDLib::TBoolean bGetStateName
        (
            const   tCIDLib::TCard4         c4At
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bGetStateResult
        (
            const   TIndexList&             fcolIndices
            , const tCQCKit::ELogOps        eLogOp
        )   const;

        tCIDLib::TCard4 c4AddState
        (
            const   TCQCIntfState&          istNew
        );

        tCIDLib::TCard4 c4StateCount() const;

        tCIDLib::TVoid DeleteAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid DeleteState
        (
            const   TString&                strName
        );

        const TCQCIntfState& istAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TCQCIntfState& istAt
        (
            const   tCIDLib::TCard4         c4At
        );

        TCQCIntfState* pistFind
        (
            const   TString&                strNameToFind
        );

        tCIDLib::TVoid SetStateAt
        (
            const   tCIDLib::TCard4         c4At
            , const TCQCIntfState&          istToSet
        );

        tCIDLib::TVoid RegisterFields
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        );

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const;


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
        //  A list of states that make up all of the states that are
        //  associated with a given template.
        // -------------------------------------------------------------------
        using TStateList = TVector<TCQCIntfState>;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bFirstTime
        //      The first time bEvaluate it called, we want to always indicate that
        //      there are changes so that the states will be evaluated the first
        //      time around. So we use this as a runtime only, one shot flag.
        //
        //  m_colStates
        //      The list of states that make up the whole state list. This is
        //      persisted.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bFirstTime;
        TStateList          m_colStates;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfStateList,TObject)
};


#pragma CIDLIB_POPPACK


