//
// FILE NAME: CQCServer_DriverInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/30/2001
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
//  This file implements a trivial class used to hold the info we need to
//  track about each driver at runtime. We keep up with the driver object
//  itself, and the facility object that was loaded for the driver. These
//  are the things that need to get cleaned up to drop a driver.
//
//  We provide some helper methods as well. StartShutdown will set the
//  shutdown request on the driver, starting it down. It does a non-syncing
//  thread shutdown request, so that it never blocks. The bIsDead method is
//  used to wait for the thread to actually end.
//
//  When a driver is fully down, DropDriver is called to clean up the driver
//  and facility object, then this object can be destroyed.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TServerDriverInfo
//  PREFIX: sdi
// ---------------------------------------------------------------------------
class TServerDriverInfo : public TObject
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TServerDriverInfo() = delete;

        TServerDriverInfo
        (
                    TFacility* const        pfacDriver
            ,       TCQCServerBase* const   psdrvDriver
        );

        TServerDriverInfo(const TServerDriverInfo&) = delete;

        ~TServerDriverInfo();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TServerDriverInfo& operator=(const TServerDriverInfo&) = delete;


        // --------------------------------------------------------------------
        //  Public, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bIsDead
        (
            const   tCIDLib::TCard4         c4WaitMS
        );

        tCIDLib::TVoid DropDriver();

        const TFacility& facDriver() const;

        TFacility& facDriver();

        const TCQCServerBase& sdrvDriver() const;

        TCQCServerBase& sdrvDriver();

        tCIDLib::TVoid StartShutdown();


    private :
        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_pfacDriver
        //      The facility object of the driver, stored via the base
        //      TFacilty class. This was loaded by the driver itself, and given
        //      back to us. It uses the 'load' option, so that the facility's
        //      ref count is bumped for each driver that uses it.
        //
        //  m_psdrvDriver
        //      This is the driver object, stored via its common CQC server
        //      base class.
        // --------------------------------------------------------------------
        TFacility*          m_pfacDriver;
        TCQCServerBase*     m_psdrvDriver;


        // --------------------------------------------------------------------
        //  Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TServerDriverInfo,TObject)
};

#pragma CIDLIB_POPPACK


