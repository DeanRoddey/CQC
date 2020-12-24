//
// FILE NAME: CQSLAudPlS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/26/2007
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
//  This is the header file for the actual driver class. Note that the
//  methods of this class, which is fairly large, are spread out over a few
//  different files.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: CQSLAudPlS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TCQSLAudPlSDriver : public TCQCStdMediaRendDrv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLAudPlSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TCQSLAudPlSDriver();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bPlayNewItem
        (
            const   TCQCMediaPLItem&        mpliNew
        );

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eAdjustVolume
        (
            const   tCIDLib::TBoolean       bUp
        );

        tCQCKit::ECommResults eCheckPlayerStatus
        (
                    tCIDLib::TEncodedTime&  enctCurPos
            ,       tCIDLib::TEncodedTime&  enctTotal
            ,       tCQCMedia::EMediaStates& eState
            ,       tCQCMedia::EEndStates&  eEndState
            ,       tCIDLib::TBoolean&      bMute
            ,       tCIDLib::TCard4&        c4Volume
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eDoTransportCmd
        (
            const   tCQCMedia::ETransCmds   eToSet
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults eSetMute
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCQCKit::ECommResults eSetVolume
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid ResetPlayer();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TCQSLAudPlSDriver();
        TCQSLAudPlSDriver(const TCQSLAudPlSDriver&);
        tCIDLib::TVoid operator=(const TCQSLAudPlSDriver&);


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      We look up the ids of our fields after registering them, so
        //      that we can do efficient 'by id' reads/writes.
        //
        //  m_enctLastRandCat
        //      If in random category mode, we try to get another batch of
        //      values when the list gets below a certain amount. But, if
        //      they give us a category with fewer items than the size we
        //      try to maintain, we'd do the query after the end of every
        //      song. It's a fairly heavy operation on the media driver, so
        //      we limit how often we'll do it to no more than one once
        //      every 10 minutes worst case. We reset this any time the
        //      playlist mode is set to random cat or any time that the
        //      random category is set and we are in random cat mode, to
        //      allow a new query immediately.
        //
        //  m_ippnQTServer
        //      If we are using the QuickTime audio engine, this is the IP
        //      port that the QT audio server needs to be told to set up
        //      on. It comes in as one of the prompts.
        //
        //  m_pauplTarget
        //      The audio player object that we use to do our playback. It's
        //      a pointer because we'll load up one or another based on the
        //      selection made by the user during driver install as to which
        //      engine he wants to use.
        //
        //  m_strAudioDev
        //      The name of the target audio device that we are to use.
        //      It's the 'friendly' name, which we'll go back and find
        //      later to get the system level id.
        //
        //  m_strAudioEngine
        //      We support more than one audio engine. This is set from the
        //      audio engine driver prompt and we use it later to create
        //      the right type of engine.
        //
        //  m_strTmp
        //  m_strTmp2
        //      Some temp strings. Don't assume that they will be unaffected
        //      across any calls.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4FldId_PlayerStatus;
        tCIDLib::TIPPortNum     m_ippnQTServer;
        TCQCAudioPlayer*        m_pauplTarget;
        TString                 m_strAudioDev;
        TString                 m_strAudioEngine;
        TString                 m_strTmp;
        TString                 m_strTmp2;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLAudPlSDriver,TCQCStdMediaRendDrv)
};

#pragma CIDLIB_POPPACK


