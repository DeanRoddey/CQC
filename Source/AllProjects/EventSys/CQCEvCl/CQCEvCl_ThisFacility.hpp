//
// FILE NAME: CQCEvCl_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCEvCl
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCEVCLEXPORT TFacCQCEvCl : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCEvCl();

        TFacCQCEvCl(const TFacCQCEvCl&) = delete;
        TFacCQCEvCl(TFacCQCEvCl&&) = delete;

        ~TFacCQCEvCl();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TFacCQCEvCl& operator=(const TFacCQCEvCl&) = delete;
        TFacCQCEvCl& operator=(TFacCQCEvCl&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::EEvSrvTypes eBrwsDTypeToEvType
        (
            const   tCQCRemBrws::EDTypes    eToXlat
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        )   const;

        tCQCRemBrws::EDTypes eEvTypeToBrwsDType
        (
            const   tCQCKit::EEvSrvTypes    eToXlat
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        )   const;

        tCIDLib::TVoid SetEvPauseState
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelPath
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const tCIDLib::TBoolean       bState
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid SetPeriodicEvTime
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const tCIDLib::TEncodedTime   enctStart
            , const tCIDLib::TCard4         c4Period
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid SetScheduledEvTime
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const tCIDLib::TCard4         c4Day
            , const tCIDLib::TCard4         c4Hour
            , const tCIDLib::TCard4         c4Min
            , const tCIDLib::TCard4         c4Mask
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid SetSunBasedEvOffset
        (
            const   TString&                strRelPath
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const tCIDLib::TInt4          i4Offset
            , const TCQCSecToken&           sectUser
        );

        tCQCEvCl::TEventSrvProxy orbcEventSrvProxy
        (
            const   tCIDLib::TCard4         c4WaitUpTo = 0
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCEvCl,TFacility)
};

#pragma CIDLIB_POPPACK


