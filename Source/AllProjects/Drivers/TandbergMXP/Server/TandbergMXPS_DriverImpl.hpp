//
// FILE NAME: TandbergMXPS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/31/2008
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
//   CLASS: TWeatherS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TTandbergMXPSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTandbergMXPSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TTandbergMXPSDriver();


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
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean&      bNewValue
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

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
        );

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TTandbergMXPSDriver();
        TTandbergMXPSDriver(const TTandbergMXPSDriver&);
        tCIDLib::TVoid operator=(const TTandbergMXPSDriver&);


        // -------------------------------------------------------------------
        //  Private data members
        //
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_RemoteKey;
        TCommPortCfg        m_cpcfgSerial;
        THeapBuf            m_mbufOut;
        TCommPortBase*      m_pcommMXP;
        TString             m_strPort;
        TUSASCIIConverter   m_tcvtIO;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTandbergMXPSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


