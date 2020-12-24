//
// FILE NAME: CQCIntfEng_VarIntf.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/03/2008
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
//  This file implements a mixin used by widgets that are based on variables.
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
#include    "CQCIntfEng_.hpp"


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_VarIntf
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfVarIntf
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MCQCIntfVarIntf: Destructor
// ---------------------------------------------------------------------------
MCQCIntfVarIntf::~MCQCIntfVarIntf()
{
}


// ---------------------------------------------------------------------------
//  MCQCIntfVarIntf: Public, virtual methods
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
//  MCQCIntfVarIntf: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Indicates whether we have a good value or a value error
tCIDLib::TBoolean MCQCIntfVarIntf::bHasGoodVarValue() const
{
    return (m_eState == tCQCIntfEng::EVarStates::Ready);
}


//
//  Indicates if we have a variable name set yet. It has to be longer than
//  the local/global prefix, else it cannot be legal.
//
tCIDLib::TBoolean MCQCIntfVarIntf::bHasVariable() const
{
    return !m_strVarName.bIsEmpty();
}


tCIDLib::TBoolean
MCQCIntfVarIntf::bSameVariable(const MCQCIntfVarIntf& miwdgSrc) const
{
    return (m_strVarName == miwdgSrc.m_strVarName);
}


// Clears our field association
tCIDLib::TVoid MCQCIntfVarIntf::ClearVarName()
{
    m_strVarName.Clear();
}


tCQCIntfEng::EVarStates MCQCIntfVarIntf::eVarState() const
{
    return m_eState;
}


tCQCKit::EFldTypes MCQCIntfVarIntf::eVarType() const
{
    return m_eType;
}


// Spit out any stuff we put into the attribute summary
tCIDLib::TVoid
MCQCIntfVarIntf::QueryVarAttrs( tCIDMData::TAttrList&   colAttrs
                                , TAttrData&            adatTmp) const
{
    adatTmp.Set(L"Variable:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Variable Name"
        , kCQCIntfEng::strAttr_Var_Variable
        , kCIDMData::strAttrLim_Required
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strVarName);
    colAttrs.objAdd(adatTmp);
}


// Read in our persistent data
tCIDLib::TVoid
MCQCIntfVarIntf::ReadInVar(TBinInStream& strmToReadFrom)
{
    // Our stuff shuold start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_VarIntf::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString(L"MCQCIntfVarIntf")
        );
    }

    strmToReadFrom  >> m_strVarName;

    // And it should all end with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Reset all the runtime stuff
    m_c4SerialNum   = 0;
    m_c4VarId       = 0;
    m_eState        = tCQCIntfEng::EVarStates::WaitVar;
    m_eType         = tCQCKit::EFldTypes::Count;
}


tCIDLib::TVoid
MCQCIntfVarIntf::ReplaceVar(const   TString&            strSrc
                            , const TString&            strTar
                            , const tCIDLib::TBoolean   bRegEx
                            , const tCIDLib::TBoolean   bFull
                            ,       TRegEx&             regxFind)
{
    facCQCKit().bDoSearchNReplace
    (
        strSrc, strTar, m_strVarName, bRegEx, bFull, regxFind
    );
}


// Return our variable name
const TString& MCQCIntfVarIntf::strVarName() const
{
    return m_strVarName;
}


tCIDLib::TVoid
MCQCIntfVarIntf::SetVarAttr(        TAttrEditWnd&   wndAttrEd
                            , const TAttrData&      adatNew
                            , const TAttrData&)
{
    if (adatNew.strId() == kCQCIntfEng::strAttr_Var_Variable)
        m_strVarName = adatNew.strValue();
}


//
//  Let's the config tab set us up with user config. Currently that's just
//  the name of the variable we should associate with. So this is only used
//  at design time.
//
tCIDLib::TVoid MCQCIntfVarIntf::SetVarInfo(const TString& strToSet)
{
    m_strVarName = strToSet;
}


// We make sure that a variable has been set
tCIDLib::TVoid
MCQCIntfVarIntf::ValidateVar(       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal
                            , const TCQCIntfWidget&         iwdgThis) const
{
    // It has to be a global variable
    if (!m_strVarName.bStartsWith(kCQCKit::strActVarPref_GVar))
    {
        TString strErr
        (
            kKitErrs::errcCmd_WrongVarPref, facCQCKit(), kCQCKit::strActVarPref_GVar
        );
        colErrs.objAdd
        (
            TCQCIntfValErrInfo(iwdgThis.c4UniqueId(), strErr, kCIDLib::True, iwdgThis.strWidgetId())
        );
    }

    // It can't just be the prefix
    if (m_strVarName.c4Length() == kCQCKit::strActVarPref_GVar.c4Length())
    {
        TString strErr(kIEngErrs::errcVar_NoVarSelected, facCQCIntfEng());
        colErrs.objAdd
        (
            TCQCIntfValErrInfo
            (
                iwdgThis.c4UniqueId(), strErr, kCIDLib::True, iwdgThis.strWidgetId()
            )
        );
    }
}


//
//  This is called regularly to let us update our value. We just ask the
//  polling info object to update. If he indicates that something has
//  changed, then we call a virtual that any of our derivatives will
//  implement.
//
tCIDLib::TVoid
MCQCIntfVarIntf::VarUpdate( const   TStdVarsTar&        ctarGlobalVars
                            , const tCIDLib::TBoolean   bNoRedraw)
{
    TString strVal;
    const tCQCKit::EValQRes eRes = ctarGlobalVars.eVarValue
    (
        m_strVarName, strVal, m_c4SerialNum, m_c4VarId
    );

    //
    //  If we are in rejected state, then the only states we can really
    //  make use of are the not found and reconfigured states, which are
    //  the only ones that could let us recover from this state.
    //
    if ((m_eState == tCQCIntfEng::EVarStates::Rejected)
    &&  (eRes != tCQCKit::EValQRes::NotFound)
    &&  (eRes != tCQCKit::EValQRes::Reconfig))
    {
        return;
    }

    tCQCIntfEng::EVarStates eOldState = m_eState;
    switch(eRes)
    {
        case tCQCKit::EValQRes::InError :
            m_eState = tCQCIntfEng::EVarStates::ValError;
            break;

        case tCQCKit::EValQRes::NewValue :
        {
            //
            //  If we were not in ready state, then we've aquired the
            //  the variable, so we need to ask the derived class if he
            //  can accept it. If so, we go to ready state, else we go
            //  into rejected state.
            //
            const TCQCActVar& varNew = ctarGlobalVars.varFindByName(m_strVarName);
            m_eType = varNew.eType();
            if (eOldState == tCQCIntfEng::EVarStates::WaitVar)
            {
                if (bCanAcceptVar(varNew))
                    m_eState = tCQCIntfEng::EVarStates::Ready;
                else
                    m_eState = tCQCIntfEng::EVarStates::Rejected;
            }

            // If we are in ready state, tell the enclosing class
            if (m_eState == tCQCIntfEng::EVarStates::Ready)
                VarValChanged(varNew, bNoRedraw, ctarGlobalVars);
            break;
        }

        case tCQCKit::EValQRes::NotFound :
        {
            //
            //  We've lost it, so we have to move everything back to a state
            //  where we are waiting to find our variable.
            //
            m_eState = tCQCIntfEng::EVarStates::WaitVar;
            m_eType = tCQCKit::EFldTypes::Count;
            m_c4SerialNum = 0;
            m_c4VarId = 0;
            break;
        }

        case tCQCKit::EValQRes::Reconfig :
        {
            //
            //  It's been deleted and recreated, so it could have changed
            //  its type or limits. So we need to re-check it and see if the
            //  contining class can accept it.
            //
            const TCQCActVar& varNew = ctarGlobalVars.varFindByName(m_strVarName);
            m_eType = varNew.eType();
            if (bCanAcceptVar(varNew))
                m_eState = tCQCIntfEng::EVarStates::Ready;
             else
                m_eState = tCQCIntfEng::EVarStates::Rejected;

            //
            //  If we are in ready state, tell the enclosing class a new value
            //  is available. It might not actually have changed, but it's not
            //  worth trying to figure that out. Just assume it did.
            //
            if (m_eState == tCQCIntfEng::EVarStates::Ready)
                VarValChanged(varNew, bNoRedraw, ctarGlobalVars);
            break;
        }

        case tCQCKit::EValQRes::NoChange :
            // Nothing to do
            break;

        default :
            CIDAssert(kCIDLib::False, L"Unknown value query result");
            break;
    };

    //
    //  If the state changed from ready to something else, then tell the
    //  enclosing class that the variable is in error now.
    //
    if ((eOldState == tCQCIntfEng::EVarStates::Ready)
    &&  (m_eState != tCQCIntfEng::EVarStates::Ready))
    {
        VarInError(bNoRedraw);
    }
}


// Stream out our persistent data
tCIDLib::TVoid
MCQCIntfVarIntf::WriteOutVar(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCIntfEng_VarIntf::c2FmtVersion;

    // Do our stuff and the end marker
    strmToWriteTo   << m_strVarName
                    << tCIDLib::EStreamMarkers::Frame;
}


// ---------------------------------------------------------------------------
//  MCQCIntfVarIntf: Hidden constructors operators
// ---------------------------------------------------------------------------
MCQCIntfVarIntf::MCQCIntfVarIntf() :

    m_c4SerialNum(0)
    , m_c4VarId(0)
    , m_eState(tCQCIntfEng::EVarStates::WaitVar)
    , m_eType(tCQCKit::EFldTypes::Count)
{
}

// We don't take source's serial number or state
MCQCIntfVarIntf::MCQCIntfVarIntf(const MCQCIntfVarIntf& miwdgToCopy) :

    m_c4SerialNum(0)
    , m_c4VarId(0)
    , m_eState(tCQCIntfEng::EVarStates::WaitVar)
    , m_eType(miwdgToCopy.m_eType)
    , m_strVarName(miwdgToCopy.m_strVarName)
{
}

MCQCIntfVarIntf&
MCQCIntfVarIntf::operator=(const MCQCIntfVarIntf& miwdgToAssign)
{
    // We don't take the source object's serial number, state, or var id
    if (this != &miwdgToAssign)
    {
        m_c4SerialNum   = 0;
        m_c4VarId       = 0;
        m_eState        = tCQCIntfEng::EVarStates::WaitVar;
        m_eType         = miwdgToAssign.m_eType;
        m_strVarName    = miwdgToAssign.m_strVarName;
    }
    return *this;
}


