//
// FILE NAME: CQCGenDrvS_DriverGlue.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/09/2003
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
//  This if the header for the TCQCGenDrvGlue class, which provides a simple
//  derivative of the standard CQC server side driver base class, and creates
//  an instance of the macro level driver class, and provides the glue code
//  required to link them together.
//
//  Drivers are event driven, so mainly this guy turns virtual methods made
//  to it, by the server side driver infrastructure, into subsequent calls
//  to the macro level method that handles that call at that level. There is
//  not a one to one correspondence though, since we try to hide as much
//  complexity as possible from the macro level drivers, in order to make
//  them as foolproof as they can be.
//
//  We have to support two ways of doing things. One is the normal way, where
//  CQCServer asks our facility object to create an instance of our driver
//  class (this class) and passes us our driver config and moniker, and we
//  create a macro engine instance, load the driver class, and then start
//  up the poll thread in our base class.
//
//  The other scheme is a simulator mode that is used when doing development
//  in the driver test harness. In that case, the macro class get's compiled
//  and the entry point of the class (which isn't used in the normal case)
//  will call the Simulate() method, which will call our Simulate() method,
//  which will pass us a ref to the driver object. We'll then act like the
//  poll thread by just calling that entry point directly, and we won't
//  return until the user stops us via the IDE. In this scenario, the driver
//  development framework (which will wrap the macro IDE engine) will be the
//  one who provides the moniker and driver config, by setting them on static
//  members of this class. The Simulate() method will then pick them up from
//  there.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCDriverInfo;
class TCQCMEngErrHandler;

// ---------------------------------------------------------------------------
//   CLASS: TCQCGenDrvGlue
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class CQCGENDRVSEXPORT TCQCGenDrvGlue : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Public, static data
        // -------------------------------------------------------------------
        static tCQCKit::EVerboseLvls    s_eDefVerbosity;
        static TMutex*                  s_pmtxSync;
        static TCQCGenDrvGlue*          s_psdrvCurrent;
        static TCQCDriverObjCfg         s_cqcdcToLoad;



        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCGenDrvGlue();

        TCQCGenDrvGlue
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TCQCGenDrvGlue();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryData
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TBoolean bQueryVal
        (
            const   TString&                strValId
        );

        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        );

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        );

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmdId
            , const TString&                strParms
        );

        tCIDLib::TInt4 i4QueryVal
        (
            const   TString&                strValId
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetDrvFields
        (
                    TSetFieldList&          colToSet
        );

        tCIDLib::TVoid Simulate
        (
                    TCIDMacroEngine&        meTarget
            ,       TMEngClassVal&          mecvDriver
            , const TCQCDriverInfo&         meciDriver
        );



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

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        );

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        );

        tCQCKit::ECommResults eFloatFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8NewValue
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

        tCQCKit::ECommResults eSListFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TVector<TString>&       colNewValue
        );

        tCQCKit::ECommResults eTimeFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard8&        c8NewValue
        );

        tCIDLib::TVoid Idle();

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();

        tCIDLib::TVoid VerboseModeChanged
        (
            const   tCQCKit::EVerboseLvls   eNewState
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TCQCGenDrvGlue(const TCQCGenDrvGlue&);
        tCIDLib::TVoid operator=(const TCQCGenDrvGlue&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetConType
        (
            const   TCQCDriverObjCfg&       cqcdcToUse
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bSimMode
        //      Indicates whether we are in simulation mode or normal mode.
        //      In sim mode, we don't own the driver and engine objects. Else,
        //      we owm them and must clean them up.
        //
        //  m_c2XXXId
        //      We look up the ids of the macro methods we call, to avoid
        //      getting them by name every time.
        //
        //  m_pmeciCommResEnum
        //      We look up this type from the driver base class, since we
        //      have to deal with it on each connect or poll method. SO we
        //      don't want to do a by name lookup each time.
        //
        //  m_pmeciDriver
        //  m_pmecvDriver
        //      The value and info object for the driver object and it's class.
        //      In normal mode we create them. In simulation mode, they are
        //      provided to us.
        //
        //  m_pmeehLogger
        //      We install a standard CQC error handler on the engine, which
        //      logs to the standard log server. The engine doesn't adopt it,
        //      so we have to keep up with it and clean it up when done with
        //      the engine.
        //
        //  m_pmefrData
        //      The file resolver that we install on the engine. It doesn't
        //      adopt, so we have to keep up with it and free it when the
        //      engine is cleaned up.
        //
        //  m_pmeTarget
        //      The macro engine we are running the driver on. In normal mode
        //      we create it. In simulation mode, it's provided to us.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bSimMode;
        tCIDLib::TCard2             m_c2MethId_ClientCmd;
        tCIDLib::TCard2             m_c2MethId_Conn;
        tCIDLib::TCard2             m_c2MethId_FldChBool;
        tCIDLib::TCard2             m_c2MethId_FldChCard;
        tCIDLib::TCard2             m_c2MethId_FldChFloat;
        tCIDLib::TCard2             m_c2MethId_FldChInt;
        tCIDLib::TCard2             m_c2MethId_FldChString;
        tCIDLib::TCard2             m_c2MethId_FldChStrList;
        tCIDLib::TCard2             m_c2MethId_FldChTime;
        tCIDLib::TCard2             m_c2MethId_FreeCommRes;
        tCIDLib::TCard2             m_c2MethId_GetCommRes;
        tCIDLib::TCard2             m_c2MethId_Poll;
        tCIDLib::TCard2             m_c2MethId_QueryBoolVal;
        tCIDLib::TCard2             m_c2MethId_QueryBufVal;
        tCIDLib::TCard2             m_c2MethId_QueryCardVal;
        tCIDLib::TCard2             m_c2MethId_QueryIntVal;
        tCIDLib::TCard2             m_c2MethId_QueryTextVal;
        tCIDLib::TCard2             m_c2MethId_WaitConfig;
        tCIDLib::TCard2             m_c2TypeId_MemBuf;
        TMEngClassVal*              m_pmecvDriver;
        const TMEngClassInfo*       m_pmeciDriver;
        TMEngMemBufInfo*            m_pmeciMemBuf;
        TMEngEnumInfo*              m_pmeciCommResEnum;
        TMEngEnumInfo*              m_pmeciVerbLevelEnum;
        TCQCMEngErrHandler*         m_pmeehLogger;
        TMEngFixedBaseFileResolver* m_pmefrData;
        TCIDMacroEngine*            m_pmeTarget;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCGenDrvGlue,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


