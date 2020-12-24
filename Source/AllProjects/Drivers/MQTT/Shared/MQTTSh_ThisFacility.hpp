//
// FILE NAME: MQTTSh_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/11/2018
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
//  This is the header for the facility class for this facility. It just provides the
//  usual 'global to the facility' type functionality.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacMQTTSh
//  PREFIX: fac
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TFacMQTTSh : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacMQTTSh();

        TFacMQTTSh(const TFacMQTTSh&) = delete;

        ~TFacMQTTSh();


        // ---------------------------------------------------------------------------
        //  Public operators
        // ---------------------------------------------------------------------------
        TFacMQTTSh& operator=(const TFacMQTTSh&) = delete;


        // ---------------------------------------------------------------------------
        //  Public, non-virtual methods
        // ---------------------------------------------------------------------------
        tCIDLib::TBoolean bTraceMode() const
        {
            return m_bTraceMode;
        }

        tCIDLib::TBoolean bTraceMode
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCQCKit::EVerboseLvls eVerbosity() const
        {
            return m_eVerbosity;
        }

        tCQCKit::EVerboseLvls eVerbosity(const tCQCKit::EVerboseLvls eToSet)
        {
            m_eVerbosity = eToSet;
            return m_eVerbosity;
        }

        tCIDLib::TVoid LogInMsg
        (
            const   TMQTTInMsg&             msgSrc
        );

        tCIDLib::TVoid LogOutMsg
        (
            const   TMQTTOutMsg&            msgSrc
        );

        tCIDLib::TVoid LogTraceMsg
        (
            const   tMQTTSh::EMsgSrcs       eSrc
            , const TString&                strMsg
            , const MFormattable&           fmtblToken1 = MFormattable::Nul_MFormattable()
            , const MFormattable&           fmtblToken2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           fmtblToken3 = MFormattable::Nul_MFormattable()
        );

        tCIDLib::TVoid LogTraceErr
        (
            const   tMQTTSh::EMsgSrcs       eSrc
            , const TString&                strMsg
            , const MFormattable&           fmtblToken1 = MFormattable::Nul_MFormattable()
            , const MFormattable&           fmtblToken2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           fmtblToken3 = MFormattable::Nul_MFormattable()
        );

        tCIDLib::TVoid LogTraceOutput
        (
            const   tMQTTSh::EMsgSrcs       eSrc
            , const TString&                strMsg
            , const tCIDLib::TBoolean       bError
            , const MFormattable&           fmtblToken1
            , const MFormattable&           fmtblToken2
            , const MFormattable&           fmtblToken3
        );

        [[nodiscard]] TMQTTValMap* pmqvmpMakeNew
        (
            const   TString&                strType
        )   const;


    private :
        // ---------------------------------------------------------------------------
        //  Private data members
        //
        //  m_bTraceMode
        //      Controls whether we dump information out to the msg trace, which is
        //      used in high verbosity debugging to avoid filling the logs. It can be
        //      set via our ORB interface and video command line. If we fail to open
        //      the trace file, we force it back off.
        //
        //  m_eVerbosity
        //      The driver passes us whatever verbosity level is set on him, so that
        //      we will be in sync with him in terms of how much logging we do.
        //
        //  m_mtxSync
        //      We need to sync access to the trace output stream.
        //
        //  m_strmTrace
        //      If the msg trace is enabled, this is set up for a text file in the
        //      server side data directory where our config is stored.
        //
        //  m_tmTrace
        //      Only used if msg trace is enable. We set it up with the desired
        //      formatting options and use it ot output the current time stamp
        //      on each traced msgs.
        // ---------------------------------------------------------------------------
        tCIDLib::TBoolean       m_bTraceMode;
        tCQCKit::EVerboseLvls   m_eVerbosity;
        TMutex                  m_mtxSync;
        TTextFileOutStream      m_strmTrace;
        TTime                   m_tmTrace;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacMQTTSh, TFacility)
};

#pragma CIDLIB_POPPACK
