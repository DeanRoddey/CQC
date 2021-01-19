//
// FILE NAME: GenProtoS_Expression.hpp
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
//  This is the header file for the GenProtoS_Expression.cpp file, which
//  defines a core class used to represent an 'expression'. The protocol files
//  are full of expressions which can be arbitrarily nested, and which resolve
//  in the end to a single value of a particular type.
//
//  So every time we parse an expression, we build a tree of expression nodes,
//  each of which either represents a constant or variable value or a 'function'
//  call that takes one or more child expressions and does something with its
//  resolved value.
//
//  So we have a base expression node class, and then many derivations
//  of the base class, for specific types of expressions. Only this base class
//  needs to be exported to the outside world, because it is visible as a
//  template element type in the main driver data class. The others are only
//  created and used internally.
//
//  The base class defines a bIsConst() method, which all derivatives must
//  override. For const nodes, they just return true. Others either return
//  false or recurse on their children and return the logical result. This
//  lets us do an optimization pass and collapse const expressions down into
//  single constant nodes so that they don't get evaluated over and over again
//  at runtime.
//
//  It also defines an Evaluate() method which is used to 'run the tree' at
//  runtime and get the final result. Each derivative must override this and
//  store its value upon demand. It receives a context object, which passes in
//  some information that is only known at runtime.
//
//  We also define here a simple derivative that holds an arbitrary number of
//  child nodes. This is kind of common, so having a base node to handle the
//  grunt work is nice. They still have to provide the evaluation and validation
//  but managing the children this class can do.
//
// CAVEATS/GOTCHAS:
//
//  1)  We cannot be compile time type safe in this stuff. So each node has
//      a type enum that indicates its type. The base class offers virtual
//      getter methods for each of the possible data types. Derived types
//      override the one that is valid for them. The others get the base
//      class' default implementation, and we throw an exception if they are
//      called.
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

class TGenProtoCtx;


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoExprNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoExprNode : public TObject, public MDuplicable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        ~TGenProtoExprNode();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bIsConst() const = 0;

        virtual tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        ) = 0;

        virtual tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        );

        virtual tCIDLib::TVoid PostParseValidation();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsCardType() const;

        tCIDLib::TBoolean bIsCharType() const;

        tCIDLib::TBoolean bIsFloatType() const;

        tCIDLib::TBoolean bIsIntType() const;

        const TGenProtoExprVal& evalCur() const;

        TGenProtoExprVal& evalCur();

        tGenProtoS::ETypes eType() const;

        const TString& strDescription() const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TGenProtoExprNode
        (
            const   tGenProtoS::ETypes      eType
            , const TString&                strDescr
        );

        TGenProtoExprNode
        (
            const   TGenProtoExprNode&      nodeSrc
        );
        TGenProtoExprNode(TGenProtoExprNode&&) = delete;

        TGenProtoExprNode& operator=
        (
            const   TGenProtoExprNode&      nodeSrc
        );
        TGenProtoExprNode& operator=(TGenProtoExprNode&&) = delete;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckIsBoolType
        (
            const   TGenProtoExprNode&      nodeToCheck
            , const tCIDLib::TCard4         c4ParmNum
        );

        tCIDLib::TVoid CheckIsCardType
        (
            const   TGenProtoExprNode&      nodeToCheck
            , const tCIDLib::TCard4         c4ParmNum
        );

        tCIDLib::TVoid CheckIsCharType
        (
            const   TGenProtoExprNode&      nodeToCheck
            , const tCIDLib::TCard4         c4ParmNum
        );

        tCIDLib::TVoid CheckIsConst
        (
            const   TGenProtoExprNode&      nodeToCheck
            , const tCIDLib::TCard4         c4ParmNum
        );

        tCIDLib::TVoid CheckIsFloatType
        (
            const   TGenProtoExprNode&      nodeToCheck
            , const tCIDLib::TCard4         c4ParmNum
        );

        tCIDLib::TVoid CheckIsIntType
        (
            const   TGenProtoExprNode&      nodeToCheck
            , const tCIDLib::TCard4         c4ParmNum
        );

        tCIDLib::TVoid CheckIsNumericType
        (
            const   TGenProtoExprNode&      nodeToCheck
            , const tCIDLib::TCard4         c4ParmNum
        );

        tCIDLib::TVoid CheckIsStringType
        (
            const   TGenProtoExprNode&      nodeToCheck
            , const tCIDLib::TCard4         c4ParmNum
        );

        tCIDLib::TVoid CheckType
        (
            const   TGenProtoExprNode&      nodeToCheck
            , const tGenProtoS::ETypes      eExpectedType
            , const tCIDLib::TCard4         c4ParmNum
        );

        TGenProtoExprNode* pnodeOptimize
        (
                    TGenProtoCtx&           ctxThis
            ,       TGenProtoExprNode* const pnodeToOptimize
            , const tCIDLib::TBoolean       bDeleteOrg = kCIDLib::True
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_evalCur
        //      Every expression evalutes to some value of some type. So we
        //      provide one of our smart union expression value objects here
        //      in the base class for all of them to use. They indicate their
        //      desired evaluation type in the ctor, and we set it on the
        //      value object, which fixes the node's return type.
        //
        //  m_strDescr
        //      A descriptive string that is provided by the derived class,
        //      which can be used in error messages. Its not supposed to be
        //      narrative, but something like "Constant Integer" or whatnot.
        // -------------------------------------------------------------------
        TGenProtoExprVal    m_evalCur;
        TString             m_strDescr;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoExprNode,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoArbChildNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoArbChildNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TRefVector<TGenProtoExprNode>   TChildList;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoArbChildNode
        (
            const   tGenProtoS::ETypes      eType
            , const TString&                strDesc
        );

        TGenProtoArbChildNode
        (
            const   TGenProtoArbChildNode&  nodeToCopy
        );

        ~TGenProtoArbChildNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoArbChildNode& operator=
        (
            const   TGenProtoArbChildNode&  nodeToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const;

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddChild
        (
                    TGenProtoExprNode* const pnodeToAdopt
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        const TChildList& colChildren() const;

        TChildList& colChildren();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colChildren
        //      Our list of child nodes. We allow an arbitrary number of
        //      child expressions.
        // -------------------------------------------------------------------
        TChildList  m_colChildren;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoArbChildNode,TGenProtoExprNode)
};

#pragma CIDLIB_POPPACK


