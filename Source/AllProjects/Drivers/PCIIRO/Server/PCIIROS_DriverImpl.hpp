//
// FILE NAME: PCIIROS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/09/2006
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
//   CLASS: TPCIIROSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TPCIIROSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TPCIIROSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TPCIIROSDriver();


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

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TPCIIROSDriver();
        TPCIIROSDriver(const TPCIIROSDriver&);
        tCIDLib::TVoid operator=(const TPCIIROSDriver&);


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        tCIDLib::TVoid PollBits
        (
            const   tCIDLib::TBoolean       bForce
        );

        tCIDLib::TVoid StoreBits
        (
            const   tCIDLib::TCard1         c1Bits
            , const tCIDLib::TCard4*        pac4FldIds
            , const tCIDLib::TCard4         c4StartInd
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_ac4Fld_DigIns
        //  m_ac4Fld_Contacts
        //      These are used to hold the field ids for the digital inputs
        //      and contact closures. We allocate them large enough to hold
        //      the worst case of 16, but only 8 might be used. m_c4Count
        //      tells us that.
        //
        //  m_c1HighContact
        //  m_c1HighDig
        //  m_c1LowContact
        //  m_c1LowDig
        //      We can reduce a lot of driver overhead by just remembering
        //      the last set of bits we saw, and not bothering to write to
        //      the fields unless these values change. We don't one forced
        //      update on connect, to insure the fields and these cached
        //      values are in sync. Then during polling we do the lighter
        //      version.
        //
        //  m_c4Count
        //      Indicates if this is an 8 or 16 type board. This tells us
        //      how many contacts/digitals we have and which port offsets to
        //      use.
        //
        //  m_c4BaseAddr
        //      The base address of the I/O board. We use this to know which
        //      registers to read/write.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_ac4Fld_DigIns[kPCIIROS::c4MaxPoints];
        tCIDLib::TCard4     m_ac4Fld_Contacts[kPCIIROS::c4MaxPoints];
        tCIDLib::TCard1     m_c1HighContact;
        tCIDLib::TCard1     m_c1HighDig;
        tCIDLib::TCard1     m_c1LowContact;
        tCIDLib::TCard1     m_c1LowDig;
        tCIDLib::TCard4     m_c4Count;
        tCIDLib::TCard4     m_c4BaseAddr;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TPCIIROSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK

