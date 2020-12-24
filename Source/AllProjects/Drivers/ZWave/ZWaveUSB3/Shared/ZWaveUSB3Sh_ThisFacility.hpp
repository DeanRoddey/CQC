//
// FILE NAME: ZWaveUSB3Sh_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/31/2017
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
//  unusual 'global to the facility' type functionality.
//
//
//  Synchronization :
//
//  We don't really have any sync issues here though we are accessed from multiple threads.
//  The device info stuff is ONLY acccessed from the main driver thread, or its replication
//  thread during which time it doesn't do anything.
//
//  The encryption stuff is only accessed from the I/O thread inside the Z-Stick object
//  in the main driver facility. This happens indirectly via the In/Out msg classes, which
//  call it to encrypt/decrypt their msg content. But those are only done from the Z-Stick's
//  I/O thread.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacZWaveUSB3Sh
//  PREFIX: fac
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TFacZWaveUSB3Sh : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacZWaveUSB3Sh();

        ~TFacZWaveUSB3Sh();


        // ---------------------------------------------------------------------------
        //  Public, non-virtual methods
        // ---------------------------------------------------------------------------
        tCIDLib::TBoolean bLegalNetworkKey
        (
            const   TMemBuf&                mbufToCheck
        )   const;

        tCIDLib::TBoolean bHighTrace() const;

        tCIDLib::TBoolean bIsGenericManId
        (
            const   tCIDLib::TCard8         c8ToCheck
        )   const;

        tCIDLib::TBoolean bLowTrace() const;

        tCIDLib::TBoolean bMediumTrace() const;

        tCIDLib::TBoolean bQueryDevInfo
        (
            const   tCIDLib::TCard8         c8ManIds
            ,       TZWDevInfo&             deviToFill
            ,       TString&                strErr
        )   const;

        tCIDLib::TBoolean bQueryDevInfo
        (
            const   tCIDLib::TCard8         c8ManIds
            ,       TString&                strMake
            ,       TString&                strModel
            ,       TString&                strCppClass
        )   const;

        tCIDLib::TBoolean bQueryMakes
        (
                    tCIDLib::TStrList&      colToFill
        )   const;

        tCIDLib::TBoolean bQueryModels
        (
            const   TString&                strMake
            ,       tCIDLib::TKValsList&    colToFill
        )   const;

        tCIDLib::TBoolean bQueryModelNotes
        (
            const   tCIDLib::TCard8         c8ManIds
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bTraceEnabled() const;

        tCIDLib::TVoid BuildCCMsg
        (
                    TZWOutMsg&              zwomToFill
            , const TZWUnitInfo&            unitiTar
            , const tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1CmdId
            , const tCIDLib::TCard1         c1ReplyCmdId
            , const tZWaveUSB3Sh::EMsgPrios ePriority
            , const tCIDLib::TBoolean       bReceiptAck = kCIDLib::False
        )   const;

        tCIDLib::TVoid BuildCCMsg
        (
                    TZWOutMsg&              zwomToFill
            , const TZWUnitInfo&            unitiTar
            , const tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1CmdId
            , const tCIDLib::TCard1         c1ReplyCmdId
            , const tCIDLib::TCard1         c1Data1
            , const tZWaveUSB3Sh::EMsgPrios ePriority
            , const tCIDLib::TBoolean       bReceiptAck = kCIDLib::False
        )   const;

        tCIDLib::TVoid BuildCCMsg
        (
                    TZWOutMsg&              zwomToFill
            , const TZWUnitInfo&            unitiTar
            , const tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1CmdId
            , const tCIDLib::TCard1         c1ReplyCmdId
            , const tCIDLib::TCard1         c1Data1
            , const tCIDLib::TCard1         c1Data2
            , const tZWaveUSB3Sh::EMsgPrios ePriority
            , const tCIDLib::TBoolean       bReceiptAck = kCIDLib::False
        )   const;

        tCIDLib::TVoid BuildCCMsg
        (
                    TZWOutMsg&              zwomToFill
            , const TZWUnitInfo&            unitiTar
            , const tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1CmdId
            , const tCIDLib::TCard1         c1ReplyCmdId
            , const tCIDLib::TCard1         c1Data1
            , const tCIDLib::TCard1         c1Data2
            , const tCIDLib::TCard1         c1Data3
            , const tZWaveUSB3Sh::EMsgPrios ePriority
            , const tCIDLib::TBoolean       bReceiptAck = kCIDLib::False
        )   const;

        tCIDLib::TVoid BuildCCMsg
        (
                    TZWOutMsg&              zwomToFill
            , const TZWUnitInfo&            unitiTar
            , const tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1CmdId
            , const tCIDLib::TCard1         c1ReplyCmdId
            , const tCIDLib::TCard1         c1Data1
            , const tCIDLib::TCard1         c1Data2
            , const tCIDLib::TCard1         c1Data3
            , const tCIDLib::TCard1         c1Data4
            , const tZWaveUSB3Sh::EMsgPrios ePriority
            , const tCIDLib::TBoolean       bReceiptAck = kCIDLib::False
        )   const;

        tCIDLib::TCard1 c1CtrlId() const;

        tCIDLib::TCard1 c1PercentToDim
        (
            const   tCIDLib::TCard4         c4ToMap
        );

        tCIDLib::TCard4 c4DimToPercent
        (
            const   tCIDLib::TCard1         c1ToMap
        );

        tCIDLib::TVoid CalculateMAC
        (
            const   tCIDLib::TCard1         c1SecCmdId
            , const tCIDLib::TCard1* const  pc1EncData
            , const tCIDLib::TCard4         c4Len
            , const tCIDLib::TCard1         c1Sender
            , const tCIDLib::TCard1         c1Receiver
            , const TMemBuf&                mbufIV
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TVoid Decrypt
        (
            const   TMemBuf&                mbufCypher
            , const tCIDLib::TCard4         c4Count
            ,       TMemBuf&                mbufPlain
            , const TMemBuf&                mbufIV
        );

        tCQCKit::EVerboseLvls eTraceLevel() const
        {
            return m_eTraceLevel;
        }

        tCQCKit::EVerboseLvls eVerboseLevel() const
        {
            return m_eVerboseLevel;
        }

        tCIDLib::TVoid Encrypt
        (
            const   TMemBuf&                mbufPlain
            , const tCIDLib::TCard4         c4Count
            ,       TMemBuf&                mbufCypher
            , const TMemBuf&                mbufIV
        );

        tCIDLib::TVoid FlushMsgTrace();

        tCIDLib::TVoid LoadDevInfoMap() const;

        tCIDLib::TVoid LogInMsg
        (
            const   tCIDLib::TCh* const     pszPrefix
            , const TZWInMsg&               zwimMsg
        );

        tCIDLib::TVoid LogOutMsg
        (
            const   tCIDLib::TCh* const     pszPrefix
            , const TZWOutMsg&              zwomMsg
        );

        tCIDLib::TVoid LogTraceErr
        (
            const   tCIDLib::TCh* const     pszMsg
        )   const;

        tCIDLib::TVoid LogTraceErr
        (
            const   TString&                strMsg
        )   const;

        tCIDLib::TVoid LogTraceErr
        (
            const   tCIDLib::TCh* const     pszMsg
            , const MFormattable&           mfmtblToken1
            , const MFormattable&           mfmtblToken2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken3 = MFormattable::Nul_MFormattable()
        )   const;

        tCIDLib::TVoid LogTraceErr
        (
            const   TString&                strMsg
            , const MFormattable&           mfmtblToken1
            , const MFormattable&           mfmtblToken2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken3 = MFormattable::Nul_MFormattable()
        )   const;

        tCIDLib::TVoid LogTraceMsg
        (
            const   tCIDLib::TCh* const     pszMsg
        )   const;

        tCIDLib::TVoid LogTraceMsg
        (
            const   TString&                strMsg
        )   const;

        tCIDLib::TVoid LogTraceMsgNL
        (
            const   tCIDLib::TCh* const     pszMsg
        )   const;

        tCIDLib::TVoid LogTraceMsgNL
        (
            const   TString&                strMsg
        )   const;

        tCIDLib::TVoid LogTraceMsg
        (
            const   tCIDLib::TCh* const     pszMsg
            , const MFormattable&           mfmtblToken1
            , const MFormattable&           mfmtblToken2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken3 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken4 = MFormattable::Nul_MFormattable()
        )   const;

        tCIDLib::TVoid LogTraceMsgNL
        (
            const   tCIDLib::TCh* const     pszMsg
            , const MFormattable&           mfmtblToken1
            , const MFormattable&           mfmtblToken2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken3 = MFormattable::Nul_MFormattable()
        )   const;

        const tCIDLib::TCh* pszXlatCCCmdId
        (
            const   tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1CmdId
        )   const;

        TString strFindDevName
        (
            const   tCIDLib::TCard8         c8ManIds
        )   const;

        tCIDLib::TVoid ReloadDevInfo
        (
            const   tCIDLib::TCard8         c8ManIds
        );

        tCIDLib::TVoid ReloadDevInfoMap() const;

        tCIDLib::TVoid ResetMsgTrace();

        tCIDLib::TVoid SendManIds
        (
                    MZWaveCtrlIntf&         mzwciIO
            , const tCIDLib::TCard1         c1TarId
            , const tCIDLib::TCard8         c8ToSend
            , const tCIDLib::TBoolean       bSecure
        );

        tCIDLib::TVoid SetCtrlId
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        tCIDLib::TVoid SetDefNetworkKey();

        tCIDLib::TVoid SetNetworkKey
        (
            const   TMemBuf&                mbufToSet
        );

        tCIDLib::TVoid SetTraceLevel
        (
            const   tCQCKit::EVerboseLvls   eToSet
        );

        tCIDLib::TVoid SetVerboseLevel
        (
            const   tCQCKit::EVerboseLvls   eToSet
        );


    private :
        // ---------------------------------------------------------------------------
        //  Private, non-virtual methods
        // ---------------------------------------------------------------------------
        tCIDLib::TCard4 c4FindCacheIndex
        (
            const   tCIDLib::TCard8         c8ManIds
        )   const;

        tCIDLib::TCard4 c4FindMapIndex
        (
            const   tCIDLib::TCard8         c8ManIds
        )   const;

        tCIDLib::TCard4 c4LoadDevInfo
        (
            const   tCIDLib::TCard8         c8ManIds
        )   const;

        tCIDLib::TVoid FormatCCIds
        (
                    TTextOutStream&         strmTar
            , const tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1CmdId
        );

        tCIDLib::TVoid LogMsgId
        (
            const   tCIDLib::TCard1         c1MsgId
        );

        tCIDLib::TVoid SecuritySetup();


        // ---------------------------------------------------------------------------
        //  Private data members
        //
        //  m_c1CtrlId
        //      The driver tells us our unit id, which we need for some things. This is
        //      set by the main driver as soon as it knows it.
        //
        //  m_ckeyNetwork
        //      This is the network key. Initially it is a pre-defined value that is
        //      used when are being included. Once we are included, we get the actual
        //      key. So, we default it to the pre-defined value. When the driver comes
        //      up, if it has the network key, it sets that on us.
        //
        //      Though currently only accessed from the Z-Stick's internal I/O thread,
        //      indirectly via the in/out msg class' encrypt/decrypt methods, we still
        //      protect it with the mutex in case that should change.
        //
        //  m_crypAuth
        //  m_crypEncrypt
        //      We need encrypters for authorization and for encryption. We will set
        //      these up initially using the default network key. If the real key is
        //      set on us during startup, or upon inclusion, we will update them
        //      again with the real keys.
        //
        //      Though currently only accessed from the Z-Stick's internal I/O thread,
        //      indirectly via the in/out msg class' encrypt/decrypt methods, we still
        //      protect it with the mutex in case that should change.
        //
        //  m_enctLastMsg
        //      If trace logging is on, we remember the time of each message logged and
        //      display the time since the last one.
        //
        //  m_esrDTD
        //      We pre-load the DTD content which we need to set up on the XML parser for
        //      each of the device info files we parse. It gets faulted in from an
        //      otherwise const method so it is mutable.
        //
        //  m_eTraceLevel
        //      We use another verbosity level value to control our trace file verbosity.
        //      So we don't have to crank up driver logging in order to get high trace
        //      verbosity.
        //
        //  m_eVerboseLevel
        //      The driver keeps us up to date with his verbosity level so that we can
        //      log appropriately, and make this info available to other things like the
        //      unit classes and the Z-stick object.
        //
        //  m_mtxSync
        //      We need to coordinate access to the trace log stream, and this can be used
        //      for any other sync we might need though mostly this class is fairly safe
        //      in how it is accessed. It's mutable because we have to be able to sync
        //      within const methods as well.
        //
        //  m_pstrmTrace
        //      The user can enable message tracing, and we automatically do so if in
        //      debug mode. It's mutable because we need to be able to lgo stuff from
        //      within const methods.
        //
        //  m_strTracePath
        //      We are given the path in which we should create our trace log file
        //      (when/if it is enabled.)
        // ---------------------------------------------------------------------------
        tCIDLib::TCard1                 m_c1CtrlId;
        TCryptoKey                      m_ckeyNetwork;
        TAESEncrypter                   m_crypAuth;
        TAESEncrypter                   m_crypEncrypt;
        tCIDLib::TEncodedTime           m_enctLastMsg;
        mutable tCIDXML::TEntitySrcRef  m_esrDTD;
        tCQCKit::EVerboseLvls           m_eTraceLevel;
        tCQCKit::EVerboseLvls           m_eVerboseLevel;
        mutable TMutex                  m_mtxSync;
        mutable TTextFileOutStream*     m_pstrmTrace;
        TString                         m_strTracePath;
};

#pragma CIDLIB_POPPACK
