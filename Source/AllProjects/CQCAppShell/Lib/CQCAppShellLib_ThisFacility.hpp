//
// FILE NAME: CQCAppShellLib_ThisFacility.hpp
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
//  This is the header for the facility class for the CQC app shell engine.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCAppShellLib
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCAPPSHLIBEXPORT TFacCQCAppShellLib : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCAppShellLib();

        TFacCQCAppShellLib(const TFacCQCAppShellLib&) = delete;
        TFacCQCAppShellLib(TFacCQCAppShellLib&&) = delete;

        ~TFacCQCAppShellLib();


        // -------------------------------------------------------------------
        //  Pubic operators
        // -------------------------------------------------------------------
        TFacCQCAppShellLib& operator=(const TFacCQCAppShellLib&) = delete;
        TFacCQCAppShellLib& operator=(TFacCQCAppShellLib&&) = delete;


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

        tCIDLib::TVoid Initialize
        (
            const   TString&                strPath
        );

        TTextOutStream& strmOut();

        tCIDLib::TVoid StartAllApps();

        tCIDLib::TVoid StopAllApps();

        tCIDLib::TVoid StopEngine();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ParseListFile
        (
            const   TString&                strFile
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_asengCtrl
        //      This is our underlying app server engine that we use to
        //      control the proceses. This guy does the grunt work. We just
        //      have to provide it with the info on the processes to run.
        //
        //  m_strmOut
        //      A queued output stream we dump out status messages to. Clients
        //      can call and get the most recents ones.
        // -------------------------------------------------------------------
        TCQCAppShEngine     m_asengCtrl;
        TTextQOutStream     m_strmOut;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCAppShellLib,TFacility)
};

#pragma CIDLIB_POPPACK


