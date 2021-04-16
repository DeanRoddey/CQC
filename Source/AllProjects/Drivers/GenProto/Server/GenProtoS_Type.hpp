//
// FILE NAME: GenProtoS_Type.hpp
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
//  This is the types header for the generic protocol server driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tGenProtoS
{
    // -----------------------------------------------------------------------
    //  We have an expression that calculates checksums, It will need over
    //  time to support multiple checksum algorithms. This allows us to support
    //  multiple schemes via the same expression.
    //
    //  1. AddWithOverflow    - Basic addition of each item into an accum
    //                          with overflow.
    //  2. Xor1               - Put first item into accum, then XOR each
    //                          successive one into the accum.
    // -----------------------------------------------------------------------
    enum class ECheckSums
    {
        AddWithOverflow
        , Xor1
    };


    // -----------------------------------------------------------------------
    //  Used when running under the test harness. This indicates what types
    //  of messages they want us to send them.
    // -----------------------------------------------------------------------
    enum class EDbgMsgs : tCIDLib::TCard2
    {
        None                = 0x0000
        , Connect           = 0x0001
        , CommRes           = 0x0002
        , PollEvents        = 0x0004
        , StateMachine      = 0x0008
        , FldWrite          = 0x0010
        , SendBytes         = 0x0020
        , ReceiveBytes      = 0x0040
        , MsgMatch          = 0x0080

        , Default           = 0x0013
    };


    // -----------------------------------------------------------------------
    //  The message delimiter formats we can handle. We have a set of
    //  fundamental attribute types, and then a common set of combinations
    //  made up from them. Being bitmapped, it'll make it easier for us to
    //  drive both the data entry interface and the message recognition
    //  algorithms.
    // -----------------------------------------------------------------------
    enum class EMsgDelims : tCIDLib::TCard4
    {
        None              = 0x0000
        , Start           = 0x0001
        , Term            = 0x0002
        , FixedLen        = 0x0100
        , EmbeddedLen     = 0x0200

        , StartTerm       = 0x0003
        , StartFixed      = 0x0101
        , StartEmbedded   = 0x0201
        , StartTermFixed  = 0x0103
        , StartTermEmb    = 0x0203
    };


    // -----------------------------------------------------------------------
    //  The protocol formats we support
    // -----------------------------------------------------------------------
    enum class EProtoFmts
    {
        Binary
        , ASCII

        , Count
        , Min           = Binary
        , Max           = ASCII
    };


    // -----------------------------------------------------------------------
    //  The protocol types we support
    // -----------------------------------------------------------------------
    enum class EProtoTypes
    {
        OneWay
        , DeadIfOff
        , TwoWay

        , Count
        , Min           = OneWay
        , Max           = TwoWay
    };


    // -----------------------------------------------------------------------
    //  Some types of expressions in the protocol can use expression nodes
    //  that others cannot. So we set up a bit mask for those node types that
    //  are treated specially, then create combo values for specific uses of
    //  expressions.
    // -----------------------------------------------------------------------
    enum class ESpecNodes : tCIDLib::TCard2
    {
        None                = 0x0000
        , ReplyBuf          = 0x0001
        , SendBuf           = 0x0002
        , WriteVal          = 0x0004
        , FldVal            = 0x0008
        , InputByte         = 0x0010

        //
        //  These represent the specific types of expressions. Note that the
        //  'reply' value is used for the matching, validation, and storing
        //  steps, all of which just operate on the reply buffer.
        //
        , SendReplyBuf      = 0x0003
        , Reply             = 0x0009
        , WriteCmd          = 0x000E
        , Query             = 0x000A
        , StateMachine      = 0x0011
    };


    // -----------------------------------------------------------------------
    //  The tokens that our parsing tokenizer returns. Its really just for
    //  internal use, but it has to be visible in a couple of class interfaces
    //  that are publically visible.
    //
    //  NOTE: Keep this in sync with the mapping array inside the tokenizer!
    // -----------------------------------------------------------------------
    enum class ETokens
    {
        Access
        , AckNak
        , Add
        , ANDBits
        , Author
        , Baud
        , Bits
        , BoolSel
        , Boolean
        , Card1
        , Card2
        , Card4
        , Case
        , CatStr
        , CheckSum
        , CommCfg
        , Constant
        , Constants
        , CRC16
        , Description
        , Div
        , DTR
        , Else
        , EMail
        , Encoding
        , EndCase
        , EndCommCfg
        , EndConstant
        , EndConstants
        , EndExchange
        , EndField
        , EndFields
        , EndGenInfo
        , EndIf
        , EndItem
        , EndItems
        , EndMap
        , EndMaps
        , EndMsgMatches
        , EndMsgMatching
        , EndPollEvents
        , EndProtocolInfo
        , EndQueries
        , EndQuery
        , EndQueryCmd
        , EndReconnect
        , EndReplies
        , EndReply
        , EndSend
        , EndState
        , EndStateMachine
        , EndStore
        , EndTryConnect
        , EndType
        , EndValidate
        , EndVariable
        , EndVariables
        , EndWriteCmd
        , EndWriteCmds
        , Equals
        , Exchange
        , Extract
        , ExtractASCIICard
        , ExtractASCIIFloat
        , ExtractASCIIInt
        , ExtractStr
        , ExtractToken
        , False
        , File
        , Field
        , FieldDeref
        , Fields
        , Fixed
        , Flags
        , Float4
        , Float8
        , SemType
        , FunctionCall
        , GenInfo
        , GreaterThan
        , IfAll
        , IfAny
        , IfModel
        , IfNModel
        , IsASCIIAlpha
        , IsASCIIAlphaNum
        , IsASCIIDecDigit
        , IsASCIIHexDigit
        , IsASCIIPunct
        , IsASCIISpace
        , Int1
        , Int2
        , Int4
        , Item
        , Items
        , LessThan
        , Limits
        , LimitFromMap
        , Make
        , Map
        , Maps
        , MapFrom
        , MapTo
        , MaxTimeout
        , MinSendInterval
        , Model
        , ModelList
        , MsgMatches
        , MsgMatching
        , Mul
        , Not
        , ORBits
        , Parity
        , PauseAfter
        , Period
        , PollEvents
        , ProtocolInfo
        , ProtocolType
        , CQCProto
        , Queries
        , Query
        , QueryCmd
        , QuotedString
        , RangeRotate
        , Read
        , ReadWrite
        , Receive
        , Reconnect
        , Replies
        , Reply
        , ReviveField
        , RoundFloat
        , RTS
        , ScaleRange
        , Send
        , ShiftLeft
        , ShiftRight
        , State
        , StateMachine
        , Store
        , StopBits
        , String
        , Sub
        , TermChar
        , TextEncoding
        , ToBool
        , ToCard1
        , ToCard2
        , ToCard4
        , ToFloat4
        , ToFloat8
        , ToInt1
        , ToInt2
        , ToInt4
        , ToLower
        , ToString
        , ToUpper
        , TranscFunc
        , True
        , TryConnect
        , Type
        , Validate
        , Value
        , Variable
        , Variables
        , Version
        , WaitFor
        , WebSite
        , Write
        , WriteAlways
        , WriteCmd
        , WriteCmds
        , XORBits

        // Thease are special case single character tokens
        , Asterisk
        , Comma
        , EqualSign
        , CloseBracket
        , CloseParen
        , OpenBracket
        , OpenParen
        , SemiColon

        // These are special tokens that have no fixed representation
        , EOF
        , FieldRef
        , InputByte
        , NoMatch
        , None
        , NumericConst
        , ReplyBuf
        , ReplyLen
        , WriteCmdBuf
        , WriteCmdLen
        , WriteVal

        , Count
        , Min               = 0
        , Max               = WriteVal
        , NormalCount       = ETokens(XORBits + 1)
    };


    // -----------------------------------------------------------------------
    //  A set of transcendental math functions supported by one of the math
    //  expression types.
    // -----------------------------------------------------------------------
    enum class ETransTypes
    {
        ArcCos
        , ArcSine
        , ArcTangent
        , Cosine
        , Log
        , Log10
        , Sine
        , Tangent

        , Count
    };


    // -----------------------------------------------------------------------
    //  The types that a protocol language expression can have.
    //
    //  NOTE:   The numeric types are carefully arrange to allow for a fast
    //          check of relative magnitude!
    // -----------------------------------------------------------------------
    enum class ETypes
    {
        Boolean
        , Int1
        , Card1
        , Int2
        , Card2
        , Int4
        , Card4
        , Float4
        , Float8
        , String

        , Count
        , Min       = Boolean
        , Max       = String
    };


    // ---------------------------------------------------------------------------
    //  Provide some enum magic macros as required
    // ---------------------------------------------------------------------------
    EnumBinStreamMacros(tGenProtoS::EMsgDelims)
    EnumBinStreamMacros(tGenProtoS::EProtoFmts)

    StdEnumTricks(tGenProtoS::ETokens)
}

