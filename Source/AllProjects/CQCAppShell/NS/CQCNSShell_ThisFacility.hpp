//
// FILE NAME: CQCNSShell_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/30/2001
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
//  This is the header for the facility class for the app shell.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCNSShell
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCNSShell : public TFacility, public MSignalHandler
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCNSShell();

        TFacCQCNSShell(const TFacCQCNSShell&) = delete;
        TFacCQCNSShell(TFacCQCNSShell&&) = delete;

        ~TFacCQCNSShell();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCNSShell& operator=(const TFacCQCNSShell&) = delete;
        TFacCQCNSShell& operator=(TFacCQCNSShell&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHandleSignal
        (
            const   tCIDLib::ESignals       eSignal
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryNewMsgs
        (
                    tCIDLib::TCard4&        c4MsgIndex
            ,       tCIDLib::TStrList&      colMsgs
            , const tCIDLib::TBoolean       bAddNewLine
        );

        tCIDLib::TCard4 c4QueryApps
        (
                    tCIDLib::TKVPList&      colApps
        );

        tCIDLib::TVoid CycleApps();

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid StartAllApps();

        tCIDLib::TVoid StartShutdown();

        tCIDLib::TVoid StopAllApps();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ShowBlurb();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_conOut
        //      To show a little output to the user
        //
        //  m_evShutdown
        //      Used to block on until we are asked to shut down
        // -------------------------------------------------------------------
        TOutConsole m_conOut;
        TEvent      m_evShutdown;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCNSShell,TFacility)
};



