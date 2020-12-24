//
// FILE NAME: ZWaveUSB3Sh_CtrlIntf.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2017
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
//  In order for the code down here in the shared facility to send and receive
//  messages (while doing all of the things that need to happen in the process of
//  doing that) we need to access the server driver. But, he is up in the server
//  facility. So we define this mixin interface. The driver implements this and
//  passes it into any calls it makes to the unit related classes so that they can
//  interact with their associated Z-Wave unit.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TZWaveUnit;
class TZWOutMsg;

// ---------------------------------------------------------------------------
//   CLASS: MZWaveCtrlIntf
//  PREFIX: mzwci
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT MZWaveCtrlIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        MZWaveCtrlIntf(const MZWaveCtrlIntf&) = delete;

        ~MZWaveCtrlIntf() {}


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        MZWaveCtrlIntf& operator=(const MZWaveCtrlIntf&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bUSB3IsUniqueUnitName
        (
            const   TString&                strToCheck
        )   const = 0;

        virtual tCIDLib::TBoolean bUSB3QueryFldName
        (
            const   tCIDLib::TCard4         c4FldId
            ,       TString&                strToFill
        )   = 0;

        virtual tCIDLib::TBoolean bUSB3WaitForMsg
        (
            const   TZWaveUnit&             unitTar
            , const tZWaveUSB3Sh::EMsgTypes eType
            , const tCIDLib::TCard1         c1MsgId
            , const tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1CmdId
            , const tCIDLib::TEncodedTime   enctWaitFor
            ,       TZWInMsg&               zwimReply
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        ) = 0;

        virtual tCIDLib::TBoolean bUSB3WaitForMsg
        (
            const   tZWaveUSB3Sh::EMsgTypes eType
            , const tCIDLib::TCard1         c1UnitId
            , const tCIDLib::TCard1         c1MsgId
            , const tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1CmdId
            , const tCIDLib::TEncodedTime   enctWaitFor
            ,       TZWInMsg&               zwimReply
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        ) = 0;

        virtual tCIDLib::TBoolean bUSB3WriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bToStore
        ) = 0;

        virtual tCIDLib::TBoolean bUSB3WriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4ToStore
        ) = 0;

        virtual tCIDLib::TBoolean bUSB3WriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8ToStore
        ) = 0;

        virtual tCIDLib::TBoolean bUSB3WriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4ToStore
        ) = 0;

        virtual tCIDLib::TBoolean bUSB3WriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strToStore
        ) = 0;

        virtual tCIDLib::TCard4 c4USB3LookupFldId
        (
            const   TString&                strName
        )   const = 0;

        virtual const TString& strUSB3Moniker() const = 0;

        virtual tCIDLib::TVoid USB3QEventTrig
        (
            const   tCQCKit::EStdDrvEvs     eEvent
            , const TString&                strFld
            , const TString&                strVal1
            , const TString&                strVal2
            , const TString&                strVal3
            , const TString&                strVal4
        ) = 0;

        virtual tCIDLib::TVoid USB3Send
        (
            const   TZWOutMsg&              zwomToSend
        ) = 0;

        virtual tCIDLib::TVoid USB3SendCCMsg
        (
            const   TZWOutMsg&              zwomToSend
        ) = 0;

        virtual tCIDLib::TVoid USB3SendCCMsg
        (
            const   TZWOutMsg&              zwomToSend
            ,       TZWInMsg&               zwimRep
            , const tCIDLib::TCard4         c4WaitFor = 2500
        ) = 0;

        virtual tCIDLib::TVoid USB3SendCtrlRes
        (
            const   TZWOutMsg&              zwomToSend
            ,       TZWInMsg&               zwimRep
            , const tCIDLib::TCard4         c4WaitFor = 2000
        ) = 0;

        virtual tCIDLib::TVoid USB3SendUnitRes
        (
            const   TZWOutMsg&              zwomToSend
            ,       TZWInMsg&               zwimRep
            , const tCIDLib::TCard4         c4WaitFor = 2500
        ) = 0;

        virtual tCIDLib::TVoid USB3SendReq
        (
            const   TZWOutMsg&              zwomToSend
            ,       TZWInMsg&               zwimRep
            , const tCIDLib::TCard4         c4WaitFor = 2500
        ) = 0;

        virtual tCIDLib::TVoid USB3SetFieldErr
        (
            const   tCIDLib::TCard4         c4FldId
        ) = 0;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        MZWaveCtrlIntf() {}
};

#pragma CIDLIB_POPPACK

