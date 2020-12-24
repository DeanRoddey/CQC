//
// FILE NAME: GenProtoS_StateMachine.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This is the header for the GenProtoS_StateMachine.cpp file, which
//  defines the the TGenProtoStateInfo class. THis class represents a single
//  state in the state machine used to pull legal messages from the incoming
//  serial data.
//
//  Each state has a name, which is the name of the state, and an array of
//  state transitions. Each state transition is a boolean expression to run
//  and a new state to move to if the expression is true. The new states are
//  represented as names, but are just converted into an index into the array.
//  Since a transition can only move forward through states, i.e. it cannot
//  transition to a state before itself, we have to parse them all and then
//  do a second pass to do the lookups and set the indexes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoStateInfo
//  PREFIX: stati
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoStateInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  We have a circular reference issue here, so we don't use the
        //  typedefs in the main header, and have to create our own.
        // -------------------------------------------------------------------
        typedef TKeyedHashSet<TGenProtoVarInfo, TString, TStringKeyOps> TVarList;
        typedef TVector<TGenProtoFldInfo>                               TFldList;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoStateInfo
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4Index
        );

        ~TGenProtoStateInfo();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddStoreOp
        (
                    TGenProtoVarInfo* const pvariTarget
            ,       TGenProtoExprNode* const pnodeValue
        );

        tCIDLib::TVoid AddTransition
        (
            const   TString&                strNextStateName
            ,       TGenProtoExprNode* const pnodeToAdopt
        );

        tCIDLib::TCard4 c4NextState
        (
                    TGenProtoCtx&           ctxToUse
        );

        tCIDLib::TCard4 c4TransCount() const;

        const TString& strName() const;

        const TString& strTransAt
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        tCIDLib::TVoid SetIndices
        (
            const   TString&                strStateName
            , const tCIDLib::TCard4         c4StateIndex
        );

        tCIDLib::TVoid StoreValues
        (
                    TGenProtoCtx&           ctxThis
        );

        tCIDLib::TVoid Validate();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors
        // -------------------------------------------------------------------
        TGenProtoStateInfo();
        TGenProtoStateInfo(const TGenProtoStateInfo&);
        tCIDLib::TVoid operator=(const TGenProtoStateInfo&);


        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TRefVector<TGenProtoStoreOp> TStoreOpList;


        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        class TStateTrans
        {
            public :
                // -----------------------------------------------------------
                //  Constructors and destructor
                // -----------------------------------------------------------
                TStateTrans
                (
                    const   TString&                strNextName
                    ,       TGenProtoExprNode* const pnodeToAdopt
                );

                ~TStateTrans();


                // -----------------------------------------------------------
                //  Public, non-virutal methods
                // -----------------------------------------------------------
                tCIDLib::TCard4 c4NextStateInd() const;

                tCIDLib::TCard4 c4NextStateInd
                (
                    const   tCIDLib::TCard4         c4ToSet
                );

                TGenProtoExprNode* pnodeTrans();

                const TString& strNextName() const;


            private :
                // -----------------------------------------------------------
                //  Private data members
                //
                //  m_c4NextStateInd
                //      The index of the state that we transition to if the
                //      expression is true. The name is set first, then a
                //      second pass is made and the indexes are set, to speed
                //      up the transitions.
                //
                //  m_pnodeTrans
                //      The root node of the expression that is executed when
                //      this state is evaulated. It must be boolean. If it
                //      evaluates to true, then the transition is taken.
                //
                //  m_strNextName
                //      The name of the next state. It is both for display
                //      and reporting purposes, and since the index cannot be
                //      known until all the states are parsed, we would have
                //      to store it anyway, to allow for a second pass that
                //      resolves names to indexes.
                // -----------------------------------------------------------
                tCIDLib::TCard4     m_c4NextStateInd;
                TGenProtoExprNode*  m_pnodeTrans;
                TString             m_strNextName;
        };
        typedef TRefVector<TStateTrans> TTransList;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Index
        //      The index of this state in the list of states. The transitions
        //      will refer to these state indexes for speed.
        //
        //  m_colTransitions
        //      The list of transitions that this state machine can take.
        //
        //  m_pcolStores
        //      This is our list of store operations. Its a pointer because
        //      often there won't be any stores, so we won't have to allocate
        //      it.
        //
        //  m_strName
        //      The name of this state.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4Index;
        TTransList      m_colTransitions;
        TStoreOpList*   m_pcolStoreOps;
        TString         m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoStateInfo,TObject)
};

#pragma CIDLIB_POPPACK


