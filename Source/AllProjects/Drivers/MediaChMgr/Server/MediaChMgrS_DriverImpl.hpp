//
// FILE NAME: MediaChMgrS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/02/2006
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
//  This is the header file for the actual driver class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TMediaChMgrSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TMediaChMgrSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMediaChMgrSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TMediaChMgrSDriver();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryData
        (
            const   TString&                strQType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        );

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TMediaChMgrSDriver();
        TMediaChMgrSDriver(const TMediaChMgrSDriver&);
        tCIDLib::TVoid operator=(const TMediaChMgrSDriver&);


        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        enum EMapFlds
        {
            EMapFld_CurrentDisc
            , EMapFld_CurrentTitle
            , EMapFld_CurrentTrack
            , EMapFld_PlayMode
            , EMapFld_Power

            , EMapFlds_Count
            , EMapFlds_Min = 0
            , EMapFlds_Max = EMapFlds_Count - 1
        };
        struct TMappedFld
        {
            TMappedFld() :

                c4OurId(0)
                , c4HisId(0)
            {}

            TMappedFld(const tCIDLib::TCard4 c4Init) :

                c4OurId(c4Init)
                , c4HisId(c4Init)
            {}

            tCIDLib::TCard4     c4OurId;
            tCIDLib::TCard4     c4HisId;
        };


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoTransportOp
        (
            const   TString&                strOp
        );

        tCIDLib::TBoolean bGetLocData
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strColCookie
            ,       TString&                strMon
            ,       tCIDLib::TCard4&        c4Slot
        );

        tCIDLib::TBoolean bLoadPollList();

        tCIDLib::TBoolean bPollChanger();

        tCIDLib::TBoolean bSelectSlot
        (
            const   tCIDLib::TCard4         c4Slot
        );

        tCIDLib::TBoolean bSetChangerPower
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bWaitChangerReady();

        tCIDLib::TCard4 c4MapFldId
        (
            const   tCIDLib::TCard4         c4SrcFldId
        );

        tCIDLib::TVoid LoadMediaData
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strTitleCookie
            , const TString&                strColCookie
        );

        tCIDLib::TVoid RegisterFields();

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid SetChanger
        (
            const   TString&                strNewChanger
            ,       tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TCQCDriverObjCfg&       cqcdcInfo
        );

        tCIDLib::TVoid SetDefFldVals();

        tCIDLib::TVoid SetErrState();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_aFldIdMap
        //      We need to map between our field ids and the ids of the fields
        //      we read from the active changer. So we use this simple map.
        //      They are in the order of the EMapFlds enum above.
        //
        //  m_c4FldXXX
        //      The ids for the fields that we create, and then look up the
        //      ids for so that we can do by id access.
        //
        //      The active changer field holds the moniker of the last changer
        //      we were told to start playing a title on (and which we were
        //      able to successfully start.) Any subsequent player oriented
        //      commands we get will be forwarded to it. If it is empty, then
        //      we've not started one yet, or the last attempt to contact it
        //      failed, in which case we reject any commands that would be
        //      forwarded to it.
        //
        //  m_c4LastTrackNum
        //      The last track number we saw. If the track number changes
        //      during polling, we'll update our item level metadata and then
        //      store the new track here. If it goes zero, we'll clear out
        //      the item level fields.
        //
        //  m_colItemArtists
        //  m_colItemCookies
        //  m_colImteNames
        //      When we start playing a new collection, we query the item
        //      detils and store the info here. When we see a change in the
        //      current track number from the changer, we'll use that as
        //      index into here (minus one) to set the item level fields.
        //
        //  m_fiopPoll
        //      A field I/O packet that we set up to poll the fields of the
        //      active changer that we echo through our fields.
        //
        //  m_mbufPoll
        //      A buffer we use for polling the active changer's fields. The
        //      read method that is used with the I/O packet returns the data
        //      flattened into a buffer.
        //
        //  m_orbcDrv
        //      If we have an active changer, this is the client proxy for
        //      the CQCServer that hosts the changer driver.
        //
        //  m_strChanger
        //      The moniker of the active changer. This is in one of our
        //      fields, but this just avoids the extra work of getting it
        //      out of the field every time we need it.
        //
        //  m_strRepoMoniker
        //      We get a driver prompt value that indicates the moniker of the
        //      media repository driver we are associated with. This is where
        //      we go to get the media data when we are told to start playing
        //      a new title.
        // -------------------------------------------------------------------
        TMappedFld              m_aFldIdMap[EMapFlds_Count];
        tCIDLib::TCard4         m_c4FldId_ActiveChanger;
        tCIDLib::TCard4         m_c4FldId_CurAspectRatio;
        tCIDLib::TCard4         m_c4FldId_CurCast;
        tCIDLib::TCard4         m_c4FldId_CurColArtist;
        tCIDLib::TCard4         m_c4FldId_CurColCookie;
        tCIDLib::TCard4         m_c4FldId_CurColName;
        tCIDLib::TCard4         m_c4FldId_CurDescr;
        tCIDLib::TCard4         m_c4FldId_CurItemArtist;
        tCIDLib::TCard4         m_c4FldId_CurItemCookie;
        tCIDLib::TCard4         m_c4FldId_CurItemName;
        tCIDLib::TCard4         m_c4FldId_CurLabel;
        tCIDLib::TCard4         m_c4FldId_CurLeadActor;
        tCIDLib::TCard4         m_c4FldId_CurRating;
        tCIDLib::TCard4         m_c4FldId_CurTitleCookie;
        tCIDLib::TCard4         m_c4FldId_CurTitleName;
        tCIDLib::TCard4         m_c4FldId_CurYear;
        tCIDLib::TCard4         m_c4FldId_PlayMedia;
        tCIDLib::TCard4         m_c4FldId_Transport;
        tCIDLib::TCard4         m_c4LastDiscNum;
        tCIDLib::TCard4         m_c4LastTrackNum;
        tCQCMedia::TNameList    m_colItemArtists;
        tCQCMedia::TNameList    m_colItemCookies;
        tCQCMedia::TNameList    m_colItemNames;
        TFldIOPacket            m_fiopPoll;
        THeapBuf                m_mbufPoll;
        tCQCKit::TCQCSrvProxy   m_orbcDrv;
        TString                 m_strChanger;
        TString                 m_strRepoMoniker;



        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMediaChMgrSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK

