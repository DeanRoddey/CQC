//
// FILE NAME: CQCTerminal_ThisFacility.Cpp
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
//  This is the implementation file for the facility object. Most all of the functionality
//  is implemented here.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CQCTerminal.hpp"


// ----------------------------------------------------------------------------
//  Magic macros
// ----------------------------------------------------------------------------
RTTIDecls(TFacCQCTerminal,TFacility)


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCTerminal
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCTerminal: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCTerminal::TFacCQCTerminal() :

    TFacility
    (
        L"CQCTerminal"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::None
    )
    , m_bExitFlag(kCIDLib::False)
    , m_bUseRemotePorts(kCIDLib::False)
    , m_conIn()
    , m_eConnType(tCQCTerminal::EConnTypes::Unknown)
    , m_eLineEnd(tCQCTerminal::ELineEnds::CRLF)
    , m_mbufIO(256U, 256U)
    , m_pcommTar(nullptr)
    , m_pfactRemote(nullptr)
    , m_psockTar(nullptr)
{
}

TFacCQCTerminal::~TFacCQCTerminal()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCTerminal: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TFacCQCTerminal::bHandleSignal(const tCIDLib::ESignals eSig)
{
    //
    //  If it's a Ctrl-C or Ctrl-Break, just set the exit flag to make our main
    //  loop fall out.
    //
    if ((eSig == tCIDLib::ESignals::CtrlC) || (eSig == tCIDLib::ESignals::Break))
        m_bExitFlag = kCIDLib::True;

    // And return that we handled it
    return kCIDLib::True;
}

// ---------------------------------------------------------------------------
//  TFacCQCTerminal: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes TFacCQCTerminal::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    // Register ourself as a signal handler
    TSignals::c4AddHandler(this);

    try
    {
        m_conOut    << L"\nCQCTerminal.Exe\n"
                       L"Charmed Quark Simple Terminal Program\n"
                    << kCIDLib::EndLn;

        //
        //  Do some initialization.
        //
        //  Amongst other things, this will get login information from the user and
        //  set up a security token as well. If it does so and it's via command line
        //  parameters, it will remove those so that we don't see them below. If not
        //  it will look for environmental user info. If that isn't there or fails,
        //  it will return false.
        //
        if (!bDoBasicInit())
            return tCIDLib::EExitCodes::InitFailed;

        //
        //  We have a variable number of parameters, so we have to check for various
        //  scenarios. First, check for some special case scenarios where we have a
        //  single parameter (plus the default first one.)
        //
        const tCIDLib::TCard4 c4PCount = TSysInfo::c4CmdLineParmCount();
        if (c4PCount == 1)
        {
            if (TSysInfo::strCmdLineParmAt(0).bCompareI(L"/ShowComPorts"))
            {
                // We need to query and dump out all of the available ports
                ShowCommPorts();
                return tCIDLib::EExitCodes::Normal;
            }

            // Can't be any valid scenario
            ShowUsage();
            return tCIDLib::EExitCodes::BadParameters;
        }


        // If not a special case, we have to have at least 3
        if (c4PCount < 3)
        {
            ShowUsage();
            return tCIDLib::EExitCodes::BadParameters;
        }

        TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
        TString strConnType = *cursParms++;;
        if (strConnType.bCompareI(L"/Socket"))
            m_eConnType = tCQCTerminal::EConnTypes::Socket;
        else if (strConnType.bCompareI(L"/Serial"))
            m_eConnType = tCQCTerminal::EConnTypes::Serial;

        // If socket we have to get the IP address/host name and IP port
        if (m_eConnType == tCQCTerminal::EConnTypes::Socket)
        {
            TString strTarAddr = *cursParms++;
            TString strIPPort = *cursParms++;

            // Make sure the port is a reasonable number
            tCIDLib::TIPPortNum ippnTar;
            if (!strIPPort.bToCard4(ippnTar, tCIDLib::ERadices::Dec))
            {
                m_conOut    << L"\nThe target port must be a number from 1 to 64K"
                            << kCIDLib::NewEndLn;
                return tCIDLib::EExitCodes::BadParameters;
            }

            //
            //  And let's try to create our stream socket from this. If we cannot
            //  resolve the address or connect to the port, we'll give up.
            //
            m_conOut << L"Connecting to port " << ippnTar << L" on host "
                     << strTarAddr << L"..." << kCIDLib::NewLn;
            try
            {
                TIPEndPoint ipepTar
                (
                    strTarAddr, ippnTar, tCIDSock::EAddrTypes::Unspec
                );
                m_psockTar = new TClientStreamSocket
                (
                    tCIDSock::ESockProtos::TCP, ipepTar
                );
                m_conOut << L"Connected successfullly" << kCIDLib::NewLn;
            }

            catch(TError& errToCatch)
            {
                m_conOut    << L"\n\nFailed to connect to the target port\n";

                {
                    TStreamIndentJan janIndent(&m_conOut, 8);
                    m_conOut << errToCatch << kCIDLib::NewLn;
                }

                m_conOut << kCIDLib::NewEndLn;
                return tCIDLib::EExitCodes::InitFailed;
            }
        }
         else
        {
            // We have to get the comm port and baud rate
            TString strCommPort = *cursParms++;
            TString strBaud = *cursParms++;

            tCIDLib::TCard4 c4Baud;
            if (!strBaud.bToCard4(c4Baud, tCIDLib::ERadices::Dec))
            {
                m_conOut    << L"The baud rate must be a number" << kCIDLib::NewEndLn;
                return tCIDLib::EExitCodes::BadParameters;
            }

            try
            {
                // Create the port and open it
                m_pcommTar = facCIDComm().pcommMakeNew(strCommPort);
                m_pcommTar->Open(tCIDComm::EOpenFlags::WriteThrough);

                TCommPortCfg cpcfgToSet
                (
                    c4Baud
                    , tCIDComm::EFlags::None
                    , tCIDComm::EParities::None
                    , tCIDComm::EDataBits::Eight
                    , tCIDComm::EStopBits::One
                    , tCIDComm::EPortDTR::Disable
                    , tCIDComm::EPortRTS::Disable
                );
                m_pcommTar->SetCfg(cpcfgToSet);
            }

            catch(TError& errToCatch)
            {
                m_conOut    << L"\n\nFailed to open the target port\n";
                {
                    TStreamIndentJan janIndent(&m_conOut, 8);
                    m_conOut << errToCatch << kCIDLib::NewLn;
                }
                m_conOut << kCIDLib::NewEndLn;
                return tCIDLib::EExitCodes::InitFailed;
            }
        }

        // Get any optional parameters
        TString strEncoding(L"US-ASCII");
        TString strLineEnd(L"CRLF");

        // Create the transcoder
        m_ptcvtIO = facCIDEncode().ptcvtMake(strEncoding);
        if (!m_ptcvtIO)
        {
            m_conOut    << L"\n\n'" << strEncoding << L"' is not a valid encoding"
                        << kCIDLib::NewEndLn;
            return tCIDLib::EExitCodes::BadParameters;
        }

        //
        //  Now enter a loop where were wait for bytes from the target or keystrokes
        //  from the user.
        //
        tCIDLib::EConKeys eKeyType;
        TString strCurCmd;
        while (!m_bExitFlag)
        {
            // Wait a short period of time for some remote and process it if so
            ProcessInput();

            // Wait for a keystroke if any
            tCIDLib::TCh chCur = m_conIn.chReadChar(eKeyType, 25);
            if (eKeyType == tCIDLib::EConKeys::Backspace)
            {
                if (strCurCmd.c4Length())
                    strCurCmd.DeleteLast();
                m_conOut.bBackspace();
            }
             else if (eKeyType == tCIDLib::EConKeys::Break)
            {
                m_bExitFlag = kCIDLib::True;
            }
             else if (eKeyType == tCIDLib::EConKeys::Char)
            {
                m_conOut << chCur << kCIDLib::FlushIt;

                // Append it to our current command
                strCurCmd.Append(chCur);
            }
             else if (eKeyType == tCIDLib::EConKeys::Enter)
            {
                //
                //  Send the current command line plus configured line end. Make
                //  sure we get the current command cleared first in case of
                //  error.
                //
                TString strToSend(strCurCmd);
                strCurCmd.Clear();
                SendCmd(strToSend);

                m_conOut << kCIDLib::EndLn;
            }
        }
    }

    // Catch any CIDLib runtime errors
    catch(TError& errToCatch)
    {
        m_conOut << L"\nA CIDLib runtime error occured during processing.\nError: \n";
        {
            TStreamIndentJan janIndent(&m_conOut, 4);
            m_conOut << errToCatch.strErrText() << kCIDLib::NewLn;
        }
        return tCIDLib::EExitCodes::RuntimeError;
    }

    //
    //  Kernel errors should never propogate out of CIDLib, but I test
    //  for them in my test programs so I can catch them if they do
    //  and fix them.
    //
    catch(const TKrnlError& kerrToCatch)
    {
        m_conOut    << L"A kernel error occured during processing.\nError: "
                    << kerrToCatch.errcId() << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::RuntimeError;
    }

    // Catch a general exception
    catch(...)
    {
        m_conOut    << L"A general exception occured during processing"
                    << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::SystemException;
    }

    return tCIDLib::EExitCodes::Normal;
}


// ---------------------------------------------------------------------------
//  Private, non-virtual methods
// ---------------------------------------------------------------------------


tCIDLib::TBoolean TFacCQCTerminal::bDoBasicInit()
{
    try
    {
        //
        //  Ask CQCKit to load core environment/parm stuff. If we can't do this,
        //  then we are doomed and just have to exit.
        //
        TString strFailReason;
        if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
        {
            m_conOut    << L"\n\nCould not get environment info.\n    Reason: "
                        << strFailReason << kCIDLib::NewEndLn;
            return kCIDLib::False;
        }

        //
        //  See if we have /User and /Password parameters. If so, get their values and
        //  remove them so the main logic above doesn't have to deal with them.
        //
        TString strUser;
        TString strPassword;
        TSysInfo::bFindCmdLinePref(L"/User=", strUser, kCIDLib::False);
        TSysInfo::bFindCmdLinePref(L"/Password=", strPassword, kCIDLib::False);

        if (strUser.bIsEmpty() || strPassword.bIsEmpty())
        {
            // See if we have environmental info
            strUser.Clear();
            strPassword.Clear();
            TProcEnvironment::bFind(L"CQC_USERNAME", strUser);
            TProcEnvironment::bFind(L"CQC_PASSWORD", strPassword);
        }

        if (strUser.bIsEmpty() || strPassword.bIsEmpty())
        {
            m_conOut    << L"\nNo login information was provided. Either provide /User= and /Password="
                           L" parameters or set the CQC_USERNAME and CQC_PASSWORD environment variables"
                        << kCIDLib::NewEndLn;
            return kCIDLib::False;
        }

        // Try to log in
        tCQCKit::ELoginRes eRes = tCQCKit::ELoginRes::Count;
        TCQCSecToken sectUser;
        TCQCUserAccount uaccUser;
        if (!facCQCKit().bDoLogin(strUser, strPassword, sectUser, uaccUser, eRes))
        {
            m_conOut    << L"\nInvalid user name or password" << kCIDLib::NewEndLn;
            return kCIDLib::False;
        }

        // Initialize the client side ORB
        facCIDOrb().InitClient();

        //
        //  Do an update of the port factories we support which will get us
        //  set up for using any virtual ports.
        //
        facGC100Ser().bUpdateFactory(sectUser);
        facJAPwrSer().bUpdateFactory(sectUser);

        // If they provided remote port server info, set that up
        if (m_bUseRemotePorts)
        {
            m_pfactRemote = new TCQCRemSrvPortFactory(m_ipepRemPort);
            facCIDComm().RegisterFactory(m_pfactRemote);
        }
    }

    catch(TError& errToCatch)
    {
        m_conOut << L"\n\nInitialization failed. Error=\n";
        TStreamIndentJan janIndent(&m_conOut, 8);
        m_conOut << errToCatch << kCIDLib::NewEndLn;
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


// Wait a short while for some input from the socket or comm port
tCIDLib::TVoid TFacCQCTerminal::ProcessInput()
{
    tCIDLib::TCard4 c4Count = 0;
    if (m_eConnType == tCQCTerminal::EConnTypes::Socket)
    {
        c4Count = m_psockTar->c4ReceiveMBufTOMS
        (
            m_mbufIO, 25, 16, tCIDLib::EAllData::OkIfNotAll
        );
    }
     else
    {
        c4Count = m_pcommTar->c4ReadMBufMS(m_mbufIO, 16, 25);
    }

    // If we got any, then process it
    if (c4Count)
    {
        m_ptcvtIO->c4ConvertFrom(m_mbufIO, c4Count, m_strInput);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const tCIDLib::TCh chCur = m_strInput[c4Index];
            if (chCur == kCIDLib::chCR)
                m_conOut << kCIDLib::EndLn;
            else if (chCur != kCIDLib::chLF)
                m_conOut << chCur << kCIDLib::FlushIt;
        }
    }
}


// Send a character to the other side
tCIDLib::TVoid TFacCQCTerminal::SendCmd(const TString& strToSend)
{
    // Transcode the text to send
    tCIDLib::TCard4 c4Bytes = 0;
    m_ptcvtIO->c4ConvertTo(strToSend, m_mbufIO, c4Bytes);

    // Add the appropriate line end
    switch(m_eLineEnd)
    {
        case tCQCTerminal::ELineEnds::CR :
            m_mbufIO.PutCard1(13, c4Bytes++);
            break;

        case tCQCTerminal::ELineEnds::CRLF :
            m_mbufIO.PutCard1(13, c4Bytes++);
            m_mbufIO.PutCard1(10, c4Bytes++);
            break;

        case tCQCTerminal::ELineEnds::LF :
            m_mbufIO.PutCard1(10, c4Bytes++);
            break;

        case tCQCTerminal::ELineEnds::LFCR :
            m_mbufIO.PutCard1(10, c4Bytes++);
            m_mbufIO.PutCard1(13, c4Bytes++);
            break;

        default :
            break;
    };

    if (m_eConnType == tCQCTerminal::EConnTypes::Socket)
        m_psockTar->Send(m_mbufIO, c4Bytes);
    else
        m_pcommTar->c4WriteMBufMS(m_mbufIO, c4Bytes, 1000);
}


//
//  We query the available ports and display them. The second parm is whether to
//  only show local ports. If remote ports are not enabled, we only want to try to
//  show local ports.
//
tCIDLib::TVoid TFacCQCTerminal::ShowCommPorts()
{
    tCIDLib::TStrList colPorts;
    if (facCIDComm().bQueryPorts(colPorts, kCIDLib::False, kCIDLib::True))
    {
        m_conOut << L"\nThe available ports are:\n";
        TStreamIndentJan janIndent(&m_conOut, 8);
        const tCIDLib::TCard4 c4Count = colPorts.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            m_conOut << colPorts[c4Index] << kCIDLib::NewLn;
        m_conOut << kCIDLib::EndLn;
    }
     else
    {
        m_conOut << L"\n\nAn error occurred while querying availble ports\n";
    }
}


tCIDLib::TVoid TFacCQCTerminal::ShowUsage()
{
    m_conOut    << L"Parameters:\n"
                   L"    CQCTerminal /Socket [addr] [port] [options]\n"
                   L"    CQCTerminal /Serial [port] [baud] [options]\n"
                   L"    CQCTerminal /ShowComPorts\n"

                   L"\n        Options:\n"
                   L"           /LEnd=[CR|CRLF|LF|LFCR]     - Line end type (CRLF)\n"

                << kCIDLib::EndLn;
}
