//
// FILE NAME: CQCKit_VarsTarget.cpp
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
//  One of the standard action command targets. Most are in CQCAct, but this one has
//  be to down here.
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
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TStdVarsTar,TObject)



// ---------------------------------------------------------------------------
//  CLASS: TStdVarsTar
// PREFIX: ctar
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TStdVarsTar: Public, static methods
// ---------------------------------------------------------------------------

//
//  Check a variable name for validity, optionally requiring that it have a
//  specific prefix. It also checks to see if it's a reference to one and returns
//  a 'by value' flag and the actual variable name. If it's just a literal variable
//  name, then it just gives back that incoming value.
//
tCIDLib::TBoolean
TStdVarsTar::bValidVarName( const   TString&    strToCheck
                            , const TString&    strPrefix
                            ,       TString&    strReason)
{
    //
    //  This one is for the common scenario where they only care if it's a valid
    //  variable name and it must be by name. We just eat the output vars and
    //  indicate deref is not ok.
    //
    tCIDLib::TBoolean bByValue;
    TString strName;
    return bValidVarName
    (
        strToCheck, strName, strPrefix, strReason, bByValue, kCIDLib::False
    );
}

tCIDLib::TBoolean
TStdVarsTar::bValidVarName( const   TString&            strToCheck
                            ,       TString&            strVarName
                            ,       TString&            strReason
                            ,       tCIDLib::TBoolean&  bByValue
                            , const tCIDLib::TBoolean   bDerefOK)
{
    return bValidVarName
    (
        strToCheck, strVarName, TString::strEmpty(), strReason, bByValue, bDerefOK
    );
}

tCIDLib::TBoolean
TStdVarsTar::bValidVarName( const   TString&            strToCheck
                            ,       TString&            strVarName
                            , const TString&            strPrefix
                            ,       TString&            strReason
                            ,       tCIDLib::TBoolean&  bByValue
                            , const tCIDLib::TBoolean   bDerefOK)
{
    bByValue = kCIDLib::False;

    //
    //  First see if it's a reference. If so, then remove the wrapper and set
    //  the by value flag.
    //
    strVarName = strToCheck;
    if ((strVarName.c4Length() > 3)
    &&  (strVarName[0] == kCIDLib::chPercentSign)
    &&  (strVarName[1] == kCIDLib::chOpenParen)
    &&  (strVarName.chLast() == kCIDLib::chCloseParen))
    {
        if (!bDerefOK)
        {
            strReason.LoadFromMsg(kKitErrs::errcCmd_NoDeref, facCQCKit());
            return kCIDLib::False;
        }

        strVarName.Cut(0, 2);
        strVarName.DeleteLast();

        bByValue = kCIDLib::True;
    }

    //
    //  If they passed a specific prefix to check, compare to that. Else,
    //  make sure it's one of the valid prefixes.
    //
    if (!strPrefix.bIsEmpty())
    {
        if (!strVarName.bStartsWith(strPrefix))
        {
            strReason.LoadFromMsg
            (
                kKitErrs::errcCmd_WrongVarPref, facCQCKit(), strPrefix
            );
            return kCIDLib::False;
        }

        // It can't just be the prefix
        if (strVarName.c4Length() == strPrefix.c4Length())
        {
            strReason.LoadFromMsg(kKitErrs::errcCmd_NoVarName, facCQCKit());
            return kCIDLib::False;
        }
    }
     else
    {
        if (!strVarName.bStartsWith(kCQCKit::strActVarPref_GVar)
        &&  !strVarName.bStartsWith(kCQCKit::strActVarPref_LVar))
        {
            strReason.LoadFromMsg
            (
                kKitErrs::errcCmd_UnknownVarPref
                , facCQCKit()
                , kCQCKit::strActVarPref_GVar
                , kCQCKit::strActVarPref_LVar
            );
            return kCIDLib::False;
        }
    }

    //
    //  It starts with the right prefix, so check some other stuff. It has
    //  to have one colon and one colon only.
    //
    tCIDLib::TCard4 c4First;
    tCIDLib::TCard4 c4Next;
    if (!strVarName.bFirstOccurrence(kCIDLib::chColon, c4First))
    {
        strReason = L"Variable names must be a prefix, a colon, and a name";
        return kCIDLib::False;
    }

    c4Next = c4First;
    if (strVarName.bNextOccurrence(kCIDLib::chColon, c4Next))
    {
        strReason = L"Variable names cannot have more than one colon in them";
        return kCIDLib::False;
    }

    // That colon cannot be the first or last character
    const tCIDLib::TCard4 c4Len = strVarName.c4Length();
    if (!c4First || (c4First == c4Len - 1))
    {
        strReason = L"Variable names must be a prefix, a colon, and a name";
        return kCIDLib::False;
    }

    // Make sure it has only valid characters
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Len; c4Index++)
    {
        const tCIDLib::TCh chCur = strVarName[c4Index];
        if (chCur == L':')
            continue;

        //
        //  If it's the first char of the prefix or suffix, it has to be
        //  alpha. Else, it has to be alpha or underscrore, dash or space.
        //
        if (!c4Index || (c4Index == c4First + 1))
        {
            if (!TRawStr::bIsAlphaNum(chCur))
            {
                strReason = L"The first character of each part of a variable name must be a character";
                return kCIDLib::False;
            }
        }
         else if (!TRawStr::bIsAlphaNum(chCur)
              &&  ((chCur != L'-') && (chCur != L'_') && (chCur != L' ')))
        {
            strReason = L"Variable names must be alpha numeric, hyphen, underscore, or space";
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  Since targets get both a local and global vars target, and have to figure
//  out which one has a variable they need to deal with, we provide some simple
//  helpers find and/or create them in the approporiate list.
//
tCIDLib::TBoolean
TStdVarsTar::bVarExists(const   TString&        strToFind
                        ,       TStdVarsTar&    ctarLocals
                        ,       TStdVarsTar&    ctarGlobals)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (strToFind.bStartsWith(kCQCKit::strActVarPref_LVar))
    {
        bRet = ctarLocals.m_colVars.bKeyExists(strToFind);
    }
     else if (strToFind.bStartsWith(kCQCKit::strActVarPref_GVar))
    {
        bRet = ctarGlobals.m_colVars.bKeyExists(strToFind);
    }
     else
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_UnknownVarPref
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , kCQCKit::strActVarPref_LVar
            , kCQCKit::strActVarPref_GVar
        );
    }
    return bRet;
}


TCQCActVar&
TStdVarsTar::varFind(const  TString&            strToFind
                    ,       TStdVarsTar&        ctarLocals
                    ,       TStdVarsTar&        ctarGlobals
                    , const tCIDLib::TCh* const pszFile
                    , const tCIDLib::TCard4     c4Line
                    , const tCIDLib::TBoolean   bCreateIfNot)
{
    TCQCActVar* pvarRet = 0;
    if (strToFind.bStartsWith(kCQCKit::strActVarPref_LVar))
    {
        pvarRet = ctarLocals.m_colVars.pobjFindByKey(strToFind, kCIDLib::False);

        //
        //  If not found, see if it's an action parameter. If so, then it
        //  cannot be faulted in.
        //
        if (!pvarRet && strToFind.bStartsWith(kCQCKit::strActVarPref_Parm))
            ThrowNotFound(strToFind, pszFile, c4Line);

        // Otherwise, if allowed we can fault it in
        if (!pvarRet && bCreateIfNot)
        {
            ctarLocals.bAddOrUpdateVar
            (
                strToFind
                , TString::strEmpty()
                , tCQCKit::EFldTypes::String
                , TString::strEmpty()
            );
            pvarRet = ctarLocals.m_colVars.pobjFindByKey(strToFind);
        }
    }
     else if (strToFind.bStartsWith(kCQCKit::strActVarPref_GVar))
    {
        pvarRet = ctarGlobals.m_colVars.pobjFindByKey(strToFind, kCIDLib::False);
        if (!pvarRet && bCreateIfNot)
        {
            ctarGlobals.bAddOrUpdateVar
            (
                strToFind
                , TString::strEmpty()
                , tCQCKit::EFldTypes::String
                , TString::strEmpty()
            );
            pvarRet = ctarGlobals.m_colVars.pobjFindByKey(strToFind);
        }
    }
     else
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_UnknownVarPref
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , kCQCKit::strActVarPref_LVar
            , kCQCKit::strActVarPref_GVar
        );
    }

    if (!pvarRet)
        ThrowNotFound(strToFind, pszFile, c4Line);
    return *pvarRet;
}

const TCQCActVar&
TStdVarsTar::varFind(const  TString&            strToFind
                    , const TStdVarsTar&        ctarLocals
                    , const TStdVarsTar&        ctarGlobals
                    , const tCIDLib::TCh* const pszFile
                    , const tCIDLib::TCard4     c4Line)
{
    const TCQCActVar* pvarRet = 0;
    if (strToFind.bStartsWith(kCQCKit::strActVarPref_LVar))
    {
        pvarRet = ctarLocals.m_colVars.pobjFindByKey(strToFind, kCIDLib::False);
        if (!pvarRet)
            ThrowNotFound(strToFind, CID_FILE, CID_LINE);
    }
     else if (strToFind.bStartsWith(kCQCKit::strActVarPref_GVar))
    {
        pvarRet = ctarGlobals.m_colVars.pobjFindByKey(strToFind, kCIDLib::False);
        if (!pvarRet)
            ThrowNotFound(strToFind, CID_FILE, CID_LINE);
    }
     else
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_UnknownVarPref
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , kCQCKit::strActVarPref_LVar
            , kCQCKit::strActVarPref_GVar
        );
    }

    if (!pvarRet)
        ThrowNotFound(strToFind, pszFile, c4Line);
    return *pvarRet;
}


//
//  An atomic create, test, and set method. It's used by the test and set
//  action command below. The variable must be boolean if it already exists,
//  else we create a boolean. If the variable is already set, we return
//  false. Else we set it and return true.
//
TCQCActVar&
TStdVarsTar::varTestAndSet( const   TString&            strToFind
                            ,       TStdVarsTar&        ctarGlobals
                            , const tCIDLib::TCh* const pszFile
                            , const tCIDLib::TCard4     c4Line
                            ,       tCIDLib::TBoolean&  bResult)
{
    TCQCActVar* pvarRet = 0;

    if (strToFind.bStartsWith(kCQCKit::strActVarPref_GVar))
    {
        pvarRet = ctarGlobals.m_colVars.pobjFindByKey(strToFind, kCIDLib::False);

        // If found, check that it's a boolean, which is must be
        if (pvarRet)
        {
            if (pvarRet->eType() != tCQCKit::EFldTypes::Boolean)
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcCmd_BadTestSetVar
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::TypeMatch
                );
            }
        }

        //
        //  OK, do the atomic test update. If it works, it returns true,
        //  else it hasn't set it (someone else has it set), so it returns
        //  false.
        //
        bResult = ctarGlobals.bAddOrUpdateVar
        (
            strToFind
            , kCQCKit::pszFld_True
            , tCQCKit::EFldTypes::Boolean
            , TString::strEmpty()
        );

        // And now look it up so we can return it
        pvarRet = ctarGlobals.m_colVars.pobjFindByKey(strToFind);
    }
     else
    {
        // We only work on globals
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_OnlyGlobalVars
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    if (!pvarRet)
        ThrowNotFound(strToFind, pszFile, c4Line);
    return *pvarRet;
}



// ---------------------------------------------------------------------------
//  TStdVarsTar: Constructors and Destructor
// ---------------------------------------------------------------------------
TStdVarsTar::TStdVarsTar(const tCIDLib::EMTStates eMTSafe, const tCIDLib::TBoolean bLocal) :

    MCQCCmdTarIntf
    (
        bLocal ? kCQCKit::strCTarId_LVars : kCQCKit::strCTarId_GVars
        , facCQCKit().strMsg
          (
            bLocal ? kKitMsgs::midCmdTar_LVars : kKitMsgs::midCmdTar_GVars
          )
        , bLocal ? kCQCKit::strCTarId_LVars : kCQCKit::strCTarId_GVars
        , L"/Reference/Actions/Variables"
    )
    , m_bIsLocal(bLocal)
    , m_colVars
      (
        tCIDLib::EAdoptOpts::Adopt, 67, TStringKeyOps(), &TCQCActVar::strKey, eMTSafe
      )
    , m_c4NextId(1)
    , m_strPrefix(bLocal ? kCQCKit::strActVarPref_LVar : kCQCKit::strActVarPref_GVar)
{
    // Set the 'special' flag on us
    bIsSpecialCmdTar(kCIDLib::True);
}

TStdVarsTar::~TStdVarsTar()
{
}


// ---------------------------------------------------------------------------
//  TStdVarsTar: Public, inherited methods
// ---------------------------------------------------------------------------

// We override this to do some parameter checking.
tCIDLib::TBoolean
TStdVarsTar::bValidateParm( const   TCQCCmd&        cmdSrc
                            , const TCQCCmdCfg&     ccfgTar
                            , const tCIDLib::TCard4 c4Index
                            , const TString&        strValue
                            ,       TString&        strErrText)
{
    // Let the command mixin do the most basic stuff
    if (!MCQCCmdTarIntf::bValidateParm(cmdSrc, ccfgTar, c4Index, strValue, strErrText))
        return kCIDLib::False;

    // That's ok, so do our own stuff
    if ((ccfgTar.strCmdId() == kCQCKit::strCmdId_IsCharAt)
    ||  (ccfgTar.strCmdId() == kCQCKit::strCmdId_PutCharAt))
    {
        // Has to be a single character, can't be empty
        if (c4Index == 2)
        {
            if (strValue.c4Length() != 1)
            {
                strErrText.LoadFromMsg(kKitErrs::errcCmd_SingleChar, facCQCKit());
                return kCIDLib::False;
            }
        }
    }
     else if (ccfgTar.strCmdId() == kCQCKit::strCmdId_Replace)
    {
        if ((c4Index == 1) || (c4Index == 2))
        {
            // These both have to be single character (can be empty)
            if (strValue.c4Length() > 1)
            {
                strErrText.LoadFromMsg(kKitErrs::errcCmd_SingleChar, facCQCKit());
                return kCIDLib::False;
            }
        }
    }
     else if (ccfgTar.strCmdId() == kCQCKit::strCmdId_SetVarFmt)
    {
        if (c4Index == 2)
        {
            // Make sure the digits are from 0 to 8
            tCIDLib::TCard4 c4Digits;
            if (!strValue.bToCard4(c4Digits) || (c4Digits > 8))
            {
                strErrText.LoadFromMsg(kKitErrs::errcCmd_TooManyDecDigits, facCQCKit());
                return kCIDLib::False;
            }
        }
    }
     else if (ccfgTar.strCmdId() == kCQCKit::strCmdId_TestAndSet)
    {
        if (c4Index == 1)
        {
            // Has to be a value from zero to 15000
            tCIDLib::TCard4 c4Millis;
            if (!strValue.bToCard4(c4Millis) || (c4Millis > 15000))
            {
                strErrText.LoadFromMsg
                (
                    kKitErrs::errcCmd_ParmRangeErr, facCQCKit()
                    , TInteger(0)
                    , TCardinal(15000)
                );
                return kCIDLib::False;
            }
        }
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TStdVarsTar::CmdTarCleanup()
{
    // A no-op for us
}


// Called when we are about to be re-used for a new action
tCIDLib::TVoid TStdVarsTar::CmdTarInitialize(const TCQCActEngine& acteToUse)
{
    //
    //  If local, then remove any existing contents and store away the incoming
    //  engine's action parameters, which we expose as local vars.
    //
    if (m_bIsLocal)
    {
        m_colVars.RemoveAll();
        acteToUse.QueryActParms(*this);
    }
}


//
//  Do the requested command. We have no parent command handler so we throw
//  if it's not one of ours.
//
//  Most of the variables we can just assume belong to us, so we don't use
//  the passed variables to get those (the locals are implicitly passed in
//  the action engine. But, in some cases, a source or target variable
//  doesn't have be from our set, such as when a variable is assigned to
//  another variable. The source could be of the other type.
//
tCQCKit::ECmdRes
TStdVarsTar::eDoCmd(const   TCQCCmdCfg&         ccfgToDo
                    , const tCIDLib::TCard4
                    , const TString&
                    , const TString&
                    ,       TStdVarsTar&        ctarGlobalVars
                    ,       tCIDLib::TBoolean&  bResult
                    ,       TCQCActEngine&      acteTar)
{
    static const tCIDLib::TCh* pszFile = CID_FILE;

    // Lock our global variables while we do this
    TLocker lockrVars(&ctarGlobalVars.m_colVars);

    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_AdjustEnumValue)
    {
        //
        //  We'll move an enumerated variable to the next or previous value. Actually
        //  it'll work these days for any variable with a limit that responds meaningfully
        //  to the bNextPrevVal() method.
        //
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFind
        (
            strLHSKey, acteTar.ctarLocals(), ctarGlobalVars, pszFile, CID_LINE, kCIDLib::False
        );

        // Get the limit object for this guy
        const TCQCFldLimit& fldlEnum = varLHS.fldlVar();

        //
        //  The enum limit class has a helper for this. Set our result to whether it
        //  was changed or not.
        //
        TString strNewVal;
        bResult = fldlEnum.bNextPrevVal
        (
            varLHS.strValue()
            , strNewVal
            , ccfgToDo.piAt(1).m_strValue == L"Next"
            , facCQCKit().bCheckBoolVal(ccfgToDo.piAt(2).m_strValue)
        );

        // If it changed, then store it
        if (bResult)
        {
            varLHS.bSetValue(strNewVal);
            if (acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varLHS.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Add)
    {
        // Get the target variable
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);
        if (varLHS.bAdd(ccfgToDo.piAt(1).m_strValue) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varLHS.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_AddQListValue)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFind
        (
            strLHSKey, acteTar.ctarLocals(), ctarGlobalVars, pszFile, CID_LINE, kCIDLib::True
        );

        m_strTmp = varLHS.strValue();
        TStringTokenizer::BuildQuotedCommaList
        (
            ccfgToDo.piAt(1).m_strValue, m_strTmp
        );

        if (varLHS.bSetValue(m_strTmp) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, m_strTmp);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_AND)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);
        if (varLHS.bBooleanOp(ccfgToDo.piAt(1).m_strValue, tCQCKit::ELogOps::AND) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varLHS.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Append)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);
        if (varLHS.bAppendValue(ccfgToDo.piAt(1).m_strValue) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varLHS.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_CapAt)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);

        const tCIDLib::TCard4 c4At = ccfgToDo.piAt(1).m_strValue.c4Val();
        if (varLHS.bCapAt(c4At) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varLHS.strValue());
    }
     else if ((ccfgToDo.strCmdId() == kCQCKit::strCmdId_CreateVariable)
          ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SafeCreateVariable))
    {
        // We get a name, a data type, a limit string, and initial value
        const TString& strName = ccfgToDo.piAt(0).m_strValue;
        const TString& strType = ccfgToDo.piAt(1).m_strValue;
        const TString& strLimits = ccfgToDo.piAt(2).m_strValue;
        const TString& strInitVal = ccfgToDo.piAt(3).m_strValue;

        // Translate the type value
        const tCQCKit::EFldTypes eType = tCQCKit::eAltXlatEFldTypes(strType);
        if (eType == tCQCKit::EFldTypes::Count)
        {
            facCQCKit().ThrowErr
            (
                pszFile
                , CID_LINE
                , kKitErrs::errcFld_UnknownType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
                , strType
            );
        }

        // If creating one, then it cannot already exist
        if (bVarExists(strName, acteTar.ctarLocals(), ctarGlobalVars)
        &&  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_CreateVariable))
        {
            facCQCKit().ThrowErr
            (
                pszFile
                , CID_LINE
                , kKitErrs::errcCmd_VarAlreadyExists
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , strName
            );
        }

        // We can create it or add it
        bAddOrUpdateVar(strName, strInitVal, eType, strLimits);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_CreateVarFromField)
    {
        // Find the server hosting the indicated field
        const TString& strTarFld = ccfgToDo.piAt(1).m_strValue;
        TString strMoniker;
        TString strField;
        facCQCKit().ParseFldName(strTarFld, strMoniker, strField);

        //
        //  And ask for the field definition info for this field, and read the value
        //  as text.
        //
        tCQCKit::EFldTypes  eType;
        TCQCFldDef          flddTmp;
        TString             strInitVal;
        {
            tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));
            orbcAdmin->QueryFieldDef(strMoniker, strField, flddTmp);

            tCIDLib::TCard4 c4SerNum = 0;
            orbcAdmin->bReadFieldByName(c4SerNum, strMoniker, strField, strInitVal, eType);
        }

        // And now create the variable
        bAddOrUpdateVar(ccfgToDo.piAt(0).m_strValue, strInitVal, eType, flddTmp.strLimits());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_DelSubStr)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);

        const tCIDLib::TCard4 c4Start = ccfgToDo.piAt(1).m_strValue.c4Val();
        const tCIDLib::TCard4 c4Count = ccfgToDo.piAt(2).m_strValue.c4Val();

        if (varLHS.bDelSubStr(c4Start, c4Count) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varLHS.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_DelVariable)
    {
        // Remove this key if it exists. Return true if removed
        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        bResult = m_colVars.bRemoveKeyIfExists(strKey);
        if (bResult && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarRemoved(strKey);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Divide)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);

        if (varLHS.bDivide(ccfgToDo.piAt(1).m_strValue) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varLHS.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ElapsedSince)
    {
        // The two last parms must be time based variables
        TCQCActVar& varFirst = varFind
        (
            ccfgToDo.piAt(2).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::False
        );

        TCQCActVar& varSecond = varFind
        (
            ccfgToDo.piAt(3).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::False
        );

        // And it must be of time type
        CheckVarType(varFirst, tCQCKit::EFldTypes::Time);
        CheckVarType(varSecond, tCQCKit::EFldTypes::Time);

        // Get the target, which doesn't have to exist
        TCQCActVar& varTar = varFind
        (
            ccfgToDo.piAt(0).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::True
        );

        // Get the variable values as their real types we know they are now
        const TCQCTimeFldValue& fvFirst
        (
            static_cast<const TCQCTimeFldValue&>(varFirst.fvCurrent())
        );
        const TCQCTimeFldValue& fvSecond
        (
            static_cast<const TCQCTimeFldValue&>(varSecond.fvCurrent())
        );

        // Get the type, verify and do the calculations as we go
        const TString& strType = ccfgToDo.piAt(1).m_strValue;
        tCIDLib::TCard4 c4Elapsed = 0;
        if (strType == L"Seconds")
        {
            if (fvSecond.c8Value() > fvFirst.c8Value())
            {
                c4Elapsed = tCIDLib::TCard4
                (
                    (fvSecond.c8Value() - fvFirst.c8Value()) / kCIDLib::enctOneSecond
                );
            }
        }
         else if (strType == L"Minutes")
        {
            if (fvSecond.c8Value() > fvFirst.c8Value())
            {
                c4Elapsed = tCIDLib::TCard4
                (
                    (fvSecond.c8Value() - fvFirst.c8Value()) / kCIDLib::enctOneMinute
                );
            }
        }
         else
        {
            facCQCKit().ThrowErr
            (
                pszFile
                , CID_LINE
                , kKitErrs::errcCmd_BadEnumVal
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , strType
            );
        }

        if (varTar.bSetValue(c4Elapsed) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Exists)
    {
        bResult = bVarExists
        (
            ccfgToDo.piAt(0).m_strValue, acteTar.ctarLocals(), ctarGlobalVars
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Find)
    {
        //
        //  Get the source var (to search), the pattern to find, and the
        //  case sensitive value.
        //
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);

        const TString& strPat = ccfgToDo.piAt(2).m_strValue;
        const TString& strCase = ccfgToDo.piAt(3).m_strValue;
        const tCIDLib::TBoolean bCaseSensitive(facCQCKit().bCheckBoolVal(strCase));

        // Find the pattern substring in the source value
        tCIDLib::TCard4 c4At;
        bResult = varLHS.strValue().bFirstOccurrence
        (
            strPat,c4At, kCIDLib::False, bCaseSensitive
        );

        // Set the result, either the index or empty
        if (!bResult)
            c4At = kCIDLib::c4MaxCard;

        TCQCActVar& varTar = varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::True
        );

        m_strTmp.SetFormatted(c4At);
        if (varTar.bSetValue(m_strTmp) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetEnumOrdinal)
    {
        // Get the source variable
        const TString& strSrcKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varSrc = varFind
        (
            strSrcKey, acteTar.ctarLocals(), ctarGlobalVars, pszFile, CID_LINE, kCIDLib::False
        );

        if ((varSrc.eType() != tCQCKit::EFldTypes::String)
        ||  !varSrc.fldlVar().bIsA(TCQCFldEnumLimit::clsThis()))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_MustBeEnum
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , strSrcKey
            );
        }

        // Get the field limits for this variable and cast to the real type
        const TCQCFldEnumLimit& fldlEnum = dynamic_cast<const TCQCFldEnumLimit&>
        (
            varSrc.fldlVar()
        );

        // Get the target variable
        const TString& strTarKey = ccfgToDo.piAt(1).m_strValue;
        TCQCActVar& varTar = varFind
        (
            strTarKey, acteTar.ctarLocals(), ctarGlobalVars, pszFile, CID_LINE, kCIDLib::True
        );

        if (varTar.bSetValue(fldlEnum.c4QueryOrdinal(varSrc.strValue())))
        {
            if (acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(strTarKey, varTar.strValue());
        }
    }
     else if ((ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetLength)
          ||  (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetText))
    {
        // Find the source variable
        const TString& strSrcKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varSrc= varFindIt(strSrcKey, CID_LINE);

        //
        //  The target variable might not be of our type, so use the general
        //  purpose lookup method.
        //
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::True
        );

        if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetLength)
        {
            // In this case we put the length of the source into the target
            bResult = varTar.bSetValue(varSrc.strValue().c4Length());
        }
         else
        {
            // In this case we put the value of the source into the target
            bResult = varTar.bSetValue(varSrc.strValue());
        }

        if (acteTar.pcmdtDebug() && bResult)
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetNthEnumVal)
    {
        // Look up the referenced variable
        TCQCActVar& varSrc = varFindIt(ccfgToDo.piAt(0).m_strValue, CID_LINE);

        TString strVal;
        varSrc.QueryNthEnumVal(ccfgToDo.piAt(1).m_strValue.c4Val(), strVal);

        // And put the result into the target variable
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            ccfgToDo.piAt(2).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::True
        );

        if (varTar.bSetValue(strVal) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetNumericRange)
    {
        // Look up the referenced variable
        TCQCActVar& varSrc = varFindIt(ccfgToDo.piAt(0).m_strValue, CID_LINE);
        varSrc.QueryNumericLimits(m_strTmp, m_strTmp2);

        // And store the values away
        TCQCActVar& varMinVal = varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::True
        );
        if (varMinVal.bSetValue(m_strTmp) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varMinVal.strName(), varMinVal.strValue());

        TCQCActVar& varMaxVal = varFind
        (
            ccfgToDo.piAt(2).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::True
        );
        if (varMaxVal.bSetValue(m_strTmp) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varMaxVal.strName(), varMaxVal.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetSubStr)
    {
        TCQCActVar& varSrc = varFindIt(ccfgToDo.piAt(0).m_strValue, CID_LINE);
        const tCIDLib::TCard4 c4Start = ccfgToDo.piAt(1).m_strValue.c4Val();
        TCQCActVar& varTar = varFind
        (
            ccfgToDo.piAt(3).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::True
        );

        // If the count is zero, set to get the rest
        tCIDLib::TCard4 c4Count = ccfgToDo.piAt(2).m_strValue.c4Val();
        if (!c4Count)
            c4Count = kCIDLib::c4MaxCard;

        varSrc.strValue().CopyOutSubStr(m_strTmp, c4Start, c4Count);
        if (varTar.bSetValue(m_strTmp) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Insert)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varTar = varFindIt(strLHSKey, CID_LINE);
        const tCIDLib::TCard4 c4At = ccfgToDo.piAt(2).m_strValue.c4Val();
        if (varTar.bInsertValue(ccfgToDo.piAt(1).m_strValue, c4At) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_IsCharAt)
    {
        const TString& strVal = ccfgToDo.piAt(0).m_strValue;
        const tCIDLib::TCard4 c4Index = ccfgToDo.piAt(1).m_strValue.c4Val();
        const tCIDLib::TCh chChar = ccfgToDo.piAt(2).m_strValue[0];

        // If the index is invalid, then return false, else do the comparison
        if (c4Index >= strVal.c4Length())
            bResult = kCIDLib::False;
         else
            bResult = (strVal[c4Index] == chChar);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Multiply)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);

        if (varLHS.bMultiply(ccfgToDo.piAt(1).m_strValue) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varLHS.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Negate)
    {
        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varVal = varFindIt(strKey, CID_LINE);
        if (varVal.bNegate() && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strKey, varVal.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_OR)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);
        if (varLHS.bBooleanOp(ccfgToDo.piAt(1).m_strValue, tCQCKit::ELogOps::OR) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varLHS.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_PutCharAt)
    {
        const TString& strVar = ccfgToDo.piAt(0).m_strValue;
        const tCIDLib::TCard4 c4Index = ccfgToDo.piAt(1).m_strValue.c4Val();
        const tCIDLib::TCh chChar = ccfgToDo.piAt(2).m_strValue[0];

        //
        //  Get the variable and ask it to do the set char. If the index is
        //  bad it returns false, else true if it did it.
        //
        TCQCActVar& varTar = varFindIt(strVar, CID_LINE);
        bResult = varTar.bSetCharAt(chChar, c4Index);

        if (acteTar.pcmdtDebug())
        {
            if (bResult)
                acteTar.pcmdtDebug()->ActVarSet(strVar, varTar.strValue());
            else
                acteTar.pcmdtDebug()->ActDebug(L"Invalid PutCharAt index");
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Replace)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);
        const TString& strToReplace = ccfgToDo.piAt(1).m_strValue;
        const TString& strRepWith = ccfgToDo.piAt(2).m_strValue;

        if (varLHS.bReplaceChars(strToReplace[0], strRepWith[0]) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varLHS.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ReplaceSubStr)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);
        const TString& strToReplace = ccfgToDo.piAt(1).m_strValue;
        const TString& strRepWith = ccfgToDo.piAt(2).m_strValue;
        const tCIDLib::TBoolean bCaseSensitive
        (
            facCQCKit().bCheckBoolVal(ccfgToDo.piAt(3).m_strValue)
        );

        const tCIDLib::TBoolean bRes = varLHS.bReplaceSubStr
        (
            strToReplace, strRepWith, bCaseSensitive
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ReplaceToken)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);
        const TString& strToken = ccfgToDo.piAt(1).m_strValue;
        const TString& strRepWith = ccfgToDo.piAt(2).m_strValue;
        if (varLHS.bReplaceToken(strToken[0], strRepWith) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varLHS.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetNowPlus)
    {
        // Look up the variable, which must exist
        TCQCActVar& varTar = varFind
        (
            ccfgToDo.piAt(0).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::False
        );

        // And it must be of time type
        CheckVarType(varTar, tCQCKit::EFldTypes::Time);

        // Get the count out
        const tCIDLib::TCard4 c4Count = ccfgToDo.piAt(2).c4Val(tCIDLib::ERadices::Auto);

        // Get the type and verify it, setting the result as we go
        const TString& strType = ccfgToDo.piAt(1).m_strValue;
        tCIDLib::TBoolean bChanged = kCIDLib::False;
        if (strType == L"Seconds")
        {
            bChanged = varTar.bSetValue(TTime::enctNowPlusSecs(c4Count));
        }
         else if (strType == L"Minutes")
        {
            bChanged = varTar.bSetValue(TTime::enctNowPlusMins(c4Count));
        }
         else
        {
            facCQCKit().ThrowErr
            (
                pszFile
                , CID_LINE
                , kKitErrs::errcCmd_BadEnumVal
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , strType
            );
        }

        if (bChanged && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetTimeVar)
    {
        // Look up the variable, which must exist
        TCQCActVar& varTar = varFind
        (
            ccfgToDo.piAt(0).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::False
        );

        // It has to be a time variable
        CheckVarType(varTar, tCQCKit::EFldTypes::Time);

        // Set up the correct parsing format, based on the format parameter
        const TTimeCompList* pcolComp = 0;
        if (ccfgToDo.piAt(2).m_strValue == L"12HH:MM")
            pcolComp = &TTime::fcol12HHMM();
        else if (ccfgToDo.piAt(2).m_strValue == L"24HH:MM")
            pcolComp = &TTime::fcol24HHMM();
        else if (ccfgToDo.piAt(2).m_strValue == L"DD/MM/YYYY")
            pcolComp = &TTime::fcolDDMMYYYY();
        else if (ccfgToDo.piAt(2).m_strValue == L"MM/DD/YYYY")
            pcolComp = &TTime::fcolMMDDYYYY();
        else if (ccfgToDo.piAt(2).m_strValue == L"YYYY/MM/DD")
            pcolComp = &TTime::fcolYYYYMMDD();
        else if (ccfgToDo.piAt(2).m_strValue == L"YYYY/MM/DD 24HH:MM")
            pcolComp = &TTime::fcolYYYYMMDD24HHMM();

        // If not a valid pattern or it can't be parsed, give up
        TTime tmNew;
        if (!pcolComp
        ||  !tmNew.bParseFromText(ccfgToDo.piAt(1).m_strValue, *pcolComp, L'/', L':'))
        {
            facCQCKit().ThrowErr
            (
                pszFile
                , CID_LINE
                , kKitErrs::errcCmd_BadTimePattern
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , ccfgToDo.piAt(2).m_strValue
            );
        }

        if (varTar.bSetValue(tmNew.enctTime()) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetVariable)
    {
        // Create or update this key. Return true if added
        SetVar
        (
            ccfgToDo.piAt(0).m_strValue
            , ccfgToDo.piAt(1).m_strValue
            , acteTar.pcmdtDebug()
            , bResult
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetValueFrom)
    {
        //
        //  This is kind of a special one, which allows the value to be set
        //  directly from a source variable, so that the target variable can
        //  see the actual source variable, instead of just getting the
        //  formatted out value.
        //
        TCQCActVar& varSrc = varFind
        (
            ccfgToDo.piAt(1).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::False
        );

        const TString& strTarKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varTar = varFind
        (
            ccfgToDo.piAt(0).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::True
        );

        bResult = varTar.bSetValueFrom(varSrc);
        if (bResult && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strTarKey, varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetVarFmt)
    {
        //
        //  Handle old style enum prefix, which might still be around. Bu tour
        //  translation expects, Bin, Dec, etc...
        //
        TString strRadix = ccfgToDo.piAt(1).m_strValue;
        if (strRadix.bStartsWithI(L"ERadix_"))
            strRadix.Cut(0, 7);

        const tCIDLib::ERadices eRad = tCIDLib::eXlatERadices(strRadix);
        const tCIDLib::TCard4 c4Digits = ccfgToDo.piAt(2).m_strValue.c4Val();

        // Look up the variable, which must exist
        TCQCActVar& varTar = varFind
        (
            ccfgToDo.piAt(0).m_strValue
            , acteTar.ctarLocals()
            , ctarGlobalVars
            , pszFile
            , CID_LINE
            , kCIDLib::False
        );
        varTar.SetFormat(eRad, c4Digits);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SplitAt)
    {
        const TString& strSrcKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varSrc = varFindIt(strSrcKey, CID_LINE);
        const tCIDLib::TCard4 c4At = ccfgToDo.piAt(2).m_strValue.c4Val();

        // The target may not exist
        const TString& strTarName = ccfgToDo.piAt(1).m_strValue;
        varSrc.strValue().CopyOutSubStr(m_strTmp, c4At);
        SetVar(strTarName, m_strTmp, acteTar.pcmdtDebug(), bResult);

        if (varSrc.bCapAt(c4At) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strSrcKey, varSrc.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Strip)
    {
        // Find the variable we will strip
        const TString& strTarKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varTar = varFindIt(strTarKey, CID_LINE);

        // It's there so get the chars to strip and interpret the mode
        const TString& strToStrip = ccfgToDo.piAt(1).m_strValue;
        const TString& strMode = ccfgToDo.piAt(2).m_strValue;
        tCIDLib::EStripModes eMode = tCIDLib::EStripModes::None;
        if (strMode == L"Lead")
            eMode = tCIDLib::EStripModes::Leading;
        else if (strMode == L"Trail")
            eMode = tCIDLib::EStripModes::Trailing;
        else if (strMode == L"LeadTrail")
            eMode = tCIDLib::EStripModes::LeadTrail;
        else if (strMode == L"Middle")
            eMode = tCIDLib::EStripModes::Middle;
        else if (strMode == L"Full")
            eMode = tCIDLib::EStripModes::Full;
        else if (strMode == L"Complete")
            eMode = tCIDLib::EStripModes::Complete;
        else
        {
            facCQCKit().ThrowErr
            (
                pszFile
                , CID_LINE
                , kKitErrs::errcCmd_BadParmValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(3)
                , TString(L"Strip")
            );
        }

        // Do the strip directly on the variable value
        if (varTar.bStrip(strToStrip, eMode) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strTarKey, varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Subtract)
    {
        const TString& strLHSKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varLHS = varFindIt(strLHSKey, CID_LINE);
        if (varLHS.bSubtract(ccfgToDo.piAt(1).m_strValue) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strLHSKey, varLHS.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_TestAndSet)
    {
        //
        //  In this case, we have to be prepared to create, test, and set
        //  the variable, all atomically, because they generally will not
        //  want to try to create it directly because that would itself be
        //  something that would require synchronization to avoid errors.
        //
        //  The return goes into our return and indicates if we were able
        //  to set it or not (we can't if it's already set.)
        //
        //  They can ask us to wait for a while to see if we can get access
        //  before returning. If zero, we just check once.
        //
        const TString& strTarKey = ccfgToDo.piAt(0).m_strValue;
        const tCIDLib::TCard4 c4Millis = ccfgToDo.piAt(1).c4Val(tCIDLib::ERadices::Auto);

        tCIDLib::TEncodedTime enctCur = TTime::enctNow();
        const tCIDLib::TEncodedTime enctEnd
        (
            enctCur + (kCIDLib::enctOneMilliSec * c4Millis)
        );

        //
        //  We have to do a Do/While because if they pass zero for a 'no wait'
        //  type of deal, we wouldn't otherwise ever enter the loop if we
        //  do a standard while.
        //
        TThread* pthrMe = TThread::pthrCaller();
        do
        {
            // Try the test and set
            TCQCActVar& varTar = TStdVarsTar::varTestAndSet
            (
                strTarKey
                , ctarGlobalVars
                , pszFile
                , CID_LINE
                , bResult
            );

            // If we got it, then we can break out now
            if (bResult)
            {
                // Report the value change if debugging
                if (acteTar.pcmdtDebug())
                    acteTar.pcmdtDebug()->ActVarSet(strTarKey, varTar.strValue());
                break;
            }

            //
            //  Pause a bit, after releasing the mutex else we'd deadlock.
            //  If we are asked to shut down, then give up with a failure.
            //  We don't need to get the lock back if we are giving up.
            //
            lockrVars.Release();
            if (!pthrMe->bSleep(100))
                break;

            // Get the lock again before we do another round
            lockrVars.Lock();

            enctCur = TTime::enctNow();
        }   while (enctCur < enctEnd);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ToLower)
    {
        const TString& strTarKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varTar = varFindIt(strTarKey, CID_LINE);
        if (varTar.bUpLow(kCIDLib::False) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strTarKey, varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ToUpper)
    {
        const TString& strTarKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varTar = varFindIt(strTarKey, CID_LINE);
        if (varTar.bUpLow(kCIDLib::True) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strTarKey, varTar.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_TrySetVariable)
    {
        try
        {
            // Look up the variable. Fail if it doesn't exist
            TCQCActVar& varTar = TStdVarsTar::varFind
            (
                ccfgToDo.piAt(0).m_strValue
                , acteTar.ctarLocals()
                , ctarGlobalVars
                , pszFile
                , CID_LINE
                , kCIDLib::False
            );

            //
            //  Try to set the value. It'll throw if the value can't be
            //  set.
            //
            if (varTar.bSetValue(ccfgToDo.piAt(1).m_strValue) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());

            // It worked, so return success
            bResult = kCIDLib::True;
        }

        catch(const TError& errToCatch)
        {
            // Return failure
            bResult = kCIDLib::False;

            // If they provided a variable for the error text, set it
            if (!ccfgToDo.piAt(2).m_strValue.bIsEmpty())
            {
                TCQCActVar& varErr = varFind
                (
                    ccfgToDo.piAt(2).m_strValue
                    , acteTar.ctarLocals()
                    , ctarGlobalVars
                    , pszFile
                    , CID_LINE
                    , kCIDLib::True
                );


                if (varErr.bSetValue(errToCatch.strErrText()) && acteTar.pcmdtDebug())
                    acteTar.pcmdtDebug()->ActVarSet(varErr.strName(), varErr.strValue());
            }
        }
    }
     else
    {
        ThrowUnknownCmd(ccfgToDo);
    }

    //
    //  NOTE: The TestAndSet command above can lease the mutex unlocked if
    //        we wer easked to shut down whlie waiting, so don't assume we
    //        have the lock at this point if you add more code here.
    //

    return tCQCKit::ECmdRes::Ok;
}


// Load up the passed collection with the commands we support
tCIDLib::TVoid
TStdVarsTar::QueryCmds(         TCollection<TCQCCmd>&   colToFill
                        , const tCQCKit::EActCmdCtx     eContext) const
{
    // Adjust an enum variable to the next or previous value
    {
        TCQCCmd cmdAdjustEnum
        (
            kCQCKit::strCmdId_AdjustEnumValue
            , facCQCKit().strMsg(kKitMsgs::midCmd_AdjustEnumVal)
            , 3
        );

        // The name of the variable
        cmdAdjustEnum.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        // The direction
        cmdAdjustEnum.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Direction)
                , L"Next, Previous"
            )
        );

        // Whether wrapping is ok
        cmdAdjustEnum.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_WrapAtEnd)
                , tCQCKit::ECmdPTypes::Boolean
            )
        );

        cmdAdjustEnum.eType(tCQCKit::ECmdTypes::Both);
        colToFill.objAdd(cmdAdjustEnum);
    }

    // Add the passed value to the indicated variable
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_Add
            , facCQCKit().strMsg(kKitMsgs::midCmd_Add)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Number
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
        )
    );

    //
    //  Append a value to a quoted comma list (the standard formatted form
    //  for lists of values.
    //
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_AddQListValue
            , facCQCKit().strMsg(kKitMsgs::midCmd_AddQListValue)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
        )
    );

    // AND the passed value with the value of the indicated variable
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_AND
            , facCQCKit().strMsg(kKitMsgs::midCmd_AND)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
        )
    );

    // Append to a variable
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_Append
            , facCQCKit().strMsg(kKitMsgs::midCmd_Append)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Source)
        )
    );

    // Truncate a variable value at the indicated index
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_CapAt
            , facCQCKit().strMsg(kKitMsgs::midCmd_CapAt)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
        )
    );

    //
    //  Also expose here a 'safe' version which we will just copy and update
    //  from this one. We'll add it below in the right alphabetical order.
    //
    TCQCCmd cmdSafeCreateVar;
    {
        TCQCCmd cmdCreateVar
        (
            kCQCKit::strCmdId_CreateVariable
            , facCQCKit().strMsg(kKitMsgs::midCmd_CreateVariable)
            , 4
        );

        // The name of the variable
        cmdCreateVar.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        //
        //  The data type. We allow a subset of possible field values, the
        //  ones that make sense for variables. We only use the human readable
        //  part which can be grafted on to a prefix at invocation to create
        //  the full field type enumeration.
        //
        cmdCreateVar.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Type)
                , L"Boolean, Card, Float, Int, String, Time"
            )
        );

        // The optional limits string
        cmdCreateVar.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Limit)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        // The optional initial value
        cmdCreateVar.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_InitialVal)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        // The name and type are required
        cmdCreateVar.SetParmReqAt(2, kCIDLib::False);
        cmdCreateVar.SetParmReqAt(3, kCIDLib::False);

        colToFill.objAdd(cmdCreateVar);


        //
        //  The Safe version is the same but for the name, so avoid the
        //  redundant code by copying and changing.
        //
        cmdSafeCreateVar = cmdCreateVar;
        cmdSafeCreateVar.Set
        (
            kCQCKit::strCmdId_SafeCreateVariable
            , facCQCKit().strMsg(kKitMsgs::midCmd_SafeCreateVariable)
        );
    }

    // Create a variable with the same type and limits and current value as a field
    {
        TCQCCmd& cmdVarFromFld = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_CreateVarFromField
                , facCQCKit().strMsg(kKitMsgs::midCmd_CreateVarFromField)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::Field
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            )
        );

        cmdVarFromFld.cmdpAt(1).eFldAccess(tCQCKit::EReqAccess::MReadCWrite);
    }

    // Delete a variable (it has side effects and is a conditional)
    {
        TCQCCmd& cmdDel = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_DelVariable
                , facCQCKit().strMsg(kKitMsgs::midCmd_DelVariable)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            )
        );
        cmdDel.eType(tCQCKit::ECmdTypes::Both);
    }

    // Delete a substring from a variable
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_DelSubStr
            , facCQCKit().strMsg(kKitMsgs::midCmd_DelSubStr)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_First)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Count)
        )
    );

    // Divide the variable by the passed value
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_Divide
            , facCQCKit().strMsg(kKitMsgs::midCmd_Divide)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Number
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
        )
    );

    //
    //  Given two time based variables, set a third one to the number
    //  of seconds or minutes that have elapsed (the first assumed to
    //  be the earlier one and the second the later one.)
    //
    {
        TCQCCmd cmdElapsedSince
        (
            kCQCKit::strCmdId_ElapsedSince
            , facCQCKit().strMsg(kKitMsgs::midCmd_ElapsedSince)
            , 4
        );

        // The target variable to set
        cmdElapsedSince.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        // Indicates whether seconds or minutes from now
        cmdElapsedSince.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Type)
                , L"Seconds, Minutes"
            )
        );

        // The first and second time variables
        cmdElapsedSince.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_First)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        cmdElapsedSince.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Second)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        colToFill.objAdd(cmdElapsedSince);
    }

    // See if the given variable exists
    {
        TCQCCmd& cmdExists = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_Exists
                , facCQCKit().strMsg(kKitMsgs::midCmd_Exists)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            )
        );
        cmdExists.eType(tCQCKit::ECmdTypes::Conditional);
    }

    //
    //  Find the index of a substring, leaves the index in a target
    //  variable. Returns a boolean result if it found it or not.
    //
    {
        TCQCCmd& cmdFind = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_Find
                , facCQCKit().strMsg(kKitMsgs::midCmd_Find)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Source)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Target)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCKit().strMsg(kKitMsgs::midCmdP_CaseSensitive)
            )
        );
        cmdFind.eType(tCQCKit::ECmdTypes::Conditional);
    }

    // get the ordinal value of an enumerated string variable into another variable
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetEnumOrdinal
            , facCQCKit().strMsg(kKitMsgs::midCmd_GetEnumOrdinal)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
        )
    );

    //
    //  Get the nth value of an enumerated variable's limits. The src is 0,
    //  the index is 1, and the target is 2.
    //
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetNthEnumVal
            , facCQCKit().strMsg(kKitMsgs::midCmd_GetNthEnumVal)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_SourceVar)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
        )
    );

    // Set variable 2 with the length of variable 1
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetLength
            , facCQCKit().strMsg(kKitMsgs::midCmd_GetLength)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_SourceVar)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
        )
    );

    // Get the numeric range of a numeric typed variable
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetNumericRange
            , facCQCKit().strMsg(kKitMsgs::midCmd_GetNumericRange)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_MinVal)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_MaxVal)
        )
    );

    // Get substring from a variable to another variable
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetSubStr
            , facCQCKit().strMsg(kKitMsgs::midCmd_GetSubStr)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_First)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Count)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Target)
        )
    );

    // Get the value of a variable by name
    {
        TCQCCmd& cmdGetText = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_GetText
                , facCQCKit().strMsg(kKitMsgs::midCmd_GetText)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_SourceVar)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
            )
        );
        cmdGetText.eType(tCQCKit::ECmdTypes::Both);
    }

    // Insert text into a variable at a given index
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_Insert
            , facCQCKit().strMsg(kKitMsgs::midCmd_Insert)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
        )
    );

    // See if a particular char is at a particular index into the value
    {
        TCQCCmd& cmdIsCharAt = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_IsCharAt
                , facCQCKit().strMsg(kKitMsgs::midCmd_IsCharAt)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Character)
            )
        );
        cmdIsCharAt.eType(tCQCKit::ECmdTypes::Conditional);
    }

    // Multiply the variable by the passed value
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_Multiply
            , facCQCKit().strMsg(kKitMsgs::midCmd_Multiply)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Number
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
        )
    );

    // Negate the value of the variable
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_Negate
            , facCQCKit().strMsg(kKitMsgs::midCmd_Negate)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );

    // OR the passed value with the value of the indicated variable
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_OR
            , facCQCKit().strMsg(kKitMsgs::midCmd_OR)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
        )
    );

    // Set the character at a given index in a variable value
    {
        TCQCCmd& cmdPutCharAt = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_PutCharAt
                , facCQCKit().strMsg(kKitMsgs::midCmd_PutCharAt)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Character)
            )
        );
        cmdPutCharAt.eType(tCQCKit::ECmdTypes::Both);
    }

    // Replace one substring with another
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_Replace
            , facCQCKit().strMsg(kKitMsgs::midCmd_Replace)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ToReplace)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ReplaceWith)
        )
    );

    //
    //  Replace all instances of a substring with another, optionally
    //  case sensitive or not. The replace with is optional since it
    //  can be an empty string. Returns true if it replaced anything,
    //  else false so can be conditional.
    //
    {
        TCQCCmd& cmdRepSS = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_ReplaceSubStr
                , facCQCKit().strMsg(kKitMsgs::midCmd_ReplaceSubStr)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_ToReplace)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_ReplaceWith)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCKit().strMsg(kKitMsgs::midCmdP_CaseSensitive)
            )
        );
        cmdRepSS.SetParmReqAt(2, kCIDLib::False);
        cmdRepSS.eType(tCQCKit::ECmdTypes::Both);
    }

    //
    //  Replace token ToReplace (a character in the form %(c), with the
    //  replacement text. The replacement param can be empty.
    //
    {
        TCQCCmd& cmdRepTok = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_ReplaceToken
                , facCQCKit().strMsg(kKitMsgs::midCmd_ReplaceToken)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_ToReplace)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_ReplaceWith)
            )
        );
        cmdRepTok.SetParmReqAt(2, kCIDLib::False);
    }

    // Add the safe create variable command that we created up above
    colToFill.objAdd(cmdSafeCreateVar);

    //
    //  Set a time based variable to now plus some number of seconds or
    //  minutes. It has to be a time variable or we throw.
    //
    {
        TCQCCmd cmdSetNowPlus
        (
            kCQCKit::strCmdId_SetNowPlus
            , facCQCKit().strMsg(kKitMsgs::midCmd_SetNowPlus)
            , 3
        );

        // The name of the variable to set
        cmdSetNowPlus.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        // Indicates whether seconds or minutes from now
        cmdSetNowPlus.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Type)
                , L"Seconds, Minutes"
            )
        );

        // And the count of seconds or minutes
        cmdSetNowPlus.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Count)
                , tCQCKit::ECmdPTypes::Unsigned
            )
        );
        colToFill.objAdd(cmdSetNowPlus);
    }

    // Set a time variable from a formatted time/date string
    {
        TCQCCmd cmdSetTP
        (
            kCQCKit::strCmdId_SetTimeVar
            , facCQCKit().strMsg(kKitMsgs::midCmd_SetTimeVar)
            , 3
        );

        cmdSetTP.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        cmdSetTP.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_ToSet)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        cmdSetTP.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Pattern)
                , L"12HH:MM, 24HH:MM, DD/MM/YYYY, MM/DD/YYYY, YYYY/MM/DD, "
                  L"YYYY/MM/DD 24HH:MM"
            )
        );

        colToFill.objAdd(cmdSetTP);
    }

    //
    //  Set/add a variable (it has side effects and is a conditional), and
    //  parm 1 is optional since they can set it to a blank value. This
    //  will create a string type variable if it has to create it. It returns
    //  true if the variable had to be created, else false if it already
    //  existed.
    //
    {
        TCQCCmd& cmdSetV = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetVariable
                , facCQCKit().strMsg(kKitMsgs::midCmd_SetVariable)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarValue)
            )
        );
        cmdSetV.eType(tCQCKit::ECmdTypes::Both);
        cmdSetV.SetParmReqAt(1, kCIDLib::False);
    }

    //
    //  Set the value of one variable directly from the other. It returns
    //  true if the value changed, else false if it didn't.
    //
    {
        TCQCCmd& cmdSetVF = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetValueFrom
                , facCQCKit().strMsg(kKitMsgs::midCmd_SetValueFrom)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_TargetVar)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_SourceVar)
            )
        );
        cmdSetVF.eType(tCQCKit::ECmdTypes::Both);
    }

    // Set the optional formatting attributes on a variable
    {
        TCQCCmd cmdSetFmt
        (
            kCQCKit::strCmdId_SetVarFmt
            , facCQCKit().strMsg(kKitMsgs::midCmd_SetVarFmt)
            , 3
        );

        // The variable key
        cmdSetFmt.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        //
        //  The radix. These are selected so that they can be appended to
        //  a prefix and directly converted to the enumeration.
        //
        cmdSetFmt.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Radix)
                , L"Bin, Oct, Dec, Hex"
            )
        );

        // The decimal digits
        cmdSetFmt.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Digits)
                , tCQCKit::ECmdPTypes::Unsigned
            )
        );

        colToFill.objAdd(cmdSetFmt);
    }

    //
    //  Split a variable at the indicated index. The second half goes into
    //  another variable.
    //
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_SplitAt
            , facCQCKit().strMsg(kKitMsgs::midCmd_SplitAt)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Source)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Target)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
        )
    );

    // Strip leading/trailing chars from the variable value
    {
        TCQCCmd cmdStrip
        (
            kCQCKit::strCmdId_Strip
            , facCQCKit().strMsg(kKitMsgs::midCmd_Strip)
            , 3
        );

        // The key of the variable to strip
        cmdStrip.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::VarName
            )
        );

        // The characters to strip
        cmdStrip.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Whitespace)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        // The stripping mode
        cmdStrip.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Mode)
                , L"Lead, Trail, LeadTrail, Middle, Full, Complete"
            )
        );
        colToFill.objAdd(cmdStrip);
    }

    // Subtract the passed value from the indicated variable
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_Subtract
            , facCQCKit().strMsg(kKitMsgs::midCmd_Subtract)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            , tCQCKit::ECmdPTypes::Number
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
        )
    );

    //
    //  An atomic test and set that works on boolean global variables.
    //  Only works only global variables so only return it if we are
    //  a global variable target.
    //
    if (!m_bIsLocal)
    {
        TCQCCmd& cmdTestAndSet = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_TestAndSet
                , facCQCKit().strMsg(kKitMsgs::midCmd_TestAndSet)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Milliseconds)
            )
        );
        cmdTestAndSet.eType(tCQCKit::ECmdTypes::Conditional);
    }

    // Convert the variable to all up or all lower case
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_ToLower
            , facCQCKit().strMsg(kKitMsgs::midCmd_ToLower)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );

    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_ToUpper
            , facCQCKit().strMsg(kKitMsgs::midCmd_ToUpper)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );


    //
    //  Similar to SetVariable above. But this one conditionally tries to
    //  set it, in order to let the caller handle any errors that might
    //  occur because the value passed isn't legal for the variable type
    //  or range. So it returns a true/false that indicates whether it
    //  was able to set the variable value. It will NOT create a new one
    //  ofr you if it doesn't already exist.
    //
    //  There's an optional third parameter which is the name of a
    //  variable to fill in with the error text if it fails.
    //
    {
        TCQCCmd& cmdTrySetV = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_TrySetVariable
                , facCQCKit().strMsg(kKitMsgs::midCmd_TrySetVariable)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
                , tCQCKit::ECmdPTypes::Text
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarValue)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Reason)
            )
        );
        cmdTrySetV.eType(tCQCKit::ECmdTypes::Both);
        cmdTrySetV.SetParmReqAt(1, kCIDLib::False);
        cmdTrySetV.SetParmReqAt(2, kCIDLib::False);
    }
}


tCIDLib::TVoid TStdVarsTar::SetDefParms(TCQCCmdCfg&) const
{
    // Nothing really to do for us
}


// ---------------------------------------------------------------------------
//  TStdVarsTar: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is a special one that is just here to help the action engine deal
//  with the special action parameters. In the interface engine and global
//  action invocation the user can provide these. They go into the locals
//  and are marked read only. This method makes it easier because these
//  shouldn't already exist and always are read only, so we can make various
//  assumptions here.
//
tCIDLib::TVoid
TStdVarsTar::AddActParmVar(const TString& strName, const TString& strValue)
{
    CIDAssert(m_bIsLocal, L"This is only valid on the local variables target");

    // It should not already exist
    CIDAssert
    (
        m_colVars.pobjFindByKey(strName, kCIDLib::False) == 0
        , L"The action parameter variable should not already exist"
    );

    CIDAssert
    (
        strName.bStartsWith(kCQCKit::strActVarPref_Parm)
        , L"Action parameter vars must start with the appropriate prefix"
    );

    // Create a new string type variable with the passed value and add it
    TCQCActVar* pvarNew = new TCQCActVar
    (
        strName
        , tCQCKit::EFldTypes::String
        , TString::strEmpty()
        , strValue
        , m_c4NextId
    );
    m_colVars.Add(pvarNew);


    // Bump the variable id counter
    m_c4NextId++;
    if (!m_c4NextId)
        m_c4NextId = 1;

    // Set it read only
    pvarNew->bReadOnly(kCIDLib::True);
}



//
//  This is an atomic test and set type of thing. It will see if the indicated
//  variable exists. If not it will create it. In either case it will set
//  the passed value on it. It will return true if it created it, or if it
//  set a new value. If it returns false, then it existed and the passed value
//  was already set on it.
//
tCIDLib::TBoolean
TStdVarsTar::bAddOrUpdateVar(const  TString&            strName
                            , const TString&            strValue
                            , const tCQCKit::EFldTypes  eType
                            , const TString&            strLimits)
{
    // Make sure the prefix is correct
    if (!strName.bStartsWith(m_strPrefix))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_WrongVarPref
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strPrefix
        );
    }

    tCIDLib::TBoolean bRet = kCIDLib::False;
    TCQCActVar* pvarTar = 0;

    // We have to lock here since it's not a single atomic op
    TLocker lockrVars(&m_colVars);

    // See if it currently exists
    pvarTar = m_colVars.pobjFindByKey(strName, kCIDLib::False);

    // If it doesn't exist then create it now
    if (!pvarTar)
    {
        // if it's an action parameter, this isn't legal
        if (strName.bStartsWith(kCQCKit::strActVarPref_Parm))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_ActParmFaultIn
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , strName
            );
        }

        bRet = kCIDLib::True;
        m_colVars.Add
        (
            new TCQCActVar(strName, eType, strLimits, strValue, m_c4NextId)
        );

        m_c4NextId++;
        if (!m_c4NextId)
            m_c4NextId = 1;
    }

    //
    //  If we didn't create it, then set the passed value. If we created it,
    //  the value was set during the creation. Even if the value is empty,
    //  that can still be a valid value. If not, this will throw. If the
    //  value we pass is the same as already there, then it returns false,
    //  which is what we want.
    //
    if (!bRet)
        bRet = pvarTar->bSetValue(strValue);
    return bRet;
}


// Look for the value of the passed variable. Return true if found
tCIDLib::TBoolean
TStdVarsTar::bVarValue( const   TString&    strKey
                        ,       TString&    strToFill) const
{
    // This one isn't one atomic op, so we have to lock
    TLocker lockrVars(&m_colVars);
    const TCQCActVar* pvarRet = m_colVars.pobjFindByKey(strKey, kCIDLib::False);
    if (!pvarRet)
        return kCIDLib::False;
    strToFill = pvarRet->strValue();
    return kCIDLib::True;
}


// Let the outside world have a const cursor to the variables
TStdVarsTar::TCursor TStdVarsTar::cursVars() const
{
    return TCursor(&m_colVars);
}


//
//  Deletes all the variables in our list. This is mainly for debugging,
//  so that the IV can flush all variables then reload a template, to
//  force a from scratch scenario. It's not something that should be called
//  while the associated viewer or other app that uses the action system is
//  using the variables.
//
tCIDLib::TVoid TStdVarsTar::DeleteAllVars()
{
    m_colVars.RemoveAll();
}


//
//  Same as above, but takes a serial number. It returns whether the variable
//  was not found, was found but hasn't changed, or was found and has changed.
//  In the latter case the new value and new serial number are returned.
//
//  If the variable id has changed, we return IdChanged and return the new id,
//  since it's been deleted and re-created, so the caller should check for any
//  changes in type or limits. If the incoming variable id is zero, we assume
//  this is an initial lookup and just give back the new id.
//
//  We have another version that does take the variable id, for those folks
//  who don't care.
//
tCQCKit::EValQRes
TStdVarsTar::eVarValue( const   TString&            strKey
                        ,       TString&            strToFill
                        ,       tCIDLib::TCard4&    c4SerialNum
                        ,       tCIDLib::TCard4&    c4VarId) const
{
    // This isn't a single atomic op, so we have to lock
    TLocker lockrVars(&m_colVars);
    const TCQCActVar* pvarRet = m_colVars.pobjFindByKey(strKey, kCIDLib::False);
    if (pvarRet)
    {
        //
        //  If the variable id isn't the same, then we may need to return a
        //  reconfig status.
        //
        if (pvarRet->c4VarId() != c4VarId)
        {
            const tCIDLib::TBoolean bInVarId(c4VarId != 0);

            // Give them back the new id one way or another
            c4VarId = pvarRet->c4VarId();

            // If the incoming id was non-zero, then return reconfig status
            if (bInVarId)
                return tCQCKit::EValQRes::Reconfig;
        }

        // If the serial number hasn't changed
        if (pvarRet->c4SerialNum() == c4SerialNum)
            return tCQCKit::EValQRes::NoChange;

        c4SerialNum = pvarRet->c4SerialNum();
        strToFill = pvarRet->strValue();

        return tCQCKit::EValQRes::NewValue;
    }
    return tCQCKit::EValQRes::NotFound;
}


tCQCKit::EValQRes
TStdVarsTar::eVarValue( const   TString&            strKey
                        ,       TString&            strToFill
                        ,       tCIDLib::TCard4&    c4SerialNum) const
{
    // This isn't a single atomic op, so we have to lock
    TLocker lockrVars(&m_colVars);
    const TCQCActVar* pvarRet = m_colVars.pobjFindByKey(strKey, kCIDLib::False);
    if (pvarRet)
    {
        if (pvarRet->c4SerialNum() == c4SerialNum)
            return tCQCKit::EValQRes::NoChange;

        c4SerialNum = pvarRet->c4SerialNum();
        strToFill = pvarRet->strValue();
        return tCQCKit::EValQRes::NewValue;
    }
    return tCQCKit::EValQRes::NotFound;
}



//
//  Get a ref to the value of a variable, mostly to support testing. It'll
//  throw if not found.
//
const TString& TStdVarsTar::strValue(const TString& strKey) const
{
    const TCQCActVar* pvarSrc = m_colVars.pobjFindByKey(strKey, kCIDLib::False);
    if (pvarSrc)
        return pvarSrc->strValue();

    // Bogus return to make the compiler happy of course
    ThrowNotFound(strKey, CID_FILE, CID_LINE);
    return TString::strEmpty();
}


//
//  Allows the outside world to set a variable value in a non action sort
//  of way, which is necessary sometimes. The variable must already exist.
//
tCIDLib::TVoid
TStdVarsTar::UpdateVar( const   TString&    strKey
                        , const TString&    strValue)
{
    // This isn't a single atomic op, so we have to lock
    TLocker lockrVars(&m_colVars);
    TCQCActVar* pvarRet = m_colVars.pobjFindByKey(strKey, kCIDLib::False);
    if (!pvarRet)
        ThrowNotFound(strKey, CID_FILE, CID_LINE);
    else
        pvarRet->bSetValue(strValue);
}


//
//  Let the outside world directly access variables, mainly it's for the
//  variable based widgets in the interface system.
//
TCQCActVar& TStdVarsTar::varFindByName(const TString& strName)
{
    TCQCActVar* pvarFind = m_colVars.pobjFindByKey(strName, kCIDLib::False);
    if (pvarFind)
        return *pvarFind;

    // Bogus return to make the compiler happy
    ThrowNotFound(strName, CID_FILE, CID_LINE);
    return *static_cast<TCQCActVar*>(nullptr);
}

const TCQCActVar& TStdVarsTar::varFindByName(const TString& strName) const
{
    const TCQCActVar* pvarFind = m_colVars.pobjFindByKey(strName, kCIDLib::False);
    if (pvarFind)
        return *pvarFind;

    // Bogus return to make the compiler happy
    ThrowNotFound(strName, CID_FILE, CID_LINE);
    return *static_cast<TCQCActVar*>(nullptr);
}


// ---------------------------------------------------------------------------
//  TStdVarsTar: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Checks the passed variable and throws if not of the indicated type
tCIDLib::TVoid
TStdVarsTar::CheckVarType(  const   TCQCActVar&         varTar
                            , const tCQCKit::EFldTypes  eExpType)
{
    if (varTar.eType() != eExpType)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_ExpectedVarType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , varTar.strName()
            , tCQCKit::strXlatEFldTypes(eExpType)
        );
    }
}


//
//  Sets the indicated variable to the indicated value, creating it if that
//  is required. We return whether we created it or now. It will be just
//  given a generic string type.
//
tCIDLib::TVoid
TStdVarsTar::SetVar(const   TString&                strKey
                    , const TString&                strValue
                    ,       MCQCCmdTracer* const    pcmdtDebug
                    ,       tCIDLib::TBoolean&      bAdded)
{
    // The key has to start with our key prefix
    if (!strKey.bStartsWith(m_strPrefix))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_WrongVarPref
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strPrefix
        );
    }

    // This isn't a single atomic op, so we have to lock
    TLocker lockrVars(&m_colVars);

    TCQCActVar* pvarRet = m_colVars.pobjFindByKey(strKey, kCIDLib::False);
    if (pvarRet)
    {
        if (pvarRet->bSetValue(strValue) && pcmdtDebug)
            pcmdtDebug->ActVarSet(strKey, strValue);
        bAdded = kCIDLib::False;
    }
     else
    {
        // if it's an action parameter, this isn't legal
        if (strKey.bStartsWith(kCQCKit::strActVarPref_Parm))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_ActParmFaultIn
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , strKey
            );
        }

        TCQCActVar* pvarNew = new TCQCActVar
        (
            strKey, tCQCKit::EFldTypes::String, strValue, m_c4NextId
        );
        m_c4NextId++;
        if (!m_c4NextId)
            m_c4NextId = 1;

        m_colVars.Add(pvarNew);
        if (pcmdtDebug)
            pcmdtDebug->ActVarSet(strKey, strValue);
        bAdded = kCIDLib::True;
    }
}


TCQCActVar&
TStdVarsTar::varFindIt(const TString& strName, const tCIDLib::TCard4 c4Line)
{
    TCQCActVar* pvarFind = m_colVars.pobjFindByKey(strName, kCIDLib::False);
    if (!pvarFind)
        ThrowNotFound(strName, CID_FILE, c4Line);
    return *pvarFind;
}


// ---------------------------------------------------------------------------
//  TStdVarsTar: Private, static methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TStdVarsTar::ThrowNotFound( const   TString&            strName
                            , const tCIDLib::TCh* const pszFile
                            , const tCIDLib::TCard4     c4Line)
{
    facCQCKit().ThrowErr
    (
        pszFile
        , c4Line
        , kKitErrs::errcTokR_VarNotFound
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strName
    );
}



