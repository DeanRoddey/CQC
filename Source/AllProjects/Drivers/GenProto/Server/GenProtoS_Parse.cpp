//
// FILE NAME: GenProtoS_Parse.cpp
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
//  This file implements the parsing oriented parts of the driver data class.
//  It implements the ParseFile() method, and the private methods that it
//  needs to do its thing. This stuff is just separated out here for
//  cleanliness, because its a good chunk of code that's never used again
//  after the data is parsed.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GenProtoS_.hpp"



// ---------------------------------------------------------------------------
//  TGenProtoSDriver: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoSDriver::ParseProtocol(const   THeapBuf&       mbufSrc
                                , const tCIDLib::TCard4 c4SrcBytes)
{
    // Greate our tokenizer that we'll use to do the parsing
    TGenProtoTokenizer tokToUse(mbufSrc, c4SrcBytes);

    //
    //  Tell the tokenizer about the target model. This is used to support
    //  conditional code per model.
    //
    tokToUse.strModel(cqcdcThis().strModel());

    // Next has to be the general protocol info
    tokToUse.CheckNextToken(tGenProtoS::ETokens::ProtocolInfo);
    ParseProtocolInfo(tokToUse);

    // Next is the optional constants and variables section
    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Constants))
        ParseVariables(tokToUse, kCIDLib::True);

    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Variables))
        ParseVariables(tokToUse, kCIDLib::False);

    // We can have optional maps
    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Maps))
        ParseMapsInfo(tokToUse);

    // And now we must see the fields
    tokToUse.CheckNextToken(tGenProtoS::ETokens::Fields);
    ParseFields(tokToUse);

    //
    //  And the queries and replies are optional, since some devices are
    //  write-only.
    //
    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Queries))
        ParseQueries(tokToUse);

    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Replies))
        ParseReplies(tokToUse);

    //
    //  We can have a msg matching block. If any replies are defined, then
    //  it must be present.
    //
    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::MsgMatching))
    {
        ParseMsgMatches(tokToUse);
    }
     else
    {
        if (!m_colReplies.bIsEmpty())
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_NoMsgMatches
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
            );
        }
    }

    // We can have optional write commands, since some devices are read only
    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::WriteCmds))
        ParseWriteCmds(tokToUse);

    // Now check for the TryConnect block
    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::TryConnect))
    {
        // If one way, then this is not valid
        if (m_eProtoType == tGenProtoS::EProtoTypes::OneWay)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_NotForOneWay
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
            );
        }
        ParseTryConnect(tokToUse);
    }

    // Now check for the Reconnect block
    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Reconnect))
    {
        // If one way, then this is not valid
        if (m_eProtoType == tGenProtoS::EProtoTypes::OneWay)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_NotForOneWay
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
            );
        }
        ParseReconnect(tokToUse);
    }

    // And the poll block is optional, since some devices are write only
    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::PollEvents))
        ParsePollEvents(tokToUse);

    // And we should now get an EOF token
    TString strText;
    if (tokToUse.eGetNextToken(strText, kCIDLib::True) != tGenProtoS::ETokens::EOF)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_ExpectedEOF
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(tokToUse.c4CurLine())
            , TCardinal(tokToUse.c4CurCol())
        );
    }
}


// ---------------------------------------------------------------------------
//  TGenProtoSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------
TGenProtoExprNode* TGenProtoSDriver::
pnodeMakeNumericConst(          TGenProtoTokenizer& tokToUse
                        , const TString&            strTokenText) const
{
    // See if it has a period in it
    tCIDLib::TCard4 c4Ofs;
    const tCIDLib::TBoolean bFloat = strTokenText.bFirstOccurrence
    (
        kCIDLib::chPeriod
        , c4Ofs
    );

    //
    //  If it starts with a minus sign, its a signed value, either integral
    //  or float. Else, assume its unsigned or possibly a positive float.
    //
    //  We convert them first at the largest of its type, and then create a
    //  node of the smallest it will fit into.
    //
    TGenProtoExprNode* pnodeRet = 0;
    try
    {
        if (bFloat)
        {
            const tCIDLib::TFloat8 f8Val = strTokenText.f8Val();
            if ((f8Val >= kCIDLib::f4MinFloat) && (f8Val <= kCIDLib::f4MaxFloat))
                pnodeRet = new TGenProtoConstNode(tCIDLib::TFloat4(f8Val));
            else
                pnodeRet = new TGenProtoConstNode(f8Val);
        }
         else if (strTokenText[0] == kCIDLib::chHyphenMinus)
        {
            const tCIDLib::TInt4 i4Val = strTokenText.i4Val();
            if ((i4Val >= kCIDLib::i1MinInt) && (i4Val <= kCIDLib::i1MaxInt))
                pnodeRet = new TGenProtoConstNode(tCIDLib::TInt1(i4Val));
            else if ((i4Val >= kCIDLib::i2MinInt) && (i4Val <= kCIDLib::i2MaxInt))
                pnodeRet = new TGenProtoConstNode(tCIDLib::TInt2(i4Val));
            else
                pnodeRet = new TGenProtoConstNode(i4Val);
        }
         else
        {
            const tCIDLib::TCard4 c4Val = strTokenText.c4Val();
            if (c4Val <= kCIDLib::c1MaxCard)
                pnodeRet = new TGenProtoConstNode(tCIDLib::TCard1(c4Val));
            else if (c4Val <= kCIDLib::c2MaxCard)
                pnodeRet = new TGenProtoConstNode(tCIDLib::TCard2(c4Val));
            else
                pnodeRet = new TGenProtoConstNode(c4Val);
        }
    }

    catch(...)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_BadNumConst
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(tokToUse.c4CurLine())
            , TCardinal(tokToUse.c4CurCol())
            , strTokenText
        );
    }
    return pnodeRet;
}


TGenProtoExprNode* TGenProtoSDriver::
pnodeParseExpression(       TGenProtoTokenizer&     tokToUse
                    ,       TString&                strToUse
                    , const tCIDLib::TCard4         c4Level
                    , const TGenProtoFldInfo* const pfldiCur
                    , const tGenProtoS::ESpecNodes  eExprType)
{
    tGenProtoS::ETokens eTok;
    TGenProtoExprNode*  pnodeCur = 0;

    //
    //  Get the next token, which must be one of the valid expressions, or
    //  a numeric or string constant.
    //
    eTok = tokToUse.eGetNextToken(strToUse);

    //
    //  Remember the current save position so that, when we get done, we can
    //  give the right position if the expression validation fails. Since
    //  expressions are nested, we could be a long way from here by the time
    //  we get back up to this level.
    //
    const tCIDLib::TCard4 c4OrgCol  = tokToUse.c4CurCol();
    const tCIDLib::TCard4 c4OrgLine = tokToUse.c4CurLine();

    switch(eTok)
    {
        // These all have one parameter
        case tGenProtoS::ETokens::ToCard1 :
        case tGenProtoS::ETokens::ToCard2 :
        case tGenProtoS::ETokens::ToCard4 :
        case tGenProtoS::ETokens::ToFloat4 :
        case tGenProtoS::ETokens::ToFloat8 :
        case tGenProtoS::ETokens::ToInt1 :
        case tGenProtoS::ETokens::ToInt2 :
        case tGenProtoS::ETokens::ToInt4 :
        case tGenProtoS::ETokens::ToLower :
        case tGenProtoS::ETokens::ToUpper :
        {
            // Next we have to have the open paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            //
            //  And get the next token, and call ourself to get our child
            //  expression that we are to cast.
            //
            TGenProtoExprNode* pnodeChild = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeChild = pnodeOptimizeNode(pnodeChild);
            TJanitor<TGenProtoExprNode> janNode(pnodeChild);

            // We have to have a close paren for our expression level
            tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);

            // Looks ok, so create our own node, store our child
            if (eTok == tGenProtoS::ETokens::ToLower)
                pnodeCur = new TGenProtoSetCaseNode(janNode.pobjOrphan(), kCIDLib::False);
            else if (eTok == tGenProtoS::ETokens::ToUpper)
                pnodeCur = new TGenProtoSetCaseNode(janNode.pobjOrphan(), kCIDLib::True);
            else
                pnodeCur = new TGenProtoCastNode(janNode.pobjOrphan(), eTok);
            break;
        }


        //
        //  This is a special cast node, because it can optionally provide
        //  formatting parameters, like this:
        //
        //  ToString(valExpr, radix, fldwidth, justify)
        //
        //  Radix is used if the value expression is card/int, else it is
        //  ignored. The last two parameters be skipped if you just want it
        //  to be big enough to hold the result. Else, you can provide them
        //  and have the resulting string formatted into a field of that
        //  width and justified (which can have the values Left or Right or
        //  Center.
        //
        //  If only the first parameter is provided, then it is assumed that
        //  the radix is decimal if the child expression is int/card.
        //
        case tGenProtoS::ETokens::ToString :
        {
            // Next we have to have the open paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            //
            //  And get the next token, and call ourself to get our child
            //  expression that we are to format.
            //
            TGenProtoExprNode* pnodeChild = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeChild = pnodeOptimizeNode(pnodeChild);
            TJanitor<TGenProtoExprNode> janNode(pnodeChild);

            //
            //  If the next parameter is a comma, we have another parm,
            //  else it must be the close paren so fall through.
            //
            tCIDLib::TCard4     c4Width = 0;
            tCIDLib::TCh        chFill = kCIDLib::chSpace;
            tCIDLib::EHJustify  eJustify = tCIDLib::EHJustify::Left;
            tCIDLib::ERadices   eRadix  = tCIDLib::ERadices::Dec;

            if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Comma))
            {
                // Get a cardinal const for the radix
                tCIDLib::TCard4 c4Tmp = tokToUse.c4GetToken();
                if (c4Tmp == 2)
                    eRadix = tCIDLib::ERadices::Dec;
                else if (c4Tmp == 8)
                    eRadix = tCIDLib::ERadices::Oct;
                else if (c4Tmp == 10)
                    eRadix = tCIDLib::ERadices::Dec;
                else if (c4Tmp == 16)
                    eRadix = tCIDLib::ERadices::Hex;
                else
                {
                    facGenProtoS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDErrs::errcParse_BadRadix
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(tokToUse.c4CurLine())
                        , TCardinal(tokToUse.c4CurCol())
                        , TCardinal(c4Tmp)
                    );
                }

                // If another parm, we have to get the width
                if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Comma))
                {
                    c4Width = tokToUse.c4GetToken();

                    // If another parm, we have to get the justification
                    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Comma))
                    {
                        tokToUse.GetTextToken(strToUse);

                        if (strToUse == L"Left")
                            eJustify = tCIDLib::EHJustify::Left;
                        else if (strToUse == L"Center")
                            eJustify = tCIDLib::EHJustify::Center;
                        else if (strToUse == L"Right")
                            eJustify = tCIDLib::EHJustify::Right;
                        else
                        {
                            facGenProtoS().ThrowErr
                            (
                                CID_FILE
                                , CID_LINE
                                , kGPDErrs::errcParse_BadJustify
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::Format
                                , TCardinal(tokToUse.c4CurLine())
                                , TCardinal(tokToUse.c4CurCol())
                                , strToUse
                            );
                        }

                        // If another comma, then its a fill char
                        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Comma))
                        {
                            tokToUse.GetTextToken(strToUse);
                            if (strToUse.c4Length() != 1)
                            {
                                facGenProtoS().ThrowErr
                                (
                                    CID_FILE
                                    , CID_LINE
                                    , kGPDErrs::errcParse_BadFillChar
                                    , tCIDLib::ESeverities::Failed
                                    , tCIDLib::EErrClasses::Format
                                    , TCardinal(tokToUse.c4CurLine())
                                    , TCardinal(tokToUse.c4CurCol())
                                    , strToUse
                                );
                            }
                            chFill = strToUse.chFirst();
                        }
                    }
                }
            }

            // We have to have a close paren for our expression level
            tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);

            // Looks ok, so create our node
            pnodeCur = new TGenProtoFormatNode
            (
                janNode.pobjOrphan()
                , c4Width
                , eJustify
                , eRadix
                , chFill
            );
            break;
        }


        //
        //  These are all ASCII character type checkers. They take one
        //  parameter, and all use the same node class, with just a different
        //  enum value.
        //
        case tGenProtoS::ETokens::IsASCIIAlpha :
        case tGenProtoS::ETokens::IsASCIIAlphaNum :
        case tGenProtoS::ETokens::IsASCIIDecDigit :
        case tGenProtoS::ETokens::IsASCIIHexDigit :
        case tGenProtoS::ETokens::IsASCIIPunct :
        case tGenProtoS::ETokens::IsASCIISpace :
        {
            // Next we have to have the open paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            //
            //  And get the next token, and call ourself to get our child
            //  expression that provides the value to check.
            //
            TGenProtoExprNode* pnodeChild = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeChild = pnodeOptimizeNode(pnodeChild);
            TJanitor<TGenProtoExprNode> janNode(pnodeChild);

            // We have to have a close paren for our expression level
            tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);

            // Looks ok, so create our own node, store our child
            pnodeCur = new TGenProtoIsASCIIXNode(janNode.pobjOrphan(), eTok);
            break;
        }


        //
        //  These have an arbitrary number of child nodes
        //
        case tGenProtoS::ETokens::ANDBits :
        case tGenProtoS::ETokens::CatStr :
        case tGenProtoS::ETokens::IfAll :
        case tGenProtoS::ETokens::IfAny :
        case tGenProtoS::ETokens::ORBits :
        case tGenProtoS::ETokens::XORBits :
        {
            // Next we have to have the open paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            // Create our node, and put a janitor on it till we are successful
            TGenProtoArbChildNode* pnodeNew = 0;
            if (eTok == tGenProtoS::ETokens::IfAll)
                pnodeNew = new TGenProtoIfAllNode();
            else if (eTok == tGenProtoS::ETokens::IfAny)
                pnodeNew = new TGenProtoIfAnyNode();
            else if (eTok == tGenProtoS::ETokens::CatStr)
                pnodeNew = new TGenProtoCatStrNode();
            else
                pnodeNew = new TGenProtoBitOpsNode(eTok);
            TJanitor<TGenProtoExprNode> janNode(pnodeNew);

            //
            //  And get the next token, and call ourself to get our child
            //  expressions that we are to OR.
            //
            tCIDLib::TCard4 c4Count = 0;
            while (kCIDLib::True)
            {
                //
                //  If not the first child, hen we have to have a comma or
                //  the terminating paren.
                //
                if (c4Count)
                {
                    eTok = tokToUse.eGetNextToken(strToUse);

                    if ((eTok != tGenProtoS::ETokens::Comma)
                    &&  (eTok != tGenProtoS::ETokens::CloseParen))
                    {
                        facGenProtoS().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kGPDErrs::errcParse_ExpectedParm
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Format
                            , TCardinal(tokToUse.c4CurLine())
                            , TCardinal(tokToUse.c4CurCol())
                        );
                    }

                    if (eTok == tGenProtoS::ETokens::CloseParen)
                        break;
                }

                //
                //  Its got to be a child expression, or the closing paren,
                //  so parse it and see.
                //
                TGenProtoExprNode* pnodeChild = pnodeParseExpression
                (
                    tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
                );
                pnodeChild = pnodeOptimizeNode(pnodeChild);

                // Else add it to our node
                pnodeNew->AddChild(pnodeChild);

                // And bump the count of children so far
                c4Count++;
            }

            // Looks ok, so release our node from the janitor
            pnodeCur = janNode.pobjOrphan();
            break;
        }


        //
        //  These require two child nodes
        //
        case tGenProtoS::ETokens::Add :
        case tGenProtoS::ETokens::Div :
        case tGenProtoS::ETokens::Equals :
        case tGenProtoS::ETokens::GreaterThan :
        case tGenProtoS::ETokens::LessThan :
        case tGenProtoS::ETokens::Mul :
        case tGenProtoS::ETokens::ShiftLeft :
        case tGenProtoS::ETokens::ShiftRight :
        case tGenProtoS::ETokens::Sub :
        case tGenProtoS::ETokens::TranscFunc :
        {
            // It starts with an open paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            TGenProtoExprNode* pnodeLHS = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeLHS = pnodeOptimizeNode(pnodeLHS);
            TJanitor<TGenProtoExprNode> janLHSNode(pnodeLHS);

            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeRHS = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeRHS = pnodeOptimizeNode(pnodeRHS);
            TJanitor<TGenProtoExprNode> janRHSNode(pnodeRHS);

            // And it must end with a close paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);

            // Ok, create our node using these child nodes
            janLHSNode.Orphan();
            janRHSNode.Orphan();
            if (eTok == tGenProtoS::ETokens::Add)
                pnodeCur = new TGenProtoAddNode(pnodeLHS, pnodeRHS);
            else if (eTok == tGenProtoS::ETokens::Div)
                pnodeCur = new TGenProtoDivNode(pnodeLHS, pnodeRHS);
            else if (eTok == tGenProtoS::ETokens::Equals)
                pnodeCur = new TGenProtoEqualsNode(pnodeLHS, pnodeRHS);
            else if (eTok == tGenProtoS::ETokens::GreaterThan)
                pnodeCur = new TGenProtoNumMagNode(pnodeLHS, pnodeRHS, kCIDLib::True);
            else if (eTok == tGenProtoS::ETokens::LessThan)
                pnodeCur = new TGenProtoNumMagNode(pnodeLHS, pnodeRHS, kCIDLib::False);
            else if (eTok == tGenProtoS::ETokens::Mul)
                pnodeCur = new TGenProtoMulNode(pnodeLHS, pnodeRHS);
            else if (eTok == tGenProtoS::ETokens::ShiftLeft)
                pnodeCur = new TGenProtoShiftNode(pnodeLHS, pnodeRHS, kCIDLib::False);
            else if (eTok == tGenProtoS::ETokens::ShiftRight)
                pnodeCur = new TGenProtoShiftNode(pnodeLHS, pnodeRHS, kCIDLib::True);
            else if (eTok == tGenProtoS::ETokens::Sub)
                pnodeCur = new TGenProtoSubNode(pnodeLHS, pnodeRHS);
            else if (eTok == tGenProtoS::ETokens::TranscFunc)
                pnodeCur = new TGenProtoTranscNode(pnodeLHS, pnodeRHS);
            else
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_UnknownNodeType
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(tokToUse.c4CurLine())
                    , TCardinal(tokToUse.c4CurCol())
                    , TCardinal(tCIDLib::c4EnumOrd(eTok))
                );
            }
            break;
        }


        //
        //  These are all related. The basic extract string takes a source
        //  buffer and an offset and length. The others can also take a radix.
        //
        case tGenProtoS::ETokens::ExtractStr :
        case tGenProtoS::ETokens::ExtractASCIICard :
        case tGenProtoS::ETokens::ExtractASCIIInt :
        case tGenProtoS::ETokens::ExtractASCIIFloat :
        {
            // It starts with an open paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            // And they have to indicate which buffer they are extracting from
            const tGenProtoS::ESpecNodes eBufType = tokToUse.eGetBufType(eExprType);
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);

            // Then we have the offset and length expressions
            TGenProtoExprNode* pnodeOfs = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeOfs = pnodeOptimizeNode(pnodeOfs);
            TJanitor<TGenProtoExprNode> janOfsNode(pnodeOfs);

            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeLen = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeLen = pnodeOptimizeNode(pnodeLen);
            TJanitor<TGenProtoExprNode> janLenNode(pnodeLen);

            if (eTok == tGenProtoS::ETokens::ExtractStr)
            {
                tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);
                pnodeCur = new TGenProtoExtractStrNode
                (
                    eBufType, janOfsNode.pobjOrphan(), janLenNode.pobjOrphan()
                );
            }
             else
            {
                // For the rest we have to have a radix
                tCIDLib::ERadices eRadix = tCIDLib::ERadices::Dec;
                tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);

                const tCIDLib::TCard4 c4Tmp = tokToUse.c4GetToken();
                if (c4Tmp == 2)
                    eRadix = tCIDLib::ERadices::Dec;
                else if (c4Tmp == 8)
                    eRadix = tCIDLib::ERadices::Oct;
                else if (c4Tmp == 10)
                    eRadix = tCIDLib::ERadices::Dec;
                else if (c4Tmp == 16)
                    eRadix = tCIDLib::ERadices::Hex;
                else
                {
                    facGenProtoS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDErrs::errcParse_BadRadix
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(tokToUse.c4CurLine())
                        , TCardinal(tokToUse.c4CurCol())
                        , TCardinal(c4Tmp)
                    );
                }

                // And it must end with a close paren
                tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);

                // According the type, create a node with the right type
                tGenProtoS::ETypes eType;
                if (eTok == tGenProtoS::ETokens::ExtractASCIICard)
                    eType = tGenProtoS::ETypes::Card4;
                else if (eTok == tGenProtoS::ETokens::ExtractASCIIFloat)
                    eType = tGenProtoS::ETypes::Float8;
                else if (eTok == tGenProtoS::ETokens::ExtractASCIIInt)
                    eType = tGenProtoS::ETypes::Int4;

                pnodeCur = new TGenProtoExtractASCIIValNode
                (
                    eType
                    , eRadix
                    , eBufType
                    , janOfsNode.pobjOrphan()
                    , janLenNode.pobjOrphan()
                );
            }
            break;
        }

        //
        //  Thse take three expresions
        //
        case tGenProtoS::ETokens::BoolSel :
        {
            // It starts with an open paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            TGenProtoExprNode* pnodeOne = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeOne = pnodeOptimizeNode(pnodeOne);
            TJanitor<TGenProtoExprNode> janNode1(pnodeOne);

            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeTwo = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeTwo = pnodeOptimizeNode(pnodeTwo);
            TJanitor<TGenProtoExprNode> janNode2(pnodeTwo);

            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeThree = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeThree = pnodeOptimizeNode(pnodeThree);
            TJanitor<TGenProtoExprNode> janNode3(pnodeThree);

            // And it must end with a close paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);

            // Ok, create our node using these child nodes
            if (eTok == tGenProtoS::ETokens::BoolSel)
            {
                pnodeCur = new TGenProtoBoolSelNode
                (
                    janNode1.pobjOrphan()
                    , janNode2.pobjOrphan()
                    , janNode3.pobjOrphan()
                );
            }
            break;
        }


        //
        //  These take 4 parms
        //
        case tGenProtoS::ETokens::ExtractToken :
        {
            // It starts with an open paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            TGenProtoExprNode* pnodeOne = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeOne = pnodeOptimizeNode(pnodeOne);
            TJanitor<TGenProtoExprNode> janNode1(pnodeOne);

            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeTwo = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeTwo = pnodeOptimizeNode(pnodeTwo);
            TJanitor<TGenProtoExprNode> janNode2(pnodeTwo);

            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeThree = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeThree = pnodeOptimizeNode(pnodeThree);
            TJanitor<TGenProtoExprNode> janNode3(pnodeThree);

            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeFour = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeFour = pnodeOptimizeNode(pnodeFour);
            TJanitor<TGenProtoExprNode> janNode4(pnodeFour);

            // And it must end with a close paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);

            // Ok, create our node using these child nodes
            if (eTok == tGenProtoS::ETokens::ExtractToken)
            {
                pnodeCur = new TGenProtoExtractTokNode
                (
                    janNode1.pobjOrphan()
                    , janNode2.pobjOrphan()
                    , janNode3.pobjOrphan()
                    , janNode4.pobjOrphan()
                );
            }
            break;
        }


        //
        //  These take 5 parms
        //
        case tGenProtoS::ETokens::RangeRotate :
        case tGenProtoS::ETokens::ScaleRange :
        {
            // It starts with an open paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            TGenProtoExprNode* pnodeOne = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeOne = pnodeOptimizeNode(pnodeOne);
            TJanitor<TGenProtoExprNode> janNode1(pnodeOne);

            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeTwo = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeTwo = pnodeOptimizeNode(pnodeTwo);
            TJanitor<TGenProtoExprNode> janNode2(pnodeTwo);

            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeThree = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeThree = pnodeOptimizeNode(pnodeThree);
            TJanitor<TGenProtoExprNode> janNode3(pnodeThree);

            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeFour = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeFour = pnodeOptimizeNode(pnodeFour);
            TJanitor<TGenProtoExprNode> janNode4(pnodeFour);

            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeFive = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeFive = pnodeOptimizeNode(pnodeFive);
            TJanitor<TGenProtoExprNode> janNode5(pnodeFive);

            // And it must end with a close paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);

            // Ok, create our node using these child nodes
            if (eTok == tGenProtoS::ETokens::RangeRotate)
            {
                pnodeCur = new TGenProtoRangeRotNode
                (
                    janNode1.pobjOrphan()
                    , janNode2.pobjOrphan()
                    , janNode3.pobjOrphan()
                    , janNode4.pobjOrphan()
                    , janNode5.pobjOrphan()
                );
            }
             else if (eTok == tGenProtoS::ETokens::ScaleRange)
            {
                pnodeCur = new TGenProtoScaleRngNode
                (
                    janNode1.pobjOrphan()
                    , janNode2.pobjOrphan()
                    , janNode3.pobjOrphan()
                    , janNode4.pobjOrphan()
                    , janNode5.pobjOrphan()
                );
            }
            break;
        }


        // This one is a speci case since it takes a buffer
        case tGenProtoS::ETokens::CRC16 :
        {
            // We have to see an open parameter next
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            //
            //  Now we have to either see the &Reply or &WriteCmd tokens
            //  which indicate the particular buffer this node is going
            //  to access. We check the passed special nodes value to
            //  know if it is legal to access that particular node in this
            //  type of expression.
            //
            const tGenProtoS::ESpecNodes eBufType = tokToUse.eGetBufType(eExprType);

            // Then a comma and the offset expression
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeOfs = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeOfs = pnodeOptimizeNode(pnodeOfs);
            TJanitor<TGenProtoExprNode> janOfsNode(pnodeOfs);

            // Then a comma and the len expression
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeLen = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeLen = pnodeOptimizeNode(pnodeLen);
            TJanitor<TGenProtoExprNode> janLenNode(pnodeLen);

            // Then a comma and the polynomial
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodePoly = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodePoly = pnodeOptimizeNode(pnodePoly);
            TJanitor<TGenProtoExprNode> janPolyNode(pnodePoly);

            // Then a comma and the loop count
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeCount = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeCount = pnodeOptimizeNode(pnodeCount);
            TJanitor<TGenProtoExprNode> janCountNode(pnodeCount);

            pnodeCur = new TGenProtoCRC16Node
            (
                eBufType
                , janOfsNode.pobjOrphan()
                , janLenNode.pobjOrphan()
                , janPolyNode.pobjOrphan()
                , janCountNode.pobjOrphan()
            );

            // And we need a close paren for this expression
            tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);
            break;
        }


        //
        //  Both of these start with a buffer reference, a data type, and a
        //  cardinal offset value, so they share enough code to do them in
        //  the same case.
        //
        case tGenProtoS::ETokens::Extract :
        case tGenProtoS::ETokens::CheckSum :
        {
            // We have to see an open parameter next
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            //
            //  Now we have to either see the &Reply or &WriteCmd tokens
            //  which indicate the particular buffer this node is going
            //  to access. We check the passed special nodes value to
            //  know if it is legal to access that particular node in this
            //  type of expression.
            //
            const tGenProtoS::ESpecNodes eBufType = tokToUse.eGetBufType(eExprType);

            // Next we have a comma and the type parameter
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            tGenProtoS::ETypes eType = tokToUse.eParseExprType();

            // Then a comma and the offset expression
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeOfs = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeOfs = pnodeOptimizeNode(pnodeOfs);
            TJanitor<TGenProtoExprNode> janOfsNode(pnodeOfs);

            //
            //  At this point the two variants go their different ways. For
            //  an extract, we are done and can create the node. For a check
            //  sum, we have two more parms to get.
            //
            if (eTok == tGenProtoS::ETokens::Extract)
            {
                pnodeCur = new TGenProtoArrayExtractNode
                (
                    eBufType, janOfsNode.pobjOrphan(), eType
                );
            }
             else
            {
                // Next we have a comma and the length expression
                tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
                TGenProtoExprNode* pnodeLen = pnodeParseExpression
                (
                    tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
                );
                pnodeLen = pnodeOptimizeNode(pnodeLen);
                TJanitor<TGenProtoExprNode> janLenNode(pnodeLen);

                // And a comma and the checksum type
                tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
                tokToUse.GetTextToken(strToUse);

                // Make sure the type is a legal type and set up the enum
                tGenProtoS::ECheckSums eSumType;
                if (strToUse == L"AddWithOverflow")
                {
                    eSumType = tGenProtoS::ECheckSums::AddWithOverflow;
                }
                 else if (strToUse == L"Xor1")
                {
                    eSumType = tGenProtoS::ECheckSums::Xor1;
                }
                 else
                {
                    facGenProtoS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDErrs::errcParse_BadChecksumType
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(tokToUse.c4CurLine())
                        , TCardinal(tokToUse.c4CurCol())
                        , strToUse
                    );
                }

                // Ok, we can create the node now
                pnodeCur = new TGenProtoCheckSumNode
                (
                    eBufType
                    , eType
                    , janOfsNode.pobjOrphan()
                    , janLenNode.pobjOrphan()
                    , eSumType
                );
            }

            // And we need a close paren for this expression
            tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);
            break;
        }


        //
        //  These reference a field or variable, via its field info or variable
        //  info object. They look the same to the tokenizer, so we have to
        //  figure out which type it is by looking up the name.
        //
        case tGenProtoS::ETokens::FieldRef :
        {
            //
            //  See if its a field ref, so the text must be the name of a
            //  defined driver field.
            //
            TGenProtoFldInfo* pfldiRef = pfldiFindByName(strToUse);
            if (pfldiRef)
            {
                // Make sure this is legal for the current type of expression
                if (!tCIDLib::bAllBitsOn(eExprType, tGenProtoS::ESpecNodes::FldVal))
                {
                    facGenProtoS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDErrs::errcParse_NoFldRef
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotFound
                        , TCardinal(tokToUse.c4CurLine())
                        , TCardinal(tokToUse.c4CurCol())
                    );
                }

                // Create a field ref node for this field
                pnodeCur = new TGenProtoFldValNode(pfldiRef);
            }
             else
            {
                // Not a field, so try a variable
                TGenProtoVarInfo* pvariRef = pvariFindByName(strToUse);
                if (pvariRef)
                {
                    // Create a variable ref node for this variable
                    pnodeCur = new TGenProtoVarValNode(pvariRef);
                }
                 else
                {
                    facGenProtoS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDErrs::errcParse_VarNotFound
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotFound
                        , TCardinal(tokToUse.c4CurLine())
                        , TCardinal(tokToUse.c4CurCol())
                        , strToUse
                    );
                }
            }
            break;
        }


        //
        //  These take a map and expression, and map to/from the two sides
        //  of the map items.
        //
        case tGenProtoS::ETokens::MapTo :
        case tGenProtoS::ETokens::MapFrom :
        {
            //
            //  We need an open param, then the first parameter which is
            //  the name of the map we are mapping from.
            //
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);
            tokToUse.CheckNameToken(strToUse);

            // See if we have a map of this name
            TGenProtoMap* pmapSrc = pmapFindByName(strToUse);
            if (!pmapSrc)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_MapNotFound
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , TCardinal(tokToUse.c4CurLine())
                    , TCardinal(tokToUse.c4CurCol())
                    , strToUse
                );
            }

            // And we must then have a comma and an expression
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);
            TGenProtoExprNode* pnodeChild = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeChild = pnodeOptimizeNode(pnodeChild);

            // Create a mapping node and give it a pointer to this map
            if (eTok == tGenProtoS::ETokens::MapTo)
                pnodeCur = new TGenProtoMapToNode(pmapSrc, pnodeChild);
            else
                pnodeCur = new TGenProtoMapFromNode(pmapSrc, pnodeChild);

            // And it must end with a close paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);
            break;
        }


        case tGenProtoS::ETokens::False :
            pnodeCur = new TGenProtoConstNode(kCIDLib::False);
            break;

        case tGenProtoS::ETokens::True :
            pnodeCur = new TGenProtoConstNode(kCIDLib::True);
            break;


        case tGenProtoS::ETokens::NumericConst :
        {
            pnodeCur = pnodeMakeNumericConst(tokToUse, strToUse);
            break;
        }


        case tGenProtoS::ETokens::QuotedString :
        {
            pnodeCur = new TGenProtoConstNode(strToUse);
            break;
        }


        case tGenProtoS::ETokens::ReplyLen :
        {
            pnodeCur = new TGenProtoArrayLenNode(tGenProtoS::ESpecNodes::ReplyBuf);
            break;
        }


        case tGenProtoS::ETokens::WriteCmdLen :
        {
            pnodeCur = new TGenProtoArrayLenNode(tGenProtoS::ESpecNodes::SendBuf);
            break;
        }


        case tGenProtoS::ETokens::InputByte :
        {
            pnodeCur = new TGenProtoInputByteNode;
            break;
        }


        //
        //  A special one that takes an expression that resolves to a float
        //  value, and a string that indicates a rounding mode. It rounds the
        //  child expression's value and sets the rounded value as its value
        //
        case tGenProtoS::ETokens::RoundFloat :
        {
            // Next we have to have the open paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            //
            //  And get the next token, and call ourself to get our child
            //  expression that we are to round.
            //
            TGenProtoExprNode* pnodeChild = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeChild = pnodeOptimizeNode(pnodeChild);
            TJanitor<TGenProtoExprNode> janNode(pnodeChild);

            // We need a comma between parms
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);

            // And now we need a string, which we convert to a rounding type
            tokToUse.GetTextToken(strToUse);
            tCIDLib::ERoundTypes eType;
            if (strToUse == L"Up")
                eType = tCIDLib::ERoundTypes::Up;
            else if (strToUse == L"Down")
                eType = tCIDLib::ERoundTypes::Down;
            else if (strToUse == L"Closest")
                eType = tCIDLib::ERoundTypes::Closest;
            else
            {
                // Not a valid value
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_BadRoundType
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , TCardinal(tokToUse.c4CurLine())
                    , TCardinal(tokToUse.c4CurCol())
                    , strToUse
                );
            }

            // We have to have a close paren for our expression level
            tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);

            // Looks ok, so create our own node, store our child
            pnodeCur = new TGenProtoRoundFloatNode(janNode.pobjOrphan(), eType);
            break;
        }


        //
        //  A special one that takes a boolean child expression and just
        //  negates the value.
        //
        case tGenProtoS::ETokens::Not :
        {
            // Next we have to have the open paren
            tokToUse.CheckNextToken(tGenProtoS::ETokens::OpenParen);

            //
            //  And get the next token, and call ourself to get our child
            //  expression that we are to cast.
            //
            TGenProtoExprNode* pnodeChild = pnodeParseExpression
            (
                tokToUse, strToUse, c4Level + 1, pfldiCur, eExprType
            );
            pnodeChild = pnodeOptimizeNode(pnodeChild);
            TJanitor<TGenProtoExprNode> janNode(pnodeChild);

            // We have to have a close paren for our expression level
            tokToUse.CheckNextToken(tGenProtoS::ETokens::CloseParen);

            // Looks ok, so create our own node, store our child
            pnodeCur = new TGenProtoNOTNode(janNode.pobjOrphan());
            break;
        }


        //
        //  This is a special case node that can only be used within a write
        //  command, and represents the field value that was sent to write out
        //  to the device.
        //
        case tGenProtoS::ETokens::WriteVal :
        {
            // Make sure this is legal for the current type of expression
            if (!tCIDLib::bAllBitsOn(eExprType, tGenProtoS::ESpecNodes::WriteVal)
            ||  !pfldiCur)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_NoWriteVal
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , TCardinal(tokToUse.c4CurLine())
                    , TCardinal(tokToUse.c4CurCol())
                );
            }

            pnodeCur = new TGenProtoWriteValNode(pfldiCur->flddInfo().eType());
            break;
        }


        default :
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_Expected
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , TString(L"expression")
            );
            break;
    }

    //
    //  Validate this node. If anything is wrong, some expression node will
    //  throw. We catch it and report it.
    //
    try
    {
        pnodeCur->PostParseValidation();
    }

    catch(const TError& errToCatch)
    {
        // Clean up the node before we throw
        delete pnodeCur;

        // Note that we use the source file position that we saved above
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_ExprValidation
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4OrgLine)
            , TCardinal(c4OrgCol)
            , errToCatch.strErrText()
        );
    }

    return pnodeCur;
}


tCIDLib::TVoid TGenProtoSDriver::ParseFields(TGenProtoTokenizer& tokToUse)
{
    // The caller checked the opening keyword, which is followed by an equal
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    //
    //  Create a special temp context for this work. The field limits is
    //  an expression which we parse and evaluate immediately, then throw
    //  away after getting the value out. It doesn't have access to any
    //  special values.
    //
    TGenProtoCtx ctxLimits(tGenProtoS::ESpecNodes::None, L"TmpFieldDef");

    //
    //  Loop until we see the EndFields token. Until then we should just see
    //  field definitions. We just assigned consequtive numbers to the fields
    //  as ids. We start at zero so that we can use them directly as indexes
    //  into some collections.
    //
    tCIDLib::TBoolean       bWriteAlways;
    tCIDLib::TCard4         c4Id = 0;
    tCQCKit::EFldAccess     eAccess;
    tGenProtoS::ETokens     eToken;
    tCQCKit::EFldTypes      eType;
    TString                 strLimits;
    TString                 strName;
    TString                 strText;
    while (kCIDLib::True)
    {
        // Break out on the end maps token
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndFields))
            break;

        //
        //  We can get a field or revive field here. The only difference is
        //  that we set a value on the revive field specially. It's followed
        //  by an equal sign then the name of the field.
        //
        eToken = tokToUse.eGetNextToken(strText);
        if ((eToken != tGenProtoS::ETokens::Field)
        &&  (eToken != tGenProtoS::ETokens::ReviveField))
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_Expected
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , TString(L"Field or ReviveField")
            );
        }
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        tokToUse.CheckNameToken(strName);

        // Remember if it's the revive field
        const tCIDLib::TBoolean bReviveField(eToken == tGenProtoS::ETokens::ReviveField);

        //
        //  Make sure that no existing field or variable has this name. If
        //  one does, then throw.
        //
        if (pfldiFindByName(strName) || pvariFindByName(strName))
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_FldOrVarExists
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strName
            );
        }

        // Ok, get the other fields that we need to create the field def object
        eType = tokToUse.eParseFieldTypeLine();
        eAccess = tokToUse.eParseAccessField();

        //
        //  Get any of the optional values. They don't have to be in any
        //  particular order, so we have to loop until we hit the end of
        //  field indicator. Set any values we are going to gather up to
        //  their defaults in case they are not set explicitly.
        //
        bWriteAlways = kCIDLib::False;
        strLimits.Clear();
        tCQCKit::EFldSTypes eSemType = tCQCKit::EFldSTypes::Generic;
        while(kCIDLib::True)
        {
            eToken = tokToUse.eGetNextToken(strText);

            // Break out on an end of field definition
            if (eToken == tGenProtoS::ETokens::EndField)
                break;

            // Anything else has to have an equal sign next before the value
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

            // And process the value based on type of of value
            if (eToken == tGenProtoS::ETokens::Limits)
            {
                //
                //  It's a limit string. Note that it's an expression, so we have
                //  to parse the expression, evaluate it, get the resulting
                //  value and store it, then throw away the expression.
                //
                //  We created a special expression context above, which doesn't
                //  allow for any special values. All they can use in these
                //  expressions are hard coded numeric/string constant values,
                //  or their previously defined variables or constants.
                //
                //  We also have a specialized LimitMap type of line which
                //  generates a limit string from a map, which is very commonly
                //  desired. So it just takes the name of a map.
                //

                //
                //  Parse the next expression, put a janitor on it, and then
                //  evaluate it. Don't worry about optimzing, since this is
                //  the only time it will be called.
                //
                {
                    TGenProtoExprNode* pnodeChild = pnodeParseExpression
                    (
                      tokToUse
                      , strText
                      , 0
                      , 0
                      , tGenProtoS::ESpecNodes::None
                    );
                    TJanitor<TGenProtoExprNode> janExpr(pnodeChild);
                    pnodeChild->Evaluate(ctxLimits);
                    strLimits = pnodeChild->evalCur().strValue();
                }

                // Has to end in a semi-colon
                tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
            }
             else if (eToken == tGenProtoS::ETokens::LimitFromMap)
            {
                // We should have a map name
                tokToUse.CheckNameToken(strText);

                // And that name must be the name of a map
                TGenProtoMap* pmapSrc = pmapFindByName(strText);
                if (!pmapSrc)
                {
                    facGenProtoS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDErrs::errcParse_MapNotFound
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotFound
                        , TCardinal(tokToUse.c4CurLine())
                        , TCardinal(tokToUse.c4CurCol())
                        , strText
                    );
                }

                //
                //  Generate the limit string by iterating the map and building
                //  up the left side text values.
                //
                pmapSrc->BuildLimString(strLimits);

                // Has to end in a semi-colon
                tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
            }
             else if (eToken == tGenProtoS::ETokens::Flags)
            {
                //
                //  Loop while we have items. Each one is a comma separated
                //  value. It's terminated by the semi-colon.
                //
                while (kCIDLib::True)
                {
                    eToken = tokToUse.eGetNextToken(strText);

                    if (eToken == tGenProtoS::ETokens::WriteAlways)
                    {
                        // Set a flag which we'll use below
                        bWriteAlways = kCIDLib::True;

                        // Get another token since we ate the one gotten above
                        eToken = tokToUse.eGetNextToken(strText);
                    }
                     else
                    {
                        // We expected to see a flag name
                        facGenProtoS().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kGPDErrs::errcParse_Expected
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Format
                            , TCardinal(tokToUse.c4CurLine())
                            , TCardinal(tokToUse.c4CurCol())
                            , TString(L"Flag")
                        );
                    }

                    // We can see a semi-colon at the end
                    if (eToken == tGenProtoS::ETokens::SemiColon)
                        break;

                    // It has to be a semi-colon before the next one
                    if (eToken != tGenProtoS::ETokens::Comma)
                    {
                        facGenProtoS().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kGPDErrs::errcParse_Expected
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Format
                            , TCardinal(tokToUse.c4CurLine())
                            , TCardinal(tokToUse.c4CurCol())
                            , TString(L"comma")
                        );
                    }
                }
            }
             else if (eToken == tGenProtoS::ETokens::SemType)
            {
                //
                //  It's a semenatic field type, so just get the next token
                //  and try to convert it to a semantic field type.
                //
                tokToUse.eGetNextToken(strText);
                eSemType = tCQCKit::eAltXlatEFldSTypes(strText);
                if (eSemType == tCQCKit::EFldSTypes::Count)
                {
                    facGenProtoS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDErrs::errcParse_Expected
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(tokToUse.c4CurLine())
                        , TCardinal(tokToUse.c4CurCol())
                        , TString(L"semantic field type")
                    );
                }

                // Has to end in a semi-colon
                tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
            }
        }

        // There has to be a semi-colon after the end field
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

        //
        //  If the revive field, then it must be boolean and read/write.
        //  If it's ok, then mark it.
        //
        if (bReviveField)
        {
            tCIDLib::TErrCode errcThrow = 0;

            // If not a DeadIfOff protocol, then obviously not valid
            if (m_eProtoType != tGenProtoS::EProtoTypes::DeadIfOff)
            {
                errcThrow = kGPDErrs::errcParse_InvalidReviveFld;
            }
             else if ((eType != tCQCKit::EFldTypes::Boolean)
                  ||  (eAccess != tCQCKit::EFldAccess::ReadWrite))
            {
                errcThrow = kGPDErrs::errcParse_BadReviveFld;
            }

            if (errcThrow)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , errcThrow
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(tokToUse.c4CurLine())
                    , TCardinal(tokToUse.c4CurCol())
                );
            }
        }

        // Alrighty, looks good so add a new field def to our list
        TGenProtoFldInfo& fldiNew = m_colFields.objAdd
        (
            TGenProtoFldInfo(strName, eType, c4Id++, strLimits, eAccess, eSemType)
        );
        if (bReviveField)
            fldiNew.bIsReviveField(bReviveField);
        if (bWriteAlways)
            fldiNew.flddInfo().bAlwaysWrite(bWriteAlways);
    }
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

    //
    //  Do some validation of info.
    //
    //  First, if this is a DeadIfOff protocol, there must be a Revive
    //  field (and just one), and if it's not, there shouldn't be one.
    //
    tCIDLib::TCard4 c4Revives = 0;
    tCIDLib::TCard4 c4FldCount = m_colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FldCount; c4Index++)
    {
        TGenProtoFldInfo& fldiCur = m_colFields[c4Index];
        if (fldiCur.bIsReviveField())
            c4Revives++;
    }

    tCIDLib::TErrCode errcThrow = 0;
    if (m_eProtoType == tGenProtoS::EProtoTypes::DeadIfOff)
    {
        if (c4Revives == 0)
            errcThrow = kGPDErrs::errcPPVal_NoReviveFld;
        else if (c4Revives > 1)
            errcThrow = kGPDErrs::errcPPVal_MultiReviveFld;
    }

    if (errcThrow)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , errcThrow
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Protocol
        );
    }
}


//
//  Parses the maps section of the file (Maps=), which contains an option
//  set of text-to-value mappings used the following areas of the file. Each
//  map is named and we load them up into a map collection with the name as
//  the key. Later, when they are referred to in MapTo() and MapFrom() methods,
//  we will look up the map and give the function call expression nodes a
//  pointer to their map.
//
tCIDLib::TVoid TGenProtoSDriver::ParseMapsInfo(TGenProtoTokenizer& tokToUse)
{
    // The caller checked the opening keyword, which is followed by an equal
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    //
    //  Loop here until we see the EndMaps item. For each loop we should see
    //  a map definition.
    //
    tGenProtoS::ETokens   eToken;
    tGenProtoS::ETypes    eType;
    TString               strText;
    while (kCIDLib::True)
    {
        eToken = tokToUse.eGetNextToken(strText);

        // Break out on the end maps token
        if (eToken == tGenProtoS::ETokens::EndMaps)
            break;

        if (eToken != tGenProtoS::ETokens::Map)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_Expected
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , TString(L"Map")
            );
        }

        // We have to have an equal sign, then the name of the map.
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        tokToUse.CheckNameToken(strText);

        // Make sure it's not a duplicate map name
        if (m_colMaps.bKeyExists(strText))
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_DupMap
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strText
            );
        }

        // And then we have to have the type=xxx line
        eType = tokToUse.eParseExprTypeLine();

        // And next we have to have Items= and the list of items
        tokToUse.CheckNextToken(tGenProtoS::ETokens::Items);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

        // Add a new map object with this name
        TGenProtoMap& mapNew = m_colMaps.objAdd(TGenProtoMap(strText, eType));

        //
        //  And now lets parse the items and add them to the map. We have
        //  to do the value based on the type of the map, so that we can
        //  parse it correctly.
        //
        while (kCIDLib::True)
        {
            //
            //  Each item starts with Item=, or it could be the EndItems
            //  token that ends our item list.
            //
            eToken = tokToUse.eGetNextToken(strText);

            // Break out on the end items token
            if (eToken == tGenProtoS::ETokens::EndItems)
                break;

            // Else its got to be an item, and an equal sign
            if (eToken != tGenProtoS::ETokens::Item)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_Expected
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(tokToUse.c4CurLine())
                    , TCardinal(tokToUse.c4CurCol())
                    , TString(L"Item")
                );
            }
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

            // Now we should get the text, a comma, and the value
            tokToUse.GetTextToken(strText);
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);

            switch(eType)
            {
                case tGenProtoS::ETypes::Card1 :
                    mapNew.AddItem(TGenProtoMapItem(strText, tokToUse.c1GetToken()));
                    break;

                case tGenProtoS::ETypes::Card2 :
                    mapNew.AddItem(TGenProtoMapItem(strText, tokToUse.c2GetToken()));
                    break;

                case tGenProtoS::ETypes::Card4 :
                    mapNew.AddItem(TGenProtoMapItem(strText, tokToUse.c4GetToken()));
                    break;

                case tGenProtoS::ETypes::Int1 :
                    mapNew.AddItem(TGenProtoMapItem(strText, tokToUse.i1GetToken()));
                    break;

                case tGenProtoS::ETypes::Int2 :
                    mapNew.AddItem(TGenProtoMapItem(strText, tokToUse.i2GetToken()));
                    break;

                case tGenProtoS::ETypes::Int4 :
                    mapNew.AddItem(TGenProtoMapItem(strText, tokToUse.i4GetToken()));
                    break;

                case tGenProtoS::ETypes::Float4 :
                    mapNew.AddItem(TGenProtoMapItem(strText, tokToUse.f4GetToken()));
                    break;

                case tGenProtoS::ETypes::Float8 :
                    mapNew.AddItem(TGenProtoMapItem(strText, tokToUse.f8GetToken()));
                    break;

                default :
                    facGenProtoS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDErrs::errcParse_Expected
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(tokToUse.c4CurLine())
                        , TCardinal(tokToUse.c4CurCol())
                        , TString(L"data type")
                    );
                    break;
            };

            // And it has to end with a semicolon
            tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
        }

        // And the list has to end with a semicolon
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

        // They have have an optional else clause, in the form Else=value
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Else))
        {
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

            // Now we should get the text, a comma, and the value
            tokToUse.GetTextToken(strText);
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Comma);

            // And next should be the value
            TGenProtoMapItem* pmapiNew = 0;
            switch(eType)
            {
                case tGenProtoS::ETypes::Card1 :
                    mapNew.AddElseItem(TGenProtoMapItem(strText, tokToUse.c1GetToken()));
                    break;

                case tGenProtoS::ETypes::Card2 :
                    mapNew.AddElseItem(TGenProtoMapItem(strText, tokToUse.c2GetToken()));
                    break;

                case tGenProtoS::ETypes::Card4 :
                    mapNew.AddElseItem(TGenProtoMapItem(strText, tokToUse.c4GetToken()));
                    break;

                case tGenProtoS::ETypes::Int1 :
                    mapNew.AddElseItem(TGenProtoMapItem(strText, tokToUse.i1GetToken()));
                    break;

                case tGenProtoS::ETypes::Int2 :
                    mapNew.AddElseItem(TGenProtoMapItem(strText, tokToUse.i2GetToken()));
                    break;

                case tGenProtoS::ETypes::Int4 :
                    mapNew.AddElseItem(TGenProtoMapItem(strText, tokToUse.i4GetToken()));
                    break;

                case tGenProtoS::ETypes::Float4 :
                    mapNew.AddElseItem(TGenProtoMapItem(strText, tokToUse.f4GetToken()));
                    break;

                case tGenProtoS::ETypes::Float8 :
                    mapNew.AddElseItem(TGenProtoMapItem(strText, tokToUse.f8GetToken()));
                    break;

                default :
                    facGenProtoS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDErrs::errcParse_Expected
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(tokToUse.c4CurLine())
                        , TCardinal(tokToUse.c4CurCol())
                        , TString(L"data type")
                    );
                    break;
            };

            // And we have to have a semicolon to end the else clause
            tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
        }

        // And we should see an end map and semicolon
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EndMap);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
    }

    // And we have to see the terminating semicolon for the whole maps=
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
}



tCIDLib::TVoid TGenProtoSDriver::ParseMsgMatches(TGenProtoTokenizer& tokToUse)
{
    // And we should see the equal sign after the opening block name
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    // First must be the state machine block
    tokToUse.CheckNextToken(tGenProtoS::ETokens::StateMachine);
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    // Reset the state machine collection to get it ready for new stuff
    m_colStateMachine.RemoveAll();

    //
    //  To speed up checking for duplicate state names, we'll create a
    //  local hash set of strings. We'll add each state as we go.
    //
    tCIDLib::TStrHashSet colNames(29, TStringKeyOps());

    // And now process all of the state transition blocks.
    TString strText;
    TString strName;
    while (kCIDLib::True)
    {
        // Break out on the end state machine token
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndStateMachine))
            break;

        //
        //  We have to now see a state block start, an equal sign, and the
        //  name of the state.
        //
        tokToUse.CheckNextToken(tGenProtoS::ETokens::State);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        tokToUse.CheckNameToken(strText);

        // Make sure it's not a duplicate state name
        if (colNames.bHasElement(strText))
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_DupState
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strText
            );
        }

        // It's not so add it to the local set
        colNames.objAdd(strText);

        //
        //  Create a new state info object and put a janitor on it. We have
        //  to tell it it's index, which is the index into the list of states.
        //
        TGenProtoStateInfo* pstatiNew = new TGenProtoStateInfo
        (
            strText
            , m_colStateMachine.c4ElemCount()
        );
        TJanitor<TGenProtoStateInfo> janState(pstatiNew);

        //
        //  Now loop through all of the states. Each of them is the name of
        //  a state to transition to, an equal sign, then a
        //
        while (kCIDLib::True)
        {
            // Break out on the end state statement
            if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndState))
                break;

            // It's got to be a state transition
            tokToUse.CheckNameToken(strName);
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

            // And we need the expression followed by a semicolon
            TGenProtoExprNode* pnodeChild = pnodeParseExpression
            (
                tokToUse
                , strText
                , 0
                , 0
                , tGenProtoS::ESpecNodes::ReplyBuf
            );
            pnodeChild = pnodeOptimizeNode(pnodeChild);
            TJanitor<TGenProtoExprNode> janNode(pnodeChild);

            // If must be a boolean expression
            if (pnodeChild->eType() != tGenProtoS::ETypes::Boolean)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcExpr_MustBeBoolean
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::TypeMatch
                );
            }

            // And it must end with a semi colon
            tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

            // Ok, add a transition object to the current state
            pstatiNew->AddTransition(strName, janNode.pobjOrphan());

            //
            //  We can have an optional list of store operations, which lets
            //  them set variables when a state is entered.
            //
            if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Store))
            {
                // We need an equal sign
                tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

                while (kCIDLib::True)
                {
                    // Break out on the end store command token
                    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndStore))
                        break;

                    tokToUse.CheckNameToken(strName);
                    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

                    // The name has to be a variable
                    TGenProtoVarInfo* pvariTarget = pvariFindByName(strName);
                    if (!pvariTarget)
                    {
                        facGenProtoS().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kGPDErrs::errcParse_VarNotFound
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::NotFound
                            , TCardinal(tokToUse.c4CurLine())
                            , TCardinal(tokToUse.c4CurCol())
                            , strName
                        );
                    }

                    // Make sure that the variable is not a constant
                    if (pvariTarget->bIsConst())
                    {
                        facGenProtoS().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kGPDErrs::errcParse_CantSetConst
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::CantDo
                            , TCardinal(tokToUse.c4CurLine())
                            , TCardinal(tokToUse.c4CurCol())
                            , strName
                        );
                    }

                    //
                    //  Now its got to be an expression and its semicolon,
                    //  which we add to the reply node as a store op.
                    //
                    TGenProtoExprNode* pnodeChild = pnodeParseExpression
                    (
                      tokToUse
                      , strText
                      , 0
                      , 0
                      , tGenProtoS::ESpecNodes::Reply
                    );
                    pnodeChild = pnodeOptimizeNode(pnodeChild);

                    // And put this store op on the state info object
                    pstatiNew->AddStoreOp(pvariTarget, pnodeChild);

                    // And it has to end with a semicon
                    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
                }

                // And a semicolon after the end store
                tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
            }
        }

        // And we need a semicolon after this state block
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

        // Looks like a good state definition, so store it
        m_colStateMachine.Add(janState.pobjOrphan());
    }

    // We have to see the semicolon after the end of the state machine block
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

    // Add the magic accept state
    m_colStateMachine.Add
    (
        new TGenProtoStateInfo
        (
            kGenProtoS_::pszAcceptState
            , m_colStateMachine.c4ElemCount()
        )
    );

    //
    //  Since all of the state transitions will forward reference states that
    //  did not exist when they were parsed, we have to run back through the
    //  list and tell each state about the indexes of the other states, so
    //  that they can update the indexes in their state transitions.
    //
    const tCIDLib::TCard4 c4StateCount = m_colStateMachine.c4ElemCount();
    if (!c4StateCount)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcState_NoStates
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4StateCount; c4Index++)
    {
        TGenProtoStateInfo& statiCur = *m_colStateMachine[c4Index];

        for (tCIDLib::TCard4 c4Index2 = 0; c4Index2 < c4StateCount; c4Index2++)
        {
            TGenProtoStateInfo& statiCur2 = *m_colStateMachine[c4Index2];
            statiCur2.SetIndices(statiCur.strName(), c4Index);
        }
    }

// #define DUMP_STATEMACHINE
#if defined(DUMP_STATEMACHINE)
TOutConsole conDump;
for (tCIDLib::TCard4 c4Index = 0; c4Index < c4StateCount; c4Index++)
{
    TGenProtoStateInfo& statiCur = *m_colStateMachine[c4Index];
    conDump << L"State=" << statiCur.strName() << kCIDLib::NewLn;

    const tCIDLib::TCard4 c4TransCount = statiCur.c4TransCount();
    for (tCIDLib::TCard4 c4Index2 = 0; c4Index2 < c4TransCount; c4Index2++)
        conDump << L"   " << statiCur.strTransAt(c4Index2) << kCIDLib::NewLn;

    conDump.Flush();
}
#endif

    //
    //  And finally do a validation pass on the states. We do some basic
    //  checks that each state can potentially make some forward progress.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4StateCount; c4Index++)
    {
        TGenProtoStateInfo& statiCur = *m_colStateMachine[c4Index];
        statiCur.Validate();
    }

    //
    //  Store the index of the accept state. Its just the index of the last
    //  state (the magic one we added above), but having in a separate field
    //  makes it more convenient and flexible.
    //
    m_c4AcceptStateInd = c4StateCount - 1;

    //
    //  Next we have to have the reply matching section. Each block in it
    //  will look at the message and match it to a Reply.
    //
    tokToUse.CheckNextToken(tGenProtoS::ETokens::MsgMatches);
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    // Reset the string set which we'll use again to check for dups
    colNames.RemoveAll();

    while(kCIDLib::True)
    {
        // Break out on the end msg matches token
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndMsgMatches))
            break;

        //
        //  We have to now see a case token, an equal sign, and the name of
        //  the target reply.
        //
        tokToUse.CheckNextToken(tGenProtoS::ETokens::Case);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        tokToUse.CheckNameToken(strName);

        // Make sure it's not a duplicate reply block name
        if (colNames.bHasElement(strName))
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_DupReply
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strName
            );
        }

        // It's not so add it to the local set
        colNames.objAdd(strName);

        // Make sure that such a reply exists
        TGenProtoReply* prepTarget = prepFindByName(strName);
        if (!prepTarget)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_ReplyNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strName
            );
        }

        // And we need the expression followed by a semicolon
        TGenProtoExprNode* pnodeChild = pnodeParseExpression
        (
            tokToUse
            , strText
            , 0
            , 0
            , tGenProtoS::ESpecNodes::ReplyBuf
        );

        // If must be a boolean expression
        if (pnodeChild->eType() != tGenProtoS::ETypes::Boolean)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcExpr_MustBeBoolean
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::TypeMatch
            );
        }

        // Ok, let's add a new msg match. It adopts the expression
        m_colMatches.Add(new TGenProtoMsgMatch(prepTarget, pnodeChild));
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

        // And we need to see the end case and semicolon
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EndCase);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
    }

    // We need a semicolon at the end of the msg matches section
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

    // And we have to see the the close of the msgs matching and the semicolon
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EndMsgMatching);
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
}


tCIDLib::TVoid TGenProtoSDriver::ParsePollEvents(TGenProtoTokenizer& tokToUse)
{
    // The caller checked the opening keyword, which is followed by an equal
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    //
    //  We will get some number of Exchange= blocks, each one of which
    //  indicates an exchange to do during polling.
    //
    while (kCIDLib::True)
    {
        // Break out on the end reconnect token
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndPollEvents))
            break;

        tokToUse.CheckNextToken(tGenProtoS::ETokens::Exchange);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

        TString strSend;
        tokToUse.CheckNextToken(tGenProtoS::ETokens::Send);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        tokToUse.CheckNameToken(strSend);
        TGenProtoQuery* pqryPoll = m_colQueries.pobjFindByKey(strSend);
        if (!pqryPoll)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_QueryNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strSend
            );
        }
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

        //
        //  Next we get the poll period, which indicates how often (in ms)
        //  we send this poll query.
        //
        tokToUse.CheckNextToken(tGenProtoS::ETokens::Period);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        const tCIDLib::TCard4 c4Period = tokToUse.c4GetToken();
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

        //
        //  In the case of polling, the reply is optional, since they may
        //  not want to wait for it, and just let the polling thread handle
        //  what comes back.
        //
        TGenProtoReply* prepPoll = 0;
        TString strReceive;
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Receive))
        {
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
            tokToUse.CheckNameToken(strReceive);
            prepPoll = prepFindByName(strReceive);
            if (!prepPoll)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_ReplyNotFound
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , TCardinal(tokToUse.c4CurLine())
                    , TCardinal(tokToUse.c4CurCol())
                    , strReceive
                );
            }
            tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
        }

        //
        //  Next we get the wait period. Its going to be ignored if there
        //  is no reply defined, so we make it optional.
        //
        tCIDLib::TCard4 c4Wait = 0;
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::WaitFor))
        {
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
            c4Wait = tokToUse.c4GetToken();
            tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
        }

        // Add a new call/response object to our poll collection
        m_colPollEvents.objAdd(TGenProtoCallRep(pqryPoll, prepPoll, c4Wait, c4Period));

        tokToUse.CheckNextToken(tGenProtoS::ETokens::EndExchange);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
    }

    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
}


tCIDLib::TVoid TGenProtoSDriver::ParseProtocolInfo(TGenProtoTokenizer& tokToUse)
{
    // The caller checked the opening keyword, which is followed by an equal
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    //
    //  Set all our values for this block to defaults, to which they will
    //  remain set unless set in this block.
    //
    m_strEncoding = L"ASCII";
    m_c4MinSendInterval = 0;
    m_c4TimeoutMax = 2;
    m_bDeadMode = kCIDLib::False;
    m_eProtoType = tGenProtoS::EProtoTypes::TwoWay;

    TString strValue;
    while (kCIDLib::True)
    {
        // Get the next token
        const tGenProtoS::ETokens eTok = tokToUse.eGetNextToken(strValue);

        // Break out on the end of section token
        if (eTok == tGenProtoS::ETokens::EndProtocolInfo)
            break;

        // Else it has to be one of the fields valid for this block
        if (eTok == tGenProtoS::ETokens::TextEncoding)
        {
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
            tokToUse.GetTextToken(m_strEncoding);
        }
         else if (eTok == tGenProtoS::ETokens::MinSendInterval)
        {
            //
            //  This sets the minimum interval, in milliseconds, that have to
            //  expire between sends to the device. If not provided, it's
            //  zero, so that no throttling is done.
            //
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
            m_c4MinSendInterval = tokToUse.c4GetToken();

            // If it's more than a maximum, we can't support it
            if (m_c4MinSendInterval > kGenProtoS::c4MaxMinSendInterval)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_BadMinSendInt
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Range
                    , TCardinal(tokToUse.c4CurLine())
                    , TCardinal(tokToUse.c4CurCol())
                    , TCardinal(m_c4MinSendInterval)
                    , TCardinal(kGenProtoS::c4MaxMinSendInterval)
                );
            }
        }
         else if (eTok == tGenProtoS::ETokens::MaxTimeout)
        {
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
            m_c4TimeoutMax = tokToUse.c4GetToken();

            // Sanity check the value
            if (!m_c4TimeoutMax
            ||  (m_c4TimeoutMax > kGenProtoS::c4MaxMaxTimeouts))
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_BadMaxTimeout
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Range
                    , TCardinal(tokToUse.c4CurLine())
                    , TCardinal(tokToUse.c4CurCol())
                    , TCardinal(kGenProtoS::c4MaxMaxTimeouts)
                );
            }
        }
         else if (eTok == tGenProtoS::ETokens::ProtocolType)
        {
            //
            //  We have to see an equal sign and then either OneWay or
            //  TwoWay.
            //
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
            tokToUse.GetTextToken(strValue);

            if (strValue == L"OneWay")
                m_eProtoType = tGenProtoS::EProtoTypes::OneWay;
            else if (strValue == L"TwoWay")
                m_eProtoType = tGenProtoS::EProtoTypes::TwoWay;
            else if (strValue == L"DeadIfOff")
                m_eProtoType = tGenProtoS::EProtoTypes::DeadIfOff;
            else
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_ExpectedProtoType
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(tokToUse.c4CurLine())
                    , TCardinal(tokToUse.c4CurCol())
                );
            }
        }
         else
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_BadProtoInfoFld
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strValue
            );
        }

        // Each one needs to end in a semicolon
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
    }

    // We have to see a semicolon at the end of the block
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

    // Update any pre-fab contexts with this info
    m_ctxQuery.ChangeEncoding(m_strEncoding);
    m_ctxReply.ChangeEncoding(m_strEncoding);
    m_ctxWriteCmd.ChangeEncoding(m_strEncoding);
}


tCIDLib::TVoid TGenProtoSDriver::ParseQueries(TGenProtoTokenizer& tokToUse)
{
    // The caller checked the opening keyword, which is followed by an equal
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    //
    //  Look until we see the end queries token. Until then we must see
    //  query blocks, which we parse out and store in the query list.
    //
    TString     strText;
    while (kCIDLib::True)
    {
        // Break out on the end of section token
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndQueries))
            break;

        //
        //  Else its got to be a query block. So we'll get the query block
        //  token, an equal sign, and the name of the query.
        //
        tokToUse.CheckNextToken(tGenProtoS::ETokens::Query);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        tokToUse.CheckNameToken(strText);

        //
        //  Ok create and add temp query object so we can store the parsed
        //  info on it. Make sure we don't already have one with this name
        //
        if (m_colQueries.bKeyExists(strText))
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_DupQuery
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strText
            );
        }
        TGenProtoQuery& qryNew = m_colQueries.objAdd(TGenProtoQuery(strText));

        // And next we have to see a query command block and and equal sign
        tokToUse.CheckNextToken(tGenProtoS::ETokens::QueryCmd);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

        //
        //  Loop until we see the end of the query command block. Until we
        //  see it, we must be seeing expressions, each of which we add to
        //  the query object.
        //
        while (kCIDLib::True)
        {
            // Break out on the end query command token
            if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndQueryCmd))
                break;

            // Ok its got to be an expression and its semicolon
            TGenProtoExprNode* pnodeChild = pnodeParseExpression
            (
                tokToUse
                , strText
                , 0
                , 0
                , tGenProtoS::ESpecNodes::Query
            );
            pnodeChild = pnodeOptimizeNode(pnodeChild);
            qryNew.AddQueryExpr(pnodeChild);
            tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
        }

        //
        //  And we have to see the semicolon at the end of the query command
        //  block, then the end of this query block, and its semicolon.
        //
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EndQuery);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
    }

    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
}


tCIDLib::TVoid TGenProtoSDriver::ParseReplies(TGenProtoTokenizer& tokToUse)
{
    // The caller checked the opening keyword, which is followed by an equal
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    //
    //  Look until we see the end replies token. Until then we must see
    //  reply blocks, which we parse out and store in the reply list.
    //
    TString strText;
    TString strName;
    while (kCIDLib::True)
    {
        // Break out on the end of section token
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndReplies))
            break;

        //
        //  Else its got to be a reply block. So we'll get the reply block
        //  token, an equal sign, and the name of the reply.
        //
        tokToUse.CheckNextToken(tGenProtoS::ETokens::Reply);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        tokToUse.CheckNameToken(strText);

        //
        //  Ok create and add a new reply object so we can store the parsed
        //  info on it. Make sure we don't already have one with this name.
        //
        if (m_colReplies.bKeyExists(strText))
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_DupReply
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strText
            );
        }
        TGenProtoReply& repNew = m_colReplies.objAdd(TGenProtoReply(strText));

        // Next is optionally the Validate= block, but it's not required
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Validate))
        {
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

            while (kCIDLib::True)
            {
                // Break out on the end query command token
                if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndValidate))
                    break;

                // Ok its got to be an expression and its semicolon
                TGenProtoExprNode* pnodeChild = pnodeParseExpression
                (
                    tokToUse
                    , strText
                    , 0
                    , 0
                    , tGenProtoS::ESpecNodes::Reply
                );
                pnodeChild = pnodeOptimizeNode(pnodeChild);
                repNew.AddValidationExpr(pnodeChild);
                tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
            }

            // And we have to see the semicolon at the end of the block
            tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
        }

        //
        //  Next we need the Store= block. But its optional, since some
        //  replies are just ack/nak type replies.
        //
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Store))
        {
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

            while (kCIDLib::True)
            {
                // Break out on the end store command token
                if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndStore))
                    break;

                //
                //  Has to be another store. So get the name followed by an
                //  equal sign. Check first though to see if it is a dynamic
                //  field, via a dererenced variable/const.
                //
                const tCIDLib::TBoolean bDeref = tokToUse.bIfPeeked
                (
                    tGenProtoS::ETokens::Asterisk
                );

                tokToUse.CheckNameToken(strName);
                tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

                // The name has to be a field or variable
                TGenProtoFldInfo* pfldiTarget = pfldiFindByName(strName);
                TGenProtoVarInfo* pvariTarget = 0;
                if (pfldiTarget)
                {
                    //
                    //  Make sure that they didn't try to derefence a field
                    //  directly.
                    //
                    if (bDeref)
                    {
                        facGenProtoS().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kGPDErrs::errcParse_DerefFld
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::NotFound
                            , TCardinal(tokToUse.c4CurLine())
                            , TCardinal(tokToUse.c4CurCol())
                        );
                    }
                }
                 else
                {
                    pvariTarget = pvariFindByName(strName);
                    if (!pvariTarget)
                    {
                        facGenProtoS().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kGPDErrs::errcParse_VarNotFound
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::NotFound
                            , TCardinal(tokToUse.c4CurLine())
                            , TCardinal(tokToUse.c4CurCol())
                            , strName
                        );
                    }

                    //
                    //  Make sure that the variable is not a constant if it
                    //  is not a dereference.
                    //
                    if (!bDeref && pvariTarget->bIsConst())
                    {
                        facGenProtoS().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kGPDErrs::errcParse_CantSetConst
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::CantDo
                            , TCardinal(tokToUse.c4CurLine())
                            , TCardinal(tokToUse.c4CurCol())
                            , strName
                        );
                    }
                }

                //
                //  Now its got to be an expression and its semicolon, which
                //  we add to the reply node as a store op.
                //
                TGenProtoExprNode* pnodeChild = pnodeParseExpression
                (
                  tokToUse
                  , strText
                  , 0
                  , 0
                  , tGenProtoS::ESpecNodes::Reply
                );
                pnodeChild = pnodeOptimizeNode(pnodeChild);

                // Store the appropriate storage op
                if (pfldiTarget)
                    repNew.AddStoreOp(pfldiTarget, pnodeChild);
                else
                    repNew.AddStoreOp(pvariTarget, pnodeChild, bDeref);

                tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
            }

            // The semicolon after the end store block
            tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
        }

        // And we have to see the end of this reply block and its semicolon
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EndReply);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
    }

    // And the semicolon after the EndReplies
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
}


tCIDLib::TVoid TGenProtoSDriver::ParseTryConnect(TGenProtoTokenizer& tokToUse)
{
    // The caller checked the opening keyword, which is followed by an equal
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    //
    //  We should see the send and receive statements, each of which is
    //  followed by an equal sign and a name token. For the send, the name
    //  refers to a Query and for the receive it refers to a Reply.
    //
    TString strSend;
    tokToUse.CheckNextToken(tGenProtoS::ETokens::Send);
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
    tokToUse.CheckNameToken(strSend);
    TGenProtoQuery* pqryConn = m_colQueries.pobjFindByKey(strSend);
    if (!pqryConn)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_QueryNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(tokToUse.c4CurLine())
            , TCardinal(tokToUse.c4CurCol())
            , strSend
        );
    }
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

    TString strReceive;
    tokToUse.CheckNextToken(tGenProtoS::ETokens::Receive);
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
    tokToUse.CheckNameToken(strReceive);
    TGenProtoReply* prepConn = prepFindByName(strReceive);
    if (!prepConn)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_ReplyNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(tokToUse.c4CurLine())
            , TCardinal(tokToUse.c4CurCol())
            , strReceive
        );
    }
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

    // And next should be the wait period
    tokToUse.CheckNextToken(tGenProtoS::ETokens::WaitFor);
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
    const tCIDLib::TCard4 c4Wait = tokToUse.c4GetToken();
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

    // Set our try connect call/response object with this information
    m_clrpTryConnect.Set(pqryConn, prepConn, c4Wait);

    //
    //  They can optionally set a post-connect pause period, so check for
    //  that.
    //
    if (tokToUse.bIfPeeked(tGenProtoS::ETokens::PauseAfter))
    {
        // We need an equal, a numeric, and a semicolon
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        m_c4PostConnWait = tokToUse.c4GetToken();

        // Clip it if necessary
        if (m_c4PostConnWait > 4000)
            m_c4PostConnWait = 4000;

        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
    }

    tokToUse.CheckNextToken(tGenProtoS::ETokens::EndTryConnect);
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
}


tCIDLib::TVoid TGenProtoSDriver::ParseReconnect(TGenProtoTokenizer& tokToUse)
{
    // The caller checked the opening keyword, which is followed by an equal
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    //
    //  We will get some number of Exchange= blocks, each one of which
    //  indicates an exchange to do right after a connect.
    //
    while (kCIDLib::True)
    {
        // Break out on the end reconnect token
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndReconnect))
            break;

        tokToUse.CheckNextToken(tGenProtoS::ETokens::Exchange);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

        TString strSend;
        tokToUse.CheckNextToken(tGenProtoS::ETokens::Send);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        tokToUse.CheckNameToken(strSend);
        TGenProtoQuery* pqryReconn = m_colQueries.pobjFindByKey(strSend);
        if (!pqryReconn)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_QueryNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strSend
            );
        }
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);


        TString strReceive;
        tokToUse.CheckNextToken(tGenProtoS::ETokens::Receive);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        tokToUse.CheckNameToken(strReceive);
        TGenProtoReply* prepReconn = prepFindByName(strReceive);
        if (!prepReconn)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_ReplyNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strReceive
            );
        }
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

        // And last should be the wait period
        tokToUse.CheckNextToken(tGenProtoS::ETokens::WaitFor);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        const tCIDLib::TCard4 c4Wait = tokToUse.c4GetToken();
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

        // Add a new call/response object to our reconnect collection.
        m_colReconnect.objAdd(TGenProtoCallRep(pqryReconn, prepReconn, c4Wait));

        tokToUse.CheckNextToken(tGenProtoS::ETokens::EndExchange);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
    }

    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
}


//
//  This really parses variables and constants. It's just a passed flag that
//  tells us which we are doing.
//
tCIDLib::TVoid
TGenProtoSDriver::ParseVariables(       TGenProtoTokenizer& tokToUse
                                , const tCIDLib::TBoolean   bConst)
{
    // The caller checked the opening keyword, which is followed by an equal
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    //
    //  Look until we see the end variables token. Until then we must see
    //  variable def blocks, which we parse out and create new variables
    //  objects for.
    //
    TString     strText;
    TString     strName;
    while (kCIDLib::True)
    {
        // Break out on the end of section token
        if (bConst)
        {
            if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndConstants))
                break;
        }
         else
        {
            if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndVariables))
                break;
        }

        //
        //  Else its got to be a variable def block. So we'll get the variable
        //  token, an equal sign, and the name of the variable.
        //
        if (bConst)
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Constant);
        else
            tokToUse.CheckNextToken(tGenProtoS::ETokens::Variable);

        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        tokToUse.CheckNameToken(strName);

        // Make sure a variable with this name doesn't already exist
        TGenProtoVarInfo* pvariNew = pvariFindByName(strName);

        if (pvariNew)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_VarExists
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strName
            );
        }

        // Next must be the type
        const tGenProtoS::ETypes eType = tokToUse.eParseExprTypeLine();

        //
        //  Could be the value next. If this is a constant, then it must
        //  be a value.
        //
        TGenProtoExprVal evalInit(eType);
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Value))
        {
            //
            //  Get the equal sign and then the next token, which we will
            //  get based on the type.
            //
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

            switch(eType)
            {
                case tGenProtoS::ETypes::Boolean :
                    evalInit.bValue(tokToUse.bGetBoolToken());
                    break;

                case tGenProtoS::ETypes::Int1 :
                    evalInit.i1Value(tokToUse.i1GetToken());
                    break;

                case tGenProtoS::ETypes::Card1 :
                    evalInit.c1Value(tokToUse.c1GetToken());
                    break;

                case tGenProtoS::ETypes::Int2 :
                    evalInit.i2Value(tokToUse.i2GetToken());
                    break;

                case tGenProtoS::ETypes::Card2 :
                    evalInit.c2Value(tokToUse.c2GetToken());
                    break;

                case tGenProtoS::ETypes::Int4 :
                    evalInit.i4Value(tokToUse.i4GetToken());
                    break;

                case tGenProtoS::ETypes::Card4 :
                    evalInit.c4Value(tokToUse.c4GetToken());
                    break;

                case tGenProtoS::ETypes::Float4 :
                    evalInit.f4Value(tokToUse.f4GetToken());
                    break;

                case tGenProtoS::ETypes::Float8 :
                    evalInit.f8Value(tokToUse.f8GetToken());
                    break;

                case tGenProtoS::ETypes::String :
                    tokToUse.GetTextToken(strText);
                    evalInit.strValue(strText);
                    break;

                default :
                    facGenProtoS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDErrs::errcParse_UnknownVarType
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Internal
                        , TCardinal(tokToUse.c4CurLine())
                        , TCardinal(tokToUse.c4CurCol())
                        , TCardinal(tCIDLib::c4EnumOrd(eType))
                    );
                    break;
            };

            tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
        }
         else if (bConst)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_MustInitConst
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::InitFailed
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
            );
        }

        // And we should see the end of the const or variable block
        if (bConst)
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EndConstant);
        else
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EndVariable);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

        // Add a new variable to the list
        TGenProtoVarInfo& variNew = m_colVariables.objAdd
        (
            TGenProtoVarInfo(strName, eType, bConst, evalInit)
        );
    }

    // And the section must end with a semicolon
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
}


tCIDLib::TVoid TGenProtoSDriver::ParseWriteCmds(TGenProtoTokenizer& tokToUse)
{
    // The caller checked the opening keyword, which is followed by an equal
    tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

    //
    //  Look until we see the end write cmds token. Until then we must see
    //  write command blocks, which we parse out and then set on their
    //  respective field info objects, which have already been parsed.
    //
    TString     strText;
    while (kCIDLib::True)
    {
        // Break out on the end of section token
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndWriteCmds))
            break;

        //
        //  Else its got to be a write command block. So we'll get the write
        //  cmd token, an equal sign, and the name of the field this write
        //  field is for.
        //
        tokToUse.CheckNextToken(tGenProtoS::ETokens::WriteCmd);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
        tokToUse.CheckNameToken(strText);

        // Look up the field with this name. Throw if not found
        TGenProtoFldInfo* pfldiRef = pfldiFindByName(strText);
        if (!pfldiRef)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_FieldNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strText
            );
        }

        // We found it, so make sure its writeable. If not, this makes no sense
        if (!pfldiRef->bIsWriteable())
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_FldNotWriteable
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strText
            );
        }

        // Also make sure we've not already done a write command for this one
        if (pfldiRef->bHasWriteCmds())
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_TwoWriteCmds
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , TCardinal(tokToUse.c4CurLine())
                , TCardinal(tokToUse.c4CurCol())
                , strText
            );
        }

        //
        //  Looks ok, so lets process the Send= block. For each expression
        //  we find, we'll add it to the field.
        //
        tokToUse.CheckNextToken(tGenProtoS::ETokens::Send);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);

        TGenProtoExprNode* pnodeNew;
        while (kCIDLib::True)
        {
            //
            //  We'll now see a list of expressions that build up the write
            //  command.
            //
            //  We break out when we see the EndWriteCmd token, so we have
            //  to peek ahead to see if that's what it is. This method will
            //  peek and if its the indicated token it will eat it and
            //  return true, else it will return false and push it back.
            //
            if (tokToUse.bIfPeeked(tGenProtoS::ETokens::EndSend))
                break;
            pnodeNew = pnodeParseExpression
            (
                tokToUse
                , strText
                , 0
                , pfldiRef
                , tGenProtoS::ESpecNodes::WriteCmd
            );

            // And add this is the next write command expression
            pnodeNew = pnodeOptimizeNode(pnodeNew);
            pfldiRef->AddWriteCmdItem(pnodeNew);

            // It was an expression, so we need a semicolon
            tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
        }

        // And a semicolon after EndSend
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

        //
        //  If the write command sends something back, they can indicate that
        //  a set of ack/nak messages that should be checked for. If they do,
        //  then they also must indicate the 'wait for' time.
        //
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::AckNak))
        {
            TString strText2;

            // The Ack has to be there
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
            tokToUse.CheckNameToken(strText);
            TGenProtoReply* prepAck = prepFindByName(strText);
            if (!prepAck)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_ReplyNotFound
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , TCardinal(tokToUse.c4CurLine())
                    , TCardinal(tokToUse.c4CurCol())
                    , strText
                );
            }

            // The nak is optional
            TGenProtoReply* prepNak = 0;
            if (tokToUse.bIfPeeked(tGenProtoS::ETokens::Comma))
            {
                tokToUse.CheckNameToken(strText2);
                prepNak = prepFindByName(strText2);
                if (!prepNak)
                {
                    facGenProtoS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDErrs::errcParse_ReplyNotFound
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotFound
                        , TCardinal(tokToUse.c4CurLine())
                        , TCardinal(tokToUse.c4CurCol())
                        , strText2
                    );
                }
            }
            tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

            // And they must provide the wait time
            const tCIDLib::TCard4 c4Wait = tokToUse.c4ParseCardField
            (
                tGenProtoS::ETokens::WaitFor
            );

            // Ok, add this info to the field info
            pfldiRef->AddAckInfo(prepAck, prepNak, c4Wait);
        }

        // And there can be an optional min send interval
        if (tokToUse.bIfPeeked(tGenProtoS::ETokens::MinSendInterval))
        {
            tokToUse.CheckNextToken(tGenProtoS::ETokens::EqualSign);
            const tCIDLib::TCard4 c4Interval = tokToUse.c4GetToken();
            tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);

            // If it's more than a maximum, we can't support it
            if (c4Interval > kGenProtoS::c4MaxMinSendInterval)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_BadMinSendInt
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Range
                    , TCardinal(tokToUse.c4CurLine())
                    , TCardinal(tokToUse.c4CurCol())
                    , TCardinal(c4Interval)
                    , TCardinal(kGenProtoS::c4MaxMinSendInterval)
                );
            }

            // Set it on the field info object
            pfldiRef->c4MinSendInterval(c4Interval);
        }

        // And now we need the end of this block
        tokToUse.CheckNextToken(tGenProtoS::ETokens::EndWriteCmd);
        tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
    }

    // And the section must end with a semicolon
    tokToUse.CheckNextToken(tGenProtoS::ETokens::SemiColon);
}



