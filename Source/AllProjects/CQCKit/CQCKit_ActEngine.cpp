//
// FILE NAME: CQCKit_ActEngine.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/10/2008
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
//  This is the small bit of non-abstract implementation for the action engine
//  interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCActEngine,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TCQCActEngine
//  PREFIX: acte
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCActEngine: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCActEngine::~TCQCActEngine()
{
}

// ---------------------------------------------------------------------------
//  TCQCActEngine: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCActEngine::Reset()
{
    // DON'T clear parameter list. They remain available
}


// ---------------------------------------------------------------------------
//  TCQCActEngine: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Provide access to the user context
const TCQCUserCtx& TCQCActEngine::cuctxToUse() const
{
    return m_cuctxToUse;
}


// Just a convenience, we pass it on to the user context
tCQCKit::EUserRoles TCQCActEngine::eUserRole() const
{
    return m_cuctxToUse.eUserRole();
}


//
//  Initialize the engine for a new run. We get any incoming action params
//  for this invocation.
//
tCIDLib::TVoid TCQCActEngine::Init(const TString& strToSet)
{
    // Clear our parm list and parse the values into it
    m_colParms.RemoveAll();
    if (!strToSet.bIsEmpty())
    {
        tCIDLib::TCard4     c4ErrInd;
        TVector<TString>    colParms;
        const tCIDLib::TBoolean bRes = TStringTokenizer::bParseQuotedCommaList
        (
            strToSet, colParms, c4ErrInd
        );

        if (!bRes)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcActEng_BadParams
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , TCardinal(c4ErrInd)
            );
        }

        // Go through and load them into our list with their names
        TString strVarName;
        const tCIDLib::TCard4 c4Count = colParms.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strVarName = kCQCKit::strActVarPref_Parm;
            strVarName.AppendFormatted(c4Index + 1);
            m_colParms.objAdd(TKeyValuePair(strVarName, colParms[c4Index]));
        }
    }
}


//
//  Put any action parms we have into the passed local variables list. We mark
//  them as read only.
//
tCIDLib::TVoid TCQCActEngine::QueryActParms(TStdVarsTar& ctarLocals) const
{
    // If no parms, do nothing
    if (m_colParms.bIsEmpty())
        return;

    const tCIDLib::TCard4 c4Count = m_colParms.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colParms[c4Index];
        ctarLocals.AddActParmVar(kvalCur.strKey(), kvalCur.strValue());
    }
}


// Just a convenience, we pass it on to the user context
const TCQCSecToken& TCQCActEngine::sectUser() const
{
    return m_cuctxToUse.sectUser();
}


// Just a convenience, we pass it on to the user context
const TString& TCQCActEngine::strUserName() const
{
    return m_cuctxToUse.strUserName();
}




// ---------------------------------------------------------------------------
//  TCQCActEngine: Hidden constructors
// ---------------------------------------------------------------------------
TCQCActEngine::TCQCActEngine(const TCQCUserCtx& cuctxToUse) :

    m_cuctxToUse(cuctxToUse)
{
}

