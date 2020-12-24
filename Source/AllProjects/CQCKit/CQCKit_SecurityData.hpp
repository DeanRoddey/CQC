//
// FILE NAME: CQCKit_SecurityData.hpp
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
//  This class contains informaiton we track about users at runtime. The security
//  server has to maintain a list of users so that it can validate logins and provide
//  user role info and such.
//
//  The user accounts are stored in a hash set, which is set up for case sensitive
//  hashing, since we want to enforce case on user login names. HOWEVER, we provide
//  a bLoginExists() method to allow clients to check for dups even if a different
//  case, so that we can avoid accounts with the same spelling but different breasts.
//
//  We also, as a fallback, check for such dups when a new account is added, though we
//  expect clients to call the above check and reject such case dups.
//
//
// CAVEATS/GOTCHAS:
//
//  1)  WE ASSUME that the security server interface that uses us will synchronize
//      access to us, so we do no sync.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCSecurityData
//  PREFIX: secd
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCSecurityData : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TUserList      = TKeyedHashSet<TCQCUserAccount,TString,TStringKeyOps>;
        using TAccountCursor = TUserList::TCursor;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSecurityData();

        TCQCSecurityData(const TCQCSecurityData&) = delete;
        TCQCSecurityData(TCQCSecurityData&&) = delete;

        ~TCQCSecurityData();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSecurityData& operator=(const TCQCSecurityData&) = delete;
        TCQCSecurityData& operator=(TCQCSecurityData&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddUserAccount
        (
            const   TCQCUserAccount&        uaccToAdd
        );

        tCIDLib::TVoid AddUserAccount
        (
            const   tCQCKit::EUserRoles     eRole
            , const TString&                strDescription
            , const TString&                strLoginName
            , const TString&                strPassword
            , const TString&                strFirstName
            , const TString&                strLastName
        );

        tCIDLib::TBoolean bFindAccountsOfType
        (
            const   tCQCKit::EUserTypes     eType
            ,       tCQCKit::TUserAcctList& colToFill
        )   const;

        tCIDLib::TBoolean bLoginExists
        (
            const   TString&                strToCheck
        )   const;

        tCIDLib::TBoolean bRemoveAllOfType
        (
            const   tCQCKit::EUserTypes     eType
        );

        TUserList::TCursor cursAccounts();

        tCIDLib::TVoid DeleteAccount
        (
            const   TString&                strLoginName
        );

        const TCQCUserAccount* puaccFind
        (
            const   TString&                strLoginName
        )   const;

        TCQCUserAccount* puaccFind
        (
            const   TString&                strLoginName
        );

        tCIDLib::TVoid ResetAccountPassword
        (
            const   TString&                strLogin
            , const TString&                strNewPW
        );

        tCIDLib::TVoid UpdateAccount
        (
            const   TCQCUserAccount&        uaccNewData
        );


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


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colUsers
        //      A collection of the currently defined users accounts. It is
        //      a hash set, keyed on the user name, which is the access scheme
        //      we need to make fast for logon purposes.
        //
        //      We set the key ops to be non-case sensitive, so that they cannot
        //      add two users with the same spelling but different case.
        // -------------------------------------------------------------------
        TUserList   m_colUsers;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSecurityData,TObject)
};

#pragma CIDLIB_POPPACK


