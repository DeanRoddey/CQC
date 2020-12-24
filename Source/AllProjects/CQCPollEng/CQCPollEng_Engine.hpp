//
// FILE NAME: CQCPollEng_Engine.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/25/2008
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
//  This is the header for the CQCPollEng_Engine.cpp file, which implements
//  the (suprise) background polling engine. Clients can create instances
//  of the engine as desired. They register fields with the engine, and
//  periodically refresh their desire to keep fields registered by asking for
//  the currnet value. The engine polls registered fields regularly. Any
//  fields that are not accessed for over a period of time are dropped from
//  the poll list. Any servers not accessed at all for a while will be pruned
//  from the list as well.
//
//  The engine has a number of internal classes that represent servers, drivers
//  and fields. The client doesn't know anything about those. All the outside
//  world needs to do is register fields and refresh that registration
//  periodically by reading the value.
//
//  We also define a field link class, which contains all the information
//  that the engine needs to identify and track changes in a given field.
//  The client just keeps them as basically magic cookies which they can
//  pass back in when they ask for the value. It also allows the polling
//  engine to quickly figure out if the field link is still valid and to
//  find the linked to field if so.
//
//  We define the TCQCFldPollInfo class, which is not used here but provides
//  for tracking the kind of info that most any client will need in order to
//  track the value of a field over time. They don't have to use it but most
//  would because it does so much work for them. It includes the field link
//  info along with the most recently gotten value, the last serial number
//  of the field that was seen, and the last state of the field that was
//  seen.
//
//  This polling engine is very asynchronous. For each CQCServer that we
//  are actively polling (or trying to connect to so that we can poll it)
//  there is a thread inside the server item that tries to maintain the
//  connection to the server and poll it periodically. There is also a
//  thread at the engine level that periodically checks to see if any servers
//  have not been accessed for a while and will remove them from the list
//  for efficiency if so. And of course we have incoming calls from clients
//  to register fields for polling and to their their values subsequently.
//
//  The server item thread has to poll the CQCServer it's talking to and
//  then update the field info with the most recently values. And it must
//  periodically remove fields from the poll list if they have not be accessed
//  in a while. And of course a server item should not be removed if it is
//  currently active or being accessed by a client.
//
//  So we have two levels of locking. There is an overall mutex on the engine
//  which insures that only one client or the internal engine thread can be
//  accessing the list at a given time. Each server item also has a mutex
//  that insures that only one client or the server item's polling thread
//  is accessing the server item at a time.
//
//  The exception is that the server item must be able to poll (a remote call
//  that might take some time in worst cases) but it cannot hold the lock
//  across that call which would prevent clients from getting field info. So
//  the m_fiopPoll poll list object can ONLY be accessed from the server
//  item's polling thread, so that it can do a poll call without locking it's
//  mutex. Once it is back from the poll call, it locks and stores the data.
//
//  It also means that incoming calls by clients to register a new field for
//  polling cannot add those fields to the poll list directly since the
//  server's poll thread may be using it. So there is a simple vector that
//  new registration requests will be added to, and the poll thread will
//  grab them and add them to the list before its next poll.
//
//  We provide a janitor class internally that handles the locking and
//  server, driver, field lookup chores. This makes things vastly easier.
//  The public methods that the clients call will first lock the main
//  mutex to insure that the pruning thread cannot drop the server (and no
//  other client can get in), then it will create the janitor and let it
//  find the server and lock it, and look up any driver and field that are
//  being referenced. Then the client can do what it needs safely. The
//  janitor will release the server when it destructs.
//
//  If a driver/field is accessed that is not in the list, unless told
//  otherwise, the janitor will force that new server objects onto the list
//  of servers, then it will return a failure since the field cannot be
//  available until the server item's poll thread gets connected to the
//  target CQCServer. The client will come back and try again later and by
//  then the field should be available if the driver is really running.
//
// CAVEATS/GOTCHAS:
//
//  1)  All of the public APIs of the polling engine MUST update the last
//      accessed time stamp of field items when they access them. The engine
//      cannot do it internally, since it doesn't know if a method is being
//      called from inside or outside, and that could allow internal methods
//      to keep a field forever on the polling list which would be bad.
//
//      So it has to be done from the public access methods.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


class TCQCPollEngine;
class TPESrvJanitor;


// ---------------------------------------------------------------------------
//   CLASS: TCQCPollEngFldLink
//  PREFIX: flnk
// ---------------------------------------------------------------------------
class CQCPOLLENGEXPORT TCQCPollEngFldLink : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCPollEngFldLink();

        TCQCPollEngFldLink
        (
            const   tCIDLib::TCard4         c4ServerId
            , const tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FieldId
        );

        TCQCPollEngFldLink(const TCQCPollEngFldLink&) = default;
        TCQCPollEngFldLink(TCQCPollEngFldLink&&) = delete;

        ~TCQCPollEngFldLink();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TBoolean operator==
        (
            const   TCQCPollEngFldLink&     flnkSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCPollEngFldLink&     flnkSrc
        )   const;

        TCQCPollEngFldLink& operator=(const TCQCPollEngFldLink&) = default;
        TCQCPollEngFldLink& operator=(TCQCPollEngFldLink&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FieldId() const
        {
            return m_c4FieldId;
        }

        tCIDLib::TCard4 c4FieldListId() const
        {
            return m_c4FieldListId;
        }

        tCIDLib::TCard4 c4DriverId() const
        {
            return m_c4DriverId;
        }

        tCIDLib::TCard4 c4DriverListId() const
        {
            return m_c4DriverListId;
        }

        tCIDLib::TCard4 c4ServerId() const
        {
            return m_c4ServerId;
        }

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid Set
        (
            const   tCIDLib::TCard4         c4ServerId
            , const tCIDLib::TCard4         c4DriverListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FieldId
        );

        tCIDLib::TVoid SetFieldId
        (
            const   tCIDLib::TCard4         c4FieldId
        );


        tCIDLib::TVoid SetServerId
        (
            const   tCIDLib::TCard4         c4ServerId
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4DriverId
        //  m_c4DriverListId
        //      The id for this driver and the driver list id for which this
        //      driver is valid. These come from CQCServer.
        //
        //  m_c4FieldId
        //  m_c4FieldListId
        //      The id for this field and the driver's field list for which
        //      this driver id is valid. These come from CQCServer.
        //
        //  m_c4ServerId
        //      The polling engine assigns a unique id to each server item
        //      and updates it with a new one if the server is dropped or
        //      contact is lost with it and then regained.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4DriverId;
        tCIDLib::TCard4         m_c4DriverListId;
        tCIDLib::TCard4         m_c4FieldId;
        tCIDLib::TCard4         m_c4FieldListId;
        tCIDLib::TCard4         m_c4ServerId;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCPollEngFldLink,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldPollInfo
//  PREFIX: cfpi
// ---------------------------------------------------------------------------
class CQCPOLLENGEXPORT TCQCFldPollInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldPollInfo();

        TCQCFldPollInfo
        (
            const   TString&                strMoniker
            , const TString&                strField
        );

        TCQCFldPollInfo
        (
            const   TCQCFldPollInfo&        cfpiSrc
        );

        TCQCFldPollInfo(TCQCFldPollInfo&&) = delete;

        ~TCQCFldPollInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldPollInfo& operator=
        (
            const   TCQCFldPollInfo&        cfpiSrc
        );

        TCQCFldPollInfo& operator=(TCQCFldPollInfo&&) = delete;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHasField() const;

        tCIDLib::TBoolean bHasGoodValue() const;

        tCIDLib::TBoolean bRegister
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        );

        tCIDLib::TBoolean bUpdateValue
        (
                    TCQCPollEngine&         polleToUse
        );

        tCQCPollEng::EFldStates eState() const;

        const TCQCFldDef& flddAssoc() const;

        const TCQCPollEngFldLink& flnkAssoc() const;

        const TCQCFldValue& fvCurrent() const;

        tCIDLib::TVoid FormatToXML
        (
                    TTextOutStream&         strmTarget
        )   const;

        tCIDLib::TVoid RejectField();

        tCIDLib::TVoid Reset();

        const TString& strFieldName() const;

        const TString& strFullFldName() const;

        const TString& strMake() const;

        const TString& strModel() const;

        const TString& strMoniker() const;

        tCIDLib::TVoid SetField
        (
            const   TString&                strMoniker
            , const TString&                strField
        );

        tCIDLib::TVoid SetMoniker
        (
            const   TString&                strMoniker
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DoFieldReg
        (
                    TCQCPollEngine&         polleToUse
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4SerialNum
        //      Used to know when we get a new value. The engine maintains a
        //      serial number for each field, bumped when it changes, either
        //      value or state.
        //
        //  m_eLastState
        //      The last state we saw for the field, during registration or
        //      update. Let's us know where we stand, and we just keep trying
        //      to get it up to ready for polling.
        //
        //  m_flddAssoc
        //      This is gotten when we successfully register our field with
        //      the polling engine.
        //
        //  m_flnkAssoc
        //      We get this info from the polling engine when we register the
        //      field (and it will get updated sometimes during polling), and
        //      we just pass it back in during value updates, since it lets
        //      the polling engine efficiently find our field and know if it
        //      has changed.
        //
        //  m_pfvCurrent
        //      This is a polymorphic field value pointer. According to the
        //      type of the field set on us, we allocate it for the appropriate
        //      field type.
        //
        //  m_strField
        //      The name of the field we are associated with. This is somewhat
        //      redundant in that it will also be in the field definition member
        //      as well after we are registered successfully.
        //
        //  m_strFullFldName
        //      Client code often wants the full moniker.field type name, so
        //      we pre-build that so that we can return it efficiently.
        //
        //  m_strMake
        //  m_strModel
        //      The make and model of the driver that owns our field. Some
        //      clients need to filter on this and only support specific
        //      make/models.
        //
        //  m_strMoniker
        //      The moniker of the field we are associated with.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4SerialNum;
        tCQCPollEng::EFldStates     m_eLastState;
        TCQCFldDef                  m_flddAssoc;
        TCQCPollEngFldLink          m_flnkAssoc;
        TCQCFldValue*               m_pfvCurrent;
        TString                     m_strField;
        TString                     m_strFullFldName;
        TString                     m_strMake;
        TString                     m_strModel;
        TString                     m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldPollInfo,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCPollEngine
//  PREFIX: polle
// ---------------------------------------------------------------------------
class CQCPOLLENGEXPORT TCQCPollEngine : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  The possible states that a server item can be in within the
        //  interface engine.
        //
        //  Offline   - Haven't found find the server. Need to try to connect
        //              on each poll round. After a period of time it will be
        //              dropped if it cannot connect.
        //  Unloaded  - Haven't got good driver/field data from the server.
        //              Need to try to get this info. After a period of time
        //              it will be dropped if we can't get data.
        //  Idle      - No fields are currently being polled for this server.
        //              If it stays in this state for a while, it will get
        //              dropped.
        //  Ready     - Got drivers/fields, and some are live so we need to
        //              be polled.
        // -------------------------------------------------------------------
        enum class ESrvStates
        {
            Offline
            , NotLoaded
            , Idle
            , Ready
        };


        // -------------------------------------------------------------------
        //   CLASS: TFldItem
        //  PREFIX: fldi
        // -------------------------------------------------------------------
        class TFldItem
        {
            public :
                // -----------------------------------------------------------
                //  Public, static methods
                // -----------------------------------------------------------
                static const TString& strKey
                (
                    const   TFldItem&           fldiSrc
                );


                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TFldItem() = delete;

                TFldItem
                (
                    const   tCIDLib::TCard4     c4DriverId
                    , const TCQCFldDef&         flddData
                    , const TCQCPollEngFldLink& flnkToSet
                );

                TFldItem(const TFldItem&) = delete;
                TFldItem(TFldItem&&) = delete;

                ~TFldItem();


                // -----------------------------------------------------------
                //  Public operators
                // -----------------------------------------------------------
                TFldItem& operator=(const TFldItem&) = delete;
                TFldItem& operator=(TFldItem&&) = delete;


                // -----------------------------------------------------------
                //  Public, non-virtual methods
                // -----------------------------------------------------------
                tCIDLib::TBoolean bInError() const;

                tCIDLib::TBoolean bOnPollList() const;

                tCIDLib::TBoolean bOnPollList
                (
                    const   tCIDLib::TBoolean   bToSet
                );

                tCIDLib::TBoolean bPolledSince
                (
                    const   tCIDLib::TEncodedTime enctSince
                )   const;

                tCIDLib::TCard4 c4FieldId() const;

                tCIDLib::TCard4 c4SerialNum() const;

                tCIDLib::TEncodedTime enctLastAccess() const;

                const TCQCFldDef& flddInfo() const;

                const TCQCPollEngFldLink& flnkThis() const;

                const TCQCFldValue& fvCurrent() const;

                TCQCFldValue& fvCurrent();

                tCIDLib::TVoid IncSerialNum();

                tCIDLib::TVoid QueryValue
                (
                            TCQCFldValue&       fvToFill
                );

                const TString& strName() const;

                const TString& strValue() const;

                tCIDLib::TVoid SetErrorState();

                tCIDLib::TVoid SetLastAccessStamp() const;

                tCIDLib::TVoid UpdateServerId
                (
                    const   tCIDLib::TCard4     c4NewSrvId
                );


            private :
                // -----------------------------------------------------------
                //  Private data members
                //
                //  m_bOnPollList
                //      Multiple client objects may want to access the same
                //      field. So we need to know if a field is already on
                //      the poll list or not, so that the subsequent calls
                //      to register it can know it doesn't need to try to
                //      put it onto the poll list yet again.
                //
                //  m_c4FmtSerialNum
                //      The format serial number is used internally and lets
                //      us know when we need to refresh the string formatted
                //      version, which we only update upon access.
                //
                //  m_enctLastAccess
                //      If a field hasn't been read by anyone for the last
                //      so many seconds, the server will periodically remove
                //      them from the poll I/O list. It has to be mutable
                //      since it's updated during otherwise constant read
                //      ops.
                //
                //      This also implies that if this value is later than
                //      the server's last prune time, then we are currently
                //      on the servers' I/O list.
                //
                //  m_flddInfo
                //      This is the field description info, which tells us
                //      all about the field. This is gotten from our driver
                //      which advertises its available fields.
                //
                //  m_flnkThis
                //      This holds our link information, which tracks all of
                //      the ids associated with a field.
                //
                //  m_pfvCurrent
                //      The current value of this field. We use the simple set
                //      of polymorphic field value classes provided by CQCKit.
                //      We store them here via the base class.
                //
                //  m_strValue
                //      We commonly provide the value as a string, so we keep
                //      the value in text format as well, and fault it in upon
                //      first access. The m_c4FmtSerialNum field is used to
                //      know if we need to update the string format because
                //      the field value has been changed.
                // -----------------------------------------------------------
                tCIDLib::TBoolean               m_bOnPollList;
                mutable tCIDLib::TCard4         m_c4FmtSerialNum;
                mutable tCIDLib::TEncodedTime   m_enctLastAccess;
                TCQCFldDef                      m_flddInfo;
                TCQCPollEngFldLink              m_flnkThis;
                TCQCFldValue*                   m_pfvCurrent;
                mutable TString                 m_strValue;
        };
        using TFldList = TRefKeyedHashSet<TFldItem,TString,TStringKeyOps>;
        using TFldIdList = TRefVector<TFldItem>;


        // -------------------------------------------------------------------
        //   CLASS: TDrvItem
        //  PREFIX: drvi
        // -------------------------------------------------------------------
        class TDrvItem
        {
            public :
                // -----------------------------------------------------------
                //  Public, static methods
                // -----------------------------------------------------------
                static const TString& strKey
                (
                    const   TDrvItem&           drviSrc
                );


                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TDrvItem();

                TDrvItem
                (
                    const   TString&            strMoniker
                    , const tCIDLib::TCard4     c4DriverId
                    , const TString&            strMake
                    , const TString&            strModel
                );

                TDrvItem(const TDrvItem&) = delete;
                TDrvItem(TDrvItem&&) = delete;

                ~TDrvItem();


                // -----------------------------------------------------------
                //  Public operators
                // -----------------------------------------------------------
                TDrvItem& operator=(const TDrvItem&) = delete;
                TDrvItem& operator=(TDrvItem&&) = delete;


                // -----------------------------------------------------------
                //  Public, non-virtual methods
                // -----------------------------------------------------------
                tCIDLib::TBoolean bOnline() const;

                tCIDLib::TCard4 c4DriverId() const;

                tCIDLib::TCard4 c4FieldListId() const;

                tCQCKit::EDrvStates eState() const;

                TFldItem* pfldiFind
                (
                    const   tCIDLib::TCard4     c4FldId
                );

                const TFldItem* pfldiFind
                (
                    const   tCIDLib::TCard4     c4FldId
                )   const;

                TFldItem* pfldiFind
                (
                    const   TString&            strFldName
                );

                tCIDLib::TVoid MarkOffline();

                tCIDLib::TVoid MarkOnline();

                tCIDLib::TVoid Reload
                (
                            TCQCSrvAdminClientProxy& orbcAdmin
                    , const tCIDLib::TCard4     c4ServerId
                    , const tCIDLib::TCard4     c4DriverListId
                );

                const TString& strMake() const;

                const TString& strModel() const;

                const TString& strMoniker() const;

                tCIDLib::TVoid SetState
                (
                    const   tCQCKit::EDrvStates eState
                );

                tCIDLib::TVoid UpdateServerId
                (
                    const   tCIDLib::TCard4     c4NewSrvId
                );


            private :
                // -----------------------------------------------------------
                //  Private data members
                //
                //  m_c4FieldListId
                //      The latest field list id that we got from the server.
                //      This will be loaded into the I/O packet, and the server
                //      will use it to know if we've gotten out of sync on the
                //      field definitions.
                //
                //  m_c4DriverId
                //      This is the id for this driver that the server gave to
                //      us the last time we got the driver info. This will be
                //      loaded into the field I/O packet, and the server will
                //      use it to know if we've gotten out of sync.
                //
                //  m_colById
                //      Field I/O is in terms of ids, so we have a separate
                //      non-adopting view of the fields, for fast id to field
                //      item mapping.
                //
                //  m_colFields
                //      The list of fields avalable from this driver.
                //
                //  m_enctLastCheck
                //      When a GUI widget is linked to a field, and its not
                //      being found, it will check on its periodic poll if
                //      it can relink to this field. This will cause the
                //      engine to check and see if any changes have shown up
                //      for that driver's field list. In order to keep this
                //      from happening constantly, we throttle it using a
                //      time stamp.
                //
                //  m_eState
                //      The last driver state we got
                //
                //  m_strMake
                //  m_strModel
                //      The make and model of the driver we are associated with.
                //
                //  m_strMoniker
                //      The moniker for this driver. It's used for error
                //      reporting, to re-query the server if we get out of
                //      sync, and to allow the searches by name.
                // -----------------------------------------------------------
                tCIDLib::TCard4         m_c4FieldListId;
                tCIDLib::TCard4         m_c4DriverId;
                TFldIdList              m_colById;
                TFldList                m_colFields;
                tCIDLib::TEncodedTime   m_enctLastCheck;
                tCQCKit::EDrvStates     m_eState;
                TString                 m_strMake;
                TString                 m_strModel;
                TString                 m_strMoniker;
        };
        using TDriverList = TRefKeyedHashSet<TDrvItem,TString,TStringKeyOps>;
        using TDriverIdList = TRefVector<TDrvItem>;


        // -------------------------------------------------------------------
        //   CLASS: TSrvItem
        //  PREFIX: srvi
        //
        // -------------------------------------------------------------------
        class TSrvItem
        {
            public :
                // -----------------------------------------------------------
                //  Public class types
                // -----------------------------------------------------------
                using TNewFldList = TVector<TKeyValuePair>;


                // -----------------------------------------------------------
                //  Public, static methods
                // -----------------------------------------------------------
                static const TString& strKey
                (
                    const   TSrvItem&           srviSrc
                );


                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TSrvItem
                (
                            TCQCPollEngine* const   polleOwner
                    , const TString&                strEngInstName
                    , const TString&                strNodeName
                    , const tCIDLib::TEncodedTime   enctDropInterval
                );

                TSrvItem(const TSrvItem&) = delete;
                TSrvItem(TSrvItem&&) = delete;

                ~TSrvItem();


                // -----------------------------------------------------------
                //  Public operators
                // -----------------------------------------------------------
                TSrvItem& operator=(const TSrvItem&) = delete;
                TSrvItem& operator=(TSrvItem&&) = delete;


                // -----------------------------------------------------------
                //  Public, non-virual methods
                // -----------------------------------------------------------
                tCIDLib::TVoid AddNewField
                (
                    const   TString&                strMoniker
                    , const TString&                strField
                );

                tCIDLib::TBoolean bHasDriver
                (
                    const   TString&                strMoniker
                )   const;

                tCIDLib::TBoolean bStateChangeThreshold
                (
                    const   tCIDLib::TEncodedTime enctNow
                )   const;

                tCIDLib::TCard4 c4DriverListId() const;

                tCIDLib::TCard4 c4ServerId() const;

                tCIDLib::TCard4 c4ServerId
                (
                    const   tCIDLib::TCard4     c4ToSet
                );

                tCIDLib::EExitCodes ePollThread
                (
                            TThread&            thrThis
                    ,       tCIDLib::TVoid*     pData
                );

                ESrvStates eState() const;

                ESrvStates eState
                (
                    const   ESrvStates          eToSet
                );

                tCIDLib::TVoid Lock();

                TMutex& mtxSync();

                tCIDLib::TVoid MakeNewProxy();

                TFldItem* pfldiFind
                (
                            TDrvItem&           drviOwner
                    , const TString&            strField
                    , const tCIDLib::TBoolean   bForceOnPoll
                );

                TFldItem* pfldiFind
                (
                            TDrvItem&           drviOwner
                    , const tCIDLib::TCard4     c4FieldId
                    , const tCIDLib::TBoolean   bForceOnPoll
                );

                const TFldItem* pfldiFind
                (
                    const   TDrvItem&           drviOwner
                    , const tCIDLib::TCard4     c4FieldId
                )   const;

                TDrvItem* pdrviFind
                (
                    const   tCIDLib::TCard4     c4DriverId
                );

                const TDrvItem* pdrviFind
                (
                    const   tCIDLib::TCard4     c4DriverId
                )   const;

                TDrvItem* pdrviFind
                (
                    const   TString&            strMoniker
                );

                tCIDLib::TVoid Poll();

                tCIDLib::TVoid QueryActiveFlds
                (
                            tCIDLib::TStrCollect& colToFill
                    , const tCIDLib::TBoolean   bAppend
                );

                tCIDLib::TVoid Reload();

                tCIDLib::TVoid Reset();

                const TString& strNodeName() const;

                tCIDLib::TVoid StartShutdown();

                tCIDLib::TVoid Unlock();

                tCIDLib::TVoid UpdateServerId
                (
                    const   tCIDLib::TCard4     c4NewSrvId
                );

                tCIDLib::TVoid WaitShutdown();


            private :
                // -----------------------------------------------------------
                //  Private, non-virtual methods
                // -----------------------------------------------------------
                tCIDLib::TVoid LoadNewFields();


                // -----------------------------------------------------------
                //  Private data members
                //
                //  m_c4DriverListId
                //      This is the id for the server's driver list, which we
                //      got the last time we updated from the server. This
                //      goes into the I/O packet, and the server will use it
                //      to know if we've gotten out of sync wrt drivers.
                //
                //  m_c4ServerId
                //      This one is set by the engine, to let outsiders know
                //      if a server was removed and re-added later. This one
                //      does not come from CQCServer, but is just set by us
                //      here by assign the next value of a running counter to
                //      each new server added.
                //
                //  m_colById
                //      Poll info comes back in terms of driver ids, not names,
                //      so we keep an alernative view (non-adopting) in id
                //      order so that we can do quick lookup by id.
                //
                //  m_colDrivers
                //      The list of drivers available on this server.
                //
                //  m_colNewFlds
                //      The clients coming in need to ask us to add new
                //      fields to the list of threads we are polling, but
                //      cannot just stick stuff directly into m_fiopPoll
                //      because it will be in use when we are actively
                //      doing a poll. So, after looking up the field to
                //      make sure it's known, it calls our AddNewField
                //      method to add the field onto a list that we can
                //      grab them off of when we get back from polling and
                //      add them to the list. It's a moniker/field name
                //      pair.
                //
                //  m_colTmpPoll
                //      Used in the polling method, to stream in string list
                //      fields that, to avoid having to create and destroy
                //      one every time through. Only used internally by
                //      our poll thread.
                //
                //  m_enctDropInterval
                //      We are given an interval that we will use to drop
                //      fields after they have not been accessed for that
                //      length of time. Only used internally by the poll
                //      thread.
                //
                //  m_enctLastConn
                //      When a server goes offline, the engine keeps trying
                //      to connect to it. But, we don't want to do this on
                //      the same schedule that we do polling, which is done
                //      quite quickly. So we limit how quickly we try to
                //      reconnect and this stamp is used to do that. Only
                //      used internally by the poll thread.
                //
                //  m_enctLastPrune
                //      The last time that we checked the I/O poll list for
                //      fields that haven't been accessed recently. Only
                //      used internally by the poll thread.
                //
                //  m_enctLastStateChange
                //      If the server stays in any non-ready state for more
                //      than a given period of time, it is removed on the
                //      assumption that it's bad. It can be forced by in by
                //      client code, but until then we'll dump it. So we
                //      bump this each time the state is changed. Only used
                //      internally by the poll thread.
                //
                //  m_eState
                //      The current state of this server, which tells the
                //      polling thread what it should do with us. NEVER
                //      set this directly. Always call eState() to set it
                //      since we have to update other things when the state
                //      changes.
                //
                //  m_fcolTmp
                //      A temporary fundamental vector of ids that we use for
                //      some queries to the server. Only used internally by
                //      the poll thread.
                //
                //  m_fiopPoll
                //      The I/O polling list that our polling thread uses
                //      to do the field polling. This cannot ever be accessed
                //      by incoming calls directly. Only our own poll thread
                //      accesses this.
                //
                //  m_mbufPoll
                //      We need a memory buffer for polling. The data is
                //      returned in a flattened format, and then we use the
                //      m_fiorPoll results object to parse the data out.
                //      Used only by our poll thraed.
                //
                //  m_mbufTmpPoll
                //      We need a temp buffer when reading in a memory buffer
                //      field value. Used only by our poll thread.
                //
                //  m_mtxSync
                //      Any time that the server object is actively
                //      accessing the driver list, it needs to lock
                //      this mutex, to prevent the outside world from
                //      trying to read field data while the list is
                //      changing. Otherwise, while the server object is
                //      off accessing field data from the server, the
                //      outside world can get to the data without being
                //      blocked.
                //
                //  m_porbcAdmin
                //      The client proxy for this server's admin interface,
                //      which lets us get field info and do bulk polling. It
                //      is a pointer since we destroy it any time we lose
                //      connection to the server and need to start trying
                //      to reconnect. Only used by the poll thread.
                //
                //  m_polleOwner
                //      We need to keep a pointer to our owning poll engine,
                //      since we need to get a new server id every time we
                //      lose the connection and reconnect because the drivers
                //      or fields it has might have changed while we were
                //      out of touch, and this will invalidate any links we
                //      have given out for them.
                //
                //  m_strBinding
                //      This is the name server binding of our host. It's
                //      the node name below formatted into a full name
                //      server binding for our target CQCServer. We pre-do
                //      this for cache refreshing purposes, so we don't have
                //      to rebuild it every time.
                //
                //  m_strNodeName
                //      This is the name of the target host, which is used
                //      for lookups by name, error reporting, and to allow
                //      us to re-query our info when we get out of sync. It
                //      never changes once the server object is created, so
                //      no sync is required for it to be used by both our
                //      polling thread and incoming client calls.
                //
                //  m_strTmpPoll
                //      Used in the polling method, to stream in fields that
                //      have string representations, to avoid having to create
                //      and destroy one every time through. Only used by the
                //      polling thread.
                //
                //  m_thrPoll
                //      Our poll thread that keeps us trying to keep in touch
                //      with and polling our server. It's pointed to the
                //      ePollThread method.
                // -----------------------------------------------------------
                tCIDLib::TCard4             m_c4DriverListId;
                tCIDLib::TCard4             m_c4ServerId;
                TDriverIdList               m_colById;
                TDriverList                 m_colDrivers;
                TNewFldList                 m_colNewFlds;
                tCIDLib::TStrList           m_colTmpPoll;
                tCIDLib::TEncodedTime       m_enctDropInterval;
                tCIDLib::TEncodedTime       m_enctLastConn;
                tCIDLib::TEncodedTime       m_enctLastPrune;
                tCIDLib::TEncodedTime       m_enctLastStateChange;
                ESrvStates                  m_eState;
                TFundVector<tCIDLib::TCard8> m_fcolTmp;
                TFldIOPacket                m_fiopPoll;
                THeapBuf                    m_mbufPoll;
                THeapBuf                    m_mbufTmpPoll;
                TMutex                      m_mtxSync;
                TCQCPollEngine*             m_polleOwner;
                TCQCSrvAdminClientProxy*    m_porbcAdmin;
                TString                     m_strBinding;
                TString                     m_strNodeName;
                TString                     m_strTmpPoll;
                TThread                     m_thrPoll;
        };
        using TSrvList = TRefVector<TSrvItem>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCPollEngine
        (
            const   tCIDLib::TEncodedTime   enctDropInterval = 0
        );

        TCQCPollEngine(const TCQCPollEngine&) = delete;
        TCQCPollEngine(TCQCPollEngine&&) = delete;

        ~TCQCPollEngine();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCPollEngine& operator=(const TCQCPollEngine&) = delete;
        TCQCPollEngine& operator=(TCQCPollEngine&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckDrvChange
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4DrvListId
        );

        tCIDLib::TBoolean bCheckDrvState
        (
            const   TString&                strMoniker
            ,       tCQCKit::EDrvStates&    eState
        );

        tCIDLib::TBoolean bCheckLinkChange
        (
            const   TCQCFldPollInfo&        cfpiToCheck
        );

        tCIDLib::TBoolean bGetSrvProxy
        (
            const   TString&                strMoniker
            ,       tCQCKit::TCQCSrvProxy&  orbcToFill
        );

        tCIDLib::TBoolean bHostForMoniker
        (
            const   TString&                strMoniker
            ,       TString&                strHost
            , const tCIDLib::TBoolean       bCreateIfNew
            ,       tCIDLib::TBoolean&      bCreated
        );

        tCIDLib::TBoolean bQueryFldInfo
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       TCQCFldDef&             flddToFill
        );

        tCIDLib::TBoolean bReadValue
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bReadValue
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       TString&                strToFill
            ,       tCQCKit::EFldTypes&     eType
        );

        tCIDLib::TBoolean bReadValue
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       tCIDLib::TCard4&        c4DataBytes
            ,       TMemBuf&                mbufValue
            ,       tCQCKit::EFldTypes&     eType
        );

        tCQCKit::EValQRes eQueryValue
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       TCQCFldValue&           fvToFill
            ,       TCQCPollEngFldLink&     flnkInfo
        );

        tCQCKit::EValQRes eQueryValue
        (
            const   TString&                strToRead
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       TCQCFldValue&           fvToFill
            ,       TCQCPollEngFldLink&     flnkInfo
        );

        tCQCPollEng::EFldRegRes eRegisterField
        (
            const   TString&                strField
            , const tCQCKit::EReqAccess     eReqAccess
            ,       TCQCFldDef&             flddInfo
            ,       TCQCPollEngFldLink&     flnkInfo
            ,       TString&                strMake
            ,       TString&                strModel
        );

        tCQCPollEng::EFldRegRes eRegisterField
        (
            const   TString&                strMoniker
            , const TString&                strField
            , const tCQCKit::EReqAccess     eReqAccess
            ,       TCQCFldDef&             flddInfo
            ,       TCQCPollEngFldLink&     flnkInfo
            ,       TString&                strMake
            ,       TString&                strModel
        );

        tCIDLib::TVoid QueryFlds
        (
                    tCIDLib::TStrList&      colFlds
        );

        const TCQCSecToken& sectUser() const
        {
            return m_sectUser;
        }

        tCIDLib::TVoid StartEngine
        (
            const   TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid StopEngine();

        tCIDLib::TVoid WriteField
        (
            const   TString&                strFullFldName
            , const TString&                strValue
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteField
        (
            const   TString&                strMoniker
            , const TString&                strField
            , const TString&                strValue
            , const TCQCSecToken&           sectUser
        );


    protected :
        // -------------------------------------------------------------------
        //  The janitor (used internally) has to be our friend and the server
        //  item class.
        // -------------------------------------------------------------------
        friend class TPESrvJanitor;
        friend class TSrvItem;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4NextServerId();

        tCIDLib::EExitCodes ePruneThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        TSrvItem* psrviFindSrvByMoniker
        (
            const   TString&                strMoniker
            , const tCIDLib::TBoolean       bCreateIfNew
            ,       tCIDLib::TBoolean&      bCreated
        );

        TSrvItem* psrviFindSrvByHost
        (
            const   TString&                strHost
        );

        [[nodiscard]] TSrvItem* psrviLockById
        (
            const   tCIDLib::TCard4         c4ToFind
        );

        TSrvItem* psrviSearchForMoniker
        (
            const   TString&                strMoniker
        );

        tCIDLib::TVoid PruneServers();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colServers
        //      The list of servers that we are talking to. We just have a
        //      simple vector of them. This is protected by the m_mtxSync
        //      mutex, which must be locked while accessing this list.
        //
        //  m_enctFldDropInterval
        //      The time that we wait before dropping a field after it's not
        //      been accessed for that period. It's set to a default value
        //      but can be changed by client code. We set this on the server
        //      items when we create them, since they are the ones that need
        //      it.
        //
        //  m_enctLastNSCheck
        //      We throttle calls to find the server associated with a driver,
        //      because if a number of widgets reference a driver that's not
        //      loaded, we'd just beat the name server to death.
        //
        //  m_mtxSync
        //      A mutex to synchronize incoming calls to the engine by
        //      clients. So only one can be accessing the engine at at time.
        //      It effectively is protecting the m_colServers list. Any
        //      time a client accesses any server items (indirectly through
        //      our public methods), this is locked. And the pruning thread
        //      also locks it while it's looking for servers to prune.
        //
        //  m_sectUser
        //      We need a security token to do some things we do, so the calling
        //      code must provide us with one. It's read only once set so we don't
        //      have to sync access to it.
        //
        //  m_scntNextSrvId
        //      This counter is used to hand out unique ids for the server
        //      objects that are added to our list. Server items call to us
        //      to get a new id whever they need one.
        //
        //  m_thrPrune
        //      This is a thread started on the ePruneThread() method. This
        //      guy just wakes up periodically and looks for any servers that
        //      have not had any fields accessed with the drop interval. If
        //      it finds any, it removes them from the list.
        // -------------------------------------------------------------------
        TSrvList                m_colServers;
        tCIDLib::TEncodedTime   m_enctFldDropInterval;
        tCIDLib::TEncodedTime   m_enctLastNSCheck;
        TMutex                  m_mtxSync;
        TSafeCard4Counter       m_scntNextSrvId;
        TCQCSecToken            m_sectUser;
        TThread                 m_thrPrune;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCPollEngine,TObject)
        NulObject(TCQCPollEngine)
};

#pragma CIDLIB_POPPACK


