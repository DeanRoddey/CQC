//
// FILE NAME: CQCKit_SecurityData.cpp
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
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic Securitys
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSecurityData,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_SecurityData
    {
        // -------------------------------------------------------------------
        //  Our current and previous persistent format versions
        //
        //  Version 2 -
        //      No actual change in format, but since we don't prevent accounts with the
        //      same spelling but different case (relying on clients not to do it) that
        //      could happen in the recent CQC Voice changes. So we need to remove the
        //      dups.
        // -------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 2;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCSecurityData
//  PREFIX: secd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSecurityData: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSecurityData::TCQCSecurityData() :

    m_colUsers(29, TStringKeyOps(), &TCQCUserAccount::strGetKey)
{
}

TCQCSecurityData::~TCQCSecurityData()
{
}


// ---------------------------------------------------------------------------
//  TCQCSecurityData: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCSecurityData::AddUserAccount(const TCQCUserAccount& uaccToAdd)
{
    //
    //  Reject any duplicates.
    //
    //  NOTE THAT WE CALL bLoginExists() so that we reject as dups those that are
    //  spelled the same but different case.
    //
    if (bLoginExists(uaccToAdd.strLoginName()))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_UserExists
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , uaccToAdd.strLoginName()
        );
    }

    // They don't mind if it exists, so add or update
    tCIDLib::TBoolean bAdded;
    m_colUsers.objAddOrUpdate(uaccToAdd, bAdded);
}

tCIDLib::TVoid
TCQCSecurityData::AddUserAccount(const  tCQCKit::EUserRoles eRole
                                , const TString&            strDescription
                                , const TString&            strLoginName
                                , const TString&            strPassword
                                , const TString&            strFirstName
                                , const TString&            strLastName)
{
    // Just create a temp user account and call the other version
    TCQCUserAccount uaccTmp
    (
        eRole
        , strDescription
        , strLoginName
        , strPassword
        , strFirstName
        , strLastName
    );
    AddUserAccount(uaccTmp);
}


// Finds all accounts marked with a given type
tCIDLib::TBoolean
TCQCSecurityData::bFindAccountsOfType(  const   tCQCKit::EUserTypes     eType
                                        ,       tCQCKit::TUserAcctList& colToFill) const
{
    colToFill.RemoveAll();
    TAccountCursor cursUsers(&m_colUsers);
    for (; cursUsers; ++cursUsers)
    {
        if (cursUsers->eType() == eType)
            colToFill.objAdd(*cursUsers);
    }
    return !colToFill.bIsEmpty();
}


//
//  This will check to see if a login exist, in a case insensitive way. This is what
//  is used to see if an account is a dup, even though we otherwise are case
//  sensitive.
//
tCIDLib::TBoolean TCQCSecurityData::bLoginExists(const TString& strToCheck) const
{
    // We have to do a manual iteration and case insensitive compares
    TAccountCursor cursUsers(&m_colUsers);
    for (; cursUsers; ++cursUsers)
    {
        if (cursUsers->strLoginName().bCompareI(strToCheck))
            return kCIDLib::True;
    }

    // Never found it
    return kCIDLib::False;
}


//
//  Removes all accounts of a particular user type. Returns true if any were
//  removed.
//
tCIDLib::TBoolean TCQCSecurityData::bRemoveAllOfType(const tCQCKit::EUserTypes eType)
{
    return tCIDColAlgo::bRemoveIf(m_colUsers, [eType](const TCQCUserAccount& uaccCur)
    {
        return uaccCur.eType() == eType;
    });
}



TCQCSecurityData::TAccountCursor TCQCSecurityData::cursAccounts()
{
    return TAccountCursor(&m_colUsers);
}


tCIDLib::TVoid TCQCSecurityData::DeleteAccount(const TString& strLoginName)
{
    // Just delegate to the user account collection
    m_colUsers.bRemoveKey(strLoginName);
}


const TCQCUserAccount*
TCQCSecurityData::puaccFind(const TString& strLoginName) const
{
    return m_colUsers.pobjFindByKey(strLoginName);
}

TCQCUserAccount* TCQCSecurityData::puaccFind(const TString& strLoginName)
{
    return m_colUsers.pobjFindByKey(strLoginName);
}


// Reset the passed of the indicated account with teh new password
tCIDLib::TVoid
TCQCSecurityData::ResetAccountPassword( const   TString&    strLoginName
                                        , const TString&    strNewPassword)
{
    // See if we can find this user logon
    TCQCUserAccount* puaccUpdate = m_colUsers.pobjFindByKey(strLoginName);
    if (!puaccUpdate)
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
    }

    // Looks ok, so set the password
    puaccUpdate->SetPassword(strNewPassword);
}


tCIDLib::TVoid
TCQCSecurityData::UpdateAccount(const TCQCUserAccount& uaccNewData)
{
    // See if we can find this user logon
    TCQCUserAccount* puaccUpdate
                        = m_colUsers.pobjFindByKey(uaccNewData.strLoginName());
    if (!puaccUpdate)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcSec_UnknownUser
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , uaccNewData.strLoginName()
        );
    }

    //
    //  We found it, so update it now. Note that we don't copy in the
    //  password hash. That is never given out, so when we get these items
    //  back, they have an empty on, or something bogus if someone set it.
    //
    TCQCUserAccount uaccTmp(uaccNewData);
    uaccTmp.mhashPassword(puaccUpdate->mhashPassword());
    *puaccUpdate = uaccTmp;
}


// ---------------------------------------------------------------------------
//  TCQCSecurityData: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCSecurityData::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_SecurityData::c2FmtVersion))
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

    if (c2FmtVersion == 1)
    {
        //
        //  We have to do a special case scenario here, to make sure we get rid of
        //  any dups (same spelling, different case.) So read them in first.
        //
        strmToReadFrom >> m_colUsers;

        //
        //  Now we have to check each one, and for each one, check the ones after
        //  it for dups. This is much easier in a vector, so get copies into a
        //  vector for that. The user account comp function is case insensitive so
        //  this will gather any dups together, making this much easier.
        //
        TVector<TCQCUserAccount> colSorted;
        TAccountCursor cursList(&m_colUsers);
        if (cursList.bIsValid())
        {
            tCIDLib::TCard4 c4At;
            for (; cursList; ++cursList)
            {
                colSorted.InsertSorted
                (
                    *cursList, TCQCUserAccount::eCompOnLogin, c4At
                );
            }

            //
            //  Now loop through and find any adjacent dups. Only need to do this if
            //  more than one, and that simplifies the loop code as well.
            //
            tCIDLib::TCard4 c4Count = colSorted.c4ElemCount();
            if (c4Count > 1)
            {
                tCIDLib::TCard4 c4Index = 0;
                while (c4Index + 1 < c4Count)
                {
                    const TString& strFirst = colSorted[c4Index].strLoginName();
                    const TString& strSec = colSorted[c4Index + 1].strLoginName();
                    if (strFirst.bCompareI(strSec))
                    {
                        // Remove the second and stay where we are
                        colSorted.RemoveAt(c4Index + 1);
                        c4Count--;
                    }
                     else
                    {
                        // No dup, so moe forward
                        c4Index++;
                    }
                }
            }
        }

        // Now let's update the actual list with the ones we kept
        m_colUsers.RemoveAll();
        const tCIDLib::TCard4 c4Count = colSorted.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            m_colUsers.objAdd(colSorted[c4Index]);
    }
     else
    {
        strmToReadFrom  >> m_colUsers;
    }

    // And check the end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCSecurityData::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_SecurityData::c2FmtVersion
                    << m_colUsers
                    << tCIDLib::EStreamMarkers::EndObject;
}


