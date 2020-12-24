//
// FILE NAME: CQLogicSh_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/07/2011
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
//  This file defines facility class for this facility.
//
//  One service we provide is for the polling of graph data from the LS'
//  graph related 'fields'. They aren't really fields, and therefore the
//  usual polling engine cannot be used to poll them. So we provide a simple
//  version of that for graph data here. We spin off a background thread that
//  periodically polls any active graphs. Any graphs that haven't been
//  accessed in the last five minutes are dropped.
//
//  This makes life vastly easier on consumers (displayers) of this graph
//  data, and also prevents a lot of extra traffic back to the logic server
//  since we cache the data.
//
// CAVEATS/GOTCHAS:
//
//  1)  We can't block the cache while we go to the logic server. So the
//      poller thread will see if an item needs updating. If so, it will
//      release the mutex and do the query. Then it will re-lock and store
//      the new data.
//
//      Since the outside world can only cause new items to be added (by
//      accessing them), the poller thread doesn't have to worry that anything
//      is going to change while it's gone, at least nothing that would
//      invalidate the reference to the item it is currently polling.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQLogicSh
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQLOGICSHEXPORT TFacCQLogicSh : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQLogicSh();

        TFacCQLogicSh(const TFacCQLogicSh&) = delete;
        TFacCQLogicSh(TFacCQLogicSh&&) = delete;

        ~TFacCQLogicSh();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQLogicSh& operator=(const TFacCQLogicSh&) = delete;
        TFacCQLogicSh& operator=(TFacCQLogicSh&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQLogicSh::EGraphQRes eQueryGraphSamples
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4CfgSerialNum
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCQLogicSh::TSampleList& fcolToFill
            ,       tCIDLib::TCard4&        c4NewSamples
        );

        tCQLogicSh::TLogicSrvProxy orbcLogicSrvProxy
        (
            const   tCIDLib::TCard4         c4WaitFor = 0
        )   const;

        tCIDLib::TVoid StopPoller();


    private :
        // -------------------------------------------------------------------
        //  We need something to hold our per-graph data in the cache. We have
        //  to keep up with the graph sample points, the name of the field, and
        //  the serial number. We also need a last accessed time stamp so that
        //  we can drop old graph items out of the cache. And we need one to
        //  keep up with the next time we need to poll it.
        //
        //  Note that we cannot keep just one copy of the overall configuration
        //  serial number. Each item must be seen to have gotten out of sync
        //  if it has. If we only kept a single copy as a member, then as
        //  soon as one out of date was polled, the others wouldn't know that
        //  they are.
        // -------------------------------------------------------------------
        class TCacheItem
        {
            public :
                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TCacheItem() = delete;

                TCacheItem
                (
                    const   TString&                strName
                );

                TCacheItem(const TCacheItem&) = default;
                TCacheItem(TCacheItem&&) = default;

                ~TCacheItem();


                // -----------------------------------------------------------
                //  Public operators
                // -----------------------------------------------------------
                TCacheItem& operator=(const TCacheItem&) = delete;
                TCacheItem& operator=(TCacheItem&&) = delete;


                // -----------------------------------------------------------
                //  Public, non-virtual methods
                // -----------------------------------------------------------
                tCIDLib::TBoolean bQueryNewSamples
                (
                            tCIDLib::TCard4&        c4SerialNum
                    ,       tCQLogicSh::TSampleList& fcolToFill
                    ,       tCIDLib::TCard4&        c4ToGet
                );

                tCIDLib::TVoid Reset();

                tCIDLib::TVoid StoreNewSamples
                (
                    const   tCQLogicSh::TSampleList& fcolNew
                    , const tCIDLib::TCard4         c4NewCnt
                    , const tCIDLib::TCard4         c4SerialNum
                );


                // -----------------------------------------------------------
                //  Public data members
                // -----------------------------------------------------------
                tCIDLib::TCard4         m_c4GraphCfgSerNum;
                tCIDLib::TEncodedTime   m_enctLastAccess;
                tCIDLib::TEncodedTime   m_enctNextPoll;
                TGraphData              m_grdatSamples;
                TString                 m_strName;
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes ePoller
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid ResetCacheItems();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bStatus
        //      We keep this set to indicate whether our polling thread is
        //      currently up and working correctly or not. This let's the
        //      incoming clients quickly check and, if not in good shape, to
        //      just reset the caller's graph data to make him clear his
        //      graph.
        //
        //  m_colCache
        //      The cache of graph items.
        //
        //  m_mtxSync
        //      We need a mutex to sync access to the cache data between
        //      outsiders calling in to get graph data, and the poller thread
        //      doing the polling and updating the cache.
        //
        //  m_pthrPoller
        //      This is a thread that is used to manage our graph data
        //      cache and the polling of graph data. It's a pointer and we
        //      fault it in as required. It's being non-zero indicates that
        //      have started it already.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bStatus;
        TVector<TCacheItem> m_colCache;
        TMutex              m_mtxSync;
        TThread*            m_pthrPoller;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQLogicSh,TFacility)

};

#pragma CIDLIB_POPPACK


