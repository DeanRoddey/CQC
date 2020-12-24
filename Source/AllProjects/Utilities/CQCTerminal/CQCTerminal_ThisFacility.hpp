//
// FILE NAME: CQCTerminal_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/18/2018
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
//  This is the header for the facility class for this program. All the functionality
//  is implemented here.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCTerminal
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCTerminal : public TFacility, public MSignalHandler
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCTerminal();

        TFacCQCTerminal(const TFacCQCTerminal&) = delete;
        TFacCQCTerminal(TFacCQCTerminal&&) = delete;

        ~TFacCQCTerminal();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCTerminal& operator=(const TFacCQCTerminal&) = delete;
        TFacCQCTerminal& operator=(TFacCQCTerminal&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHandleSignal
        (
            const   tCIDLib::ESignals       eSignal
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoBasicInit();

        tCIDLib::TVoid ProcessInput();

        tCIDLib::TVoid SendCmd
        (
            const   TString&                strToSend
        );

        tCIDLib::TVoid ShowCommPorts();

        tCIDLib::TVoid ShowUsage();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bExitFlag
        //      Our signal handler sets this and the main loop uses it as a loop
        //      flag.
        //
        //  m_bUseRemotePorts
        //      The user can provide us with info to contact one of our remote port
        //      servers so that we can also use those ports. If so, this is set and
        //      the m_ipepRemSrv member is valid, and we will use that to set up the
        //      remote port factory.
        //
        //  m_conIn
        //  m_conOut
        //      We are an interactive console program so we need in and out consoles.
        //
        //  m_eConnType
        //      The type of connection, as driven by user options.
        //
        //  m_eLineEnd
        //      The type of line ends to send with outgoing commands
        //
        //  m_ipepRemPort
        //      If doing remote ports, this is the remote port end point.
        //
        //  m_mbufIO
        //      Used to transcode the typed in commands to bytes to send and incoming
        //      bytes to characters for display.
        //
        //  m_pcommTar
        //      If doing comm ports, we create a comm port of some sort, so we store
        //      it via the base port type. It could be a real one or a virtual CQC
        //      port.
        //
        //  m_pfactRemote
        //      A pointer to a remote port factory which lets us use remote ports via
        //      our remote port server as well as local and local system virtual ports.
        //      If this isn't null, then we know we have to do remote ports later on.
        //
        //  m_psockTar
        //      If socket based we create a client stream socket.
        //
        //  m_ptcvtIO
        //      We create a text converter based on user options (defaults to ASCII)
        //      to do in/out transcoding.
        //
        //  m_strInput
        //      Used by the ProcessInput method to avoid constantly recreating one.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bExitFlag;
        tCIDLib::TBoolean           m_bUseRemotePorts;
        TInConsole                  m_conIn;
        TOutConsole                 m_conOut;
        tCQCTerminal::EConnTypes    m_eConnType;
        tCQCTerminal::ELineEnds     m_eLineEnd;
        TIPEndPoint                 m_ipepRemPort;
        THeapBuf                    m_mbufIO;
        TCommPortBase*              m_pcommTar;
        TCQCRemSrvPortFactory*      m_pfactRemote;
        TClientStreamSocket*        m_psockTar;
        TTextConverter*             m_ptcvtIO;
        TString                     m_strInput;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCTerminal,TFacility)
};

