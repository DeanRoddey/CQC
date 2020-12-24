//
// FILE NAME: CQCAct_SystemTarget.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/16/2005
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
//  This file handles the System target.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAct_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TStdSystemCmdTar,TObject)



// ---------------------------------------------------------------------------
//  Some local helpers used by the System::EvalExpr command. One parses the expression
//  and builds up a tree for evaluation. Another goes through the tree, replaces field
//  and variable references, and then evaluates it. We have a small class to hold the
//  info for the terminal nodes. Each one is either an immediate value, a variable
//  reference, or a field reference.
//
//  All immediate values are stored as text. Each expression must come down to a
//  comparison of an immediate with a variable or field, or a comparison between
//  variables and fields. In the former case, the type of the variable or field drives
//  the data type of the comparison. If the field/variable is a float, for instance,
//  the other side must be convertable to a float or the evaluation fails. We don't know
//  what any of the node data types are until evaluation time.
//
//  There's never any need to compare immediate values, since you know what the result
//  will be. If this happens, the left side will be an unknown data type since it will
//  not have been set, and the right side will then as well, so we will fail.
//
//  Operators supported are:
//
//      Boolean: AND, OR, XOR
//      Math: <, >, <=, >=, !=, ==
//
//  Parentheses define the tree structure. All expressions must be parenthesized, including
//  the outer one. So something like:
//
//  (($(Bubba.CurCount) > 15) AND (%(LVar:Test) == 10))
//
//  That evaluates to a tree like this:
//
//            ------------- AND ------------
//            |                            |
//         Greater                       Equals
//    CurCount    15                  Test     10
//
//
//  Evaluation is depth first, so left side of Greater, then right side of Greater,
//  Same for Equals, then AND is done. Each node returns a result that is used by
//  the parent.
// ---------------------------------------------------------------------------

// A type for the nodes
enum class ENTypes
{
    Unknown

    // It's a terminal
    , FieldRef
    , Literal
    , VarRef

    // It's a non-terminal (expression)
    , AND
    , OR
    , XOR

    , Equal
    , GtThan
    , GtThanEq
    , LsThan
    , LsThanEq
    , NotEqual
};

// Used during expression evaluation
enum class EExprRes
{
    False
    , True
    , Value
};

// Used during tokenization of the expression text
enum class ETokens
{
    CloseParen
    , FieldRef
    , Literal
    , OpenParen
    , VarRef
};




//
//  Looks at two data types and selects the one that would be best to use to compare
//  them.
//
static tCQCKit::EFldTypes
eBestType(const tCQCKit::EFldTypes eT1, const tCQCKit::EFldTypes eT2)
{
    // If they are the same, use that obviously
    if (eT1 == eT2)
        return eT1;

    // This is the largest numeric
    if ((eT1 == tCQCKit::EFldTypes::Time) || (eT2 == tCQCKit::EFldTypes::Time))
        return tCQCKit::EFldTypes::Time;

    if ((eT1 == tCQCKit::EFldTypes::Float) || (eT2 == tCQCKit::EFldTypes::Float))
        return tCQCKit::EFldTypes::Float;

    if ((eT1 == tCQCKit::EFldTypes::Int) || (eT2 == tCQCKit::EFldTypes::Int))
        return tCQCKit::EFldTypes::Int;

    if ((eT1 == tCQCKit::EFldTypes::Card) || (eT2 == tCQCKit::EFldTypes::Card))
        return tCQCKit::EFldTypes::Card;

    if ((eT1 == tCQCKit::EFldTypes::Boolean) || (eT2 == tCQCKit::EFldTypes::Boolean))
        return tCQCKit::EFldTypes::Boolean;

    // Strings can represent anything
    if ((eT1 == tCQCKit::EFldTypes::String) || (eT2 == tCQCKit::EFldTypes::String))
        return tCQCKit::EFldTypes::String;

    facCQCAct().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kActErrs::errcExpr_UnknownDataType
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Format
        , tCQCKit::strXlatEFldTypes(eT1)
    );

    // Make the compiler happy
    return tCQCKit::EFldTypes::Count;
}


// A node in the tree
class TTNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTNode() :

            m_eDType(tCQCKit::EFldTypes::Count)
            , m_eNType(ENTypes::Unknown)
            , m_strVal(L"[NE]")
        {
        }

        TTNode(const TTNode& nodeSrc) :

            m_eDType(nodeSrc.m_eDType)
            , m_eNType(nodeSrc.m_eNType)
            , m_strSrc(nodeSrc.m_strSrc)
            , m_strVal(nodeSrc.m_strVal)
        {
        }

        ~TTNode() {}


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        const TTNode& operator=(const TTNode& nodeSrc)
        {
            if (this != &nodeSrc)
            {
                m_eDType  = nodeSrc.m_eDType;
                m_eNType  = nodeSrc.m_eNType;
                m_strSrc  = nodeSrc.m_strSrc;
                m_strVal  = nodeSrc.m_strVal;
            }
            return *this;
        }


        // -------------------------------------------------------------------
        //  Public methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAsBool() const;

        tCIDLib::TCard4 c4AsCard() const;

        tCIDLib::TCard8 c8AsTime() const;

        tCIDLib::TFloat8 f8AsFloat() const;

        tCIDLib::TInt4 i4AsInt() const;

        EExprRes eCompareVals
        (
            const   TTNode&                 nodeComp
            , const ENTypes                 eParType
        )   const;


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_eDType
        //      The data type that this node was evaluated to. Used by the parent node
        //      to in turn evaluate its type and so forth. For immediate values we just
        //      try to convert them to the other side's value. We use the basic field
        //      type for this since fields and variables use these same types.
        //
        //  m_eNType
        //      The type of the node, which indicate how the other bits are interpreted.
        //
        //  m_strSrc
        //      For immediates, it's the value. For fields it's the field name. For
        //      variables it's the variable name. For expressions, not used.
        //
        //  m_strVal
        //
        //      The value that we evaluated to. For immediates, it's just a copy of
        //      m_strSrc. For fields and variables, it's the value we got from them.
        //      For expression nodes, it's the result of the two child nodes.
        //
        //      The parent nodes use this to do their left/right comparisons.
        // -------------------------------------------------------------------
        tCQCKit::EFldTypes  m_eDType;
        ENTypes             m_eNType;
        TString             m_strSrc;
        TString             m_strVal;
};

typedef TBasicTreeCol<TTNode>   TNodeTree;


tCIDLib::TBoolean TTNode::bAsBool() const
{
    if (m_strVal.bCompareI(kCQCKit::pszFld_False))
        return kCIDLib::False;

    if (m_strVal.bCompareI(kCQCKit::pszFld_True))
        return kCIDLib::True;

    // Try to convert it to a number
    tCIDLib::TInt4 i4Val;
    if (m_strVal.bToInt4(i4Val, tCIDLib::ERadices::Auto))
        return (i4Val != 0);

    facCQCAct().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kActErrs::errcExpr_CannotConvert
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Format
        , m_strVal
        , tCQCKit::strXlatEFldTypes(tCQCKit::EFldTypes::Boolean)
    );

    // Make the compiler happy
    return kCIDLib::False;
}

tCIDLib::TCard4 TTNode::c4AsCard() const
{
    tCIDLib::TCard4 c4Val;
    if (!m_strVal.bToCard4(c4Val, tCIDLib::ERadices::Auto))
    {
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcExpr_CannotConvert
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strVal
            , tCQCKit::strXlatEFldTypes(tCQCKit::EFldTypes::Card)
        );
    }
    return c4Val;
}

tCIDLib::TCard8 TTNode::c8AsTime() const
{
    tCIDLib::TCard8 c8Val;
    if (!m_strVal.bToCard8(c8Val, tCIDLib::ERadices::Auto))
    {
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcExpr_CannotConvert
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strVal
            , tCQCKit::strXlatEFldTypes(tCQCKit::EFldTypes::Time)
        );
    }
    return c8Val;
}

tCIDLib::TFloat8 TTNode::f8AsFloat() const
{
    tCIDLib::TFloat8 f8Val;
    if (!m_strVal.bToFloat8(f8Val))
    {
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcExpr_CannotConvert
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strVal
            , tCQCKit::strXlatEFldTypes(tCQCKit::EFldTypes::Float)
        );
    }
    return f8Val;
}

tCIDLib::TInt4 TTNode::i4AsInt() const
{
    tCIDLib::TInt4 i4Val;
    if (!m_strVal.bToInt4(i4Val, tCIDLib::ERadices::Auto))
    {
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcExpr_CannotConvert
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strVal
            , tCQCKit::strXlatEFldTypes(tCQCKit::EFldTypes::Int)
        );
    }
    return i4Val;
}


//
//  Both sides are typed. So we need to figure out if its us or the other one that
//  has the more expansive type and covert the other one to that form and compare.
//
//  The comparison is based on the parent node's node type,w hich we are given.
//
EExprRes
TTNode::eCompareVals(const  TTNode&     nodeComp
                    , const ENTypes     eParType) const
{
    // Take the more expansive data type. We'll convert both to that format.
    tCQCKit::EFldTypes eTarType = eBestType(m_eDType, nodeComp.m_eDType);

    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (eTarType == tCQCKit::EFldTypes::Boolean)
    {
        const tCIDLib::TBoolean bUs = bAsBool();
        const tCIDLib::TBoolean bHim = nodeComp.bAsBool();
        switch(eParType)
        {
            case ENTypes::Equal :
                bRet = (bUs == bHim);
                break;

            case ENTypes::GtThan :
                bRet = bUs && !bHim;
                break;

            case ENTypes::GtThanEq :
                bRet = (bUs == bHim) || (bUs && !bHim);
                break;

            case ENTypes::LsThan :
                bRet = !bUs && bHim;
                break;

            case ENTypes::LsThanEq :
                bRet = (bUs == bHim) || (!bUs && bHim);
                break;

            case ENTypes::NotEqual :
                bRet = (bUs != bHim);
                break;

            default :
                CIDAssert2(L"Uknown comparison operator")
                break;
        };
    }
     else if (eTarType == tCQCKit::EFldTypes::Card)
    {
        const tCIDLib::TCard4 c4Us = c4AsCard();
        const tCIDLib::TCard4 c4Him = nodeComp.c4AsCard();

        switch(eParType)
        {
            case ENTypes::Equal :
                bRet = (c4Us == c4Him);
                break;

            case ENTypes::GtThan :
                bRet = (c4Us > c4Him);
                break;

            case ENTypes::GtThanEq :
                bRet = (c4Us >= c4Him);
                break;

            case ENTypes::LsThan :
                bRet = (c4Us < c4Him);
                break;

            case ENTypes::LsThanEq :
                bRet = (c4Us <= c4Him);
                break;

            case ENTypes::NotEqual :
                bRet = (c4Us != c4Him);
                break;

            default :
                CIDAssert2(L"Uknown comparison operator")
                break;
        };
    }
     else if (eTarType == tCQCKit::EFldTypes::Float)
    {
        const tCIDLib::TFloat8 f8Us = f8AsFloat();
        const tCIDLib::TFloat8 f8Him = nodeComp.f8AsFloat();

        switch(eParType)
        {
            case ENTypes::Equal :
                bRet = (f8Us == f8Him);
                break;

            case ENTypes::GtThan :
                bRet = (f8Us > f8Him);
                break;

            case ENTypes::GtThanEq :
                bRet = (f8Us >= f8Him);
                break;

            case ENTypes::LsThan :
                bRet = (f8Us < f8Him);
                break;

            case ENTypes::LsThanEq :
                bRet = (f8Us <= f8Him);
                break;

            case ENTypes::NotEqual :
                bRet = (f8Us != f8Him);
                break;

            default :
                CIDAssert2(L"Uknown comparison operator")
                break;
        };
    }
     else if (eTarType == tCQCKit::EFldTypes::Int)
    {
        const tCIDLib::TInt4 i4Us = i4AsInt();
        const tCIDLib::TInt4 i4Him = nodeComp.i4AsInt();

        switch(eParType)
        {
            case ENTypes::Equal :
                bRet = (i4Us == i4Him);
                break;

            case ENTypes::GtThan :
                bRet = (i4Us > i4Him);
                break;

            case ENTypes::GtThanEq :
                bRet = (i4Us >= i4Him);
                break;

            case ENTypes::LsThan :
                bRet = (i4Us < i4Him);
                break;

            case ENTypes::LsThanEq :
                bRet = (i4Us <= i4Him);
                break;

            case ENTypes::NotEqual :
                bRet = (i4Us != i4Him);
                break;

            default :
                CIDAssert2(L"Uknown comparison operator")
                break;
        };
    }
     else if (eTarType == tCQCKit::EFldTypes::String)
    {
        switch(eParType)
        {
            case ENTypes::Equal :
                bRet = m_strVal.bCompareI(nodeComp.m_strVal);
                break;

            case ENTypes::GtThan :
                bRet = (m_strVal > nodeComp.m_strVal);
                break;

            case ENTypes::GtThanEq :
                bRet = (m_strVal >= nodeComp.m_strVal);
                break;

            case ENTypes::LsThan :
                bRet = (m_strVal < nodeComp.m_strVal);
                break;

            case ENTypes::LsThanEq :
                bRet = (m_strVal <= nodeComp.m_strVal);
                break;

            case ENTypes::NotEqual :
                bRet = !m_strVal.bCompareI(nodeComp.m_strVal);
                break;

            default :
                CIDAssert2(L"Uknown comparison operator")
                break;
        };
    }
     else if (eTarType == tCQCKit::EFldTypes::Time)
    {
        const tCIDLib::TCard8 c8Us = c8AsTime();
        const tCIDLib::TCard8 c8Him = nodeComp.c8AsTime();

        switch(eParType)
        {
            case ENTypes::Equal :
                bRet = (c8Us == c8Him);
                break;

            case ENTypes::GtThan :
                bRet = (c8Us > c8Him);
                break;

            case ENTypes::GtThanEq :
                bRet = (c8Us >= c8Him);
                break;

            case ENTypes::LsThan :
                bRet = (c8Us < c8Him);
                break;

            case ENTypes::LsThanEq :
                bRet = (c8Us <= c8Him);
                break;

            case ENTypes::NotEqual :
                bRet = (c8Us != c8Him);
                break;

            default :
                CIDAssert2(L"Uknown comparison operator")
                break;
        };
    }
     else
    {
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcExpr_UnknownDataType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , tCQCKit::strXlatEFldTypes(eTarType)
        );
    }

    // Return the result
    return bRet ? EExprRes::True : EExprRes::False;
}



// Eats chars until a non-space
static tCIDLib::TVoid EatSpace(TTextInStream& strmSrc, tCIDLib::TCard4& c4Ofs)
{
    while (!strmSrc.bEndOfStream())
    {
        const tCIDLib::TCh chNext = strmSrc.chPeek();
        if (!TRawStr::bIsSpace(chNext))
            break;

        // It is space so eat it
        c4Ofs++;
        strmSrc.chRead();
    }
}


// Pulls the next token from the input, returning the type and the text of it
static ETokens eGetNextToken(TTextInStream&     strmSrc
                            , TString&          strToFill
                            , tCIDLib::TCard4&  c4Ofs)
{
    strToFill.Clear();

    // Eat any leading whitespace
    EatSpace(strmSrc, c4Ofs);

    // Remember the starting offset, for error message
    const tCIDLib::TCard4 c4OrgOfs = c4Ofs;

    // Check the special case of a quoted string
    if (strmSrc.chPeek() == kCIDLib::chQuotation)
    {
        //
        //  Just read until we get the closing quote, allowing for escapement via a
        //  slash character.
        //
        strmSrc.chRead();
        c4Ofs++;
        while (kCIDLib::True)
        {
            // Never valid inside a
            if (strmSrc.bEndOfStream())
            {
                facCQCAct().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kActErrs::errcExpr_EOSNotLegal
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(c4Ofs)
                );
            }

            const tCIDLib::TCh chCur = strmSrc.chRead();
            c4Ofs++;

            // If an unescaped closing quote, we are done
            if ((chCur == kCIDLib::chQuotation)
            &&  (strToFill.chLast() != kCIDLib::chBackSlash))
            {
                break;
            }

            // Just add it to the string
            strToFill.Append(chCur);
        }

        return ETokens::Literal;
    }

    //
    //  Not a string so handle the other options. Since we know we are not in a string,
    //  space is a separator for any of this below.
    //
    while (!strmSrc.bEndOfStream())
    {
        // Check the next character
        tCIDLib::TCh chCur = strmSrc.chPeek();

        //
        //  We stop here and return what we have. It has to be trailing space since
        //  we ate leading above.
        //
        if (TRawStr::bIsSpace(chCur))
        {
            c4Ofs++;
            strmSrc.chRead();
            break;
        }

        // If it's a special character, deal with that.
        if (chCur == kCIDLib::chOpenParen)
        {
            //
            //  We can't have any token content yet. An open paren can only come
            //  at the start of the left or right side expressions.
            //
            if (!strToFill.bIsEmpty())
            {
                facCQCAct().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kActErrs::errcExpr_NoOpenParenHere
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(c4OrgOfs)
                );
            }

            // We are done
            c4Ofs++;
            strToFill.Append(strmSrc.chRead());
            return ETokens::OpenParen;
        }

        if (chCur == kCIDLib::chCloseParen)
        {
            //
            //  If no current content, take it as is. Else, we treat the content we
            //  have as a literal, or ref and leave this character unprocessed.
            //
            if (strToFill.bIsEmpty())
            {
                c4Ofs++;
                strToFill.Append(strmSrc.chRead());
                return ETokens::CloseParen;
            }
            break;
        }

        // Nothing that will make us stop, so store it away
        c4Ofs++;
        strToFill.Append(strmSrc.chRead());

        //
        //  If it's a dollar or percent, see if it's followed by a paren. If so, we
        //  need to eat chars up to the closing parent, since it's a field or variable
        //  reference. If not, we continue forward as is.
        //
        if ((chCur == kCIDLib::chDollarSign)
        ||  (chCur == kCIDLib::chPercentSign))
        {
            const tCIDLib::TBoolean bFldRef(chCur == kCIDLib::chDollarSign);

            if (strmSrc.chPeek() == kCIDLib::chOpenParen)
            {
                // Eat the open paren, just keep the name part
                c4Ofs++;
                strmSrc.chRead();
                strToFill.Clear();

                while (kCIDLib::True)
                {
                    chCur = strmSrc.chRead();
                    c4Ofs++;

                    // Don't include the closing paren
                    if (chCur == kCIDLib::chCloseParen)
                        break;

                    strToFill.Append(chCur);
                }

                if (bFldRef)
                {
                    TString strMon, strFld;
                    if (!facCQCKit().bParseFldName(strToFill, strMon, strFld))
                    {
                        facCQCAct().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kActErrs::errcExpr_BadFldRef
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Format
                            , TCardinal(c4OrgOfs)
                        );
                    }
                    return ETokens::FieldRef;
                }
                 else
                {
                    tCIDLib::TBoolean bByValue;
                    TString strName;
                    TString strErr;
                    if (!TStdVarsTar::bValidVarName(strToFill, strName, strErr, bByValue))
                    {
                        facCQCAct().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kActErrs::errcExpr_BadVarRef
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Format
                            , TCardinal(c4OrgOfs)
                        );
                    }
                    return ETokens::VarRef;
                }

                // If not, fall through and keep accumulating
            }
        }
    }

    // If we have nothing, then has to be end of string
    if (strToFill.bIsEmpty())
    {
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcExpr_EOSNotLegal
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4Ofs)
        );
    }

    // Has to be some literal value
    return ETokens::Literal;
}


//
//  This method will attempt to parse an expression and build up a tree. It is recursive.
//  We get a parent node the type of which is not yet known.
//
//  And expression is (lhs op rhs), i.e left and right hand sides separated by an
//  operator. We recursively parse the left, parse the operator, then recursively parse
//  the right hand side.
//
//  Each expression is either two terminal nodes, with some sort of comparison operator
//  between them, or two non-terminal nodes with a boolean operator between them. We
//  consider anything else an error.
//
static tCIDLib::TVoid ParseExpr(        TTreeNodeNT<TTNode>&    nodePar
                                ,       TTextInStream&          strmSrc
                                ,       tCIDLib::TCard4&        c4Ofs
                                , const tCIDLib::TCard4         c4Level)
{
    ETokens eToken;

    // First has to be an open paren if not 0th level
    TString strToken;
    if (c4Level)
    {
        eToken = eGetNextToken(strmSrc, strToken, c4Ofs);
        if (eToken != ETokens::OpenParen)
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcExpr_ExpOpenParen
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4Ofs)
            );
        }
    }

    //
    //  Next we either get an open paren, which means a nested expression, or not, in
    //  which case it has to be a value. First eat white space, then peak the next
    //  character.
    //
    EatSpace(strmSrc, c4Ofs);

    tCIDLib::TBoolean bIsBoolExpr;
    tCIDLib::TCh chPeek = strmSrc.chPeek();
    if (chPeek == kCIDLib::chOpenParen)
    {
        // Create a new non-terminal and add it
        TTreeNodeNT<TTNode>* pnodeLHS = nodePar.pnodeAddNonTerminal
        (
            L"LHS", TString::strEmpty(), kCIDLib::False, kCIDLib::False
        );

        // Set the data object on it, which the recursive call will fill in
        pnodeLHS->objData(TTNode());
        ::ParseExpr(*pnodeLHS, strmSrc, c4Ofs, c4Level + 1);

        // This has to be a boolean comparison
        bIsBoolExpr = kCIDLib::True;
    }
     else if (chPeek == kCIDLib::chCloseParen)
    {
        // Not valid here
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcExpr_NoCloseParenHere
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4Ofs)
        );
    }
     else
    {
        // Parse the next token and add a terminal for it if valid
        eToken = eGetNextToken(strmSrc, strToken, c4Ofs);

        if ((eToken == ETokens::Literal)
        ||  (eToken == ETokens::FieldRef)
        ||  (eToken == ETokens::VarRef))
        {
            TTNode nodeVal;
            if (eToken == ETokens::Literal)
            {
                nodeVal.m_eNType = ENTypes::Literal;
                nodeVal.m_strVal = strToken;
            }
             else if (eToken == ETokens::FieldRef)
            {
                nodeVal.m_eNType = ENTypes::FieldRef;
                nodeVal.m_strSrc = strToken;
            }
             else
            {
                nodeVal.m_eNType = ENTypes::VarRef;
                nodeVal.m_strSrc = strToken;
            }

            nodePar.objAddTerminal
            (
                nodeVal, L"LHS", TString::strEmpty(), kCIDLib::False, kCIDLib::False
            );

            // It's a value comparison expression
            bIsBoolExpr = kCIDLib::False;
        }
         else
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcExpr_ExpValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4Ofs)
            );
        }
    }

    // Now we have to get an operator token, which sets the parent node's type
    eToken = eGetNextToken(strmSrc, strToken, c4Ofs);
    if (eToken != ETokens::Literal)
    {
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcExpr_ExpOperator
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4Ofs)
        );
    }

    // Based on the token, set the node type
    if (strToken.bCompareI(L"AND"))
        nodePar.objData().m_eNType = ENTypes::AND;
    else if (strToken.bCompareI(L"OR"))
        nodePar.objData().m_eNType = ENTypes::OR;
    else if (strToken.bCompareI(L"XOR"))
        nodePar.objData().m_eNType = ENTypes::XOR;
    else if (strToken == L"==")
        nodePar.objData().m_eNType = ENTypes::Equal;
    else if (strToken == L">")
        nodePar.objData().m_eNType = ENTypes::GtThan;
    else if (strToken == L">=")
        nodePar.objData().m_eNType = ENTypes::GtThanEq;
    else if (strToken == L"<")
        nodePar.objData().m_eNType = ENTypes::LsThan;
    else if (strToken == L"<=")
        nodePar.objData().m_eNType = ENTypes::LsThanEq;
    else if (strToken == L"!=")
        nodePar.objData().m_eNType = ENTypes::NotEqual;
    else
    {
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcExpr_ExpOperator
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4Ofs)
        );
    }

    // If this is a boolean type expression, make sure we got a boolean, and vice versa
    const tCIDLib::TBoolean bGotBoolOp
    (
        (nodePar.objData().m_eNType == ENTypes::AND)
        || (nodePar.objData().m_eNType == ENTypes::OR)
        || (nodePar.objData().m_eNType == ENTypes::XOR)
    );

    if (bIsBoolExpr && !bGotBoolOp)
    {
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcExpr_ExpBoolOp
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4Ofs)
        );
    }
     else if (!bIsBoolExpr && bGotBoolOp)
    {
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcExpr_ExpTestOp
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4Ofs)
        );
    }

    // Now let's do the right hand side
    if (chPeek == kCIDLib::chOpenParen)
    {
        // If the LHS is a value, then this has to be a value
        if (!bIsBoolExpr)
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcExpr_ExpValueExpr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4Ofs)
            );
        }

        // Create a new non-terminal and add it
        TTreeNodeNT<TTNode>* pnodeLHS = nodePar.pnodeAddNonTerminal
        (
            L"RHS", TString::strEmpty(), kCIDLib::False, kCIDLib::False
        );

        // Set the data object on it, which the recursive call will fill in
        pnodeLHS->objData(TTNode());
        ::ParseExpr(*pnodeLHS, strmSrc, c4Ofs, c4Level + 1);
    }
     else if (chPeek == kCIDLib::chCloseParen)
    {
        // Not valid here
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcExpr_NoCloseParenHere
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4Ofs)
        );
    }
     else
    {
        // If the LHS is a bool, then this has to be a value
        if (bIsBoolExpr)
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcExpr_ExpResultExpr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4Ofs)
            );
        }

        // Parse the next token and add a terminal for it if valid
        eToken = eGetNextToken(strmSrc, strToken, c4Ofs);

        if ((eToken == ETokens::Literal)
        ||  (eToken == ETokens::FieldRef)
        ||  (eToken == ETokens::VarRef))
        {
            TTNode nodeVal;
            if (eToken == ETokens::Literal)
            {
                nodeVal.m_eNType = ENTypes::Literal;
                nodeVal.m_strVal = strToken;
            }
             else if (eToken == ETokens::FieldRef)
            {
                nodeVal.m_eNType = ENTypes::FieldRef;
                nodeVal.m_strSrc = strToken;
            }
             else
            {
                nodeVal.m_eNType = ENTypes::VarRef;
                nodeVal.m_strSrc = strToken;
            }

            nodePar.objAddTerminal
            (
                nodeVal, L"RHS", TString::strEmpty(), kCIDLib::False, kCIDLib::False
            );
        }
         else
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcExpr_ExpValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4Ofs)
            );
        }
    }

    //
    //  And it has to end with a close paren if not level 0. If we are on level 0, then
    //  there cannot be any more content.
    //
    if (c4Level)
    {
        eToken = eGetNextToken(strmSrc, strToken, c4Ofs);
        if (eToken != ETokens::CloseParen)
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcExpr_ExpCloseParen
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4Ofs)
            );
        }
    }
     else
    {
        EatSpace(strmSrc, c4Ofs);

        if (!strmSrc.bEndOfStream())
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcExpr_TrailingChars
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4Ofs)
            );
        }
    }
}


//
//  A helper for the run method below. It's called for terminal nodes. If the node is
//  a field or variable reference, we get the value out and store it, along with the
//  data type. If it's a literal, we just leave it as is.
//
static tCIDLib::TVoid
GetNodeVal(TTNode& nodeVal, TStdVarsTar& ctarGVars, TStdVarsTar& ctarLVars)
{
    if (nodeVal.m_eNType == ENTypes::FieldRef)
    {
        //
        //  Parse the moniker and field out. We tested it during expr parsing so we
        //  know its good.
        //
        TString strMon, strFld;
        facCQCKit().bParseFldName(nodeVal.m_strSrc, strMon, strFld);

        // Get a proxy for the moniker
        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMon);

        // And read the field and it's type. A failure will throw
        tCIDLib::TCard4 c4SerNum = 0;
        orbcDrv->bReadFieldByName(c4SerNum, strMon, strFld, nodeVal.m_strVal, nodeVal.m_eDType);
    }
     else if (nodeVal.m_eNType == ENTypes::VarRef)
    {
        // If it isn't found, this will throw, as we want
        TCQCActVar& varSrc = TStdVarsTar::varFind
        (
            nodeVal.m_strSrc
            , ctarLVars
            , ctarGVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::False
        );

        // Store the type and the value as a string
        nodeVal.m_eDType = varSrc.eType();
        nodeVal.m_strVal = varSrc.strValue();
    }
}


//
//  This is called to actually run the tree. We do a depth first iteration of the
//  tree. We could use the tree cursor, but then we'd lose the recursive processing,
//  which makes it a lot easier for us to do our thing. So we just directly iterate
//  the nodes.
//
//  <TBD>   Only some basic short circuiting is done here. For the most part it's
//          probably not even worth the extra complexity since these expressions aren't
//          likely to be more a handful of comparisons at worst. It would prevent
//          unneeded field reads if the user doesn't bother to get the value into a
//          variable for multiple uses.
//
static EExprRes
eRunExpr(TTreeNodeNT<TTNode>&   nodePar
        , TStdVarsTar&          ctarGVars
        , TStdVarsTar&          ctarLVars)
{
    //
    //  Get our left hand node and recurse on it if a non-terminal. Else, process
    //  it to get its value, which handles field and variable references. If it's a
    //  literal it is just left alone since we already have the value.
    //
    EExprRes eLHSRes = EExprRes::Value;
    TBasicTreeNode<TTNode>* pnodeLHS = nodePar.pnodeFirstChild();
    if (pnodeLHS->eType() == tCIDLib::ETreeNodes::NonTerminal)
    {
        eLHSRes = eRunExpr
        (
            *static_cast<TTreeNodeNT<TTNode>*>(pnodeLHS), ctarGVars, ctarLVars
        );

        //
        //  If our parent is a boolean expression, let's do a little short circuiting
        //  if possible.
        //
        if ((nodePar.objData().m_eNType == ENTypes::AND)
        &&  (eLHSRes == EExprRes::False))
        {
            // If an AND, a failure of one child means a failure at our level
            return EExprRes::False;
        }
         else if ((nodePar.objData().m_eNType == ENTypes::OR)
              &&  (eLHSRes == EExprRes::True))
        {
            // If OR, then any child success is a success
            return EExprRes::True;
        }
    }
     else
    {
        GetNodeVal(pnodeLHS->objData(), ctarGVars, ctarLVars);
    }

    // Do the same for the right hand side
    EExprRes eRHSRes = EExprRes::Value;
    TBasicTreeNode<TTNode>* pnodeRHS = pnodeLHS->pnodeNext();
    if (pnodeRHS->eType() == tCIDLib::ETreeNodes::NonTerminal)
    {
        // If it fails, we propagate back the failure
        eRHSRes = eRunExpr
        (
            *static_cast<TTreeNodeNT<TTNode>*>(pnodeRHS), ctarGVars, ctarLVars
        );
    }
     else
    {
        GetNodeVal(pnodeRHS->objData(), ctarGVars, ctarLVars);
    }

    //
    //  OK, we have our values, so let's create our own result based on those and
    //  our node type. Both sides must either be a boolean result or a value
    //  comparison, which simplifies things.
    //
    EExprRes eRetRes;
    if (eLHSRes == EExprRes::Value)
    {
        // Both sides are values, so compare them (in the left/right order!)
        eRetRes = pnodeLHS->objData().eCompareVals
        (
            pnodeRHS->objData(), nodePar.objData().m_eNType
        );
    }
     else
    {
        //
        //  Both are non-terminals, so we just compare the true/false results to set
        //  our own result.
        //
        if (eLHSRes == eRHSRes)
            eRetRes = EExprRes::True;
        else
            eRetRes = EExprRes::False;
    }

    return eRetRes;
}


//
//  A recursive method to format the expression out. Since the expression is not expanded
//  in the normal way during command pre-processing (because we need access to all of
//  field and variable references), we want to be able to format out the final, expanded
//  version if we have a debug output object, and send it for display.
//
static tCIDLib::TVoid FormatExpr(       TTreeNodeNT<TTNode>&    nodePar
                                ,       TTextOutStream&         strmTar
                                , const tCIDLib::TCard4         c4Level)
{
    // IF not level 0, then put out an open paren
    if (c4Level)
        strmTar << kCIDLib::chOpenParen;

    // Get the left side and format it out or recurse
    TBasicTreeNode<TTNode>* pnodeLHS = nodePar.pnodeFirstChild();
    if (pnodeLHS->eType() == tCIDLib::ETreeNodes::NonTerminal)
    {
        // Recurse on the left side
        FormatExpr(*static_cast<TTreeNodeNT<TTNode>*>(pnodeLHS), strmTar, c4Level + 1);
    }
     else
    {
        // Just display the value
        strmTar << pnodeLHS->objData().m_strVal;
    }

    // Format out the parent's operator
    if (nodePar.objData().m_eNType == ENTypes::AND)
        strmTar << L" AND ";
    else if (nodePar.objData().m_eNType == ENTypes::OR)
        strmTar << L" OR ";
    else if (nodePar.objData().m_eNType == ENTypes::XOR)
        strmTar << L" XOR ";
    else if (nodePar.objData().m_eNType == ENTypes::Equal)
        strmTar << L" == ";
    else if (nodePar.objData().m_eNType == ENTypes::GtThan)
        strmTar << L" > ";
    else if (nodePar.objData().m_eNType == ENTypes::GtThanEq)
        strmTar << L" >= ";
    else if (nodePar.objData().m_eNType == ENTypes::LsThan)
        strmTar << L" < ";
    else if (nodePar.objData().m_eNType == ENTypes::LsThanEq)
        strmTar << L" <= ";
    else if (nodePar.objData().m_eNType == ENTypes::NotEqual)
        strmTar << L" != ";
    else
        strmTar << L" ??? ";

    // Get the right side and format it out or recurse
    TBasicTreeNode<TTNode>* pnodeRHS = pnodeLHS->pnodeNext();
    if (pnodeRHS->eType() == tCIDLib::ETreeNodes::NonTerminal)
    {
        // Recurse on the left side
        FormatExpr(*static_cast<TTreeNodeNT<TTNode>*>(pnodeRHS), strmTar, c4Level + 1);
    }
     else
    {
        // Just display the value
        strmTar << pnodeRHS->objData().m_strVal;
    }

    // If not level 0, then put out a close paren
    if (c4Level)
        strmTar << kCIDLib::chCloseParen;
}



// ---------------------------------------------------------------------------
//  CLASS: TStdSystemCmdTar
// PREFIX: ctar
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TStdSystemCmdTar: Constructors and Destructor
// ---------------------------------------------------------------------------
TStdSystemCmdTar::TStdSystemCmdTar() :

    MCQCCmdTarIntf
    (
        kCQCKit::strCTarId_System
        , facCQCAct().strMsg(kActMsgs::midCmdTar_System)
        , kCQCKit::strCTarId_System
        , L"/Reference/Actions/System"
    )
    , m_c4ReqNum(1)
    , m_enctNextLLCheck(0)
    , m_f8Lat(0)
    , m_f8Long(0)
    , m_pspchTTS(nullptr)
{
    // Set the 'special' flag on us
    bIsSpecialCmdTar(kCIDLib::True);

    // Initialize the timers
    InitTimers();
}

TStdSystemCmdTar::~TStdSystemCmdTar()
{
}


// ---------------------------------------------------------------------------
//  TStdSystemCmdTar: Public, inherited methods
// ---------------------------------------------------------------------------

// We override this to do some parameter checking
tCIDLib::TBoolean
TStdSystemCmdTar::bValidateParm(const   TCQCCmd&        cmdSrc
                                , const TCQCCmdCfg&     ccfgTar
                                , const tCIDLib::TCard4 c4Index
                                , const TString&        strValue
                                ,       TString&        strErrText)
{
    // Let the command mixin do the most basic stuff
    if (!MCQCCmdTarIntf::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText))
        return kCIDLib::False;

    // That's ok, so do our own stuff
    if (ccfgTar.strCmdId() == kCQCKit::strCmdId_CalcGeoDistance)
    {
        //
        //  Check the first two wcich must be lat.long values, or the special
        //  values that represent the CQC/Home location or the client's location.
        //
        if ((c4Index == 0) || (c4Index == 1))
        {
            if (ccfgTar.piAt(c4Index).m_strValue == L"[SERVER]")
            {
            }
             else if (ccfgTar.piAt(c4Index).m_strValue == L"[CLIENT]")
            {
            }
             else
            {
                tCIDLib::TFloat8 f8Val;
                TString strLat, strLong;

                strLat = strValue;
                if (!strLat.bSplit(strLong, kCIDLib::chSpace)
                ||  !strLat.bToFloat8(f8Val)
                ||  !strLong.bToFloat8(f8Val))
                {
                    strErrText.LoadFromMsg(kActErrs::errcCmd_LatLong, facCQCAct());
                    return kCIDLib::False;
                }
            }
        }
    }
     else if (ccfgTar.strCmdId() == kCQCKit::strCmdId_EvalExpr)
    {
        // Test parse the expression
        if (!c4Index)
        {
            // Create a temp try
            TNodeTree colTree;

            // Set a  data object on the root
            colTree.pnodeRoot()->objData(TTNode());

            // Create an input stream over the parameter value and try the parse
            TTextStringInStream strmSrc(&strValue);
            try
            {
                tCIDLib::TCard4 c4Ofs = 0;
                ::ParseExpr(*colTree.pnodeRoot(), strmSrc, c4Ofs, 0);
            }

            catch(TError& errToCatch)
            {
                strErrText = errToCatch.strErrText();
                return kCIDLib::False;
            }
        }
    }
     else if (((ccfgTar.strCmdId() == kCQCKit::strCmdId_GetTimerVal) && !c4Index)
          ||  ((ccfgTar.strCmdId() == kCQCKit::strCmdId_ResetTimer) && !c4Index)
          ||  ((ccfgTar.strCmdId() == kCQCKit::strCmdId_WaitTimer) && !c4Index))
    {
        // All of these have a timer index
        tCIDLib::TCard4 c4Index = 0;
        if (!strValue.bToCard4(c4Index) || (c4Index >= kCQCAct::c4MaxSysTarTimers))
        {
            strErrText.LoadFromMsg
            (
                kActErrs::errcCmd_BadSysTarTimer, facCQCAct(), TCardinal(c4Index)
            );
            return kCIDLib::False;
        }
    }
     else if ((ccfgTar.strCmdId() == kCQCKit::strCmdId_IsNightTime)&& (c4Index == 0))
    {
        // Make sure it's a signed value from -60 to +60
        tCIDLib::TInt4 i4Ofs = 0;
        if (!strValue.bToInt4(i4Ofs) || (i4Ofs < -60) || (i4Ofs > 60))
        {
            strErrText.LoadFromMsg(kActErrs::errcCmd_NightOfs, facCQCAct());
            return kCIDLib::False;
        }

    }
     else if ((ccfgTar.strCmdId() == kCQCKit::strCmdId_GetNthToken) && (c4Index == 2))
    {
        // Make sure the Nth value isn't zero
        tCIDLib::TCard4 c4Nth = 0;
        if (!strValue.bToCard4(c4Nth) || (c4Nth == 0))
        {
            strErrText.LoadFromMsg(kActErrs::errcCmd_NthToken, facCQCAct());
            return kCIDLib::False;
        }
    }
     else if ((ccfgTar.strCmdId() == kCQCKit::strCmdId_DoGlobalAct) && (c4Index == 1))
    {
        // Make sure we can parse this as a quoted comma list if not empty
        if (!strValue.bIsEmpty())
        {
            tCIDLib::TCard4  c4ErrInd;
            TVector<TString> colDummy;
            if (!TStringTokenizer::bParseQuotedCommaList(strValue, colDummy, c4ErrInd))
            {
                strErrText.LoadFromMsg
                (
                    kKitErrs::errcCmd_BadQuotedList, facCQCKit(), TCardinal(c4ErrInd)
                );
                return kCIDLib::False;
            }
        }
    }
     else if (ccfgTar.strCmdId() == kCQCKit::strCmdId_HTTPGet)
    {
        if (c4Index == 0)
        {
            // Make sure the URL is a valid, fully qualified URL
            try
            {
                TURL urlTest(strValue, tCIDSock::EQualified::Full);
            }

            catch(...)
            {
                strErrText.LoadFromMsg(kActErrs::errcCmd_BadURL, facCQCAct());
                return kCIDLib::False;
            }
        }
         else if (c4Index == 2)
        {
            // Has to be a valid form 4 to 30
            tCIDLib::TCard4 c4Secs = 0;
            if (!strValue.bToCard4(c4Secs) || (c4Secs < 4) || (c4Secs > 30))
            {
                strErrText.LoadFromMsg
                (
                    kKitErrs::errcCmd_ParmRangeErr
                    , facCQCKit()
                    , TCardinal(4)
                    , TCardinal(30)
                );
                return kCIDLib::False;
            }
        }
    }
    return kCIDLib::True;
}


//
//  Called when the action has ended. This is called in the context of the thread
//  that invoked the action.
//
tCIDLib::TVoid TStdSystemCmdTar::CmdTarCleanup()
{
    //
    //  Clean up any speech object we might have faulted in. We do this here
    //  instead of in the destructor because it is thread specific.
    //
    if (m_pspchTTS)
    {
        delete m_pspchTTS;
        m_pspchTTS = nullptr;
    }
}


// Called when we are about to be re-used for a new action
tCIDLib::TVoid TStdSystemCmdTar::CmdTarInitialize(const TCQCActEngine&)
{
    // Initialize our 'timer channels'
    InitTimers();
}


// We override this to process command requests
tCQCKit::ECmdRes
TStdSystemCmdTar::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
                        , const tCIDLib::TCard4
                        , const TString&
                        , const TString&
                        ,       TStdVarsTar&        ctarGlobalVars
                        ,       tCIDLib::TBoolean&  bResult
                        ,       TCQCActEngine&      acteTar)
{
    tCQCKit::ECmdRes eRes = tCQCKit::ECmdRes::Ok;

    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_BitIsOn)
    {
        const tCIDLib::TCard4 c4SrcVal = ccfgToDo.piAt(0).m_strValue.c4Val();
        const tCIDLib::TCard4 c4Bit = ccfgToDo.piAt(1).m_strValue.c4Val();
        bResult = ((c4SrcVal & (0x1UL << c4Bit)) != 0);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_CvtToElapsed)
    {
        // Get the incoming seconds value
        tCIDLib::TCard4 c4Secs = ccfgToDo.piAt(0).m_strValue.c4Val();

        // And break out the values
        tCIDLib::TCard4 c4Hours;
        tCIDLib::TCard4 c4Mins;

        c4Hours = c4Secs / 3600;
        if (c4Hours)
            c4Secs -= c4Hours * 3600;
        c4Mins = c4Secs / 60;
        if (c4Mins)
            c4Secs -= c4Mins * 60;

        // And now fill in the three variables
        TCQCActVar& varHrs = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varHrs.bSetValue(c4Hours) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varHrs.strName(), varHrs.strValue());

        TCQCActVar& varMins = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(2).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varMins.bSetValue(c4Mins) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varMins.strName(), varMins.strValue());

        TCQCActVar& varSecs = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(3).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varSecs.bSetValue(c4Secs) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varSecs.strName(), varSecs.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_CalcGeoDistance)
    {
        //
        //  Calc the distance between two lat/long positions, in miles or km.
        //  The values are either the special values that represent the CQC
        //  server or the IV/RIVA client, or literal values. If literal values
        //  we get them as lat.long, so we have to break them apart.
        //
        //  Note that, if running under the RIVA server, it will replace [CLIENT]
        //  with whatever location the RIVA client reports himself at. Under the
        //  IV or C++ RIVA, we'll see the special value and look it up locally.
        //
        tCIDLib::TFloat8 f8Lat1;
        tCIDLib::TFloat8 f8Long1;
        tCIDLib::TFloat8 f8Lat2;
        tCIDLib::TFloat8 f8Long2;
        TString strLat, strLong;

        if (ccfgToDo.piAt(0).m_strValue.bCompareI(L"[SERVER]")
        ||  ccfgToDo.piAt(0).m_strValue.bCompareI(L"[CLIENT]"))
        {
            // Query the server or client side lat/long info
            GetLatLongInfo
            (
                ccfgToDo.piAt(0).m_strValue.bCompareI(L"[SERVER]")
                , f8Lat1
                , f8Long1
                , acteTar.sectUser()
            );
        }
         else
        {
            strLat = ccfgToDo.piAt(0).m_strValue;
            strLat.bSplit(strLong, kCIDLib::chSpace);
            f8Lat1 = strLat.f8Val();
            f8Long1 = strLong.f8Val();
        }

        if (ccfgToDo.piAt(1).m_strValue.bCompareI(L"[SERVER]")
        ||  ccfgToDo.piAt(1).m_strValue.bCompareI(L"[CLIENT]"))
        {
            // Query the server or client side lat/long info
            GetLatLongInfo
            (
                ccfgToDo.piAt(1).m_strValue.bCompareI(L"[SERVER]")
                , f8Lat2
                , f8Long2
                , acteTar.sectUser()
            );
        }
         else
        {
            strLat = ccfgToDo.piAt(1).m_strValue;
            strLat.bSplit(strLong, kCIDLib::chSpace);
            f8Lat2 = strLat.f8Val();
            f8Long2 = strLong.f8Val();
        }

        const tCIDLib::TBoolean bMiles(ccfgToDo.piAt(2).m_strValue.bCompareI(L"Miles"));
        const tCIDLib::TCard4 c4Dist = TMathLib::c4GeoDistance
        (
            f8Lat1, f8Long1, f8Lat2, f8Long2, bMiles
        );

        // Put the result back into the target variable
        TCQCActVar& varDist = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(3).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varDist.bSetValue(c4Dist) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varDist.strName(), varDist.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_DoGlobalAct)
    {
        // The first parm is the path to the action, so try to download it
        TCQCStdCmdSrc csrcRun;
        {
            tCIDLib::TCard4 c4SerialNum = 0;
            TDataSrvClient dsclRead;
            tCIDLib::TEncodedTime enctLastChange;
            tCIDLib::TKVPFList colMeta;
            dsclRead.bReadGlobalAction
            (
                ccfgToDo.piAt(0).m_strValue
                , c4SerialNum
                , enctLastChange
                , csrcRun
                , colMeta
                , acteTar.cuctxToUse().sectUser()
            );
        }

        //
        //  We now have to cheat a bit. We have no way to queue this up on
        //  the calling engine, so we create a nested one. Set any incoming
        //  parameters on it. We have to pass along the user credentials
        //  from the incoming action engine.
        //
        //  Note that any action parms of the calling engine are not passed
        //  in because we are setting new ones that are for this action. If
        //  the user wants to pass them in, he has to do so himself.
        //
        TCQCStdActEngine acteGlobal(acteTar.cuctxToUse());
        eRes = acteGlobal.eInvokeOps
        (
            csrcRun
            , kCQCKit::strEvId_OnTrigger
            , ctarGlobalVars
            , acteTar.colExtra()
            , acteTar.pcmdtDebug()
            , ccfgToDo.piAt(1).m_strValue
        );
    }
     else if ((ccfgToDo.strCmdId() == kCQCKit::strCmdId_EMail)
          ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_HTMLEMail))
    {
        // Call a helper for this, which involves a bit of work
        facCQCKit().SendEMail
        (
            ccfgToDo.piAt(0).m_strValue
            , ccfgToDo.piAt(1).m_strValue
            , ccfgToDo.piAt(2).m_strValue
            , ccfgToDo.piAt(3).m_strValue
            , (ccfgToDo.strCmdId() == kCQCKit::strCmdId_HTMLEMail)
            , acteTar.sectUser()
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_EMailURLImg)
    {
        bResult = kCIDLib::False;
        try
        {
            // Get the image from the indicated URL
            TURL urlImg(ccfgToDo.piAt(3).m_strValue, tCIDSock::EQualified::Full);

            //
            //  Provide a small buffer with a large max, and let the HTTP
            //  client resize it up as required.
            //
            TString strContType;
            THeapBuf mbufImg(512, 0xFF0000, 32 * 1024);
            const tCIDLib::TCard4 c4Bytes = c4GetImage(urlImg, mbufImg, strContType);

            // If no image bytes, do nothing
            if (!c4Bytes)
            {
                bResult = kCIDLib::False;
                return tCQCKit::ECmdRes::Ok;
            }

            // Probe it to see if we can provide a raesonable file name
            TPathStr pathFile = L"urlimage";
            const tCIDImage::EImgTypes eType
            (
                facCIDImgFact().eProbeImg(mbufImg, c4Bytes)
            );
            pathFile.AppendExt(facCIDImgFact().strImgTypeExt(eType));

            // And now do an e-mail send
            facCQCKit().SendEMail
            (
                ccfgToDo.piAt(0).m_strValue
                , ccfgToDo.piAt(1).m_strValue
                , ccfgToDo.piAt(2).m_strValue
                , tCIDLib::ForceMove(mbufImg)
                , c4Bytes
                , strContType
                , pathFile
                , acteTar.sectUser()
            );

            // It worked
            bResult = kCIDLib::True;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            throw;
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_EvalExpr)
    {
        // Create a tree and set a data object on the root, to start the process
        TNodeTree colTree;
        colTree.pnodeRoot()->objData(TTNode());

        // Create an input stream over the parameter value and try the parse
        TTextStringInStream strmSrc(&ccfgToDo.piAt(0).m_strValue);

        tCIDLib::TCard4 c4Ofs = 0;
        ::ParseExpr(*colTree.pnodeRoot(), strmSrc, c4Ofs, 0);

        // Run it and get the result
        EExprRes eRes = ::eRunExpr(*colTree.pnodeRoot(), ctarGlobalVars, acteTar.ctarLocals());

        // If we have a debug interface, format out the expanded expression and show it
        if (acteTar.pcmdtDebug())
        {
            TTextStringOutStream strmTar(1024UL);
            strmTar << L"                      -> ";
            ::FormatExpr(*colTree.pnodeRoot(), strmTar, 0);

            strmTar.Flush();
            acteTar.pcmdtDebug()->ActDebug(strmTar.strData());
        }

        //
        //  Set the command result based on that. The only things we can get here are
        //  true and false result, so we can just compare it to true.
        //
        bResult = (eRes == EExprRes::True);

        // If they asked, flip the result
        if (facCQCKit().bCheckBoolVal(ccfgToDo.piAt(1).m_strValue))
            bResult = !bResult;
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ExecApp)
    {
        // Translate the show option enum value
        const TString& strShow = ccfgToDo.piAt(3).m_strValue;
        tCIDLib::EExtProcShows eShow = tCIDLib::EExtProcShows::Normal;
        if (strShow == L"Hidden")
            eShow = tCIDLib::EExtProcShows::Hidden;
        else if (strShow == L"Show No Activate")
            eShow = tCIDLib::EExtProcShows::NormNoActivate;
        else if (strShow == L"Maximized")
            eShow = tCIDLib::EExtProcShows::Maximized;
        else if (strShow == L"Minimized")
            eShow = tCIDLib::EExtProcShows::Minimized;
        else if (strShow == L"Min No Activate")
            eShow = tCIDLib::EExtProcShows::MinNoActivate;

        // Make sure the path has the right slash types
        TString strPath(ccfgToDo.piAt(0).m_strValue);
        strPath.bReplaceChar(L'/', L'\\');

        facCQCKit().ExecApp
        (
            ccfgToDo.piAt(0).m_strValue
            , ccfgToDo.piAt(2).m_strValue
            , ccfgToDo.piAt(1).m_strValue
            , eShow
            , acteTar.strUserName()
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Equals)
    {
        //
        //  It has two parms, which we compare literally as strings. We can
        //  do it case sensitive or insensitive.
        //
        const TString& strL = ccfgToDo.piAt(0).m_strValue;
        const TString& strR = ccfgToDo.piAt(1).m_strValue;

        if (ccfgToDo.piAt(2).m_strValue.bCompareI(L"Case"))
            bResult = strL.bCompare(strR);
        else
            bResult = strL.bCompareI(strR);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_FmtNumAsSpoken)
    {
        tCIDLib::TCard4 c4Val = ccfgToDo.piAt(0).m_strValue.c4Val();
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::False
        );

        // Call a helper to do this work
        FmtNumForSpeech(c4Val, m_strTmp);
        if (varTar.bSetValue(m_strTmp) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_FormatTimeVal)
    {
        const TString& strTarVar = ccfgToDo.piAt(1).m_strValue;
        const TString& strFmt = ccfgToDo.piAt(2).m_strValue;
        const TString& strPattern = ccfgToDo.piAt(3).m_strValue;

        //
        //  If the target value is [local], then we just get the local time. Else, it has
        //  to be a Card8 time stamp value, which will be formatted in hex radix.
        //
        tCIDLib::TEncodedTime enctTime;
        if (ccfgToDo.piAt(0).m_strValue == L"[local]")
            enctTime = TTime::enctNow();
         else if (!ccfgToDo.piAt(0).m_strValue.bToCard8(enctTime, tCIDLib::ERadices::Hex))
            ThrowBadParmVal(ccfgToDo, 0);

        if (strFmt == L"Seconds")
        {
            enctTime /= kCIDLib::enctOneSecond;
            m_strTmp.SetFormatted(enctTime);
        }
         else if (strFmt == L"MilliSecs")
        {
            enctTime /= kCIDLib::enctOneMilliSec;
            m_strTmp.SetFormatted(enctTime);
        }
         else if (strFmt == L"Pattern")
        {
            TTime tmCur(enctTime);
            tmCur.FormatToStr(m_strTmp, strPattern);
        }
         else if (strFmt == L"Raw")
        {
            m_strTmp.SetFormatted(enctTime, tCIDLib::ERadices::Hex);
        }

        // Find the target variable and fill it in
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            strTarVar
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varTar.bSetValue(m_strTmp) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetCurTimeStamp)
    {
        const TString& strTarVar = ccfgToDo.piAt(0).m_strValue;

        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            strTarVar
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );


        if (varTar.bSetTime(TTime::enctNow()) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GenRandNum)
    {
        // And generate a number less than the max number
        const tCIDLib::TCard4 c4Max = ccfgToDo.piAt(0).m_strValue.c4Val(tCIDLib::ERadices::Auto);
        tCIDLib::TCard4 c4Num = facCIDCrypto().c4GetRandom();
        c4Num %= c4Max;

        // Create or find the variable and store the value
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );
        if (varTar.bSetValue(c4Num) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetDateParts)
    {
        //
        //  Get the stamp value out and put it into a time object, then
        //  pull out the date values. It will be in hex format, though it
        //  won't have the leading 0x, so we need to force the hex radix
        //
        //  We also will take [local] to mean local time, and provide it ourself.
        //
        TTime tmVal;
        if (ccfgToDo.piAt(0).m_strValue.bCompareI(L"[local]"))
            tmVal.SetToNow();
        else
            tmVal.enctTime(ccfgToDo.piAt(0).m_strValue.c8Val(tCIDLib::ERadices::Hex));

        tCIDLib::TCard4  c4Day;
        tCIDLib::TCard4  c4Year;
        tCIDLib::EMonths eMonth;
        tmVal.eAsDateInfo(c4Year, eMonth, c4Day);

        // Now write them out to their respective variable parms
        {
            TCQCActVar& varDay = TStdVarsTar::varFind
            (
                ccfgToDo.piAt(1).m_strValue
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );
            if (varDay.bSetValue(c4Day) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varDay.strName(), varDay.strValue());
        }

        {
            TCQCActVar& varMon = TStdVarsTar::varFind
            (
                ccfgToDo.piAt(2).m_strValue
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );
            if (varMon.bSetValue(tCIDLib::TInt4(eMonth) + 1) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varMon.strName(), varMon.strValue());
        }

        {
            TCQCActVar& varYear = TStdVarsTar::varFind
            (
                ccfgToDo.piAt(3).m_strValue
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );
            if (varYear.bSetValue(c4Year) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varYear.strName(), varYear.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetDayOfWeek)
    {
        // Get the format value and make sure it's good
        const TString& strFmt = ccfgToDo.piAt(0).m_strValue;
        tCIDLib::TBoolean bShort;
        if (strFmt.bCompareI(L"short"))
            bShort = kCIDLib::True;
        else if (strFmt.bCompareI(L"long"))
            bShort = kCIDLib::False;
        else
            ThrowBadParmVal(ccfgToDo, 0);

        // Get or create the two target vars
        TCQCActVar& varDay = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );

        TCQCActVar& varName = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(2).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );

        // Get the current day of week
        TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);
        const tCIDLib::EWeekDays eDay = tmNow.eDayOfWeek();

        // Set the numeric day variable from that
        if (varDay.bSetValue(tCIDLib::TInt4(eDay)) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varDay.strName(), varDay.strValue());

        // Get the appropriate day name and set the other var
        const TString& strDay
        (
            bShort ? TLocale::strDayAbbrev(eDay) : TLocale::strDay(eDay)
        );

        if (varName.bSetValue(strDay) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varName.strName(), varName.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetJulianDate)
    {
        // Get the target variable
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(0).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );

        TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);
        if (varTar.bSetValue(tmNow.c4ToJulian()) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetNthToken)
    {
        // Get the target variable
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(3).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );

        // Pull out the requested token and put it into the variable
        TStringTokenizer stokTmp
        (
            &ccfgToDo.piAt(0).m_strValue, ccfgToDo.piAt(1).m_strValue
        );
        const tCIDLib::TCard4 c4Nth = ccfgToDo.piAt(2).m_strValue.c4Val(tCIDLib::ERadices::Auto);

        tCIDLib::TBoolean bChanged = kCIDLib::False;
        tCIDLib::TCard4 c4Index = 1;
        if (c4Nth)
        {
            while (stokTmp.bGetNextToken(m_strTmp))
            {
                if (c4Index == c4Nth)
                {
                    bChanged = varTar.bSetValue(m_strTmp);
                    break;
                }
                c4Index++;
            }
        }

        // If we didn't get there, then set it to empty
        if (c4Index != c4Nth)
            bChanged = varTar.bSetValue(TString::strEmpty());

        if (bChanged && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetStrListCnt)
    {
        // Get the value and separator list and set up the tokenizer
        TStringTokenizer stokTmp
        (
            &ccfgToDo.piAt(0).m_strValue, ccfgToDo.piAt(1).m_strValue
        );

        tCIDLib::TCard4 c4Count = 0;
        while (stokTmp.bGetNextToken(m_strTmp))
            c4Count++;

        // Get the target variable and set it
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(2).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );

        if (varTar.bSetValue(c4Count) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetTimeParts)
    {
        //
        //  Get the stamp value out and put it into a time object, then
        //  pull out the date values. It will be in hex format, though it
        //  won't have the leading 0x, so we need to force the hex radix
        //
        //  We also will take [local] to mean local time, and provide it ourself.
        //
        TTime tmVal;
        if (ccfgToDo.piAt(0).m_strValue.bCompareI(L"[local]"))
            tmVal.SetToNow();
        else
            tmVal.enctTime(ccfgToDo.piAt(0).m_strValue.c8Val(tCIDLib::ERadices::Hex));

        tCIDLib::TCard4 c4Hour;
        tCIDLib::TCard4 c4Minute;
        tCIDLib::TCard4 c4Second;
        tmVal.c4AsTimeInfo(c4Hour, c4Minute, c4Second);

        // Now write them out to their respective variable parms
        {
            TCQCActVar& varHour = TStdVarsTar::varFind
            (
                ccfgToDo.piAt(1).m_strValue
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );
            if (varHour.bSetValue(c4Hour) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varHour.strName(), varHour.strValue());
        }

        {
            TCQCActVar& varMin = TStdVarsTar::varFind
            (
                ccfgToDo.piAt(2).m_strValue
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );
            if (varMin.bSetValue(c4Minute) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varMin.strName(), varMin.strValue());
        }

        {
            TCQCActVar& varSec = TStdVarsTar::varFind
            (
                ccfgToDo.piAt(3).m_strValue
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );
            if (varSec.bSetValue(c4Second) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varSec.strName(), varSec.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetTimerVal)
    {
        const tCIDLib::TCard4 c4Index = ccfgToDo.piAt(0).c4Val(tCIDLib::ERadices::Auto);
        if (c4Index >= kCQCAct::c4MaxSysTarTimers)
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcCmd_BadSysTarTimer
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Index
                , TCardinal(c4Index)
            );
        }

        // Get the target variable
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );

        const tCIDLib::TEncodedTime enctTm = m_aenctTimers[c4Index];
        const tCIDLib::TEncodedTime enctNow = TTime::enctNow();

        tCIDLib::TBoolean bChanged;
        if (enctTm >= enctNow)
        {
            bChanged = varTar.bSetValue(0UL);
        }
         else
        {
            bChanged = varTar.bSetValue
            (
                (enctNow - enctTm) / kCIDLib::enctOneMilliSec
            );
        }
        if (bChanged && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GreaterTh)
    {
        //
        //
        //  If either of them has a decimal, then conver them both to float
        //  format. Else assume int.
        //
        if (TRawStr::pszFindChar(ccfgToDo.piAt(0).m_strValue.pszBuffer(), L'.')
        ||  TRawStr::pszFindChar(ccfgToDo.piAt(1).m_strValue.pszBuffer(), L'.'))
        {
            tCIDLib::TFloat8 f8LHS = ccfgToDo.piAt(0).m_strValue.f8Val();
            tCIDLib::TFloat8 f8RHS = ccfgToDo.piAt(1).m_strValue.f8Val();
            bResult = (f8LHS > f8RHS);
        }
         else
        {
            tCIDLib::TInt8 i8LHS = ccfgToDo.piAt(0).m_strValue.i8Val();
            tCIDLib::TInt8 i8RHS = ccfgToDo.piAt(1).m_strValue.i8Val();
            bResult = (i8LHS > i8RHS);
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GreaterThEq)
    {
        if (TRawStr::pszFindChar(ccfgToDo.piAt(0).m_strValue.pszBuffer(), L'.')
        ||  TRawStr::pszFindChar(ccfgToDo.piAt(1).m_strValue.pszBuffer(), L'.'))
        {
            tCIDLib::TFloat8 f8LHS = ccfgToDo.piAt(0).m_strValue.f8Val();
            tCIDLib::TFloat8 f8RHS = ccfgToDo.piAt(1).m_strValue.f8Val();
            bResult = (f8LHS >= f8RHS);
        }
         else
        {
            tCIDLib::TInt8 i4LHS = ccfgToDo.piAt(0).m_strValue.i4Val();
            tCIDLib::TInt8 i4RHS = ccfgToDo.piAt(1).m_strValue.i4Val();
            bResult = (i4LHS >= i4RHS);
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_HTTPGet)
    {
        // Just do a non-persistent (1.0) type get on the provided URL
        THTTPClient httpcGet;

        // Try to parse the URL. Has to be fully qualified
        TURL urlToGet(ccfgToDo.piAt(0).m_strValue, tCIDSock::EQualified::Full);

        // The time has to be a number, which we clip back if unresaonable
        tCIDLib::TCard4 c4Secs = ccfgToDo.piAt(2).m_strValue.c4Val();
        if (c4Secs > 30)
            c4Secs = 30;
        else if (!c4Secs)
            c4Secs = 5;

        // Remember for below if we are giving back text
        const tCIDLib::TBoolean bRetText = !ccfgToDo.piAt(1).m_strValue.bIsEmpty();

        // An empty data source janitor means it will create a temporary one
        TCIDDataSrcJan janSrc;

        tCIDLib::TCard4     c4ContLen = 0;
        tCIDLib::TKVPList   colInHdrs;
        tCIDLib::TKVPList   colOutHdrs;
        tCIDNet::EHTTPCodes eCodeType;
        THeapBuf            mbufCont;
        TString             strRepLine;
        TString             strContType;
        TString             strEndURL;
        TString             strRetText;

        tCIDLib::TCard4 c4Res = 0;
        try
        {
            c4Res = httpcGet.c4SendGetRedir
            (
                janSrc
                , urlToGet
                , TTime::enctNowPlusSecs(c4Secs)
                , L"CQCActHTTPGet"
                , L"*/*"
                , eCodeType
                , strRepLine
                , colOutHdrs
                , strContType
                , mbufCont
                , c4ContLen
                , strEndURL
                , colInHdrs
            );

            // Set the result of the command
            bResult = (c4Res >= 200) && (c4Res < 300);

            // If we failed and are returning text, format out the status
            if (!bResult && bRetText)
            {
                strRetText = L"GET failed, code=";
                strRetText.AppendFormatted(c4Res);
            }
        }

        catch(TError& errToCatch)
        {
            if (facCQCAct().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            // If we are returning text, format out the error reason
            if (bRetText)
            {
                strRetText = L"GET failed, error=";
                strRetText.Append(errToCatch.strErrText());
            }
            bResult = kCIDLib::False;
        }

        //
        //  If successful, and they provided an output var and we got back a text
        //  format and the content length is non-zero, then transcode it in and
        //  put it into the variable.
        //
        if (bResult && bRetText && c4ContLen && strContType.bStartsWithI(L"text"))
        {
            // Figure out the encoding
            TString strEncoding;
            if (!TNetCoreParser::bFindTextEncoding(colOutHdrs, mbufCont, c4ContLen, strEncoding))
                strEncoding = kCIDNet::pszDefHTTPEncoding;

            //
            //  It is text, so see if we have a transcoder for this encoding in our
            //  CIDEncode facility. If so, transcode it.
            //
            if (facCIDEncode().bSupportsEncoding(strEncoding))
            {
                tCIDEncode::TTCvtPtr cptrBody
                (
                    facCIDEncode().ptcvtMake(strEncoding, tCIDLib::ETCvtActs::StopThenThrow)
                );
                cptrBody->c4ConvertFrom(mbufCont.pc1Data(), c4ContLen, strRetText);
            }
             else
            {
                // Fail it since they wanted output text and we can't provide it
                strRetText = L"Get Failed, error=Could not process body text";
                bResult = kCIDLib::False;
            }
        }

        // If returning text, get the output variable and store the text we have
        if (bRetText)
        {
            TCQCActVar& varText = TStdVarsTar::varFind
            (
                ccfgToDo.piAt(1).m_strValue
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , CID_FILE
                , CID_LINE
                , kCIDLib::True
            );

            if (varText.bSetValue(strRetText) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varText.strName(), varText.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsInList)
    {
        // There's a bit of code to this one, so we call a helper
        bResult = bIsInList(ccfgToDo);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsInNumericRange)
    {
        //
        //  We check to see if the value in P1 is within the range of the
        //  values indicated in P2(min) and P3(max).
        //
        if (TRawStr::pszFindChar(ccfgToDo.piAt(0).m_strValue.pszBuffer(), L'.')
        ||  TRawStr::pszFindChar(ccfgToDo.piAt(1).m_strValue.pszBuffer(), L'.')
        ||  TRawStr::pszFindChar(ccfgToDo.piAt(2).m_strValue.pszBuffer(), L'.'))
        {
            tCIDLib::TFloat8 f8Val = ccfgToDo.piAt(0).m_strValue.f8Val();
            tCIDLib::TFloat8 f8Min = ccfgToDo.piAt(1).m_strValue.f8Val();
            tCIDLib::TFloat8 f8Max = ccfgToDo.piAt(2).m_strValue.f8Val();
            bResult = ((f8Val >= f8Min) && (f8Val <= f8Max));
        }
         else
        {
            tCIDLib::TInt8 i8Val = ccfgToDo.piAt(0).m_strValue.i8Val();
            tCIDLib::TInt8 i8Min = ccfgToDo.piAt(1).m_strValue.i8Val();
            tCIDLib::TInt8 i8Max = ccfgToDo.piAt(2).m_strValue.i8Val();
            bResult = ((i8Val >= i8Min) && (i8Val <= i8Max));
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsInTimeRange)
    {
        // Get the two values out and convert to unsigned
        const tCIDLib::TCard4 c4Start = ccfgToDo.piAt(0).m_strValue.c4Val();
        const tCIDLib::TCard4 c4End = ccfgToDo.piAt(1).m_strValue.c4Val();

        TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);
        tCIDLib::TCard4 c4Hour;
        tCIDLib::TCard4 c4Minute;
        tCIDLib::TCard4 c4Second;
        tmNow.c4AsTimeInfo(c4Hour, c4Minute, c4Second);

        // We have the simple scenario and the wrap around scenario
        if (c4Start < c4End)
            bResult = ((c4Hour >= c4Start) && (c4Hour < c4End));
        else
            bResult = ((c4Hour >= c4Start) || (c4Hour < c4End));
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsNightTime)
    {
        // Get the server side lat/long info
        tCIDLib::TFloat8 f8Lat, f8Long;
        GetLatLongInfo(kCIDLib::True, f8Lat, f8Long, acteTar.sectUser());

        // Get the offset parameter as a signed value
        tCIDLib::TInt4 i4Offset = ccfgToDo.piAt(0).m_strValue.i4Val();

        // See if it's night time or not. This is a little trickey
        TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);
        TTime tmNoon(tCIDLib::ESpecialTimes::NoonToday);
        TTime tmTarget;

        //
        //  If we have an offset, then we add that many minutes to the target
        //  time. Negative offsets mean a more restrictive range, so add to
        //  the sunset time, and subtract from the sunrise time, else the
        //  other way around
        //
        if (tmNow < tmNoon)
        {
            //
            //  It is before noon today, so we are in 0 to 12 part of the
            //  day. So get the sunrise time. If the current time is before
            //  that time, then it's night.
            //
            tmTarget = tmNoon;
            tmTarget.SetToSunrise(f8Lat, f8Long);

            // Apply the offset if needed
            if (i4Offset > 0)
                tmTarget += kCIDLib::enctOneMinute * tCIDLib::TCard4(i4Offset);
            else if (i4Offset < 0)
                tmTarget -= kCIDLib::enctOneMinute * tCIDLib::TCard4(i4Offset * -1);
            bResult = (tmNow < tmTarget);
        }
         else
        {
            //
            //  It is after noon, so we are in the 12 to 23 part of the day.
            //  So get the sunset time. If the current time is after that
            //  time, then it's night.
            //
            tmTarget = tmNoon;
            tmTarget.SetToSunset(f8Lat, f8Long);

            // Apply the offset if needed
            if (i4Offset > 0)
                tmTarget -= kCIDLib::enctOneMinute * tCIDLib::TCard4(i4Offset);
            else if (i4Offset < 0)
                tmTarget += kCIDLib::enctOneMinute * tCIDLib::TCard4(i4Offset * -1);
            bResult = (tmNow > tmTarget);
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsWeekDay)
    {
        // Just set a time object to now and get the weekday from it
        TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);

        const tCIDLib::EWeekDays eDay = tmNow.eDayOfWeek();
        bResult =
        (
            (eDay != tCIDLib::EWeekDays::Saturday)
            && (eDay != tCIDLib::EWeekDays::Sunday)
        );

        // Give back the day number
        TCQCActVar& varDay = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(0).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );

        if (varDay.bSetValue(tCIDLib::TCard4(eDay)) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varDay.strName(), varDay.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_LessTh)
    {
        //
        //
        //  If either of them has a decimal, then conver them both to float
        //  format. Else assume int.
        //
        if (TRawStr::pszFindChar(ccfgToDo.piAt(0).m_strValue.pszBuffer(), L'.')
        ||  TRawStr::pszFindChar(ccfgToDo.piAt(1).m_strValue.pszBuffer(), L'.'))
        {
            tCIDLib::TFloat8 f8LHS = ccfgToDo.piAt(0).m_strValue.f8Val();
            tCIDLib::TFloat8 f8RHS = ccfgToDo.piAt(1).m_strValue.f8Val();
            bResult = (f8LHS < f8RHS);
        }
         else
        {
            tCIDLib::TInt8 i8LHS = ccfgToDo.piAt(0).m_strValue.i8Val();
            tCIDLib::TInt8 i8RHS = ccfgToDo.piAt(1).m_strValue.i8Val();
            bResult = (i8LHS < i8RHS);
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_LessThEq)
    {
        //
        //
        //  If either of them has a decimal, then conver them both to float
        //  format. Else assume int.
        //
        if (TRawStr::pszFindChar(ccfgToDo.piAt(0).m_strValue.pszBuffer(), L'.')
        ||  TRawStr::pszFindChar(ccfgToDo.piAt(1).m_strValue.pszBuffer(), L'.'))
        {
            tCIDLib::TFloat8 f8LHS = ccfgToDo.piAt(0).m_strValue.f8Val();
            tCIDLib::TFloat8 f8RHS = ccfgToDo.piAt(1).m_strValue.f8Val();
            bResult = (f8LHS <= f8RHS);
        }
         else
        {
            tCIDLib::TInt8 i8LHS = ccfgToDo.piAt(0).m_strValue.i8Val();
            tCIDLib::TInt8 i8RHS = ccfgToDo.piAt(1).m_strValue.i8Val();
            bResult = (i8LHS <= i8RHS);
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_LogMsg)
    {
        // Translate the severity
        tCIDLib::ESeverities eSev = tCIDLib::ESeverities::Status;
        const TString& strSev = ccfgToDo.piAt(3).m_strValue;
        if (strSev == L"Warning")
            eSev = tCIDLib::ESeverities::Warn;
        else if (strSev == L"Failure")
            eSev = tCIDLib::ESeverities::Failed;

        facCQCAct().LogMsg
        (
            ccfgToDo.piAt(0).m_strValue
            , ccfgToDo.piAt(1).m_strValue.c4Val()
            , ccfgToDo.piAt(2).m_strValue
            , eSev
            , tCIDLib::EErrClasses::AppStatus
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_MuteSystem)
    {
        TAudio::MuteSystemAudio(facCQCKit().bCheckBoolVal(ccfgToDo.piAt(0).m_strValue));
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Pause)
    {
        // Use the version of sleep that watches for shutdown requests
        const TString& strMS = ccfgToDo.piAt(0).m_strValue;
        TThread* pthrCaller = TThread::pthrCaller();

        // Just for safety, clip the value to less than 2 minutes
        tCIDLib::TCard4 c4MS = strMS.c4Val();
        if (c4MS > kCIDLib::c4OneMinute * 2)
            c4MS = kCIDLib::c4OneMinute * 2;
        pthrCaller->bSleep(c4MS);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_InvokeFile)
    {
        // Get the binding parm and try to get a proxy for that app ctrl server
        const TString& strBinding = ccfgToDo.piAt(0).m_strValue;
        TString strBind(TAppCtrlClientProxy::strImplBinding);
        strBind.eReplaceToken(strBinding, kCIDLib::chLatin_b);

        try
        {
            TAppCtrlClientProxy* porbcProxy
            (
                facCIDOrbUC().porbcMakeClient<TAppCtrlClientProxy>(strBind)
            );
            if (porbcProxy)
            {
                // That worked, so invoke it and store the result
                TJanitor<TAppCtrlClientProxy> janProxy(porbcProxy);
                bResult = porbcProxy->bInvokeFile
                (
                    ccfgToDo.piAt(1).m_strValue
                    , ccfgToDo.piAt(2).m_strValue
                );
            }
             else
            {
                bResult = kCIDLib::False;
            }
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            bResult = kCIDLib::False;
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_PlayWAV)
    {
        // Be tolerant and check for the file being present first
        const TString& strWF = ccfgToDo.piAt(0).m_strValue;
        if (TFileSys::bExists(strWF, tCIDLib::EDirSearchFlags::NormalFiles))
        {
            const TString& strAsync = ccfgToDo.piAt(1).m_strValue;
            TAudio::PlayWAVFile
            (
                strWF
                , (strAsync == L"True") ? tCIDLib::EWaitModes::NoWait
                                        : tCIDLib::EWaitModes::Wait
            );
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ResetTimer)
    {
        const tCIDLib::TCard4 c4Index = ccfgToDo.piAt(0).c4Val(tCIDLib::ERadices::Auto);
        if (c4Index >= kCQCAct::c4MaxSysTarTimers)
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcCmd_BadSysTarTimer
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Index
                , TCardinal(c4Index)
            );
        }
        m_aenctTimers[c4Index] = TTime::enctNow();
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SaveURLImg)
    {
        bResult = kCIDLib::False;
        try
        {
            // Get the image from the indicated URL
            TURL urlImg(ccfgToDo.piAt(0).m_strValue, tCIDSock::EQualified::Full);

            // Provide a small buf with a large max, and let it reallocate up
            TString strContType;
            THeapBuf mbufImg(512, 0xFF0000, 32 * 1024);
            const tCIDLib::TCard4 c4Bytes = c4GetImage(urlImg, mbufImg, strContType);

            // If no bytes, then do nothing
            if (!c4Bytes)
            {
                bResult = kCIDLib::False;
                return tCQCKit::ECmdRes::Ok;
            }

            // Open the file to write to
            TBinaryFile bflTar(ccfgToDo.piAt(1).m_strValue);
            bflTar.Open
            (
                tCIDLib::EAccessModes::Excl_Write
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );

            // If we write it all out, then it worked
            bResult = bflTar.c4WriteBuffer(mbufImg, c4Bytes) == c4Bytes;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            throw;
        }
    }
     else if ((ccfgToDo.strCmdId() == kCQCKit::strCmdId_SayText)
          ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SayTextAndWait))
    {
        tCIDLib::ESpeechFlags eFlag;
        if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SayText)
            eFlag = tCIDLib::ESpeechFlags::Async;
        else
            eFlag = tCIDLib::ESpeechFlags::None;

        FaultInTTS();
        m_pspchTTS->Speak(ccfgToDo.piAt(0).m_strValue, eFlag);
    }
     else if ((ccfgToDo.strCmdId() == kCQCKit::strCmdId_PlayIVWAV)
          ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SayIVText)
          ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetIVBlanker)
          ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetIVTemplate))
    {
        //
        //  In this case, we work through some target interface viewer
        //  that's running. We need to look for an IV control interface
        //  at the binding indicated.
        //
        //  All these the commands are the same other than the actual call
        //  we make, so we process them together.
        //
        const TString& strBindName = ccfgToDo.piAt(0).m_strValue;
        TString strBindPath(TIntfCtrlClientProxy::strBinding);
        strBindPath.eReplaceToken(strBindName, kCIDLib::chLatin_b);

        TIntfCtrlClientProxy* porbcProxy
        (
            facCIDOrbUC().porbcMakeClient<TIntfCtrlClientProxy >(strBindPath)
        );

        bResult = kCIDLib::True;
        if (porbcProxy)
        {
            TJanitor<TIntfCtrlClientProxy> janProxy(porbcProxy);

            //
            //  Send the command along. In the case of TTS, It will return
            //  false if it got the request but the queue was full, and we
            //  give that back as the command status.
            //
            if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SayIVText)
                bResult = porbcProxy->bSpeakText(ccfgToDo.piAt(1).m_strValue);
            else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_PlayIVWAV)
                porbcProxy->PlayWAV(ccfgToDo.piAt(1).m_strValue);
            else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetIVTemplate)
                porbcProxy->LoadTemplate(ccfgToDo.piAt(1).m_strValue);
            else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetIVBlanker)
                porbcProxy->DoMiscOp(tCQCKit::eAltXlatEIVMiscOps(ccfgToDo.piAt(1).m_strValue));
        }
         else
        {
            bResult = kCIDLib::False;
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SendWOL)
    {
        //
        //  They provide us provide us a string formatted MAC address. We create
        //  a UDP datagram, and enable broadcasts on it.
        //
        //  They can also provide us an explicit target IP address. If so, we will
        //  try to bind to a local interface that is appropriate for that target
        //  address.
        //
        TClientDatagramSocket sockDGram(tCIDSock::ESockProtos::UDP, tCIDSock::EAddrTypes::IPV4);

        if (!ccfgToDo.piAt(1).m_strValue.bIsEmpty())
        {
            TIPEndPoint ipepTar(ccfgToDo.piAt(1).m_strValue, 9, tCIDSock::EAddrTypes::IPV4);
            sockDGram.BindForRemote(ipepTar, 0);
        }
        sockDGram.bAllowBroadcast(kCIDLib::True);

        //
        //  We send the command three times, with a small delay between them, to help
        //  insure it gets seen. An extra one won't hurt anything.
        //
        sockDGram.SendWOLBroadcast(ccfgToDo.piAt(0).m_strValue);
        TThread::Sleep(150);
        sockDGram.SendWOLBroadcast(ccfgToDo.piAt(0).m_strValue);
        TThread::Sleep(150);
        sockDGram.SendWOLBroadcast(ccfgToDo.piAt(0).m_strValue);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetSysVolume)
    {
        //
        //  The parm should be a percent. The default is to throw if this cannot
        //  be done, which is appropriate, since we want the action to fail if
        //  the volume cannot be set.
        //
        const tCIDLib::TCard4 c4Per = ccfgToDo.piAt(0).c4Val(tCIDLib::ERadices::Auto);
        TAudio::bSetSystemVolume(c4Per);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SpeakChars)
    {
        //
        //  Get the parameter and build a new string which is the input string
        //  with a space between each character.
        //
        const TString& strSrc = ccfgToDo.piAt(0).m_strValue;
        const tCIDLib::TCard4 c4Count = strSrc.c4Length();
        if (c4Count)
        {
            TString strToSay(c4Count * 2);
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                strToSay.Append(strSrc[c4Index]);
                strToSay.Append(kCIDLib::chSpace);
            }

            FaultInTTS();
            m_pspchTTS->Speak(strToSay, tCIDLib::ESpeechFlags::Async);
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_StartRemApp)
    {
        // Get the binding parm and try to get a proxy for that app ctrl server
        const TString& strBinding = ccfgToDo.piAt(0).m_strValue;
        TString strBind(TAppCtrlClientProxy::strImplBinding);
        strBind.eReplaceToken(strBinding, kCIDLib::chLatin_b);

        try
        {
            TJanitor<TAppCtrlClientProxy> janProxy
            (
                facCIDOrbUC().porbcMakeClient<TAppCtrlClientProxy>(strBind)
            );
            if (janProxy.pobjThis())
            {
                // That worked, so invoke it and store the result
                bResult = janProxy.pobjThis()->bStartApp
                (
                    ccfgToDo.piAt(1).m_strValue
                    , ccfgToDo.piAt(2).m_strValue
                    , ccfgToDo.piAt(3).m_strValue
                );
            }
             else
            {
                bResult = kCIDLib::False;
            }
        }

        catch(const TError& errToCatch)
        {
            if (facCQCAct().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);
            bResult = kCIDLib::False;
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_StartsWith)
    {
        // It has two parms, We see if the first starts with the second
        bResult = ccfgToDo.piAt(0).m_strValue.bStartsWith(ccfgToDo.piAt(1).m_strValue);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Stop)
    {
        // Ask the engine to stop this action
        eRes = tCQCKit::ECmdRes::Stop;
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_WaitTimer)
    {
        // The parms are the timer index and a target time in millis
        const tCIDLib::TCard4 c4Index = ccfgToDo.piAt(0).c4Val(tCIDLib::ERadices::Auto);
        if (c4Index >= kCQCAct::c4MaxSysTarTimers)
        {
            facCQCAct().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kActErrs::errcCmd_BadSysTarTimer
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Index
                , TCardinal(c4Index)
            );
        }
        const tCIDLib::TCard4 c4Millis = ccfgToDo.piAt(1).c4Val(tCIDLib::ERadices::Auto);

        //
        //  If the timer has counted up fewer millis than the parm, then we
        //  pause. So calculate the elapsed millis on the indicated timer and
        //  compare.
        //
        const tCIDLib::TCard4 c4TimerMS
        (
            tCIDLib::TCard4
            (
                (TTime::enctNow() - m_aenctTimers[c4Index]) / kCIDLib::enctOneMilliSec
            )
        );

        if (c4TimerMS < c4Millis)
        {
            //
            //  We use the interruptable sleep call, so we'll exit on a
            //  thread shutdown request.
            //
            TThread* pthrCaller = TThread::pthrCaller();
            pthrCaller->bSleep(c4Millis - c4TimerMS);
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_WaitTTSDone)
    {
        // We'll wait for some more than reasonable time, then give up
        FaultInTTS();
        bResult = m_pspchTTS->bWaitTillDone(30000);
    }
     else
    {
        ThrowUnknownCmd(ccfgToDo);
    }
    return eRes;
}


// Return our commands that don't conflict with the given context
tCIDLib::TVoid
TStdSystemCmdTar::QueryCmds(        TCollection<TCQCCmd>&   colToFill
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    // 'BitIsOn' check
    {
        TCQCCmd& cmdBitIsOn = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_BitIsOn
                , facCQCAct().strMsg(kActMsgs::midCmd_BitIsOn)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Source)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
            )
        );
        cmdBitIsOn.eType(tCQCKit::ECmdTypes::Conditional);
    }

    // Calc the distance between two lat/long pairs
    {
        TCQCCmd cmdCalcGeoDist
        (
            kCQCKit::strCmdId_CalcGeoDistance
            , facCQCAct().strMsg(kActMsgs::midCmd_CalcGeoDistance)
            , 4
        );

        // Lat/long 1
        cmdCalcGeoDist.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_LatLong)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        // Lat/long 2
        cmdCalcGeoDist.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_LatLong)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        // Units indicator
        cmdCalcGeoDist.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Units)
                , L"Miles, Kilometers"
            )
        );

        // Output variable
        cmdCalcGeoDist.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::VarName
            )
        );
        colToFill.objAdd(cmdCalcGeoDist);
    }

    // Convert a number of seconds to elapsed hrs, mins, secs
    {
        TCQCCmd cmdCvtToElapsed
        (
            kCQCKit::strCmdId_CvtToElapsed
            , facCQCKit().strMsg(kKitMsgs::midCmd_CvtToElapsed)
            , 4
        );

        // The value to convert
        cmdCvtToElapsed.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
                , tCQCKit::ECmdPTypes::Unsigned
            )
        );

        // Then we need three outgoing variables to fill in
        cmdCvtToElapsed.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Hours)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        cmdCvtToElapsed.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Minutes)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        cmdCvtToElapsed.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Seconds)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        colToFill.objAdd(cmdCvtToElapsed);
    }

    if (eContext != tCQCKit::EActCmdCtx::IVEvent)
    {
        TCQCCmd& cmdGlobAct = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_DoGlobalAct
                , facCQCAct().strMsg(kActMsgs::midCmd_DoGlobalAct)
                , tCQCKit::ECmdPTypes::GlobAction
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Path)
                , tCQCKit::ECmdPTypes::QuotedList
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Parms)
            )
        );

        // The params are optional
        cmdGlobAct.SetParmReqAt(1, kCIDLib::False);
    }

    // Sends an E-mail to a defined CQC e-mail account
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_EMail
                , facCQCAct().strMsg(kActMsgs::midCmd_EMail)
                , tCQCKit::ECmdPTypes::EMailAcct
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Account)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Address)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Subject)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Msg)
            )
        );
    }

    // Sends an email containing an image from a URL
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd& cmdEMURLImg = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_EMailURLImg
                , facCQCAct().strMsg(kActMsgs::midCmd_EMailURLImg)
                , tCQCKit::ECmdPTypes::EMailAcct
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Account)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Address)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Subject)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_URL)
            )
        );
        cmdEMURLImg.eType(tCQCKit::ECmdTypes::Both);
    }

    // Evaluate an expression
    if (eContext != tCQCKit::EActCmdCtx::Preload)
    {
        TCQCCmd& cmdEvalExpr = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_EvalExpr
                , facCQCAct().strMsg(kActMsgs::midCmd_EvalExpr)
                , tCQCKit::ECmdPTypes::Expression
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Expression)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Negate)
            )
        );
        cmdEvalExpr.eType(tCQCKit::ECmdTypes::Conditional);
    }

    //
    // Execute an application
    //
    // Should we suppress this in IV based events?
    if (eContext != tCQCKit::EActCmdCtx::Preload)
    {
        TCQCCmd cmdExApp
        (
            kCQCKit::strCmdId_ExecApp
            , facCQCAct().strMsg(kActMsgs::midCmd_ExecApp)
            , 4
        );

        // The app to run
        cmdExApp.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_ToRun)
                , tCQCKit::ECmdPTypes::Application
            )
        );

        // The parms
        cmdExApp.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Parms)
                , tCQCKit::ECmdPTypes::Text
                , kCIDLib::False
            )
        );

        // The start path
        cmdExApp.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_StartPath)
                , tCQCKit::ECmdPTypes::Path
                , kCIDLib::False
            )
        );

        // The show options
        cmdExApp.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_ShowOpts)
                , L"Show, Show No Activate, Maximized, Minimized, Min No Activate, Hidden"
            )
        );

        colToFill.objAdd(cmdExApp);
    }

    //
    //  'Equals' comparison. We allow the second parm to be empty, so that
    //  they can compare something to an empty string.
    //
    {
        TCQCCmd cmdEquals
        (
            kCQCKit::strCmdId_Equals
            , facCQCAct().strMsg(kActMsgs::midCmd_Equals)
            , 3
        );

        cmdEquals.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Value1)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        cmdEquals.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Value2)
                , tCQCKit::ECmdPTypes::Text
                , kCIDLib::False
            )
        );

        cmdEquals.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_CompType)
                , L"Case, No Case"
            )
        );

        cmdEquals.eType(tCQCKit::ECmdTypes::Conditional);
        colToFill.objAdd(cmdEquals);
    }

    // Format a number as it would be spoken, to the passed variable
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_FmtNumAsSpoken
                , facCQCAct().strMsg(kActMsgs::midCmd_FormatNumAsSpoken)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            )
        );
    }

    //
    //  Take a time stamp value (100-ns interval type value) from a source
    //  variable, and formats it in various ways into another variable.
    //
    {
        TCQCCmd cmdFormatTimeVal
        (
            kCQCKit::strCmdId_FormatTimeVal
            , facCQCAct().strMsg(kActMsgs::midCmd_FormatTimeVal)
            , 4
        );

        cmdFormatTimeVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
                , tCQCKit::ECmdPTypes::Text
                , kCIDLib::True
            )
        );

        cmdFormatTimeVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
                , tCQCKit::ECmdPTypes::VarName
                , kCIDLib::True
            )
        );

        cmdFormatTimeVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Operation)
                , L"Seconds, MilliSecs, Pattern, Raw"
                , kCIDLib::True
            )
        );

        cmdFormatTimeVal.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
                , tCQCKit::ECmdPTypes::Text
                , kCIDLib::False
            )
        );

        colToFill.objAdd(cmdFormatTimeVal);
    }

    // Generate a random number between a min and max value
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GenRandNum
            , facCQCAct().strMsg(kActMsgs::midCmd_GenRandNum)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_MaxVal)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );

    // Get the current time stamp
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetCurTimeStamp
            , facCQCAct().strMsg(kActMsgs::midCmd_GetCurTimeStamp)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ToSet)
        )
    );

    // Get the date related values out of a time stamp
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetDateParts
            , facCQCAct().strMsg(kActMsgs::midCmd_GetDateParts)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Stamp)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Day)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Month)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Year)
        )
    );

    // Get current day of the week info
    {
        TCQCCmd cmdGetDOW
        (
            kCQCKit::strCmdId_GetDayOfWeek
            , facCQCAct().strMsg(kActMsgs::midCmd_GetDayOfWeek)
            , 3
        );

        cmdGetDOW.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Format)
                , L"Short, Long"
                , kCIDLib::True
            )
        );

        cmdGetDOW.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Day)
                , tCQCKit::ECmdPTypes::VarName
                , kCIDLib::True
            )
        );

        cmdGetDOW.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Name)
                , tCQCKit::ECmdPTypes::VarName
                , kCIDLib::True
            )
        );

        colToFill.objAdd(cmdGetDOW);
    }

    // Get the julian date
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetJulianDate
            , facCQCAct().strMsg(kActMsgs::midCmd_GetJulianDate)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
        )
    );

    // Get the Nth token into a passed variable
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetNthToken
            , facCQCAct().strMsg(kActMsgs::midCmd_GetNthToken)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Source)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Whitespace)
            , tCQCKit::ECmdPTypes::Signed
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );

    // Get the count of tokens in a string, given the provided token separator
    {
        TCQCCmd& cmdGetStrListCnt = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_GetStrListCnt
                , facCQCAct().strMsg(kActMsgs::midCmd_GetStrListCnt)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Whitespace)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Count)
            )
        );
    }

    // Get the time related values out of a time stamp
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetTimeParts
            , facCQCAct().strMsg(kActMsgs::midCmd_GetTimeParts)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Stamp)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Hour)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Minute)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Second)
        )
    );

    // Get the value (expired in MS) of one of the timers into a variable
    if (eContext != tCQCKit::EActCmdCtx::Preload)
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_GetTimerVal
                , facCQCAct().strMsg(kActMsgs::midCmd_GetTimerVal)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            )
        );
    }

    // Greater than comparison
    TCQCCmd& cmdGrTh = colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GreaterTh
            , facCQCAct().strMsg(kActMsgs::midCmd_GreaterTh)
            , tCQCKit::ECmdPTypes::Signed
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value1)
            , tCQCKit::ECmdPTypes::Signed
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value2)
        )
    );
    cmdGrTh.eType(tCQCKit::ECmdTypes::Conditional);

    // Greater than or equal comparison
    TCQCCmd& cmdGrThEq = colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GreaterThEq
            , facCQCAct().strMsg(kActMsgs::midCmd_GreaterThEq)
            , tCQCKit::ECmdPTypes::Signed
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value1)
            , tCQCKit::ECmdPTypes::Signed
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value2)
        )
    );
    cmdGrThEq.eType(tCQCKit::ECmdTypes::Conditional);


    // Send an e-mail message that contain HTML content
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_HTMLEMail
                , facCQCAct().strMsg(kActMsgs::midCmd_HTMLEMail)
                , tCQCKit::ECmdPTypes::EMailAcct
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Account)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Address)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Subject)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Msg)
            )
        );
    }

    // Do an HTTP Get and return the status, and optionally body text
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdHTTPGet
        (
            kCQCKit::strCmdId_HTTPGet
            , facCQCAct().strMsg(kActMsgs::midCmd_HTTPGet)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_URL)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Output)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Seconds)
        );

        cmdHTTPGet.SetParmReqAt(1, kCIDLib::False);
        cmdHTTPGet.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdHTTPGet);
    }


    //
    //  See if the numeric value of P1 is within the numeric range indicated
    //  by P2/P3 (min/max values.)
    //
    {
        TCQCCmd& cmdIsInNumR = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_IsInNumericRange
                , facCQCAct().strMsg(kActMsgs::midCmd_IsInNumRange)
                , tCQCKit::ECmdPTypes::Signed
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
                , tCQCKit::ECmdPTypes::Signed
                , facCQCKit().strMsg(kKitMsgs::midCmdP_MinVal)
                , tCQCKit::ECmdPTypes::Signed
                , facCQCKit().strMsg(kKitMsgs::midCmdP_MaxVal)
            )
        );
        cmdIsInNumR.eType(tCQCKit::ECmdTypes::Conditional);
    }

    //
    //  Checks to see if a value is in a list of values. The list can be
    //  space separated, comma separated, or a quoted comma list. The
    //  value can be a straight comparison or a reg ex.
    //
    {
        TCQCCmd cmdInList
        (
            kCQCKit::strCmdId_IsInList
            , facCQCAct().strMsg(kActMsgs::midCmd_IsInList)
            , 4
        );

        cmdInList.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        cmdInList.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_List)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        cmdInList.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Format)
                , L"Space Sep, Comma Sep, Quoted Comma List"
            )
        );

        cmdInList.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_CompType)
                , L"Equals, Equals Case, Starts With, Starts With Case, " \
                  L"Contains, Contains Case, RegEx Full, RegEx Partial, " \
                  L"RegEx Full Case, RegEx Partial Case"
            )
        );

        cmdInList.eType(tCQCKit::ECmdTypes::Conditional);
        colToFill.objAdd(cmdInList);
    }

    // Sees if the current time is within the two hours (24 format)
    {
        TCQCCmd& cmdIsInTmR = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_IsInTimeRange
                , facCQCAct().strMsg(kActMsgs::midCmd_IsInTimeRange)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_First)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Last)
            )
        );
        cmdIsInTmR.eType(tCQCKit::ECmdTypes::Conditional);
    }

    // See if it's night time or not
    {
        TCQCCmd& cmdIsNT = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_IsNightTime
                , facCQCAct().strMsg(kActMsgs::midCmd_IsNight)
                , tCQCKit::ECmdPTypes::Signed
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Offset)
            )
        );
        cmdIsNT.eType(tCQCKit::ECmdTypes::Conditional);
    }

    //
    //  Sees if it is currently a weekday or not, and returns the day number
    //  (sunday = 0, monday = 1, etc...)
    {
        TCQCCmd& cmdIsWeekDay = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_IsWeekDay
                , facCQCAct().strMsg(kActMsgs::midCmd_IsWeekDay)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Day)
            )
        );
        cmdIsWeekDay.eType(tCQCKit::ECmdTypes::Conditional);
    }

    // Invoke the default configured system action for a remote file
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdInvFile
        (
            kCQCKit::strCmdId_InvokeFile
            , facCQCAct().strMsg(kActMsgs::midCmd_InvokeFile)
            , 3
        );

        cmdInvFile.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_TrayMonitor)
                , tCQCKit::ECmdPTypes::TrayMonBinding
            )
        );

        cmdInvFile.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Target)
                , tCQCKit::ECmdPTypes::File
            )
        );

        cmdInvFile.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_ShowOpts)
                , L"Max, Min, Normal"
            )
        );

        cmdInvFile.cmdpAt(1).strExtra(L"*.*");
        cmdInvFile.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdInvFile);
    }


    // Less than comparison
    {
        TCQCCmd& cmdLsTh = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_LessTh
                , facCQCAct().strMsg(kActMsgs::midCmd_LessTh)
                , tCQCKit::ECmdPTypes::Signed
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value1)
                , tCQCKit::ECmdPTypes::Signed
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value2)
            )
        );
        cmdLsTh.eType(tCQCKit::ECmdTypes::Conditional);
    }

    // Less than or equal comparison
    {
        TCQCCmd& cmdLsThEq = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_LessThEq
                , facCQCAct().strMsg(kActMsgs::midCmd_LessThEq)
                , tCQCKit::ECmdPTypes::Signed
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value1)
                , tCQCKit::ECmdPTypes::Signed
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value2)
            )
        );
        cmdLsThEq.eType(tCQCKit::ECmdTypes::Conditional);
    }

    // Log a message
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdLogMsg
        (
            kCQCKit::strCmdId_LogMsg
            , facCQCAct().strMsg(kActMsgs::midCmd_LogMsg)
            , 4
        );

        // The 'source file'
        cmdLogMsg.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Source)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        // The 'line number'
        cmdLogMsg.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Line)
                , tCQCKit::ECmdPTypes::Unsigned
            )
        );

        // The message to log
        cmdLogMsg.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Msg)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        // The severity
        cmdLogMsg.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Severity)
                , L"Status, Warning, Failure"
            )
        );
        colToFill.objAdd(cmdLogMsg);
    }

    // Mute the system volume
    if (eContext != tCQCKit::EActCmdCtx::Preload)
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_MuteSystem
                , facCQCAct().strMsg(kActMsgs::midCmd_MuteSystem)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCKit().strMsg(kKitMsgs::midCmdP_MuteState)
            )
        );
    }

    // Sleep for a given number of milliseconds
    if ((eContext != tCQCKit::EActCmdCtx::Preload)
    &&  (eContext != tCQCKit::EActCmdCtx::IVEvent))
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_Pause
                , facCQCAct().strMsg(kActMsgs::midCmd_Pause)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_MilliSecs)
            )
        );
    }

    // Play a WAV file
    if (eContext != tCQCKit::EActCmdCtx::Preload)
    {
        TCQCCmd cmdPopout
        (
            kCQCKit::strCmdId_PlayWAV
            , facCQCAct().strMsg(kActMsgs::midCmd_PlayWAV)
            , 2
        );

        cmdPopout.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_WAVFile)
                , tCQCKit::ECmdPTypes::File
            )
        );

        cmdPopout.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Async)
                , tCQCKit::ECmdPTypes::Boolean
            )
        );

        // For file parms, the extra string is set to a wildcard
        cmdPopout.cmdpAt(0).strExtra(L"*.WAV");
        colToFill.objAdd(cmdPopout);
    }

    // Play a WAV remotely on an IV
    if ((eContext != tCQCKit::EActCmdCtx::Preload)
    &&  (eContext != tCQCKit::EActCmdCtx::IVEvent))
    {
        TCQCCmd& cmdPlayIVWAV = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_PlayIVWAV
                , facCQCAct().strMsg(kActMsgs::midCmd_PlayIVWAV)
                , tCQCKit::ECmdPTypes::IV
                , facCQCKit().strMsg(kKitMsgs::midCmdP_IntfViewer)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Path)
            )
        );
        cmdPlayIVWAV.eType(tCQCKit::ECmdTypes::Both);
    }

    // Reset of one of the timers
    if (eContext != tCQCKit::EActCmdCtx::Preload)
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_ResetTimer
                , facCQCAct().strMsg(kActMsgs::midCmd_ResetTimer)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
            )
        );
    }

    // Send a wake on LAN event
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SendWOL
                , facCQCAct().strMsg(kActMsgs::midCmd_SendWOL)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_MACAddr)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetAddr)
            )
        );
    }

    // Save an image from a URL to a local file
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd& cmdSaveURLImg = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SaveURLImg
                , facCQCAct().strMsg(kActMsgs::midCmd_SaveURLImg)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_URL)
                , tCQCKit::ECmdPTypes::File
                , facCQCKit().strMsg(kKitMsgs::midCmdP_File)
            )
        );

        cmdSaveURLImg.cmdpAt(1).strExtra(L"*.*");
        cmdSaveURLImg.eType(tCQCKit::ECmdTypes::Both);
    }


    // Speak text with field replacement tokens
    if ((eContext != tCQCKit::EActCmdCtx::Preload)
    &&  (eContext != tCQCKit::EActCmdCtx::IVEvent))
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SayText
                , facCQCAct().strMsg(kActMsgs::midCmd_SayText)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
            )
        );

        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SayTextAndWait
                , facCQCAct().strMsg(kActMsgs::midCmd_SayTextAndWait)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
            )
        );

        TCQCCmd& cmdSayIVText = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SayIVText
                , facCQCAct().strMsg(kActMsgs::midCmd_SayIVText)
                , tCQCKit::ECmdPTypes::IV
                , facCQCKit().strMsg(kKitMsgs::midCmdP_IntfViewer)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
            )
        );
        cmdSayIVText.eType(tCQCKit::ECmdTypes::Both);
    }


    // Send a blanker state msg to an IV
    if ((eContext != tCQCKit::EActCmdCtx::Preload)
    &&  (eContext != tCQCKit::EActCmdCtx::IVEvent))
    {
        TCQCCmd cmdSetIVBl
        (
            kCQCKit::strCmdId_SetIVBlanker
            , facCQCAct().strMsg(kActMsgs::midCmd_SetIVBlanker)
            , 2
        );

        cmdSetIVBl.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_IntfViewer)
                , tCQCKit::ECmdPTypes::IV
            )
        );

        cmdSetIVBl.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_State)
                , L"Blanker, Blank Display, Blank and Blanker, Clock Blanker, Slideshow Blanker, Unblank"
            )
        );

        // For file parms, the extra string is set to a wildcard
        colToFill.objAdd(cmdSetIVBl);
    }


    // Tell an IV to load a template
    if ((eContext != tCQCKit::EActCmdCtx::Preload)
    &&  (eContext != tCQCKit::EActCmdCtx::IVEvent))
    {
        TCQCCmd& cmdSetIVTemplate = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetIVTemplate
                , facCQCAct().strMsg(kActMsgs::midCmd_SetIVTemplate)
                , tCQCKit::ECmdPTypes::IV
                , facCQCKit().strMsg(kKitMsgs::midCmdP_IntfViewer)
                , tCQCKit::ECmdPTypes::Template
                , facCQCKit().strMsg(kKitMsgs::midCmdP_TemplateName)
            )
        );
        cmdSetIVTemplate.eType(tCQCKit::ECmdTypes::Both);
    }

    // Set the system volume
    if (eContext != tCQCKit::EActCmdCtx::Preload)
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetSysVolume
                , facCQCAct().strMsg(kActMsgs::midCmd_SetSysVolume)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Volume)
            )
        );
    }

    //
    //  Speak each character separately in the passed text. Usually used
    //  to speak things like telephone numbers.
    //
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SpeakChars
                , facCQCAct().strMsg(kActMsgs::midCmd_SpeakChars)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
            )
        );
    }

    // Start an app remotely via the app control part of the tray monitor
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd& cmdStartApp = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_StartRemApp
                , facCQCAct().strMsg(kActMsgs::midCmd_StartRemApp)
                , tCQCKit::ECmdPTypes::TrayMonBinding
                , facCQCKit().strMsg(kKitMsgs::midCmdP_TrayMonitor)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_ToRun)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Parms)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_StartPath)
            )
        );

        // Parms and start path are optional
        cmdStartApp.SetParmReqAt(2, kCIDLib::False);
        cmdStartApp.SetParmReqAt(3, kCIDLib::False);
        cmdStartApp.eType(tCQCKit::ECmdTypes::Both);
    }

    // A StartsWith conditional expression
    {
        TCQCCmd& cmdStartsWith = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_StartsWith
                , facCQCAct().strMsg(kActMsgs::midCmd_StartsWith)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
            )
        );
        cmdStartsWith.eType(tCQCKit::ECmdTypes::Conditional);
    }

    // A 'Stop' command to exit an action
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_Stop
            , facCQCAct().strMsg(kActMsgs::midCmd_Stop)
            , 0
        )
    );

    // Wait on one of the timers
    if ((eContext != tCQCKit::EActCmdCtx::Preload)
    &&  (eContext != tCQCKit::EActCmdCtx::IVEvent))
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_WaitTimer
                , facCQCAct().strMsg(kActMsgs::midCmd_WaitTimer)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_MilliSecs)
            )
        );
    }

    // Wait for any TTS invoked locally to complete
    if ((eContext != tCQCKit::EActCmdCtx::Preload)
    &&  (eContext != tCQCKit::EActCmdCtx::IVEvent))
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_WaitTTSDone
                , facCQCAct().strMsg(kActMsgs::midCmd_WaitTTSDone)
                , 0
            )
        );
    }
}


// Provide default parameters for some of our commands
tCIDLib::TVoid TStdSystemCmdTar::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    if (ccfgToSet.strCmdId() == kCQCKit::strCmdId_GetNthToken)
    {
        ccfgToSet.SetParmAt(2, L"1");
    }
     else if (ccfgToSet.strCmdId() == kCQCKit::strCmdId_Pause)
    {
        ccfgToSet.SetParmAt(0, L"1000");
    }
     else if (ccfgToSet.strCmdId() == kCQCKit::strCmdId_HTTPGet)
    {
        ccfgToSet.SetParmAt(2, L"5");
    }
}


// ---------------------------------------------------------------------------
//  TStdSystemCmdTar: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This guy supports the IsInList command. We have to parse out the values in
//  a list, the compare them all against the pattern. We support a number of
//  types of comparisons.
//
tCIDLib::TBoolean TStdSystemCmdTar::bIsInList(const TCQCCmdCfg& ccfgSrc)
{
    //
    //  First let's try to parse the list vals, based on the format they
    //  tell us to expect.
    //
    tCIDLib::TBoolean bRes;
    tCIDLib::TCard4   c4Ind;
    tCIDLib::TStrList colListVals;
    const TString& strList = ccfgSrc.piAt(1).m_strValue;
    const TString& strListFmt = ccfgSrc.piAt(2).m_strValue;
    if (strListFmt.bCompareI(L"Space Sep"))
    {
        TStringTokenizer stokSpace(&strList, L" ");
        while (stokSpace.bGetNextToken(m_strTmp))
            colListVals.objAdd(m_strTmp);
    }
     else if (strListFmt.bCompareI(L"Comma Sep"))
    {
        TStringTokenizer stokSpace(&strList, L",");
        while (stokSpace.bGetNextToken(m_strTmp))
        {
            m_strTmp.StripWhitespace();
            colListVals.objAdd(m_strTmp);
        }
    }
     else if (strListFmt.bCompareI(L"Quoted Comma List"))
    {
        bRes = TStringTokenizer::bParseQuotedCommaList(strList, colListVals, c4Ind);
        if (!bRes)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_BadQuotedList
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4Ind)
            );
        }
    }
     else
    {
        // Not something we understand
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_BadCmdParm
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(3)
            , ccfgSrc.strName()
            , TString(L"Unknown list format")
        );
    }

    // If we happen to get no values, then return false now
    const tCIDLib::TCard4 c4Count = colListVals.c4ElemCount();
    if (!c4Count)
        return kCIDLib::False;

    //
    //  OK, we have our list of values to compare against. Now, based on the
    //  the comparison type we do a loop and do that type of compare.
    //
    const TString& strPat = ccfgSrc.piAt(0).m_strValue;
    const TString& strType = ccfgSrc.piAt(3).m_strValue;
    if (strType.bCompareI(L"Equals"))
    {
        // Just a simple comparison, case insensitive
        for (c4Ind = 0; c4Ind < c4Count; c4Ind++)
        {
            if (colListVals[c4Ind].bCompareI(strPat))
                return kCIDLib::True;
        }
    }
     else if (strType.bCompareI(L"Equals Case"))
    {
        // A case sensitive comparison
        for (c4Ind = 0; c4Ind < c4Count; c4Ind++)
        {
            if (colListVals[c4Ind].bCompare(strPat))
                return kCIDLib::True;
        }
    }
     else if (strType.bCompareI(L"Starts With"))
    {
        // A case insensitive starts with
        for (c4Ind = 0; c4Ind < c4Count; c4Ind++)
        {
            if (colListVals[c4Ind].bStartsWithI(strPat))
                return kCIDLib::True;
        }
    }
     else if (strType.bCompareI(L"Starts With Case"))
    {
        // A case sensitive starts with
        for (c4Ind = 0; c4Ind < c4Count; c4Ind++)
        {
            if (colListVals[c4Ind].bStartsWith(strPat))
                return kCIDLib::True;
        }
    }
     else if (strType.bCompareI(L"Contains")
          ||  strType.bCompareI(L"Contains Case"))
    {
        // These are the same other than the case sensitive flag
        const tCIDLib::TBoolean bCase
        (
            strType.eCompareI(L"Contains Case") == tCIDLib::ESortComps::Equal
        );

        for (c4Ind = 0; c4Ind < c4Count; c4Ind++)
        {
            tCIDLib::TCard4 c4Pos = 0;
            bRes = colListVals[c4Ind].bFirstOccurrence
            (
                strPat, c4Pos, kCIDLib::False, bCase
            );
            if (bRes)
                return kCIDLib::True;
        }
    }
     else if (strType.bCompareI(L"RegEx Full")
          ||  strType.bCompareI(L"RegEx Partial")
          ||  strType.bCompareI(L"RegEx Full Case")
          ||  strType.bCompareI(L"RegEx Partial Case"))

    {
        //
        //  Set up the regular expression using the pattern. Depending on
        //  the type we do a full or partial match and case sensitive or
        //  not.
        //
        const tCIDLib::TBoolean bFull
        (
            strType.bCompareI(L"RegEx Full") || strType.bCompareI(L"RegEx Full Case")
        );
        const tCIDLib::TBoolean bCase
        (
            strType.bCompareI(L"RegEx Full Case")
            || strType.bCompareI(L"RegEx Partial Case")
        );

        TRegEx regxComp(strPat);

        for (c4Ind = 0; c4Ind < c4Count; c4Ind++)
        {
            tCIDLib::TCard4 c4Ofs;
            tCIDLib::TCard4 c4Len;
            if (regxComp.bFindMatch(colListVals[c4Ind], c4Ofs, c4Len, bFull, bCase))
                return kCIDLib::True;
        }
    }
     else
    {
        // Not something we understand
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_BadCmdParm
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(3)
            , ccfgSrc.strName()
            , TString(L"Unknown comparison type")
        );
    }

    // If we get here, never found it
    return kCIDLib::False;
}


//
//  Get an image at the passed URL. We return the binary data we got, and
//  the content type returned in the query.
//
tCIDLib::TCard4
TStdSystemCmdTar::c4GetImage(const  TURL&       urlImg
                            ,       TMemBuf&    mbufImg
                            ,       TString&    strContType)
{
    tCIDLib::TCard4         c4ContLen = 0;
    tCIDLib::TKVPList       colInHdrs;
    tCIDLib::TKVPList       colOutHdrs;
    tCIDNet::EHTTPCodes     eCodeType;
    THTTPClient             httpcImg;
    TString                 strRepText;
    TString                 strUltimateURL;

    // Enable automatic authentication
    httpcImg.bAutoAuth(kCIDLib::True);

    // Let it create a temporary data source for us
    TCIDDataSrcJan janEmpty;
    tCIDLib::TCard4 c4Res = httpcImg.c4SendGetRedir
    (
        janEmpty
        , urlImg
        , TTime::enctNowPlusSecs(8)
        , L"CQCWebClient"
        , L"image/*"
        , eCodeType
        , strRepText
        , colOutHdrs
        , strContType
        , mbufImg
        , c4ContLen
        , strUltimateURL
        , colInHdrs
    );

    if (c4Res != kCIDNet::c4HTTPStatus_OK)
    {
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcCmd_GetURLImg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TCardinal(c4Res)
        );
    }

    // Make sure it's an image of some type
    if (!strContType.bStartsWithI(L"image/"))
    {
        facCQCAct().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kActErrs::errcCmd_NotAnImage
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , strContType
        );
    }

    // If there's anything besides jsut the type, toss it
    strContType.bCapAtChar(L';');
    strContType.StripWhitespace();

    return c4ContLen;
}


//
//  Not much to this, but it's done from anumber of places and we may need to do more
//  work at some point.
//
tCIDLib::TVoid TStdSystemCmdTar::FaultInTTS()
{
    if (!m_pspchTTS)
        m_pspchTTS = new TSpeech;
}


//
//  Takes a number and formats it out in a speakable form. Handles up to
//  999, English only.
//
tCIDLib::TVoid
TStdSystemCmdTar::FmtNumForSpeech(const tCIDLib::TCard4 c4Val
                                ,       TString&        strToFill)
{
    // Some lookup tables to use
    static const tCIDLib::TCh* apszOnes[10] =
    {
        L"zeroth", L"first", L"second", L"third", L"fourth", L"fifth"
        , L"sixth", L"seventh", L"eighth", L"ninth"
    };

    static const tCIDLib::TCh* apszTens[10] =
    {
        L"tenth", L"eleventh", L"twelfth", L"thirteenth"
        , L"fourteenth", L"fifteenth", L"sixteenth"
        , L"seventeenth", L"eighteenth", L"ninteenth"
    };

    static const tCIDLib::TCh* apszTens2[10] =
    {
        L"", L"tenth", L"twentieth", L"thirtieth", L"fortieth", L"fiftieth", L"sixtieth"
        , L"seventieth", L"eightieth", L"ninetieth"
    };

    static const tCIDLib::TCh* apszTens3[10] =
    {
        L"", L"ten", L"twenty", L"thirty", L"forty", L"fifty", L"sixty"
        , L"seventy", L"eighty", L"ninety"
    };

    static const tCIDLib::TCh* apszDigits[10] =
    {
        L"zero", L"one", L"two", L"three", L"four", L"five", L"six"
        , L"seven", L"eight", L"nine"
    };

    //
    //  If it's greater than 999, then just do a standard format of it,
    //  else do our fancy format.
    //
    if (c4Val > 999)
    {
        strToFill.SetFormatted(c4Val);
    }
     else if (c4Val == 0)
    {
        // Special case this one
        strToFill = L"zeroth";
    }
     else
    {
        // Break out the 3 digits
        const tCIDLib::TCard4 c4Hundreds = c4Val / 100;
        const tCIDLib::TCard4 c4Tens = (c4Val - (c4Hundreds * 100)) / 10;
        const tCIDLib::TCard4 c4Ones = (c4Val % 10);

        // If we have hundreds, then deal with that
        strToFill.Clear();
        if (c4Hundreds)
        {
            strToFill.Append(apszDigits[c4Hundreds]);
            strToFill.Append(L" hundred");

            // If no tens or ones, add the suffix
            if (!c4Tens && !c4Ones)
                strToFill.Append(L"th");
        }

        // If we have any tens, the deal with that
        if (c4Tens)
        {
            if (!c4Hundreds && (c4Tens == 1))
                strToFill.Append(apszTens[c4Tens]);
            else if (c4Ones)
                strToFill.Append(apszTens3[c4Tens]);
            else
                strToFill.Append(apszTens2[c4Tens]);
        }

        // Deal with any ones, deal with that
        if (c4Ones)
            strToFill.Append(apszOnes[c4Ones]);
    }
}


//
//  Gets the lat/long info for either the server or the local client values.
//  On the server side, we cache them and recheck periodically, in case it has
//  changed.
//
tCIDLib::TVoid
TStdSystemCmdTar::GetLatLongInfo(const  tCIDLib::TBoolean       bServer
                                ,       tCIDLib::TFloat8&       f8Lat
                                ,       tCIDLib::TFloat8&       f8Long
                                , const TCQCSecToken&           sectUser)
{
    if (bServer)
    {
        // If we've not gotten the Lat/Long info lately, then get it again
        tCIDLib::TEncodedTime enctNow = TTime::enctNow();
        if (enctNow > m_enctNextLLCheck)
        {
            m_enctNextLLCheck = enctNow + (2 * kCIDLib::enctOneMinute);
            facCQCKit().bGetLocationInfo
            (
                tCIDLib::ECSSides::Server, m_f8Lat, m_f8Long, kCIDLib::True, sectUser
            );
        }

        // And return the new or cached values
        f8Lat = m_f8Lat;
        f8Long = m_f8Long;
    }
     else
    {
        // Just return the local lat/long info. Throw if not set yet
        facCQCKit().bGetLocationInfo(tCIDLib::ECSSides::Client, f8Lat, f8Long, kCIDLib::True, sectUser);
    }
}


// Handles some common init for our time channels
tCIDLib::TVoid TStdSystemCmdTar::InitTimers()
{
    // Init the 'timer channel' values
    const tCIDLib::TEncodedTime enctInit = TTime::enctNow();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCAct::c4MaxSysTarTimers; c4Index++)
        m_aenctTimers[c4Index] = enctInit;
}



