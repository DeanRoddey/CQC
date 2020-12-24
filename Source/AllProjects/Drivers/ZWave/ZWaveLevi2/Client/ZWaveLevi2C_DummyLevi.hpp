//
// FILE NAME: ZWaveLevi2C_DummyLevi.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/06/2014
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
//  The server side driver defines a mixin that the server side driver implements
//  in order to expose info to the shared facility. That is part of the interface
//  so it has to exist. So we create one that just does nothing. Generally few of
//  the methods will even get called based on what we do, but it has to be there.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TDummyLevi
//  PREFIX: mzwsf
// ---------------------------------------------------------------------------
class TDummyLevi : public MZWLeviSrvFuncs
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDummyLevi();

        ~TDummyLevi();



        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bLeviFldExists
        (
            const   TString&                strName
        )   const;

        tCIDLib::TBoolean bLeviQueryFldName
        (
            const   tCIDLib::TCard4         c4FldId
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bLeviStoreBoolFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bToStore
        );

        tCIDLib::TBoolean bLeviStoreCardFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4ToStore
        );

        tCIDLib::TBoolean bLeviStoreFloatFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8ToStore
        );

        tCIDLib::TBoolean bLeviStoreIntFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4ToStore
        );

        tCIDLib::TBoolean bLeviStoreStrFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strToStore
        );

        tCIDLib::TBoolean bLeviTraceEnabled() const;

        tCIDLib::TBoolean bLeviWaitBasicRep
        (
            const   TZWaveUnit&             unitSrc
            , const TZWCmdClass&            zwccSrc
            ,       tCIDLib::TCard4&        c4Value
        );

        tCIDLib::TBoolean bLeviWaitUnitMsg
        (
                    TZWaveUnit&             unitSrc
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCard4         c4CmdClass
            , const tCIDLib::TCard4         c4CmdId
            ,       tCIDLib::TCardList&     fcolVals
        );

        tCIDLib::TCard4 c4LeviFldIdFromName
        (
            const   TString&                strName
        )   const;

        tCIDLib::TCard4 c4LeviVRCOPId() const;

        tCQCKit::EVerboseLvls eLeviVerboseLevel() const;

        tCIDLib::TVoid LeviQEventTrig
        (
            const   tCQCKit::EStdDrvEvs     eEvent
            , const TString&                strFld
            , const TString&                strVal1
            , const TString&                strVal2
            , const TString&                strVal3
            , const TString&                strVal4
        )   override;

        tCIDLib::TVoid LeviSendMsg
        (
            const   TZWaveUnit&             unitTar
            , const tCIDLib::TCardList&     fcolVals
        );

        tCIDLib::TVoid LeviSendUnitMsg
        (
            const   TZWaveUnit&             unitTar
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCard4         c4CmdClass
            , const tCIDLib::TCard4         c4Cmd
        );

        tCIDLib::TVoid LeviSendUnitMsg
        (
            const   TZWaveUnit&             unitTar
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCardList&     fcolVals
        );

        tCIDLib::TVoid LeviSetFldErr
        (
            const   tCIDLib::TCard4         c4FldId
        );

        const TCQCFldLimit* pfldlLeviLimsFor
        (
            const   tCIDLib::TCard4         c4FldId
            , const TClass&                 clsType
        )   const;

        const TString& strLeviMoniker() const;

        TTextOutStream& strmLeviTrace();


        TTextStringOutStream m_strmDummy;
};

#pragma CIDLIB_POPPACK

