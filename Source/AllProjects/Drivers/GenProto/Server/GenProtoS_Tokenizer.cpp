//
// FILE NAME: GenProtoS_Tokenizer.cpp
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
//  This file implements the syntax tokenizer code
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
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGenProtoTokenizer,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace GenProtoS_Tokenizer
{
    // Used to do the one time calc of the hashes of the lookup table
    TAtomicFlag                 atomInitDone;

    // The modulus that we use for hashing the strings
    tCIDLib::TCard4             c4Modulus = 19;

    // We have an array of these items to do token matching
    struct TMapItem
    {
        const   tCIDLib::TCh*   pszName;
        tCIDLib::THashVal       hshName;
        tGenProtoS::ETokens     eToken;
    };

    //
    //  This maps to the ETokens enum value. We look up the string in here and,
    //  if found, cast that index to the enum value.
    //
    //  NOTE:   Keep this in sync with the enum of course!
    //
    TMapItem aitemMapVals[] =
    {
          { L"Access"           , 0 , tGenProtoS::ETokens::Access           }
        , { L"AckNak"           , 0 , tGenProtoS::ETokens::AckNak           }
        , { L"Add"              , 0 , tGenProtoS::ETokens::Add              }
        , { L"ANDBits"          , 0 , tGenProtoS::ETokens::ANDBits          }
        , { L"Author"           , 0 , tGenProtoS::ETokens::Author           }
        , { L"Baud"             , 0 , tGenProtoS::ETokens::Baud             }
        , { L"BoolSel"          , 0 , tGenProtoS::ETokens::BoolSel          }
        , { L"Bits"             , 0 , tGenProtoS::ETokens::Bits             }
        , { L"Boolean"          , 0 , tGenProtoS::ETokens::Boolean          }
        , { L"Card1"            , 0 , tGenProtoS::ETokens::Card1            }
        , { L"Card2"            , 0 , tGenProtoS::ETokens::Card2            }
        , { L"Card4"            , 0 , tGenProtoS::ETokens::Card4            }
        , { L"Case"             , 0 , tGenProtoS::ETokens::Case             }
        , { L"CatStr"           , 0 , tGenProtoS::ETokens::CatStr           }
        , { L"CheckSum"         , 0 , tGenProtoS::ETokens::CheckSum         }
        , { L"CommCfg"          , 0 , tGenProtoS::ETokens::CommCfg          }
        , { L"Constant"         , 0 , tGenProtoS::ETokens::Constant         }
        , { L"Constants"        , 0 , tGenProtoS::ETokens::Constants        }
        , { L"CRC16"            , 0 , tGenProtoS::ETokens::CRC16            }
        , { L"Description"      , 0 , tGenProtoS::ETokens::Description      }
        , { L"Div"              , 0 , tGenProtoS::ETokens::Div              }
        , { L"DTR"              , 0 , tGenProtoS::ETokens::DTR              }
        , { L"Else"             , 0 , tGenProtoS::ETokens::Else             }
        , { L"EMail"            , 0 , tGenProtoS::ETokens::EMail            }
        , { L"Encoding"         , 0 , tGenProtoS::ETokens::Encoding         }
        , { L"EndCase"          , 0 , tGenProtoS::ETokens::EndCase          }
        , { L"EndCommCfg"       , 0 , tGenProtoS::ETokens::EndCommCfg       }
        , { L"EndConstant"      , 0 , tGenProtoS::ETokens::EndConstant      }
        , { L"EndConstants"     , 0 , tGenProtoS::ETokens::EndConstants     }
        , { L"EndExchange"      , 0 , tGenProtoS::ETokens::EndExchange      }
        , { L"EndField"         , 0 , tGenProtoS::ETokens::EndField         }
        , { L"EndFields"        , 0 , tGenProtoS::ETokens::EndFields        }
        , { L"EndGeneralInfo"   , 0 , tGenProtoS::ETokens::EndGenInfo       }
        , { L"#EndIf"           , 0 , tGenProtoS::ETokens::EndIf            }
        , { L"EndItem"          , 0 , tGenProtoS::ETokens::EndItem          }
        , { L"EndItems"         , 0 , tGenProtoS::ETokens::EndItems         }
        , { L"EndMap"           , 0 , tGenProtoS::ETokens::EndMap           }
        , { L"EndMaps"          , 0 , tGenProtoS::ETokens::EndMaps          }
        , { L"EndMsgMatches"    , 0 , tGenProtoS::ETokens::EndMsgMatches    }
        , { L"EndMsgMatching"   , 0 , tGenProtoS::ETokens::EndMsgMatching   }
        , { L"EndPollEvents"    , 0 , tGenProtoS::ETokens::EndPollEvents    }
        , { L"EndProtocolInfo"  , 0 , tGenProtoS::ETokens::EndProtocolInfo  }
        , { L"EndQueries"       , 0 , tGenProtoS::ETokens::EndQueries       }
        , { L"EndQuery"         , 0 , tGenProtoS::ETokens::EndQuery         }
        , { L"EndQueryCmd"      , 0 , tGenProtoS::ETokens::EndQueryCmd      }
        , { L"EndReconnect"     , 0 , tGenProtoS::ETokens::EndReconnect     }
        , { L"EndReplies"       , 0 , tGenProtoS::ETokens::EndReplies       }
        , { L"EndReply"         , 0 , tGenProtoS::ETokens::EndReply         }
        , { L"EndSend"          , 0 , tGenProtoS::ETokens::EndSend          }
        , { L"EndState"         , 0 , tGenProtoS::ETokens::EndState         }
        , { L"EndStateMachine"  , 0 , tGenProtoS::ETokens::EndStateMachine  }
        , { L"EndStore"         , 0 , tGenProtoS::ETokens::EndStore         }
        , { L"EndTryConnect"    , 0 , tGenProtoS::ETokens::EndTryConnect    }
        , { L"EndType"          , 0 , tGenProtoS::ETokens::EndType          }
        , { L"EndValidate"      , 0 , tGenProtoS::ETokens::EndValidate      }
        , { L"EndVariable"      , 0 , tGenProtoS::ETokens::EndVariable      }
        , { L"EndVariables"     , 0 , tGenProtoS::ETokens::EndVariables     }
        , { L"EndWriteCmd"      , 0 , tGenProtoS::ETokens::EndWriteCmd      }
        , { L"EndWriteCmds"     , 0 , tGenProtoS::ETokens::EndWriteCmds     }
        , { L"Equals"           , 0 , tGenProtoS::ETokens::Equals           }
        , { L"Exchange"         , 0 , tGenProtoS::ETokens::Exchange         }
        , { L"Extract"          , 0 , tGenProtoS::ETokens::Extract          }
        , { L"ExtractASCIICard" , 0 , tGenProtoS::ETokens::ExtractASCIICard }
        , { L"ExtractASCIIFloat", 0 , tGenProtoS::ETokens::ExtractASCIIFloat }
        , { L"ExtractASCIIInt"  , 0 , tGenProtoS::ETokens::ExtractASCIIInt  }
        , { L"ExtractStr"       , 0 , tGenProtoS::ETokens::ExtractStr       }
        , { L"ExtractToken"     , 0 , tGenProtoS::ETokens::ExtractToken     }
        , { L"False"            , 0 , tGenProtoS::ETokens::False            }
        , { L"File"             , 0 , tGenProtoS::ETokens::File             }
        , { L"Field"            , 0 , tGenProtoS::ETokens::Field            }
        , { L"FieldDeref"       , 0 , tGenProtoS::ETokens::FieldDeref       }
        , { L"Fields"           , 0 , tGenProtoS::ETokens::Fields           }
        , { L"Fixed"            , 0 , tGenProtoS::ETokens::Fixed            }
        , { L"Flags"            , 0 , tGenProtoS::ETokens::Flags            }
        , { L"Float4"           , 0 , tGenProtoS::ETokens::Float4           }
        , { L"Float8"           , 0 , tGenProtoS::ETokens::Float8           }
        , { L"SemType"          , 0 , tGenProtoS::ETokens::SemType          }
        , { L"FunctionCall"     , 0 , tGenProtoS::ETokens::FunctionCall     }
        , { L"GeneralInfo"      , 0 , tGenProtoS::ETokens::GenInfo          }
        , { L"GreaterThan"      , 0 , tGenProtoS::ETokens::GreaterThan      }
        , { L"IfAll"            , 0 , tGenProtoS::ETokens::IfAll            }
        , { L"IfAny"            , 0 , tGenProtoS::ETokens::IfAny            }
        , { L"#IfModel"         , 0 , tGenProtoS::ETokens::IfModel          }
        , { L"#IfNotModel"      , 0 , tGenProtoS::ETokens::IfNModel         }
        , { L"IsASCIIAlpha"     , 0 , tGenProtoS::ETokens::IsASCIIAlpha     }
        , { L"IsASCIIAlphaNum"  , 0 , tGenProtoS::ETokens::IsASCIIAlphaNum  }
        , { L"IsASCIIDecDigit"  , 0 , tGenProtoS::ETokens::IsASCIIDecDigit  }
        , { L"IsASCIIHexDigit"  , 0 , tGenProtoS::ETokens::IsASCIIHexDigit  }
        , { L"IsASCIIPunct"     , 0 , tGenProtoS::ETokens::IsASCIIPunct     }
        , { L"IsASCIISpace"     , 0 , tGenProtoS::ETokens::IsASCIISpace     }
        , { L"Int1"             , 0 , tGenProtoS::ETokens::Int1             }
        , { L"Int2"             , 0 , tGenProtoS::ETokens::Int2             }
        , { L"Int4"             , 0 , tGenProtoS::ETokens::Int4             }
        , { L"Item"             , 0 , tGenProtoS::ETokens::Item             }
        , { L"Items"            , 0 , tGenProtoS::ETokens::Items            }
        , { L"LessThan"         , 0 , tGenProtoS::ETokens::LessThan         }
        , { L"Limits"           , 0 , tGenProtoS::ETokens::Limits           }
        , { L"LimitFromMap"     , 0 , tGenProtoS::ETokens::LimitFromMap     }
        , { L"Make"             , 0 , tGenProtoS::ETokens::Make             }
        , { L"Map"              , 0 , tGenProtoS::ETokens::Map              }
        , { L"Maps"             , 0 , tGenProtoS::ETokens::Maps             }
        , { L"MapFrom"          , 0 , tGenProtoS::ETokens::MapFrom          }
        , { L"MapTo"            , 0 , tGenProtoS::ETokens::MapTo            }
        , { L"MaxTimeout"       , 0 , tGenProtoS::ETokens::MaxTimeout       }
        , { L"MinSendInterval"  , 0 , tGenProtoS::ETokens::MinSendInterval  }
        , { L"Model"            , 0 , tGenProtoS::ETokens::Model            }
        , { L"ModelList"        , 0 , tGenProtoS::ETokens::ModelList        }
        , { L"MsgMatches"       , 0 , tGenProtoS::ETokens::MsgMatches       }
        , { L"MsgMatching"      , 0 , tGenProtoS::ETokens::MsgMatching      }
        , { L"Mul"              , 0 , tGenProtoS::ETokens::Mul              }
        , { L"Not"              , 0 , tGenProtoS::ETokens::Not              }
        , { L"ORBits"           , 0 , tGenProtoS::ETokens::ORBits           }
        , { L"Parity"           , 0 , tGenProtoS::ETokens::Parity           }
        , { L"PauseAfter"       , 0 , tGenProtoS::ETokens::PauseAfter       }
        , { L"Period"           , 0 , tGenProtoS::ETokens::Period           }
        , { L"PollEvents"       , 0 , tGenProtoS::ETokens::PollEvents       }
        , { L"ProtocolInfo"     , 0 , tGenProtoS::ETokens::ProtocolInfo     }
        , { L"ProtocolType"     , 0 , tGenProtoS::ETokens::ProtocolType     }
        , { L"CQCProto"         , 0 , tGenProtoS::ETokens::CQCProto         }
        , { L"Queries"          , 0 , tGenProtoS::ETokens::Queries          }
        , { L"Query"            , 0 , tGenProtoS::ETokens::Query            }
        , { L"QueryCmd"         , 0 , tGenProtoS::ETokens::QueryCmd         }
        , { L"QuotedString"     , 0 , tGenProtoS::ETokens::QuotedString     }
        , { L"RangeRotate"      , 0 , tGenProtoS::ETokens::RangeRotate      }
        , { L"Read"             , 0 , tGenProtoS::ETokens::Read             }
        , { L"ReadWrite"        , 0 , tGenProtoS::ETokens::ReadWrite        }
        , { L"Receive"          , 0 , tGenProtoS::ETokens::Receive          }
        , { L"Reconnect"        , 0 , tGenProtoS::ETokens::Reconnect        }
        , { L"Replies"          , 0 , tGenProtoS::ETokens::Replies          }
        , { L"Reply"            , 0 , tGenProtoS::ETokens::Reply            }
        , { L"ReviveField"      , 0 , tGenProtoS::ETokens::ReviveField      }
        , { L"RoundFloat"       , 0 , tGenProtoS::ETokens::RoundFloat       }
        , { L"RTS"              , 0 , tGenProtoS::ETokens::RTS              }
        , { L"ScaleRange"       , 0 , tGenProtoS::ETokens::ScaleRange       }
        , { L"Send"             , 0 , tGenProtoS::ETokens::Send             }
        , { L"ShiftLeft"        , 0 , tGenProtoS::ETokens::ShiftLeft        }
        , { L"ShiftRight"       , 0 , tGenProtoS::ETokens::ShiftRight       }
        , { L"State"            , 0 , tGenProtoS::ETokens::State            }
        , { L"StateMachine"     , 0 , tGenProtoS::ETokens::StateMachine     }
        , { L"Store"            , 0 , tGenProtoS::ETokens::Store            }
        , { L"StopBits"         , 0 , tGenProtoS::ETokens::StopBits         }
        , { L"String"           , 0 , tGenProtoS::ETokens::String           }
        , { L"Sub"              , 0 , tGenProtoS::ETokens::Sub              }
        , { L"TermChar"         , 0 , tGenProtoS::ETokens::TermChar         }
        , { L"TextEncoding"     , 0 , tGenProtoS::ETokens::TextEncoding     }
        , { L"ToBool"           , 0 , tGenProtoS::ETokens::ToBool           }
        , { L"ToCard1"          , 0 , tGenProtoS::ETokens::ToCard1          }
        , { L"ToCard2"          , 0 , tGenProtoS::ETokens::ToCard2          }
        , { L"ToCard4"          , 0 , tGenProtoS::ETokens::ToCard4          }
        , { L"ToFloat4"         , 0 , tGenProtoS::ETokens::ToFloat4         }
        , { L"ToFloat8"         , 0 , tGenProtoS::ETokens::ToFloat8         }
        , { L"ToInt1"           , 0 , tGenProtoS::ETokens::ToInt1           }
        , { L"ToInt2"           , 0 , tGenProtoS::ETokens::ToInt1           }
        , { L"ToInt4"           , 0 , tGenProtoS::ETokens::ToInt1           }
        , { L"ToLower"          , 0 , tGenProtoS::ETokens::ToLower          }
        , { L"ToString"         , 0 , tGenProtoS::ETokens::ToString         }
        , { L"ToUpper"          , 0 , tGenProtoS::ETokens::ToUpper          }
        , { L"TranscFunc"       , 0 , tGenProtoS::ETokens::TranscFunc       }
        , { L"True"             , 0 , tGenProtoS::ETokens::True             }
        , { L"TryConnect"       , 0 , tGenProtoS::ETokens::TryConnect       }
        , { L"Type"             , 0 , tGenProtoS::ETokens::Type             }
        , { L"Validate"         , 0 , tGenProtoS::ETokens::Validate         }
        , { L"Value"            , 0 , tGenProtoS::ETokens::Value            }
        , { L"Variable"         , 0 , tGenProtoS::ETokens::Variable         }
        , { L"Variables"        , 0 , tGenProtoS::ETokens::Variables        }
        , { L"Version"          , 0 , tGenProtoS::ETokens::Version          }
        , { L"WaitFor"          , 0 , tGenProtoS::ETokens::WaitFor          }
        , { L"WebSite"          , 0 , tGenProtoS::ETokens::WebSite          }
        , { L"Write"            , 0 , tGenProtoS::ETokens::Write            }
        , { L"WriteAlways"      , 0 , tGenProtoS::ETokens::WriteAlways      }
        , { L"WriteCmd"         , 0 , tGenProtoS::ETokens::WriteCmd         }
        , { L"WriteCmds"        , 0 , tGenProtoS::ETokens::WriteCmds        }
        , { L"XORBits"          , 0 , tGenProtoS::ETokens::XORBits          }

        //
        //  These are not used for lookup, but we do them for translation from
        //  a token to its description.
        //
        , { L"*"                , 0 , tGenProtoS::ETokens::Asterisk         }
        , { L","                , 0 , tGenProtoS::ETokens::Comma            }
        , { L"="                , 0 , tGenProtoS::ETokens::EqualSign        }
        , { L"]"                , 0 , tGenProtoS::ETokens::CloseBracket     }
        , { L")"                , 0 , tGenProtoS::ETokens::CloseParen       }
        , { L"["                , 0 , tGenProtoS::ETokens::OpenBracket      }
        , { L"("                , 0 , tGenProtoS::ETokens::OpenParen        }
        , { L";"                , 0 , tGenProtoS::ETokens::SemiColon        }

        , { L"EOF"              , 0 , tGenProtoS::ETokens::EOF              }
        , { L"Field Reference"  , 0 , tGenProtoS::ETokens::FieldRef         }
        , { L"InputByte"        , 0 , tGenProtoS::ETokens::InputByte        }
        , { L"No Match"         , 0 , tGenProtoS::ETokens::NoMatch          }
        , { L"None"             , 0 , tGenProtoS::ETokens::None             }
        , { L"Numeric Const"    , 0 , tGenProtoS::ETokens::NumericConst     }
        , { L"ReplyBuf Ref"     , 0 , tGenProtoS::ETokens::ReplyBuf         }
        , { L"ReplyLen"         , 0 , tGenProtoS::ETokens::ReplyLen         }
        , { L"WriteCmd Ref"     , 0 , tGenProtoS::ETokens::WriteCmdBuf      }
        , { L"WriteCmd Len"     , 0 , tGenProtoS::ETokens::WriteCmdLen      }
        , { L"Write Value"      , 0 , tGenProtoS::ETokens::WriteVal         }
    };

    TEArray<TMapItem, tGenProtoS::ETokens, tGenProtoS::ETokens::Count> aitemMap(aitemMapVals);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoTokenizer::TTokenInfo
//  PREFIX: toki
//
//  TTokenInfo is used to support token look ahead. We have to remember
//  the token type, its text, and its starting line/col position. These
//  objects can be pushed onto a token pushback stack. eNextToken() will
//  look there first before parsing more source text.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoTokenizer::TTokenInfo: Constructors and destructor
// ---------------------------------------------------------------------------
TGenProtoTokenizer::
TTokenInfo::TTokenInfo( const   tGenProtoS::ETokens eToken
                        , const TString&            strText
                        , const tCIDLib::TCard4     c4StartLine
                        , const tCIDLib::TCard4     c4StartCol) :

    m_c4StartCol(c4StartCol)
    , m_c4StartLine(c4StartLine)
    , m_eToken(eToken)
    , m_strText(strText)
{
}

TGenProtoTokenizer::TTokenInfo::TTokenInfo(const TTokenInfo& tokiToCopy) :

    m_c4StartCol(tokiToCopy.m_c4StartCol)
    , m_c4StartLine(tokiToCopy.m_c4StartLine)
    , m_eToken(tokiToCopy.m_eToken)
    , m_strText(tokiToCopy.m_strText)
{
}

TGenProtoTokenizer::TTokenInfo::~TTokenInfo()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoTokenizer::TTokenInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoTokenizer::TTokenInfo::operator=(const TTokenInfo& tokiToAssign)
{
    if (this != &tokiToAssign)
    {
        m_c4StartCol    = tokiToAssign.m_c4StartCol;
        m_c4StartLine   = tokiToAssign.m_c4StartLine;
        m_eToken        = tokiToAssign.m_eToken;
        m_strText       = tokiToAssign.m_strText;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoTokenizer
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoTokenizer: Public, static methods
// ---------------------------------------------------------------------------
TString
TGenProtoTokenizer::strTokenName(const tGenProtoS::ETokens eToken)
{
    if (eToken >= tGenProtoS::ETokens::Count)
    {
        return TString(L"???");
    }
    return TString(GenProtoS_Tokenizer::aitemMap[eToken].pszName);
}


// ---------------------------------------------------------------------------
//  TGenProtoTokenizer: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoTokenizer::TGenProtoTokenizer( const   TMemBuf&        mbufSrc
                                        , const tCIDLib::TCard4 c4SrcBytes) :
    m_c4CurCol(1)
    , m_c4CurLine(1)
    , m_c4SaveCol(1)
    , m_c4SaveLine(1)
    , m_fcolCharStack(32)
    , m_fcolCondStack(32)
    , m_strmSrc
    (
        new THeapBuf(mbufSrc.pc1Data(), c4SrcBytes)
        , c4SrcBytes
        , tCIDLib::EAdoptOpts::Adopt
        , facCIDEncode().ptcvtMake(L"Latin1")
    )
{
    //
    //  If we haven't initialized the hash table, do it. We have to calc the
    //  hashes for the strings in the map.
    //
    if (!GenProtoS_Tokenizer::atomInitDone)
    {
        TBaseLock lockInit;
        if (!GenProtoS_Tokenizer::atomInitDone)
        {
            // We only do the normal tokens here, not the trailing special ones!
            tGenProtoS::ETokens eIndex = tGenProtoS::ETokens::Min;
            for (; eIndex < tGenProtoS::ETokens::NormalCount; eIndex++)
            {
                GenProtoS_Tokenizer::aitemMap[eIndex].hshName = TRawStr::hshHashStr
                (
                    GenProtoS_Tokenizer::aitemMap[eIndex].pszName
                    , GenProtoS_Tokenizer::c4Modulus
                );
            }

            // And mark us done now
            GenProtoS_Tokenizer::atomInitDone.Set();
        }
    }
    ptcvtFindEncoding(mbufSrc, c4SrcBytes);
}

TGenProtoTokenizer::~TGenProtoTokenizer()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoTokenizer: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoTokenizer::bGetBoolToken()
{
    TString strValue;
    const tGenProtoS::ETokens eTok = eGetNextToken(strValue, kCIDLib::False);
    if ((eTok != tGenProtoS::ETokens::False)
    &&  (eTok != tGenProtoS::ETokens::True))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(L"boolean constant")
        );
    }
    return (eTok == tGenProtoS::ETokens::True);
}


tCIDLib::TBoolean
TGenProtoTokenizer::bIfPeeked(const tGenProtoS::ETokens eToCheck)
{
    //
    //  Peek the next token. If its the one indicated, then eat it and
    //  return true, else return false.
    //
    TString strText;
    const tGenProtoS::ETokens eNext = ePeekNextToken(strText);
    if (eToCheck == eNext)
    {
        eGetNextToken(strText);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TCard4 TGenProtoTokenizer::c4CurCol() const
{
    // We actually give the saved position
    return m_c4SaveCol;
}


tCIDLib::TCard4 TGenProtoTokenizer::c4CurLine() const
{
    // We actually give the saved position
    return m_c4SaveLine;
}


tCIDLib::TCard1 TGenProtoTokenizer::c1GetToken()
{
    // Call the Card4 version, then just see if it'll fit
    const tCIDLib::TCard4 c4Val = c4GetToken();

    if (c4Val > kCIDLib::c1MaxCard)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_ConstOverflow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TCardinal(c4Val)
            , TFacGenProtoS::strXlatType(tGenProtoS::ETypes::Card1)
        );
    }
    return tCIDLib::TCard1(c4Val);
}


tCIDLib::TCard2 TGenProtoTokenizer::c2GetToken()
{
    // Call the Card4 version, then just see if it'll fit
    const tCIDLib::TCard4 c4Val = c4GetToken();

    if (c4Val > kCIDLib::c2MaxCard)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_ConstOverflow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TCardinal(c4Val)
            , TFacGenProtoS::strXlatType(tGenProtoS::ETypes::Card2)
        );
    }
    return tCIDLib::TCard2(c4Val);
}


tCIDLib::TCard4 TGenProtoTokenizer::c4GetToken()
{
    TString strValue;
    const tGenProtoS::ETokens eTok = eGetNextToken(strValue, kCIDLib::False);
    if (eTok != tGenProtoS::ETokens::NumericConst)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(L"numeric constant")
        );
    }

    // Make sure it can be converted into a cardinal value
    tCIDLib::TBoolean     bOk;
    const tCIDLib::TCard4 c4Val = TRawStr::c4AsBinary(strValue.pszBuffer(), bOk);
    if (!bOk)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(L"numeric constant")
        );
    }

    return c4Val;
}


tCIDLib::TCard4
TGenProtoTokenizer::c4ParseCardField(const tGenProtoS::ETokens eToCheck)
{
    CheckNextToken(eToCheck);
    CheckNextToken(tGenProtoS::ETokens::EqualSign);
    const tCIDLib::TCard4 c4Ret = c4GetToken();
    CheckNextToken(tGenProtoS::ETokens::SemiColon);
    return c4Ret;
}


tCIDLib::TVoid TGenProtoTokenizer::CheckNameToken(TString& strToFill)
{
    //
    //  It should come back as a 'no match' since it shouldn't match any
    //  defined token and its not a quoted string.
    //
    if (eGetNextToken(strToFill, kCIDLib::False) != tGenProtoS::ETokens::NoMatch)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(L"name token")
        );
    }

    //
    //  Name tokens must start with a character, but special case the
    //  Accept magic target state.
    //
    if (!TRawStr::bIsAlpha(strToFill.chFirst())
    &&  (strToFill != kGenProtoS_::pszAcceptState))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_BadNameToken
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
        );
    }
}


tCIDLib::TVoid
TGenProtoTokenizer::CheckNextToken(const tGenProtoS::ETokens eToCheck)
{
    TString strTmp;
    if (eGetNextToken(strTmp, kCIDLib::False) != eToCheck)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , strTokenName(eToCheck)
        );
    }
}


tCIDLib::TVoid
TGenProtoTokenizer::CheckTextToken(const tCIDLib::TCh* const pszToCheck)
{
    TString strText;
    const tGenProtoS::ETokens eTok = eGetNextToken(strText, kCIDLib::False);
    if ((eTok != tGenProtoS::ETokens::QuotedString) || (strText != pszToCheck))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(pszToCheck)
        );
    }
}


tGenProtoS::ESpecNodes
TGenProtoTokenizer::eGetBufType(const tGenProtoS::ESpecNodes eLegal)
{
    TString strTmp;
    const tGenProtoS::ETokens eBufTok = eGetNextToken(strTmp);
    tGenProtoS::ESpecNodes eBufType;
    if (eBufTok == tGenProtoS::ETokens::ReplyBuf)
    {
        // Make sure this is legal for the current type of expression
        if (!tCIDLib::bAllBitsOn(eLegal, tGenProtoS::ESpecNodes::ReplyBuf))
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_NoReplyBuf
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotSupported
                , TCardinal(m_c4SaveLine)
                , TCardinal(m_c4SaveCol)
            );
        }
        eBufType = tGenProtoS::ESpecNodes::ReplyBuf;
    }
     else if (eBufTok == tGenProtoS::ETokens::WriteCmdBuf)
    {
        // Make sure this is legal for the current type of expression
        if (!tCIDLib::bAllBitsOn(eLegal, tGenProtoS::ESpecNodes::SendBuf))
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_NoSendBuf
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotSupported
                , TCardinal(m_c4SaveLine)
                , TCardinal(m_c4SaveCol)
            );
        }
        eBufType = tGenProtoS::ESpecNodes::SendBuf;
    }
     else
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_ExpectedBufRef
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
        );
    }
    return eBufType;
}


tGenProtoS::ETokens
TGenProtoTokenizer::eGetNextToken(          TString&            strText
                                    , const tCIDLib::TBoolean   bEOFOk)
{
    //
    //  Extract the next token. If its not a conditional statement, then
    //  just return it. If its the start of a conditional statement, then
    //  we need to just stay here and parse until we get back out of the
    //  conditional section.
    //
    tGenProtoS::ETokens eRet = eExtractToken(strText, bEOFOk);

    if ((eRet != tGenProtoS::ETokens::IfModel)
    &&  (eRet != tGenProtoS::ETokens::IfNModel)
    &&  (eRet != tGenProtoS::ETokens::EndIf))
    {
        // If it's end of file, then make sure the conditional stack is empty
        if ((eRet == tGenProtoS::ETokens::EOF) && !m_fcolCondStack.bIsEmpty())
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_EOFInCond
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(m_c4SaveLine)
                , TCardinal(m_c4SaveCol)
            );
        }
        return eRet;
    }

    //
    //  Its an end, so we have to pop the stack. If the stack is empty, then
    //  we have an unbalanced end.
    //
    if (eRet == tGenProtoS::ETokens::EndIf)
    {
        // If the stack is empty, then an underflow
        if (m_fcolCondStack.bIsEmpty())
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_CondUnderflow
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(m_c4SaveLine)
                , TCardinal(m_c4SaveCol)
            );
        }

        // Else pop the top
        m_fcolCondStack.tPop();
    }
     else
    {
        //
        //  It's a conditional section. So let's parse out the starting
        //  section which has the list of models that this section apply to.
        //  We keep up along the way if one of them matches the target model,
        //  so that we don't have to store them.
        //
        CheckNextToken(tGenProtoS::ETokens::OpenParen);
        tCIDLib::TBoolean bMatch = bParseConditional(strText);

        // If it was a not, then flip the logic
        if (eRet == tGenProtoS::ETokens::IfNModel)
            bMatch = !bMatch;

        // Ok, push the match flag onto the conditional stack
        const tCIDLib::TCard4 c4OrgLevel = m_fcolCondStack.c4ElemCount();
        m_fcolCondStack.Push(bMatch);

        //
        //  If was a non-match, then nothing else matters until we get out
        //  of this conditional block. So let's do a loop where we eat
        //  tokens until we back to this level.
        //
        //  If it matches, we just get another token, the first after the
        //  conditional statement, and return it.
        //
        if (!bMatch)
        {
            while (kCIDLib::True)
            {
                //
                //  Eat tokens, pushing and popping conditional statements
                //  as we go, until we get back to our starting level. We
                //  indicate that end of file is ok, so that it won't throw
                //  if we hit the end. We want to put out a specific error
                //  if that happens, not a generic unexpected EOF.
                //
                eRet = eExtractToken(strText, kCIDLib::True);

                if ((eRet == tGenProtoS::ETokens::IfModel)
                ||  (eRet == tGenProtoS::ETokens::IfNModel))
                {
                    bMatch = bParseConditional(strText);
                    if (eRet == tGenProtoS::ETokens::IfNModel)
                        bMatch = !bMatch;
                    m_fcolCondStack.Push(bMatch);
                }
                 else if (eRet == tGenProtoS::ETokens::EndIf)
                {
                    // If the stack is empty, then an underflow
                    if (m_fcolCondStack.bIsEmpty())
                    {
                        facGenProtoS().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kGPDErrs::errcParse_CondUnderflow
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Format
                            , TCardinal(m_c4SaveLine)
                            , TCardinal(m_c4SaveCol)
                        );
                    }

                    // Else pop the top
                    m_fcolCondStack.tPop();

                    // If we are back to where we were, we can now break out
                    if (m_fcolCondStack.c4ElemCount() == c4OrgLevel)
                        break;
                }
                 else if (eRet == tGenProtoS::ETokens::EOF)
                {
                    facGenProtoS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDErrs::errcParse_EOFInCond
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(m_c4SaveLine)
                        , TCardinal(m_c4SaveCol)
                    );
                }
            }
        }
    }

    //
    //  Ok, now we can get the real token. However, in order to handle the
    //  possibility that it might be another conditional, we call ourself
    //  instead of calling the private extraction method. This way we will
    //  one way or another, get back to here with a real token for the
    //  caller, or hit the end of file.
    //
    //  If they had a really long file, that was nothing but conditional
    //  blocks, that could cause stack problems. But that is so pathological
    //  that its not worth worrying about.
    //
    eRet = eGetNextToken(strText, bEOFOk);

    return eRet;
}


tCQCKit::EFldAccess TGenProtoTokenizer::eParseAccessField()
{
    CheckNextToken(tGenProtoS::ETokens::Access);
    CheckNextToken(tGenProtoS::ETokens::EqualSign);

    //
    //  The next one must be one of the access tokens, which we will translate
    //  to a field access enum.
    //
    TString strValue;
    const tGenProtoS::ETokens eTok = eGetNextToken(strValue, kCIDLib::False);

    tCQCKit::EFldAccess eRet;
    if (eTok == tGenProtoS::ETokens::Read)
        eRet = tCQCKit::EFldAccess::Read;
    else if (eTok == tGenProtoS::ETokens::ReadWrite)
        eRet = tCQCKit::EFldAccess::ReadWrite;
    else if (eTok == tGenProtoS::ETokens::Write)
        eRet = tCQCKit::EFldAccess::Write;
    else
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(L"access type token")
        );
    }

    CheckNextToken(tGenProtoS::ETokens::SemiColon);
    return eRet;
}


tGenProtoS::ETypes TGenProtoTokenizer::eParseExprType()
{
    //
    //  The next one must be one of the type tokens, which we will translate
    //  to an expression type enum.
    //
    TString strValue;
    const tGenProtoS::ETokens eTok = eGetNextToken(strValue, kCIDLib::False);

    tGenProtoS::ETypes eRet;
    if (eTok == tGenProtoS::ETokens::Boolean)
        eRet = tGenProtoS::ETypes::Boolean;
    else if (eTok == tGenProtoS::ETokens::Card1)
        eRet = tGenProtoS::ETypes::Card1;
    else if (eTok == tGenProtoS::ETokens::Card2)
        eRet = tGenProtoS::ETypes::Card2;
    else if (eTok == tGenProtoS::ETokens::Card4)
        eRet = tGenProtoS::ETypes::Card4;
    else if (eTok == tGenProtoS::ETokens::Float4)
        eRet = tGenProtoS::ETypes::Float4;
    else if (eTok == tGenProtoS::ETokens::Float8)
        eRet = tGenProtoS::ETypes::Float8;
    else if (eTok == tGenProtoS::ETokens::Int1)
        eRet = tGenProtoS::ETypes::Int1;
    else if (eTok == tGenProtoS::ETokens::Int2)
        eRet = tGenProtoS::ETypes::Int2;
    else if (eTok == tGenProtoS::ETokens::Int4)
        eRet = tGenProtoS::ETypes::Int4;
    else if (eTok == tGenProtoS::ETokens::String)
        eRet = tGenProtoS::ETypes::String;
    else
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(L"expression type")
        );
    }
    return eRet;
}


tGenProtoS::ETypes TGenProtoTokenizer::eParseExprTypeLine()
{
    CheckNextToken(tGenProtoS::ETokens::Type);
    CheckNextToken(tGenProtoS::ETokens::EqualSign);
    const tGenProtoS::ETypes eRet = eParseExprType();
    CheckNextToken(tGenProtoS::ETokens::SemiColon);
    return eRet;
}


tCQCKit::EFldTypes TGenProtoTokenizer::eParseFieldType()
{
    //
    //  The next one must be one of the type tokens, which we will translate
    //  to a field type enum.
    //
    TString strValue;
    const tGenProtoS::ETokens eTok = eGetNextToken(strValue, kCIDLib::False);

    tCQCKit::EFldTypes eRet;
    if (eTok == tGenProtoS::ETokens::Boolean)
        eRet = tCQCKit::EFldTypes::Boolean;
    else if (eTok == tGenProtoS::ETokens::Card4)
        eRet = tCQCKit::EFldTypes::Card;
    else if (eTok == tGenProtoS::ETokens::Float8)
        eRet = tCQCKit::EFldTypes::Float;
    else if (eTok == tGenProtoS::ETokens::Int4)
        eRet = tCQCKit::EFldTypes::Int;
    else if (eTok == tGenProtoS::ETokens::String)
        eRet = tCQCKit::EFldTypes::String;
    else
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(L"driver field type")
        );
    }
    return eRet;
}


tCQCKit::EFldTypes TGenProtoTokenizer::eParseFieldTypeLine()
{
    CheckNextToken(tGenProtoS::ETokens::Type);
    CheckNextToken(tGenProtoS::ETokens::EqualSign);
    tCQCKit::EFldTypes eRet = eParseFieldType();
    CheckNextToken(tGenProtoS::ETokens::SemiColon);
    return eRet;
}


tGenProtoS::ETokens TGenProtoTokenizer::ePeekNextToken(TString& strText)
{
    // If something is on the pushback stack, then just re-peek it
    if (!m_colTokenStack.bIsEmpty())
    {
        const TTokenInfo& tokiPeek = m_colTokenStack.objPeek();
        strText = tokiPeek.m_strText;
        return tokiPeek.m_eToken;
    }

    // Nothing in the stack, so get the next token
    tGenProtoS::ETokens eNext = eGetNextToken(strText, kCIDLib::True);

    //
    //  Save the current position values, because we have to put them back so
    //  as not to affect the publically visible position.
    //
    const tCIDLib::TCard4 c4OldCol  = m_c4SaveCol;
    const tCIDLib::TCard4 c4OldLine = m_c4SaveLine;

    //
    //  And store the token info into our info object and then push it onto the
    //  token stack.
    //
    TTokenInfo tokiPush
    (
        eNext
        , strText
        , m_c4SaveLine
        , m_c4SaveCol
    );
    m_colTokenStack.objPush(tokiPush);

    // Put the previous positions back, so we don't affect it
    m_c4SaveCol  = c4OldCol;
    m_c4SaveLine = c4OldLine;

    return tokiPush.m_eToken;
}


tCIDLib::TFloat4 TGenProtoTokenizer::f4GetToken()
{
    // Call the Float8 version, then just see if it'll fit
    const tCIDLib::TFloat8 f8Val = f8GetToken();

    if ((f8Val < kCIDLib::f4MinFloat) || (f8Val > kCIDLib::f4MaxFloat))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_ConstOverflow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TFloat(f8Val)
            , TFacGenProtoS::strXlatType(tGenProtoS::ETypes::Float4)
        );
    }
    return tCIDLib::TFloat4(f8Val);
}


tCIDLib::TFloat8 TGenProtoTokenizer::f8GetToken()
{
    TString strValue;
    const tGenProtoS::ETokens eTok = eGetNextToken(strValue, kCIDLib::False);
    if (eTok != tGenProtoS::ETokens::NumericConst)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(L"numeric constant")
        );
    }

    // Make sure it can be converted into a float value
    tCIDLib::TBoolean      bOk;
    const tCIDLib::TFloat8 f8Val = TRawStr::f8AsBinary(strValue.pszBuffer(), bOk);
    if (!bOk)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(L"numeric constant")
        );
    }
    return f8Val;
}


tCIDLib::TFloat8
TGenProtoTokenizer::f8ParseFloatField(const tGenProtoS::ETokens eToCheck)
{
    CheckNextToken(eToCheck);
    CheckNextToken(tGenProtoS::ETokens::EqualSign);
    const tCIDLib::TFloat8 f8Ret = f8GetToken();
    CheckNextToken(tGenProtoS::ETokens::SemiColon);
    return f8Ret;
}


tCIDLib::TVoid TGenProtoTokenizer::GetTextToken(TString& strValue)
{
    const tGenProtoS::ETokens eTok = eGetNextToken(strValue, kCIDLib::False);
    if (eTok != tGenProtoS::ETokens::QuotedString)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(L"quoted string")
        );
    }
}


tCIDLib::TInt1 TGenProtoTokenizer::i1GetToken()
{
    // Call the Int4 version, then just see if it'll fit
    const tCIDLib::TInt4 i4Val = c4GetToken();

    if ((i4Val < kCIDLib::i1MinInt) || (i4Val > kCIDLib::i1MaxInt))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_ConstOverflow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TInteger(i4Val)
            , TFacGenProtoS::strXlatType(tGenProtoS::ETypes::Int1)
        );
    }
    return tCIDLib::TInt1(i4Val);
}


tCIDLib::TInt2 TGenProtoTokenizer::i2GetToken()
{
    // Call the Int4 version, then just see if it'll fit
    const tCIDLib::TInt4 i4Val = c4GetToken();

    if ((i4Val < kCIDLib::i2MinInt) || (i4Val > kCIDLib::i2MaxInt))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_ConstOverflow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TInteger(i4Val)
            , TFacGenProtoS::strXlatType(tGenProtoS::ETypes::Int2)
        );
    }
    return tCIDLib::TInt2(i4Val);
}


tCIDLib::TInt4 TGenProtoTokenizer::i4GetToken()
{
    TString strValue;
    const tGenProtoS::ETokens eTok = eGetNextToken(strValue, kCIDLib::False);
    if (eTok != tGenProtoS::ETokens::NumericConst)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(L"numeric constant")
        );
    }

    // Make sure it can be converted into an int value
    tCIDLib::TBoolean    bOk;
    const tCIDLib::TInt4 i4Val = TRawStr::i4AsBinary(strValue.pszBuffer(), bOk);
    if (!bOk)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_Expected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_c4SaveLine)
            , TCardinal(m_c4SaveCol)
            , TString(L"numeric constant")
        );
    }
    return i4Val;
}


tCIDLib::TInt4
TGenProtoTokenizer::i4ParseIntField(const tGenProtoS::ETokens eToCheck)
{
    CheckNextToken(eToCheck);
    CheckNextToken(tGenProtoS::ETokens::EqualSign);
    const tCIDLib::TInt4 i4Ret = i4GetToken();
    CheckNextToken(tGenProtoS::ETokens::SemiColon);
    return i4Ret;
}


tCIDLib::TVoid TGenProtoTokenizer::
ParseQuotedField(const  tGenProtoS::ETokens eToCheck
                ,       TString&            strValToFill)
{
    CheckNextToken(eToCheck);
    CheckNextToken(tGenProtoS::ETokens::EqualSign);
    GetTextToken(strValToFill);
    CheckNextToken(tGenProtoS::ETokens::SemiColon);
}


const TString& TGenProtoTokenizer::strModel(const TString& strToSet)
{
    m_strTargetModel = strToSet;
    return m_strTargetModel;
}


// ---------------------------------------------------------------------------
//  TGenProtoTokenizer: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoTokenizer::bParseConditional(TString& strToUse)
{
    tCIDLib::TBoolean bMatched = kCIDLib::False;
    while (kCIDLib::True)
    {
        // It has to be a text token
        GetTextToken(strToUse);

        // If its our target model, remember that we saw it
        if (strToUse == m_strTargetModel)
            bMatched = kCIDLib::True;

        // Break out on the closing paren
        if (bIfPeeked(tGenProtoS::ETokens::CloseParen))
            break;

        // Else it has to be a comma
        CheckNextToken(tGenProtoS::ETokens::Comma);
    }
    return bMatched;
}


//
//  Second most primitve char spooling method. At this level, we handle new
//  line processing and current position updating.
//
tCIDLib::TCh TGenProtoTokenizer::chGetNextChar()
{
    // Spool in the next char available
    tCIDLib::TCh chRet = chSpoolAChar();

    //
    //  We convert all newline combinations to a single LF, and we have to
    //  bump the line number and reset the column.
    //
    if ((chRet == kCIDLib::chCR) || (chRet == kCIDLib::chLF))
    {
        //
        //  If a CR, and the next char is LF, then eat the CR and return LF.
        //  Else, just replace it with LF.
        //
        if (chRet == kCIDLib::chCR)
        {
            if (chPeekNextChar() == kCIDLib::chLF)
                chRet = chSpoolAChar();
            else
                chRet = kCIDLib::chLF;
        }

        // Move us to the start of the next line, position-wise
        m_c4CurLine++;
        m_c4CurCol = 1;
    }
     else
    {
        // Must another char on the current line
        m_c4CurCol++;
    }

    return chRet;
}


tCIDLib::TCh TGenProtoTokenizer::chPeekNextChar()
{
    //
    //  If we have something on the pushback stack, just peek at the top
    //  and return that. Else we have to go to the stream. If its at the
    //  end, then return null (we check because we never want to throw an
    //  end of stream from a peek), else read the next char and then
    //  immediately push it back (which doesn't affect current position.)
    //
    tCIDLib::TCh chRet = kCIDLib::chNull;
    if (!m_fcolCharStack.bIsEmpty())
    {
        chRet = m_fcolCharStack.tPeek();
    }
     else
    {
        if (m_strmSrc.bEndOfStream())
        {
            chRet = kCIDLib::chNull;
        }
         else
        {
            chRet = m_strmSrc.chRead();
            m_fcolCharStack.Push(chRet);
        }
    }
    return chRet;
}


//
//  The most primitive char spooling method. Everything gets sucked through
//  here, and we just handle pulling out of the pushback stack first, then
//  going to the stream.
//
tCIDLib::TCh TGenProtoTokenizer::chSpoolAChar()
{
    // First see if there are chars on the pushback stack. If so, use those
    tCIDLib::TCh chRet = kCIDLib::chNull;
    if (!m_fcolCharStack.bIsEmpty())
        chRet = m_fcolCharStack.tPop();
    else
        chRet = m_strmSrc.chRead();

    return chRet;
}


tGenProtoS::ETokens
TGenProtoTokenizer::eExtractToken(TString& strText, const tCIDLib::TBoolean bEOFOk)
{
    //
    //  First check the token stack. If there's something on it, then we
    //  need to give that back first.
    //
    if (!m_colTokenStack.bIsEmpty())
    {
        TTokenInfo tokiRet = m_colTokenStack.objPop();
        strText = tokiRet.m_strText;
        m_c4SaveCol  = tokiRet.m_c4StartCol;
        m_c4SaveLine = tokiRet.m_c4StartLine;
        return tokiRet.m_eToken;
    }

    // Nothing on the stack, so lets get fresh meat
    tCIDLib::TBoolean       bField      = kCIDLib::False;
    tCIDLib::TBoolean       bQuoted     = kCIDLib::False;
    tCIDLib::TBoolean       bSpecial    = kCIDLib::False;
    tCIDLib::TCh            chCur;
    tGenProtoS::ETokens   eRet = tGenProtoS::ETokens::NoMatch;
    try
    {
        // Empty the string
        strText.Clear();

        // Skip any white space. We eat whitespace and don't report it
        while (kCIDLib::True)
        {
            chCur = chGetNextChar();
            while (TRawStr::bIsSpace(chCur))
                chCur = chGetNextChar();

            //
            //  Before we push the break out char back, see if its a forward
            //  slash character. If so, check the next char. If its one too,
            //  then this is a quote and we want to eat it and try again.
            //
            if ((chCur == L'/') && (chPeekNextChar() == L'/'))
            {
                EatLineRemainder();
            }
             else
            {
                // Looks like something we want, so push it back and break out
                PushBack(chCur);
                break;
            }
        }

        // Set the current position as the save position
        SavePos();

        //
        //  First peek the next char and see if its a quote. If so, then we
        //  are in a quoted string and will ignore special chars other than
        //  the end quote. Also, see if its a & sign, in which case it must
        //  be a field reference, or a % sign, in which case it refers to the
        //  the write value.
        //
        bQuoted     = (chPeekNextChar() == kCIDLib::chQuotation);
        bField      = (chPeekNextChar() == kCIDLib::chDollarSign);
        bSpecial    = (chPeekNextChar() == kCIDLib::chAmpersand);

        // If its one of the special values, each this char
        if (bQuoted || bField || bSpecial)
            chGetNextChar();

        //
        //  Ok, we now read chars until we hit one that forces a token. This
        //  can be a whitespace, or one of the special chars such as equal
        //  sign, semi-colon, etc..., or the ending quote if in a quoted
        //  string.
        //
        while (kCIDLib::True)
        {
            // Get the next char
            chCur = chGetNextChar();

            if (bQuoted)
            {
                // Break out on the closing quote, else store the char
                if (chCur == kCIDLib::chQuotation)
                {
                    eRet = tGenProtoS::ETokens::QuotedString;
                    break;
                }
                 else
                {
                    //
                    //  We need to watch for escape characters in quoted
                    //  strings and handle them.
                    //
                    if (chCur == kCIDLib::chBackSlash)
                    {
                        const tCIDLib::TCh chNext = chGetNextChar();

                        switch(chNext)
                        {
                            case kCIDLib::chLatin_r :
                                chCur = kCIDLib::chCR;
                                break;

                            case kCIDLib::chLatin_n :
                                chCur = kCIDLib::chLF;
                                break;

                            case kCIDLib::chLatin_t :
                                chCur = kCIDLib::chTab;
                                break;

                            default :
                                // Just take the char as is
                                PushBack(chNext);
                                break;
                        };
                    }
                    strText.Append(chCur);
                }
            }
             else if (TRawStr::bIsSpace(chCur))
            {
                //
                //  Break out if we hit a whitespace, since its not in a
                //  quoted string at this point.
                //
                break;
            }
             else
            {
                //
                //  Check for a special char. If we get one, and this is the
                //  first char of this token, then it is the token itself. Else,
                //  it terminates the token and we push it back. Either way,
                //  we break out.
                //
                if ((chCur == kCIDLib::chAsterisk)
                ||  (chCur == kCIDLib::chCloseBracket)
                ||  (chCur == kCIDLib::chCloseParen)
                ||  (chCur == kCIDLib::chComma)
                ||  (chCur == kCIDLib::chEquals)
                ||  (chCur == kCIDLib::chOpenBracket)
                ||  (chCur == kCIDLib::chOpenParen)
                ||  (chCur == kCIDLib::chQuotation)
                ||  (chCur == kCIDLib::chSemiColon))
                {
                    if (strText.bIsEmpty())
                    {
                        switch(chCur)
                        {
                            case kCIDLib::chAsterisk :
                                eRet = tGenProtoS::ETokens::Asterisk;
                                break;

                            case kCIDLib::chCloseBracket :
                                eRet = tGenProtoS::ETokens::CloseBracket;
                                break;

                            case kCIDLib::chCloseParen :
                                eRet = tGenProtoS::ETokens::CloseParen;
                                break;

                            case kCIDLib::chComma :
                                eRet = tGenProtoS::ETokens::Comma;
                                break;

                            case kCIDLib::chEquals :
                                eRet = tGenProtoS::ETokens::EqualSign;
                                break;

                            case kCIDLib::chOpenBracket :
                                eRet = tGenProtoS::ETokens::OpenBracket;
                                break;

                            case kCIDLib::chOpenParen :
                                eRet = tGenProtoS::ETokens::OpenParen;
                                break;

                            case kCIDLib::chQuotation :
                                // A quote in the middle of something else
                                facGenProtoS().ThrowErr
                                (
                                    CID_FILE
                                    , CID_LINE
                                    , kGPDErrs::errcParse_BadQuoteHere
                                    , tCIDLib::ESeverities::Failed
                                    , tCIDLib::EErrClasses::BadParms
                                    , TCardinal(m_c4CurLine)
                                    , TCardinal(m_c4CurCol)
                                );
                                break;

                            case kCIDLib::chSemiColon :
                                eRet = tGenProtoS::ETokens::SemiColon;
                                break;

                            default :
                                break;
                        }
                    }
                     else
                    {
                        PushBack(chCur);
                    }

                    // And break out out of the loop
                    break;
                }
                 else
                {
                    strText.Append(chCur);
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        //
        //  If its an EOF, then see if we can legally get one here. If not,
        //  or its some other error, then just throw. Even if they say EOF
        //  is ok, if we were in a quoted string, then this is not legal, so
        //  check that also.
        //
        const tCIDLib::TBoolean bEOF = errToCatch.bCheckEvent
        (
            facCIDLib().strName()
            , kCIDErrs::errcStrm_EndOfStream
        );

        if (bEOF)
        {
            if (bQuoted)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_UnterminatedStr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(m_c4SaveLine)
                    , TCardinal(m_c4SaveCol)
                );
            }

            if (!bEOFOk)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcParse_UnexpectedEOF
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(m_c4SaveLine)
                    , TCardinal(m_c4SaveCol)
                );
            }
        }
         else
        {
            // Just rethrow original
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            throw;
        }
    }

    //
    //  Ok it it wasn't a special character token, then the return is set
    //  to 'no match' still. So lets look it up in the map, but we still have
    //  two special cases to check.
    //
    if (bField)
    {
        eRet = tGenProtoS::ETokens::FieldRef;
    }
     else if (bSpecial)
    {
        if (strText == L"InputByte")
            eRet = tGenProtoS::ETokens::InputByte;
        else if (strText == L"Reply")
            eRet = tGenProtoS::ETokens::ReplyBuf;
        else if (strText == L"ReplyLen")
            eRet = tGenProtoS::ETokens::ReplyLen;
        else if (strText == L"WriteCmd")
            eRet = tGenProtoS::ETokens::WriteCmdBuf;
        else if (strText == L"WriteCmdLen")
            eRet = tGenProtoS::ETokens::WriteCmdLen;
        else if (strText == L"WriteVal")
            eRet = tGenProtoS::ETokens::WriteVal;
        else
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_UnknownSpecialVal
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(m_c4SaveLine)
                , TCardinal(m_c4SaveCol)
                , strText
            );
        }
    }
     else if (eRet == tGenProtoS::ETokens::NoMatch)
    {
        if (strText.bIsEmpty())
            eRet = tGenProtoS::ETokens::EOF;
        else
            eRet = eMapToken(strText.pszBuffer());

        //
        //  If still no match, see if it looks like a legal numeric
        //  constant.
        //
        if (eRet == tGenProtoS::ETokens::NoMatch)
        {
            //
            //  In order to be a numeric constant. The first character must
            //  be a minus sign, or a decimal digit. So we can do a quick check
            //  that will prevent us from wasting time trying to convert many
            //  name tokens.
            //
            const tCIDLib::TCh chFirst = strText.chFirst();
            if ((chFirst == kCIDLib::chHyphenMinus)
            ||  TRawStr::bIsDigit(chFirst))
            {
                tCIDLib::TBoolean bOk;
                TRawStr::c4AsBinary(strText.pszBuffer(), bOk);
                if (!bOk)
                {
                    TRawStr::i4AsBinary(strText.pszBuffer(), bOk);
                    if (!bOk)
                        TRawStr::f8AsBinary(strText.pszBuffer(), bOk);
                }

                if (bOk)
                    eRet = tGenProtoS::ETokens::NumericConst;
            }
        }
    }
    return eRet;
}


tGenProtoS::ETokens
TGenProtoTokenizer::eMapToken(const tCIDLib::TCh* const pszToMap) const
{
    // Hash the passed string
    const tCIDLib::THashVal hshFind = TRawStr::hshHashStr
    (
        pszToMap
        , GenProtoS_Tokenizer::c4Modulus
    );

    //
    //  Look through the map. If we get a hash match, then check the whole
    //  string.
    //
    tGenProtoS::ETokens  eIndex = tGenProtoS::ETokens::Min;
    for (; eIndex < tGenProtoS::ETokens::NormalCount; eIndex++)
    {
        if (hshFind != GenProtoS_Tokenizer::aitemMap[eIndex].hshName)
            continue;

        if (TRawStr::bCompareStr(GenProtoS_Tokenizer::aitemMap[eIndex].pszName, pszToMap))
            break;
    }

    // If we found a match, then return the token for that guy
    if (eIndex < tGenProtoS::ETokens::NormalCount)
        return GenProtoS_Tokenizer::aitemMap[eIndex].eToken;

    // We never found a match
    return tGenProtoS::ETokens::NoMatch;
}


tCIDLib::TVoid TGenProtoTokenizer::EatLineRemainder()
{
    // Eat until we it a new line or the end of the input
    while (kCIDLib::True)
    {
        if ((chGetNextChar() == kCIDLib::chLF) || m_strmSrc.bEndOfStream())
            break;
    }
}


//
//  All protocol files must start with an introductory line similar to that
//  of an XML file. It allows us to figure out the encoding of the file on
//  our own, which means that we can parse a file in an encoding that the
//  user doesn't know anything about (and therefore cannot tell us what the
//  encoding is.)
//
//  NOTE:   For the moment, we just assume its ISO-8859-1. We do require
//          that the line be there for future implementation of the auto-
//          sensing, but its not used yet. We enforce the fixed format for
//          now.
//
TTextConverter*
TGenProtoTokenizer::ptcvtFindEncoding(const TMemBuf&, const tCIDLib::TCard4)
{
    // We are looking for: [CQCProto Version="2.0" Encoding="ISO-8859-1"]
    CheckNextToken(tGenProtoS::ETokens::OpenBracket);
    CheckNextToken(tGenProtoS::ETokens::CQCProto);
    CheckNextToken(tGenProtoS::ETokens::Version);
    CheckNextToken(tGenProtoS::ETokens::EqualSign);
    CheckTextToken(L"2.0");
    CheckNextToken(tGenProtoS::ETokens::Encoding);
    CheckNextToken(tGenProtoS::ETokens::EqualSign);
    CheckTextToken(L"ISO-8859-1");
    CheckNextToken(tGenProtoS::ETokens::CloseBracket);

    return 0;
}


tCIDLib::TVoid TGenProtoTokenizer::PushBack(const tCIDLib::TCh chToPush)
{
    //
    //  Don't allow them to push back over a newline. So if either the new
    //  char is a new line, or we are at at column 1.
    //
    if ((chToPush == kCIDLib::chLF) || (m_c4CurCol == 1))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcParse_PushBackNL
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }

    // Looks ok, so push it back and decrement the column number
    m_fcolCharStack.Push(chToPush);
    m_c4CurCol--;
}


tCIDLib::TVoid TGenProtoTokenizer::SavePos()
{
    m_c4SaveCol  = m_c4CurCol;
    m_c4SaveLine = m_c4CurLine;
}


