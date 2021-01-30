//
// FILE NAME: CQCKit_CndIntf.cpp
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
//  This file implements the classes that make up CQC's generic command
//  architcture.
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
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Local types and data
// ---------------------------------------------------------------------------
namespace CQCKit_CmdIntf
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The parameter definition persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2ParmVersionFmt    = 2;

        // -----------------------------------------------------------------------
        //  The command persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2CmdVersionFmt     = 1;

        // -----------------------------------------------------------------------
        //  The command config persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2CfgVersionFmt     = 1;

        // -----------------------------------------------------------------------
        //  The RT value persistent format version.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2RTValVersionFmt   = 1;


        // -----------------------------------------------------------------------
        //  The command source persistent format version
        //
        //  Version 2 -
        //      The action context was moved into the event info, so that we can
        //      have a separate command context for ech event, which has become
        //      necessary. We'll initialize old ones to the standard context and
        //      it's the responsibility of the client code to change that after
        //      streaming us in if necessary.
        //
        //  Version 3 -
        //      There was no change in the storage format, however all of the
        //      action parameters that are variable names were changed to a newly
        //      created variable parameter type so that they can be recognized as
        //      such generically. There are a lot of them, so we don't want to
        //      all that checking every time forever. So this way we only have to
        //      do the conversion when we see a V2.
        //
        //  Version 4 -
        //      Again, no change in format, but we now support hierarchical storage
        //      for templates. So any commands that reference them have to move
        //      them into a \User area.
        //
        //  Version 5 -
        //      And again, no change in format. We just modified some commands
        //      and need to update them when loaded from older versions.
        //
        //  Version 6 -
        //      And once again, no change in format. We just need to update cmds
        //      during the upgrade process.
        //
        //  Version 7 -
        //      Again to be able to adjust commands. We redo the media repo
        //      searching command names and ids to make them consistent.
        //
        //  Version 8 -
        //      Add a new parameter to the Exit command.
        //
        //  Version 9 -
        //      Added a new case/no case paramater to the system equals command.
        //
        //  Version 10 -
        //      Added a new parameter to the set blanker options command of the
        //      intf engine's view class.
        //
        //  Version 11 -
        //      Added a new parameter to the IsNight command, to allow for an
        //      offset.
        //
        //  Version 12 -
        //      The cover art browser got rid of the UpLevel command and it's now
        //      just the same as the 'Back' command, so we convert the up level
        //      commands to back commands.
        //
        //  Version 13 -
        //      Added a 'format' parameter to the set widget color commands, which
        //      allow the various colors of interface widgets to be set on the fly.
        //
        //  Version 14 - 4.4.925
        //      Added new parameters to some of the event target commands, to get
        //      the newly added name values out of some of the incoming triggers.
        //
        //  Version 15 - 4.5.8
        //      More additional parameters added to commands
        //
        //  Version 16 - 4.6.910
        //      More addition parameters added to commands, the WaitDriverRead cmd
        //
        //  Version 17 - 4.8.900
        //      Our new, unified view of data server base resources uses forward slashes.
        //      This gets rid of the issues with escapement (\ is the escape char) for
        //      all of those types of resources. So, we have to update all parameters
        //      that refer to such resources to replace the slashes.
        //
        //      We also moved scheduled events and event monitors into a the hierarchical
        //      space, so we will update those to take the existing event id and convert
        //      it to /User/[eventid], which is where the installer will move them during
        //      the upgrade to 5.0. Triggered events are currently not referenced by
        //      any action commands, so those we can ignore.
        //
        //  Version 18 - 5.2.913
        //      Added a new parameter to the InvokePopup command which we need to fault
        //      in.
        //
        //  Version 19 - 5.3.903
        //      Needed to add a new parameter to the Devices::GetSemFields() command, which
        //      required doubling up the old parm 3/4 because we already were using them
        //      all. So we have to move parm 4 to append to parm 3, and add a new parm
        //      4 default value.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2SrcVersionFmt     = 19;


        // -----------------------------------------------------------------------
        //  The opcode persistent format version
        //
        //  Version 2 -
        //      Added support for a disable flag, to allow commands to be disabled
        //      temporarily without being removed.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2OpVersionFmt      = 2;

        // -----------------------------------------------------------------------
        //  The command target persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2TarVersionFmt     = 1;


        // -----------------------------------------------------------------------
        //  The action event info persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2ActEvInfoVersionFmt = 1;


        // -----------------------------------------------------------------------
        //  A simple class to hold the definitions of the base runtime values that
        //  we can use to fault in the human readable names and load up the list
        //  in the command source class.
        // -----------------------------------------------------------------------
        class TRTValItem
        {
            public :
                TRTValItem( const   tCIDLib::TMsgId     midLoad
                            , const TString&            strId
                            , const tCQCKit::ECmdPTypes eType) :
                    m_eType(eType)
                    , m_midLoad(midLoad)
                    , m_strId(strId)
                {
                    m_strText.LoadFromMsg(midLoad, facCQCKit());
                }

                tCIDLib::TMsgId     m_midLoad;
                TString             m_strId;
                tCQCKit::ECmdPTypes m_eType;
                TString             m_strText;
        };
        TAtomicFlag          atomRTValsLoaded;
        TVector<TRTValItem>  colRTVs(32);
    }
}



// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCActEvInfo,TObject)
RTTIDecls(TCQCCmdParm,TObject)
RTTIDecls(TCQCCmd,TObject)
AdvRTTIDecls(TActOpcode,TObject)
RTTIDecls(TCQCCmdCfg,TObject)
RTTIDecls(TCQCCmdRTVal,TObject)
RTTIDecls(TCQCCmdRTVSrc,TObject)



static tCIDLib::TVoid FixCondBug(MCQCCmdSrcIntf& mcsrcFix)
{
    TFundStack<tCIDLib::TCard4> fcolOfs(64);
    TVector<TCQCCmd> colCmds;

    TCQCCmd cmdTmp;
    const tCIDLib::TCard4 c4EventCnt = mcsrcFix.c4EventCount();
    for (tCIDLib::TCard4 c4EvIndex = 0; c4EvIndex < c4EventCnt; c4EvIndex++)
    {
        const TCQCActEvInfo& caeiCur = mcsrcFix.caeiEventAt(c4EvIndex);
        const TString& strEvId = caeiCur.m_strEventId;
        MCQCCmdSrcIntf::TOpcodeBlock* pcolOps
        (
            mcsrcFix.pcolOpsForEvent(strEvId)
        );

        const tCIDLib::TCard4 c4OpCount = pcolOps->c4ElemCount();
        for (tCIDLib::TCard4 c4OpIndex = 0; c4OpIndex < c4OpCount; c4OpIndex++)
        {
            TActOpcode& aocCur = pcolOps->objAt(c4OpIndex);
            switch(aocCur.m_eOpcode)
            {
                case tCQCKit::EActOps::Cmd :
                {
                    break;
                }

                case tCQCKit::EActOps::End :
                {
                    //
                    //  Pop the previous If/Else offset off the stack, and go
                    //  back and update it with the difference.
                    //
                    const tCIDLib::TCard4 c4Ofs = fcolOfs.tPop();
                    pcolOps->objAt(c4Ofs).m_i4Ofs = tCIDLib::TInt4(c4OpIndex - c4Ofs);
                    break;
                }

                case tCQCKit::EActOps::Else :
                {
                    //
                    //  Update the If we are for so that it jumps to here, then
                    //  replace the top of stack with the current offset, so
                    //  that the End will see this Else as it's precedent.
                    //
                    const tCIDLib::TCard4 c4IfOfs = fcolOfs.tPop();
                    pcolOps->objAt(c4IfOfs).m_i4Ofs = tCIDLib::TInt4(c4OpIndex - c4IfOfs);
                    fcolOfs.Push(c4OpIndex);
                    break;
                }

                case tCQCKit::EActOps::If :
                {
                    fcolOfs.Push(c4OpIndex);
                    break;
                }

                default :
                    // We can ignore this one
                    break;
            };
        }
    }
}



// ---------------------------------------------------------------------------
//  Local helper functions
// ---------------------------------------------------------------------------

//
//  Loads up the human readable text into the list of base RTVs that we
//  support at this level. The caller locks before calling us. We have to
//  fault it in from a couple places below, to make sure it's available when
//  needed.
//
static tCIDLib::TVoid LoadRTVList()
{
    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_CurMillis, kCQCKit::strRTVId_Millis, tCQCKit::ECmdPTypes::Unsigned
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_CurUserName, kCQCKit::strRTVId_CurUserName, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_CurUserRole, kCQCKit::strRTVId_CurUserRole, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_CQCVersion, kCQCKit::strRTVId_CQCVersion, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_DDMMYYYY, kCQCKit::strRTVId_DDMMYYYY, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_DayNumber, kCQCKit::strRTVId_DayNumber, tCQCKit::ECmdPTypes::Unsigned
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_CurDateTime, kCQCKit::strRTVId_CurDateTime, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_Env01, kCQCKit::strRTVId_Env01, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_Env02, kCQCKit::strRTVId_Env02, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_Env03, kCQCKit::strRTVId_Env03, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_Env04, kCQCKit::strRTVId_Env04, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_Env05, kCQCKit::strRTVId_Env05, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_Env06, kCQCKit::strRTVId_Env06, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_Env07, kCQCKit::strRTVId_Env07, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_Env08, kCQCKit::strRTVId_Env08, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_Env09, kCQCKit::strRTVId_Env09, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_HHMM, kCQCKit::strRTVId_HHMM, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_HHMMSS, kCQCKit::strRTVId_HHMMSS, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_HostName, kCQCKit::strRTVId_HostName, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_Hour, kCQCKit::strRTVId_Hour, tCQCKit::ECmdPTypes::Unsigned
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_IPAddr, kCQCKit::strRTVId_IPAddr, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_Minute, kCQCKit::strRTVId_Minute, tCQCKit::ECmdPTypes::Unsigned
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_MMDDYYYY, kCQCKit::strRTVId_MMDDYYYY, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_Month, kCQCKit::strRTVId_Month, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_MonthNumber, kCQCKit::strRTVId_MonthNumber, tCQCKit::ECmdPTypes::Unsigned
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_MSHost, kCQCKit::strRTVId_MSHost, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_TimeStamp, kCQCKit::strRTVId_TimeStamp, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_WeekDay, kCQCKit::strRTVId_WeekDay, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::colRTVs.objPlace
    (
        kKitMsgs::midRT_YYYY, kCQCKit::strRTVId_YYYY, tCQCKit::ECmdPTypes::Text
    );

    CQCKit_CmdIntf::atomRTValsLoaded.Set();
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCCmdParm
//  PREFIX: cmdp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCCmdParm: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCCmdParm::TCQCCmdParm() :

    m_bRequired(kCIDLib::True)
    , m_eFldAccess(tCQCKit::EReqAccess::None)
    , m_eType(tCQCKit::ECmdPTypes::None)
{
}

TCQCCmdParm::TCQCCmdParm(const  TString&            strName
                        , const tCQCKit::ECmdPTypes eType
                        , const tCIDLib::TBoolean   bRequired) :

    m_bRequired(bRequired)
    , m_eFldAccess(tCQCKit::EReqAccess::None)
    , m_eType(eType)
    , m_strName(strName)
{
}

TCQCCmdParm::TCQCCmdParm(const  TString&            strName
                        , const TString&            strEnumVals
                        , const tCIDLib::TBoolean   bRequired) :

    m_bRequired(bRequired)
    , m_eFldAccess(tCQCKit::EReqAccess::None)
    , m_eType(tCQCKit::ECmdPTypes::Enum)
    , m_strExtra(strEnumVals)
    , m_strName(strName)
{
}

TCQCCmdParm::TCQCCmdParm(const  TString&            strName
                        , const TString&            strEnumVals
                        , const TString&            strDisplayVals
                        , const tCIDLib::TBoolean   bRequired) :

    m_bRequired(bRequired)
    , m_eFldAccess(tCQCKit::EReqAccess::None)
    , m_eType(tCQCKit::ECmdPTypes::Enum)
    , m_strExtra(strEnumVals)
    , m_strExtra2(strDisplayVals)
    , m_strName(strName)
{
}

TCQCCmdParm::~TCQCCmdParm()
{
}


// ---------------------------------------------------------------------------
//  TCQCCmdParm: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCCmdParm::operator==(const TCQCCmdParm& cmdpSrc) const
{
    if (this != &cmdpSrc)
    {
        if ((m_strName     != cmdpSrc.m_strName)
        ||  (m_eType       != cmdpSrc.m_eType)
        ||  (m_bRequired   != cmdpSrc.m_bRequired)
        ||  (m_eFldAccess  != cmdpSrc.m_eFldAccess)
        ||  (m_strExtra    != cmdpSrc.m_strExtra)
        ||  (m_strExtra2   != cmdpSrc.m_strExtra2))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TCQCCmdParm::operator!=(const TCQCCmdParm& cmdpSrc) const
{
    return !operator==(cmdpSrc);
}


// ---------------------------------------------------------------------------
//  TCQCCmdParm: Public, non-virtual methods
// ---------------------------------------------------------------------------

// See if the passed value is one of the enumerated values
tCIDLib::TBoolean
TCQCCmdParm::bIsLegalEnumVal(const  TString&            strToCheck
                            ,       tCIDLib::TCard4&    c4Index) const
{
    CIDAssert
    (
        m_eType == tCQCKit::ECmdPTypes::Enum
        , L"The parameter is not an enumerated parm"
    );

    TStringTokenizer stokVals(&m_strExtra, L",");
    TString strTok;
    c4Index = 0;
    while (stokVals.bGetNextToken(strTok))
    {
        strTok.StripWhitespace();
        if (strTok == strToCheck)
            return kCIDLib::True;
        c4Index++;
    }
    return kCIDLib::False;
}


// Get/set the required field
tCIDLib::TBoolean TCQCCmdParm::bRequired() const
{
    return m_bRequired;
}

tCIDLib::TBoolean TCQCCmdParm::bRequired(const tCIDLib::TBoolean bToSet)
{
    m_bRequired = bToSet;
    return m_bRequired;
}


tCQCKit::EReqAccess TCQCCmdParm::eFldAccess() const
{
    return m_eFldAccess;
}

tCQCKit::EReqAccess TCQCCmdParm::eFldAccess(const tCQCKit::EReqAccess eToSet)
{
    m_eFldAccess = eToSet;
    return m_eFldAccess;
}


tCQCKit::ECmdPTypes TCQCCmdParm::eType() const
{
    return m_eType;
}


//
//  If this is an enum parameter, we get the first value in either the internal or the display
//  values, depending on what is asked for and what is available. If they ask for display but
//  we don't have a separate display value set, we give them the internal value, which is
//  always what is desired.
//
tCIDLib::TVoid
TCQCCmdParm::QueryDefEnumVal(TString& strToFill, const tCIDLib::TBoolean bDisplayVal) const
{
    CIDAssert
    (
        m_eType == tCQCKit::ECmdPTypes::Enum
        , L"The parameter is not an enumerated parm"
    );

    // Figure out which src string to use
    const TString* pstrSrc = &m_strExtra;
    if (bDisplayVal && !m_strExtra2.bIsEmpty())
        pstrSrc = &m_strExtra2;

    // Just scan up to the first comma
    const TString& strSrc = *pstrSrc;
    tCIDLib::TCard4 c4Length = strSrc.c4Length();
    strToFill.Clear();
    tCIDLib::TCard4 c4Index = 0;
    while (c4Index < c4Length)
    {
        const tCIDLib::TCh chCur = strSrc[c4Index];
        if (chCur == kCIDLib::chComma)
            break;
        strToFill.Append(chCur);
        c4Index++;
    }
    strToFill.StripWhitespace();
}


//
//  The value of each returned key/value pair is the display name and the value is the
//  internal name. If we only have internal names, then both will be set to the same
//  thing. The display name is the key since if we sort the list for display we want to
//  sort by what is being shown.
//
tCIDLib::TVoid TCQCCmdParm::QueryEnumVals(tCIDLib::TKVPCollect& colToFill) const
{
    CIDAssert
    (
        m_eType == tCQCKit::ECmdPTypes::Enum
        , L"The parameter is not an enumerated parm"
    );

    colToFill.RemoveAll();

    // Remember if we have separate lists
    const tCIDLib::TBoolean bSepList = !m_strExtra2.bIsEmpty();

    //
    //  Break out the values and load them up. If we have a separate display values list,
    //  we break it out also, else we set both lists to the same thing.
    //
    TStringTokenizer stokVals(&m_strExtra, L",");
    TString strTok;
    TKeyValuePair kvalCur;

    tCIDLib::TStrList colIntVals;
    while (stokVals.bGetNextToken(strTok))
    {
        strTok.StripWhitespace();
        colIntVals.objAdd(strTok);
    }

    // If we have separate lists, do the display values
    const tCIDLib::TCard4 c4ValCount = colIntVals.c4ElemCount();
    if (bSepList)
    {
        tCIDLib::TStrList colDisplayVals;

        stokVals.Reset(&m_strExtra2, L",");
        while (stokVals.bGetNextToken(strTok))
        {
            strTok.StripWhitespace();
            colDisplayVals.objAdd(strTok);
        }

        // Both lists have to be the same size
        if (c4ValCount != colDisplayVals.c4ElemCount())
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_EnumValsListSizes
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::OutOfSync
            );
        }

        // Load up the caller's list from both our lists
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCount; c4Index++)
        {
            kvalCur.Set(colDisplayVals[c4Index], colIntVals[c4Index]);
            colToFill.objAdd(kvalCur);
        }
    }
     else
    {
        // Load up the caller's list just from our internal values list
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCount; c4Index++)
        {
            kvalCur.Set(colIntVals[c4Index], colIntVals[c4Index]);
            colToFill.objAdd(kvalCur);
        }
    }
}


// Get/set the extra value (whose meaning depends on the parm type)
const TString& TCQCCmdParm::strExtra() const
{
    return m_strExtra;
}

const TString& TCQCCmdParm::strExtra(const TString& strToSet)
{
    m_strExtra = strToSet;

    // If we are an enum, then clear the extra 2 value. They must be set together
    if (m_eType == tCQCKit::ECmdPTypes::Enum)
        m_strExtra2.Clear();

    return m_strExtra;
}


const TString& TCQCCmdParm::strName() const
{
    return m_strName;
}


// ---------------------------------------------------------------------------
//  TCQCCmdParm: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCCmdParm::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff must start with an object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version for this level
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_CmdIntf::c2ParmVersionFmt))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_strName
                    >> m_eFldAccess
                    >> m_eType
                    >> m_strExtra;

    if (c2FmtVersion > 1)
        strmToReadFrom >> m_strExtra2;
    else
        m_strExtra2.Clear();

    // And our stuff must end with a marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCCmdParm::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_CmdIntf::c2ParmVersionFmt
                    << m_strName
                    << m_eFldAccess
                    << m_eType
                    << m_strExtra

                    // V2 stuff
                    << m_strExtra2

                    << tCIDLib::EStreamMarkers::Frame;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCCmd
//  PREFIX: cmd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCCmd: Constructors and Destructor
// ---------------------------------------------------------------------------
const TString& TCQCCmd::strKey(const TCQCCmd& cmdSrc)
{
    return cmdSrc.m_strId;
}


// ---------------------------------------------------------------------------
//  TCQCCmd: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCCmd::TCQCCmd() :

    m_c4MaxParms(0)
    , m_colParms()
    , m_eType(tCQCKit::ECmdTypes::SideEffect)
{
}

TCQCCmd::TCQCCmd(const  TString&            strId
                , const TString&            strName
                , const tCIDLib::TCard4     c4ParmCnt) :

    m_c4MaxParms(c4ParmCnt)
    , m_colParms(c4ParmCnt)
    , m_eType(tCQCKit::ECmdTypes::SideEffect)
    , m_strId(strId)
    , m_strName(strName)
{
}

TCQCCmd::TCQCCmd(const  TString&            strId
                , const TString&            strName
                , const tCQCKit::ECmdPTypes eParmType
                , const TString&            strParmName) :

    m_c4MaxParms(1)
    , m_colParms(1)
    , m_eType(tCQCKit::ECmdTypes::SideEffect)
    , m_strId(strId)
    , m_strName(strName)
{
    m_colParms.objPlace(strParmName, eParmType);
}

TCQCCmd::TCQCCmd(const  TString&            strId
                , const TString&            strName
                , const TString&            strParmName
                , const TString&            strEnumVals) :

    m_c4MaxParms(1)
    , m_colParms(1)
    , m_eType(tCQCKit::ECmdTypes::SideEffect)
    , m_strId(strId)
    , m_strName(strName)
{
    m_colParms.objPlace(strParmName, strEnumVals);
}

TCQCCmd::TCQCCmd(const  TString&            strId
                , const TString&            strName
                , const tCQCKit::ECmdPTypes eParm1Type
                , const TString&            strParm1Name
                , const tCQCKit::ECmdPTypes eParm2Type
                , const TString&            strParm2Name) :

    m_c4MaxParms(2)
    , m_colParms(2)
    , m_eType(tCQCKit::ECmdTypes::SideEffect)
    , m_strId(strId)
    , m_strName(strName)
{
    m_colParms.objPlace(strParm1Name, eParm1Type);
    m_colParms.objPlace(strParm2Name, eParm2Type);
}

TCQCCmd::TCQCCmd(const  TString&            strId
                , const TString&            strName
                , const tCQCKit::ECmdPTypes eParm1Type
                , const TString&            strParm1Name
                , const tCQCKit::ECmdPTypes eParm2Type
                , const TString&            strParm2Name
                , const tCQCKit::ECmdPTypes eParm3Type
                , const TString&            strParm3Name) :

    m_c4MaxParms(3)
    , m_colParms(3)
    , m_eType(tCQCKit::ECmdTypes::SideEffect)
    , m_strId(strId)
    , m_strName(strName)
{
    m_colParms.objPlace(strParm1Name, eParm1Type);
    m_colParms.objPlace(strParm2Name, eParm2Type);
    m_colParms.objPlace(strParm3Name, eParm3Type);
}

TCQCCmd::TCQCCmd(const  TString&            strId
                , const TString&            strName
                , const tCQCKit::ECmdPTypes eParm1Type
                , const TString&            strParm1Name
                , const tCQCKit::ECmdPTypes eParm2Type
                , const TString&            strParm2Name
                , const tCQCKit::ECmdPTypes eParm3Type
                , const TString&            strParm3Name
                , const tCQCKit::ECmdPTypes eParm4Type
                , const TString&            strParm4Name) :

    m_c4MaxParms(4)
    , m_colParms(4)
    , m_eType(tCQCKit::ECmdTypes::SideEffect)
    , m_strId(strId)
    , m_strName(strName)
{
    m_colParms.objPlace(strParm1Name, eParm1Type);
    m_colParms.objPlace(strParm2Name, eParm2Type);
    m_colParms.objPlace(strParm3Name, eParm3Type);
    m_colParms.objPlace(strParm4Name, eParm4Type);
}

TCQCCmd::~TCQCCmd()
{
}


// ---------------------------------------------------------------------------
//  TCQCCmd: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get the count of parameters this command needs
tCIDLib::TCard4 TCQCCmd::c4ParmCnt() const
{
    return m_c4MaxParms;
}


//
//  Adds a new parameter as long as we've not maxed out the number that we
//  were told in the ctor would be the parameter count.
//
TCQCCmdParm& TCQCCmd::cmdpAddParm(const TCQCCmdParm& cmdpToAdd)
{
    const tCIDLib::TCard4 c4CurCnt = m_colParms.c4ElemCount();
    if (c4CurCnt >= m_c4MaxParms)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_BadParmCnt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutResource
            , TCardinal(c4CurCnt)
        );
    }
    return m_colParms.objAdd(cmdpToAdd);
}


// Get a const/non-const ref to the parameter at the indicated index
const TCQCCmdParm& TCQCCmd::cmdpAt(const tCIDLib::TCard4 c4At) const
{
    if (c4At >= m_colParms.c4ElemCount())
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_BadParmIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , m_strName
            , TCardinal(m_colParms.c4ElemCount())
            , TCardinal(c4At)
        );
    }
    return m_colParms[c4At];
}


TCQCCmdParm& TCQCCmd::cmdpAt(const tCIDLib::TCard4 c4At)
{
    if (c4At >= m_colParms.c4ElemCount())
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_BadParmIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , m_strName
            , TCardinal(m_colParms.c4ElemCount())
            , TCardinal(c4At)
        );
    }
    return m_colParms[c4At];
}


// Get/set the command type flag
tCQCKit::ECmdTypes TCQCCmd::eType() const
{
    return m_eType;
}

tCQCKit::ECmdTypes TCQCCmd::eType(const tCQCKit::ECmdTypes eToSet)
{
    m_eType = eToSet;
    return m_eType;
}


// Get the unqiue command id
const TString& TCQCCmd::strId() const
{
    return m_strId;
}


// Get the human readable command name
const TString& TCQCCmd::strName() const
{
    return m_strName;
}


// Set up the command, and indicate how many parms
tCIDLib::TVoid TCQCCmd::Set(const   TString&            strId
                            , const TString&            strName
                            , const tCIDLib::TCard4     c4ParmCnt)
{
    m_c4MaxParms = c4ParmCnt;
    m_colParms.Reallocate(c4ParmCnt, kCIDLib::False);
    m_strId     = strId;
    m_strName   = strName;
}


// Change just the name/id of this command. Keep any existing type, parms, etc...
tCIDLib::TVoid TCQCCmd::Set(const   TString&    strId
                            , const TString&    strName)
{
    m_strId     = strId;
    m_strName   = strName;
}


//
//  Set a command paraemter at a given index. The index must be valid
//  for the number of parameters this command was set up for.
//
tCIDLib::TVoid TCQCCmd::SetParmAt(  const   tCIDLib::TCard4 c4At
                                    , const TCQCCmdParm&    cmdpToSet)
{
    if (c4At >= m_colParms.c4ElemCount())
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_BadParmIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strName
            , TCardinal(m_colParms.c4ElemCount())
            , TCardinal(c4At)
        );
    }
    m_colParms[c4At] = cmdpToSet;
}


tCIDLib::TVoid TCQCCmd::SetParmReqAt(const  tCIDLib::TCard4     c4At
                                    , const tCIDLib::TBoolean   bToSet)
{
    if (c4At >= m_colParms.c4ElemCount())
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_BadParmIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strName
            , TCardinal(m_colParms.c4ElemCount())
            , TCardinal(c4At)
        );
    }
    m_colParms[c4At].bRequired(bToSet);
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCCmdCfg
//  PREFIX: ccfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//   CLASS: TParmInfo
//  PREFIX: pi
//
//  A little class that comprises a single parameter in our list
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCCmdCfg:TParmInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCCmdCfg::TParmInfo::TParmInfo() :

    m_eType(tCQCKit::ECmdPTypes::None)
{
}

TCQCCmdCfg::TParmInfo::TParmInfo(const  tCQCKit::ECmdPTypes eType
                                , const TString&            strValue) :
    m_eType(eType)
    , m_strValue(strValue)
{
}

TCQCCmdCfg::TParmInfo::~TParmInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCCmdCfg::TParmInfo: Constructors and Destructor
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCCmdCfg::TParmInfo::bVal() const
{
    return facCQCKit().bCheckBoolVal(m_strValue);
}


tCIDLib::TCard4
TCQCCmdCfg::TParmInfo::c4Val(const tCIDLib::ERadices eRadix) const
{
    return m_strValue.c4Val(eRadix);
}


tCIDLib::TInt4
TCQCCmdCfg::TParmInfo::i4Val(const tCIDLib::ERadices eRadix) const
{
    return m_strValue.i4Val(eRadix);
}


tCIDLib::TVoid TCQCCmdCfg::TParmInfo::Reset()
{
    m_eType = tCQCKit::ECmdPTypes::None;
    m_strValue.Clear();
}



// ---------------------------------------------------------------------------
//  TCQCCmdCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCCmdCfg::TCQCCmdCfg() :

    m_c4ParmCnt(0)
    , m_c4TargetId(0)
    , m_colParms(kCQCKit::c4MaxCmdParms)
{
}

//
//  In a couple of places we need to kind of fake in some commands that don't
//  have to be fully set up, just enough to get the command dispatched.
//
TCQCCmdCfg::TCQCCmdCfg( const   TString&            strCmdId
                        , const TString&            strCmdName
                        , const TString&            strTargetId
                        , const TString&            strTargetName) :

    m_c4ParmCnt(0)
    , m_c4TargetId(0)
    , m_colParms(kCQCKit::c4MaxCmdParms)
    , m_strCmdId(strCmdId)
    , m_strName(strCmdName)
    , m_strTargetId(strTargetId)
    , m_strTargetName(strTargetName)
{
}

TCQCCmdCfg::TCQCCmdCfg(const TCQCCmd& cmdSrc) :

    m_c4ParmCnt(cmdSrc.c4ParmCnt())
    , m_c4TargetId(0)
    , m_colParms(kCQCKit::c4MaxCmdParms)
    , m_strCmdId(cmdSrc.strId())
    , m_strName(cmdSrc.strName())
{
}

TCQCCmdCfg::~TCQCCmdCfg()
{
}


// ---------------------------------------------------------------------------
//  TCQCCmdCfg: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCCmdCfg::operator==(const TCQCCmdCfg& ccfgSrc) const
{
    if (this != &ccfgSrc)
    {
        if ((m_c4ParmCnt     != ccfgSrc.m_c4ParmCnt)
        ||  (m_c4TargetId    != ccfgSrc.m_c4TargetId)
        ||  (m_strCmdId      != ccfgSrc.m_strCmdId)
        ||  (m_strName       != ccfgSrc.m_strName)
        ||  (m_strTargetId   != ccfgSrc.m_strTargetId)
        ||  (m_strTargetName != ccfgSrc.m_strTargetName))
        {
            return kCIDLib::False;
        }

        const tCIDLib::TCard4 c4Count = m_colParms.tElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if ((m_colParms[c4Index].m_eType != ccfgSrc.m_colParms[c4Index].m_eType)
            ||  (m_colParms[c4Index].m_strValue != ccfgSrc.m_colParms[c4Index].m_strValue))
            {
                return kCIDLib::False;
            }
        }
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TCQCCmdCfg::operator!=(const TCQCCmdCfg& ccfgSrc) const
{
    return !operator==(ccfgSrc);
}


// ---------------------------------------------------------------------------
//  TCQCCmdCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We search through our configured parameter values and do a search and
//  replace. We return true if we actually changed anything, so the caller
//  can only update any persisted stuff if things actually change.
//
//  To avoid creating the regular expression over and over, we get a reg
//  ex engine, and just ignore it if not in reg ex mode.
//
tCIDLib::TBoolean
TCQCCmdCfg::bSearchNReplace(const   TString&            strFind
                            , const TString&            strRepStr
                            , const tCIDLib::TBoolean   bRegEx
                            , const tCIDLib::TBoolean   bFullMatch
                            ,       TRegEx&             regxFind)
{
    tCIDLib::TBoolean bChanges = kCIDLib::False;

    // For each parameter, do the check and update
    for (tCIDLib::TCard4 c4PInd = 0; c4PInd < m_c4ParmCnt; c4PInd++)
    {
        TParmInfo& piCur = piAt(c4PInd);
        bChanges |= facCQCKit().bDoSearchNReplace
        (
            strFind, strRepStr, piCur.m_strValue, bRegEx, bFullMatch, regxFind
        );
    }
    return bChanges;
}


// Get or set the parameter count
tCIDLib::TCard4 TCQCCmdCfg::c4ParmCnt() const
{
    return m_c4ParmCnt;
}

tCIDLib::TCard4 TCQCCmdCfg::c4ParmCnt(const tCIDLib::TCard4 c4ToSet)
{
    // If debugging, make sure the number of parameters is valid
    #if CID_DEBUG_ON
    if (c4ToSet > kCQCKit::c4MaxCmdParms)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_BadParmCnt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TCardinal(c4ToSet)
        );
    }
    #endif

    m_c4ParmCnt = c4ToSet;
    return m_c4ParmCnt;
}


// Get/set the target id
tCIDLib::TCard4 TCQCCmdCfg::c4TargetId() const
{
    return m_c4TargetId;
}

tCIDLib::TCard4 TCQCCmdCfg::c4TargetId(const tCIDLib::TCard4 c4ToSet)
{
    m_c4TargetId = c4ToSet;
    return m_c4TargetId;
}


//
//  Formats our info out in a long format, where we try to show as much of
//  the parameter values as possible, so we show them on separate lines under
//  the main target::method value.
//
tCIDLib::TVoid
TCQCCmdCfg::FormatLongText(         TString&            strToFill
                            , const tCIDLib::TBoolean   bAppend) const
{
    if (!bAppend)
        strToFill.Clear();

    strToFill.Append(m_strTargetName);
    strToFill.Append(L"::");
    strToFill.Append(m_strName);
    strToFill.Append(L"\n");
    if (m_c4ParmCnt)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4ParmCnt; c4Index++)
        {
            const TString& strCur = m_colParms[c4Index].m_strValue;

            strToFill.Append(L"    P");
            strToFill.AppendFormatted(c4Index + 1);
            strToFill.Append(L"=");
            strToFill.Append(strCur);
            strToFill.Append(L"\n");
        }
    }
}


//
//  Formats our info in a method acll format, with truncated parameter values
//  if they are fairly long. Used for one line per command type displays such
//  as the action editor.
//
tCIDLib::TVoid
TCQCCmdCfg::FormatShortText(        TString&            strToFill
                            , const tCIDLib::TBoolean   bAppend) const
{
    if (!bAppend)
        strToFill.Clear();

    strToFill.Append(m_strTargetName);
    strToFill.Append(L"::");
    strToFill.Append(m_strName);
    strToFill.Append(L"(");
    if (m_c4ParmCnt)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4ParmCnt; c4Index++)
        {
            const TString& strCur = m_colParms[c4Index].m_strValue;

            //
            //  If longer than 32 chars, just chop it off at 29, and ellipsis
            //  it.
            //
            if (strCur.c4Length() > 32)
            {
                strToFill.AppendSubStr(strCur, 0, 29);
                strToFill.Append(L"...");
            }
             else
            {
                strToFill.Append(strCur);
            }
            if (c4Index + 1 < m_c4ParmCnt)
                strToFill.Append(L", ");
        }
    }

    strToFill.Append(L") ");
}


const TCQCCmdCfg::TParmInfo& TCQCCmdCfg::piAt(const tCIDLib::TCard4 c4At) const
{
    CheckParmIndex(c4At, CID_LINE);
    return m_colParms[c4At];
}

TCQCCmdCfg::TParmInfo& TCQCCmdCfg::piAt(const tCIDLib::TCard4 c4At)
{
    CheckParmIndex(c4At, CID_LINE);
    return m_colParms[c4At];
}


// Just resets this guy back to a non-configured state
tCIDLib::TVoid TCQCCmdCfg::Reset()
{
    m_c4ParmCnt  = 0;
    m_strCmdId.Clear();
    m_strName.Clear();
    m_strTargetId.Clear();
    m_strTargetName.Clear();
}


// Get/set the command id for this command
const TString& TCQCCmdCfg::strCmdId() const
{
    return m_strCmdId;
}

const TString& TCQCCmdCfg::strCmdId(const TString& strToSet)
{
    m_strCmdId = strToSet;
    return m_strCmdId;
}


// Get/Set the name of this command
const TString& TCQCCmdCfg::strName() const
{
    return m_strName;
}

const TString& TCQCCmdCfg::strName(const TString& strToSet)
{
    m_strName = strToSet;
    return m_strName;
}


// Get or set the target id
const TString& TCQCCmdCfg::strTargetId() const
{
    return m_strTargetId;
}

const TString& TCQCCmdCfg::strTargetId(const TString& strToSet)
{
    m_strTargetId = strToSet;
    return m_strTargetId;
}


// Get or set the target name for this command
const TString& TCQCCmdCfg::strTargetName() const
{
    return m_strTargetName;
}

const TString& TCQCCmdCfg::strTargetName(const TString& strToSet)
{
    m_strTargetName = strToSet;
    return m_strTargetName;
}


// Reset ourselfs to hold config for the passed command
tCIDLib::TVoid TCQCCmdCfg::SetFrom( const   TCQCCmd&                cmdSrc
                                    , const MCQCCmdSrcIntf&
                                    , const MCQCCmdTarIntf* const   pmctarInit)
{
    m_c4ParmCnt = cmdSrc.c4ParmCnt();
    m_strCmdId  = cmdSrc.strId();
    m_strName   = cmdSrc.strName();

    //
    //  For any parms that are defined, provide defaults for those that
    //  need them, else clear the value.
    ///
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < m_c4ParmCnt; c4Index++)
    {
        TParmInfo& piVal = m_colParms[c4Index];
        piVal.m_eType = cmdSrc.cmdpAt(c4Index).eType();

        // Set a default value on some
        switch(piVal.m_eType)
        {
            case tCQCKit::ECmdPTypes::Area :
                // Set it to an empty area at the origin
                piVal.m_strValue = L"0 0 0 0";
                break;

            case tCQCKit::ECmdPTypes::Boolean :
                piVal.m_strValue = kCQCKit::pszFld_False;
                break;

            case tCQCKit::ECmdPTypes::Color :
            {
                piVal.m_strValue = L"255 255 255";
                break;
            }

            case tCQCKit::ECmdPTypes::Enum :
                // Take the first enum value, the internal value, not the display value
                cmdSrc.cmdpAt(c4Index).QueryDefEnumVal(piVal.m_strValue, kCIDLib::False);
                break;

            case tCQCKit::ECmdPTypes::Point :
                // Set it to a point at the origin
                piVal.m_strValue = L"0 0";
                break;

            case tCQCKit::ECmdPTypes::RegXText :
                piVal.m_strValue = L".*";
                break;

            case tCQCKit::ECmdPTypes::Signed :
            case tCQCKit::ECmdPTypes::Unsigned :
            case tCQCKit::ECmdPTypes::Number :
                piVal.m_strValue = L"0";
                break;

            default :
                // Nothing special, so just clear it
                piVal.m_strValue.Clear();
                break;
        };
    }

    // If we got a target, then set us up for that target
    if (pmctarInit)
        SetTarget(*pmctarInit);

    // And for the rest, just reset them to unused values
    for (; c4Index < kCQCKit::c4MaxCmdParms; c4Index++)
        m_colParms[c4Index].Reset();
}


// Set the parameter value at the indicated index
tCIDLib::TVoid
TCQCCmdCfg::SetParmAt(  const   tCIDLib::TCard4     c4At
                        , const TString&            strToSet
                        , const tCQCKit::ECmdPTypes eType)
{
    CheckParmIndex(c4At, CID_LINE);
    m_colParms[c4At].m_eType = eType;
    m_colParms[c4At].m_strValue = strToSet;
}

tCIDLib::TVoid
TCQCCmdCfg::SetParmAt(  const   tCIDLib::TCard4     c4At
                        , const TString&            strToSet)
{
    CheckParmIndex(c4At, CID_LINE);
    m_colParms[c4At].m_strValue = strToSet;
}


tCIDLib::TVoid
TCQCCmdCfg::SetParmAtAsRTV( const   tCIDLib::TCard4 c4At
                            , const TString&        strRTVToSet)
{
    CheckParmIndex(c4At, CID_LINE);
    m_colParms[c4At].m_strValue = L"%(";
    m_colParms[c4At].m_strValue.Append(strRTVToSet);
    m_colParms[c4At].m_strValue.Append(L")");
}


tCIDLib::TVoid TCQCCmdCfg::SetTarget(const MCQCCmdTarIntf& mctarNew)
{
    m_strTargetId   = mctarNew.strCmdTargetId();
    m_strTargetName = mctarNew.strCmdTargetName();
}


// Format our info in XML to the target stream
tCIDLib::TVoid TCQCCmdCfg::ToXML(TTextOutStream& strmTarget) const
{
    //
    //  Format out the open element, which has everything but the parm
    //  info. Have to escape the target, just in case.
    //
    strmTarget  << L"\n<Cmd PCnt=\"" << m_c4ParmCnt << L"\" TarId=\""
                << m_strTargetId << L"\" TarName=\"";
    facCIDXML().EscapeFor(m_strTargetName, strmTarget, tCIDXML::EEscTypes::Attribute);
    strmTarget  << L"\" Id=\"" << m_strCmdId << L"\">";

    // And now format the parameter info
    {
        TStreamIndentJan janIndent(&strmTarget, 4);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4ParmCnt; c4Index++)
        {
            const TParmInfo& piCur = m_colParms[c4Index];

            strmTarget  << L"\n<Parm Val=\"";
            facCIDXML().EscapeFor(piCur.m_strValue, strmTarget, tCIDXML::EEscTypes::Attribute);
            strmTarget  << L"\" Type=\""
                        << tCQCKit::strXlatECmdPTypes(piCur.m_eType)
                        << L"\"/>";
        }
    }

    // And close out our element now
    strmTarget << L"\n</Cmd>";
}


// ---------------------------------------------------------------------------
//  TCQCCmdCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCCmdCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff must start with an object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version for this level
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion > CQCKit_CmdIntf::c2CfgVersionFmt)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_strCmdId
                    >> m_c4ParmCnt
                    >> m_strName
                    >> m_strTargetId
                    >> m_strTargetName;

    // Stream the parameters in
    TString strVal;
    tCQCKit::ECmdPTypes eType;
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < m_c4ParmCnt; c4Index++)
    {
        strmToReadFrom  >> eType
                        >> strVal;

        TParmInfo& piNew = m_colParms[c4Index];
        piNew.m_eType = eType;
        piNew.m_strValue = strVal;
    }

    // Reset the rest to defaults
    for (; c4Index < kCQCKit::c4MaxCmdParms; c4Index++)
        m_colParms[c4Index].Reset();
    m_c4TargetId = 0;

    // And our stuff must end with a marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCCmdCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_CmdIntf::c2CfgVersionFmt
                    << m_strCmdId
                    << m_c4ParmCnt
                    << m_strName
                    << m_strTargetId
                    << m_strTargetName;

    // Stream out the parameter info
    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4ParmCnt; c4Index++)
    {
        const TParmInfo& piCur = m_colParms[c4Index];
        strmToWriteTo   << piCur.m_eType
                        << piCur.m_strValue;
    }
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCCmdCfg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// A helper to validate a provided parameter index and throw if bad
tCIDLib::TVoid
TCQCCmdCfg::CheckParmIndex( const   tCIDLib::TCard4 c4ToCheck
                            , const tCIDLib::TCard4 c4Line) const
{
    if (c4ToCheck >= m_c4ParmCnt)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , c4Line
            , kKitErrs::errcCmd_BadParmIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strCmdId
            , TCardinal(m_c4ParmCnt)
            , TCardinal(c4ToCheck)
        );
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TActOpcode
//  PREFIX: aoc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TActOpcode: Constructors and Destructor
// ---------------------------------------------------------------------------
TActOpcode::TActOpcode() :

    m_bDisabled(kCIDLib::False)
    , m_bNegate(kCIDLib::False)
    , m_c4Id(kCIDLib::c4MaxCard)
    , m_i4Ofs(kCIDLib::i4MaxInt)
    , m_eOpcode(tCQCKit::EActOps::Cmd)
{
}

TActOpcode::TActOpcode(const tCQCKit::EActOps eOpcode) :

    m_bDisabled(kCIDLib::False)
    , m_bNegate(kCIDLib::False)
    , m_c4Id(kCIDLib::c4MaxCard)
    , m_i4Ofs(kCIDLib::i4MaxInt)
    , m_eOpcode(eOpcode)
{
}

TActOpcode::~TActOpcode()
{
}


// ---------------------------------------------------------------------------
//  MCQCCmdSrcIntf::TActOpcode: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TActOpcode::operator==(const TActOpcode& aocSrc) const
{
    // The id member is purely an editing thing so we don't count it here
    if (this != &aocSrc)
    {
        if ((m_bDisabled != aocSrc.m_bDisabled)
        ||  (m_bNegate   != aocSrc.m_bNegate)
        ||  (m_i4Ofs     != aocSrc.m_i4Ofs)
        ||  (m_eOpcode   != aocSrc.m_eOpcode)
        ||  (m_ccfgStep  != aocSrc.m_ccfgStep))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TActOpcode::operator!=(const TActOpcode& aocSrc) const
{
    return !operator==(aocSrc);
}



// ---------------------------------------------------------------------------
//  TActOpcode: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Returns a boolean that indicates if this opcode is a conditional one
tCIDLib::TBoolean
TActOpcode::bIsConditional(const tCIDLib::TBoolean bStartOnly) const
{
    // For now the only starting conditional is the If
    if (bStartOnly)
        return (m_eOpcode == tCQCKit::EActOps::If);

    // Else it can be if, else, or end
    return ((m_eOpcode == tCQCKit::EActOps::If)
           || (m_eOpcode == tCQCKit::EActOps::Else)
           || (m_eOpcode == tCQCKit::EActOps::End));
}


//
//  A helper, mostly for the action trace interface handlers, that formats
//  out our contents in a long form, multi-line, no nesting level.
//
tCIDLib::TVoid
TActOpcode::FormatLongTo(       TString&            strToFill
                        , const tCIDLib::TBoolean   bAppend)
{
    if (!bAppend)
        strToFill.Clear();

    // If disabled, indicate that
    if (m_bDisabled)
        strToFill.Append(kCIDLib::chHyphenMinus);

    if (m_eOpcode == tCQCKit::EActOps::Else)
    {
        strToFill.Append(L"Else\n");
        return;
    }
     else if (m_eOpcode == tCQCKit::EActOps::End)
    {
        strToFill.Append(L"End\n");
        return;
    }

    // For one of the conditional statemnts, we need to format out that
    if (m_eOpcode == tCQCKit::EActOps::If)
    {
        strToFill.Append(L"If ");

        // If negated, output the not
        if (m_bNegate)
            strToFill.Append(L"Not ");

        strToFill.Append(L"\n");
    }

    if (m_eOpcode == tCQCKit::EActOps::Comment)
    {
        //
        //  The 0th parm of the (otherwise unused in this case) config object
        //  is used to hold the comment text.
        //
        strToFill.Append(L"// ");
        strToFill.Append(m_ccfgStep.piAt(0).m_strValue);
    }
     else
    {
        //
        //  Ask the config object to format out itself long form, whatever
        //  the append flag comming in, for us we want this guy to append.
        //
        m_ccfgStep.FormatLongText(strToFill, kCIDLib::True);
    }
}


//
//  A helper method for folks who want to format out opcodes and their
//  commands. We support both flat and conditional nesting formats.
//
tCIDLib::TVoid
TActOpcode::FormatTo(       TString&            strToFill
                    , const tCIDLib::TCard4     c4Nesting
                    , const tCIDLib::TBoolean   bFlat
                    , const tCIDLib::TBoolean   bAppend) const
{
    if (!bAppend)
        strToFill.Clear();

    // If not flat, generate the indentation
    if (!bFlat)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Nesting; c4Index++)
            strToFill.Append(L"   ");
    }

    // If disabled indicate that
    if (m_bDisabled)
        strToFill.Append(kCIDLib::chHyphenMinus);

    //
    //  If it's a conditional, we need to do a little prep work on it, to
    //  indicate the conditional statement. If it's an Else or End, we can
    //  just put out Else or End and we are done.
    //
    if (m_eOpcode == tCQCKit::EActOps::Else)
    {
        strToFill.Append(L"Else");
        return;
    }
     else if (m_eOpcode == tCQCKit::EActOps::End)
    {
        strToFill.Append(L"End");
        return;
    }

    // For one of the conditional statemnts, we need to format out that
    if (m_eOpcode == tCQCKit::EActOps::If)
    {
        strToFill.Append(L"If ");

        // If negated, output the not
        if (m_bNegate)
            strToFill.Append(L"Not ");
    }

    //
    //  Ask the command to format itself at the end now, unless it's a comment,
    //  in which case we just handle it ourself.
    //
    if (m_eOpcode == tCQCKit::EActOps::Comment)
    {
        //
        //  The 0th parm of the (otherwise unused in this case) config object
        //  is used to hold the comment text.
        //
        strToFill.Append(L"// ");
        strToFill.Append(m_ccfgStep.piAt(0).m_strValue);
    }
     else
    {
        // Whatever the append setting we get, we want this guy to append
        m_ccfgStep.FormatShortText(strToFill, kCIDLib::True);
    }
}


//
//  Formats out just the prefix stuff, that is part of the opcode, not part
//  of the command config.
//
tCIDLib::TVoid
TActOpcode::FormatPrefixTo(         TString&            strToFill
                            , const tCIDLib::TBoolean   bAppend) const
{
    if (!bAppend)
        strToFill.Clear();

    //
    //  If it's a conditional, we need to do a little prep work on it, to
    //  indicate the conditional statement. If it's an Else or End, we can
    //  just put out Else or End and we are done.
    //
    if (m_eOpcode == tCQCKit::EActOps::Else)
    {
        strToFill.Append(L"Else");
        return;
    }
     else if (m_eOpcode == tCQCKit::EActOps::End)
    {
        strToFill.Append(L"End");
        return;
    }

    // For one of the conditional statemnts, we need to format out that
    if (m_eOpcode == tCQCKit::EActOps::If)
    {
        strToFill.Append(L"If ");

        // If negated, output the not
        if (m_bNegate)
            strToFill.Append(L"Not ");
    }
}


tCIDLib::TVoid TActOpcode::Reset(const tCQCKit::EActOps eOpcode)
{
    m_bDisabled = kCIDLib::False;
    m_bNegate = kCIDLib::False;
    m_c4Id = kCIDLib::c4MaxCard;
    m_i4Ofs = kCIDLib::i4MaxInt;
    m_eOpcode = eOpcode;
    m_ccfgStep.Reset();
}


// ---------------------------------------------------------------------------
//  TActOpcode: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TActOpcode::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff must start with an object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version for this level
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_CmdIntf::c2OpVersionFmt))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_eOpcode
                    >> m_bNegate
                    >> m_i4Ofs
                    >> m_ccfgStep;

    // If v2, read in the disabled flag, else default it
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_bDisabled;
    else
        m_bDisabled = kCIDLib::False;

    // Reset our id
    m_c4Id = kCIDLib::c4MaxCard;

    // And our stuff must end with a marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TActOpcode::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_CmdIntf::c2OpVersionFmt
                    << m_eOpcode
                    << m_bNegate
                    << m_i4Ofs
                    << m_ccfgStep
                    << m_bDisabled
                    << tCIDLib::EStreamMarkers::Frame;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCActEvInfo
//  PREFIX: caei
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCActEvInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCActEvInfo::TCQCActEvInfo() :

    m_eContext(tCQCKit::EActCmdCtx::Standard)
{
}

TCQCActEvInfo::TCQCActEvInfo(const  TString&            strName
                            , const TString&            strEventId
                            , const tCQCKit::EActCmdCtx eContext) :
    m_eContext(eContext)
    , m_strEventId(strEventId)
    , m_strName(strName)
{
}

// ---------------------------------------------------------------------------
//  TCQCActEvInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCActEvInfo::operator==(const TCQCActEvInfo& caeiSrc) const
{
    if (this != &caeiSrc)
    {
        if ((m_eContext   != caeiSrc.m_eContext)
        ||  (m_strEventId != caeiSrc.m_strEventId)
        ||  (m_strName    != caeiSrc.m_strName))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TCQCActEvInfo::operator!=(const TCQCActEvInfo& caeiSrc) const
{
    return !operator==(caeiSrc);
}


// ---------------------------------------------------------------------------
//  TCQCActEvInfo: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCActEvInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff must start with an object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version for this level
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_CmdIntf::c2ActEvInfoVersionFmt))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_strName
                    >> m_eContext
                    >> m_strEventId;

    // And our stuff must end with a marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCActEvInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_CmdIntf::c2ActEvInfoVersionFmt
                    << m_strName
                    << m_eContext
                    << m_strEventId
                    << tCIDLib::EStreamMarkers::Frame;
}



// ---------------------------------------------------------------------------
//   CLASS: MCQCCmdSrcIntf
//  PREFIX: mcsrc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MCQCCmdSrcIntf: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 MCQCCmdSrcIntf::c4StdRTVCount()
{
    // If we've not faulted in the runtime values list, then do that.
    if (!CQCKit_CmdIntf::atomRTValsLoaded)
    {
        TBaseLock lockInit;
        if (!CQCKit_CmdIntf::atomRTValsLoaded)
            LoadRTVList();
    }

    return CQCKit_CmdIntf::colRTVs.c4ElemCount();
}


// ---------------------------------------------------------------------------
//  MCQCCmdSrcIntf: Constructors and Destructor
// ---------------------------------------------------------------------------
MCQCCmdSrcIntf::MCQCCmdSrcIntf() :

    m_colEventOps()
{
    //
    //  Load up the list of standard RTVs we support from the global list. The
    //  method that we call to get the count will force the list in if it has
    //  not been yet.
    //
    const tCIDLib::TCard4 c4RTVCnt = c4StdRTVCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RTVCnt; c4Index++)
    {
        const CQCKit_CmdIntf::TRTValItem& itemCur = CQCKit_CmdIntf::colRTVs[c4Index];
        m_colRTVals.objPlace(itemCur.m_strText, itemCur.m_strId, itemCur.m_eType);
    }
}

MCQCCmdSrcIntf::~MCQCCmdSrcIntf()
{
}


// ---------------------------------------------------------------------------
//  MCQCCmdSrcIntf: Public operators
// ---------------------------------------------------------------------------

//
//  Does a search and replace for all of the opcodes in all of the events
//  of this command source. We have various classes that derive from us,
//  and they may need to do their own handling of this, we we make it
//  virtual.
//
tCIDLib::TVoid
MCQCCmdSrcIntf::CmdSearchNReplace(  const   TString&            strFind
                                    , const TString&            strReplaceWith
                                    , const tCIDLib::TBoolean   bRegEx
                                    , const tCIDLib::TBoolean   bFullMatch
                                    ,       TRegEx&             regxFind)
{
    const tCIDLib::TCard4 c4EventCnt = m_colEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4EventCnt; c4Index++)
    {
        TCQCActEvInfo& caeiCur = m_colEvents[c4Index];

        // Get the opcode list
        MCQCCmdSrcIntf::TOpcodeBlock* pcolOps = pcolOpsForEvent(caeiCur.m_strEventId);

        // For each opcode, ask it to do a replace
        const tCIDLib::TCard4 c4OpCnt = pcolOps->c4ElemCount();
        for (tCIDLib::TCard4 c4OpInd = 0; c4OpInd < c4OpCnt; c4OpInd++)
        {
            TActOpcode& aocCur = pcolOps->objAt(c4OpInd);
            aocCur.m_ccfgStep.bSearchNReplace
            (
                strFind, strReplaceWith, bRegEx, bFullMatch, regxFind
            );
        }
    }
}



//
//  If the derived class has no special RTVs of its own, we provide the default
//  action and just return a standard one.
//
TCQCCmdRTVSrc* MCQCCmdSrcIntf::pcrtsMakeNew(const TCQCUserCtx& cuctxUser) const
{
    return new TCQCCmdRTVSrc(cuctxUser);
}


// ---------------------------------------------------------------------------
//  MCQCCmdSrcIntf: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
MCQCCmdSrcIntf::AddEvent(const  TString&            strName
                        , const TString&            strId
                        , const tCQCKit::EActCmdCtx eContext)
{
    // Add a key/value pair to the events list
    m_colEvents.objPlace(strName, strId, eContext);

    // And put an entry into the event op list
    m_colEventOps.objAdd(TOpcodeBlock());
}


// Add's a runtime value definition to our list
tCIDLib::TVoid
MCQCCmdSrcIntf::AddRTValue( const   TString&            strName
                            , const TString&            strId
                            , const tCQCKit::ECmdPTypes eType)
{
    m_colRTVals.objPlace(strName, strId, eType);
}


//
//  Return a reference to the indicated command. We just let the collection
//  handle index errors.
//
const TActOpcode&
MCQCCmdSrcIntf::aocAt(  const   TString&        strEvent
                        , const tCIDLib::TCard4 c4At) const
{
    // See if this event is in our list
    tCIDLib::TCard4 c4Index;
    pcaeiFindEv(strEvent, c4Index, kCIDLib::True);
    return m_colEventOps[c4Index][c4At];
}

TActOpcode& MCQCCmdSrcIntf::aocAt(  const   TString&        strEvent
                                    , const tCIDLib::TCard4 c4At)
{
    // See if this event is in our list
    tCIDLib::TCard4 c4Index;
    pcaeiFindEv(strEvent, c4Index, kCIDLib::True);
    return m_colEventOps[c4Index][c4At];
}


// Sees if this guy has at least one opcode defined for at least one event
tCIDLib::TBoolean MCQCCmdSrcIntf::bHasAnyOps() const
{
    const tCIDLib::TCard4 c4Count = m_colEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (!m_colEventOps[c4Index].bIsEmpty())
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Returns a copy of the opcodes for a given event. Retursn true if there
//  were any, else false. This is the raw list, unresolved.
//
tCIDLib::TBoolean MCQCCmdSrcIntf::
bQueryOpsForEvent(  const   TString&                        strEventId
                    ,       MCQCCmdSrcIntf::TOpcodeBlock&   colToFill) const
{
    colToFill.RemoveAll();

    // Find the commands for this event
    tCIDLib::TCard4 c4Index;
    if (!pcaeiFindEv(strEventId, c4Index, kCIDLib::False))
        return kCIDLib::False;

    if (m_colEventOps[c4Index].bIsEmpty())
        return kCIDLib::False;

    colToFill = m_colEventOps[c4Index];
    return kCIDLib::True;
}


// Sees if any commands in this source references the target id
tCIDLib::TBoolean MCQCCmdSrcIntf::bReferencesTarId(const TString& strTarId) const
{
    const tCIDLib::TCard4 c4EvCount = m_colEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4EvIndex = 0; c4EvIndex < c4EvCount; c4EvIndex++)
    {
        const TOpcodeBlock& colOps = m_colEventOps[c4EvIndex];
        const tCIDLib::TCard4 c4OpCount = colOps.c4ElemCount();

        for (tCIDLib::TCard4 c4OpIndex = 0; c4OpIndex < c4OpCount; c4OpIndex++)
        {
            if (colOps[c4OpIndex].m_ccfgStep.strTargetId() == strTarId)
                return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


// Returns the runtime value definition with the indicated id
tCIDLib::TBoolean MCQCCmdSrcIntf::bRemoveRTValue(const TString& strId)
{
    const tCIDLib::TCard4 c4Count = m_colRTVals.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        if (m_colRTVals[c4Index].strId() == strId)
        {
            m_colRTVals.RemoveAt(c4Index);
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


// Checks to see if a RTV with the indicated id exists
tCIDLib::TBoolean MCQCCmdSrcIntf::bRTValExists(const TString& strId) const
{
    // Run through the list and find the indicated id
    const tCIDLib::TCard4 c4Count = m_colRTVals.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        if (m_colRTVals[c4Index].strId() == strId)
            break;
    }
    return (c4Index != c4Count);
}


// Check to see if our command setup is the same as the source's
tCIDLib::TBoolean
MCQCCmdSrcIntf::bSameCmds(const MCQCCmdSrcIntf& miwdgSrc) const
{
    const tCIDLib::TCard4 c4Count = m_colEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colEvents[c4Index] != miwdgSrc.m_colEvents[c4Index])
            return kCIDLib::False;

        // The event is the same, check the opcodes
        const TOpcodeBlock& colBlock = m_colEventOps[c4Index];
        const TOpcodeBlock& colSrcBlock = miwdgSrc.m_colEventOps[c4Index];

        const tCIDLib::TCard4 c4OpCnt = colBlock.c4ElemCount();
        if (c4OpCnt != colSrcBlock.c4ElemCount())
            return kCIDLib::False;

        // Each of these is itself a vector of opcodes
        for (tCIDLib::TCard4 c4OpInd = 0; c4OpInd < c4OpCnt; c4OpInd++)
        {
            if (colBlock[c4OpInd] != colSrcBlock[c4OpInd])
                return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  Go through all of the action events in the source and, for any that we have, we take
//  the sources opcodes for those events. We return whether we updated any events.
//
tCIDLib::TBoolean MCQCCmdSrcIntf::bTakeMatchingEvents(const MCQCCmdSrcIntf& miwdgSrc)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    const tCIDLib::TCard4 c4EvCount = miwdgSrc.m_colEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4EvIndex = 0; c4EvIndex < c4EvCount; c4EvIndex++)
    {
        const TCQCActEvInfo& caeiCurSrc = miwdgSrc.m_colEvents[c4EvIndex];

        // See if we have an event with the name of this source one
        TOpcodeBlock* pcolCurTar = pcolOpsForEvent(caeiCurSrc.m_strEventId);
        if (pcolCurTar)
        {
            // We have it so replace ours with his
            *pcolCurTar = m_colEventOps[c4EvIndex];
            bRet = kCIDLib::True;
        }
    }

    return bRet;
}


//
//  Finds any commands that are targeted towards the old command target id
//  and update them to reference the new target id. This allows for renaming
//  of non-standard (usually template widget) targets. In order to make sure
//  that any other widgets referencing the renamed target will work, we have
//  update those other widgets.
//
tCIDLib::TBoolean
MCQCCmdSrcIntf::bUpdateCmdTarget(const  TString&    strOldTarId
                                , const TString&    strNewTarName
                                , const TString&    strNewTarId)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;

    const tCIDLib::TCard4 c4EvCount = m_colEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4EvIndex = 0; c4EvIndex < c4EvCount; c4EvIndex++)
    {
        TOpcodeBlock& colOps = m_colEventOps[c4EvIndex];
        const tCIDLib::TCard4 c4OpCount = colOps.c4ElemCount();

        for (tCIDLib::TCard4 c4OpIndex = 0; c4OpIndex < c4OpCount; c4OpIndex++)
        {
            TActOpcode& aocCur = colOps[c4OpIndex];
            TCQCCmdCfg& ccfgCur = aocCur.m_ccfgStep;

            if (ccfgCur.strTargetId() == strOldTarId)
            {
                ccfgCur.strTargetName(strNewTarName);
                ccfgCur.strTargetId(strNewTarId);

                bRet = kCIDLib::True;
            }
        }
    }
    return bRet;
}


// Returns the count of events set on this command source
tCIDLib::TCard4 MCQCCmdSrcIntf::c4EventCount() const
{
    return m_colEvents.c4ElemCount();
}


// Returns the count of runtime value definitions we have
tCIDLib::TCard4 MCQCCmdSrcIntf::c4RTValCount() const
{
    return m_colRTVals.c4ElemCount();
}


// Returns a ref to the runtime value at the indicated index
const TCQCCmdRTVal& MCQCCmdSrcIntf::crtvAt(const tCIDLib::TCard4 c4At) const
{
    return m_colRTVals[c4At];
}


// Returns the count of command steps for the indicated event or at an index
tCIDLib::TCard4 MCQCCmdSrcIntf::c4OpCount(const TString& strEvent) const
{
    // Find the commands for this event
    tCIDLib::TCard4 c4Index;
    if (!pcaeiFindEv(strEvent, c4Index, kCIDLib::False))
        return 0;
    return m_colEventOps[c4Index].c4ElemCount();
}

tCIDLib::TCard4 MCQCCmdSrcIntf::c4OpCountAt(const tCIDLib::TCard4 c4At) const
{
    return m_colEventOps[c4At].c4ElemCount();
}


// Return the info for the event at the indicated index
const TCQCActEvInfo&
MCQCCmdSrcIntf::caeiEventAt(const tCIDLib::TCard4 c4At) const
{
    return m_colEvents[c4At];
}


// Finds a runtime value definition by id. Throws if not found
const TCQCCmdRTVal& MCQCCmdSrcIntf::crtvFind(const TString& strId) const
{
    // Run through the list and find the indicated id
    const tCIDLib::TCard4 c4Count = m_colRTVals.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        if (m_colRTVals[c4Index].strId() == strId)
            break;
    }

    if (c4Index == c4Count)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_RTValNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strId
        );
    }
    return m_colRTVals[c4Index];
}


// Delete an event event opcode list, if we find it
tCIDLib::TVoid MCQCCmdSrcIntf::DeleteEvent(const TString& strEventId)
{
    tCIDLib::TCard4 c4Index;
    if (pcaeiFindEv(strEventId, c4Index, kCIDLib::False))
        m_colEvents.RemoveAt(c4Index);
}


// Delete an event event opcode list, by index
tCIDLib::TVoid MCQCCmdSrcIntf::DeleteEventAt(const tCIDLib::TCard4 c4At)
{
    m_colEvents.RemoveAt(c4At);
}


// Return the action context configured for the events
tCQCKit::EActCmdCtx
MCQCCmdSrcIntf::eContextOfEvent(const TString& strEventId) const
{
    // Look for the event and return whatever i
    tCIDLib::TCard4 c4Index;
    const TCQCActEvInfo* pcaeiRet = pcaeiFindEv(strEventId, c4Index, kCIDLib::True);
    return pcaeiRet->m_eContext;
}


// Format out the opcodes for the indicated event
tCIDLib::TVoid
MCQCCmdSrcIntf::FormatOpsForEvent(  const   TString&        strEventId
                                    ,       TTextOutStream& strmToFill
                                    , const tCIDLib::TCard4 c4IndentOfs) const
{
    // Get the list of ops for the indicated event. If none, then we are done
    const TOpcodeBlock* pcolOps = pcolOpsForEvent(strEventId);
    if (!pcolOps)
        return;

    // See how many we have. If none, then we are done
    const tCIDLib::TCard4 c4OpCnt = pcolOps->c4ElemCount();
    if (!c4OpCnt)
        return;

    // Initialize the previous op to an invalid value!
    tCQCKit::EActOps ePrevOp = tCQCKit::EActOps::Count;
    tCIDLib::TCard4  c4Nesting = c4IndentOfs;
    TString          strFmt;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4OpCnt; c4Index++)
    {
        const TActOpcode& aocCur = pcolOps->objAt(c4Index);

        // Based on the previous state, deal with the current opcode
        const tCQCKit::EActOps eCurOp = pcolOps->objAt(c4Index).m_eOpcode;
        switch(ePrevOp)
        {
            case tCQCKit::EActOps::Comment :
            case tCQCKit::EActOps::Cmd :
                //
                //  If we see an End or Else after a command or comment,
                //  then the nesting goes down.
                //
                if ((eCurOp == tCQCKit::EActOps::End)
                ||  (eCurOp == tCQCKit::EActOps::Else))
                {
                    if (c4Nesting)
                        c4Nesting--;
                }
                break;

            case tCQCKit::EActOps::Else :
                //
                //  If we see a cmd, comment, or if after an else, then the
                //  nesting goes up.
                //
                if ((eCurOp == tCQCKit::EActOps::Cmd)
                ||  (eCurOp == tCQCKit::EActOps::Comment)
                ||  (eCurOp == tCQCKit::EActOps::If))
                {
                    c4Nesting++;
                }
                break;

            case tCQCKit::EActOps::End :
                // If we send an end or else after an end, then nesting goes down
                if ((eCurOp == tCQCKit::EActOps::End)
                ||  (eCurOp == tCQCKit::EActOps::Else))
                {
                    if (c4Nesting)
                        c4Nesting--;
                }
                break;

            case tCQCKit::EActOps::If :
                // If we see a cmd, comment, or If after an If, then it goes up
                if ((eCurOp == tCQCKit::EActOps::Cmd)
                ||  (eCurOp == tCQCKit::EActOps::Comment)
                ||  (eCurOp == tCQCKit::EActOps::If))
                {
                    c4Nesting++;
                }
                break;

            default :
                // Nothing to do here
                break;
        };

        // Now make the current opcode the previous op
        ePrevOp = eCurOp;

        // Format this guy and format it out
        aocCur.FormatTo(strFmt, c4Nesting);
        strmToFill << strFmt << kCIDLib::NewLn;
    }

    // Flush the stream
    strmToFill.Flush();
}


//
//  We give the caller back a list of the opcodes we have for the indicated
//  event id, with all the target ids looked up based on the passed list
//  of targets.
//
//  For any targets we don't find, we set the target ids to zero, which will
//  cause them to fail when the opcodes are invoked.
//
tCIDLib::TCard4 MCQCCmdSrcIntf::
c4GetResolvedOpList(const   tCQCKit::TCmdTarList&           colTargets
                    ,       MCQCCmdSrcIntf::TOpcodeBlock&   colOpsToFill
                    , const TString&                        strEventId) const
{
    const tCIDLib::TCard4 c4TarCnt = colTargets.c4ElemCount();
    colOpsToFill.RemoveAll();

    // Find the list for the indicated event
    tCIDLib::TCard4 c4OpInd;
    pcaeiFindEv(strEventId, c4OpInd, kCIDLib::True);

    // Copy the opcodes to the caller's list
    colOpsToFill = m_colEventOps[c4OpInd];
    const tCIDLib::TCard4 c4OpCount = colOpsToFill.c4ElemCount();

    // Process the commands for this event
    TString strTarId;
    for (tCIDLib::TCard4 c4OpIndex = 0; c4OpIndex < c4OpCount; c4OpIndex++)
    {
        TActOpcode& aocCur = colOpsToFill[c4OpIndex];
        TCQCCmdCfg& ccfgCur = aocCur.m_ccfgStep;

        //
        //  If the id is empty (shouldn't be, but we can't do anything
        //  about it here), then skip this one.
        //
        const TString& strId = ccfgCur.strTargetId();
        if (strId.bIsEmpty())
            continue;

        // There's an id, so see if we can find it in the list
        const MCQCCmdTarIntf* pmctarCur = 0;
        for (tCIDLib::TCard4 c4TInd = 0; c4TInd < c4TarCnt; c4TInd++)
        {
            if (colTargets[c4TInd]->strCmdTargetId() == strId)
            {
                pmctarCur =  colTargets[c4TInd];
                break;
            }
        }

        //
        //  If not found, there's nothing we can do but set the id to
        //  zero so that it won't be found when the command is run later.
        //
        if (pmctarCur)
            ccfgCur.c4TargetId(pmctarCur->c4UniqueId());
        else
            ccfgCur.c4TargetId(0);
    }
    return c4OpCount;
}


// Get the opcode block at a particular event index
const MCQCCmdSrcIntf::TOpcodeBlock&
MCQCCmdSrcIntf::colOpsAt(const tCIDLib::TCard4 c4At) const
{
    return m_colEventOps[c4At];
}

MCQCCmdSrcIntf::TOpcodeBlock& MCQCCmdSrcIntf::colOpsAt(const  tCIDLib::TCard4 c4At)
{
    return m_colEventOps[c4At];
}


//
// Gets the opcode block for a given event, one for const and one for non-const
//
const MCQCCmdSrcIntf::TOpcodeBlock*
MCQCCmdSrcIntf::pcolOpsForEvent(const TString& strEventId) const
{
    // Find the commands for this event
    tCIDLib::TCard4 c4Index;
    if (!pcaeiFindEv(strEventId, c4Index, kCIDLib::False))
        return 0;
    return &m_colEventOps[c4Index];
}

MCQCCmdSrcIntf::TOpcodeBlock*
MCQCCmdSrcIntf::pcolOpsForEvent(const TString& strEventId)
{
    // Find the commands for this event
    tCIDLib::TCard4 c4Index;
    if (!pcaeiFindEv(strEventId, c4Index, kCIDLib::False))
        return nullptr;
    return &m_colEventOps[c4Index];
}


// Finds a runtime value definition by id. Returns null if not found
const TCQCCmdRTVal* MCQCCmdSrcIntf::pcrtvFind(const TString& strId) const
{
    // Run through the list and find the indicated id
    const tCIDLib::TCard4 c4Count = m_colRTVals.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        if (m_colRTVals[c4Index].strId() == strId)
            break;
    }

    if (c4Index == c4Count)
        return nullptr;
    return &m_colRTVals[c4Index];
}


//
//  We use a binary type and just flatten our action info out to a binary buffer
//  that can be read back in later. We can optionally add a separator, though sometimes
//  the caller may just want to slot us unto an existing chunk of attributes.
//
tCIDLib::TVoid
MCQCCmdSrcIntf::QueryCmdAttrs(          tCIDMData::TAttrList&   colAttrs
                                ,       TAttrData&              adatTmp
                                , const tCIDLib::TBoolean       bAddSep
                                , const tCIDLib::TBoolean       bRequired
                                , const TString&                strItemText) const
{
    if (bAddSep)
    {
        colAttrs.objPlace
        (
            L"Action Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
        );
    }

    // This one we have to flatten out the run time values
    TBinMBufOutStream strmOut(2048UL);
    WriteOutOps(strmOut);

    //
    //  Write out the runtime value definitions as well. That's a runtime thing
    //  so the normal streaming doesn't use it. But for editing the action, we
    //  need it.
    //
    strmOut << m_colRTVals << tCIDLib::EStreamMarkers::Frame;
    strmOut.Flush();

    colAttrs.objPlace
    (
        strItemText
        , kCQCKit::strAttr_Actions
        , strmOut.mbufData()
        , strmOut.c4CurPos()
        , bRequired ? kCIDMData::strAttrLim_Required : TString::strEmpty()
        , tCIDMData::EAttrEdTypes::Visual
    );
}


// Stream outself in from the passed input stream
tCIDLib::TVoid MCQCCmdSrcIntf::ReadInOps(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;

    if (!c2FmtVersion || (c2FmtVersion > CQCKit_CmdIntf::c2SrcVersionFmt))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString(L"MCQCCmdSrcIntf")
        );
    }

    //
    //  Looks ok, so stream in our stuff. If V1, we have to read into a
    //  temp list for the events and convert them to the new format.
    //
    if (c2FmtVersion == 1)
    {
        tCIDLib::TKVPList colTmp;
        strmToReadFrom >> colTmp;

        m_colEvents.RemoveAll();
        const tCIDLib::TCard4 c4Count = colTmp.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TKeyValuePair& kvalCur = colTmp[c4Index];
            m_colEvents.objPlace
            (
                kvalCur.strKey()
                , kvalCur.strValue()
                , tCQCKit::EActCmdCtx::Standard

            );
        }
    }
     else
    {
        strmToReadFrom  >> m_colEvents;
    }

    // Now bring in the event ops
    strmToReadFrom >> m_colEventOps;

    // And it should all end with another frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    //
    //  Do any upgrading of commands that we need to do. This allows us to
    //  expand some commands dynamically. At the next major version release
    //  we generally invalidate any import of old packages and so this
    //  stuff gets written back out during the upgrade and we can dump these
    //  and start over for the next major release.
    //
    //  The installer reads all of the interfaces in and writes them back
    //  out, so it will always pick up any of these upgrades and commit them
    //  in the case of interfaces (the largest source of commands generally.)
    //
    TString strTmp;
    const tCIDLib::TCard4 c4EvCount = m_colEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4EvIndex = 0; c4EvIndex < c4EvCount; c4EvIndex++)
    {
        TOpcodeBlock& colOps = m_colEventOps[c4EvIndex];
        const tCIDLib::TCard4 c4OpCount = colOps.c4ElemCount();

        for (tCIDLib::TCard4 c4OpIndex = 0; c4OpIndex < c4OpCount; c4OpIndex++)
        {
            TActOpcode& aocCur = colOps[c4OpIndex];
            TCQCCmdCfg& ccfgCur = aocCur.m_ccfgStep;

            // If it's a comment, skip it
            if (aocCur.m_eOpcode == tCQCKit::EActOps::Comment)
                continue;

            if (c2FmtVersion < 3)
            {
                //
                //  We added a new parameter to the field read and write cmds.
                //  They can now indicate if they want it to just return a
                //  true or false status or to fail the command if the read
                //  or write cannot be done. We default it to fail, since
                //  that was the old scheme.
                //
                if ((ccfgCur.strCmdId() == L"StdCmd:FieldRead")
                ||  (ccfgCur.strCmdId() == L"StdCmd:FieldWrite"))
                {
                    if (ccfgCur.c4ParmCnt() == 2)
                    {
                        ccfgCur.c4ParmCnt(3);
                        ccfgCur.SetParmAt(2, kCQCKit::pszFld_True, tCQCKit::ECmdPTypes::Boolean);
                    }
                }
                 else if ((ccfgCur.strCmdId() == L"StdCmd:FieldRead2")
                      ||  (ccfgCur.strCmdId() == L"StdCmd:FieldWrite2"))
                {
                    if (ccfgCur.c4ParmCnt() == 3)
                    {
                        ccfgCur.c4ParmCnt(4);
                        ccfgCur.SetParmAt(3, kCQCKit::pszFld_True, tCQCKit::ECmdPTypes::Boolean);
                    }
                }
                 else if (ccfgCur.strCmdId() == L"IntfCmd::SetTimeout")
                {
                    // This one accidentally got two colons, so get rid of one
                    ccfgCur.strCmdId(L"IntfCmd:SetTimeout");
                }
                 else if ((ccfgCur.strCmdId() == L"IntfCmd:LoadOverlay")
                      &&  (m_colEvents[c4EvIndex].m_strEventId == L"StdEvent:OnPreload"))
                {
                    //
                    //  We convert this to a SetInitTemplate command, which
                    //  has the same parms. And it's on the same target. So
                    //  all we need to is change the command name and id.
                    //
                    ccfgCur.strCmdId(L"IntfCmd:SetInitTemplate");
                    ccfgCur.strName(L"SetInitTemplate");
                }

                //
                //  Originally we had no special parameter tpye for variable
                //  names, which was dumb because we couldn't do a lot
                //  auto-magical things. So find all of them and update
                //  the correct parameters to indicate the newly available
                //  type.
                //
                //  To disambiguate some command names used in separate
                //  places, the difference is always between the standard
                //  and GUI based (widget) targets. So we check the GUI
                //  dispatch flag.
                //
                if ((ccfgCur.strCmdId() == kCQCKit::strCmdId_Add)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_AddQListValue)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_AND)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_Append)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_CapAt)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_CreateVariable)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_DelVariable)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_DelSubStr)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_Divide)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_Exists)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetCount)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetIndex)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetLength)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetSubStr)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetShowText)
                ||  (ccfgCur.strCmdId() == L"MediaCmd:GetSortOrder")
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetUserActionVals)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_Insert)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_Multiply)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_OR)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_Replace)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_ReplaceToken)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_SetVariable)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_SetVarFmt)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_SplitAt)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_Strip)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_Subtract)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_ToLower)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_ToUpper))
                {
                    // These all have a first parameter to change
                    ccfgCur.piAt(0).m_eType = tCQCKit::ECmdPTypes::VarName;
                }

                if ((ccfgCur.strCmdId() == kCQCKit::strCmdId_CvtToElapsed)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_FieldRead)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_Find)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_FmtNumAsSpoken)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_FormatTimeVal)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GenRandNum)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetDateParts)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetEventFld)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetLength)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetNumericRange)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetSchEvInfo)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetSunEvInfo)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetTimeParts)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetTimerVal)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetUserActionVals)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_HasQueuedFldChange)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_SplitAt))
                {
                    // These all have a second parameter to change
                    ccfgCur.piAt(1).m_eType = tCQCKit::ECmdPTypes::VarName;
                }

                if ((ccfgCur.strCmdId() == kCQCKit::strCmdId_CvtToElapsed)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_FieldRead2)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetDateParts)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetNumericRange)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetSchEvInfo)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetSunEvInfo)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetTimeParts))
                {
                    // These all have a third parameter to change
                    ccfgCur.piAt(2).m_eType = tCQCKit::ECmdPTypes::VarName;
                }

                if ((ccfgCur.strCmdId() == kCQCKit::strCmdId_CvtToElapsed)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetDateParts)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetNthToken)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetSchEvInfo)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetSubStr)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetTimeParts)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_QueryDrvText))
                {
                    // These all have a fourth parameter to change
                    ccfgCur.piAt(3).m_eType = tCQCKit::ECmdPTypes::VarName;
                }

                //
                //  This one has some special considerations. It's used by
                //  more than one target, with different parameters.
                //
                if (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetText)
                {
                    if ((ccfgCur.strTargetId() == kCQCKit::strCTarId_GVars)
                    ||  (ccfgCur.strTargetId() == kCQCKit::strCTarId_LVars))
                    {
                        ccfgCur.piAt(0).m_eType = tCQCKit::ECmdPTypes::VarName;
                        ccfgCur.piAt(1).m_eType = tCQCKit::ECmdPTypes::VarName;
                    }
                     else
                    {
                        //
                        //  It's got to be one of the widget targets. There
                        //  are two variations. If two parms it's the second
                        //  one, else the first one.
                        //
                        if (ccfgCur.c4ParmCnt() == 1)
                            ccfgCur.piAt(0).m_eType = tCQCKit::ECmdPTypes::VarName;
                        else
                            ccfgCur.piAt(1).m_eType = tCQCKit::ECmdPTypes::VarName;
                    }
                }
            }

            if (c2FmtVersion < 4)
            {
                // This one has two colons instead of one as it should
                if (ccfgCur.strCmdId() == L"IntfCmd::LoadSiblingOverlay")
                    ccfgCur.strCmdId(L"IntfCmd:LoadSiblingOverlay");

                //
                //  Any commands that reference templates have to be moved
                //  into a default initial /User area. In all but one of these
                //  cases it is the first or only parameter. One has it on
                //  the second parm.
                //
                if ((ccfgCur.strCmdId() == L"IntfCmd:DoLink")
                ||  (ccfgCur.strCmdId() == L"IntfCmd:InvokePopup")
                ||  (ccfgCur.strCmdId() == L"IntfCmd:InvokePopout")
                ||  (ccfgCur.strCmdId() == L"IntfCmd:LoadOverlay")
                ||  (ccfgCur.strCmdId() == L"IntfCmd:LoadSiblingOverlay")
                ||  (ccfgCur.strCmdId() == L"IntfCmd:SetInitTemplate"))
                {
                    tCIDLib::TCard4 c4PInd = 0;
                    if (ccfgCur.strCmdId() == L"IntfCmd:LoadSiblingOverlay")
                        c4PInd = 1;

                    TString& strTar = ccfgCur.piAt(c4PInd).m_strValue;
                    if (!strTar.bIsEmpty())
                        strTar.Insert(L"/User/", 0);
                }
            }

            if (c2FmtVersion < 5)
            {
                //
                //  We added a new one in 3.1.25 to set a list from an enum
                //  variable. But then wanted to add a field version. So we
                //  modify the original one's name and id a bit.
                //
                if (ccfgCur.strCmdId() == L"IntfCmd:SetFromEnum")
                {
                    ccfgCur.strCmdId(L"IntfCmd:SetFromEnumVar");
                    ccfgCur.strName(L"SetFromEnumVar");
                }

                //
                //  For the invoke popup command, change the second parm from
                //  the original (and never used) Opacity to a Parameters type
                //  command. For the popout version, we have to add a new one
                //  at the end.
                //
                if (ccfgCur.strCmdId() == L"IntfCmd:InvokePopup")
                    ccfgCur.SetParmAt(1, L"", tCQCKit::ECmdPTypes::QuotedList);

                if (ccfgCur.strCmdId() == L"IntfCmd:InvokePopout")
                {
                    ccfgCur.c4ParmCnt(4);
                    ccfgCur.SetParmAt(3, L"", tCQCKit::ECmdPTypes::QuotedList);
                }

                // And add the same for the do global action command
                if (ccfgCur.strCmdId() == kCQCKit::strCmdId_DoGlobalAct)
                {
                    ccfgCur.c4ParmCnt(2);
                    ccfgCur.SetParmAt(1, L"", tCQCKit::ECmdPTypes::QuotedList);
                }
            }

            if (c2FmtVersion < 6)
            {
                //
                //  The base widget class in the interface system was changed
                //  so that the old set visibility command became a new
                //  set widget state command.
                //
                if (ccfgCur.strCmdId() == L"StdCmd:HideShow")
                {
                    ccfgCur.strCmdId(L"IntfCmd:SetWdgState");
                    ccfgCur.strName(L"SetWidgetState");

                    if (facCQCKit().bCheckBoolVal(ccfgCur.piAt(0).m_strValue))
                        ccfgCur.SetParmAt(0, L"Normal", tCQCKit::ECmdPTypes::Enum);
                    else
                        ccfgCur.SetParmAt(0, L"Hidden", tCQCKit::ECmdPTypes::Enum);
                }
            }

            if (c2FmtVersion < 7)
            {
                if (ccfgCur.strCmdId() == L"MediaCmd:QueryByArtist")
                {
                    ccfgCur.strCmdId(L"MediaCmd:SearchByArtist");
                    ccfgCur.strName(L"SearchByArtist");
                }
                 else if (ccfgCur.strCmdId() == L"MediaCmd:SearchTitleNames")
                {
                    ccfgCur.strCmdId(L"MediaCmd:SearchByTitle");
                    ccfgCur.strName(L"SearchByTitle");
                }
                 else if (ccfgCur.strCmdId() == L"MediaCmd:SearchActors")
                {
                    ccfgCur.strCmdId(L"MediaCmd:SearchByActor");
                    ccfgCur.strName(L"SearchByActor");
                }
                 else if (ccfgCur.strCmdId().bStartsWith(L"StdCmd:StdCmd:"))
                {
                    // Some of them somehow got a double prefix, remove one
                    strTmp = ccfgCur.strCmdId();
                    strTmp.Cut(0, 7);
                    ccfgCur.strCmdId(strTmp);
                }
            }


            if (c2FmtVersion < 8)
            {
                // A new return result parameter was added to the exit command
                if (ccfgCur.strCmdId() == kCQCKit::strCmdId_Exit)
                {
                    ccfgCur.c4ParmCnt(1);
                    ccfgCur.SetParmAt(0, L"True", tCQCKit::ECmdPTypes::Boolean);
                }
            }

            if (c2FmtVersion < 9)
            {
                if (ccfgCur.strCmdId() == kCQCKit::strCmdId_Equals)
                {
                    ccfgCur.c4ParmCnt(3);
                    ccfgCur.SetParmAt(2, L"Case", tCQCKit::ECmdPTypes::Enum);
                }
            }

            if (c2FmtVersion < 10)
            {
                if (ccfgCur.strCmdId() == L"IntfCmd:SetBlankOpts")
                {
                    //
                    //  Add a new template parm and params, with empty values
                    //  as defaults.
                    //
                    ccfgCur.c4ParmCnt(3);
                    ccfgCur.SetParmAt(1, L"", tCQCKit::ECmdPTypes::Template);
                    ccfgCur.SetParmAt(2, L"", tCQCKit::ECmdPTypes::QuotedList);
                }
            }

            if (c2FmtVersion < 11)
            {
                if (ccfgCur.strCmdId() == kCQCKit::strCmdId_IsNightTime)
                {
                    // Add the new offset parameter
                    ccfgCur.c4ParmCnt(1);
                    ccfgCur.SetParmAt(0, L"0", tCQCKit::ECmdPTypes::Signed);
                }
            }

            if (c2FmtVersion < 12)
            {
                // Convert UpLevel commands to Back commands
                if (ccfgCur.strCmdId() == kCQCKit::strCmdId_UpLevel)
                {
                    ccfgCur.strCmdId(kCQCKit::strCmdId_Back);
                    ccfgCur.strName
                    (
                        facCQCKit().strMsg(kKitMsgs::midCmd_Back)
                    );
                }
            }

            if (c2FmtVersion < 13)
            {
                // Add a new 'format' parameter to the set widget color command
                if (ccfgCur.strCmdId() == L"IntfCmd:SetWidgetClr")
                {
                    ccfgCur.c4ParmCnt(3);
                    ccfgCur.SetParmAt(2, L"RGB", tCQCKit::ECmdPTypes::Enum);
                }
            }

            if (c2FmtVersion < 14)
            {
                if ((ccfgCur.strCmdId() == kCQCKit::strCmdId_GetLoadChangeInfo)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetMotionInfo)
                ||  (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetZoneAlarmInfo))
                {
                    ccfgCur.c4ParmCnt(3);
                    ccfgCur.SetParmAt(2, L"", tCQCKit::ECmdPTypes::VarName);
                }
            }

            if (c2FmtVersion < 15)
            {
                if (ccfgCur.strCmdId() == kCQCKit::strCmdId_SendWOL)
                {
                    ccfgCur.c4ParmCnt(2);
                    ccfgCur.SetParmAt(1, L"", tCQCKit::ECmdPTypes::Host);
                }
            }

            if (c2FmtVersion < 16)
            {
                // A new boolean parameter was added, default to true
                if (ccfgCur.strCmdId() == kCQCKit::strCmdId_WaitDriverReady)
                {
                    ccfgCur.c4ParmCnt(3);
                    ccfgCur.SetParmAt(2, L"True", tCQCKit::ECmdPTypes::Boolean);
                }
            }

            if (c2FmtVersion < 17)
            {
                const tCIDLib::TCard4   c4PCnt = ccfgCur.c4ParmCnt();

                for (tCIDLib::TCard4 c4PInd = 0; c4PInd < c4PCnt; c4PInd++)
                {
                    TCQCCmdCfg::TParmInfo& piCur = ccfgCur.piAt(c4PInd);
                    switch(piCur.m_eType)
                    {
                        case tCQCKit::ECmdPTypes::GlobAction :
                        case tCQCKit::ECmdPTypes::ImgPath :
                        case tCQCKit::ECmdPTypes::Macro :
                        case tCQCKit::ECmdPTypes::Template :
                            facCQCKit().Make50Path(piCur.m_strValue);
                            break;

                        case tCQCKit::ECmdPTypes::EvMonId :
                        case tCQCKit::ECmdPTypes::SchEvId :
                        case tCQCKit::ECmdPTypes::TrgEvId :

                            // Move these into the hierarchical space now
                            strTmp = L"/User/";
                            strTmp.Append(piCur.m_strValue);
                            piCur.m_strValue = strTmp;
                            break;

                        default :
                            // Nothing to do
                            break;
                    };
                }

                // Some special case ones
                if (ccfgCur.strCmdId() == L"IntfCmd:LoadSiblingOverlay")
                {
                    TCQCCmdCfg::TParmInfo& piCur = ccfgCur.piAt(0);
                    facCQCKit().Make50Path(piCur.m_strValue);
                }
            }

            if (c2FmtVersion < 18)
            {
                // A new, fourth enumerated parameter was added
                if (ccfgCur.strCmdId() == L"IntfCmd:InvokePopup")
                {
                    ccfgCur.c4ParmCnt(4);

                    //
                    //  If the third parm is -1,-1, then default to center, else to
                    //  to upper left, for backwards compatibility.
                    //
                    const TString& strAt = ccfgCur.piAt(2).m_strValue;
                    TPoint pntAt;
                    if (pntAt.bParseFromText(strAt, tCIDLib::ERadices::Auto, kCIDLib::chSpace)
                    ||  pntAt.bParseFromText(strAt, tCIDLib::ERadices::Auto, kCIDLib::chComma))
                    {
                        if (pntAt == TPoint(-1, -1))
                            ccfgCur.SetParmAt(3, L"Center", tCQCKit::ECmdPTypes::Text);
                        else
                            ccfgCur.SetParmAt(3, L"Upper Left", tCQCKit::ECmdPTypes::Text);
                    }
                }
            }

            if (c2FmtVersion < 19)
            {
                if (ccfgCur.strCmdId() == kCQCKit::strCmdId_GetSemFields)
                {
                    // Append p4 to p3, with a space between
                    TCQCCmdCfg::TParmInfo& pi3 = ccfgCur.piAt(2);
                    TCQCCmdCfg::TParmInfo& pi4 = ccfgCur.piAt(3);
                    pi3.m_strValue.Append(kCIDLib::chSpace);
                    pi3.m_strValue.Append(pi4.m_strValue);

                    //
                    //  And default the access to read and write, which is what the old
                    //  hard coded value was, so should cause the same results.
                    //
                    pi4.m_strValue = tCQCKit::strXlatEReqAccess(tCQCKit::EReqAccess::ReadAndWrite);
                }
            }
        }
    }
}


//
//  Clear our command list, either all of them or for a given event, and
//  optionally removing the events themselves. Normally we don't remove
//  them. It's generally done when a class has ops added to them after
//  the fact. So it needs to add new ops to older ones, but not to newer
//  ones, and it might change them, so it that case it wants to remove
//  them and add the ones it wants.
//
tCIDLib::TVoid MCQCCmdSrcIntf::ResetAllOps(const tCIDLib::TBoolean bRemove)
{
    if (bRemove)
    {
        // We are removing so just flush the list
        m_colEventOps.RemoveAll();
    }
     else
    {
        // Iterate all the events and remove the opcodes for each one
        const tCIDLib::TCard4 c4EvCount = m_colEvents.c4ElemCount();
        for (tCIDLib::TCard4 c4EvIndex = 0; c4EvIndex < c4EvCount; c4EvIndex++)
            m_colEventOps[c4EvIndex].RemoveAll();
    }
}

tCIDLib::TVoid MCQCCmdSrcIntf::ResetOps(const TString& strEvent)
{
    // Find the commands for this event
    tCIDLib::TCard4 c4Index;
    if (pcaeiFindEv(strEvent, c4Index, kCIDLib::False))
        m_colEventOps[c4Index].RemoveAll();
}


// Clear the runtime value definition list
tCIDLib::TVoid MCQCCmdSrcIntf::ResetRTVals()
{
    m_colRTVals.RemoveAll();
}


// Return the id of the event at the indicated index
const TString& MCQCCmdSrcIntf::strEventIdAt(const tCIDLib::TCard4 c4At) const
{
    return m_colEvents[c4At].m_strEventId;
}


// Gets the name (human readable name) of the runtime value at the index
const TString&
MCQCCmdSrcIntf::strRTValNameAt(const tCIDLib::TCard4 c4At) const
{
    if (c4At >= m_colRTVals.c4ElemCount())
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_BadRTVIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4At)
        );
    }
    return m_colRTVals[c4At].strName();
}


// Set the action context for the indicated event
tCIDLib::TVoid
MCQCCmdSrcIntf::SetEventContext(const   TString&            strEventId
                                , const tCQCKit::EActCmdCtx eContext)
{
    tCIDLib::TCard4 c4Index;
    TCQCActEvInfo* pcaeiRet = pcaeiFindEv(strEventId, c4Index, kCIDLib::True);
    pcaeiRet->m_eContext = eContext;
}


//
//  We watch for our actions attribute and stream our content in from it if so.
//
tCIDLib::TVoid
MCQCCmdSrcIntf::SetCmdAttr(const TAttrData& adatNew, const TAttrData&)
{
    if (adatNew.strId() == kCQCKit::strAttr_Actions)
    {
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        ReadInOps(strmSrc);

        //
        //  There is also a list of RTV definitions but we don't need to pull those out. We
        //  just put them in there for the benefit of the action editor.
        //
    }
}


//
//  Stores a list of opcodes for the indicated event. We just overwrite any
//  existing opcodes for the target event. We have one version that takes a
//  standard opcode list (by value vector) and one that takes a ref collection,
//  which is needed to make the action editing dialog efficient.
//
tCIDLib::TVoid
MCQCCmdSrcIntf::SetEventOps(const   TString&        strEventId
                            , const TOpcodeBlock&   colNewOps)
{
    // Find the commands for this event
    tCIDLib::TCard4 c4Index;
    pcaeiFindEv(strEventId, c4Index, kCIDLib::True);
    TOpcodeBlock& colOps = m_colEventOps[c4Index];
    colOps = colNewOps;
}

tCIDLib::TVoid
MCQCCmdSrcIntf::SetEventOps(const   TString&                    strEventId
                            , const TRefCollection<TActOpcode>& colNewOps)
{
    // Find the commands for this event
    tCIDLib::TCard4 c4Index;
    pcaeiFindEv(strEventId, c4Index, kCIDLib::True);
    TOpcodeBlock& colOps = m_colEventOps[c4Index];

    // Clear out the list, then loop through the source and add them
    colOps.RemoveAll();

    TColCursor<TActOpcode>* pcursOps = colNewOps.pcursNew();
    TJanitor<TColCursor<TActOpcode> > janCurs(pcursOps);
    if (pcursOps->bReset())
    {
        do
        {
            colOps.objAdd(pcursOps->objRCur());
        }   while(pcursOps->bNext());
    }
}


//
//  Finds any commands for the indicated command id, and updates them
//  to have the new indicated target and new command id.
//
tCIDLib::TVoid
MCQCCmdSrcIntf::UpdateOps(  const   TString&        strOldCmdId
                            , const MCQCCmdTarIntf& mctarNewTar
                            , const TString&        strNewTarId
                            , const TString&        strNewCmdId)
{
    const tCIDLib::TCard4 c4EvCount = m_colEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4EvIndex = 0; c4EvIndex < c4EvCount; c4EvIndex++)
    {
        TOpcodeBlock& colOps = m_colEventOps[c4EvIndex];
        const tCIDLib::TCard4 c4OpCount = colOps.c4ElemCount();

        for (tCIDLib::TCard4 c4OpIndex = 0; c4OpIndex < c4OpCount; c4OpIndex++)
        {
            TActOpcode& aocCur = colOps[c4OpIndex];
            TCQCCmdCfg& ccfgCur = aocCur.m_ccfgStep;

            //
            //  If this one is one of the old commands indicated, then
            //  update it.
            //
            if (ccfgCur.strCmdId() == strOldCmdId)
            {
                ccfgCur.SetTarget(mctarNewTar);
                ccfgCur.strTargetId(strNewTarId);
                ccfgCur.strCmdId(strNewCmdId);
            }
        }
    }
}


//
//  Stream ours key info to the passed stream. This is used to support the action system
//  which needs to get a flattened buffer of the stuff required to edit our configured
//  action.
//
tCIDLib::TVoid
MCQCCmdSrcIntf::WriteOutOps(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCKit_CmdIntf::c2SrcVersionFmt
                    << m_colEvents
                    << m_colEventOps
                    << tCIDLib::EStreamMarkers::Frame;
}


// ---------------------------------------------------------------------------
//  MCQCCmdSrcIntf: Private, static methods
// ---------------------------------------------------------------------------

// Sees if the passed event id is n our list. If so, return a pointer and index
TCQCActEvInfo*
MCQCCmdSrcIntf::pcaeiFindEv(const   TString&            strEvent
                            ,       tCIDLib::TCard4&    c4Index
                            , const tCIDLib::TBoolean   bThrowIfNot)
{
    const tCIDLib::TCard4 c4Count = m_colEvents.c4ElemCount();
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colEvents[c4Index].m_strEventId == strEvent)
            return &m_colEvents[c4Index];
    }

    if (bThrowIfNot)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_EventNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strEvent
        );
    }
    return 0;
}

const TCQCActEvInfo*
MCQCCmdSrcIntf::pcaeiFindEv(const   TString&            strEvent
                            ,       tCIDLib::TCard4&    c4Index
                            , const tCIDLib::TBoolean   bThrowIfNot) const
{
    const tCIDLib::TCard4 c4Count = m_colEvents.c4ElemCount();
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colEvents[c4Index].m_strEventId == strEvent)
            return &m_colEvents[c4Index];
    }

    if (bThrowIfNot)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_EventNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strEvent
        );
    }
    return 0;
}



// ---------------------------------------------------------------------------
//   CLASS: MCQCCmdTarIntf
//  PREFIX: mctar
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MCQCCmdTarIntf: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
MCQCCmdTarIntf::eCompByName(const   MCQCCmdTarIntf&     mctar1
                            , const MCQCCmdTarIntf&     mctar2)
{
    return mctar1.strCmdTargetName().eCompareI(mctar2.strCmdTargetName());
}

const TString& MCQCCmdTarIntf::strKey(const MCQCCmdTarIntf& mctarSrc)
{
    return mctarSrc.m_strId;
}


// ---------------------------------------------------------------------------
//  MCQCCmdTarIntf: Destructor
// ---------------------------------------------------------------------------
MCQCCmdTarIntf::~MCQCCmdTarIntf()
{
}


// ---------------------------------------------------------------------------
//  MCQCCmdTarIntf: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  Some targets support a set of reusable sub-actions that can be invoked from
//  its own actions. In order to be able to represent this list to the user
//  during action editing, the action editor needs to be able to query that
//  list of action names, to allow the user to select one. If not overridden,
//  we just return false to say none are supported.
//
//  For added flexibility, the command id and the action context are passed as
//  well, so that filtering can be done based on those.
//
tCIDLib::TBoolean
MCQCCmdTarIntf::bQueryTarSubActNames(       TCollection<TString>&
                                    , const TString&
                                    , const tCQCKit::EActCmdCtx) const
{
    return kCIDLib::False;
}


//
//  We can do some basic validation for all command targets, just based on the command
//  parameter type. For a lot of them, this is all that will ever be needed, so this will
//  save a lot of redundant busy work
//
//  They just have to call us first, and return false if we return false. Else they do
//  their own validation.
//
//  We get the original command definition (from the source, TCQCCmd) and we get the
//  configured command info that has been configured into this target that we need to
//  validate. The target info doesn't have some info we need, such as enum values. But
//  the source info doesn't have the values. WE get the value of the param to validate,
//  but sometimes other parameters are dependent so the derived class needs to know
//  those as well, so we have to pass the configured target command info for that.
//
tCIDLib::TBoolean
MCQCCmdTarIntf::bValidateParm(  const   TCQCCmd&        cmdSrc
                                , const TCQCCmdCfg&     ccfgTar
                                , const tCIDLib::TCard4 c4Index
                                , const TString&        strPVal
                                ,       TString&        strErrText)
{
    // Get the indicate parameter's info
    const TCQCCmdParm& cmdpVal = cmdSrc.cmdpAt(c4Index);

    tCIDLib::TBoolean bRet = kCIDLib::True;
    switch(cmdpVal.eType())
    {
        case tCQCKit::ECmdPTypes::Area :
        {
            TArea areaTest;
            bRet = areaTest.bParseFromText
            (
                strPVal, tCIDLib::ERadices::Dec, kCIDLib::chComma
            );

            if (!bRet)
            {
                // We used to accept space based, so check that as well
                bRet = areaTest.bParseFromText
                (
                    strPVal, tCIDLib::ERadices::Dec, kCIDLib::chSpace
                );
                if (!bRet)
                    strErrText = L"The area is incorrectly formatted (format: x, y, cx, cy)";
            }
            break;
        }

        case tCQCKit::ECmdPTypes::Boolean :
        {
            tCIDLib::TBoolean bVal;
            bRet = facCQCKit().bCheckBoolVal(strPVal, bVal);
            if (!bRet)
                strErrText = L"Boolean values must be either True or False (or 1 or 0)";
            break;
        }

        case tCQCKit::ECmdPTypes::Color :
        {
            TRGBClr rgbTest;
            bRet = rgbTest.bParseFromText
            (
                strPVal, tCIDLib::ERadices::Dec, kCIDLib::chSpace
            );

            if (!bRet)
                strErrText = L"The color is incorrectly formatted (format: rr gg bb)";
            break;
        }

        case tCQCKit::ECmdPTypes::Enum :
        {
            tCIDLib::TCard4 c4Dummy;
            if (!cmdpVal.bIsLegalEnumVal(strPVal, c4Dummy))
            {
                strErrText.LoadFromMsg(kKitErrs::errcCmd_BadEnumVal, facCQCKit(), strPVal);
                bRet = kCIDLib::False;
            }
            break;
        }

        case tCQCKit::ECmdPTypes::Field :
        {
            TString strTmp1, strTmp2;
            if (!facCQCKit().bParseFldName(strPVal, strTmp1, strTmp2))
            {
                strErrText.LoadFromMsg(kKitErrs::errcFld_BadFldNameFmt, facCQCKit(), strPVal);
                bRet = kCIDLib::False;
            }
            break;
        }

        case tCQCKit::ECmdPTypes::Host :
        {
            //
            //  Attempt to resolve the address. The IV type is also
            //  a host but is just a separate type because it provides
            //  a specialized selection dialog.
            //
            try
            {
                TIPAddress ipaTest(strPVal, tCIDSock::EAddrTypes::Unspec);
            }

            catch(...)
            {
                bRet = kCIDLib::False;
            }
            break;
        }

        case tCQCKit::ECmdPTypes::Macro :
        {
            //
            //  We say empty is ok here. It's the upstream responsibility to know if
            //  this parameter is required or not and that would have already been
            //  checked.
            //
            bRet = facCIDMacroEng().bIsValidCMLPath(strPVal, strErrText, kCIDLib::True);
            break;
        }

        case tCQCKit::ECmdPTypes::Number :
        {
            //
            //  Probe the value to make sure it evalutes to some sort of number. This
            //  will modify the string if there is any suffix it wants to remove, so
            //  we have to make a copy.
            //
            TString strTmp(strPVal);
            tCIDLib::ERadices eRadix;
            tCIDLib::TBoolean bExplicit;
            if (TMacroEngParser::eCheckNumericLiteral(strTmp, eRadix, bExplicit) == tCIDMacroEng::ENumTypes::None)
            {
                strErrText = L"The value must evaluate to some numeric value";
                bRet = kCIDLib::False;
            }
            break;
        }

        case tCQCKit::ECmdPTypes::Point :
        {
            TPoint pntTest;
            bRet = pntTest.bParseFromText(strPVal, tCIDLib::ERadices::Dec, kCIDLib::chComma);
            if (!bRet)
            {
                // We used to accept space based, so accept that as well
                bRet = pntTest.bParseFromText(strPVal, tCIDLib::ERadices::Dec, kCIDLib::chSpace);
                if (!bRet)
                    strErrText = L"The point is incorrectly formatted (format: x, y)";
            }
            break;
        }

        case tCQCKit::ECmdPTypes::RegXText :
        {
            // Make sure it's parsable as a regular expression
            try
            {
                TRegEx regxTest(strPVal);
            }

            catch(TError& errToCatch)
            {
                bRet = kCIDLib::False;
                strErrText = L"The regular expression is invalid. Reason=";
                strErrText.Append(errToCatch.strErrText());
            }
            break;
        }

        case tCQCKit::ECmdPTypes::Signed :
        {
            tCIDLib::TInt4 i4Tmp;
            if (!strPVal.bToInt4(i4Tmp))
            {
                strErrText.LoadFromMsg(kKitErrs::errcCmd_ExpectedInt, facCQCKit());
                bRet = kCIDLib::False;
            }
            break;
        }

        case tCQCKit::ECmdPTypes::Unsigned :
        {
            tCIDLib::TCard4 c4Tmp;
            if (!strPVal.bToCard4(c4Tmp))
            {
                strErrText.LoadFromMsg(kKitErrs::errcCmd_ExpectedCard, facCQCKit());
                bRet = kCIDLib::False;
            }
            break;
        }

        case tCQCKit::ECmdPTypes::VarName :
        {
            bRet = TStdVarsTar::bValidVarName(strPVal, TString::strEmpty(), strErrText);
            break;
        }

        case tCQCKit::ECmdPTypes::EvMonId :
        case tCQCKit::ECmdPTypes::GlobAction :
        case tCQCKit::ECmdPTypes::ImgPath :
        case tCQCKit::ECmdPTypes::SchEvId :
        case tCQCKit::ECmdPTypes::Template :
        case tCQCKit::ECmdPTypes::TrgEvId :
            //
            //  Make sure it's a syntactically correct remote browser path. All of these
            //  are referencing files on the data server. Relatives are only OK if it's
            //  a template.
            //
            bRet = facCQCKit().bIsValidRemTypeRelPath
            (
                strPVal, strErrText, cmdpVal.eType() == tCQCKit::ECmdPTypes::Template
            );
            break;

        case tCQCKit::ECmdPTypes::EMailAcct :
        default :
            // Not one we care about
            strErrText.Clear();
            break;
    };
    return bRet;
}


tCIDLib::TVoid MCQCCmdTarIntf::SetDefParms(TCQCCmdCfg&) const
{
    // Default is to do nothing if not overridden
}


// ---------------------------------------------------------------------------
//  MCQCCmdTarIntf: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the GUI dispatch flag
tCIDLib::TBoolean MCQCCmdTarIntf::bGUIDispatch() const
{
    return m_bGUIDispatch;
}

tCIDLib::TBoolean
MCQCCmdTarIntf::bGUIDispatch(const tCIDLib::TBoolean bToSet)
{
    m_bGUIDispatch = bToSet;
    return m_bGUIDispatch;
}


// Get/set the special command target flag
tCIDLib::TBoolean MCQCCmdTarIntf::bIsSpecialCmdTar() const
{
    return m_bSpecial;
}

tCIDLib::TBoolean
MCQCCmdTarIntf::bIsSpecialCmdTar(const tCIDLib::TBoolean bToSet)
{
    m_bSpecial = bToSet;
    return m_bSpecial;
}


// Give access to the unique id
tCIDLib::TCard4 MCQCCmdTarIntf::c4UniqueId() const
{
    return m_c4UniqueId;
}


// Get or set the help id
const TString& MCQCCmdTarIntf::strCmdHelpId() const
{
    return m_strHelpId;
}

const TString& MCQCCmdTarIntf::strCmdHelpId(const TString& strToSet)
{
    m_strHelpId = strToSet;
    return m_strHelpId;
}


//  Query the command target id, name, or type
const TString& MCQCCmdTarIntf::strCmdTargetId() const
{
    return m_strId;
}

const TString& MCQCCmdTarIntf::strCmdTargetName() const
{
    return m_strName;
}

const TString& MCQCCmdTarIntf::strCmdTargetType() const
{
    return m_strType;
}


//
//  Sets the target id and name. There are some targets that must maintain
//  their own name and target id, or at least build the id off the name and
//  so forth. So they need to set these after the fact.
//
tCIDLib::TVoid
MCQCCmdTarIntf::SetCmdTargetIds(const   TString&    strId
                                , const TString&    strName
                                , const TString&    strType)
{
    m_strId = strId;
    m_strName = strName;
    m_strType = strType;
}



// ---------------------------------------------------------------------------
//  MCQCCmdTarIntf: Hidden constructors and operators
// ---------------------------------------------------------------------------

MCQCCmdTarIntf::MCQCCmdTarIntf() :

    m_bGUIDispatch(kCIDLib::False)
    , m_bSpecial(kCIDLib::False)
    , m_c4UniqueId(0)
{
    AssignId();
}

MCQCCmdTarIntf::MCQCCmdTarIntf( const   TString&    strTarId
                                , const TString&    strTarName
                                , const TString&    strTarType
                                , const TString&    strHelpId) :

    m_bGUIDispatch(kCIDLib::False)
    , m_bSpecial(kCIDLib::False)
    , m_c4UniqueId(0)
    , m_strHelpId(strHelpId)
    , m_strId(strTarId)
    , m_strName(strTarName)
    , m_strType(strTarType)
{
    AssignId();
}

MCQCCmdTarIntf::MCQCCmdTarIntf(const MCQCCmdTarIntf& mctarSrc) :

    m_bGUIDispatch(mctarSrc.m_bGUIDispatch)
    , m_bSpecial(mctarSrc.m_bSpecial)
    , m_c4UniqueId(mctarSrc.m_c4UniqueId)
    , m_strHelpId(mctarSrc.m_strHelpId)
    , m_strId(mctarSrc.m_strId)
    , m_strName(mctarSrc.m_strName)
    , m_strType(mctarSrc.m_strType)
{
}

MCQCCmdTarIntf& MCQCCmdTarIntf::operator=(const MCQCCmdTarIntf& mctarSrc)
{
    if (this != &mctarSrc)
    {
        m_bGUIDispatch  = mctarSrc.m_bGUIDispatch;
        m_bSpecial      = mctarSrc.m_bSpecial;
        m_c4UniqueId    = mctarSrc.m_c4UniqueId;
        m_strHelpId     = mctarSrc.m_strHelpId;
        m_strId         = mctarSrc.m_strId;
        m_strName       = mctarSrc.m_strName;
        m_strType       = mctarSrc.m_strType;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  MCQCCmdTarIntf: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
MCQCCmdTarIntf::ThrowUnknownCmd(const TCQCCmdCfg& ccfgBad) const
{
    facCQCKit().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kKitErrs::errcCmd_UnsupportedCmd
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Unknown
        , ccfgBad.strName()
        , m_strName
    );
}


tCIDLib::TVoid
MCQCCmdTarIntf::ThrowBadParmVal(const   TCQCCmdCfg&     ccfgBad
                                , const tCIDLib::TCard4 c4ParmInd) const
{
    facCQCKit().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kKitErrs::errcCmd_BadParmValue
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Unknown
        , TCardinal(c4ParmInd + 1)
        , ccfgBad.strName()
    );
}


// ---------------------------------------------------------------------------
//  MCQCCmdTarIntf: Private static data
// ---------------------------------------------------------------------------
TSafeCard4Counter   MCQCCmdTarIntf::s_scntNextId(0);


// ---------------------------------------------------------------------------
//  MCQCCmdTarIntf: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Assign a unique id. Watch for wrap around and don't allow 0 as an id
tCIDLib::TVoid MCQCCmdTarIntf::AssignId()
{
    m_c4UniqueId = ++s_scntNextId;
    if (!m_c4UniqueId)
        m_c4UniqueId = ++s_scntNextId;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCCmdRTVal
//  PREFIX: crtv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCCmdRTVal: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCCmdRTVal::TCQCCmdRTVal() :

    m_eType(tCQCKit::ECmdPTypes::None)
    , m_strName()
{
}

TCQCCmdRTVal::TCQCCmdRTVal( const   TString&            strName
                            , const TString&            strId
                            , const tCQCKit::ECmdPTypes eType) :
    m_eType(eType)
    , m_strId(strId)
    , m_strName(strName)
{
}

TCQCCmdRTVal::~TCQCCmdRTVal()
{
}


// ---------------------------------------------------------------------------
//  TCQCCmdRTVal: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCKit::ECmdPTypes TCQCCmdRTVal::eType() const
{
    return m_eType;
}


const TString& TCQCCmdRTVal::strId() const
{
    return m_strId;
}


const TString& TCQCCmdRTVal::strName() const
{
    return m_strName;
}


// ---------------------------------------------------------------------------
//  TCQCCmdParm: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCCmdRTVal::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff must start with an object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version for this level
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion > CQCKit_CmdIntf::c2RTValVersionFmt)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_strName
                    >> m_eType
                    >> m_strId;

    // And our stuff must end with a marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCCmdRTVal::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_CmdIntf::c2RTValVersionFmt
                    << m_strName
                    << m_eType
                    << m_strId
                    << tCIDLib::EStreamMarkers::Frame;
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCCmdRTVSrc
//  PREFIX: mcrtv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCCmdRTVSrc: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCCmdRTVSrc::TCQCCmdRTVSrc(const TCQCUserCtx& cuctxToUse) :

    m_cuctxToUse(cuctxToUse)
{
    // If we've not faulted in the runtime values list, then do that.
    if (!CQCKit_CmdIntf::atomRTValsLoaded)
    {
        TBaseLock lockInit;
        if (!CQCKit_CmdIntf::atomRTValsLoaded)
            LoadRTVList();
    }
}

TCQCCmdRTVSrc::~TCQCCmdRTVSrc()
{
}


// ---------------------------------------------------------------------------
//  TCQCCmdRTVSrc: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  At this level we handle the standard values that are always available. If
//  we don't handle it, we return false.
//
tCIDLib::TBoolean
TCQCCmdRTVSrc::bRTValue(const   TString&    strId
                        ,       TString&    strToFill) const
{
    tCIDLib::TBoolean   bFound = kCIDLib::False;

    const TTime         tmCur(tCIDLib::ESpecialTimes::CurrentTime);
    const tCIDLib::TCard4 c4RTVCnt = CQCKit_CmdIntf::colRTVs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RTVCnt; c4Index++)
    {
        const CQCKit_CmdIntf::TRTValItem& itemCur = CQCKit_CmdIntf::colRTVs[c4Index];
        if (strId.bCompareI(itemCur.m_strId))
        {
            //
            //  It should be found at this point. If not, it'll get reset again
            //  to false below.
            //
            bFound = kCIDLib::True;
            switch(itemCur.m_midLoad)
            {
                case kKitMsgs::midRT_CurMillis :
                    strToFill.SetFormatted(TTime::c4Millis());
                    break;

                case kKitMsgs::midRT_CQCVersion:
                    strToFill = kCQCKit::pszVersion;
                    break;

                case kKitMsgs::midRT_MMDDYYYY :
                {
                    tmCur.FormatToStr(strToFill, TTime::strMMDDYYYY());
                    break;
                }

                case kKitMsgs::midRT_DDMMYYYY :
                {
                    TString strFmt(L"%(D,2,0)");
                    strFmt.Append(TLocale::chDateSeparator());
                    strFmt.Append(L"%(M,2,0)");
                    strFmt.Append(TLocale::chDateSeparator());
                    strFmt.Append(L"%(Y,4,0)");
                    tmCur.FormatToStr(strToFill, strFmt);
                    break;
                }

                case kKitMsgs::midRT_CurDateTime :
                {
                    tmCur.FormatToStr(strToFill, TTime::strDTStamp());
                    break;
                }

                case kKitMsgs::midRT_DayNumber :
                case kKitMsgs::midRT_Month :
                case kKitMsgs::midRT_MonthNumber :
                case kKitMsgs::midRT_WeekDay :
                case kKitMsgs::midRT_YYYY :
                {
                    tCIDLib::TCard4     c4Year;
                    tCIDLib::EMonths    eMonth;
                    tCIDLib::TCard4     c4Day;
                    tCIDLib::EWeekDays eDay = tmCur.eAsDateInfo(c4Year, eMonth, c4Day);

                    if (itemCur.m_midLoad == kKitMsgs::midRT_DayNumber)
                        strToFill.SetFormatted(c4Day);
                    else if (itemCur.m_midLoad == kKitMsgs::midRT_Month)
                        strToFill = TLocale::strMonth(eMonth);
                    else if (itemCur.m_midLoad == kKitMsgs::midRT_MonthNumber)
                        strToFill.SetFormatted(tCIDLib::TInt4(eMonth));
                    else if (itemCur.m_midLoad == kKitMsgs::midRT_WeekDay)
                        strToFill = TLocale::strDay(eDay);
                    else if (itemCur.m_midLoad == kKitMsgs::midRT_YYYY)
                        strToFill.SetFormatted(c4Year);
                    break;
                }

                case kKitMsgs::midRT_Hour :
                case kKitMsgs::midRT_Minute :
                {
                    tCIDLib::TCard4     c4Hour;
                    tCIDLib::TCard4     c4Minute;
                    tCIDLib::TCard4     c4Second;
                    tmCur.c4AsTimeInfo(c4Hour, c4Minute, c4Second);

                    if (itemCur.m_midLoad == kKitMsgs::midRT_Hour)
                        strToFill.SetFormatted(c4Hour);
                    else if (itemCur.m_midLoad == kKitMsgs::midRT_Minute)
                        strToFill.SetFormatted(c4Minute);
                    break;
                }

                case kKitMsgs::midRT_HHMM :
                {
                    tmCur.FormatToStr(strToFill, TTime::strHHMM());
                    break;
                }

                case kKitMsgs::midRT_HHMMSS :
                {
                    tmCur.FormatToStr(strToFill, TTime::strHHMMSS());
                    break;
                }

                case kKitMsgs::midRT_HostName :
                    strToFill = TSysInfo::strIPHostName();
                    break;

                case kKitMsgs::midRT_Env01 :
                case kKitMsgs::midRT_Env02 :
                case kKitMsgs::midRT_Env03 :
                case kKitMsgs::midRT_Env04 :
                case kKitMsgs::midRT_Env05 :
                case kKitMsgs::midRT_Env06 :
                case kKitMsgs::midRT_Env07 :
                case kKitMsgs::midRT_Env08 :
                case kKitMsgs::midRT_Env09 :
                {
                    strToFill = m_cuctxToUse.strEnvRTVAt
                    (
                        itemCur.m_midLoad - kKitMsgs::midRT_Env01
                    );
                    bFound = kCIDLib::True;
                    break;
                }

                case kKitMsgs::midRT_IPAddr :
                {
                    try
                    {
                        TIPAddress ipaRTV = facCIDSock().ipaDefLocalAddr();
                        strToFill = ipaRTV.strAsText();
                    }

                    catch(...)
                    {
                        strToFill.Clear();
                    }
                    break;
                }

                case kKitMsgs::midRT_MSHost :
                {
                    strToFill = TSysInfo::strNSAddr();
                    strToFill.bCapAtChar(kCIDLib::chColon);
                    break;
                }

                case kKitMsgs::midRT_TimeStamp :
                {
                    strToFill = L"0x";
                    strToFill.AppendFormatted(TTime::enctNow(), tCIDLib::ERadices::Hex);
                    break;
                }

                case kKitMsgs::midRT_CurUserName :
                {
                    strToFill = m_cuctxToUse.strUserName();
                    break;
                }

                case kKitMsgs::midRT_CurUserRole :
                {
                    strToFill = tCQCKit::strLoadEUserRoles(m_cuctxToUse.eUserRole());
                    break;
                }

                default :
                    // Not one of ours
                    bFound = kCIDLib::False;
                    break;
            };
        }

        // If we found it, then we are done
        if (bFound)
            break;
    }
    return bFound;
}


// ---------------------------------------------------------------------------
//  TCQCCmdRTVSrc: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Gives derived classes access to our temp format string
TString& TCQCCmdRTVSrc::strTmpFmt() const
{
    return m_strTmpFmt;
}


