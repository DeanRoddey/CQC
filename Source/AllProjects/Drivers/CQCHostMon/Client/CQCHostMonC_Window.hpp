//
// FILE NAME: CQCHostMonC_Window_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/05/2000
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
//  This file implements the main window for the host monitor interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: THostMonCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class THostMonCWnd : public TCQCDriverClient
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TVector<TCQCVolPollInfo>    TVolInfo;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THostMonCWnd() = delete;

        THostMonCWnd(const THostMonCWnd&) = delete;

        THostMonCWnd
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TCQCUserCtx&            cuctxToUse
        );

        ~THostMonCWnd();


        // -------------------------------------------------------------------
        //  Unimplemented ctors and operators
        // -------------------------------------------------------------------
        THostMonCWnd& operator=(const THostMonCWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bChanges() const override;

        tCIDLib::TBoolean bSaveChanges
        (
                    TString&                strErrMsg
        )   override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        //
        //  These methods are called by our parent class and implementing a
        //  driver is basically responding to these calls.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TBoolean bDoPoll
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   override;

        tCIDLib::TBoolean bGetConnectData
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        )   override;

        tCIDLib::TVoid Connected() override;

        tCIDLib::TVoid DoCleanup() override;

        tCIDLib::TVoid LostConnection() override;

        tCIDLib::TVoid UpdateDisplayData() override;


    private :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TFundVector<tCIDLib::TCard4>    TPollVector;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bExtractResults
        (
            const   tCIDLib::TBoolean       bInit
        );

        const TCQCFldDef* pflddFind
        (
            const   TString&                strFind
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        )   const;

        tCIDLib::TVoid InitVolDisplay();

        tCIDLib::TVoid SetIOList
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        );

        tCIDLib::TVoid UpdateDisplay
        (
            const   tCIDLib::TBoolean       bForce
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4DriverId
        //  m_c4DriverListId
        //  m_c4FieldListId
        //      These are where we store the current ids for our driver and
        //      and field list and overall driver list. We have to provide
        //      these back to the server on each poll, so he can insure that
        //      we are in sync still.
        //
        //  m_c4LeftVol
        //      We have a set of display slots for volumes. If there are more
        //      than that, we use a scroll bar to scroll between them. This
        //      is the one that is currently on the left (i.e. the first one
        //      visible.)
        //
        //  m_c4ReadBytes
        //      Set to the bytes returned from each field poll call. This
        //      tells us how many bytes we have to stream from.
        //
        //  m_colFlds
        //      A vector of field def objects that we query from our server
        //      when we connect. This describes our 'devices' fields, provides
        //      their ids, and gives us debug info on the field if we need it.
        //      It is updated each time we reconnect or have to refresh our
        //      field list (since we have a variable number of fields due to
        //      volumes being added or removed on the target host.)
        //
        //  m_colVolInfo
        //      This is where we store the volume info. It is kept up to date
        //      by the poll callback. If the volumes change, then we will set
        //      the m_bVolChanges flag so that the GUI update callback will
        //      know to refresh itself.
        //
        //  m_fcolFldIds
        //  m_fcolSerialNums
        //      The field ids and serial numbers for the fields that we are
        //      going to poll. This format is required for the simple, single
        //      driver polling method we use.
        //
        //  m_mbufPollData
        //      We need a memory buffer for our poll call, which returns
        //      flattened data in a memory buffer. We use the m_fiorPoll
        //      results object to parse the data out.
        //
        //  m_pwndXXX
        //      For convenience, we get some typed pointers to some of our
        //      child widgets. We don't own them, we just look at them.
        //
        //
        //  These are the poll data members
        //
        //  m_c4CPUCount
        //      This is one of the 'on connect' fields that is loaded up upon
        //      connection or reconnection. It holds the number of CPUs in
        //      the target host.
        //
        //  m_c4PerMemUsedK
        //  m_c4PerMemUsedK2
        //      The percent of memory used on the target host. The poll
        //      callback updates the 2 version, and the update callback uses
        //      that to know if it has a new value to display.
        //
        //  m_c4PhysicalMemK
        //      This is one of the 'on connect' fields that is loaded up upon
        //      connection or reconnection. It holds the amount of physical
        //      memory in the host machine, in 1K chunks.
        //
        //  m_strHostName
        //  m_strOSInfo
        //      These are strings that are filled with 'on connect' type
        //      info by the GetConnectData() callback. It will be used by
        //      the Connected() method to load up widgets that display this
        //      information.
        //
        //  m_strmIn
        //      A stream we wrap around the m_mbufPollData so that we can
        //      read in polled data.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bListChanged;
        tCIDLib::TCard4     m_c4DriverId;
        tCIDLib::TCard4     m_c4DriverListId;
        tCIDLib::TCard4     m_c4FieldListId;
        tCIDLib::TCard4     m_c4LeftVol;
        tCIDLib::TCard4     m_c4ReadBytes;
        TVector<TCQCFldDef> m_colFlds;
        TPollVector         m_fcolFldIds;
        TPollVector         m_fcolSerialNums;
        THeapBuf            m_mbufPollData;
        TProgressBar*       m_pwndMemBar;
        TBinMBufInStream    m_strmIn;

        // These are for holding connect data
        tCIDLib::TCard4     m_c4CPUCount;
        tCIDLib::TCard4     m_c4PhysicalMemK;
        TString             m_strCPUType;
        TString             m_strHostName;
        TString             m_strOSInfo;

        // These are for holding poll data
        tCIDLib::TCard4     m_c4PerMemUsedK;
        tCIDLib::TCard4     m_c4PerMemUsedK2;
        TVolInfo            m_colVolInfo;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THostMonCWnd,TCQCDriverClient)
};

#pragma CIDLIB_POPPACK


