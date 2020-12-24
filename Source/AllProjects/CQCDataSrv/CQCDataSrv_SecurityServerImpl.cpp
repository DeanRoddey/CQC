//
// FILE NAME: CQCDataSrv_SecurityServerImpl.cpp
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
#include    "CQCDataSrv.hpp"
#include    "CIDKernel_RegistryWin32.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSecuritySrvImpl,TCQCSecureServerBase)


// ---------------------------------------------------------------------------
//   CLASS: TCQCSecuritySrvImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSecuritySrvImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSecuritySrvImpl::TCQCSecuritySrvImpl() :

    m_colChallenges(29, TMD5KeyOps(), &TCQCSrvSecChallenge::mhashGetKey)
{
}

TCQCSecuritySrvImpl::~TCQCSecuritySrvImpl()
{
}


// ---------------------------------------------------------------------------
//  TCQCSecuritySrvImpl: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Checks the passed security token to see if it is still good. For now, if it
//  decodes successfull, we consider it good since it had to have bene encrypted
//  with our key. But we at some point need to start requiring a refresh if the
//  token is beyond a certain date.
//
tCIDLib::TBoolean
TCQCSecuritySrvImpl::bCheckToken(const TCQCSecToken& sectToCheck)
{
    TLocker lockrSync(&m_mtxSync);

    // Else check it. If it's bad, this will throw, else we'll return true
    TString                 strLoginName;
    tCQCKit::EUserRoles     eRole;
    tCQCKit::EUserTypes     eType;
    tCIDLib::TEncodedTime   enctStamp;
    TString strDefTmpl;     strDefTmpl;
    DecodeSecurityToken(sectToCheck, strLoginName, eRole, eType, enctStamp, strDefTmpl);
    return kCIDLib::True;
}


//
//  This is called by clients to get a security token, during the logon
//  process. If they provide valid credentials, we allow them to logon.
//
tCIDLib::TBoolean
TCQCSecuritySrvImpl::bGetSToken(const   TCQCSecChallenge&    seccResponse
                                ,       TCQCSecToken&        sectToFill
                                ,       TCQCUserAccount&     uaccToFill
                                ,       tCQCKit::ELoginRes&  eResult)
{
    TLocker lockrSync(&m_mtxSync);

    //
    //  Ok, lets try to validate the response. So get the hash id out of the
    //  challenge object and try to look up that id in our pending list.
    //
    TCQCSrvSecChallenge* psseccTest = m_colChallenges.pobjFindByKey(seccResponse.mhashId());

    // If not one, then throw
    if (!psseccTest)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_UnknownChallenge
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );

        // Make analyzer happy
        return kCIDLib::False;
    }

    // See if its stale
    if (TTime::enctNow() > psseccTest->enctStale())
    {
        // Remove it then throw
        m_colChallenges.bRemoveKey(seccResponse.mhashId(), kCIDLib::False);
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_StaleChallenge
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
        );
    }

    // And we must be have a user account for it still
    const TCQCUserAccount* puaccLogin = m_secdCurrent.puaccFind(psseccTest->strLoginName());
    if (!puaccLogin)
    {
        // See if it's the special server account
        if (psseccTest->strLoginName() == m_uaccSrv.strLoginName())
        {
            puaccLogin = &m_uaccSrv;
        }
         else
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcSec_UnknownUser
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , psseccTest->strLoginName()
            );

            // Make analylzer happy
            return kCIDLib::False;
        }
    }

    //
    //  We found one, so ask it to validate the response. If it says its
    //  not a match, then return a failure now.
    //
    if (!psseccTest->bValidateResponse(seccResponse))
    {
        if (facCQCDataSrv.bLogWarnings())
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kDSrvMsgs::midStatus_FailedLogon
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::Protocol
                , psseccTest->strLoginName()
            );
        }
        eResult = tCQCKit::ELoginRes::BadCred;
        return kCIDLib::False;
    }

    //
    //  See if the account has time limits on it. If so, then make sure that
    //  we are within the time limits.
    //
    if (puaccLogin->bLimitTime())
    {
        // Get the current time info
        TTime tmCurrent(tCIDLib::ESpecialTimes::CurrentTime);

        tCIDLib::TCard4 c4Hour;
        tCIDLib::TCard4 c4Minute;
        tCIDLib::TCard4 c4Second;
        tmCurrent.c4AsTimeInfo(c4Hour, c4Minute, c4Second);

        if ((c4Hour < puaccLogin->c4StartHour())
        ||  (c4Hour > puaccLogin->c4EndHour()))
        {
            eResult = tCQCKit::ELoginRes::TimeLimits;
            return kCIDLib::False;
        }
    }

    // Well, it passed, so lets generate them a security token
    EncodeSecToken(sectToFill, *puaccLogin);

    // And give them back the account info, clearing the password info
    uaccToFill = *puaccLogin;
    uaccToFill.ClearPassword();

    // And let them know that they have a good security token
    eResult = tCQCKit::ELoginRes::Ok;

    if (facCQCDataSrv.bLogInfo())
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvMsgs::midStatus_LoggedOn
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , psseccTest->strLoginName()
        );
    }
    return kCIDLib::True;
}


//
//  Returns whether the indicated login name exists. THIS IS CASE INSENSITIVE,
//  so that we can avoid dups with the same spelling but different case, but otherwise
//  require case sensitive names for greater security.
//
tCIDLib::TBoolean
TCQCSecuritySrvImpl::bLoginExists(const TString& strToCheck, const TCQCSecToken& sectUser)
{
    // The caller must provide a power user or above login
    {
        tCQCKit::EUserRoles     eRole;
        tCQCKit::EUserTypes     eType;
        TString                 strLoginName;
        tCIDLib::TEncodedTime   enctStamp;
        TString                 strDefTmpl;
        DecodeSecurityToken(sectUser, strLoginName, eRole, eType, enctStamp, strDefTmpl);
        facCQCKit().TestPrivileges(eRole, tCQCKit::EUserRoles::PowerUser, strLoginName);
    }

    TLocker lockrSync(&m_mtxSync);
    return m_secdCurrent.bLoginExists(strToCheck);
}


tCIDLib::TBoolean
TCQCSecuritySrvImpl::bLoginReq( const   TString&            strLogin
                                ,       TCQCSecChallenge&   seccToFill)
{
    TLocker lockrSync(&m_mtxSync);

    // See if this login is in our list. If not, then return false now
    const TCQCUserAccount* puaccLogin = m_secdCurrent.puaccFind(strLogin);
    if (!puaccLogin)
    {
        // See if it's the special server account
        if (strLogin == m_uaccSrv.strLoginName())
            puaccLogin = &m_uaccSrv;
        else
            return kCIDLib::False;
    }

    // Prune the login challenge list to get rid of any out of date ones
    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    tCIDColAlgo::bRemoveIf(m_colChallenges, [enctNow](const TCQCSrvSecChallenge& sseccCur)
    {
        return enctNow > sseccCur.enctStale();
    });

    //
    //  Create one of our security challenge objects. This is what we need
    //  to store away for the subsequent call to GetSecurityToken(). We'll
    //  use it to fill in the caller's challenge object with challenge data.
    //
    TCQCSrvSecChallenge sseccNew;
    sseccNew.GenerateChallenge(*puaccLogin, seccToFill);

    // And add this guy to our list of pending challenges
    m_colChallenges.objAdd(sseccNew);

    // Return true to tell them we accept the challenge
    return kCIDLib::True;
}


//
//  We validate the passed security token. If it's valid, we return our current event
//  key hash.
//
tCIDLib::TBoolean
TCQCSecuritySrvImpl::bQueryEventKey(        TMD5Hash&       mhashKey
                                    ,       TMD5Hash&       mhashSyId
                                    , const TCQCSecToken&   sectUser)
{
    try
    {
        tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();

        TString                 strName;
        tCQCKit::EUserRoles     eRole;
        tCQCKit::EUserTypes     eType;
        tCIDLib::TEncodedTime   enctStamp;
        TString                 strDefTmpl;
        orbcSS->DecodeSecurityToken(sectUser, strName, eRole, eType, enctStamp, strDefTmpl);

        mhashKey = m_mhashEventKey;
        mhashSyId = m_mhashEventSysId;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        facCQCDataSrv.LogEventObj(errToCatch);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  This is called by the installer to get the secondary server login credentials so that
//  it can set them in the registry on those machines. They have to pass a admin credentials
//  to get this info, and the installer makes them log in as such.
//
tCIDLib::TBoolean
TCQCSecuritySrvImpl::bQuerySrvCreds(        TString&            strUserName
                                    ,       TString&            strPassword
                                    , const TCQCSecToken&       sectUser)
{
    TLocker lockrSync(&m_mtxSync);

    // The caller must pass admin privs
    {
        tCQCKit::EUserRoles     eRole;
        tCQCKit::EUserTypes     eType;
        TString                 strLoginName;
        tCIDLib::TEncodedTime   enctStamp;
        TString                 strDefTmpl;
        DecodeSecurityToken(sectUser, strLoginName, eRole, eType, enctStamp, strDefTmpl);
        facCQCKit().TestPrivileges(eRole, tCQCKit::EUserRoles::SystemAdmin, strLoginName);
    }

    strUserName = m_uaccSrv.strLoginName();
    strPassword = m_strSrvPW;
    return !strUserName.bIsEmpty() && !strPassword.bIsEmpty();
}


//
//  For the web server. Returns the user role and web password of the indicated
//  login account.
//
tCIDLib::TBoolean
TCQCSecuritySrvImpl::bQueryWAccount(const   TString&                strLogin
                                    ,       tCIDLib::TCard4&        c4Size
                                    ,       THeapBuf&               mbufPW
                                    ,       tCQCKit::EUserRoles&    eRole
                                    , const TCQCSecToken&           sectUser)
{
    //
    //  Make sure we zero the size no matter what, because the memory buffer
    //  is an out parm and if it has junk in the size, it'll try to return
    //  that much memory.
    //
    c4Size = 0;
    tCQCKit::EUserTypes     eType;
    TString                 strLoginName;
    tCIDLib::TEncodedTime   enctStamp;
    TString                 strDefTmpl;

    TLocker lockrSync(&m_mtxSync);

    // Validate the passed security token to make sure only at least normal users can query this
    DecodeSecurityToken(sectUser, strLoginName, eRole, eType, enctStamp, strDefTmpl);
    facCQCKit().TestPrivileges(eRole, tCQCKit::EUserRoles::NormalUser, strLoginName);

    // See if we can find this user logon
    const TCQCUserAccount* puaccLogin = m_secdCurrent.puaccFind(strLogin);
    if (!puaccLogin)
        return kCIDLib::False;

    // We have to encrypt it with the hash of the server password
    if (!puaccLogin->strWebPassword().bIsEmpty())
    {
        TCryptoKey ckeyPW(m_mhashSrvPW);
        TBlowfishEncrypter crypWebPW(ckeyPW);
        c4Size = crypWebPW.c4Encrypt(puaccLogin->strWebPassword(), mbufPW);
    }

    eRole = puaccLogin->eRole();
    return kCIDLib::True;
}


//
//  Queries a list of available accounts. The caller must provide creds for at least
//  power user level. This is primiarily for the admin interface. We only return
//  standard user type accounts.
//
tCIDLib::TCard4
TCQCSecuritySrvImpl::c4QueryAccounts(       TVector<TCQCUserAccount>& colToFill
                                    , const TCQCSecToken&             sectUser)
{
    TLocker lockrSync(&m_mtxSync);

    // Make sure that they have at least power user rights
    TString                 strLoginName;
    tCQCKit::EUserRoles     eRole;
    tCQCKit::EUserTypes     eType;
    tCIDLib::TEncodedTime   enctStamp;
    TString                 strDefTmpl;
    DecodeSecurityToken(sectUser, strLoginName, eRole, eType, enctStamp, strDefTmpl);
    facCQCKit().TestPrivileges(eRole, tCQCKit::EUserRoles::PowerUser, strLoginName);

    //
    //  Iterate the accounts collection and copy each account into the caller's
    //  collection.
    //
    //  NOTE: We clear out the password hash so that it cannot be seen, even
    //        by someone with admin rights. And we only return standard accounts.
    //
    TCQCSecurityData::TAccountCursor cursAccounts(m_secdCurrent.cursAccounts());
    for (; cursAccounts; ++cursAccounts)
    {
        if (cursAccounts->eType() == tCQCKit::EUserTypes::Standard)
            colToFill.objAdd(*cursAccounts).ClearPassword();
    }

    return colToFill.c4ElemCount();
}


//
//  Allows the CQC or Web access password for an account to be updated. The
//  caller must pass in the security token for the account being changed,
//  to prove that he knows the current password.
//
tCIDLib::TVoid
TCQCSecuritySrvImpl::ChangePassword(const   TString&            strLogin
                                    , const tCIDLib::TBoolean   bIsCQCPW
                                    , const TCQCSecToken&       sectUser
                                    , const tCIDLib::TCard4     c4DataBytes
                                    , const THeapBuf&           mbufData)
{
    TLocker lockrSync(&m_mtxSync);

    //
    //  Decode the security token. It doesn't matter what their user role is,
    //  just that the security token represents the id of the account whose
    //  password is being changed.
    //
    TString                 strLoginName;
    tCQCKit::EUserRoles     eRole;
    tCQCKit::EUserTypes     eType;
    tCIDLib::TEncodedTime   enctStamp;
    TString                 strDefTmpl;
    DecodeSecurityToken(sectUser, strLoginName, eRole, eType, enctStamp, strDefTmpl);
    if (strLoginName != strLogin)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_NotCurrentUser
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
            , strLogin
        );
    }

    // See if we can find this user logon
    TCQCUserAccount* puaccLogin = m_secdCurrent.puaccFind(strLogin);
    if (!puaccLogin)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_UnknownUser
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strLogin
        );

        // Make analyzer happy
        return;
    }

    //
    //  Use the account's current password as a key hash to decrypt the passed
    //  info.
    //
    TCryptoKey ckeyPW(puaccLogin->mhashPassword());
    TBlowfishEncrypter crypPW(ckeyPW);

    // Decrypt the info into a local buffer
    THeapBuf mbufDecrypted(4192UL);
    const tCIDLib::TCard4 c4DecBytes = crypPW.c4Decrypt
    (
        mbufData, mbufDecrypted, c4DataBytes
    );

    // Create a stream over it so we can pull the flatted info
    TBinMBufInStream strmSrc(&mbufDecrypted, c4DecBytes);

    //
    //  Handle it differently based on whether it's a CQC password update or
    //  a web access password update.
    //
    if (bIsCQCPW)
    {
        // The data should be a streamed MD5 hash
        TMD5Hash mhashPW;
        strmSrc >> mhashPW;

        // Make sure it is the right size for an HD5 hash that we use
        if (mhashPW.c4Bytes() != kCIDCrypto::c4MD5HashBytes)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcSec_BadPWHashSize
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        // And store it
        puaccLogin->mhashPassword(mhashPW);
    }
     else
    {
        //
        //  To avoid any hacking, make sure that this account had a web
        //  password already. If not, then one cannot be set. The admin has
        //  to set a web password before the user can change it.
        //
        if (puaccLogin->strWebPassword().bIsEmpty())
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcSec_NotWebEnabledAcc
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Authority
                , strLogin
            );
        }

        //
        //  For the web password, it's just a string, since we have to store
        //  them as actual text, not a hash.
        //
        TString strWebPS;
        strmSrc >> strWebPS;

        // Udpate the password
        puaccLogin->strWebPassword(strWebPS);
    }

    //
    //  And flush the account info back to the repository. Note that we are
    //  doing a remove call with the mutex locked.
    //
    m_oseRepo.c4UpdateObject(kCQCKit::pszSecKey_UserAccountInfo, m_secdCurrent);
}


//
//  Called by the admin client to create a new account
//
tCIDLib::TVoid
TCQCSecuritySrvImpl::CreateAccount( const   TCQCUserAccount&    uaccNew
                                    , const TCQCSecToken&       sectUser)
{
    TLocker lockrSync(&m_mtxSync);

    //
    //  Make sure that they have sufficient rights. They must have at least
    //  power user rights, period. If they are a power user, they can only
    //  create normal user or limited user accounts.
    //
    TString                 strLoginName;
    tCQCKit::EUserRoles     eRole;
    tCQCKit::EUserTypes     eType;
    tCIDLib::TEncodedTime   enctStamp;
    TString                 strDefTmpl;
    DecodeSecurityToken(sectUser, strLoginName, eRole, eType, enctStamp, strDefTmpl);

    if ((eRole < tCQCKit::EUserRoles::PowerUser)
    ||  ((eRole == tCQCKit::EUserRoles::PowerUser)
    &&   (uaccNew.eRole() >  tCQCKit::EUserRoles::NormalUser)))
    {
        // Throwing a CQCKit error here
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_InsufficientRole
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
            , strLoginName
        );
    }

    // Get a copy of the account, and set a default password
    TCQCUserAccount uaccTmp(uaccNew);
    uaccTmp.SetPassword(L"Welcome");

    //
    //  Ask the security data object to add this new account. They must call
    //  UpdateAccount() to modify an existing account. This will throw if the
    //  name is a dup (case insensitive.) Clients should enforce that on their
    //  end but we catch them just in case.
    //
    m_secdCurrent.AddUserAccount(uaccTmp);

    // And flush the account info back to the repo
    m_oseRepo.c4UpdateObject(kCQCKit::pszSecKey_UserAccountInfo, m_secdCurrent);

    // Log that an account was created
    if (facCQCDataSrv.bLogInfo())
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvMsgs::midStatus_NewAccount
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , uaccNew.strLoginName()
        );
    }
}


//
//  Allows admins to delete accounts. We get the login of the account to
//  delete, plus we have to get a security token for an administrative
//  account which has the rights to do this.
//
tCIDLib::TVoid
TCQCSecuritySrvImpl::DeleteAccount( const   TString&        strLogin
                                    , const TCQCSecToken&   sectUser)
{
    TLocker lockrSync(&m_mtxSync);

    //
    //  Make sure that they have sufficient rights. They must have at least
    //  power user rights, period. If they are a power user, they can only
    //  delete normal user or limited user accounts.
    //
    TString                 strLoginName;
    tCQCKit::EUserRoles     eRole;
    tCQCKit::EUserTypes     eType;
    tCIDLib::TEncodedTime   enctStamp;
    TString                 strDefTmpl;
    DecodeSecurityToken(sectUser, strLoginName, eRole, eType, enctStamp, strDefTmpl);

    // See if we can find this user logon
    const TCQCUserAccount* puaccLogin = m_secdCurrent.puaccFind(strLogin);
    if (!puaccLogin)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_UnknownUser
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strLogin
        );

        // Make analyzer happy
        return;
    }

    if ((eRole < tCQCKit::EUserRoles::PowerUser)
    ||  ((eRole == tCQCKit::EUserRoles::PowerUser)
    &&   (puaccLogin->eRole() >  tCQCKit::EUserRoles::NormalUser)))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_InsufficientRole
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
            , strLoginName
        );

        // Make analyzer happy
        return;
    }

    // They cannot delete their current account
    if (strLogin == strLoginName)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_DeleteSelf
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
    }

    // Ok, looks like its legal to delete it
    m_secdCurrent.DeleteAccount(strLogin);

    // And flush the account info back to the repo
    m_oseRepo.c4UpdateObject(kCQCKit::pszSecKey_UserAccountInfo, m_secdCurrent);

    // Log that an account was created
    if (facCQCDataSrv.bLogInfo())
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvMsgs::midStatus_AccountDeleted
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , strLogin
        );
    }
}


//
//  Used by this and other servers to decode security tokens to see if they
//  are valid and what the user role is that they represent. Also returns
//  some user account info that's commonly desired.
//
tCIDLib::TVoid
TCQCSecuritySrvImpl::DecodeSecurityToken(const  TCQCSecToken&           sectSrc
                                        ,       TString&                strLoginName
                                        ,       tCQCKit::EUserRoles&    eRole
                                        ,       tCQCKit::EUserTypes&    eType
                                        ,       tCIDLib::TEncodedTime&  enctStamp
                                        ,       TString&                strDefTmpl)
{
    TLocker lockrSync(&m_mtxSync);

    try
    {
        // Decrypt the token buffer using our current decrypter
        THeapBuf mbufTmp(1024UL);
        const tCIDLib::TCard4 c4Bytes = m_crypTokens.c4Decrypt(sectSrc.mbufData(), mbufTmp, sectSrc.c4Bytes());

        // Create an input stream over the decrypted buffer
        TBinMBufInStream strmSrc(&mbufTmp, c4Bytes);

        // We should get a start marker first
        strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

        // Ok, looks reasonable, so pull in the other fields
        strmSrc >> strLoginName
                >> eRole
                >> eType
                >> enctStamp;

        // Check if it's stale
        // We aren't currently doing anything for this
        // if (enctStamp > enctWhatever)

        // And we should end up with an end marker
        strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);

        // Look this account up and get the default template out
        const TCQCUserAccount* puaccLogin = m_secdCurrent.puaccFind(strLoginName);
        if (!puaccLogin)
        {
            // See if it's the special server account
            if (strLoginName == m_uaccSrv.strLoginName())
            {
                puaccLogin = &m_uaccSrv;
            }
             else
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcSec_UnknownUser
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , strLoginName
                );

                // Make analyzer happy
                return;
            }
        }
        strDefTmpl = puaccLogin->strDefInterfaceName();
    }

    catch(const TError& errToCatch)
    {
        if (facCQCDataSrv.bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        // Throw a bad security token error
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_InvalidToken
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
}



//
//  Returns account information (sans password hash) for the indicated
//  login. The caller must pass a security token for the user on whose
//  behalf this query is being made, since only certain users can get
//  this info.
//
tCIDLib::TVoid
TCQCSecuritySrvImpl::QueryAccount(  const   TString&            strLogin
                                    , const TCQCSecToken&       sectUser
                                    ,       TCQCUserAccount&    uaccToFill)
{
    TLocker lockrSync(&m_mtxSync);

    // Decode the passed security token
    TString                 strLoginName;
    tCQCKit::EUserRoles     eRole;
    tCQCKit::EUserTypes     eType;
    tCIDLib::TEncodedTime   enctStamp;
    TString                 strDefTmpl;
    DecodeSecurityToken(sectUser, strLoginName, eRole, eType, enctStamp, strDefTmpl);

    //
    //  The caller must either be a power user or greater, or the requested
    //  account must be the account the person is logged into.
    //
    if ((eRole < tCQCKit::EUserRoles::PowerUser)
    &&  (strLoginName != strLogin))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_InsufficientRole
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
            , strLoginName
        );
    }

    // See if we can find this user logon
    const TCQCUserAccount* puaccLogin = m_secdCurrent.puaccFind(strLogin);
    if (!puaccLogin)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_UnknownUser
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strLogin
        );

        // Make analyzer happy
        return;
    }

    // Copy it to the caller's account, and clear the password info
    uaccToFill = *puaccLogin;
    uaccToFill.ClearPassword();
}


//
//  Just returns our CQC version, as a sanity check for clients, so that they
//  can warn the user if the client is out of version.
//
tCIDLib::TCard8 TCQCSecuritySrvImpl::c8QueryVersion()
{
    return kCQCKit::c8Ver_Current;
}


//
//  Let's the admins reset account passwords when they have been forgotten
//  and whatnot.
//
tCIDLib::TVoid
TCQCSecuritySrvImpl::ResetAccountPassword(  const   TString&        strLogin
                                            , const TString&        strNewPW
                                            , const TCQCSecToken&   sectUser)
{
    TLocker lockrSync(&m_mtxSync);

    // Decode the passed security token
    TString                 strLoginName;
    tCQCKit::EUserRoles     eRole;
    tCQCKit::EUserTypes     eType;
    tCIDLib::TEncodedTime   enctStamp;
    TString                 strDefTmpl;
    DecodeSecurityToken(sectUser, strLoginName, eRole, eType, enctStamp, strDefTmpl);

    //
    //  For security, we do a first level check now. We make sure that they
    //  are a power user at least. If we combined this with the other check
    //  below, then someone could just do a dictionary attack to find user
    //  names, after getting any legal login. This way, they have to at least
    //  dup a power user or system admin.
    //
    facCQCKit().TestPrivileges(eRole, tCQCKit::EUserRoles::PowerUser, strLoginName);

    // See if we can find this user logon
    const TCQCUserAccount* puaccLogin = m_secdCurrent.puaccFind(strLogin);
    if (!puaccLogin)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_UnknownUser
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strLogin
        );

        // Make analyzer happy
        return;
    }

    //
    //  Make sure that they have sufficient rights. If they are a power user,
    //  they can only reset normal user or limited user accounts. If they aren't
    //  a power user, our check above insures that they are a system admin.
    //
    if ((eRole == tCQCKit::EUserRoles::PowerUser)
    &&  (puaccLogin->eRole() > tCQCKit::EUserRoles::NormalUser))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_InsufficientRole
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
            , strLoginName
        );
    }

    // Looks ok so reset the account
    m_secdCurrent.ResetAccountPassword(strLogin, strNewPW);

    // And flush the account info back to the repository
    m_oseRepo.c4UpdateObject(kCQCKit::pszSecKey_UserAccountInfo, m_secdCurrent);

    // Log that the account was reset
    if (facCQCDataSrv.bLogInfo())
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvMsgs::midStatus_AccountReset
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , strLogin
        );
    }
}


//
//  Called to update a user account. It's called from the admin client.
//
tCIDLib::TVoid
TCQCSecuritySrvImpl::UpdateAccount( const   TCQCUserAccount&    uaccNewData
                                    , const TCQCSecToken&       sectUser)
{
    TLocker lockrSync(&m_mtxSync);

    //
    //  Make sure that they either have a system admin user role,or if they
    //  are a power user, that the target account is a normal/limited user
    //  account.
    //
    TString                 strLoginName;
    tCQCKit::EUserRoles     eRole;
    tCQCKit::EUserTypes     eType;
    tCIDLib::TEncodedTime   enctStamp;
    TString                 strDefTmpl;
    DecodeSecurityToken(sectUser, strLoginName, eRole, eType, enctStamp, strDefTmpl);

    if ((eRole < tCQCKit::EUserRoles::PowerUser)
    ||  ((eRole == tCQCKit::EUserRoles::PowerUser)
    &&   (uaccNewData.eRole() > tCQCKit::EUserRoles::NormalUser)))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_InsufficientRole
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
            , strLoginName
        );
    }

    //
    //  No account can change its own user role, so find this account
    //  and see if its the same one as the user whose security token we
    //  got. If so, then make sure that role wasn't changed.
    //
    const TCQCUserAccount* puaccUpdate
                    = m_secdCurrent.puaccFind(uaccNewData.strLoginName());
    if (puaccUpdate && (puaccUpdate->strLoginName() == strLoginName))
    {
        if (uaccNewData.eRole() != puaccUpdate->eRole())
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcSec_ChangeSelfRole
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Authority
            );
        }
    }

    //
    //  Ok, update it. We know that this will not change the login name,
    //  since that's what we looked it up by.
    //
    m_secdCurrent.UpdateAccount(uaccNewData);

    // And flush the account info back to the  repository
    m_oseRepo.c4UpdateObject(kCQCKit::pszSecKey_UserAccountInfo, m_secdCurrent);
}


// ---------------------------------------------------------------------------
//  TCQCSecuritySrvImpl: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We don't sync this since its only called during startup, before any
//  clients can get to us.
//
tCQCSrvFW::EStateRes TCQCSecuritySrvImpl::eLoadSecurityInfo(tCIDLib::TCard4& c4WaitNext)
{
    //
    //  If the repo path doesn't exist, let's try to create it. Tell it not
    //  to throw if the path already exists, but it will if the path cannot
    //  be created.
    //
    TFileSys::MakeDirectory(facCQCKit().strRepositoryDir(), kCIDLib::False);

    //
    //  Let's initialize the object store. This will create if if needed or open it
    //  if it exists.
    //
    try
    {
        m_oseRepo.bInitialize(facCQCKit().strRepositoryDir(), L"SecuritySrvData");
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return tCQCSrvFW::EStateRes::Retry;
    }


    // Try a read and see if we find anything
    tCIDLib::TCard4 c4Version = 0;
    tCIDLib::ELoadRes eRes = m_oseRepo.eReadObject
    (
        kCQCKit::pszSecKey_UserAccountInfo, c4Version, m_secdCurrent, kCIDLib::False
    );

    //
    //  If not found then we can't continue, though if we are running in the
    //  development environment we'll create a default account in order to allow
    //  for starting up from an empty repo.
    //
    if (eRes == tCIDLib::ELoadRes::NotFound)
    {
        if (facCQCKit().bDevMode())
        {
            // Create a default initial system admin account
            m_secdCurrent.AddUserAccount
            (
                tCQCKit::EUserRoles::SystemAdmin
                , L"Default system admin account"
                , L"Admin"
                , L"Welcome"
                , L"System"
                , L"Administrator"
            );

            //
            //  Add this object to the repository. Give it a good bit of reserve
            //  space because this one can grow a good bit.
            //
            m_oseRepo.AddObject(kCQCKit::pszSecKey_UserAccountInfo, m_secdCurrent, 8 * 1024);
        }
         else
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcSec_NoAccounts
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );

            // We have to fail here
            c4WaitNext = 10000;
            return tCQCSrvFW::EStateRes::Failed;
        }
    }

    //
    //  Look for the master hash. If not found, we can't do anything, though if we are in
    //  the development environment generate a random one so we can continue.
    //
    c4Version = 0;
    TMD5Hash mhashMaster;
    eRes = m_oseRepo.eReadObject(kCQCKit::pszSecKey_MasterHash, c4Version, mhashMaster, kCIDLib::False);
    if (eRes == tCIDLib::ELoadRes::NotFound)
    {
        if (facCQCKit().bDevMode())
        {
            mhashMaster = TUniqueId::mhashMakeId();
        }
         else
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcSec_NoMasterHash
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );

            // We have to fail here
            c4WaitNext = 10000;
            return tCQCSrvFW::EStateRes::Failed;
        }
    }
    m_crypTokens.SetNewKey(TCryptoKey(mhashMaster));

    //
    //  Look for the event key hash. If not found, we'll generate on on the fly if in
    //  development. Else we fail and retry later.
    //
    //  Then the same for the event system id.
    //
    c4Version = 0;
    eRes = m_oseRepo.eReadObject(kCQCKit::pszSecKey_EventKey, c4Version, m_mhashEventKey, kCIDLib::False);
    if (eRes == tCIDLib::ELoadRes::NotFound)
    {
        if (facCQCKit().bDevMode())
        {
            m_mhashEventKey = TUniqueId::mhashMakeId();
        }
         else
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcSec_NoEventKey
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );

            // We have to fail here
            c4WaitNext = 10000;
            return tCQCSrvFW::EStateRes::Failed;
        }
    }

    c4Version = 0;
    eRes = m_oseRepo.eReadObject(kCQCKit::pszSecKey_EventSysId, c4Version, m_mhashEventSysId, kCIDLib::False);
    if (eRes == tCIDLib::ELoadRes::NotFound)
    {
        if (facCQCKit().bDevMode())
        {
            m_mhashEventSysId = TUniqueId::mhashMakeId();
        }
         else
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcSec_NoEventSysId
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );

            // We have to fail here
            c4WaitNext = 10000;
            return tCQCSrvFW::EStateRes::Failed;
        }
    }


    // Find any accounts of the secondary server type
    tCQCKit::TUserAcctList colAccounts;
    if (m_secdCurrent.bFindAccountsOfType(tCQCKit::EUserTypes::SecServer, colAccounts))
    {
        // If there is more than one, something is really wrong, so remove them all
        if (colAccounts.c4ElemCount() > 1)
            m_secdCurrent.bRemoveAllOfType(tCQCKit::EUserTypes::SecServer);
    }

    // And now deal with whether we have one or not
    if (colAccounts.bIsEmpty())
    {
        //
        //  If in development mode we can create a dummy one with known values. Else we
        //  have to complain that we don't have one and fail.
        //
        if (facCQCKit().bDevMode())
        {
            //
            //  We store the password text in the description for this one since we need
            //  to have the actual password text to send to the installer.
            //
            m_uaccSrv = TCQCUserAccount
            (
                tCQCKit::EUserRoles::PowerUser
                , L"Welcome"
                , L"SecServer"
                , L"Welcome"
                , L"Secondary"
                , L"Server"
            );
            m_uaccSrv.eType(tCQCKit::EUserTypes::SecServer);
            m_secdCurrent.AddUserAccount(m_uaccSrv);

            TMessageDigest5 mdigPW;
            mdigPW.DigestStr(m_uaccSrv.strDescription());
            mdigPW.Complete(m_mhashSrvPW);

            m_strSrvPW = L"Welcome";
        }
         else
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcSec_NoSrvAccount
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );

            // We have to fail here
            c4WaitNext = 10000;
            return tCQCSrvFW::EStateRes::Failed;
        }
    }
     else
    {
        // Save away the info for the account we found
        TMessageDigest5 mdigPW;
        m_uaccSrv = colAccounts[0];
        mdigPW.DigestStr(m_uaccSrv.strDescription());
        mdigPW.Complete(m_mhashSrvPW);
        m_strSrvPW = m_uaccSrv.strDescription();

        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvMsgs::midStatus_SecSrvCredsLoaded
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    if (facCQCDataSrv.bLogStatus())
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvMsgs::midStatus_SecurityLoadDone
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    return tCQCSrvFW::EStateRes::Success;
}


// ---------------------------------------------------------------------------
//  TCQCSecuritySrvImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCSecuritySrvImpl::Initialize()
{
    // Nothing to do at this time, just pass to our parent
    TParent::Initialize();
}


tCIDLib::TVoid TCQCSecuritySrvImpl::Terminate()
{
    // Nothing to do at this time, just pass to our parent
    TParent::Terminate();
}


// ---------------------------------------------------------------------------
//  TCQCSecuritySrvImpl: Private, non-virtual methods
// ---------------------------------------------------------------------------

// A helper to encode a security token
tCIDLib::TVoid
TCQCSecuritySrvImpl::EncodeSecToken(        TCQCSecToken&           sectToFill
                                    , const TCQCUserAccount&        uaccToUse)
{
    //
    //  Create a temp output memory stream, and stream out the data. Note
    //  that we don't format version this object because its never persisted.
    //  Its just a runtime structure.
    //
    TBinMBufOutStream strmOut(1024UL);
    strmOut << tCIDLib::EStreamMarkers::StartObject
            << uaccToUse.strLoginName()
            << uaccToUse.eRole()
            << uaccToUse.eType()
            << TTime::enctNow()
            << tCIDLib::EStreamMarkers::EndObject
            << kCIDLib::FlushIt;

    //
    //  Ok, now encrypt that using our current token encrypter, into the
    //  a temp buffer.
    //
    THeapBuf mbufTmp(1024UL);
    const tCIDLib::TCard4 c4Bytes = m_crypTokens.c4Encrypt
    (
        strmOut.mbufData(), mbufTmp, strmOut.c4CurSize()
    );

    // And set up the token data on the token object
    sectToFill.Set(mbufTmp, c4Bytes);
}


