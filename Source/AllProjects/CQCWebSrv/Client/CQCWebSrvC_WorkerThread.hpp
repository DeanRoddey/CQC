//
// FILE NAME: CQCWebSrvC_WorkerThread.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/01/2005
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
//  This is the header for the class that implements the worker threasd of our thread
//  farm. We use a single thread per client in this server, in order to simplify things.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TWSEchoHandler;
class TWSCQCImgHandler;
class TWSCMLBinHandler;
class TWSFileHandler;


// ---------------------------------------------------------------------------
//   CLASS: TWorkerThread
//  PREFIX: thr
// ---------------------------------------------------------------------------
class TWorkerThread : public TThread
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructors
        // --------------------------------------------------------------------
        TWorkerThread() = delete;

        TWorkerThread
        (
            const   TString&                strName
        );

        TWorkerThread(const TWorkerThread&) = delete;
        TWorkerThread(TWorkerThread&&) = delete;

        ~TWorkerThread();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TWorkerThread& operator=(const TWorkerThread&) = delete;
        TWorkerThread& operator=(TWorkerThread&&) = delete;


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::EExitCodes eProcess() final;


    private :
        // --------------------------------------------------------------------
        //  Private, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TCard4 c4Authenticate
        (
            const   TString&                strURLPath
            , const TString&                strType
            ,       TString&                strUserName
            ,       tCQCKit::EUserRoles&    eRole
        );

        tCIDLib::TVoid GenerateNonce
        (
            const   TString&                strURL
            ,       TString&                strNonce
        );

        tCIDLib::TVoid SendAuthChallenge
        (
                    TCIDDataSrc&            cdsClient
            , const TURL&                   urlReq
            , const TString&                strType
            , const tCIDLib::TBoolean       bStale
        );

        tCIDLib::TVoid ServiceClient
        (
                    TCIDDataSrcJan&         janSrc
        );


        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_colInHdrLines
        //      The header lines parsed out of the incoming message, which
        //      are needed in various places during processing of the
        //      message.
        //
        //  m_colOutHdrLines
        //      The handler can provide outgoing header lines to send back
        //      out. So we keep this around to avoid constantly recreating
        //      one of these.
        //
        //  m_colValues
        //      We call into the URL class to parse out any GET/POST params
        //      so we can pass them in to the macro. It gives us back a C++
        //      level list of TKeyValuePair objects, which we then put into
        //      a CML level vector.
        //
        //  m_crypAuth
        //      We get some encrypted info from the security server during
        //      authorization, so to avoid the overhead we keep one of these
        //      arond.
        //
        //  m_httpcSrv
        //      We use a THTTPClient object for reading requests from the
        //      client. Though it's a client object, it has methods for reading
        //      in messages that we can use, and for sending error replies.
        //
        //  m_mbufData
        //  m_mbufData2
        //      Memory buffers for reading in data and for formatting data
        //      to go out, or scratch use. We generally format the header
        //      stuff the the m_strmOutput stream, and the body content into
        //      one of these buffers.
        //
        //  m_mdigAuth
        //  m_mhashAuth1
        //  m_mhashAuth2
        //      We support Digest authorization only, and have to generate
        //      some MD5 hashes.
        //
        //  m_purlhXXX
        //      URL handlers for the various types of retrieval methods we
        //      support. They are faulted in as actually used to avoid
        //      overhead. Some will often never get used in a given system.
        //
        //  m_strServer
        //      We use a generic error reply generator in THTTPClient, and
        //      it wants a server name to put into the Server: line. So this
        //      is set up once at ctor so we can use it any time we need to
        //      generate an error reply.
        //
        //  m_strmOutput
        //      We install this stream on the macro engine as the console out
        //      stream. The macro just formats text into this stream in order
        //      to create the output content.
        //
        //  m_tmHeader
        //      We have to put out times in a particular format in UTC, in all
        //      headers, so we keep one around with the correct formatting set
        //      and just update the time stamp when we need to use it.
        // --------------------------------------------------------------------
        tCIDLib::TKVPList           m_colInHdrLines;
        tCIDLib::TKVPList           m_colOutHdrLines;
        tCIDLib::TKVPList           m_colValues;
        TBlowfishEncrypter          m_crypAuth;
        THTTPClient                 m_httpcSrv;
        THeapBuf                    m_mbufData;
        THeapBuf                    m_mbufData2;
        TMessageDigest5             m_mdigAuth;
        TMD5Hash                    m_mhashAuth1;
        TMD5Hash                    m_mhashAuth2;
        TWSEchoHandler*             m_purlhEcho;
        TWSCQCImgHandler*           m_purlhCQCImg;
        TWSCMLBinHandler*           m_purlhCMLBin;
        TWSFileHandler*             m_purlhFile;
        TString                     m_strServer;
        TTextMBufOutStream          m_strmOutput;
        TTime                       m_tmHeader;


        // --------------------------------------------------------------------
        //  Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TWorkerThread,TThread)
};

#pragma CIDLIB_POPPACK


