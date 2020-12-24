//
// FILE NAME: CQCGKit_DrvPoller.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/07/2003
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
//  This is the header for the CQCGKit_DrvPoller.cpp file, which implements a helper
//  class that simplifies a lot of the grunt work that client side drivers have to
//  do to poll their server side drivers. Client side drivers don't use the field I/O
//  mechanism, since that is multi-driver oriented, and far more complex than they
//  need. They only need to poll a single driver. Some other things will use this as
//  well.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCDrvPoller
//  PREFIX: dplr
// ---------------------------------------------------------------------------
class CQCGKITEXPORT TCQCDrvPoller : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TCQCDrvPoller() = delete;

        TCQCDrvPoller
        (
            const   TString&                strMoniker
        );

        TCQCDrvPoller(const TCQCDrvPoller&) = delete;
        TCQCDrvPoller(TCQCDrvPoller&&) = delete;

        ~TCQCDrvPoller();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDrvPoller& operator=(const TCQCDrvPoller&) = delete;
        TCQCDrvPoller& operator=(TCQCDrvPoller&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFldChanged
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        tCIDLib::TBoolean bInError
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        tCIDLib::TBoolean bPoll
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        );

        tCIDLib::TCard4 c4IndexFromFldId
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        const TCQCFldStoreBool& cfsBool
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        const TCQCFldStoreCard& cfsCard
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        const TCQCFldStoreFloat& cfsFloat
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        const TCQCFldStoreInt& cfsInt
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        const TCQCFldStoreString& cfsString
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        const TCQCFldStoreSList& cfsSList
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        const TCQCFldStoreTime& cfsTime
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        tCIDLib::TVoid ClearFldChange
        (
            const   tCIDLib::TCard4         c4ToClear
        );

        const TCQCFldDef& flddAt
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        tCIDLib::TVoid Initialize
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
            , const tCIDLib::TStrCollect&   colFldNames
        );

        tCIDLib::TVoid WriteBool
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
            , const tCIDLib::TCard4         c4Index
            , const tCIDLib::TBoolean       bToWrite
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteCard
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
            , const tCIDLib::TCard4         c4Index
            , const tCIDLib::TCard4         c4ToWrite
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteFloat
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
            , const tCIDLib::TCard4         c4Index
            , const tCIDLib::TFloat8        f8ToWrite
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteInt
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
            , const tCIDLib::TCard4         c4Index
            , const tCIDLib::TInt4          i4ToWrite
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteString
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
            , const tCIDLib::TCard4         c4Index
            , const TString&                strToWrite
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteSList
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
            , const tCIDLib::TCard4         c4Index
            , const tCIDLib::TStrList&      colToWrite
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid WriteTime
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
            , const tCIDLib::TCard4         c4Index
            , const tCIDLib::TCard8&        c8ToWrite
            , const TCQCSecToken&           sectUser
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bExtractResults();

        const TCQCFldStore& cfsGetStore
        (
            const   tCIDLib::TCard4         c4Index
            , const tCQCKit::EFldTypes      eType
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TPollVector = tCIDLib::TCardList;
        using TModChangeList = tCIDLib::TBoolList;
        using TFldStores = TRefVector<TCQCFldStore>;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4DriverId
        //  m_c4DriverListId
        //  m_c4FieldListId
        //      These are part of our polling info. We have to get this info from the
        //      server and pass it back on each poll cycle, to prove we are up to date.
        //
        //  m_c4ReadBytes
        //      The number of bytes read in on the last poll.
        //
        //  m_colFldStores
        //      A heterogenous vector of field storage objects, which can hold data
        //      of all of the field types. They will also hold the field def objects
        //      for us.
        //
        //  m_fcolChanges
        //      A set of change flags for each field.
        //
        //  m_fcolFldIds
        //      Storage for the ids of the fields that we poll. This is the form
        //      required to pass into the particular polling method that we use. It's
        //      set up when we connect.
        //
        //  m_fcolSerialNums
        //      A list of field serial numbers, the same size as m_colFldIds, which
        //      holds the current field serial number for each field. The server won't
        //      stream back any data that hasn't changed, and these serial numbers tell
        //      it what field values it has are newer than our values.
        //
        //  m_mbufPollData
        //      A buffer that we provide to poll data into.
        //
        //  m_strMoniker
        //      The moniker of the driver we are reading from.
        //
        //  m_strmRead
        //      A binary input stream created over the m_mbufPollData field, to stream
        //      in data with.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4DriverId;
        tCIDLib::TCard4         m_c4DriverListId;
        tCIDLib::TCard4         m_c4FieldListId;
        tCIDLib::TCard4         m_c4ReadBytes;
        TFldStores              m_colFldStores;
        TModChangeList          m_fcolChanges;
        TPollVector             m_fcolFldIds;
        TPollVector             m_fcolSerialNums;
        THeapBuf                m_mbufPollData;
        TString                 m_strMoniker;
        TBinMBufInStream        m_strmRead;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDrvPoller,TObject)
};


#pragma CIDLIB_POPPACK


