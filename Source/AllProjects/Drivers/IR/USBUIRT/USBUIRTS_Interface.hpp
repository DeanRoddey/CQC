//
// FILE NAME: USBUIRTS_BlasterImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/12/2003
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
//  This is the header file for our derivative of the generic IR blaster
//  ORB interface that lets the generic IR client control the USB-UIRT.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


extern "C"
{
    // A small structure that the UUIRT returns
    struct TUIRTInfo
    {
        tCIDLib::TCard4 c4FWVersion;
        tCIDLib::TCard4 c4ProtVersion;
        tCIDLib::TCard1 c1Day;
        tCIDLib::TCard1 c1Month;
        tCIDLib::TCard1 c1Year;
    };


    // A handle for the UUIRT
    typedef tCIDLib::TVoid** TUUHandle;


    // Prototypes for some callbacks the UUIRT makes to us
    typedef tCIDLib::TVoid (WNDPROCAPI* TCallbackProc)
    (
        const   tCIDLib::TSCh* const    pszIREventStr
        ,       tCIDLib::TVoid* const   pUserData
    );

    typedef tCIDLib::TVoid (WNDPROCAPI* TLearnCallbackProc)
    (
        const   tCIDLib::TCard4         c4Progress
        , const tCIDLib::TCard4         c4SigQuality
        , const tCIDLib::TCard4         c4CarrierFreq
        ,       tCIDLib::TVoid* const   c4UserData
    );


    //
    //  Typedefs for the APIs that we load up dynamically to get function
    //  pointers that we can call.
    //
    typedef TUUHandle (WNDPROCAPI* TUUIRTOpen)();

    typedef tCIDLib::TSInt (WNDPROCAPI* TUUIRTClose)
    (
                TUUHandle               hHandle
    );

    typedef tCIDLib::TSInt (WNDPROCAPI* TUUIRTGetDrvInfo)
    (
                tCIDLib::TCard4* const  pc4DrvVersion
    );

    typedef tCIDLib::TSInt (WNDPROCAPI* TUUIRTGetUUIRTInfo)
    (
                TUUHandle               hHandle
        ,       TUIRTInfo* const        pInfo
    );

    typedef tCIDLib::TSInt (WNDPROCAPI* TUUIRTGetUUIRTConfig)
    (
                TUUHandle               hHandle
        ,       tCIDLib::TCard4* const  pc4Config
    );

    typedef tCIDLib::TSInt (WNDPROCAPI* TUUIRTSetUUIRTConfig)
    (
                TUUHandle               hHandle
        , const tCIDLib::TCard4         c4Config
    );

    typedef tCIDLib::TSInt (WNDPROCAPI* TUUIRTSetRecCB)
    (
                TUUHandle               hHandle
        ,       TCallbackProc           pfnReceiveProc
        ,       tCIDLib::TVoid* const   pUserData
    );

    typedef tCIDLib::TSInt (WNDPROCAPI* TUUIRTTransmitIR)
    (
                TUUHandle               hHandle
        , const tCIDLib::TSCh* const    pszIRCode
        , const tCIDLib::TInt4          i4CodeFormat
        , const tCIDLib::TInt4          i4RepeatCount
        , const tCIDLib::TInt4          i4InactivityWaitTime
        ,       tCIDLib::TVoid* const   hEvent
        , const tCIDLib::TVoid* const   pReserved0
        , const tCIDLib::TVoid* const   pReserved1
    );

    typedef tCIDLib::TSInt (WNDPROCAPI* TUUIRTLearnIR)
    (
                TUUHandle               hHandle
        , const tCIDLib::TCard4         c4CodeFormat
        ,       tCIDLib::TSCh* const    pszIRCode
        ,       TLearnCallbackProc      pfnProgressProc
        ,       tCIDLib::TVoid* const   pUserData
        ,       tCIDLib::TCard4* const  pc4Abort
        , const tCIDLib::TCard4         c4Param1
        , const tCIDLib::TVoid* const   pReserved0
        , const tCIDLib::TVoid* const   pReserved1
    );
}


//
//  UUIRT related constants
//
namespace kUSBUIRT
{
    const tCIDLib::TCard4   c4Cfg_LEDRX         = 0x0001;
    const tCIDLib::TCard4   c4Cfg_LEDTX         = 0x0002;
    const tCIDLib::TCard4   c4Cfg_LegacyRX      = 0x0004;

    const tCIDLib::TCard4   c4IRFmt_UUIRT       = 0x0000;
    const tCIDLib::TCard4   c4IRFmt_Pronto      = 0x0010;

    const tCIDLib::TCard4   c4Learn_ForceRaw    = 0x0100;
    const tCIDLib::TCard4   c4Learn_ForceStruc  = 0x0200;
    const tCIDLib::TCard4   c4Learn_ForceFreq   = 0x0400;
    const tCIDLib::TCard4   c4Learn_FreqDetect  = 0x0800;

    const tCIDLib::TCard4   c4Err_NoDevice      = 0x20000001;
    const tCIDLib::TCard4   c4Err_NoResp        = 0x20000002;
    const tCIDLib::TCard4   c4Err_NoDLL         = 0x20000003;
    const tCIDLib::TCard4   c4Err_Version       = 0x20000004;

    const TUUHandle         hInvalid            = (TUUHandle)kCIDLib::c4MaxCard;
}

