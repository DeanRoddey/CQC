//
// FILE NAME: CQCKit_UserAccount.hpp
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
//  This file defines the contents of a user account. It holds the information
//  for a CQC defined user, which gives them the ability to log on and provides
//  them with certain rights once logged in. Rights are based on 'roles', of
//  which there are a small set. Each user account is assigned to one of these
//  roles, and that sets the rights of that user. Each user account can only
//  be assigned one role. Users which require different roles must have multiple
//  account.
//
//  Rights are not strictly defined, since each device driver decides what
//  roles are required to access its functionality. However, in fundamentl
//  terms, the roles and their rights are:
//
//  1.System Administrator
//      All possible rights, on all machines in the network. These users
//      maintain the system. Only system admins can add or remove or
//      configure CQC drivers or CQC servers. Only a system admin can
//      create another system admin or power user user account.
//
//  2.PowerUser
//      Has all the rights of a normal user, but can also create user
//      drawn interfaces. Can create user accounts, but only for the
//      Normal User or Limited User roles. Some types of device drivers
//      may allow some or full configuration capabilities for power users,
//      if that configuration is not considered too dangerous.
//
//  3.Normal User
//      A trusted non-administrative user, such as a spouse for instance.
//      No configuration rights are given, but these users can run the
//      CQCClient GUI. They can interact with client driver GUIs, but
//      cannot modify any configuration.
//
//  4.Limited User
//      A less trusted non-administrative user, such as a adolescent
//      family member. Cannot run the standard CQC client GUI, so this
//      user is limited to user drawn interfaces created by the system
//      admins.
//
//  As of 1.5, there is also an optional web password, which can be used by
//  the user to access secure pages on the CQC web server. This password should
//  be different from the main password, but doesn't have to be. It must be
//  stored as plain text since that's required to do the HTTP Digest
//  authentication.
//
//
//  We also define an E-mail account. This is not part of the CQC user
//  account stuff, it's for defining and storing named e-mail accounts in
//  the master config server. Action commands to send e-mails references these
//  named accounts.
//
//  And we also define a small class that is used by the action editor dialog,
//  to allow each user to have a cheat sheet of variable names used in the
//  actions he/she edits. This is not part of the user account, it's just
//  stored on a per user basis on the master server.
//
//  And we define one that holds the user context info that is required by
//  many helper methods and therefore has to be passed down from the app
//  that provided the login to the stuff that needs it. Since we have apps
//  (XML GW and RIVA) that can have multiple user sessions per process, we
//  can't just store it away in a common area for everyone to access.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCEMailAccount
//  PREFIX: emacct
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCEMailAccount : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCEMailAccount();

        TCQCEMailAccount
        (
            const   TString&                strNewName
            , const TCQCEMailAccount&       emacctSrc
        );

        TCQCEMailAccount(const TCQCEMailAccount&) = default;
        TCQCEMailAccount(TCQCEMailAccount&&) = default;

        ~TCQCEMailAccount();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCEMailAccount& operator=(const TCQCEMailAccount&) = default;
        TCQCEMailAccount& operator=(TCQCEMailAccount&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCEMailAccount&       uaccSrc

        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCEMailAccount&       uaccSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::EEMailTypes eType() const;

        tCIDLib::TIPPortNum ippnMail() const;

        const TString& strAcctName() const;

        const TString& strFromAddr() const;

        const TString& strPassword() const;

        const TString& strServer() const;

        const TString& strUserName() const;

        tCIDLib::TVoid Set
        (
            const   tCQCKit::EEMailTypes    eType
            , const TString&                strAcctName
            , const TString&                strFromAddr
            , const TString&                strPassword
            , const TString&                strServer
            , const TString&                strUserName
            , const tCIDLib::TIPPortNum     ippnMail
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
        //  m_eType
        //      The type of e-mail account, i.e. SMTP, POP3, etc...
        //
        //  m_ippnMail
        //      The port to use when talking to the mail server.
        //
        //  m_strAcctName
        //      The name of the account, which is how it is referenced in the
        //      e-mail sending commands. This is also the unique id for the
        //      account, and we limit it to alpha-nums so that it can be used
        //      as the config server storage key name as well, for fast
        //      access.
        //
        //  m_strFromAddr
        //      The e-mail address that the message should show up as coming
        //      from.
        //
        //  m_strPassword
        //  m_strUserName
        //      The name and password to send in the e-mail.
        //
        //  m_strServer
        //      The DNS name of the server to send the e-mail through.
        // -------------------------------------------------------------------
        tCQCKit::EEMailTypes    m_eType;
        tCIDLib::TIPPortNum     m_ippnMail;
        TString                 m_strAcctName;
        TString                 m_strFromAddr;
        TString                 m_strPassword;
        TString                 m_strServer;
        TString                 m_strUserName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCEMailAccount,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCActVarList
//  PREFIX: cavl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCActVarList : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCActVarList();

        TCQCActVarList(const TCQCActVarList&) = default;

        ~TCQCActVarList();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCActVarList& operator=(const TCQCActVarList&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddNew
        (
            const   TString&                strName
            , const TString&                strNotes
        );

        tCIDLib::TBoolean bChanges() const;

        tCIDLib::TBoolean bIsEmpty() const;

        tCIDLib::TBoolean bIsFull() const;

        tCIDLib::TBoolean bNameExists
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4At
        )   const;

        tCIDLib::TBoolean bRemoveName
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4At
        );

        tCIDLib::TCard4 c4VarCount() const;

        tCIDLib::TVoid ClearChanges();

        tCIDLib::TVoid DeleteAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TKeyValuePair& kvalAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid MoveToTop
        (
            const   tCIDLib::TCard4         c4Src
        );

        tCIDLib::TVoid SetNameAt
        (
            const   tCIDLib::TCard4         c4At
            , const TString&                strToSet
        );

        tCIDLib::TVoid SetNoteAt
        (
            const   tCIDLib::TCard4         c4At
            , const TString&                strToSet
        );

        const TString& strTopKey() const;


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
        )   const   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bChanges
        //      We keep up with whether any changes have been made, so that
        //      the client code can only store us away if needed. This isn't
        //      persisted. It's cleared when we are streamed in and set any
        //      time we are modified after that. It's not cleared when we
        //      are streamed out since that streaming doesn't insure we got
        //      stored successfully.
        //
        //  m_colList
        //      We have a vector of key/value pairs. The key is the var
        //      name and the value is any notes for that variable. We keep
        //      it sorted by the variable name.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bChanges;
        tCIDLib::TKVPList   m_colList;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCActVarList,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCUserAccount
//  PREFIX: uacc
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCUserAccount : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompOnLogin
        (
            const   TCQCUserAccount&        uacc1
            , const TCQCUserAccount&        uacc2
        );

        static const TString& strGetKey
        (
            const   TCQCUserAccount&        uaccSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCUserAccount();

        TCQCUserAccount
        (
            const   tCQCKit::EUserRoles     eRole
            , const TString&                strDescription
            , const TString&                strLoginName
            , const TString&                strPassword
            , const TString&                strFirstName
            , const TString&                strLastName
        );

        TCQCUserAccount(const TCQCUserAccount&) = default;
        TCQCUserAccount(TCQCUserAccount&&) = default;

        ~TCQCUserAccount() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCUserAccount& operator=(const TCQCUserAccount&) = default;
        TCQCUserAccount& operator=(TCQCUserAccount&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCUserAccount&        uaccSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCUserAccount&        uaccSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bLimitTime() const
        {
            return m_bLimitTime;
        }

        tCIDLib::TBoolean bLimitTime
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4EndHour() const
        {
            return m_c4EndHour;
        }

        tCIDLib::TCard4 c4EndHour
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4StartHour() const
        {
            return m_c4StartHour;
        }

        tCIDLib::TCard4 c4StartHour
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid ClearPassword();

        tCQCKit::EUserRoles eRole() const
        {
            return m_eRole;
        }

        tCQCKit::EUserRoles eRole
        (
            const   tCQCKit::EUserRoles     eToSet
        );

        tCQCKit::EUserTypes eType() const
        {
            return m_eType;
        }

        tCQCKit::EUserTypes eType
        (
            const   tCQCKit::EUserTypes     eToSet
        );

        const TMD5Hash& mhashPassword() const
        {
            return m_mhashPassword;
        }

        const TMD5Hash& mhashPassword
        (
            const   TMD5Hash&               mhashToSet
        );

        const TString& strDescription() const
        {
            return m_strDescription;
        }

        const TString& strDescription
        (
            const   TString&                strToSet
        );

        const TString& strFirstName() const
        {
            return m_strFirstName;
        }

        const TString& strLastName() const
        {
            return m_strLastName;
        }

        const TString& strDefInterfaceName() const
        {
            return m_strDefIntfName;
        }

        const TString& strDefInterfaceName
        (
            const   TString&                strToSet
        );

        const TString& strLoginName() const
        {
            return m_strLoginName;
        }

        const TString& strLoginName
        (
            const   TString&                strToSet
        );

        const TString& strUserId() const
        {
            return m_strUserId;
        }

        const TString& strUserId
        (
            const   TString&                strToSet
        );

        const TString& strWebPassword() const
        {
            return m_strWebPassword;
        }

        const TString& strWebPassword
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetName
        (
            const   TString&                strFirstToSet
            , const TString&                strLastToSet
        );

        tCIDLib::TVoid SetPassword
        (
            const   TString&                strToSet
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
        //  m_bLimitTime
        //      Indicates whether the start/end times should be used.
        //
        //  m_bSystemAccount
        //      This is used to mark system accounts, which are not reported by
        //      the security server when accounts as queried. These are used by
        //      CQC back end services.
        //
        //  m_c4EndHour
        //  m_c4StartHour
        //      The start and end hours of the day that the user can log on,
        //      in 24 hour day format. Ignored unless the m_bLimitTime flag
        //      is set. Inclusive of the last hour, so 0 to 23 means any time.
        //
        //  m_eRole
        //      The role assigned to this user.
        //
        //  m_mhashPassword
        //      The hashed password of the user. The actual password cannot
        //      be recreated. Only the user knows that. When this object is
        //      retrieved over the remote interface from the data server, this
        //      field will be empty, for security reasons. There is never any
        //      need for any client application to know this information.
        //
        //  m_strDefIntfName
        //      The interface that will always be shown initially for
        //      limited users when the run the interface viewer (the only
        //      client app they can run.)
        //
        //  m_strDescription
        //      An optional description field for this user account
        //
        //  m_strFirstName
        //  m_strLastName
        //      The formal first,last name of the user. This is just for
        //      display and human consumption.
        //
        //  m_strLoginName
        //      The login name of this user. This is what they type into the
        //      user name login screen.
        //
        //  m_strUserId
        //      A unique id generated for each user account. Its use for
        //      user account references purposes, so that the user name can
        //      be changed if needed, without invalidating references.
        //
        //  strWebPassword
        //      The optional password for access via the web server. If not
        //      used, it'll be blank. Added in 1.5.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bLimitTime;
        tCIDLib::TCard4     m_c4EndHour;
        tCIDLib::TCard4     m_c4StartHour;
        tCQCKit::EUserRoles m_eRole;
        tCQCKit::EUserTypes m_eType;
        TMD5Hash            m_mhashPassword;
        TString             m_strDefIntfName;
        TString             m_strDescription;
        TString             m_strFirstName;
        TString             m_strLastName;
        TString             m_strLoginName;
        TString             m_strUserId;
        TString             m_strWebPassword;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCUserAccount,TObject)
};


#pragma CIDLIB_POPPACK


