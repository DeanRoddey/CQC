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
RTTIDecls(TCQCEMailAccount,TObject)
RTTIDecls(TCQCActVarList,TObject)
RTTIDecls(TCQCUserAccount,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_UserAccount
    {
        // -----------------------------------------------------------------------
        //  Our current persistent email account format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2EMFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  Our current persistent variable list format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2VLFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  Our current persistent user account format version
        //
        //  Version 2 -
        //      Added the web password in version 1.5.
        //
        //  Version 3 -
        //      No format change, but hierarchical interface storage was added, so
        //      we have to add a \User\ prefix to the default template if present.
        //
        //  Version 4 -
        //      No format change, but we need to convert the template path to the
        //      forward slash format used in 5.0.
        //
        //  Version 5 -
        //      Added the 'account type' enum, which lets us define some special user
        //      accounts.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2UserFmtVersion = 5;


        // -----------------------------------------------------------------------
        //  Maximum values in the action variables list
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4       c4ActVarsListSz = 128;
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCEMailAccount
//  PREFIX: emacct
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCEMailAccount::TCQCEMailAccount() :

    m_eType(tCQCKit::EEMailTypes::Unused)
    , m_ippnMail(tCIDLib::TIPPortNum(tCIDLib::EIPPorts::SMTP))
{
}


// Copy one but with a new name
TCQCEMailAccount::TCQCEMailAccount( const   TString&            strName
                                    , const TCQCEMailAccount&   emacctSrc) :

    m_eType(emacctSrc.m_eType)
    , m_ippnMail(emacctSrc.m_ippnMail)
    , m_strAcctName(strName)
    , m_strFromAddr(emacctSrc.m_strFromAddr)
    , m_strPassword(emacctSrc.m_strPassword)
    , m_strServer(emacctSrc.m_strServer)
    , m_strUserName(emacctSrc.m_strUserName)
{
}

TCQCEMailAccount::~TCQCEMailAccount()
{
}


// ---------------------------------------------------------------------------
//  Public operators
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCEMailAccount::operator==(const TCQCEMailAccount& emacctSrc) const
{
    if (this == &emacctSrc)
        return kCIDLib::True;

    return
    (
        (m_eType == emacctSrc.m_eType)
        && (m_ippnMail == emacctSrc.m_ippnMail)
        && (m_strAcctName == emacctSrc.m_strAcctName)
        && (m_strFromAddr == emacctSrc.m_strFromAddr)
        && (m_strPassword == emacctSrc.m_strPassword)
        && (m_strServer == emacctSrc.m_strServer)
        && (m_strUserName == emacctSrc.m_strUserName)
    );
}

tCIDLib::TBoolean TCQCEMailAccount::operator!=(const TCQCEMailAccount& emacctSrc) const
{
    return !operator==(emacctSrc);
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCKit::EEMailTypes TCQCEMailAccount::eType() const
{
    return m_eType;
}


tCIDLib::TIPPortNum TCQCEMailAccount::ippnMail() const
{
    return m_ippnMail;
}


const TString& TCQCEMailAccount::strAcctName() const
{
    return m_strAcctName;
}


const TString& TCQCEMailAccount::strFromAddr() const
{
    return m_strFromAddr;
}


const TString& TCQCEMailAccount::strPassword() const
{
    return m_strPassword;
}


const TString& TCQCEMailAccount::strServer() const
{
    return m_strServer;
}


const TString& TCQCEMailAccount::strUserName() const
{
    return m_strUserName;
}


tCIDLib::TVoid
TCQCEMailAccount::Set(  const   tCQCKit::EEMailTypes    eType
                        , const TString&                strAcctName
                        , const TString&                strFromAddr
                        , const TString&                strPassword
                        , const TString&                strServer
                        , const TString&                strUserName
                        , const tCIDLib::TIPPortNum     ippnMail)
{
    m_eType       = eType;
    m_ippnMail    = ippnMail;
    m_strAcctName = strAcctName;
    m_strFromAddr = strFromAddr;
    m_strPassword = strPassword;
    m_strServer   = strServer;
    m_strUserName = strUserName;
}


// ---------------------------------------------------------------------------
//  Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCEMailAccount::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check for a start marker and valid format version
    const tCIDLib::TCard2 c2FmtVersion = TBinInStream::c2CheckFmtVersion
    (
        strmToReadFrom
        , tCIDLib::EStreamMarkers::StartObject
        , CQCKit_UserAccount::c2EMFmtVersion
        , clsThis()
        , CID_FILE
        , CID_LINE
    );

    strmToReadFrom  >> m_eType
                    >> m_strAcctName
                    >> m_ippnMail
                    >> m_strFromAddr
                    >> m_strPassword
                    >> m_strServer
                    >> m_strUserName;

    // And the end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

}


tCIDLib::TVoid TCQCEMailAccount::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_UserAccount::c2EMFmtVersion
                    << m_eType
                    << m_strAcctName
                    << m_ippnMail
                    << m_strFromAddr
                    << m_strPassword
                    << m_strServer
                    << m_strUserName
                    << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCActVarList
//  PREFIX: cavl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCActVarList: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCActVarList::TCQCActVarList() :

    m_bChanges(kCIDLib::False)
    , m_colList(CQCKit_UserAccount::c4ActVarsListSz)
{
}

TCQCActVarList::~TCQCActVarList()
{
}


// ---------------------------------------------------------------------------
//  TCQCActVarList: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCActVarList::AddNew(  const   TString&    strName
                                        , const TString&    strNotes)
{
    //
    //  If it's full, then we have to throw out the one at the end of the
    //  list.
    //
    if (m_colList.bIsFull(CQCKit_UserAccount::c4ActVarsListSz))
        m_colList.bRemoveLast();

    // And insert this one at the top
    m_colList.InsertAt(TKeyValuePair(strName, strNotes), 0);

    // Mark us as changed
    m_bChanges = kCIDLib::True;
}


tCIDLib::TBoolean TCQCActVarList::bChanges() const
{
    return m_bChanges;
}


tCIDLib::TBoolean TCQCActVarList::bIsEmpty() const
{
    return m_colList.bIsEmpty();
}


tCIDLib::TBoolean TCQCActVarList::bIsFull() const
{
    return m_colList.bIsFull(CQCKit_UserAccount::c4ActVarsListSz);
}


tCIDLib::TBoolean
TCQCActVarList::bNameExists(const   TString&            strName
                            ,       tCIDLib::TCard4&    c4At) const
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colList[c4Index].strKey() == strName)
        {
            c4At = c4Index;
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCActVarList::bRemoveName(const TString& strName, tCIDLib::TCard4& c4At)
{
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colList[c4Index].strKey() == strName)
        {
            c4At = c4Index;
            m_colList.RemoveAt(c4Index);
            m_bChanges = kCIDLib::True;
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


tCIDLib::TCard4 TCQCActVarList::c4VarCount() const
{
    return m_colList.c4ElemCount();
}


tCIDLib::TVoid TCQCActVarList::ClearChanges()
{
    m_bChanges = kCIDLib::False;
}


tCIDLib::TVoid TCQCActVarList::DeleteAt(const tCIDLib::TCard4 c4At)
{
    m_bChanges = kCIDLib::True;
    m_colList.RemoveAt(c4At);
}


const TKeyValuePair& TCQCActVarList::kvalAt(const tCIDLib::TCard4 c4At) const
{
    return m_colList[c4At];
}


// Moves the indicated entry to the top
tCIDLib::TVoid TCQCActVarList::MoveToTop(const tCIDLib::TCard4 c4Src)
{
    if (c4Src)
    {
        m_bChanges = kCIDLib::True;
        TKeyValuePair kvalTmp = m_colList[c4Src];
        m_colList.RemoveAt(c4Src);
        m_colList.InsertAt(kvalTmp, 0);
    }
}


tCIDLib::TVoid
TCQCActVarList::SetNameAt(const tCIDLib::TCard4 c4At, const TString& strToSet)
{
    TKeyValuePair& kvalTar = m_colList[c4At];
    if (strToSet != kvalTar.strKey())
    {
        kvalTar.strKey(strToSet);

        // We have to resort
        m_colList.Sort(&TKeyValuePair::eCompKey);
        m_bChanges = kCIDLib::True;
    }
}



tCIDLib::TVoid
TCQCActVarList::SetNoteAt(const tCIDLib::TCard4 c4At, const TString& strNotes)
{
    TKeyValuePair& kvalTar = m_colList[c4At];
    if (strNotes != kvalTar.strValue())
    {
        kvalTar.strValue(strNotes);
        m_bChanges = kCIDLib::True;
    }
}


const TString& TCQCActVarList::strTopKey() const
{
    return m_colList[0].strKey();
}


// ---------------------------------------------------------------------------
//  TCQCActVarList: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCActVarList::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check for a start marker and valid format version
    const tCIDLib::TCard2 c2FmtVersion = TBinInStream::c2CheckFmtVersion
    (
        strmToReadFrom
        , tCIDLib::EStreamMarkers::StartObject
        , CQCKit_UserAccount::c2VLFmtVersion
        , clsThis()
        , CID_FILE
        , CID_LINE
    );

    strmToReadFrom  >>  m_colList;

    // Clear the changes flag since this is fresh stuff
    m_bChanges = kCIDLib::False;

    // And the end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

}


tCIDLib::TVoid TCQCActVarList::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_UserAccount::c2VLFmtVersion
                    << m_colList
                    << tCIDLib::EStreamMarkers::EndObject;
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCUserAccount
//  PREFIX: uacc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCUserAccount: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TCQCUserAccount::eCompOnLogin(  const   TCQCUserAccount&    uacc1
                                , const TCQCUserAccount&    uacc2)
{
    return uacc1.m_strLoginName.eCompareI(uacc2.m_strLoginName);
}


const TString& TCQCUserAccount::strGetKey(const TCQCUserAccount& uaccSrc)
{
    return uaccSrc.strLoginName();
}


// ---------------------------------------------------------------------------
//  TCQCUserAccount: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCUserAccount::TCQCUserAccount() :

    m_bLimitTime(kCIDLib::False)
    , m_c4EndHour(23)
    , m_c4StartHour(0)
    , m_eRole(tCQCKit::EUserRoles::LimitedUser)
    , m_eType(tCQCKit::EUserTypes::Standard)
    , m_mhashPassword()
    , m_strDescription()
    , m_strLoginName()
    , m_strUserId(TUniqueId::strMakeId())
{
}

TCQCUserAccount::TCQCUserAccount(const  tCQCKit::EUserRoles eRole
                                , const TString&            strDescription
                                , const TString&            strLoginName
                                , const TString&            strPassword
                                , const TString&            strFirstName
                                , const TString&            strLastName) :

    m_bLimitTime(kCIDLib::False)
    , m_c4EndHour(23)
    , m_c4StartHour(0)
    , m_eRole(eRole)
    , m_eType(tCQCKit::EUserTypes::Standard)
    , m_strDescription(strDescription)
    , m_strFirstName(strFirstName)
    , m_strLastName(strLastName)
    , m_strLoginName(strLoginName)
    , m_strUserId(TUniqueId::strMakeId())
{
    // Store the hash of the user password
    TMessageDigest5 mdigTmp;
    mdigTmp.DigestStr(strPassword);
    mdigTmp.Complete(m_mhashPassword);
}


// ---------------------------------------------------------------------------
//  TCQCUserAccount: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCUserAccount::operator==(const TCQCUserAccount& uaccSrc) const
{
    if (this == &uaccSrc)
        return kCIDLib::True;

    return
    (
        (m_bLimitTime        == uaccSrc.m_bLimitTime)
        && (m_c4EndHour      == uaccSrc.m_c4EndHour)
        && (m_c4StartHour    == uaccSrc.m_c4StartHour)
        && (m_eRole          == uaccSrc.m_eRole)
        && (m_eType          == uaccSrc.m_eType)
        && (m_mhashPassword  == uaccSrc.m_mhashPassword)
        && (m_strDefIntfName == uaccSrc.m_strDefIntfName)
        && (m_strDescription == uaccSrc.m_strDescription)
        && (m_strFirstName   == uaccSrc.m_strFirstName)
        && (m_strLastName    == uaccSrc.m_strLastName)
        && (m_strLoginName   == uaccSrc.m_strLoginName)
        && (m_strUserId      == uaccSrc.m_strUserId)
        && (m_strWebPassword == uaccSrc.m_strWebPassword)
    );
}

tCIDLib::TBoolean TCQCUserAccount::operator!=(const TCQCUserAccount& uaccSrc) const
{
    return !operator==(uaccSrc);
}


// ---------------------------------------------------------------------------
//  TCQCUserAccount: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCUserAccount::bLimitTime(const tCIDLib::TBoolean bToSet)
{
    m_bLimitTime = bToSet;
    return m_bLimitTime;
}


tCIDLib::TCard4 TCQCUserAccount::c4EndHour(const tCIDLib::TCard4 c4ToSet)
{
    m_c4EndHour = c4ToSet;
    return m_c4EndHour;
}


tCIDLib::TCard4 TCQCUserAccount::c4StartHour(const tCIDLib::TCard4 c4ToSet)
{
    m_c4StartHour = c4ToSet;
    return m_c4StartHour;
}


tCIDLib::TVoid TCQCUserAccount::ClearPassword()
{
    m_mhashPassword.Zero();
}


tCQCKit::EUserRoles TCQCUserAccount::eRole(const tCQCKit::EUserRoles eToSet)
{
    m_eRole = eToSet;
    return m_eRole;
}


tCQCKit::EUserTypes TCQCUserAccount::eType(const tCQCKit::EUserTypes eToSet)
{
    m_eType = eToSet;
    return m_eType;
}

const TMD5Hash& TCQCUserAccount::mhashPassword(const TMD5Hash& mhashToSet)
{
    m_mhashPassword = mhashToSet;
    return m_mhashPassword;
}


const TString& TCQCUserAccount::strDefInterfaceName(const TString& strToSet)
{
    m_strDefIntfName = strToSet;
    return m_strDefIntfName;
}


const TString& TCQCUserAccount::strDescription(const TString& strToSet)
{
    m_strDescription = strToSet;
    return m_strDescription;
}


const TString& TCQCUserAccount::strLoginName(const TString& strToSet)
{
    m_strLoginName = strToSet;
    return m_strLoginName;
}


const TString& TCQCUserAccount::strUserId(const TString& strToSet)
{
    m_strUserId = strToSet;
    return m_strUserId;
}


const TString& TCQCUserAccount::strWebPassword(const TString& strToSet)
{
    m_strWebPassword = strToSet;
    return m_strWebPassword;
}


tCIDLib::TVoid TCQCUserAccount::SetName(const   TString&    strFirstToSet
                                        , const TString&    strLastToSet)
{
    m_strFirstName = strFirstToSet;
    m_strLastName  = strLastToSet;
}


tCIDLib::TVoid TCQCUserAccount::SetPassword(const TString& strToSet)
{
    //
    //  Hash the password using MD5 and store the result in our password hash
    //  member.
    //
    TMessageDigest5 mdigTmp;
    mdigTmp.DigestStr(strToSet);
    mdigTmp.Complete(m_mhashPassword);
}


// ---------------------------------------------------------------------------
//  TCQCUserAccount: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCUserAccount::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check for a start marker and valid format version
    const tCIDLib::TCard2 c2FmtVersion = TBinInStream::c2CheckFmtVersion
    (
        strmToReadFrom
        , tCIDLib::EStreamMarkers::StartObject
        , CQCKit_UserAccount::c2UserFmtVersion
        , clsThis()
        , CID_FILE
        , CID_LINE
    );

    strmToReadFrom  >> m_bLimitTime
                    >> m_c4StartHour
                    >> m_c4EndHour
                    >> m_eRole
                    >> m_mhashPassword
                    >> m_strDefIntfName
                    >> m_strDescription
                    >> m_strFirstName
                    >> m_strLastName
                    >> m_strLoginName
                    >> m_strUserId;

    // If version 2, read in the web password, else clear it
    if (c2FmtVersion >= 2)
        strmToReadFrom >> m_strWebPassword;
    else
        m_strWebPassword.Clear();

    // If less than V3, move the default interface into the user area
    if ((c2FmtVersion < 3) && !m_strDefIntfName.bIsEmpty())
        m_strDefIntfName.Insert(kCQCKit::strRepoScope_UserTS, 0);

    //
    //  If less than 4, then we have to flip the slashes in the interface path. We
    //  also need to deal with double slashes they may have used.
    //
    if (c2FmtVersion < 4)
        facCQCKit().Make50Path(m_strDefIntfName);

    // If less than 5, then default the user type
    if (c2FmtVersion < 5)
        m_eType = tCQCKit::EUserTypes::Standard;
    else
        strmToReadFrom >> m_eType;

    // And the end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCUserAccount::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_UserAccount::c2UserFmtVersion
                    << m_bLimitTime
                    << m_c4StartHour
                    << m_c4EndHour
                    << m_eRole
                    << m_mhashPassword
                    << m_strDefIntfName
                    << m_strDescription
                    << m_strFirstName
                    << m_strLastName
                    << m_strLoginName
                    << m_strUserId

                    // Do V2 stuff
                    << m_strWebPassword

                    // Do V5 stuff
                    << m_eType

                    << tCIDLib::EStreamMarkers::EndObject;
}

