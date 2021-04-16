//
// FILE NAME: GenProtoS_ExprMathFuncs_.hpp
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
//  This is the header file for the GenProtoS_ExprMathFuncs.cpp file, which
//  defines some simple derivatives of the basic expression node class. These
//  are all for the math 'functions' that the expression language supports.
//  These all take one or more child expressions and do something with the
//  evaluated values.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoAddNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoAddNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoAddNode
        (
                    TGenProtoExprNode* const pnodeLHS
            ,       TGenProtoExprNode* const pnodeRHS
        );

        TGenProtoAddNode
        (
            const   TGenProtoAddNode&       nodeToCopy
        );

        ~TGenProtoAddNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoAddNode& operator=
        (
            const   TGenProtoAddNode&       nodeToAssign
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
        //  m_pnodeLHS
        //  m_pnodeRHS
        //      Our child expressions. We have a left and right hand side,
        //      and the two are added together and the result returned. Neither
        //      are affected.
        // -------------------------------------------------------------------
        TGenProtoExprNode*  m_pnodeLHS;
        TGenProtoExprNode*  m_pnodeRHS;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoAddNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoAddNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoDivNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoDivNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoDivNode
        (
                    TGenProtoExprNode* const pnodeLHS
            ,       TGenProtoExprNode* const pnodeRHS
        );

        TGenProtoDivNode
        (
            const   TGenProtoDivNode&       nodeToCopy
        );

        ~TGenProtoDivNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoDivNode& operator=
        (
            const   TGenProtoDivNode&       nodeToAssign
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
        //  m_pnodeLHS
        //  m_pnodeRHS
        //      Our child expressions. We have a left and right hand side
        //      which are mutltiplied together.
        // -------------------------------------------------------------------
        TGenProtoExprNode*  m_pnodeLHS;
        TGenProtoExprNode*  m_pnodeRHS;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoDivNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoDivNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoEqualsNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoEqualsNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoEqualsNode
        (
                    TGenProtoExprNode* const pnodeLHS
            ,       TGenProtoExprNode* const pnodeRHS
        );

        TGenProtoEqualsNode
        (
            const   TGenProtoEqualsNode&    nodeToCopy
        );

        ~TGenProtoEqualsNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoEqualsNode& operator=
        (
            const   TGenProtoEqualsNode&    nodeToAssign
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
        //  m_pnodeLHS
        //  m_pnodeRHS
        //      Our child expressions. We have a left and right hand side
        //      which are compared.
        // -------------------------------------------------------------------
        TGenProtoExprNode*  m_pnodeLHS;
        TGenProtoExprNode*  m_pnodeRHS;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoEqualsNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoEqualsNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoMulNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoMulNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoMulNode
        (
                    TGenProtoExprNode* const pnodeLHS
            ,       TGenProtoExprNode* const pnodeRHS
        );

        TGenProtoMulNode
        (
            const   TGenProtoMulNode&       nodeSrc
        );

        ~TGenProtoMulNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoMulNode& operator=
        (
            const   TGenProtoMulNode&       nodeSrc
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
        //  m_pnodeLHS
        //  m_pnodeRHS
        //      Our child expressions. We have a left and right hand side
        //      which are mutltiplied together.
        // -------------------------------------------------------------------
        TGenProtoExprNode*  m_pnodeLHS;
        TGenProtoExprNode*  m_pnodeRHS;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoMulNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoMulNode);
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoNumCompNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoNumMagNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoNumMagNode
        (
                    TGenProtoExprNode* const pnodeLHS
            ,       TGenProtoExprNode* const pnodeRHS
            , const tCIDLib::TBoolean       bGreater
        );

        TGenProtoNumMagNode
        (
            const   TGenProtoNumMagNode&    nodeSRc
        );

        ~TGenProtoNumMagNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoNumMagNode& operator=
        (
            const   TGenProtoNumMagNode&    nodeSrc
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
        //  m_bGreater
        //      Indicates whether we do a greater than or lesser than
        //      comparison.
        //
        //  m_pnodeLHS
        //  m_pnodeRHS
        //      Our child expressions. We have a left and right hand side,
        //      and the two are compared for relative magnitude. They both
        //      must be numeric nodes.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bGreater;
        TGenProtoExprNode*  m_pnodeLHS;
        TGenProtoExprNode*  m_pnodeRHS;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoNumMagNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoNumMagNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoRangeRotNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoRangeRotNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoRangeRotNode
        (
                    TGenProtoExprNode* const pnodeOrgVal
            ,       TGenProtoExprNode* const pnodeDirection
            ,       TGenProtoExprNode* const pnodeRangeLow
            ,       TGenProtoExprNode* const pnodeRangeHigh
            ,       TGenProtoExprNode* const pnodeWrapAround
        );

        TGenProtoRangeRotNode
        (
            const   TGenProtoRangeRotNode&       nodeSrc
        );

        ~TGenProtoRangeRotNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoRangeRotNode& operator=
        (
            const   TGenProtoRangeRotNode&       nodeSrc
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
        //  m_pnodeDirection
        //  m_pnode
        //      Our child expressions. We have four of them. We have the
        //      direction to rotate (true is up and false is down), the
        //      original value to rotate from, and the low and high values of
        //      the range we rotate within. The WrapAround value is a boolean
        //      and indicates if we stop at the limits or wrap around to the
        //      other side.
        // -------------------------------------------------------------------
        TGenProtoExprNode*  m_pnodeDirection;
        TGenProtoExprNode*  m_pnodeOrgVal;
        TGenProtoExprNode*  m_pnodeRangeHigh;
        TGenProtoExprNode*  m_pnodeRangeLow;
        TGenProtoExprNode*  m_pnodeWrapAround;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoRangeRotNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoRangeRotNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoRoundFloatNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoRoundFloatNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoRoundFloatNode
        (
                    TGenProtoExprNode* const pnodeValue
            , const tCIDLib::ERoundTypes    eType
        );

        TGenProtoRoundFloatNode
        (
            const   TGenProtoRoundFloatNode& nodeSrc
        );

        ~TGenProtoRoundFloatNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoRoundFloatNode& operator=
        (
            const   TGenProtoRoundFloatNode& nodeSrc
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
        //  m_eType
        //      Indicates what style of rounding to do.
        //
        //  m_pnodeValue
        //      Our child expression that provides the floating point value to
        //      round.
        // -------------------------------------------------------------------
        tCIDLib::ERoundTypes    m_eType;
        TGenProtoExprNode*      m_pnodeValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoRoundFloatNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoRoundFloatNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoScaleRngNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoScaleRngNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoScaleRngNode
        (
                    TGenProtoExprNode* const pnodeVal
            ,       TGenProtoExprNode* const pnodeMinVal
            ,       TGenProtoExprNode* const pnodeMaxVal
            ,       TGenProtoExprNode* const pnodeMinRng
            ,       TGenProtoExprNode* const pnodeMaxRng
        );

        TGenProtoScaleRngNode
        (
            const   TGenProtoScaleRngNode&  nodeSrc
        );

        ~TGenProtoScaleRngNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoScaleRngNode& operator=
        (
            const   TGenProtoScaleRngNode&  nodeSrc
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
        //  m_pnodeVal
        //      The value to scale
        //
        //  m_pnodeMaxVal
        //  m_pnodeMinVal
        //      The values that indicate the range of the source scale.
        //
        //  m_pnodeMaxRng
        //  m_pnodeMinRng
        //      The values that indicate the range to scale the source values
        //      to.
        // -------------------------------------------------------------------
        TGenProtoExprNode*  m_pnodeVal;
        TGenProtoExprNode*  m_pnodeMaxRng;
        TGenProtoExprNode*  m_pnodeMinRng;
        TGenProtoExprNode*  m_pnodeMaxVal;
        TGenProtoExprNode*  m_pnodeMinVal;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoScaleRngNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoScaleRngNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoSubNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoSubNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoSubNode
        (
                    TGenProtoExprNode* const pnodeLHS
            ,       TGenProtoExprNode* const pnodeRHS
        );

        TGenProtoSubNode
        (
            const   TGenProtoSubNode&       nodeSrc
        );

        ~TGenProtoSubNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoSubNode& operator=
        (
            const   TGenProtoSubNode&       nodeSrc
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
        //  m_pnodeLHS
        //  m_pnodeRHS
        //      Our child expressions. We have a left and right hand side,
        //      and the right is subtracted from the left and the result
        //      returned. Neither is affected.
        // -------------------------------------------------------------------
        TGenProtoExprNode*  m_pnodeLHS;
        TGenProtoExprNode*  m_pnodeRHS;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoSubNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoSubNode)
};


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoTranscNode
//  PREFIX: node
//
//  Applies some transcendental math operation to a value.
// ---------------------------------------------------------------------------
class TGenProtoTranscNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoTranscNode
        (
                    TGenProtoExprNode* const pnodeValue
            ,       TGenProtoExprNode* const pnodeType
        );

        TGenProtoTranscNode
        (
            const   TGenProtoTranscNode&       nodeSrc
        );

        ~TGenProtoTranscNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoTranscNode& operator=
        (
            const   TGenProtoTranscNode&       nodeSrc
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
        //  m_eType
        //      We require the type value to be a constant, so we can evaluate
        //      it during the post-parse validation after checking it. So we
        //      can set this value and much more efficiently do our thing during
        //      the actual evaluation of the expression.
        //
        //  m_pnodeType
        //  m_pnodeVal
        //      Our child expressions. We get a value to apply the operation
        //      to, and an operation type.
        // -------------------------------------------------------------------
        tGenProtoS::ETransTypes m_eType;
        TGenProtoExprNode*      m_pnodeType;
        TGenProtoExprNode*      m_pnodeVal;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoTranscNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoTranscNode)
};


#pragma CIDLIB_POPPACK


