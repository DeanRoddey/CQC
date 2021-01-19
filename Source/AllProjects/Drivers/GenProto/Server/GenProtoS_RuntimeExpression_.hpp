//
// FILE NAME: GenProtoS_RuntimeExpression_.hpp
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
//  This is the header file for the GenProtoS_RuntimeExpression.cpp file,
//  which defines some expression class derivatives. These all represent
//  runtime values, such as the current value of driver fields.
//
//  The expressions that access the current input byte or the input our output
//  buffers, get their content from the expression context that is passed to
//  them when they are evaluated.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoCheckSumNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoCheckSumNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoCheckSumNode
        (
            const   tGenProtoS::ESpecNodes  eExprType
            , const tGenProtoS::ETypes      eType
            ,       TGenProtoExprNode* const pnodeOfs
            ,       TGenProtoExprNode* const pnodeLen
            , const tGenProtoS::ECheckSums  eCheckSumType
        );

        TGenProtoCheckSumNode
        (
            const   TGenProtoCheckSumNode&  nodesrc
        );

        ~TGenProtoCheckSumNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoCheckSumNode& operator=
        (
            const   TGenProtoCheckSumNode&  nodeSRc
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
        //  m_eCheckSumType
        //      The checksum algorithm that we are to use in this case.
        //
        //  m_eExprType
        //      Indicates the specific buffer that it accesses.
        //
        //  m_pnodeLen
        //      The expression that yields the number of bytes to checksum
        //
        //  m_pnodeOfs
        //      The expression which yieds the offset within the buffer to
        //      start. Usually it will be zero but not always.
        // -------------------------------------------------------------------
        tGenProtoS::ECheckSums  m_eCheckSumType;
        tGenProtoS::ESpecNodes  m_eExprType;
        TGenProtoExprNode*      m_pnodeLen;
        TGenProtoExprNode*      m_pnodeOfs;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoCheckSumNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoCheckSumNode)
};


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoCRC16Node
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoCRC16Node : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoCRC16Node
        (
            const   tGenProtoS::ESpecNodes  eExprType
            ,       TGenProtoExprNode* const pnodeOfs
            ,       TGenProtoExprNode* const pnodeLen
            ,       TGenProtoExprNode* const pnodePoly
            ,       TGenProtoExprNode* const pnodeCount
        );

        TGenProtoCRC16Node
        (
            const   TGenProtoCRC16Node&     nodeSrc
        );

        ~TGenProtoCRC16Node();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoCRC16Node& operator=
        (
            const   TGenProtoCRC16Node&     nodeSrc
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
        //      Indicates the specific buffer that it accesses.
        //
        //  m_pnodeCount
        //      The number of times we should loop on each character.
        //
        //  m_pnodeLen
        //      The expression that yields the number of bytes to CRC
        //
        //  m_pnodeOfs
        //      The expression which yieds the offset within the buffer to
        //      start. Usually it will be zero but not always.
        //
        //  m_pnodePoly
        //      Provides the polynomial to use. Must be Card2.
        // -------------------------------------------------------------------
        tGenProtoS::ESpecNodes  m_eExprType;
        TGenProtoExprNode*      m_pnodeCount;
        TGenProtoExprNode*      m_pnodeLen;
        TGenProtoExprNode*      m_pnodeOfs;
        TGenProtoExprNode*      m_pnodePoly;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoCRC16Node,TGenProtoExprNode)
        DefPolyDup(TGenProtoCRC16Node)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoFldValNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoFldValNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoFldValNode
        (
                    TGenProtoFldInfo* const pfldiSrc
        );

        TGenProtoFldValNode
        (
            const   TGenProtoFldValNode&    nodeSrc
        );

        ~TGenProtoFldValNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoFldValNode& operator=
        (
            const   TGenProtoFldValNode&    nodeSrc
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
        //  m_c4SerialNum
        //      The last serial number we got from the field when we read
        //      it.
        //
        //  m_pfldiSrc
        //      A pointer to the field info object that holds the information
        //      about, and current value of, the field we are attached to.
        //      We don't own it, we just point to it, so we can safely just
        //      do a shallow copy when assigned or copied.
        //
        //  m_strValue
        //      The last value we read from the field.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4SerialNum;
        TGenProtoFldInfo*   m_pfldiSrc;
        TString             m_strValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoFldValNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoFldValNode)
};


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoInputByteNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoInputByteNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoInputByteNode();

        TGenProtoInputByteNode
        (
            const   TGenProtoInputByteNode& nodeSrc
        );

        ~TGenProtoInputByteNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoInputByteNode& operator=
        (
            const   TGenProtoInputByteNode& nodeSrc
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
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoInputByteNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoInputByteNode)
};


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoVarValNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoVarValNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoVarValNode
        (
                    TGenProtoVarInfo* const pvariSrc
        );

        TGenProtoVarValNode
        (
            const   TGenProtoVarValNode&    nodeSrc
        );

        ~TGenProtoVarValNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoVarValNode& operator=
        (
            const   TGenProtoVarValNode&    nodeSrc
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
        //  m_pvariSrc
        //      A pointer to the variable info object that holds the info
        //      about, and current value of, the variable we are attached to.
        //      We don't own it, we just point to it, so we can safely just
        //      do a shallow copy when assigned or copied.
        // -------------------------------------------------------------------
        TGenProtoVarInfo*   m_pvariSrc;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoVarValNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoVarValNode)
};


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoWriteValNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoWriteValNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoWriteValNode
        (
            const   tCQCKit::EFldTypes      eType
        );

        TGenProtoWriteValNode
        (
            const   TGenProtoWriteValNode&  nodeSrc
        );

        ~TGenProtoWriteValNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoWriteValNode& operator=
        (
            const   TGenProtoWriteValNode&  nodeSrc
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
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoWriteValNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoWriteValNode)
};

#pragma CIDLIB_POPPACK



