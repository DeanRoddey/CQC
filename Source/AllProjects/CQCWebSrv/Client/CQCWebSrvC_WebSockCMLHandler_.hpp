//
// FILE NAME: CQCWebSrvC_WebSockCMLHandler.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 091/17/2014
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
//  This is the header for the class that implements a derivative of the base
//  Websockets handler class. This one is specialized for CML based handlers. It
//  acts as a middleman between the client and a CML class that derives from a
//  special Websockets abstract class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TWebsockCMLThread
//  PREFIX: thr
// ---------------------------------------------------------------------------
class TWebsockCMLThread : public TWebsockThread
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructors
        // --------------------------------------------------------------------
        TWebsockCMLThread
        (
            const   tCIDLib::TBoolean       bSimMode
        );

        TWebsockCMLThread(const TWebsockCMLThread&) = delete;
        TWebsockCMLThread(TWebsockCMLThread&&) = delete;

        ~TWebsockCMLThread();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TWebsockCMLThread& operator=(const TWebsockCMLThread&) = delete;
        TWebsockCMLThread& operator=(TWebsockCMLThread&&) = delete;


        // --------------------------------------------------------------------
        //  Public, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid Simulate
        (
                    TCIDMacroEngine&        meTarget
            ,       TMEngClassVal&          mecvHandler
            , const TString&                strClassPath
        );

        tCIDLib::TVoid SendTextMsg
        (
            const   TString&                strToSend
        );


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TCard4 c4WSInitialize
        (
            const   TURL&                   urlReq
            ,       TString&                strRepText
            ,       TString&                strErrText
        )   final;

        tCIDLib::TVoid CheckShutdownReq() const final;

        tCIDLib::TVoid Connected() final;

        tCIDLib::TVoid Disconnected() final;

        tCIDLib::TVoid FieldChanged
        (
            const   TString&                strMon
            , const TString&                strField
            , const tCIDLib::TBoolean       bGoodValue
            , const TCQCFldValue&           fvValue
        )   final;

        tCIDLib::TVoid Idle() final;

        tCIDLib::TVoid ProcessMsg
        (
            const   TString&                strMsg
        )   final;

        tCIDLib::TVoid WSTerminate() final;


    private :
        // --------------------------------------------------------------------
        //  Private, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid DoInit();


        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2MethId_xxx
        //      We look up the ids of the methods and some classes for quicker access.
        //
        //  m_pmecvHandler
        //  m_pmeciHandler
        //      During the init, we keep around pointers to the CML handler's value and
        //      info classes, since we need to use them a lot when making calls to the
        //      derived class. If not in sim mode, we own the value object since we
        //      created it.
        //
        //  m_pmeTarget
        //      Our macro engine. We exist effectively to act as a coordinator between
        //      the client and the CML level handler the client has invoked. When run
        //      normally, we create this. In simulator mode, in the IDE, it is provided
        //      to us by the IDE. That's why it's a pointer.
        //
        //  m_pmefrData
        //      We install a standard CQC CML file resolver so that the macros can
        //      access files in MacroFileRoot for storage and such.
        //
        //  m_strClassPath
        //      This is stored during basic init, after we work out what the actual
        //      class path should be, based in the incoming URL. In simulator mode it
        //      it set by Simulate() and the incoming URL must resolve to the same
        //      class.
        // --------------------------------------------------------------------
        TMEngClassVal*              m_pmecvHandler;
        const TMEngClassInfo*       m_pmeciHandler;
        TCIDMacroEngine*            m_pmeTarget;
        TMEngFixedBaseFileResolver* m_pmefrData;
        TString                     m_strClassPath;
};

#pragma CIDLIB_POPPACK


