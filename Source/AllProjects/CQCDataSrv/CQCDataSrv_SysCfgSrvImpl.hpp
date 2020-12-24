//
// FILE NAME: CQCDataSrv_SysCfgSrvImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/29/2002
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
//  This is the header for the CQCDataSrv_SysCfgSrvImpl.cpp file, which provides an
//  implementation of a simple interface to get/set some system configuration data that
//  this server stores. And also used by the CQC Voice client to get his data, and he also
//  uses the system configuration data. Those are just raw file reads, never writes. They
//  are shipped files.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCSysCfgSrvImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TCQCSysCfgSrvImpl : public TCQCSysCfgServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSysCfgSrvImpl();

        TCQCSysCfgSrvImpl(const TCQCSysCfgSrvImpl&) = delete;
        TCQCSysCfgSrvImpl(TCQCSysCfgSrvImpl&&) = delete;

        ~TCQCSysCfgSrvImpl();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSysCfgSrvImpl& operator=(const TCQCSysCfgSrvImpl&) = delete;
        TCQCSysCfgSrvImpl& operator=(TCQCSysCfgSrvImpl&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryRoomCfg
        (
                    tCIDLib::TCard4&        c4SerialNum
            ,       TCQCSysCfg&             scfgToFill
        )   override;

        tCIDLib::TVoid StoreRoomCfg
        (
            const   TCQCSysCfg&             scfgToStore
            ,       tCIDLib::TCard4&        c4NewSerialNum
            , const TCQCSecToken&           sectUser
        )   override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_scfgData
        //      The system configuration data. We load it up when we are initialized
        //      if it exists (else we just leave it defaulted.) This is a write-through
        //      when clients write new data (though we recreate the correct serial number
        //      ignoring the one in the sent data.)
        //
        //  m_mtxSync
        //      Everything happens though us, so we provide all of the sync
        //      that's required, both for internal invocation of macros and
        //      field writes, as well as macros invoked on behalf of incoming
        //      clients.
        // -------------------------------------------------------------------
        TCQCSysCfg  m_scfgData;
        TMutex      m_mtxSync;


        // -------------------------------------------------------------------
        //  Magic Intfs
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSysCfgSrvImpl, TCQCSysCfgServerBase)
};

#pragma CIDLIB_POPPACK

