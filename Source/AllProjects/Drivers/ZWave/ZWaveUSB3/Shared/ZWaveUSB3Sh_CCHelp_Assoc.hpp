//
// FILE NAME: ZWaveUSB3Sh_CCHelp_Assoc.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/21/2017
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
//  This is a CC helper namespace to provide some grunt work helpers for the
//  Association command class. These helper namespaces are for utility type CCs,
//  not for those that implement actual automation functionality.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   NAMESPACE: TZWUSB3CCAssoc
// ---------------------------------------------------------------------------
namespace TZWUSB3CCAssoc
{
    //
    //  Get the number of association groups for the indicated unit.
    //
    ZWUSB3SHEXPORT tCIDLib::TBoolean bQueryAssocGrps
    (
                TZWUnitInfo&            unitiTar
        ,       tCIDLib::TCard4&        c4ToFill
        , const tCIDLib::TBoolean       bAsync
        , const tCIDLib::TBoolean       bKnownAwake
        ,       TString&                strErrMsg
    );


    //
    //  Query the associations for a specific group. This one can only be done
    //  synchronously, so we have to get a driver control interface. If it ever
    //  needs to be done async by the units, they will have to do their own sending
    //  of the query and waiting on the replies. This one can send multiple replies
    //  and the unit iterface doesn't supporting doing that synchronously.
    //
    //  If the unit supports multi-channel association we do that type of query,
    //  else we do a regular association query.
    //
    //  The values in fcolToFill are:
    //
    //  1.  For non-end points the top word is the unit id and the bottom is 0xFFFF
    //      to indicate no end point.
    //  2.  For end point ones, the top word is the unit id and the bottom word is
    //      the end point id, which can be 0 to 0x7F.
    //
    //  Doing it this way will make them naturally sort by unit id and then end point.
    //
    ZWUSB3SHEXPORT tCIDLib::TBoolean bQueryAssociations
    (
                MZWaveCtrlIntf* const   pmzwciOwner
        , const TZWUnitInfo&            unitiSrc
        , const tCIDLib::TCard1         c1GroupNum
        ,       tCIDLib::TCardList&     fcolToFill
        ,       TString&                strErrMsg
    );


    //
    //  If c1EPId is not 0xFF, then it is assumed that we need to do a multi-channel
    //  association. We don't send an encapsulated msg since we are talking to the
    //  unit itself here, not an individual end point.
    //
    //  This would only happen when we set an association for the user upon request.
    //  We ourself are not multi-channel.
    //
    //  If the unit is a non-listener, the msg will just be queued up on the unit
    //  info object, unless the known awake flag is true. If you do non-async, and
    //  its a non-listern and bKnownAwake is false, an exception will occur.
    //
    ZWUSB3SHEXPORT tCIDLib::TBoolean bRemoveAssoc
    (
                TZWUnitInfo&            unitiTar
        , const tCIDLib::TCard1         c1GroupNum
        , const tCIDLib::TCard1         c1IdToRemove
        , const tCIDLib::TBoolean       bAsync
        , const tCIDLib::TBoolean       bKnownAwake
        ,       TString&                strErrMsg
        , const tCIDLib::TCard1         c1EPId = 0xFF
    );

    ZWUSB3SHEXPORT tCIDLib::TBoolean bSetAssoc
    (
                TZWUnitInfo&            unitiTar
        , const tCIDLib::TCard1         c1GroupNum
        , const tCIDLib::TCard1         c1IdToAdd
        , const tCIDLib::TBoolean       bAsync
        , const tCIDLib::TBoolean       bKnownAwake
        ,       TString&                strErrMsg
        , const tCIDLib::TCard1         c1EPId = 0xFF
    );
}

#pragma CIDLIB_POPPACK

