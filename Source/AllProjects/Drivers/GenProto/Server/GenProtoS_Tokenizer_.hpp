//
// FILE NAME: GenProtoS_Tokenizer_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/26/2002
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
//  This is the header file for the GenProtoS_Tokenizer.cpp file, which
//  defines a worker class, TGenProtoTokenizer, which just understands how to
//  break out tokens from the input protocol file. It returns us one token at
//  a time. Mainly it just serves to keep this grunt work code out of the
//  main body of the parsing code, so that that parsing code can remain more
//  oriented towards syntax without the confusion of this stuff.
//
//  Each call returns an enumerated value to indicate the token, and if it
//  has associated text, it returns the text.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoTokenizer
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoTokenizer : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Static methods
        // -------------------------------------------------------------------
        static TString strTokenName
        (
            const   tGenProtoS::ETokens   eToken
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoTokenizer
        (
            const   TMemBuf&                mbufSrc
            , const tCIDLib::TCard4         c4SrcBytes
        );

        ~TGenProtoTokenizer();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetBoolToken();

        tCIDLib::TBoolean bIfPeeked
        (
            const   tGenProtoS::ETokens     eToCheck
        );

        tCIDLib::TCard4 c4CurCol() const;

        tCIDLib::TCard4 c4CurLine() const;

        tCIDLib::TCard1 c1GetToken();

        tCIDLib::TCard2 c2GetToken();

        tCIDLib::TCard4 c4GetToken();

        tCIDLib::TCard4 c4ParseCardField
        (
            const   tGenProtoS::ETokens     eToCheck
        );

        tCIDLib::TVoid CheckNameToken
        (
                    TString&                strToFill
        );

        tCIDLib::TVoid CheckNextToken
        (
            const   tGenProtoS::ETokens     eToCheck
        );

        tCIDLib::TVoid CheckTextToken
        (
            const   tCIDLib::TCh* const     pszToCheck
        );

        tGenProtoS::ETokens eGetNextToken
        (
                    TString&                strText
            , const tCIDLib::TBoolean       bEOFOk = kCIDLib::False
        );

        tGenProtoS::ESpecNodes eGetBufType
        (
            const   tGenProtoS::ESpecNodes eLegal
        );

        tCQCKit::EFldAccess eParseAccessField();

        tGenProtoS::ETypes eParseExprType();

        tGenProtoS::ETypes eParseExprTypeLine();

        tCQCKit::EFldTypes eParseFieldType();

        tCQCKit::EFldTypes eParseFieldTypeLine();

        tGenProtoS::ETokens ePeekNextToken
        (
                    TString&                strText
        );

        tCIDLib::TFloat4 f4GetToken();

        tCIDLib::TFloat8 f8GetToken();

        tCIDLib::TFloat8 f8ParseFloatField
        (
            const   tGenProtoS::ETokens     eToCheck
        );

        tCIDLib::TVoid GetTextToken
        (
                    TString&                strValue
        );

        tCIDLib::TInt1 i1GetToken();

        tCIDLib::TInt2 i2GetToken();

        tCIDLib::TInt4 i4GetToken();

        tCIDLib::TInt4 i4ParseIntField
        (
            const   tGenProtoS::ETokens     eToCheck
        );

        tCIDLib::TVoid ParseQuotedField
        (
            const   tGenProtoS::ETokens     eToCheck
            ,       TString&                strValToFill
        );

        const TString& strModel
        (
            const   TString&                strToSet
        );

        const TString& strModel() const;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented ctors and operators
        // -------------------------------------------------------------------
        TGenProtoTokenizer(const TGenProtoTokenizer&);
        tCIDLib::TVoid operator=(const TGenProtoTokenizer&);


        // -------------------------------------------------------------------
        //   CLASS: TTokenInfo
        //  PREFIX: toki
        //
        //  TTokenInfo is used to support token look ahead. We have to remember
        //  the token type, its text, and its starting line/col position. These
        //  objects can be pushed onto a token pushback stack. eGetNextToken()
        //  will look there first before parsing more source text.
        // -------------------------------------------------------------------
        class TTokenInfo
        {
            public :
                // -----------------------------------------------------------
                //  Constructors and destructor
                // -----------------------------------------------------------
                TTokenInfo
                (
                    const   tGenProtoS::ETokens eToken
                    , const TString&            strText
                    , const tCIDLib::TCard4     c4StartLine
                    , const tCIDLib::TCard4     c4StartCol
                );

                TTokenInfo
                (
                    const   TTokenInfo&         tokiToCopy
                );

                ~TTokenInfo();


                tCIDLib::TVoid operator=
                (
                    const   TTokenInfo&         tokiToAssign
                );


                // -----------------------------------------------------------
                //  Public data members
                // -----------------------------------------------------------
                tCIDLib::TCard4     m_c4StartCol;
                tCIDLib::TCard4     m_c4StartLine;
                tGenProtoS::ETokens m_eToken;
                TString             m_strText;
        };
        typedef TStack<TTokenInfo>  TTokenStack;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseConditional
        (
                    TString&                strToUse
        );

        tCIDLib::TCh chGetNextChar();

        tCIDLib::TCh chPeekNextChar();

        tCIDLib::TCh chSpoolAChar();

        tCIDLib::TVoid CheckNumericExprType
        (
            const   TGenProtoExprNode* const pnodeToCheck
        );

        tGenProtoS::ETokens eExtractToken
        (
                    TString&                strText
            , const tCIDLib::TBoolean       bEOFOk
        );

        tGenProtoS::ETokens eMapToken
        (
            const   tCIDLib::TCh* const     pszToMap
        )   const;

        tCIDLib::TVoid EatLineRemainder();

        TTextConverter* ptcvtFindEncoding
        (
            const   TMemBuf&                mbufSrc
            , const tCIDLib::TCard4         c4Bytes
        );

        tCIDLib::TVoid PushBack
        (
            const   tCIDLib::TCh            chToPush
        );

        tCIDLib::TVoid SavePos();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4CurCol
        //  m_c4CurLine
        //      The actual current line/col info. This is not what is given
        //      to the outside world. They get the Save version, which represent
        //      where we started parsing thing that caused the problem. This
        //      represents the actual parsing position past what was just
        //      parsed.
        //
        //  m_c4SaveCol
        //  m_c4SaveLine
        //      The SavePos() method will put the current line/col into these
        //      variables. This lets the parsing code remember the place where
        //      it started parsing, so that it can give that as the error pos.
        //
        //  m_colTokenStack
        //      If we need to look ahead for whole tokens, then we get the
        //      token and push its info onto this stack.
        //
        //  m_fcolCharStack
        //      We use a fundamental stack collection as a single char
        //      pushback stack, so that we can look ahead one or more chars in
        //      the stream when needed.
        //
        //  m_fcolCondStack
        //      We have to track conditional sections that we have entered,
        //      so that we can know when we exit them. We just push a boolean
        //      onto the stack as we enter each block (since they can be
        //      nested) and pop it when we hit an end section. The boolean
        //      tells us whether each entered section would be valid for the
        //      current model or not.
        //
        //  m_strTargetModel
        //      We have to know the target device model, because we handle
        //      conditional sections automatically.
        //
        //  m_strmSrc
        //      A pointer to the stream to parse the text from. We create it
        //      from the passed buffer and encoding info.
        // -------------------------------------------------------------------
        tCIDLib::TCard4                 m_c4CurCol;
        tCIDLib::TCard4                 m_c4CurLine;
        tCIDLib::TCard4                 m_c4SaveCol;
        tCIDLib::TCard4                 m_c4SaveLine;
        TTokenStack                     m_colTokenStack;
        TFundStack<tCIDLib::TCh>        m_fcolCharStack;
        TFundStack<tCIDLib::TBoolean>   m_fcolCondStack;
        TString                         m_strTargetModel;
        TTextMBufInStream               m_strmSrc;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoTokenizer,TObject)
};

#pragma CIDLIB_POPPACK



