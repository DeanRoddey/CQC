//
// FILE NAME: VantageQS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2006
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
//  This is the header file for the driver implementation class. We derive
//  from the base server side class and override callbacks.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TVantageQS
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class TVantageQSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TVantageQSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TVantageQSDriver();



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

        tCQCKit::ECommResults eIntFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4NewValue
        );

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strValue
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TVantageQSDriver();
        TVantageQSDriver(const TVantageQSDriver&);
        tCIDLib::TVoid operator=(const TVantageQSDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetMsg
        (
            const   tCIDLib::TCard4         c4Timeout
            , const tCIDLib::TBoolean       bThrowIfNot
            ,       TString&                strToFill
        );

        tCIDLib::TCard4 c4BreakOutReply
        (
            const   TString&                strReply
            ,       tCIDLib::TStrList&      colTokens
            , const TString&                strSepChars
            , const tCIDLib::TCard4         c4ExpectedToks
        );

        tCIDLib::TCard4 c4ProcessFloorCfg
        (
            const   tCIDLib::TStrList&      colConfig
            , const tCIDLib::TCard4         c4Index
        );

        tCIDLib::TVoid HandleMsg
        (
            const   TString&                strMsg
        );

        tCIDLib::TVoid ProbeConfig();

        tCIDLib::TVoid SendMsg
        (
            const   TString&                strToSend
        );

        tCIDLib::TVoid WaitForMsg
        (
            const   tCIDLib::TCard4         c4Timeout
            , const TString&                strExpRep
            ,       TString&                strToFill
        );

        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cpcfgSerial
        //      The comm port configuration that we were given upon creation.
        //      We use this to configure our serial connection.
        //
        //  m_mbufWrite
        //      A buffer for transcoding outgoing messages into before we send
        //      them.
        //
        //  m_pcommVantage
        //      The comm port via which we talk to the Vantage.
        //
        //  m_strPort
        //      The serial port we were told to configuration ourself on.
        //
        //  m_strSepComma
        //  m_strSepSpace
        //      We have to break out parameters from replies and these are the
        //      command separator character sets we use, to avoid creating
        //      them over and over.
        // -------------------------------------------------------------------
        TCommPortCfg        m_cpcfgSerial;
        THeapBuf            m_mbufWrite;
        TCommPortBase*      m_pcommVantage;
        TString             m_strPort;
        const TString       m_strSepComma;
        const TString       m_strSepSpace;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TVantageQSDriver,TCQCServerBase)
};


#pragma CIDLIB_POPPACK


