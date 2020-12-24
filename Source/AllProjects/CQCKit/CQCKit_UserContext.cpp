//
// FILE NAME: CQCKit_UserAccount.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/29/2002
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
//  This is the implementation file for the user account class.
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
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCUserCtx,TCIDUserCtx)



// ---------------------------------------------------------------------------
//   CLASS: TCQCUserCtx
//  PREFIX: cuctx
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCUserCtx: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCUserCtx::TCQCUserCtx() :

    m_eUserRole(tCQCKit::EUserRoles::Count)
    , m_objaEnvRTVs(9)
{
}

TCQCUserCtx::TCQCUserCtx(const  TString&            strUserName
                        , const tCQCKit::EUserRoles eUserRole
                        , const TCQCSecToken&       sectUser
                        , const TString&            strDefTemplate) :

    TCIDUserCtx(strUserName, tCQCKit::strLoadEUserRoles(eUserRole))
    , m_eUserRole(eUserRole)
    , m_objaEnvRTVs(9)
    , m_sectUser(sectUser)
    , m_strDefTemplate(strDefTemplate)
{
}

TCQCUserCtx::TCQCUserCtx(const  TString&            strUserName
                        , const tCQCKit::EUserRoles eUserRole
                        , const TString&            strDefTemplate) :

    TCIDUserCtx(strUserName, tCQCKit::strLoadEUserRoles(eUserRole))
    , m_eUserRole(eUserRole)
    , m_objaEnvRTVs(9)
    , m_strDefTemplate(strDefTemplate)
{
}

TCQCUserCtx::~TCQCUserCtx()
{
}

// ---------------------------------------------------------------------------
//  TCQCUserCtx: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCUserCtx::Reset()
{
    // Call our parent first
    TParent::Reset();

    m_eUserRole = tCQCKit::EUserRoles::Count;
    m_objaEnvRTVs.SetAll(TString::strEmpty());
    m_sectUser.Reset();
    m_strDefTemplate.Clear();
}



// ---------------------------------------------------------------------------
//  TCQCUserCtx: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCKit::EUserRoles TCQCUserCtx::eUserRole() const
{
    return m_eUserRole;
}


//
//  A helper that will do the default of loading up all of the variables
//  from the host environment.
//
tCIDLib::TVoid TCQCUserCtx::LoadEnvRTVsFromHost()
{
    TString strCur;
    TString strName(L"CQC_EnvRTV0");
    for (tCIDLib::TCard4 c4Index = 0; c4Index < 9; c4Index++)
    {
        strName.AppendFormatted(c4Index + 1);
        if (TProcEnvironment::bFind(strName, strCur))
            m_objaEnvRTVs[c4Index] = strCur;
        else
            m_objaEnvRTVs[c4Index].Clear();

        // Get the index number back off of it
        strName.DeleteLast();
    }
}


const TCQCSecToken& TCQCUserCtx::sectUser() const
{
    return m_sectUser;
}


const TString& TCQCUserCtx::strEnvRTVAt(const tCIDLib::TCard4 c4At) const
{
    if (c4At > 8)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_IndexError
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4At)
            , TString(L"CQC User Context")
            , TCardinal(9)
        );
    }
    return m_objaEnvRTVs[c4At];
}


const TString& TCQCUserCtx::strDefTemplate() const
{
    return m_strDefTemplate;
}


//
//  Given a user account and security token from a successful login, we
//  set up the user context info that we need.
//
tCIDLib::TVoid TCQCUserCtx::Set(const   TCQCUserAccount&    uaccSrc
                                , const TCQCSecToken&       sectUser)
{
    m_eUserRole      = uaccSrc.eRole();
    m_sectUser       = sectUser;
    m_strDefTemplate = uaccSrc.strDefInterfaceName();

    // Update our parent class
    TParent::SetUserInfo
    (
        uaccSrc.strLoginName()
        , tCQCKit::strLoadEUserRoles(m_eUserRole)
    );
}


tCIDLib::TVoid
TCQCUserCtx::SetEnvRTVAt(const  tCIDLib::TCard4 c4At
                        , const TString&        strToSet)
{
    if (c4At > 8)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_IndexError
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4At)
            , TString(L"CQC User Context")
            , TCardinal(9)
        );
    }
    m_objaEnvRTVs[c4At] = strToSet;
}


