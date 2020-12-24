//
// FILE NAME: ZWaveUSBSh_Ctrl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/06/2007
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
//  This is the header file for the ZWaveUSBS_Ctrl.cpp file, which
//  implements the TZWaveCtrl class. Because we have abstracted out the
//  unit classes, so that we can plug in various types of units over time
//  without changing the main driver, we have to have some way for them to
//  access the USB controller so that they can send/receive messages.
//
//  But, the driver really needs to provide this functionality for various
//  reasons. So, we create this mixin class that defines the interfaces
//  required and the driver mixes it in. Then the driver can pass itself
//  in to the unit classes, via this mixin class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveCtrl
//  PREFIX: zwctrl
// ---------------------------------------------------------------------------
class ZWUSBSHEXPORT MZWaveCtrl
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~MZWaveCtrl();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TCard1 c1NextCallbackId() = 0;

        virtual tCIDLib::TCard4 c4SendAndWaitForReply
        (
            const   TMemBuf&                mbufOut
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard4         c4ExpRepLen
            ,       TMemBuf&                mbufIn
        ) = 0;

        virtual tCIDLib::TCard4 c4SendReqAndWaitForReply
        (
                    tCIDLib::TCard1         c1Req
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard4         c4ExpRepLen
            ,       TMemBuf&                mbufIn
        ) = 0;

        virtual tCIDLib::TCard4 c4SendReqAndWaitForReply
        (
                    tCIDLib::TCard1         c1Req
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard4         c4ExpRepLen
            , const tCIDLib::TCard1         c1Parm
            ,       TMemBuf&                mbufIn
        ) = 0;

        virtual tCIDLib::TCard4 c4SendReqAndWaitForReply
        (
                    tCIDLib::TCard1         c1Req
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard4         c4ExpRepLen
            , const tCIDLib::TCard1         c1Parm
            , const tCIDLib::TCard1         c1Parm2
            ,       TMemBuf&                mbufIn
        ) = 0;

        virtual tCIDLib::TCard4 c4WaitForReply
        (
            const   tCIDLib::TCard1         c1Expected
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard4         c4ExpRepLen
            , const tCIDLib::TBoolean       bCallback
            ,       TMemBuf&                mbufToFill
        ) = 0;

        virtual THeapBufPool& splBufs() = 0;

        virtual tCIDLib::TVoid SendAndWaitForAck
        (
            const   TMemBuf&                mbufToSend
        ) = 0;

        virtual tCIDLib::TVoid SendData
        (
            const   TMemBuf&                mbufToSend
            , const tCIDLib::TBoolean       bGetAck = kCIDLib::True
        ) = 0;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        MZWaveCtrl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        MZWaveCtrl(const MZWaveCtrl&);
        tCIDLib::TVoid operator=(const MZWaveCtrl&);
};

#pragma CIDLIB_POPPACK

