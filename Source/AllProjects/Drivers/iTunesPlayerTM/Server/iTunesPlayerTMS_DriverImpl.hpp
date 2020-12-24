//
// FILE NAME: iTunesPlayerTMS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/09/2012
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
//  This is the header file for the actual driver class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: iTunesPlayerTMS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TiTunesPlayerTMSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Create a type short cut
        // -------------------------------------------------------------------
        typedef TiTPlayerIntfClientProxy    TPlBase;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TiTunesPlayerTMSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TiTunesPlayerTMSDriver();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------


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

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

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
        TiTunesPlayerTMSDriver();
        TiTunesPlayerTMSDriver(const TiTunesPlayerTMSDriver&);
        tCIDLib::TVoid operator=(const TiTunesPlayerTMSDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::ECommResults eGetPlayerStatus();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      We look up the ids of our fields after registering them, so
        //      that we can do efficient 'by id' reads/writes.
        //
        //  m_c4SerialNum
        //      The tray monitor maintains a serial number that it bumps when
        //      any status changes. So the IDL interface uses a poll method
        //      for the status query. It only streams back status info if we
        //      are getting new info. We have to reset this any time we lose
        //      connection.
        //
        //  m_porbcTray
        //      A pointer to a client proxy for the tray app. We create it
        //      during get comm resource and delete it if we lose connection.
        //
        //  m_strBinding
        //      We pre-build the name server binding that we expect to find
        //      the tray monitor at.
        //
        //  m_strPollTmpX
        //      Some strings for the poll method to use to grab data from
        //      the tray monitor.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4FldId_CurAlbum;
        tCIDLib::TCard4             m_c4FldId_CurArtist;
        tCIDLib::TCard4             m_c4FldId_CurTrack;
        tCIDLib::TCard4             m_c4FldId_Mute;
        tCIDLib::TCard4             m_c4FldId_PBPercent;
        tCIDLib::TCard4             m_c4FldId_PlayerCmd;
        tCIDLib::TCard4             m_c4FldId_SelPLByCookie;
        tCIDLib::TCard4             m_c4FldId_SelTrackByCookie;
        tCIDLib::TCard4             m_c4FldId_State;
        tCIDLib::TCard4             m_c4FldId_Volume;
        tCIDLib::TCard4             m_c4SerialNum;

        TiTPlayerIntfClientProxy*   m_porbcTray;
        TString                     m_strBinding;
        TString                     m_strPollTmp1;
        TString                     m_strPollTmp2;
        TString                     m_strPollTmp3;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TiTunesPlayerTMSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


