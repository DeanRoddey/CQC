//
// FILE NAME: LexMC1CS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2000
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
//  This is the header file for the actual driver class. We just inherit from
//  the IDL generated base server class, and provide implementations for the
//  virtual methods it created for us.
//
//  The Lexicon both responds to messages we send to it and it sends async
//  notifications. We turn most of them off, but wakeup/sleep notifications
//  still come asynchronously. So, to deal with this, we have a single
//  GetReply() method that will look for async messages, process them, and
//  hide their presence from the calling thread.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TLexMC1SDriver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class TLexMC1SDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TLexMC1SDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TLexMC1SDriver();


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

        tCQCKit::ECommResults eIntFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4NewValue
        );

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
        TLexMC1SDriver();
        TLexMC1SDriver(const TLexMC1SDriver&);
        tCIDLib::TVoid operator=(const TLexMC1SDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bProcessMsg
        (
            const   TLexMsg&                lmToProc
            , const tCIDLib::TBoolean       bFromPollThread = kCIDLib::False
        );

        tLexMC1S::EEffects eXlatEffectName
        (
            const   TString&                strToXlat
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        );

        tLexMC1S::EInputs eXlatInputName
        (
            const   TString&                strToXlat
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        );

        tCIDLib::TVoid GetReply
        (
            const   TLexMsg&                lmOrg
            ,       TLexMsg&                lmToFill
            , const tCIDLib::TBoolean       bFromPollThread = kCIDLib::False
            , const tCIDLib::TCard4         c4Timeout = 500
        );

        tCIDLib::TVoid InitComm();

        tCIDLib::TVoid LoadFieldList
        (
                    TVector<TCQCFldDef>&    colLoad
        );

        tCIDLib::TVoid PollFromMC1
        (
                    TLexMsg&                lmSend
            ,       TLexMsg&                lmReply
        );

        tCIDLib::TVoid SendIRCode
        (
                    TThread&                thrThis
            , const tLexMC1S::EIRCodes      eIRCode
        );

        tCIDLib::TVoid SetVolume
        (
            const   tCIDLib::TInt4          i4ToSet
        );

        tCIDLib::TVoid StoreSysStatus
        (
            const   TLexMsg&                lmStatus
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldXXX
        //      We look up our fields after registration so that we can
        //      use by id methods, for efficiency.
        //
        //      Note that BassLevel is deprecated. We later added fields to
        //      set all of the speaker levels and added a new field for this
        //      that is consistent with the other similar fields.
        //
        //  m_c4MaxSysParms
        //  m_c4MaxEffects
        //      These are gotten upon connect to the Lex and just let us do
        //      some sanity checking internally.
        //
        //  m_c4Timeouts
        //      We don't give up after a single timeout. Sometimes the Lexicon
        //      can get hung up on an operation, so we want to give it a little
        //      leeway. So this is used to keep up with sequential timeouts,
        //      and we give up after a few in a row.
        //
        //  m_cpcfgSerial
        //      The comm port configuration that we were given upon creation.
        //      We use this to configure our serial connection.
        //
        //  m_pcommLex
        //      The comm port via which we talk to the Lexicon.
        //
        //  m_strPort
        //      The serial port we were told to configuration ourself on.
        //
        //  m_tcvrtLex
        //      We get US-ASCII text from the Lexicon. We cannot assume that
        //      our local code page is ASCII compatible, so we have to use a
        //      converter to get it internal Unicode format.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4FldAdjustVol;
        tCIDLib::TCard4         m_c4FldBassLevel;
        tCIDLib::TCard4         m_c4FldEffect;
        tCIDLib::TCard4         m_c4FldFPDisplay;
        tCIDLib::TCard4         m_c4FldInputSrc;
        tCIDLib::TCard4         m_c4FldMute;
        tCIDLib::TCard4         m_c4FldNPInput;
        tCIDLib::TCard4         m_c4FldPower;
        tCIDLib::TCard4         m_c4FldRate;
        tCIDLib::TCard4         m_c4FldSetFLLev;
        tCIDLib::TCard4         m_c4FldSetCLev;
        tCIDLib::TCard4         m_c4FldSetFRLev;
        tCIDLib::TCard4         m_c4FldSetRLLev;
        tCIDLib::TCard4         m_c4FldSetRRLev;
        tCIDLib::TCard4         m_c4FldSetSLLev;
        tCIDLib::TCard4         m_c4FldSetSRLev;
        tCIDLib::TCard4         m_c4FldSetSubLev;
        tCIDLib::TCard4         m_c4FldStream;
        tCIDLib::TCard4         m_c4FldTogglePower;
        tCIDLib::TCard4         m_c4FldVolume;
        tCIDLib::TCard4         m_c4MaxSysParms;
        tCIDLib::TCard4         m_c4MaxEffects;
        tCIDLib::TCard4         m_c4Timeouts;
        TCommPortCfg            m_cpcfgSerial;
        TCommPortBase*          m_pcommLex;
        TString                 m_strPort;
        TUSASCIIConverter       m_tcvrtLex;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TLexMC1SDriver, TCQCServerBase)
};

#pragma CIDLIB_POPPACK

