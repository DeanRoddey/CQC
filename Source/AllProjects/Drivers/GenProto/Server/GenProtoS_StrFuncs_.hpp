//
// FILE NAME: GenProtoS_StrFuncs_.hpp
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
//  This is the header file for the GenProtoS_StrFuncs.cpp file, which
//  defines some simple derivatives of the basic expression node class.
//  These are all oriented towards string processing.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoCatStrNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoCatStrNode : public TGenProtoArbChildNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoCatStrNode();

        TGenProtoCatStrNode
        (
            const   TGenProtoCatStrNode&    nodeToCopy
        );

        ~TGenProtoCatStrNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoCatStrNode& operator=
        (
            const   TGenProtoCatStrNode&    nodeToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        );

        tCIDLib::TVoid PostParseValidation();


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoCatStrNode,TGenProtoArbChildNode)
        DefPolyDup(TGenProtoCatStrNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoSetCaseNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoSetCaseNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoSetCaseNode
        (
                    TGenProtoExprNode* const pnodeStr
            , const tCIDLib::TBoolean        bToUpper
        );

        TGenProtoSetCaseNode
        (
            const   TGenProtoSetCaseNode&    nodeToCopy
        );

        ~TGenProtoSetCaseNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoSetCaseNode& operator=
        (
            const   TGenProtoSetCaseNode&    nodeToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        );

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bToUpper
        //      Indicates whether we are doing a toupper or tolower operation.
        //
        //  m_pnodeOfsExpr
        //      The string expression node that we get our value from.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bToUpper;
        TGenProtoExprNode*  m_pnodeStr;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoSetCaseNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoSetCaseNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoExtractStrBaseNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoExtractStrBaseNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TGenProtoExtractStrBaseNode();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const;

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        );

        tCIDLib::TVoid PostParseValidation();


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors and operators
        // -------------------------------------------------------------------
        TGenProtoExtractStrBaseNode
        (
            const   TString&                strName
            , const tGenProtoS::ESpecNodes  eSrcBuf
            , const tGenProtoS::ETypes      eType
            ,       TGenProtoExprNode* const pnodeOfsExpr
            ,       TGenProtoExprNode* const pnodeLenExpr
        );

        TGenProtoExtractStrBaseNode
        (
            const   TGenProtoExtractStrBaseNode& nodeToCopy
        );

        TGenProtoExtractStrBaseNode& operator=
        (
            const   TGenProtoExtractStrBaseNode& nodeToAssign
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid GetText
        (
                    TGenProtoCtx&           ctxThis
            ,       TString&                strToFill
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eSrcBuf
        //      The buffer to extract from.
        //
        //  m_pnodeOfsExpr
        //  m_pnodeLenExpr
        //      These are the two expressions which provide us with the
        //      offset and length values that we need to extract the string.
        // -------------------------------------------------------------------
        tGenProtoS::ESpecNodes  m_eSrcBuf;
        TGenProtoExprNode*      m_pnodeOfsExpr;
        TGenProtoExprNode*      m_pnodeLenExpr;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoExtractStrBaseNode,TGenProtoExprNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoExtractASCIIValNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoExtractASCIIValNode : public TGenProtoExtractStrBaseNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoExtractASCIIValNode
        (
            const   tGenProtoS::ETypes      eType
            , const tCIDLib::ERadices       eRadix
            , const tGenProtoS::ESpecNodes  eSrcBuf
            ,       TGenProtoExprNode* const pnodeOfsExpr
            ,       TGenProtoExprNode* const pnodeLenExpr
        );

        TGenProtoExtractASCIIValNode
        (
            const   TGenProtoExtractASCIIValNode& nodeToCopy
        );

        ~TGenProtoExtractASCIIValNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoExtractASCIIValNode& operator=
        (
            const   TGenProtoExtractASCIIValNode& nodeToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eRadix
        //      The radix to expect the text to be in if its a card/int
        //      type.
        // -------------------------------------------------------------------
        tCIDLib::ERadices   m_eRadix;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoExtractASCIIValNode,TGenProtoExtractStrBaseNode)
        DefPolyDup(TGenProtoExtractASCIIValNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoExtractStrNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoExtractStrNode : public TGenProtoExtractStrBaseNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoExtractStrNode
        (
            const   tGenProtoS::ESpecNodes  eType
            ,       TGenProtoExprNode* const pnodeOfsExpr
            ,       TGenProtoExprNode* const pnodeLenExpr
        );

        TGenProtoExtractStrNode
        (
            const   TGenProtoExtractStrNode& nodeToCopy
        );

        ~TGenProtoExtractStrNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoExtractStrNode& operator=
        (
            const   TGenProtoExtractStrNode& nodeToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        );


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoExtractStrNode,TGenProtoExtractStrBaseNode)
        DefPolyDup(TGenProtoExtractStrNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoExtractTokNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoExtractTokNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoExtractTokNode
        (
                    TGenProtoExprNode* const pnodeSrcExpr
            ,       TGenProtoExprNode* const pnodeTokNumExpr
            ,       TGenProtoExprNode* const pnodeSepCharExp
            ,       TGenProtoExprNode* const pnodeStripExpr
        );

        TGenProtoExtractTokNode
        (
            const   TGenProtoExtractTokNode& nodeToCopy
        );

        ~TGenProtoExtractTokNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoExtractTokNode& operator=
        (
            const   TGenProtoExtractTokNode& nodeToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        );

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        );

        tCIDLib::TVoid PostParseValidation();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pnodeSepCharExpr
        //  m_pnodeSrcCharExpr
        //  m_pnodeStripExpr
        //  m_pnodeTokNumExpr
        //      We get a separator character, a source value (which must be
        //      a string), the number of a token to extract, and whether we
        //      should strip whitespace from it.
        // -------------------------------------------------------------------
        TGenProtoExprNode*  m_pnodeSepCharExpr;
        TGenProtoExprNode*  m_pnodeSrcExpr;
        TGenProtoExprNode*  m_pnodeStripExpr;
        TGenProtoExprNode*  m_pnodeTokNumExpr;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoExtractTokNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoExtractTokNode)
};



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoFormatNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoFormatNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoFormatNode
        (
                    TGenProtoExprNode* const pnodeToFormat
            , const tCIDLib::TCard4         c4Width
            , const tCIDLib::EHJustify      eJustify
            , const tCIDLib::ERadices       eRadix
            , const tCIDLib::TCh            chFill
        );

        TGenProtoFormatNode
        (
            const   TGenProtoFormatNode&    nodeToCopy
        );

        ~TGenProtoFormatNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoFormatNode& operator=
        (
            const   TGenProtoFormatNode&    nodeToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        );

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Width
        //      The field width to format into. If it is zero, then just enough
        //      space to hold the result will be used.
        //
        //  m_chFill
        //      The character to use to fill the unused area of the target
        //      field, if any.
        //
        //  m_eJustify
        //      The justification to use. This is ignored unless the width is
        //      such that there is any justifying to do.
        //
        //  m_eRadix
        //      If the format expression is card/int, we will use this radix
        //      to format it.
        //
        //  m_pnodeToFormat
        //      The expression that provides the value we will format.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4Width;
        tCIDLib::TCh        m_chFill;
        tCIDLib::EHJustify  m_eJustify;
        tCIDLib::ERadices   m_eRadix;
        TGenProtoExprNode*  m_pnodeToFormat;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoFormatNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoFormatNode)
};


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoIsASCIIXNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoIsASCIIXNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        enum class EASCIITypes
        {
            Alpha
            , AlphaNum
            , DecDigit
            , HexDigit
            , Punct
            , Space

            , Count
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoIsASCIIXNode
        (
                    TGenProtoExprNode* const pnodeToCheckExpr
            , const EASCIITypes             eType
        );

        TGenProtoIsASCIIXNode
        (
                    TGenProtoExprNode* const pnodeToCheckExpr
            , const tGenProtoS::ETokens     eToken
        );

        TGenProtoIsASCIIXNode
        (
            const   TGenProtoIsASCIIXNode&  nodeToCopy
        );

        ~TGenProtoIsASCIIXNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoIsASCIIXNode& operator=
        (
            const   TGenProtoIsASCIIXNode&  nodeToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        );

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        );

        tCIDLib::TVoid PostParseValidation();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckIt
        (
            const   tCIDLib::TCard4         c4ToCheck
            , const EASCIITypes             eType
        )   const;

        EASCIITypes eTokToType
        (
            const   tGenProtoS::ETokens     eToXlat
        )   const;

        const tCIDLib::TCh* pszTypeToName
        (
            const   EASCIITypes             eToXlat
        )   const;

        const tCIDLib::TCh* pszTokToName
        (
            const   tGenProtoS::ETokens     eToXlat
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eType
        //      The particular type of ASCII category we are going to check
        //      for.
        //
        //  m_pnodeToCheckExpr
        //      Our child expression that provides the value that we are to
        //      check.
        // -------------------------------------------------------------------
        EASCIITypes         m_eType;
        TGenProtoExprNode*  m_pnodeToCheckExpr;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoIsASCIIXNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoIsASCIIXNode)
};

#pragma CIDLIB_POPPACK



