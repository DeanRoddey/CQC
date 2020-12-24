//
// FILE NAME: CQCKit_CmdIntf.hpp
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
//  This file defines the core classes and mixins that implement the 'action'
//  system in CQC. Actions are a list in the form:
//
//      Target::Cmd(parm1, parm2)
//
//  plus some flow control commands (If, Else, End, etc...) The user configures
//  actions on a 'source' object (a button for instance) to send out an
//  action, which is a list of commands sent to selected targets.
//
//  Each source object and provide dynamic values that are resolved at
//  runtime (RTVs.) The TCQCCmdRTVSrc is a class that they derive from t
//  make those values availble. This class provides some common RTVs that
//  are always available, and the derivative provides situation specific
//  ones. TCQCRTVal represents a single runtime value, and holds the name,
//  the value, and the type of the value. It's just used internally
//  within the RTV source objects.
//
//  TCQCCmdParm and TCQCCmd store the info about the commands in the
//  target. I.e. the target has to make a list of his commands (and their
//  parms) available to the editing dialog so that it can present those
//  options ot the user. These are used to provide that info. So they are
//  metadata about the available commands, not the configured actual command/
//  data.
//
//  TCQCCmdCfg stores the actual configured command info. So it is
//  constructed from a TCQCCmd object, which tells it what parameters it
//  needs to store, and then the actual information that the user configured
//  for a command is stored in it. So these are stored in the source object,
//  e.g. a set of these are stored in a button to be invoked when the button
//  is pressed and it contains the actual values the user wants to send
//  out to the commands when it's pressed.
//
//  TActOpcode is the actual thing that represents the steps in the action.
//  Since we can have command but also we can have flow control stuff, we
//  need a way to indicate whether a given step in the action is a command
//  to perform or a conditional, and if a conditional where to jump to
//  if that is required and so forth. So the action opcode contains a
//  TCQCCmdCfg object for the actual command or conditional expression, and
//  the extra info about how to react to the result of the command or
//  expression.
//
//  MCQCCmdSrcIntf is a mixin that any class wanting to be an action source,
//  i.e. that wants to be configurable to send out action commands, needs
//  to implement. The action editing dialog and the action engine work in
//  terms of this interface. A command source supports one or more 'events',
//  each of which can contain a list of commands to invoke. So for a button,
//  it would have OnPress, OnRelease, and OnClick events, each of which can
//  be set up to send out different action commands, so each one of them
//  contains an array of TActOpcode objects. An event may be unused and often
//  just one of the available ones is set up to do anything.
//
//  MCQCCmdTarIntf is a mixin that any class wanting to be an action target,
//  i.e. that wants to be able to have sources send it comamnds, needs to
//  implement. It provides the interfaces to let the editing dialog ask
//  for the supported commands of that target, and to invoke a command
//  against the target at runtime.
//
//  MCQCCmdTracer is a mixin that allows for the creation of a trace
//  facility for actions. There are a couple places where actions are
//  invoked, and in the ones that happen in a GUI context, we want to be
//  able to show the user the actual processed steps for debugging
//  purposes. This mixin can be implemented by a class that can provide
//  such a display, and plugged into to the standard actio processing
//  engines, which will call back on it as importing things happen. It is
//  actually passed into each command step and the command targets will
//  also call back on it if they do certain operations.
//
//  Each command can be a conditional one, meaning that it returns a true
//  or false result. They are marked as conditionals if they do. This tells
//  the editing dialog that these can be used as the command of a conditional
//  opcode, which requires a true/false result to know whether to jump or
//  not.
//
//  Each command has an id that identifies it. This is human readable but
//  it's not translatable since it must be the same always because it is
//  what is used internally by the targets to know what command is being
//  invoked on them. So we keep that id internal and just present a loadable
//  text name to the user.
//
// CAVEATS/GOTCHAS:
//
//  1)  There is a standard action engine in CQCKit_StdCmdEngine.cpp
//      which is used in all background programs that invoke actions.
//      For GUI based apps they must be run in a separate thread, which
//      greatly complicates things. The CQCIGKit facility has a 'Run
//      Action Window' class that provides the engine for GUI apps. It
//      is quite complicated and must also deal with the fact that some
//      of the targets can be GUI based objects, so it has to do blocking
//      message sends to the GUI thread and so forth.
//
//  2)  Once a source invokes an action, it's always possible that that
//      action will make the source object go away. So we can never
//      depend on the source being around. This is why there is a separate
//      RTV class instead of just having some methods on the cmd source
//      class. We ask the source for all his RTVs up front in a separate
//      object, which we can keep around and use to satisfy all of the
//      RTV requirements. This means that RTVs cannot change as the
//      action is being processed.
//
//  3)  It is allowed for sources to target themselves, such as a button
//      changing its own text when it's pressed. This is just a convention
//      and it's allowed or disallowed by the editing dialog making the
//      source object available as a target or not. The action editing
//      dialog currently does this. The dialog is in the CQCIGKit facility.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class MCQCCmdTarIntf;
class MCQCCmdSrcIntf;
class MCQCCmdRTValIntf;
class TCQCFldCache;
class TStdVarsTar;
class MCQCCmdTracer;
class TCQCActEngine;
class TCQCUserCtx;


// ---------------------------------------------------------------------------
//   CLASS: TCQCCmdRTVSrc
//  PREFIX: crts
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCCmdRTVSrc : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCCmdRTVSrc
        (
            const   TCQCUserCtx&            cuctxToUse
        );

        TCQCCmdRTVSrc(const TCQCCmdRTVSrc&) = delete;

        ~TCQCCmdRTVSrc();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCCmdRTVSrc& operator=(const TCQCCmdRTVSrc&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bRTValue
        (
            const   TString&                strId
            ,       TString&                strToFill
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        TString& strTmpFmt() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cuctxToUse
        //      A const reference to the user context of the user account
        //      that the RTV object is being used in the context of.
        //
        //  m_strTmpFmt
        //      A temp string to use when formatting RTVs. It's mutable since
        //      it's used from the constant bRTValue() above so it's mutable.
        //      We proivde a protected method to get to it, so that derived
        //      classes can use it also.
        // -------------------------------------------------------------------
        const TCQCUserCtx&  m_cuctxToUse;
        mutable TString     m_strTmpFmt;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCCmdRTVSrc,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCCmdRTVal
//  PREFIX: crtv
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCCmdRTVal : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCCmdRTVal();

        TCQCCmdRTVal
        (
            const   TString&                strName
            , const TString&                strId
            , const tCQCKit::ECmdPTypes     eType
        );

        TCQCCmdRTVal
        (
            const   TCQCCmdRTVal&           crtvToCopy
        );

        virtual ~TCQCCmdRTVal();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCCmdRTVal& operator=
        (
            const   TCQCCmdRTVal&           crtvToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::ECmdPTypes eType() const;

        const TString& strId() const;

        const TString& strName() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eType
        //      The type of the value we represent.
        //
        //  m_strId
        //      A unique id for this runtime value. Configured command params
        //      can refer to runtime values to be used for that parameter at
        //      runtime, and need an unambiguous indentifier for that. We have
        //      to translate the human readable name, so we have to have a
        //      separate id for this purpose.
        //
        //  m_strName
        //      The name of the value.
        // -------------------------------------------------------------------
        tCQCKit::ECmdPTypes m_eType;
        TString             m_strId;
        TString             m_strName;

        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCCmdRTVal,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCCmdParm
//  PREFIX: cmdp
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCCmdParm : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCCmdParm();

        TCQCCmdParm
        (
            const   TString&                strName
            , const tCQCKit::ECmdPTypes     eParmType
            , const tCIDLib::TBoolean       bRequired = kCIDLib::True
        );

        TCQCCmdParm
        (
            const   TString&                strName
            , const TString&                strEnumVals
            , const tCIDLib::TBoolean       bRequired = kCIDLib::True
        );

        TCQCCmdParm
        (
            const   TString&                strName
            , const TString&                strEnumVals
            , const TString&                strDisplayVals
            , const tCIDLib::TBoolean       bRequired = kCIDLib::True
        );

        TCQCCmdParm
        (
            const   TCQCCmdParm&            cmdpToCopy
        );

        ~TCQCCmdParm
        (
        );


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCCmdParm& operator=
        (
            const   TCQCCmdParm&            cmdpToAssign
        );

        tCIDLib::TBoolean operator==(const TCQCCmdParm& ccfgSrc) const;

        tCIDLib::TBoolean operator!=(const TCQCCmdParm& ccfgSrc) const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsLegalEnumVal
        (
            const   TString&                strToCheck
            ,       tCIDLib::TCard4&        c4Index
        )   const;

        tCIDLib::TBoolean bRequired() const;

        tCIDLib::TBoolean bRequired
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCQCKit::EReqAccess eFldAccess() const;

        tCQCKit::EReqAccess eFldAccess
        (
            const   tCQCKit::EReqAccess     eToset
        );

        tCQCKit::ECmdPTypes eType() const;

        tCIDLib::TVoid QueryDefEnumVal
        (
                    TString&                strToFill
            , const tCIDLib::TBoolean       bDisplayVal
        )   const;

        tCIDLib::TVoid QueryEnumVals
        (
                    tCIDLib::TKVPCollect&   colToFill
        )   const;

        const TString& strExtra() const;

        const TString& strExtra
        (
            const   TString&                strToSet
        );

        const TString& strName() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bRequired
        //      Indicates whether this parameter is required or not. Defaults
        //      to true since they almost all are. This is used during command
        //      validation, to make sure that all require parms have something
        //      in them.
        //
        //  m_eFldAccess
        //      If this is a field parameter, then this indicates the type
        //      of access that is required.
        //
        //  m_eType
        //      The type of this parameter
        //
        //  m_strExtra
        //      If the type of this paramter is 'enum', then this is the list of values
        //      the parameter can take. It's a string of comma separated values. If it's
        //      a file, then this is a wild card to use during file selection.
        //
        //  m_strExtra2
        //      If the type of this parameter is 'enum', then this is a parallel list
        //      of values with the same number of entries as m_strExtra. These are the
        //      display values, whereas m_strExtra has the internally used values. In
        //      some cases we need a different human readable value.
        //
        //  m_strName
        //      The name of this parameter, also for human consumption.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bRequired;
        tCQCKit::EReqAccess m_eFldAccess;
        tCQCKit::ECmdPTypes m_eType;
        TString             m_strExtra;
        TString             m_strExtra2;
        TString             m_strName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCCmdParm,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCCmd
//  PREFIX: cmd
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCCmd : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TCQCCmd&                cmdSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCCmd();

        TCQCCmd
        (
            const   TString&                strId
            , const TString&                strName
            , const tCIDLib::TCard4         c4ParmCnt
        );

        TCQCCmd
        (
            const   TString&                strId
            , const TString&                strName
            , const tCQCKit::ECmdPTypes     eParmType
            , const TString&                strParmName
        );

        TCQCCmd
        (
            const   TString&                strId
            , const TString&                strName
            , const TString&                strParmName
            , const TString&                strEnumVals
        );

        TCQCCmd
        (
            const   TString&                strId
            , const TString&                strName
            , const tCQCKit::ECmdPTypes     eParm1Type
            , const TString&                strParm1Name
            , const tCQCKit::ECmdPTypes     eParm2Type
            , const TString&                strParm2Name
        );

        TCQCCmd
        (
            const   TString&                strId
            , const TString&                strName
            , const tCQCKit::ECmdPTypes     eParm1Type
            , const TString&                strParm1Name
            , const tCQCKit::ECmdPTypes     eParm2Type
            , const TString&                strParm2Name
            , const tCQCKit::ECmdPTypes     eParm3Type
            , const TString&                strParm3Name
        );

        TCQCCmd
        (
            const   TString&                strId
            , const TString&                strName
            , const tCQCKit::ECmdPTypes     eParm1Type
            , const TString&                strParm1Name
            , const tCQCKit::ECmdPTypes     eParm2Type
            , const TString&                strParm2Name
            , const tCQCKit::ECmdPTypes     eParm3Type
            , const TString&                strParm3Name
            , const tCQCKit::ECmdPTypes     eParm4Type
            , const TString&                strParm4Name
        );

        TCQCCmd(const TCQCCmd&) = default;

        ~TCQCCmd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCCmd& operator=(const TCQCCmd&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ParmCnt() const;

        TCQCCmdParm& cmdpAddParm
        (
            const   TCQCCmdParm&            cmdpToAdd
        );

        const TCQCCmdParm& cmdpAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TCQCCmdParm& cmdpAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCQCKit::ECmdTypes eType() const;

        tCQCKit::ECmdTypes eType
        (
            const   tCQCKit::ECmdTypes      eToSet
        );

        const TString& strId() const;

        const TString& strName() const;

        tCIDLib::TVoid Set
        (
            const   TString&                strId
            , const TString&                strName
            , const tCIDLib::TCard4         c4ParmCnt
        );

        tCIDLib::TVoid Set
        (
            const   TString&                strId
            , const TString&                strName
        );

        tCIDLib::TVoid SetParmAt
        (
            const   tCIDLib::TCard4         c4At
            , const TCQCCmdParm&            cmdpToSet
        );

        tCIDLib::TVoid SetParmReqAt
        (
            const   tCIDLib::TCard4         c4At
            , const tCIDLib::TBoolean       bToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4MaxParms
        //      In the ctor we get told how many parameters. Normally they are set
        //      up front, but not always, so we have to store this to limit them.
        //
        //  m_colParms
        //      A list of command parameters for this command.
        //
        //  m_eType
        //      Indicates whether this command has side effects and/or whether
        //      it returns a conditional result.  Defaults to a regular command
        //      (side effects but no return value.)
        //
        //  m_strId
        //      A command id that is independent of the human consumable
        //      name, which really represents the command.
        //
        //  m_strName
        //      The name of the commmand. It should be human readable for
        //      display in lists of commands to select from. So it should
        //      be loaded from message text.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4MaxParms;
        TVector<TCQCCmdParm>    m_colParms;
        tCQCKit::ECmdTypes      m_eType;
        TString                 m_strId;
        TString                 m_strName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCCmd,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCCmdCfg
//  PREFIX: ccfg
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCCmdCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //   CLASS: TParmInfo
        //  PREFIX: pi
        //
        //  A little class that comprises a single parameter in our list
        //  in memory. We just stream in/out the members ourself.
        // -------------------------------------------------------------------
        class CQCKITEXPORT TParmInfo
        {
            public :
                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TParmInfo();

                TParmInfo
                (
                    const   tCQCKit::ECmdPTypes eType
                    , const TString&            strValue
                );

                TParmInfo(const TParmInfo&) = default;

                ~TParmInfo();


                // -----------------------------------------------------------
                //  Public operators
                // -----------------------------------------------------------
                TParmInfo& operator=(const TParmInfo&) = default;


                // -----------------------------------------------------------
                //  Public, non-virtual methods
                // -----------------------------------------------------------
                tCIDLib::TBoolean bVal() const;

                tCIDLib::TCard4 c4Val
                (
                    const   tCIDLib::ERadices   eRadix = tCIDLib::ERadices::Auto
                )   const;

                tCIDLib::TInt4 i4Val
                (
                    const   tCIDLib::ERadices   eRadix = tCIDLib::ERadices::Auto
                )   const;

                tCIDLib::TVoid Reset();


                // -----------------------------------------------------------
                //  Public data members
                //
                //  m_eType
                //      The data type of this parameter
                //
                //  m_strValue
                //      The configured value for the parameter.
                // -----------------------------------------------------------
                tCQCKit::ECmdPTypes m_eType;
                TString             m_strValue;
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCCmdCfg();

        TCQCCmdCfg
        (
            const   TString&                strCmdId
            , const TString&                strCmdName
            , const TString&                strTargetId
            , const TString&                strTargetName
        );

        TCQCCmdCfg
        (
            const   TCQCCmd&                cmdSrc
        );

        TCQCCmdCfg
        (
            const   TCQCCmdCfg&             ccfgToCopy
        );

        ~TCQCCmdCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCCmdCfg& operator=
        (
            const   TCQCCmdCfg&             ccfgToAssign
        );

        tCIDLib::TBoolean operator==(const TCQCCmdCfg& ccfgSrc) const;

        tCIDLib::TBoolean operator!=(const TCQCCmdCfg& ccfgSrc) const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSearchNReplace
        (
            const   TString&                strFind
            , const TString&                strReplaceWith
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        tCIDLib::TCard4 c4ParmCnt() const;

        tCIDLib::TCard4 c4ParmCnt
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4TargetId() const;

        tCIDLib::TCard4 c4TargetId
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid FormatLongText
        (
                    TString&                strToFill
            , const tCIDLib::TBoolean       bAppend = kCIDLib::False
        )   const;

        tCIDLib::TVoid FormatShortText
        (
                    TString&                strToFill
            , const tCIDLib::TBoolean       bAppend = kCIDLib::False
        )   const;

        const TParmInfo& piAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TParmInfo& piAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid Reset();

        const TString& strCmdId() const;

        const TString& strCmdId
        (
            const   TString&                strToSet
        );

        const TString& strName() const;

        const TString& strName
        (
            const   TString&                strToSet
        );

        const TString& strTargetId() const;

        const TString& strTargetId
        (
            const   TString&                strToSet
        );

        const TString& strTargetName() const;

        const TString& strTargetName
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetFrom
        (
            const   TCQCCmd&                cmdSrc
            , const MCQCCmdSrcIntf&         mcsrcSender
            , const MCQCCmdTarIntf* const   pmctarInit
        );

        tCIDLib::TVoid SetParmAt
        (
            const   tCIDLib::TCard4         c4At
            , const TString&                strToSet
            , const tCQCKit::ECmdPTypes     eType
        );

        tCIDLib::TVoid SetParmAt
        (
            const   tCIDLib::TCard4         c4At
            , const TString&                strToSet
        );

        tCIDLib::TVoid SetParmAtAsRTV
        (
            const   tCIDLib::TCard4         c4At
            , const TString&                strRTVToSet
        );

        tCIDLib::TVoid SetTarget
        (
            const   MCQCCmdTarIntf&         mctarNew
        );

        tCIDLib::TVoid ToXML
        (
                    TTextOutStream&         strmTarget
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckParmIndex
        (
            const   tCIDLib::TCard4         c4ToCheck
            , const tCIDLib::TCard4         c4Line
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4ParmCnt
        //      Indicates the number of runtime parameters that the command
        //      we were configured for indicated it needed.
        //
        //  m_c4TargetId
        //      The m_strTargetId is persisted, but at runtime we use that
        //      to look up the unique numerical id that is assigned to each
        //      target as they are created. That numerical id is used to
        //      match us to our target at runtime, since the text id is not
        //      guaranteed to be unique in all cases (only within some parent
        //      container.)
        //
        //  m_colParms
        //      The list of parameters that will be sent to the target
        //      object. The number of items in this list will be defined
        //      by the m_c4ParmCount member. This guys is just allocated to
        //      the max possible parms, which is a small number.
        //
        //  m_strCmdId
        //      The id of the command we are going to send. It's gotten from
        //      the command object we were created for.
        //
        //  m_strCmdName
        //      We have to pull over the name of the command, for formatting
        //      purposes in the attributes tab. We have to be able to format
        //      out a short description of the command.
        //
        //  m_strTargetId
        //  m_strTargetName
        //      The id and name of the target object that this command is to
        //      be sent to. The id is an identifier that will uniquely
        //      identify it within the list of possible targets offered to
        //      the user. The name is for human readable identification.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4ParmCnt;
        tCIDLib::TCard4         m_c4TargetId;
        TObjArray<TParmInfo>    m_colParms;
        TString                 m_strCmdId;
        TString                 m_strName;
        TString                 m_strTargetId;
        TString                 m_strTargetName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCCmdCfg,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TActOpcode
//  PREFIX: aoc
//
//  We need a class to represent a single 'opcode' in the command sequence.
//  Each opcode is a conditional, a jump to an offset in the sequence, or a
//  command to carry out.
// ---------------------------------------------------------------------------
class CQCKITEXPORT TActOpcode : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TActOpcode();

        TActOpcode
        (
            const   tCQCKit::EActOps        eOpcode
        );

        TActOpcode
        (
            const   TActOpcode&             aocToCopy
        );

        ~TActOpcode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TActOpcode& aocSrc);

        tCIDLib::TBoolean operator==(const TActOpcode& aocSrc) const;

        tCIDLib::TBoolean operator!=(const TActOpcode& aocSrc) const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConditional
        (
            const   tCIDLib::TBoolean       bStartOnly
        )   const;

        tCIDLib::TVoid FormatLongTo
        (
                    TString&                strToFill
            , const tCIDLib::TBoolean       bAppend = kCIDLib::False
        );

        tCIDLib::TVoid FormatTo
        (
                    TString&                strToFill
            , const tCIDLib::TCard4         c4Nesting
            , const tCIDLib::TBoolean       bFlat = kCIDLib::False
            , const tCIDLib::TBoolean       bAppend = kCIDLib::False
        )   const;

        tCIDLib::TVoid FormatPrefixTo
        (
                    TString&                strToFill
            , const tCIDLib::TBoolean       bAppend = kCIDLib::False
        )   const;

        tCIDLib::TVoid Reset
        (
            const   tCQCKit::EActOps        eOpcode
        );


        // -------------------------------------------------------------------
        //  Pubic data members
        //
        //  m_bDisabled
        //      We allow commands to be temporarily disabled by setting this flag. It
        //      will be displayed greyed out in the editor and skipped by action engines.
        //
        //  m_bNegate
        //      If a conditional, this indicates whether the result should be negated.
        //      This provides a NOT type operator basically.
        //
        //  m_c4Id
        //      This is to support editing, where we need a way to map opcodes in a
        //      list to the list control that is displaying the formatted version
        //      of the data.
        //
        //  m_i4Ofs
        //      If this is a conditional or jump opcode, this is the offset to jump to.
        //      It is a relative value from the current offset.
        //
        //  m_eOpcode
        //      Indicates what type of opcode this is.
        //
        //  m_ccfgStep
        //      If a conditional or command opcode, this is the cmd config. If it's a
        //      conditional, then it's not for a target, it's just handled specially.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bDisabled;
        tCIDLib::TBoolean   m_bNegate;
        tCIDLib::TCard4     m_c4Id;
        tCIDLib::TInt4      m_i4Ofs;
        tCQCKit::EActOps    m_eOpcode;
        TCQCCmdCfg          m_ccfgStep;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TActOpcode,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: MCQCCmdTarIntf
//  PREFIX: mctar
// ---------------------------------------------------------------------------
class CQCKITEXPORT MCQCCmdTarIntf
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompByName
        (
            const   MCQCCmdTarIntf&         mctar1
            , const MCQCCmdTarIntf&         mctar2
        );

        static const TString& strKey
        (
            const   MCQCCmdTarIntf&         mctarSrc
        );


        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        virtual ~MCQCCmdTarIntf();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bQueryTarSubActNames
        (
                    TCollection<TString>&   colToFill
            , const TString&                strCmdId
            , const tCQCKit::EActCmdCtx     eContext
        )   const;

        virtual tCIDLib::TBoolean bValidateParm
        (
            const   TCQCCmd&                cmdSrc
            , const TCQCCmdCfg&             ccfgTar
            , const tCIDLib::TCard4         c4Index
            , const TString&                strValue
            ,       TString&                strErrText
        );

        virtual tCIDLib::TVoid CmdTarCleanup() = 0;

        virtual tCIDLib::TVoid CmdTarInitialize
        (
            const   TCQCActEngine&          acteTar
        ) = 0;

        virtual tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobalVars
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        ) = 0;

        virtual tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const = 0;

        virtual tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGUIDispatch() const;

        tCIDLib::TBoolean bGUIDispatch
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIsSpecialCmdTar() const;

        tCIDLib::TBoolean bIsSpecialCmdTar
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4UniqueId() const;

        const TString& strCmdHelpId() const;

        const TString& strCmdHelpId
        (
            const   TString&                strToSet
        );

        const TString& strCmdTargetId() const;

        const TString& strCmdTargetName() const;

        const TString& strCmdTargetType() const;

        tCIDLib::TVoid SetCmdTargetIds
        (
            const   TString&                strId
            , const TString&                strName
            , const TString&                strType
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        MCQCCmdTarIntf();

        MCQCCmdTarIntf
        (
            const   TString&                strTarId
            , const TString&                strTarName
            , const TString&                strTarType
            , const TString&                strHelpId
        );

        MCQCCmdTarIntf
        (
            const   MCQCCmdTarIntf&         mctarToCopy
        );

        tCIDLib::TVoid operator=
        (
            const   MCQCCmdTarIntf&         mctarToAssign
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ThrowUnknownCmd
        (
            const   TCQCCmdCfg&             ccfgBad
        )   const;

        tCIDLib::TVoid ThrowBadParmVal
        (
            const   TCQCCmdCfg&             ccfgBad
            , const tCIDLib::TCard4         c4ParmInd
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AssignId();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bGUIDispatch
        //      Some command targets cannot be directly dispatched to because
        //      they are GUI based. So they set this to let our engine know
        //      this and dispatch to them via the GUI. It defaults to false
        //      since most can be directly called.
        //
        //  m_bSpecial
        //      Some targets are 'special' targets, in that they are magic
        //      ones that should be displayed specially in the command config
        //      dialog. This allows them to be marked.
        //
        //  m_c4UniqueId
        //      As each target is created, it is assigned a unique id, from a
        //      a running 32 bit counter. The m_strId value below is not
        //      necessarily unique within a hierarhical list of targets, where
        //      it may be unique only within the parent container. We need a
        //      way to refer to any target. This is purely a runtime thing,
        //      used at runtime, not at design time. All command sources will
        //      be updated to refer to the unique id of it's target.
        //
        //  m_strHelpId
        //      This is an id that is used to map to a specific action target help
        //      file, to support context sensitive help in the action editor. This
        //      is set to the name of the XML (and hence HTML) help for for this
        //      target, and is used to build up a URL to that file.
        //
        //  m_strId
        //      The unique id that we provide for senders to store when they
        //      are configured to send us a command.
        //
        //  m_strName
        //      The human readable name we present in the list of targets.
        //
        //  m_strType
        //      Since we have to deal with targets via this generic interface,
        //      it can be tough later to find another or other targets of the
        //      same type. So, the derived class must provide a type id that
        //      is unique, and this can be used for this purpose.
        //
        //      For the standard targets, of which there is only ever one
        //      instance of each of them in a given action context at a time,
        //      they just use their command target ids, so in those cases the
        //      m_strId and m_strType will be the same.
        //
        //      But in other cases, such as interfaces, there are no standard
        //      target names since there can be many instances of a given
        //      type of target.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bGUIDispatch;
        tCIDLib::TBoolean   m_bSpecial;
        tCIDLib::TCard4     m_c4UniqueId;
        TString             m_strHelpId;
        TString             m_strId;
        TString             m_strName;
        TString             m_strType;


        // -------------------------------------------------------------------
        //  Private, static data
        // -------------------------------------------------------------------
        static TSafeCard4Counter    s_scntNextId;
};


//
//  We need to slip in a collection of command targets, since it's used
//  in the source class below.
//
namespace tCQCKit
{
    using TCmdTarList = TRefVector<MCQCCmdTarIntf>;
    using TCCmdTarList =  TRefVector<const MCQCCmdTarIntf>;
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCActEvInfo
//  PREFIX: caei
//
//  A simple class we use to store the main event info, which is a human readable
//  name, a unique id, and an action context.
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCActEvInfo : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCActEvInfo();

        TCQCActEvInfo
        (
            const   TString&                strName
            , const TString&                strEventId
            , const tCQCKit::EActCmdCtx     eContext
        );

        TCQCActEvInfo(const TCQCActEvInfo&) = default;
        TCQCActEvInfo(TCQCActEvInfo&&) = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCActEvInfo& operator=(const TCQCActEvInfo&) = default;
        TCQCActEvInfo& operator=(TCQCActEvInfo&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCActEvInfo&          caeiSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCActEvInfo&          caeiSrc
        )   const;


        tCQCKit::EActCmdCtx m_eContext;
        TString             m_strEventId;
        TString             m_strName;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCActEvInfo,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: MCQCCmdSrcIntf
//  PREFIX: mcsrc
// ---------------------------------------------------------------------------
class CQCKITEXPORT MCQCCmdSrcIntf
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TCard4 c4StdRTVCount();


        // -------------------------------------------------------------------
        //  Public types
        // -------------------------------------------------------------------
        using TOpcodeBlock =  TVector<TActOpcode>;
        using TEventList =  TVector<TCQCActEvInfo>;
        using TEventOpList =  TVector<TOpcodeBlock>;
        using TRTVList =  TVector<TCQCCmdRTVal>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        MCQCCmdSrcIntf();

        MCQCCmdSrcIntf
        (
            const   MCQCCmdSrcIntf&         mcsrcSrc
        );

        MCQCCmdSrcIntf
        (
                    MCQCCmdSrcIntf&&        mcsrcSrc
        );

        virtual ~MCQCCmdSrcIntf();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        MCQCCmdSrcIntf& operator=
        (
            const   MCQCCmdSrcIntf&         mcsrcSrc
        );

        MCQCCmdSrcIntf& operator=
        (
                    MCQCCmdSrcIntf&&        mcsrcSrc
        );


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid CmdSearchNReplace
        (
            const   TString&                strFind
            , const TString&                strReplaceWith
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        [[nodiscard]] virtual TCQCCmdRTVSrc* pcrtsMakeNew
        (
            const   TCQCUserCtx&            cuctxToUse
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddEvent
        (
            const   TString&                strName
            , const TString&                strId
            , const tCQCKit::EActCmdCtx     eContext
        );

        tCIDLib::TVoid AddRTValue
        (
            const   TString&                strName
            , const TString&                strId
            , const tCQCKit::ECmdPTypes     eType
        );

        const TActOpcode& aocAt
        (
            const   TString&                strEventId
            , const tCIDLib::TCard4         c4At
        )   const;

        TActOpcode& aocAt
        (
            const   TString&                strEventId
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TBoolean bHasAnyOps() const;

        tCIDLib::TBoolean bQueryOpsForEvent
        (
            const   TString&                strEventId
            ,       TOpcodeBlock&           colToFill
        )   const;

        tCIDLib::TBoolean bReferencesTarId
        (
            const   TString&                strId
        )   const;

        tCIDLib::TBoolean bRemoveRTValue
        (
            const   TString&                strId
        );

        tCIDLib::TBoolean bRTValExists
        (
            const   TString&                strId
        )   const;

        tCIDLib::TBoolean bSameCmds
        (
            const   MCQCCmdSrcIntf&         miwdgSrc
        )   const;

        tCIDLib::TBoolean bTakeMatchingEvents
        (
            const   MCQCCmdSrcIntf&         miwdgSrc
        );

        tCIDLib::TBoolean bUpdateCmdTarget
        (
            const   TString&                strOldTarId
            , const TString&                strNewTarName
            , const TString&                strNewTarId
        );

        tCIDLib::TCard4 c4EventCount() const;

        tCIDLib::TCard4 c4GetResolvedOpList
        (
            const   tCQCKit::TCmdTarList&   colTargets
            ,       MCQCCmdSrcIntf::TOpcodeBlock& colOpsToFill
            , const TString&                strEventId
        )   const;

        tCIDLib::TCard4 c4OpCount
        (
            const   TString&                strEventId
        )   const;

        tCIDLib::TCard4 c4OpCountAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TCard4 c4RTValCount() const;

        const TCQCActEvInfo& caeiEventAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TOpcodeBlock& colOpsAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TOpcodeBlock& colOpsAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TCQCCmdRTVal& crtvAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TCQCCmdRTVal& crtvFind
        (
            const   TString&                strId
        )   const;

        tCIDLib::TVoid DeleteEvent
        (
            const   TString&                strEventId
        );

        tCIDLib::TVoid DeleteEventAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCQCKit::EActCmdCtx eContextOfEvent
        (
            const   TString&                strEventId
        )   const;

        tCIDLib::TVoid FormatOpsForEvent
        (
            const   TString&                strEventId
            ,       TTextOutStream&         strmToFill
            , const tCIDLib::TCard4         c4IndentOfs = 0
        )   const;

        const TOpcodeBlock* pcolOpsForEvent
        (
            const   TString&                strEventId
        )   const;

        TOpcodeBlock* pcolOpsForEvent
        (
            const   TString&                strEventId
        );

        const TCQCCmdRTVal* pcrtvFind
        (
            const   TString&                strId
        )   const;

        tCIDLib::TVoid QueryCmdAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
            , const tCIDLib::TBoolean       bAddSep
            , const tCIDLib::TBoolean       bRequired
            , const TString&                strItemText
        )   const;

        tCIDLib::TVoid ReadInOps
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid ResetAllOps
        (
            const   tCIDLib::TBoolean       bRemove
        );

        tCIDLib::TVoid ResetOps
        (
            const   TString&                strEventId
        );

        tCIDLib::TVoid ResetRTVals();

        const TString& strEventIdAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TString& strRTValNameAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid SetCmdAttr
        (
            const   TAttrData&              adatNew
            , const TAttrData&              adatOld
        );

        tCIDLib::TVoid SetEventContext
        (
            const   TString&                strId
            , const tCQCKit::EActCmdCtx     eContext
        );

        tCIDLib::TVoid SetEventOps
        (
            const   TString&                strEventId
            , const TOpcodeBlock&           colNewOps
        );

        tCIDLib::TVoid SetEventOps
        (
            const   TString&                strEventId
            , const TRefCollection<TActOpcode>& colNewOps
        );

        tCIDLib::TVoid UpdateOps
        (
            const   TString&                strOldCmdId
            , const MCQCCmdTarIntf&         mctarNewTar
            , const TString&                strNewTarId
            , const TString&                strNewCmdId
        );

        tCIDLib::TVoid WriteOutOps
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        TCQCActEvInfo* pcaeiFindEv
        (
            const   TString&                strEvent
            ,       tCIDLib::TCard4&        c4Index
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        const TCQCActEvInfo* pcaeiFindEv
        (
            const   TString&                strEvent
            ,       tCIDLib::TCard4&        c4Index
            , const tCIDLib::TBoolean       bThrowIfNot
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colEvents
        //      The list of events set for this source.
        //
        //  m_colEventOps
        //      The opcodes that have been configured for this source to
        //      send. It's a vector of opcode blocks. There is one entry
        //      in here for each one in m_colEvents.
        //
        //  m_colRTVals
        //      The list of runtime values we currently have. These are the
        //      definitions of the RTVs, not the actual values at runtime.
        //      That is provided by the TCQCCmdRTVSrc and it's derivatives.
        //      We don't persist these. The derived classes just set them
        //      as required at creation.
        // -------------------------------------------------------------------
        TEventList      m_colEvents;
        TEventOpList    m_colEventOps;
        TRTVList        m_colRTVals;
};




// ---------------------------------------------------------------------------
//   CLASS: MCQCCmdTracer
//  PREFIX: mcmdt
// ---------------------------------------------------------------------------
class CQCKITEXPORT MCQCCmdTracer
{
    public  :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        virtual ~MCQCCmdTracer() {}


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid ActCmd
        (
            const   TActOpcode&             aocOriginal
            , const TCQCCmdCfg&             ccfgExpanded
            , const tCIDLib::TCard4         c4Step
        ) = 0;

        virtual tCIDLib::TVoid ActDebug
        (
            const   TString&                strMsg
        ) = 0;

        virtual tCIDLib::TVoid ActEnd
        (
            const   tCIDLib::TBoolean       bStatus
        ) = 0;

        virtual tCIDLib::TVoid ActStartNew
        (
            const   TStdVarsTar&            ctarGlobals
        ) = 0;

        virtual tCIDLib::TVoid ActVarRemoved
        (
            const   TString&                strKey
        ) = 0;

        virtual tCIDLib::TVoid ActVarSet
        (
            const   TString&                strKey
            , const TString&                strValue
        ) = 0;

        virtual tCIDLib::TBoolean bLocked() const = 0;

        virtual tCIDLib::TBoolean bLocked
        (
            const   tCIDLib::TBoolean       bToSet
        ) = 0;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        MCQCCmdTracer() = default;
        MCQCCmdTracer(const MCQCCmdTracer&) = default;
        MCQCCmdTracer(MCQCCmdTracer&&) = default;
        MCQCCmdTracer& operator=(const MCQCCmdTracer&) = default;
        MCQCCmdTracer& operator=(MCQCCmdTracer&&) = default;
};


// ---------------------------------------------------------------------------
//   CLASS: TActTraceJan
//  PREFIX: jan
// ---------------------------------------------------------------------------
class CQCKITEXPORT TActTraceJan
{
    public :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TActTraceJan(MCQCCmdTracer* const pmcmdtTrace) :

            m_pmcmdtTrace(0)
        {
            if (pmcmdtTrace)
            {
                m_pmcmdtTrace = pmcmdtTrace;
                m_pmcmdtTrace->bLocked(kCIDLib::True);
            }
        }

        TActTraceJan(const TActTraceJan&) = delete;
        TActTraceJan(TActTraceJan&&) = delete;

        ~TActTraceJan()
        {
            if (m_pmcmdtTrace)
                m_pmcmdtTrace->bLocked(kCIDLib::False);
        }


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TActTraceJan& operator=(const TActTraceJan&);
        TActTraceJan& operator=(TActTraceJan&&);


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        MCQCCmdTracer* pmcmdtTrace() const
        {
            return m_pmcmdtTrace;
        }


    private :
        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        MCQCCmdTracer*  m_pmcmdtTrace;
};

#pragma CIDLIB_POPPACK


