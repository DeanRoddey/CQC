//
// FILE NAME: GenLTCPOutS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/23/2013
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
//  This is the header file for the actual driver class. This one is pretty
//  simple. It's unusual in that it's a listening driver. We get a port to
//  listen on and accept an incoming connection. We just have a couple fields
//  and we accept binary or text content which we format out to a buffer
//  and send through the port.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TGenLTCPOutSDriver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class TGenLTCPOutSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenLTCPOutSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TGenLTCPOutSDriver();


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

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TGenLTCPOutSDriver(const TGenLTCPOutSDriver&);
        tCIDLib::TVoid operator=(const TGenLTCPOutSDriver&);


        // -------------------------------------------------------------------
        //  Helpers for the two modes we support, to build up the messages and
        //  send them.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSendBinMsg
        (
            const   TString&                strToSend
        );

        tCIDLib::TBoolean bSendTextMsg
        (
            const   TString&                strToSend
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldIdXXX
        //      The field ids of our fields, which we look up after we
        //      register them, so that we can do 'by id' field access.
        //
        //  m_eDecor
        //      The text decoration that the user asked us to apply to out
        //      going text msgs.
        //
        //  m_ippnListen
        //      The port we were told to listen on. Since we aren't a normal
        //      IP based driver, we don't use the IP comm config stuff in the
        //      manifest. We just use a user prompt to let the user tell us
        //      the port.
        //
        //  m_mbufOut
        //  m_mbufTmp
        //      The output buffer we build up msgs in before we send them,
        //      and a temporary one for occasional use.
        //
        //  m_psocklTrig
        //      The socket listener we create to listen for connections. We
        //      set it up for just one connection at a time. We treat our
        //      get comm resource as creating this guy, so we won't come
        //      online until we get it created. We clean it up during driver
        //      termination.
        //
        //  m_psockTrig
        //      This socket we create when a client connects to us. It's
        //      created and destroyed as the client connects and disconnects.
        //
        //  m_ptcvtEncode
        //      A text encoder for the encoding they selected during driver
        //      installation.
        //
        //  m_strToken
        //      Some times for use while building up msgs
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4FldId_SendBinMsg;
        tCIDLib::TCard4         m_c4FldId_SendTextMsg;
        tGenLTCPOutS::EDecors   m_eDecor;
        tCIDLib::TIPPortNum     m_ippnListen;
        THeapBuf                m_mbufOut;
        THeapBuf                m_mbufTmp;
        TSocketListener*        m_psocklOut;
        TStreamSocket*          m_psockOut;
        TTextConverter*         m_ptcvtEncode;
        TString                 m_strToken;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenLTCPOutSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


