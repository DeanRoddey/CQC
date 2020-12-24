//
// FILE NAME: CQCMedia_StdRendDrv.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/31/2013
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
//  This is the main implementation file of the based class for standard
//  renderer drivers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMedia_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCStdMediaRendDrv,TCQCServerBase)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCMedia_StdRendDrv
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The point at which, in random category mode, we'll start trying to
        //  reload the list (in terms of the list getting down to this few items
        //  or less) and the minimum time interval that'll do a random cat reload
        //  unless forced to.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4MinRandCatItems = 5;

        #if CID_DEBUG_ON
        constexpr tCIDLib::TEncodedTime enctMinRandCatInt = kCIDLib::enctOneMinute;
        #else
        constexpr tCIDLib::TEncodedTime enctMinRandCatInt = kCIDLib::enctOneMinute * 5;
        #endif


        // -----------------------------------------------------------------------
        //  A collection of standard render field names we fault in upon first
        //  access.
        // -----------------------------------------------------------------------
        using TRFldsList    = TObjArray<TString, tCQCMedia::ERendFlds>;
        static TAtomicFlag  atomFldListLoaded;
        static TRFldsList   colRendFlds(tCQCMedia::ERendFlds::Count);
        static TRFldsList   colRendFldsV2(tCQCMedia::ERendFlds::Count);
    }
}



// ---------------------------------------------------------------------------
//  Local helpers
// ---------------------------------------------------------------------------

//
//  Faults in the standard render field name lists. THe caller should check
//  the flag first aad only call us if it's false. We'll lock and then check
//  it again, then fault in if no one else beats us to the punch.
//
static tCIDLib::TVoid LoadRedFldLists()
{
    TBaseLock lockInit;
    if (!CQCMedia_StdRendDrv::atomFldListLoaded)
    {
        tCQCMedia::ERendFlds eCur = tCQCMedia::ERendFlds::Min;
        while (eCur <= tCQCMedia::ERendFlds::Max)
        {
            CQCMedia_StdRendDrv::colRendFlds[eCur] = tCQCMedia::strXlatERendFlds(eCur);
            CQCMedia_StdRendDrv::colRendFldsV2[eCur] = tCQCMedia::strAltXlatERendFlds(eCur);
            eCur++;
        }

        CQCMedia_StdRendDrv::atomFldListLoaded.Set();
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCStdMediaRendDrv
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  CQSLAudPlSDriver: Public, static methods
// ---------------------------------------------------------------------------

//
//  All renderer drivers have to support a common set of fields. To avoid
//  them redundantly having to load these definitions up, and to insure
//  consistency, we provide this helper. There are couple that are media
//  type specific, so they tell us what types they support.
//
//  They also tell us the driver achitcture version, so we can set up the
//  field names appropriately.
//
tCIDLib::TVoid
TCQCStdMediaRendDrv::QueryStdRendFields(        TVector<TCQCFldDef>&    colToFill
                                        , const tCQCMedia::EMTFlags     eTypes
                                        , const tCIDLib::TCard4         c4ArchVer
                                        , const tCIDLib::TCard4         c4Flags)
{
    const tCIDLib::TBoolean bV2(c4ArchVer > 1);
    TString strLims;


    //
    //  These are the media renderer oriented fields
    //

    // The currently set default repo
    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::ActiveRepo, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::MediaRepoDrv
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::ClearPL, bV2)
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
    );

    if (tCIDLib::bAllBitsOn(eTypes, tCQCMedia::EMTFlags::Movie))
    {
        colToFill.objPlace
        (
            TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurAspect, bV2)
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
        );

        colToFill.objPlace
        (
            TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurCast, bV2)
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
        );
    }

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurColArtist, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurColCookie, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::ColCookie
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurColName, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurDescr, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurItName, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurItArtist, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurItCookie, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::ItemCookie
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurItTime, bV2)
        , tCQCKit::EFldTypes::Time
        , (c4Flags & kCQCMedia::c4RendFlag_Seekable)
                    ? tCQCKit::EFldAccess::ReadWrite : tCQCKit::EFldAccess::Read
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurItTotal, bV2)
        , tCQCKit::EFldTypes::Time
        , tCQCKit::EFldAccess::Read
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurLabel, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );

    if (tCIDLib::bAllBitsOn(eTypes, tCQCMedia::EMTFlags::Movie))
    {
        colToFill.objPlace
        (
            TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurLeadActor, bV2)
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
        );
    }

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurRating, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurTiName, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurTiCookie, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::TitleCookie
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurYear, bV2)
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );


    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::DelPLItem, bV2)
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Write
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::EnqMedia, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );

    // The current playback percentage
    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PBPercent, bV2)
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
        , L"Range: 0, 100"
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PlayMedia, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PLIndex, bV2)
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PLItemCnt, bV2)
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PLItemKey, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );

    // Build up the play list mode field and set it up
    strLims = L"Enum: ";
    tCQCMedia::EPLModes ePLMode = tCQCMedia::EPLModes::Min;
    for (; ePLMode <= tCQCMedia::EPLModes::Max; ePLMode++)
    {
        if (ePLMode > tCQCMedia::EPLModes::Min)
            strLims.Append(L", ");

        strLims.Append(tCQCMedia::strXlatEPLModes(ePLMode));
    }
    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PLMode, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::ReadWrite
        , strLims
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PLSerialNum, bV2)
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );


    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::SelPLItem, bV2)
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Write
    );

    // Set random cat
    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::SetRandomCat, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );

    // Set a new active repo
    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::SetRepo, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );

    // A zone number field for user purposes
    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::ZoneNum, bV2)
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::ReadWrite
    );


    //
    //  These are from the Power device class, and are only done if in V2 mode,
    //  since they are purely V2 oriented. So we can use the helper to load these
    //  since it doesn't have to deal with V1/V2 mode.
    //
    if (bV2)
        TCQCDevClass::c4LoadPowerFlds(colToFill, TString::strEmpty());


    //
    //  These are from the Media Transport device class
    //
    strLims = L"Enum: ";
    tCQCMedia::EMediaStates eMedState = tCQCMedia::EMediaStates::Min;
    for (; eMedState <= tCQCMedia::EMediaStates::Max; eMedState++)
    {
        if (eMedState > tCQCMedia::EMediaStates::Min)
            strLims.Append(L", ");
        strLims.Append(tCQCMedia::strXlatEMediaStates(eMedState));
    }
    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::MediaState, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::MediaState
        , strLims
    );


    strLims = L"Enum: ";
    tCQCMedia::ETransCmds eTransCmd = tCQCMedia::ETransCmds::Min;
    for (; eTransCmd <= tCQCMedia::ETransCmds::Max; eTransCmd++)
    {
        if (eTransCmd > tCQCMedia::ETransCmds::Min)
            strLims.Append(L", ");
        strLims.Append(tCQCMedia::strXlatETransCmds(eTransCmd));
    }

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::Transport, bV2)
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , tCQCKit::EFldSTypes::MediaTransport
        , strLims
    );


    //
    //  These are from the Audio device class
    //
    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::AdjustVolume, bV2)
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
        , tCQCKit::EFldSTypes::VolumeAdj
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::Mute, bV2)
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::Mute
    );

    colToFill.objPlace
    (
        TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::Volume, bV2)
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::Volume
        , L"Range: 0, 100"
    );
}



//
//  A helper to get the name of a standard renderer field, in either V1
//  or V2 compatible mode.
//
const TString&
TCQCStdMediaRendDrv::strRendFld(const   tCQCMedia::ERendFlds    eFld
                                , const tCIDLib::TBoolean       bV2)
{
    if (!CQCMedia_StdRendDrv::atomFldListLoaded)
        LoadRedFldLists();

    if (bV2)
        return CQCMedia_StdRendDrv::colRendFldsV2[eFld];
    return CQCMedia_StdRendDrv::colRendFlds[eFld];
}

const TString&
TCQCStdMediaRendDrv::strRendFld1(const tCQCMedia::ERendFlds eFld)
{
    if (!CQCMedia_StdRendDrv::atomFldListLoaded)
        LoadRedFldLists();

    return CQCMedia_StdRendDrv::colRendFlds[eFld];
}

const TString&
TCQCStdMediaRendDrv::strRendFld2(const tCQCMedia::ERendFlds eFld)
{
    if (!CQCMedia_StdRendDrv::atomFldListLoaded)
        LoadRedFldLists();

    return CQCMedia_StdRendDrv::colRendFldsV2[eFld];
}



// ---------------------------------------------------------------------------
//  CQSLAudPlSDriver: Destructor
// ---------------------------------------------------------------------------
TCQCStdMediaRendDrv::~TCQCStdMediaRendDrv()
{
}


// ---------------------------------------------------------------------------
//  TCQCStdMediaRendDrv: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We let clients get the image data for the title we are currently playing.
//  They could get it themselves by asking for our art path and the repository
//  we are associate with and going directly to the repository, but it's
//  more convenient and more flexible to just let them ask us.
//
//  We can fully handle this stuff for derived classes, though they can
//  override and handle their own if they need to.
//
tCIDLib::TBoolean
TCQCStdMediaRendDrv::bQueryData(const   TString&            strQType
                                , const TString&            strDataName
                                ,       tCIDLib::TCard4&    c4OutBytes
                                ,       THeapBuf&           mbufToFill)
{
    // Handle the query based on the query type
    tCIDLib::TBoolean bRet = kCIDLib::True;

    if (strQType == kCQCMedia::strQuery_CurPLItem)
    {
        c4OutBytes = facCQCMedia().c4FormatCurPLItem(m_mplmList, mbufToFill);
        bRet = (c4OutBytes != 0);
    }
     else if (strQType == kCQCMedia::strQuery_PLItems)
    {
        // Format our PL items out in standard format
        c4OutBytes = facCQCMedia().c4FormatPLItems(m_mplmList, mbufToFill);
        bRet = (c4OutBytes != 0);
    }
     else if (strQType == kCQCMedia::strQuery_QueryArt)
    {
        //
        //  Call a helper and pass in the correct cache members for the
        //  requested art size.
        //
        const tCIDLib::TBoolean bLarge(strDataName.bCompareI(L"Lrg"));
        bRet = bCheckArt
        (
            bLarge
            , bLarge ? m_c4CurLArtBytes : m_c4CurSArtBytes
            , bLarge ? m_mbufCurLArt : m_mbufCurSArt
            , c4OutBytes
            , mbufToFill
        );
    }
     else
    {
        // Don't know what it is, so throw
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDrv_UnknownDataQuery
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strQType
            , strMoniker()
        );
    }
    return bRet;
}


//
//  We handle some standard backdoor queries on behalf of derived classes.
//
tCIDLib::TCard4 TCQCStdMediaRendDrv::c4QueryVal(const TString& strValId)
{
    tCIDLib::TCard4 c4RetVal = 0;

    if (strValId == kCQCMedia::strQuery_QueryCurPLItemId)
    {
        if (!m_mplmList.bQueryCurItemId(c4RetVal))
            c4RetVal = kCIDLib::c4MaxCard;
    }
     else
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDrv_UnknownValQuery
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strValId
            , strMoniker()
        );
    }


    return c4RetVal;
}


// ---------------------------------------------------------------------------
//  TCQCStdMediaRendDrv: Protected, virtual methods
// ---------------------------------------------------------------------------

//
//  Derived classes don't have to override these, so we provide empty default
//  implementations.
//

tCQCKit::ECommResults
TCQCStdMediaRendDrv::eStartPowerTrans(const tCIDLib::TBoolean bNewState)
{
    // Ignore this if in V1 mode, sine the power fields don't exist
    if (c4ArchVersion() > 1)
    {
        // If not overridden, we just fake power management
        tCQCKit::EPowerStatus eNew;
        if (bNewState)
            eNew = tCQCKit::EPowerStatus::Ready;
        else
            eNew = tCQCKit::EPowerStatus::Off;

        bStoreStringFld
        (
            m_c4FldId_PowerStatus, tCQCKit::strXlatEPowerStatus(eNew), kCIDLib::True
        );
    }

    // We always work
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid
TCQCStdMediaRendDrv::LoadDrvSpecificFlds(TVector<TCQCFldDef>&)
{
}

tCIDLib::TVoid
TCQCStdMediaRendDrv::LookupDrvSpecificFlds(const tCIDLib::TCard4)
{
}


// ---------------------------------------------------------------------------
//  TCQCStdMediaRendDrv: Protected, inherited methods
// ---------------------------------------------------------------------------

// Try to load configuration data
tCIDLib::TBoolean TCQCStdMediaRendDrv::bWaitConfig(TThread&)
{
    // We have no config to get unless the derived class overrides
    return kCIDLib::True;
}


//
//  A Boolean field changed. Handle any that are standard fields.
//
tCQCKit::ECommResults
TCQCStdMediaRendDrv::eBoolFldValChanged(const   TString&
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bNewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;
    if (c4FldId == m_c4FldId_AdjVolume)
    {
        // Let the derived class handle this
        eRes = eAdjustVolume(bNewValue);
    }
     else if (c4FldId == m_c4FldId_ClearPL)
    {
        // Tell the derived class to reset its player
        ResetPlayer();

        // Reset the playlist manager back to default state
        m_mplmList.Reset();

        // Clear the metadata fields
        ClearMetaFields();

        //
        //  If we are in random category mode, then see if we can do a random
        //  category update.
        //
        if (m_mplmList.ePlayListMode() == tCQCMedia::EPLModes::RandomCat)
        {
            // Reset the throttling timer to insure we can do a user driver one
            m_enctLastRandCat = 0;
            bRandomCatUpdate();
        }
         else
        {
            // We are going to remain empty, so update playlist related fields
            StorePLFields();
        }
    }
     else if (c4FldId == m_c4FldId_Mute)
    {
        // Let the derived class handle this
        eRes = eSetMute(bNewValue);
    }
     else if (c4FldId == m_c4FldId_Power)
    {
        //
        //  Let the derived class react to this. He must call StorePowerStatus()
        //  to keep us informed of the actual power state transitions.
        //
        eRes = eStartPowerTrans(bNewValue);
    }
     else
    {
        IncUnknownWriteCounter();
        eRes = tCQCKit::ECommResults::FieldNotFound;
    }
    return eRes;
}


//
//  A Cardinal field changed. Handle any that are standard fields.
//
tCQCKit::ECommResults
TCQCStdMediaRendDrv::eCardFldValChanged(const   TString&
                                        , const tCIDLib::TCard4 c4FldId
                                        , const tCIDLib::TCard4 c4NewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;

    if (c4FldId == m_c4FldId_DelPLItem)
    {
        if (!bDelPLItem(c4NewValue))
            eRes = tCQCKit::ECommResults::ValueRejected;
    }
     else if (c4FldId == m_c4FldId_SelPLItem)
    {
        if (!bSelPLItem(c4NewValue))
            eRes = tCQCKit::ECommResults::ValueRejected;
    }
     else if (c4FldId == m_c4FldId_Volume)
    {
        // Let the derived class handle this one
        eRes = eSetVolume(c4NewValue);
    }
     else if (c4FldId == m_c4FldId_ZoneNum)
    {
        // Do nothing. This is a field for customer use to store a zone number
    }
     else
    {
        IncUnknownWriteCounter();
        eRes = tCQCKit::ECommResults::FieldNotFound;
    }
    return eRes;
}


//
//  The derived class will override this and do the actual connection to the
//  player. If he succeeds, he calls us and we'll set up with an empty playlist
//  and clear metadata fields. The derived class should stop playback as well
//  and update any player state fields he has.
//
tCQCKit::ECommResults TCQCStdMediaRendDrv::eConnectToDevice(TThread&)
{
    //
    //  Force a check of current player state so that the derived class can
    //  tell us if he connected ok.
    //
    tCIDLib::TBoolean       bMute;
    tCIDLib::TCard4         c4Volume;
    tCIDLib::TEncodedTime   enctCurPos;
    tCIDLib::TEncodedTime   enctTotal;
    const tCQCKit::ECommResults eRet = eCheckPlayerStatus
    (
        enctCurPos, enctTotal, m_eLastState, m_eLastEndState, bMute, c4Volume
    );

    //
    //  If he connected, then initialize our states assuming the derived
    //  class did the right thing and is stopped right now. If he didn't
    //  connect, don't bother since no one can see the fields anyway.
    //
    if (eRet == tCQCKit::ECommResults::Success)
    {
        m_c4CurLArtBytes = 0;
        m_c4CurSArtBytes = 0;
        m_eLastEndState = tCQCMedia::EEndStates::Ended;
        m_eLastState = tCQCMedia::EMediaStates::Stopped;

        bStoreStringFld
        (
            m_c4FldId_MediaState
            , tCQCMedia::strXlatEMediaStates(m_eLastState)
            , kCIDLib::True
        );
        bStoreBoolFld(m_c4FldId_Mute, bMute, kCIDLib::True);
        bStoreCardFld(m_c4FldId_Volume, c4Volume, kCIDLib::True);

        // Clear all metadata related fields
        ClearMetaFields();

        // Clear the playlist and update fields related to that
        m_mplmList.Reset();
        StorePLFields();
    }
    return eRet;
}


// A floating point field changed
tCQCKit::ECommResults
TCQCStdMediaRendDrv::eFloatFldValChanged(const  TString&
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TFloat8    f8NewValue)
{
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::FieldNotFound;
}



// Initialize ourself and register fields
tCQCKit::EDrvInitRes TCQCStdMediaRendDrv::eInitializeImpl()
{
    // Set a flag to use for V1/V2 stuff below
    const tCIDLib::TBoolean bV2(c4ArchVersion() > 1);

    //
    //  At this level we will handle any common driver prompts. The derived
    //  class must handle any that he defines for himself.
    //
    const TCQCDriverObjCfg& cqcdcTmp = cqcdcThis();

    m_strDefRepoMoniker.Clear();
    if (!cqcdcTmp.bFindPromptValue(L"RepoDriver", L"Moniker", m_strDefRepoMoniker))
    {
        //
        //  Log something but don't consider it a failure. It's not required
        //  anymore, though it will be needed if they try to set random
        //  category playback mode.
        //
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The repository driver prompt was not set for driver %(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker()
            );
        }
    }

    //
    //  A new prompt was added to let them set the zone number field. It
    //  may not be present in previously installed instances.
    //
    TString strZoneNum;
    if (!cqcdcTmp.bFindPromptValue(L"ZoneNum", L"Selected", strZoneNum))
        strZoneNum = L"1";

    //
    //  And another new one for the initial volume. It was added after the
    //  fact and might not be there, so default it to 60.
    //
    TString strTmp;
    m_c4DefVolume = 60;
    if (cqcdcTmp.bFindPromptValue(L"LoadVolume", L"Value", strTmp))
        m_c4DefVolume = strTmp.c4Val(tCIDLib::ERadices::Dec);

    //
    //  And set up our device fields
    //
    TVector<TCQCFldDef> colFlds(128);
    TCQCFldDef flddNew;

    //
    //  Load up the standard ones using a helper, and then let the derived
    //  class add any of his own.
    //
    QueryStdRendFields
    (
        colFlds, m_eMTFlags, c4ArchVersion(), m_c4FldFlags
    );
    LoadDrvSpecificFlds(colFlds);

    //
    //  Now register our fields with the base driver class, and go back
    //  and look up the ids.
    //
    SetFields(colFlds);

    // Look up the standard fields
    m_c4FldId_ActiveRepository = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::ActiveRepo, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_AdjVolume = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::AdjustVolume, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_ClearPL = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::ClearPL, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurColName = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurColName, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurColArtist = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurColArtist, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurColCookie = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurColCookie, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurDescr = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurDescr, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurItemName = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurItName, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurItemArtist = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurItArtist, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurItemCookie = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurItCookie, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurItemTime = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurItTime, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurItemTotal = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurItTotal, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurLabel = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurLabel, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurRating = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurRating, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurTitleName = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurTiName, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurTitleCookie = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurTiCookie, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_CurYear = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurYear, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_DelPLItem = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::DelPLItem, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_EnqueueMedia = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::EnqMedia, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_MediaState = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::MediaState, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_Mute = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::Mute, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_PBPercent = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PBPercent, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_PlayMedia = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PlayMedia, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_PLIndex = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PLIndex, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_PLItemCnt = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PLItemCnt, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_PLItemKey = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PLItemKey, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_PLMode = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PLMode, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_PLSerialNum = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PLSerialNum, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_SelPLItem = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::SelPLItem, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_SetRandomCat = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::SetRandomCat, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_SetRepository = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::SetRepo, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_Transport = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::Transport, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_Volume = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::Volume, bV2), kCIDLib::True)->c4Id();
    m_c4FldId_ZoneNum = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::ZoneNum, bV2), kCIDLib::True)->c4Id();

    // Some only if this one support movie media
    if (tCIDLib::bAllBitsOn(m_eMTFlags, tCQCMedia::EMTFlags::Movie))
    {
        m_c4FldId_CurAspectRatio = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurAspect, bV2), kCIDLib::True)->c4Id();
        m_c4FldId_CurCast = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurCast, bV2), kCIDLib::True)->c4Id();
        m_c4FldId_CurLeadActor = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::CurLeadActor, bV2), kCIDLib::True)->c4Id();
    }

    // If V2, look up the power fields
    if (bV2)
    {
        m_c4FldId_Power = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::Power, bV2), kCIDLib::True)->c4Id();
        m_c4FldId_PowerStatus = pflddFind(TCQCStdMediaRendDrv::strRendFld(tCQCMedia::ERendFlds::PowerStatus, bV2), kCIDLib::True)->c4Id();
    }

    // And let the derived class look his up
    LookupDrvSpecificFlds(c4ArchVersion());

    //
    //  Store initial default values in the metadata fields because they won't
    //  otherwise get set until someone starts us playing a title or writes
    //  to them, and we don't want them in error state meantime.
    //
    ClearMetaFields();

    // Also initialize some other fields to defaults
    bStoreStringFld(m_c4FldId_ActiveRepository, m_strDefRepoMoniker, kCIDLib::True);
    bStoreStringFld
    (
        m_c4FldId_PLMode
        , tCQCMedia::strXlatEPLModes(m_mplmList.ePlayListMode())
        , kCIDLib::True
    );
    bStoreCardFld(m_c4FldId_PLSerialNum, m_mplmList.c4SerialNum(), kCIDLib::True);
    bStoreCardFld(m_c4FldId_ZoneNum, strZoneNum.c4Val(tCIDLib::ERadices::Dec), kCIDLib::True);

    // Init the power state to the Off value until proven otherwise, if V2
    if (bV2)
    {
        bStoreStringFld
        (
            m_c4FldId_PowerStatus
            , tCQCKit::strXlatEPowerStatus(tCQCKit::EPowerStatus::Off)
            , kCIDLib::True
        );
    }
    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults TCQCStdMediaRendDrv::ePollDevice(TThread&)
{
    //
    //  Ask the derived class to check the state of the player it controls
    //  and store any driver specific fields related to the state. It
    //  returns us a generic state that we can use, and end of media state,
    //  current playback position, and optionally total time for the current
    //  media. If they don't provide total, we use the time in the metadata,
    //  so we initialize it to a known incorrect value that it will have if
    //  they don't set it.
    //
    tCIDLib::TBoolean       bMute;
    tCIDLib::TCard4         c4Volume;
    tCIDLib::TEncodedTime   enctCurPos;
    tCIDLib::TEncodedTime   enctTotal = tCIDLib::TEncodedTime(-1);
    tCQCMedia::EEndStates   eEndState;
    tCQCMedia::EMediaStates eState;
    tCQCKit::ECommResults eRet = eCheckPlayerStatus
    (
        enctCurPos, enctTotal, eState, eEndState, bMute, c4Volume
    );

    // If he reports anything but success, return that now
    if (eRet != tCQCKit::ECommResults::Success)
        return eRet;

    // He's happy, so let's respond to the info he returned
    try
    {
        // Store the volume and mute values
        bStoreBoolFld(m_c4FldId_Mute, bMute, kCIDLib::True);
        bStoreCardFld(m_c4FldId_Volume, c4Volume, kCIDLib::True);

        //
        //  If the changed store the new state as the last state we saw,
        //  and the state text.
        //
        if (eState != m_eLastState)
        {
            m_eLastState = eState;

            // Store the new state
            try
            {
                bStoreStringFld
                (
                    m_c4FldId_MediaState
                    , tCQCMedia::strXlatEMediaStates(eState)
                    , kCIDLib::True
                );
            }

            catch(...)
            {
            }
        }

        //
        //  If our end state has changed, then we need to see if we can
        //  move to a new playlist item.
        //
        if (eEndState != m_eLastEndState)
        {
            m_eLastEndState = eEndState;

            //
            //  If it indicates the current song has ended, or has failed, then
            //  we need to see if we can load up something else. Else, it's
            //  in the process of loading media or it's playing media.
            //
            if (eEndState != tCQCMedia::EEndStates::Running)
            {
                tCIDLib::TBoolean bActDone = kCIDLib::False;
                if (m_mplmList.ePlayListMode() == tCQCMedia::EPLModes::RandomCat)
                    bActDone = bRandomCatUpdate();

                //
                //  If we didn't do anything in the random category update
                //  above, then select a new item now. It might not if we
                //  recently did a check or it still has enough items that
                //  we don't want to do a random cat reload yet, or we just
                //  aren't in that mode.
                //
                if (!bActDone)
                {
                    //
                    //  If it ended in an error, and the player mode isn't
                    //  going to remove the file from the playlist anyway,
                    //  then remove it now so it won't come back around.
                    //
                    if ((eEndState == tCQCMedia::EEndStates::Failed)
                    &&  (m_mplmList.ePlayListMode() != tCQCMedia::EPLModes::Jukebox))
                    {
                        facCQCMedia().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kMedErrs::errcMP_RemovingBadItem
                            , m_mplmList.mpliCurrent().strItemName()
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , strMoniker()
                        );

                        m_mplmList.eRemoveItemAt(m_mplmList.c4CurIndex());
                    }

                    // And now let the playlist select the next one to play
                    tCIDLib::TCard4 c4NewInd;
                    const tCQCMedia::EPLMgrSelRes eRes = m_mplmList.eSelectNewItem
                    (
                        kCIDLib::True, c4NewInd
                    );

                    //
                    //  We either have emptied the playlist now, or we have
                    //  items to try to load. If empty, clear the metadata
                    //  fields.
                    //
                    if (eRes == tCQCMedia::EPLMgrSelRes::Empty)
                        ClearMetaFields();
                    else
                        LoadNewItem();
                }

                //
                //  Either way, store the serial number, in case it changed.
                //  If it didn't, then nothing will happen, so it's easier to
                //  just be safe and always store it after an end of media
                //  event.
                //
                //  Update the playlist related fields
                //
                StorePLFields();
            }
        }

        // If our general state is playing, store the time info
        if (eState == tCQCMedia::EMediaStates::Playing)
        {
            // Store the returned current position time
            bStoreTimeFld(m_c4FldId_CurItemTime, enctCurPos, kCIDLib::True);

            //
            //  If the derived class provided a total time, or we can get one
            //  from the current playlist, then let's store that.
            //
            if ((enctTotal != tCIDLib::TEncodedTime(-1))
            ||  !m_mplmList.bIsEmpty())
            {
                //
                //  Get the time in total seconds. If the derived class provided
                //  one, we have to convert. Else get it from metadata.
                //
                tCIDLib::TCard4 c4TotSecs;
                if (enctTotal == tCIDLib::TEncodedTime(-1))
                    c4TotSecs = m_mplmList.mpliCurrent().c4ItemSeconds();
                else
                    c4TotSecs = tCIDLib::TCard4(enctTotal / kCIDLib::enctOneSecond);

                // Convert the current time to seconds as well
                const tCIDLib::TCard4 c4CurSecs = tCIDLib::TCard4
                (
                    enctCurPos / kCIDLib::enctOneSecond
                );

                // And now calculate a percent, clipping to limits if needed
                tCIDLib::TCard4 c4Percent = 0;
                if (c4TotSecs)
                {
                    c4Percent = tCIDLib::TCard4
                    (
                        (tCIDLib::TFloat4(c4CurSecs) / c4TotSecs) * 100.0
                    );
                    if (c4Percent > 100)
                        c4Percent = 100;
                }
                bStoreCardFld(m_c4FldId_PBPercent, c4Percent, kCIDLib::True);
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


// A string field changed
tCQCKit::ECommResults
TCQCStdMediaRendDrv::eStringFldValChanged(const TString&
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;

    if ((c4FldId == m_c4FldId_EnqueueMedia)
    ||  (c4FldId == m_c4FldId_PlayMedia))
    {
        // Call a helper for this one since it's a good bit of code
        if (!bDoPlayMedia(strNewValue, (c4FldId == m_c4FldId_EnqueueMedia)))
            eRes = tCQCKit::ECommResults::ValueRejected;
    }
     else if (c4FldId == m_c4FldId_PLMode)
    {
        //
        //  We allow them to set the play list mode by writing to this
        //  field. We just need to translate it to our play list enum
        //  and store that value for later use.
        //
        tCQCMedia::EPLModes ePLMode = tCQCMedia::EPLModes::Min;
        for (; ePLMode <= tCQCMedia::EPLModes::Max; ePLMode++)
        {
            if (strNewValue == tCQCMedia::strXlatEPLModes(ePLMode))
            {
                m_mplmList.ePlayListMode(ePLMode);

                // If random category mode, then get a new random whack
                if (ePLMode == tCQCMedia::EPLModes::RandomCat)
                {
                    // Reset the timer to insure we can do one
                    m_enctLastRandCat = 0;
                    bRandomCatUpdate();
                }
                break;
            }
        }

        if (ePLMode > tCQCMedia::EPLModes::Max)
            eRes = tCQCKit::ECommResults::BadValue;
    }
     else if (c4FldId == m_c4FldId_SetRandomCat)
    {
        // Make sure a default repository is set
        if (m_strDefRepoMoniker.bIsEmpty())
        {
            LogMsg
            (
                L"No default repository is set, can't do random mode"
                , tCQCKit::EVerboseLvls::Low
                , CID_FILE
                , CID_LINE
            );
            return tCQCKit::ECommResults::MissingInfo;
        }

        // Make sure it's a cat cookie and for music
        tCIDLib::TCard2         c2CatId;
        tCIDLib::TCard2         c2ColId;
        tCIDLib::TCard2         c2ItemId;
        tCIDLib::TCard2         c2TitleId;
        tCQCMedia::EMediaTypes  eMType;
        tCQCMedia::ECookieTypes eCookieType = facCQCMedia().eCheckCookie
        (
            strNewValue, eMType, c2CatId, c2TitleId, c2ColId, c2ItemId
        );
        if ((eMType != tCQCMedia::EMediaTypes::Music)
        ||  (eCookieType != tCQCMedia::ECookieTypes::Cat))
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_RandCatPlayType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::TypeMatch
            );
            return tCQCKit::ECommResults::BadValue;
        }
        m_strRandomCat = strNewValue;

        //
        //  If already in random cat mode, reset our throttling timer (so
        //  we can force it to happen now) and do an update.
        //
        if (m_mplmList.ePlayListMode() == tCQCMedia::EPLModes::RandomCat)
        {
            m_enctLastRandCat = 0;
            bRandomCatUpdate();
        }
    }
     else if (c4FldId == m_c4FldId_SetRepository)
    {
        // If the new value is empty, take the default repository
        if (strNewValue.bIsEmpty())
            bStoreStringFld(m_c4FldId_ActiveRepository, m_strDefRepoMoniker, kCIDLib::True);
        else
            bStoreStringFld(m_c4FldId_ActiveRepository, strNewValue, kCIDLib::True);
    }
     else if (c4FldId == m_c4FldId_Transport)
    {
        // Make sure it's a valid transport command
        const tCQCMedia::ETransCmds eCmd = tCQCMedia::eXlatETransCmds(strNewValue);
        if (eCmd == tCQCMedia::ETransCmds::Count)
            eRes = tCQCKit::ECommResults::BadValue;
        else
            eRes = eDoTransportCmd(eCmd);
    }
     else
    {
        IncUnknownWriteCounter();
        eRes = tCQCKit::ECommResults::FieldNotFound;
    }
    return eRes;
}


// ---------------------------------------------------------------------------
//  CQSLAudPlSDriver: Hidden constructors
// ---------------------------------------------------------------------------
TCQCStdMediaRendDrv::TCQCStdMediaRendDrv(const  TCQCDriverObjCfg&   cqcdcToLoad
                                        , const tCQCMedia::EPLModes eDefPLMode
                                        , const tCIDLib::TCard4     c4FldFlags
                                        , const tCQCMedia::EMTFlags eMTFlags) :
    TCQCServerBase(cqcdcToLoad)
    , m_c4CurLArtBytes(0)
    , m_c4CurSArtBytes(0)
    , m_c4DefVolume(60)
    , m_c4FldFlags(c4FldFlags)
    , m_c4FldId_ActiveRepository(kCIDLib::c4MaxCard)
    , m_c4FldId_AdjVolume(kCIDLib::c4MaxCard)
    , m_c4FldId_ClearPL(kCIDLib::c4MaxCard)
    , m_c4FldId_CurAspectRatio(kCIDLib::c4MaxCard)
    , m_c4FldId_CurCast(kCIDLib::c4MaxCard)
    , m_c4FldId_CurColArtist(kCIDLib::c4MaxCard)
    , m_c4FldId_CurColCookie(kCIDLib::c4MaxCard)
    , m_c4FldId_CurColName(kCIDLib::c4MaxCard)
    , m_c4FldId_CurDescr(kCIDLib::c4MaxCard)
    , m_c4FldId_CurItemName(kCIDLib::c4MaxCard)
    , m_c4FldId_CurItemArtist(kCIDLib::c4MaxCard)
    , m_c4FldId_CurItemCookie(kCIDLib::c4MaxCard)
    , m_c4FldId_CurItemTime(kCIDLib::c4MaxCard)
    , m_c4FldId_CurItemTotal(kCIDLib::c4MaxCard)
    , m_c4FldId_CurLabel(kCIDLib::c4MaxCard)
    , m_c4FldId_CurLeadActor(kCIDLib::c4MaxCard)
    , m_c4FldId_CurRating(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTitleName(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTitleCookie(kCIDLib::c4MaxCard)
    , m_c4FldId_CurYear(kCIDLib::c4MaxCard)
    , m_c4FldId_DelPLItem(kCIDLib::c4MaxCard)
    , m_c4FldId_EnqueueMedia(kCIDLib::c4MaxCard)
    , m_c4FldId_PBPercent(kCIDLib::c4MaxCard)
    , m_c4FldId_MediaState(kCIDLib::c4MaxCard)
    , m_c4FldId_Mute(kCIDLib::c4MaxCard)
    , m_c4FldId_PlayMedia(kCIDLib::c4MaxCard)
    , m_c4FldId_PLIndex(kCIDLib::c4MaxCard)
    , m_c4FldId_PLItemCnt(kCIDLib::c4MaxCard)
    , m_c4FldId_PLItemKey(kCIDLib::c4MaxCard)
    , m_c4FldId_PLMode(kCIDLib::c4MaxCard)
    , m_c4FldId_PLSerialNum(kCIDLib::c4MaxCard)
    , m_c4FldId_Power(kCIDLib::c4MaxCard)
    , m_c4FldId_PowerStatus(kCIDLib::c4MaxCard)
    , m_c4FldId_SelPLItem(kCIDLib::c4MaxCard)
    , m_c4FldId_SetRepository(kCIDLib::c4MaxCard)
    , m_c4FldId_SetRandomCat(kCIDLib::c4MaxCard)
    , m_c4FldId_Transport(kCIDLib::c4MaxCard)
    , m_c4FldId_Volume(kCIDLib::c4MaxCard)
    , m_c4FldId_ZoneNum(kCIDLib::c4MaxCard)
    , m_eLastEndState(tCQCMedia::EEndStates::Ended)
    , m_eLastState(tCQCMedia::EMediaStates::Undefined)
    , m_eMTFlags(eMTFlags)
    , m_enctLastRandCat(0)
    , m_mplmList(cqcdcToLoad.strMoniker())
{
    // Default our random category to the all music category
    facCQCMedia().FormatCatCookie
    (
        tCQCMedia::EMediaTypes::Music, kCQCMedia::c2CatId_AllMusic, m_strRandomCat
    );

    // Set the default playlist mode that we were given
    m_mplmList.ePlayListMode(eDefPLMode);
}


// ---------------------------------------------------------------------------
//  TCQCStdMediaRendDrv: Protected, virtual methods
// ---------------------------------------------------------------------------

//
//  Just a helper to reset all the metadata fields to default values. The
//  derived clas can override if he has more, and then call us.
//
tCIDLib::TVoid TCQCStdMediaRendDrv::ClearMetaFields()
{
    bStoreStringFld(m_c4FldId_CurColName, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurColArtist, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurColCookie, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurDescr, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurItemName, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurItemArtist, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurItemCookie, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurLabel, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurRating, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurTitleName, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurTitleCookie, TString::strEmpty(), kCIDLib::True);
    bStoreCardFld(m_c4FldId_CurYear, 0, kCIDLib::True);

    // Some movie only ones
    if (tCIDLib::bAllBitsOn(m_eMTFlags, tCQCMedia::EMTFlags::Movie))
    {
        bStoreStringFld(m_c4FldId_CurAspectRatio, TString::strEmpty(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurCast, TString::strEmpty(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurLeadActor, TString::strEmpty(), kCIDLib::True);
    }

    // Zero out the playback related fields
    bStoreTimeFld(m_c4FldId_CurItemTime, 0, kCIDLib::True);
    bStoreTimeFld(m_c4FldId_CurItemTotal, 0, kCIDLib::True);
    bStoreCardFld(m_c4FldId_PBPercent, 0, kCIDLib::True);

    // No cover art anymore either, do this before we change the key field
     m_c4CurLArtBytes = 0;
     m_c4CurSArtBytes = 0;

    // Clear the playlist item key field
    bStoreStringFld(m_c4FldId_PLItemKey, TString::strEmpty(), kCIDLib::True);
}


//
//  Update our medatadata fields for the current playlist item. The derived
//  class can override if needed if he has fields of his own, and then call
//  us to do the standard ones.
//
tCIDLib::TVoid
TCQCStdMediaRendDrv::UpdateMetaFields(const TCQCMediaPLItem& mpliCur)
{
    // Store the collection level stuff
    bStoreStringFld(m_c4FldId_CurColArtist, mpliCur.strColArtist(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurColName, mpliCur.strColName(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurColCookie, mpliCur.strColCookie(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurDescr, mpliCur.strColDescription(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurLabel, mpliCur.strColLabel(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurRating, mpliCur.strColRating(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurTitleCookie, mpliCur.strTitleCookie(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurTitleName, mpliCur.strTitleName(), kCIDLib::True);
    bStoreCardFld(m_c4FldId_CurYear, mpliCur.c4ColYear(), kCIDLib::True);

    //
    //  Store the item level stuff. Duration is a special case. We have to keep
    //  it as seconds becasue thats what is expected by the standard methods
    //  that we use to provide access to our play list info to clients. But
    //  the field we want to be in HH:MM:SS format, with leading zero if it's
    //  less than 10 in any case.
    //
    bStoreStringFld(m_c4FldId_CurItemCookie, mpliCur.strItemCookie(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurItemName, mpliCur.strItemName(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurItemArtist, mpliCur.strItemArtist(), kCIDLib::True);
    bStoreTimeFld
    (
        m_c4FldId_CurItemTotal
        , mpliCur.c4ItemSeconds() * kCIDLib::enctOneSecond
        , kCIDLib::True
    );

    // Some movie only ones
    if (tCIDLib::bAllBitsOn(m_eMTFlags, tCQCMedia::EMTFlags::Movie))
    {
        bStoreStringFld(m_c4FldId_CurAspectRatio, mpliCur.strColAspectRatio(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurCast, mpliCur.strColCast(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurLeadActor, mpliCur.strColLeadActor(), kCIDLib::True);
    }

    //
    //  No cover art until we load it again. If no one asks, then we won't
    //  ever bother. Do this before we update the key field below.
    //
    m_c4CurLArtBytes = 0;
    m_c4CurSArtBytes = 0;

    //
    //  Set up the playlist item key, which provides info to clients that they
    //  can use to get more info about the current item.
    //
    //  Any classes derived from us are completely normal ones based on a
    //  standard repo. So we do repo, space, and cookie as our key. If there
    //  is an item cookie we use that, else the collection cookie.
    //
    //  In order to allow clients to know that this is standard renderer/repo
    //  stuff, we use a prefix. If they don't see this prefix, they will not try
    //  to interpret the value.
    //
    m_strTmp = kCQCMedia::strRend_StdPLItemKeyPref;
    m_strTmp.Append(mpliCur.strRepoMoniker());
    m_strTmp.Append(kCIDLib::chSpace);
    if (!mpliCur.strItemCookie().bIsEmpty())
        m_strTmp.Append(mpliCur.strItemCookie());
    else
        m_strTmp.Append(mpliCur.strColCookie());
    bStoreStringFld(m_c4FldId_PLItemKey, m_strTmp, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TCQCStdMediaRendDrv: Protected, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TCard4 TCQCStdMediaRendDrv::c4DefVolume() const
{
    return m_c4DefVolume;
}


//
//  This is called if the derived class wants to make us move to the next
//  or previous item in the playlist.
//
tCIDLib::TVoid
TCQCStdMediaRendDrv::SelectNextPrev(const tCIDLib::TBoolean bNext)
{
    //
    //  Check to see if we need to get more random items. If it loaded a
    //  new item, then we are done.
    //
    if (m_mplmList.ePlayListMode() == tCQCMedia::EPLModes::RandomCat)
    {
        if (bRandomCatUpdate())
            return;
    }

    // If the playlist is empty, then nothing to do
    if (m_mplmList.bIsEmpty())
        return;

    // Else let's try to select a new item
    tCIDLib::TCard4 c4NewInd;
    const tCQCMedia::EPLMgrSelRes eRes = m_mplmList.eSelectNewItem
    (
        bNext, c4NewInd
    );

    //
    //  If this emptied the list, we need to clean up. Else play the
    //  new selection.
    //
    if (eRes == tCQCMedia::EPLMgrSelRes::Empty)
    {
        ResetPlayer();
    }
     else
    {
        //
        //  A new item one way or another, maybe with a change to the
        //  playlist contents, maybe not. If so the storing of the PL fields
        //  below will update the serial number if the list was changed.
        //  Here we just need to load the new item.
        //
        LoadNewItem();
    }

    // Update the playlist related fields since they likely changed
    StorePLFields();
}


//
//  Most drivers set this in the call to our ctor, but sometimes they don't know it until
//  later on. They have to set this before they call our initialization method.
//
tCIDLib::TVoid TCQCStdMediaRendDrv::SetMTFlags(const tCQCMedia::EMTFlags eToSet)
{
    m_eMTFlags = eToSet;
}


//
//  Let's the derived class tell us his power status. If we are in V1 mode, we ignore
//  this.
//
tCIDLib::TVoid
TCQCStdMediaRendDrv::StorePowerStatus(const tCQCKit::EPowerStatus eStatus)
{
    if (c4ArchVersion() > 1)
    {
        bStoreStringFld
        (
            m_c4FldId_PowerStatus, tCQCKit::strXlatEPowerStatus(eStatus), kCIDLib::True
        );
    }
}



// ---------------------------------------------------------------------------
//  TCQCStdMediaRendDrv: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper for the backdoor command that serves up current cover art to
//  the clients. Since we have to handle both large and small art, this
//  avoids some redundant code. They just pass us the appropriate values
//  for the size queried.
//
tCIDLib::TBoolean
TCQCStdMediaRendDrv::bCheckArt( const   tCIDLib::TBoolean   bLarge
                                ,       tCIDLib::TCard4&    c4CBytes
                                ,       THeapBuf&           mbufCache
                                ,       tCIDLib::TCard4&    c4RetBytes
                                ,       THeapBuf&           mbufRet)
{
    // If we have this size art cached, do the simple scenario
    if (c4CBytes)
    {
        c4RetBytes = c4CBytes;
        if (c4RetBytes)
            mbufRet.CopyIn(mbufCache, c4RetBytes);
        return kCIDLib::True;
    }

    // If the playlist is not empty, then let's query the art
    tCIDLib::TBoolean bItem;
    TString strRepo;
    TString strCookie;
    if (m_mplmList.bQueryCurCookie(strRepo, strCookie, bItem))
    {
        try
        {
            // Get a proxy for the repo
            tCQCKit::TCQCSrvProxy orbcAdmin
            (
                facCQCKit().orbcCQCSrvAdminProxy(strRepo)
            );

            //
            //  Figure out which art query to send based on cookie type
            //  and art size.
            //
            TString strQ;
            if (bItem)
            {
                if (bLarge)
                    strQ = kCQCMedia::strQuery_QueryItemArt;
                else
                    strQ = kCQCMedia::strQuery_QueryItemThumbArt;
            }
             else
            {
                if (bLarge)
                    strQ = kCQCMedia::strQuery_QueryArt;
                else
                    strQ = kCQCMedia::strQuery_QueryThumbArt;
            }

            //
            //  Do the query to the repo. If it returns false, zero the
            //  size just in case.
            //
            if (!orbcAdmin->bQueryData(strRepo, strQ, strCookie, c4CBytes, mbufCache))
                c4CBytes = 0;
        }

        catch(TError& errToCatch)
        {
            c4CBytes = 0;
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }


    // If we got any new stuff, put it into the return info
    c4RetBytes = c4CBytes;
    if (c4RetBytes)
    {
        mbufRet.CopyIn(mbufCache, c4RetBytes);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Delete a playlist item by its unique item id. The playlist manager will
//  tell us that he rejected it if he can't find an item with that id, or
//  that the playlist is empty so we should stop, or he selected a new item
//  that we should load.
//
tCIDLib::TBoolean
TCQCStdMediaRendDrv::bDelPLItem(const tCIDLib::TCard4 c4ItemId)
{
    // Ask the manager to remove it and se what the result is
    tCQCMedia::EPLMgrActs eAct = m_mplmList.eRemoveItemById(c4ItemId);

    //
    //  If it was rejected, then don't do anything. It's always possible
    //  that the item finished playing just as they sent the command.
    //
    tCIDLib::TBoolean bRet = kCIDLib::True;
    if (eAct == tCQCMedia::EPLMgrActs::Rejected)
    {
        bRet = kCIDLib::False;
    }
     else
    {
        //
        //  Stop playback if we deleted the last one. If a new item was
        //  selected, then start playing the new one.
        //
        if (eAct == tCQCMedia::EPLMgrActs::Stop)
        {
            ResetPlayer();
            ClearMetaFields();
        }
         else if (eAct == tCQCMedia::EPLMgrActs::NewItem)
        {
            LoadNewItem();
        }

        //
        //  We at least removed something, and possibly started somthing else
        //  playing,  so update the playlist related fields
        //
        StorePLFields();
    }
    return bRet;
}



//
//  This is called when we get a write to our PlayMedia or EnqueueMedia fields.
//  We get a cookie and must react appropriately. The cookie can be a title
//  cookie, a collection cookie, or an item cookie. We can also get a repository
//  as well. If so, we take that, else we assume it's from our default repo.
//
//  If a title cookie, we just get the first collection of the title and then
//  fall through as though we got a collection.
//
//  If it's an item cookie, for a play we just look for that item cookie in
//  our list and switch to it if found. For an enqueue, we just add that
//  new item to our list.
//
tCIDLib::TBoolean
TCQCStdMediaRendDrv::bDoPlayMedia(const TString&            strToPlay
                                , const tCIDLib::TBoolean   bEnqueue)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facCQCMedia().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Renderer %(1) received info '%(2)' for %(3)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strMoniker()
            , strToPlay
            , TString(bEnqueue ? L"queing" : L"playing")
        );
    }

    //
    //  If it's a play command, and we are in random category mode, then
    //  to back to normal mode, since this is overriding that.
    //
    if (!bEnqueue
    &&  (m_mplmList.ePlayListMode() == tCQCMedia::EPLModes::RandomCat))
    {
        m_mplmList.ePlayListMode(tCQCMedia::EPLModes::Normal);
        bStoreStringFld
        (
            m_c4FldId_PLMode
            , tCQCMedia::strXlatEPLModes(m_mplmList.ePlayListMode())
            , kCIDLib::True
        );
    }

    // Get our current repository moniker from the field
    TString strDefRepo = strFldVal(m_c4FldId_ActiveRepository);

    //
    //  Ask the play list manager to add this new stuff. Indicate that
    //  we only support music.
    //
    const tCQCMedia::EPLMgrActs eAct = m_mplmList.eAddMedia
    (
        strDefRepo
        , strToPlay
        , bEnqueue
        , m_eMTFlags
        , TString::strEmpty()
    );

    //
    //  Based on the reply, we need to either reject the request, or we
    //  might need to start playing something if the list was empty before.
    //
    if (eAct == tCQCMedia::EPLMgrActs::Rejected)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Driver %(1) rejected media info '%(2)'"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker()
                , strToPlay
            );
        }
        return kCIDLib::False;
    }

    //
    //  And if we need to start playing, then do so. It could be because
    //  a new item was selected via item cookie, or because this is the
    //  first stuff added to a previously empty list.
    //
    if ((eAct == tCQCMedia::EPLMgrActs::NewSelection)
    ||  (eAct == tCQCMedia::EPLMgrActs::NewItem))
    {
        //
        //  If a new selection the select one to play. In the case of a
        //  new item selection, it was selected already. If we select a
        //  new one, this will mark it as having been played.
        //
        if (eAct == tCQCMedia::EPLMgrActs::NewSelection)
        {
            //
            //  Note that we know in this case that it cannot return that
            //  it emptied the list, since we just added at least one new item
            //  to the list which has not been played yet.
            //
            tCIDLib::TCard4 c4NewInd;
            m_mplmList.eSelectNewItem(kCIDLib::True, c4NewInd);
        }

        LoadNewItem();
    }

    // Update the playlist related fields
    StorePLFields();

    return kCIDLib::True;
}


//
//  If we are in random category mode, this will be called when our list
//  gets down to 10 or fewer items. We'll query up to x random items.
//
//  We return true if we selected a new item, else false.
//
tCIDLib::TBoolean TCQCStdMediaRendDrv::bRandomCatUpdate()
{
    //
    //  If not in random category mode, or the play list is larger than
    //  our minimum count we'll let it get down to, then do nothing.
    //
    if ((m_mplmList.ePlayListMode() != tCQCMedia::EPLModes::RandomCat)
    ||  (m_mplmList.c4ItemCount() > CQCMedia_StdRendDrv::c4MinRandCatItems))
    {
        return kCIDLib::False;
    }

    //
    //  If we still have more than one item in the list and it's not been
    //  the minimum time since we checked, then don't do it.
    //
    if ((m_mplmList.c4ItemCount() > 1)
    &&  ((TTime::enctNow() - m_enctLastRandCat) < CQCMedia_StdRendDrv::enctMinRandCatInt))
    {
        return kCIDLib::False;
    }

    // Reset out timer first, in case we get an error
    m_enctLastRandCat = TTime::enctNow();

    // Read in the current active repository
    TString strRepoMoniker = strFldVal(m_c4FldId_ActiveRepository);

    const tCQCMedia::EPLMgrActs eAct = m_mplmList.eRandomCatUpdate
    (
        strRepoMoniker, m_strRandomCat
    );

    //
    //  If told we now need to make a new selection, then do so. This guy
    //  only returns either no action or new selection.
    //
    if (eAct == tCQCMedia::EPLMgrActs::NewSelection)
    {
        tCIDLib::TCard4 c4NewInd;
        const tCQCMedia::EPLMgrSelRes eRes = m_mplmList.eSelectNewItem
        (
            kCIDLib::True, c4NewInd
        );

        if (eRes != tCQCMedia::EPLMgrSelRes::Empty)
            LoadNewItem();

        // Update the playlist related fields
        StorePLFields();

        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Called when we need to select a new playlist item by its id. The PL
//  manager will tell us either he selected the new item or rejected the
//  request because he couldn't find that id.
//
tCIDLib::TBoolean
TCQCStdMediaRendDrv::bSelPLItem(const tCIDLib::TCard4 c4ItemId)
{
    //
    //  Ask the PL manager to select this item as the current one, if it
    //  has one with this id.
    //
    tCQCMedia::EPLMgrActs eAct = m_mplmList.eSelectItemById(c4ItemId);

    // If it indicates it selected a new item, then load it
    const tCIDLib::TBoolean bRet = (eAct == tCQCMedia::EPLMgrActs::NewItem);
    if (bRet)
    {
        LoadNewItem();

        // Update the playlsit related fields
        StorePLFields();
    }
    return bRet;
}



//
//  Called when the playlist manager tells us its time to load a new item
//  We get the current item from the playlist manager and give the derived
//  class a chance to start it up. If that goes well, we set metadata fields
//  based on the current item.
//
tCIDLib::TVoid TCQCStdMediaRendDrv::LoadNewItem()
{
    // Set our states so that any failure will be seen as a change
    m_eLastState = tCQCMedia::EMediaStates::Undefined;
    m_eLastEndState = tCQCMedia::EEndStates::Running;

    const TCQCMediaPLItem& mpliCur = m_mplmList.mpliCurrent();
    if (bPlayNewItem(mpliCur))
    {
        // Load up metadata
        UpdateMetaFields(mpliCur);
    }
     else
    {
        m_c4CurLArtBytes = 0;
        m_c4CurSArtBytes = 0;
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcMP_FailedStart
                , mpliCur.strItemName()
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker()
            );
        }
    }

}


//
//  There are a number of places where we need to update the playlist state
//  related fields after we change the list. So we break that out here to make
//  it easier to insure we do the right thing always and to add new stuff
//  later if needed.
//
tCIDLib::TVoid TCQCStdMediaRendDrv::StorePLFields()
{
    bStoreCardFld(m_c4FldId_PLIndex, m_mplmList.c4CurIndex(), kCIDLib::True);
    bStoreCardFld(m_c4FldId_PLItemCnt, m_mplmList.c4ItemCount(), kCIDLib::True);
    bStoreCardFld(m_c4FldId_PLSerialNum, m_mplmList.c4SerialNum(), kCIDLib::True);
}


