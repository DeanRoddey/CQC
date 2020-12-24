//
// FILE NAME: ZWaveLevi2Sh_CmdQ.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/13/2014
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
//  This is the header file for the ZWaveLevi2Sh_CmdQ.cpp file, which implements
//  the TZWQCmd class. For Wakeup type devices, we can't just send commands when
//  they arrive, since the device likely is not awake. So they have to be queued
//  up and sent when a wakeup notification comes in. The TZWQCmd class holds the
//  info required to play out the command later.
//
//  Note we don't need to store the unit id since the msg is stored in the unit that
//  will later send it.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWQCmd
//  PREFIX: zwqc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWQCmd : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWQCmd();

        TZWQCmd
        (
            const   tCIDLib::TCard4         m_c4InstId
            , const tZWaveLevi2Sh::ECClasses m_eCmdClass
            , const tCIDLib::TCardList      m_fcolBytes
        );

        TZWQCmd
        (
            const   TZWQCmd&                zwqcSrc
        );

        ~TZWQCmd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWQCmd& operator=
        (
            const   TZWQCmd&                zwqcSrc
        );


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_c4InstId
        //      The instance id to target.
        //
        //  m_eCmdClass
        //      The command class to target. This plus the instance id gets us to the
        //      right command class instance.
        //
        //  m_fcolBytes
        //      The actual message bytes to send.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4InstId;
        tZWaveLevi2Sh::ECClasses    m_eCmdClass;
        tCIDLib::TCardList          m_fcolBytes;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWQCmd,TObject)
};

#pragma CIDLIB_POPPACK


