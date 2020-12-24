//
// FILE NAME: GenProtoS_Variable.cpp
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
//  This file implements the definition of a variable.
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
RTTIDecls(TGenProtoVarInfo,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoVarInfo
//  PREFIX: vari
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoVarInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TGenProtoVarInfo::strKey(const TGenProtoVarInfo& variSrc)
{
    return variSrc.m_strName;
}


// ---------------------------------------------------------------------------
//  TGenProtoVarInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoVarInfo::TGenProtoVarInfo( const   TString&            strName
                                    , const tGenProtoS::ETypes  eType
                                    , const tCIDLib::TBoolean   bConst) :
    m_bConst(bConst)
    , m_evalCurrent(eType)
    , m_strName(strName)
{
}

TGenProtoVarInfo::TGenProtoVarInfo( const   TString&            strName
                                    , const tGenProtoS::ETypes  eType
                                    , const tCIDLib::TBoolean   bConst
                                    , const TGenProtoExprVal&   evalInit) :
    m_bConst(bConst)
    , m_evalCurrent(eType)
    , m_strName(strName)
{
    m_evalCurrent.SetFrom(evalInit);
}

TGenProtoVarInfo::TGenProtoVarInfo(const TGenProtoVarInfo& variToCopy) :

    m_bConst(variToCopy.m_bConst)
    , m_evalCurrent(variToCopy.m_evalCurrent)
    , m_strName(variToCopy.m_strName)
{
}

TGenProtoVarInfo::~TGenProtoVarInfo()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoVarInfo: Public operators
// ---------------------------------------------------------------------------
TGenProtoVarInfo&
TGenProtoVarInfo::operator=(const TGenProtoVarInfo& variToAssign)
{
    if (this == &variToAssign)
        return *this;

    m_bConst        = variToAssign.m_bConst;
    m_evalCurrent   = variToAssign.m_evalCurrent;
    m_strName       = variToAssign.m_strName;
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoVarInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoVarInfo::bIsConst() const
{
    return m_bConst;
}


tGenProtoS::ETypes TGenProtoVarInfo::eType() const
{
    return m_evalCurrent.eType();
}


const TGenProtoExprVal& TGenProtoVarInfo::evalThis() const
{
    return m_evalCurrent;
}

TGenProtoExprVal& TGenProtoVarInfo::evalThis()
{
    return m_evalCurrent;
}


const TString& TGenProtoVarInfo::strName() const
{
    return m_strName;
}


tCIDLib::TVoid TGenProtoVarInfo::Set(const TGenProtoExprVal& evalToSet)
{
    // <ake sure that they don't try to set a const
    if (m_bConst)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcRunT_CanSetConst
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , m_strName
        );
    }

    //
    //  Do set from, not assignment, since we want to get just the value,
    //  not the other stuff.
    //
    m_evalCurrent.SetFrom(evalToSet);
}


