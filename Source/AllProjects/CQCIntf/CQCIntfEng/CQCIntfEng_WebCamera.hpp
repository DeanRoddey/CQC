//
// FILE NAME: CQCIntfEng_WebCamera.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/07/2017
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
//  This is the header for the CQCIntfEng_WebCamera.cpp file, which implements the
//  TCQCIntfWebCamera class. It creates and manages a separate process which creates a
//  window and places it over this widget's area. It uses libvlc to display web cam
//  video into that window. The user configures a URL that represents the target to
//  connect to for the video content. We point the VLC engine at that URL.

//  So it looks like it's part of our window but it's really not. This is done because
//  we don't want that code inside our main IV process. Digital media and web cams are not
//  the most stable of technologies, so we want it well separated. So we want to be able
//  to close them down and next time they start completely from scratch again.
//
//  So it's a pain because it's tricky to maintain a separate process window like this,
//  but it's the only thing that is practical under the circumstances.
//
//  The helper will shut down when we tell it to, or if it doesn't get at least a ping from
//  us within 10 seconds. In the normal course of events we are just pinging it regularly
//  using the active update callback. We ping every 5 seconds to be safe.
//
//  If we lose connection to it, we see if it's still running. If not, we'll try to restart
//  it once. If that fails, we give up and it just stays down.
//
//  This guy doesn't support any of the visual styles of a widget other than the bgn
//  fill. It is just a place holder for the, always fully covering and always fully opaque,
//  helper process' window. We do of course have other styles, for setting up the necessary
//  information to pass to the helper.
//
//  To support the WebRIVA client, we allow an alternative URL, since the client will
//  often need some other means of accessing the string, such as a WebRTC gateway program.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCWCHelperClientProxy;


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWebCamera
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfWebCamera : public TCQCIntfWidget
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bValidateParms
        (
            const   TString&                strToVal
            ,       TString&                strErr
            ,       tCIDLib::TKVPList&      colToFill
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfWebCamera();

        TCQCIntfWebCamera
        (
            const   TCQCIntfWebCamera&      iwdgToCopy
        );

        ~TCQCIntfWebCamera();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfWebCamera& operator=
        (
            const   TCQCIntfWebCamera&      iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TBoolean bValidateParm
        (
            const   TCQCCmd&                cmdSrc
            , const TCQCCmdCfg&             ccfgTar
            , const tCIDLib::TCard4         c4Index
            , const TString&                strValue
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid PostInit() override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        TSize szDefaultSize() const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Terminate() override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActiveUpdate
        (
            const   tCIDLib::TBoolean       bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
        )   final;

        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        )   final;

        tCIDLib::TVoid DisplayStateChanged() final;

        tCIDLib::TVoid ParentDisplayStateChanged
        (
            const   tCQCIntfEng::EDispStates eState
        )   final;

        tCIDLib::TVoid ScrPosChanged
        (
            const   TArea&                  areaTmplWnd
        )   final;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bShowHelper();

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid SendRemoteURL
        (
            const   TString&                strURL
        );

        tCIDLib::TVoid StartHelper();

        tCIDLib::TVoid TryReconnect();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bMute
        //      The last mute state that was requested. It is initialized based on the
        //      configured attribute and potentially changed at viewing time via a set mute
        //      command. We need to remember this at viewing time since we have to pass it
        //      back in if we have to restart the helper. This last set state gets passed in
        //      with the initial info in the memory buffer.
        //
        //      True means muted.
        //
        //  m_c4RetryCnt
        //      If we lose connection, we'll try to recover a few times before we give up.
        //      Once we get that far, we just stop trying becuase it's probably a waste of
        //      time.
        //
        //  m_c4Volume
        //      The volume that was requested. It is initialized based on the configured
        //      attribute, and potentially changed at viewing time via an action command.
        //      We need to remember this at viewing time since we have to pass it back in
        //      if we have to restart the helper. This last set state gets passed in
        //      with the initial info in the memory buffer.
        //
        //  m_enctNextPing
        //      The next time we need to ping our external process, so that he doesn't close
        //      down, thinking we are gone away.
        //
        //  m_enctRestart
        //      Because digital media playback isn't exactly a science, and running a
        //      stream for a long period of time is likely to be tempting fate, when we
        //      start up, we set this to a period of time forward. When we hit this time
        //      we will ask the helper to shut down if it he is running, and then restart
        //      him immediately. We reset this to the next restart time, and just keep
        //      doing this.
        //
        //  m_extpHelper
        //      The external process object we use to start the helper. This allows us to
        //      check if it's still there or not if we lose connection.
        //
        //  m_hwndHelper
        //      We get back the main frame window handle of the helper.
        //
        //  m_porbcCamera
        //      A client proxy that we use to talk to the external application that we spawn
        //      of to display the camera content.
        //
        //  m_strParams
        //      In order to support various, optional, bits of configuration info that might
        //      be needed for this or that camera, we have this parameters string. The user
        //      can set it to a quoted comma list of key=value pairs. The keys will be
        //      defined by the helper and documented in our widget documentation.
        //
        //  m_strUID
        //      We generate a unique id that is used to tie us to the display mechanism.
        //      In native mode it's used to create the shared resources between us and the
        //      helper program. In WebRIVA mode, it's used to let us and the client agree
        //      on which camera widget is being referred to in messages we send.
        //
        //  m_strURL
        //      The URL to use for the connection.
        //
        //  m_strWebURL
        //      An alternate URL for use with the WebRIVA client. If this is configured
        //      it will be sent to the WebRIVA client, else the regular URL will be.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bMute;
        tCIDLib::TCard4             m_c4RetryCnt;
        tCIDLib::TCard4             m_c4Volume;
        tCIDLib::TEncodedTime       m_enctNextPing;
        tCIDLib::TEncodedTime       m_enctRestart;
        TExternalProcess            m_extpHelper;
        tCIDCtrls::TWndHandle       m_hwndHelper;
        TCQCWCHelperClientProxy*    m_porbcCamera;
        TString                     m_strParams;
        TString                     m_strUID;
        TString                     m_strURL;
        TString                     m_strWebURL;

        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfWebCamera,TCQCIntfWidget)
        DefPolyDup(TCQCIntfWebCamera)
};

#pragma CIDLIB_POPPACK
