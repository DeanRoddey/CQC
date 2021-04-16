//
// FILE NAME: GenProtoS_ExprFunctions_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/25/2002
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
//  This is the header file for the GenProtoS_ExprFunctions.cpp file, which
//  defines some simple derivatives of the basic expression node class. These
//  are all for the common 'functions' that the expression language supports.
//  These all take one or more child expressions and do something with the
//  evaluated values.
//
// CAVEATS/GOTCHAS:
//
//  1)  The bit fiddling function nodes only take cardinal (unsigned) values
//      and only return them.
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoArrayExtractNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoArrayExtractNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoArrayExtractNode
        (
            const   tGenProtoS::ESpecNodes  eExprType
            ,       TGenProtoExprNode* const pnodeOffset
            , const tGenProtoS::ETypes      eType
        );

        TGenProtoArrayExtractNode
        (
            const   TGenProtoArrayExtractNode& nodeSrc
        );

        ~TGenProtoArrayExtractNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoArrayExtractNode& operator=
        (
            const   TGenProtoArrayExtractNode& nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const  final;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid PostParseValidation()  final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eExprType
        //      Indicates which type of expression we are, i.e. which buffer
        //      we access.
        //
        //  m_pnodeOffset
        //      The expression that yields the offset into the array that we
        //      are to extract at.
        // -------------------------------------------------------------------
        tGenProtoS::ESpecNodes  m_eExprType;
        TGenProtoExprNode*      m_pnodeOffset;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoArrayExtractNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoArrayExtractNode)
};


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoArrayLenNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoArrayLenNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoArrayLenNode
        (
            const   tGenProtoS::ESpecNodes  eExprType
        );

        TGenProtoArrayLenNode
        (
            const   TGenProtoArrayLenNode&  nodeSrc
        );

        ~TGenProtoArrayLenNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoArrayLenNode& operator=
        (
            const   TGenProtoArrayLenNode&  nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const  final;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eExprType
        //      Indicates which type of expression we are, i.e. which buffer
        //      we access.
        // -------------------------------------------------------------------
        tGenProtoS::ESpecNodes  m_eExprType;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoArrayLenNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoArrayLenNode)
};


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoBitOpsNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoBitOpsNode : public TGenProtoArbChildNode
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        enum class EBitOps
        {
            And
            , Or
            , Xor
        };


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static EBitOps eTokenToOp
        (
            const   tGenProtoS::ETokens     eToXlat
        );

        static const tCIDLib::TCh* pszXlatBitOp
        (
            const   EBitOps                 eToXlat
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoBitOpsNode
        (
            const   EBitOps                 eOp
        );

        TGenProtoBitOpsNode
        (
            const   tGenProtoS::ETokens     eToken
        );

        TGenProtoBitOpsNode
        (
            const   TGenProtoBitOpsNode&    nodeSrc
        );

        ~TGenProtoBitOpsNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoBitOpsNode& operator=
        (
            const   TGenProtoBitOpsNode&    nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid PostParseValidation()  final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eOp
        //      The bit op that we are to do. This lets us have one node that
        //      does all of the bit operations.
        // -------------------------------------------------------------------
        EBitOps m_eOp;

        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoBitOpsNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoBitOpsNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoBitsAreSetNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoBitsAreSetNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoBitsAreSetNode
        (
                    TGenProtoExprNode* const pnodeValue
            ,       TGenProtoExprNode* const pnodeMask
        );

        TGenProtoBitsAreSetNode
        (
            const   TGenProtoBitsAreSetNode& nodeSrc
        );

        ~TGenProtoBitsAreSetNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoBitsAreSetNode& operator=
        (
            const   TGenProtoBitsAreSetNode& nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const  final;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid PostParseValidation()  final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pnodeMask
        //  m_pnodeValue
        //      Our child expressions. The first node is a a value that we
        //      will check. The second is a mask that holds the bits to check.
        //      They must all be one. They both have to be of the cardinal
        //      type.
        // -------------------------------------------------------------------
        TGenProtoExprNode*  m_pnodeMask;
        TGenProtoExprNode*  m_pnodeValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoBitsAreSetNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoBitsAreSetNode)
};


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoBoolSelNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoBoolSelNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoBoolSelNode
        (
                    TGenProtoExprNode* const  pnodeSelVal
            ,       TGenProtoExprNode* const  pnodeTrueVal
            ,       TGenProtoExprNode* const  pnodeFalseVal
        );

        TGenProtoBoolSelNode
        (
            const   TGenProtoBoolSelNode&   nodeSrc
        );

        ~TGenProtoBoolSelNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoBoolSelNode& operator=
        (
            const   TGenProtoBoolSelNode&   nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const  final;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid PostParseValidation()  final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pnodeFalseVal
        //      If the selection value comes out false, this node is evaluated
        //      and returned.
        //
        //  m_pnodeSelVal
        //      The value of this expression determins which of the other two
        //      nodes we return as our value. It must be boolean or castable
        //      to a boolean result.
        //
        //  m_pnodeTrueVal
        //      If the selection value comes out true, this node is evaluated
        //      and returned.
        // -------------------------------------------------------------------
        TGenProtoExprNode*  m_pnodeFalseVal;
        TGenProtoExprNode*  m_pnodeSelVal;
        TGenProtoExprNode*  m_pnodeTrueVal;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoBoolSelNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoBoolSelNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoIfAllNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoIfAllNode : public TGenProtoArbChildNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoIfAllNode();

        TGenProtoIfAllNode
        (
            const   TGenProtoIfAllNode&     nodeSrc
        );

        ~TGenProtoIfAllNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoIfAllNode& operator=
        (
            const   TGenProtoIfAllNode&     nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid PostParseValidation()  final;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoIfAllNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoIfAllNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoIfAnyNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoIfAnyNode : public TGenProtoArbChildNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoIfAnyNode();

        TGenProtoIfAnyNode
        (
            const   TGenProtoIfAnyNode&     nodeSrc
        );

        ~TGenProtoIfAnyNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoIfAnyNode& operator=
        (
            const   TGenProtoIfAnyNode&     nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid PostParseValidation()  final;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoIfAnyNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoIfAnyNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoIsZeroNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoIsZeroNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoIsZeroNode
        (
                    TGenProtoExprNode* const pnodeToAdopt
        );

        TGenProtoIsZeroNode
        (
            const   TGenProtoIsZeroNode&    nodeSrc
        );

        ~TGenProtoIsZeroNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoIsZeroNode& operator=
        (
            const   TGenProtoIsZeroNode&    nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const  final;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid PostParseValidation()  final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pnodeValue
        //      Our one child expression. It must be of some numeric type.
        // -------------------------------------------------------------------
        TGenProtoExprNode*    m_pnodeValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoIsZeroNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoIsZeroNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoMapFromNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoMapFromNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoMapFromNode
        (
            const   TGenProtoMap* const     pmapSrc
            ,       TGenProtoExprNode* const pnodeMapVal
        );

        TGenProtoMapFromNode
        (
            const   TGenProtoMapFromNode&   nodeSrc
        );

        ~TGenProtoMapFromNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoMapFromNode& operator=
        (
            const   TGenProtoMapFromNode&   nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const  final;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid PostParseValidation()  final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pmapSrc
        //      A pointer to our source map. We don't own it, we just reference
        //      it, so we can just make shallow copies of it during copy and
        //      assignment.
        //
        //  m_pnodeMapVal
        //      An expression that we evaluate, and then use to look up the
        //      value in the map. It must resolve to a numeric type.
        // -------------------------------------------------------------------
        const TGenProtoMap*   m_pmapSrc;
        TGenProtoExprNode*    m_pnodeMapVal;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoMapFromNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoMapFromNode)
};




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoMapToNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoMapToNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoMapToNode
        (
            const   TGenProtoMap* const     pmapSrc
            ,       TGenProtoExprNode* const pnodeMapVal
        );

        TGenProtoMapToNode
        (
            const   TGenProtoMapToNode&     nodeSrc
        );

        ~TGenProtoMapToNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoMapToNode& operator=
        (
            const   TGenProtoMapToNode&     nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const  final;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid PostParseValidation()  final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pmapSrc
        //      A pointer to our source map. We don't own it, we just reference
        //      it, so we can just make shallow copies of it during copy and
        //      assignment.
        //
        //  m_pnodeMapVal
        //      An expression that we evaluate, and then use to look up the
        //      value in the map. It must resolve to a string type.
        // -------------------------------------------------------------------
        const TGenProtoMap* m_pmapSrc;
        TGenProtoExprNode*  m_pnodeMapVal;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoMapToNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoMapToNode)
};


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoNOTNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoNOTNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoNOTNode
        (
                    TGenProtoExprNode* const pnodeValue
        );

        TGenProtoNOTNode
        (
            const   TGenProtoNOTNode&       nodeSrc
        );

        ~TGenProtoNOTNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoNOTNode& operator=
        (
            const   TGenProtoNOTNode&       nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const  final;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid PostParseValidation()  final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pnodeValue
        //      Our child expression. It must resolve to a boolean. amd we
        //      just flip its value for our value.
        // -------------------------------------------------------------------
        TGenProtoExprNode*  m_pnodeValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoNOTNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoNOTNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoShiftNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoShiftNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoShiftNode
        (
                    TGenProtoExprNode* const pnodeValue
            ,       TGenProtoExprNode* const pnodeMask
            , const tCIDLib::TBoolean       bRight
        );

        TGenProtoShiftNode
        (
            const   TGenProtoShiftNode&     nodeSrc
        );

        ~TGenProtoShiftNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoShiftNode& operator=
        (
            const   TGenProtoShiftNode&     nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const  final;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid PostParseValidation()  final;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented ctors and operators
        // -------------------------------------------------------------------
        TGenProtoShiftNode();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bRight
        //      Indicates whether we are doing a left or right shift.
        //
        //  m_pnodeValue
        //  m_pnodeShift
        //      Our two children. The value holds the value to shift, and the
        //      shift one indicates the number of bits to shift. We rotate
        //      really, so we clip to 32 bits and then apply. Both must be
        //      cardinal valued nodes.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bRight;
        TGenProtoExprNode*  m_pnodeShift;
        TGenProtoExprNode*  m_pnodeValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoShiftNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoShiftNode)
};


#pragma CIDLIB_POPPACK


