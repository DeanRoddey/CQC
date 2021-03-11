//
// FILE NAME: CQCTrayMon_AppCtrlSrvImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/15/2004
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
//  This is the header for the CQCTrayMon_AppCtrlSrvImpl.cpp file, which
//  provides the implementation of the IDL generated server stub for the
//  application control portion of the tray monitor app.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


class TCQCTrayAppCtrlTab;


// ---------------------------------------------------------------------------
//   CLASS: TAppCtrlServerImpl
//  PREFIX: acr
// ---------------------------------------------------------------------------
class TAppCtrlServerImpl : public TAppCtrlServerBase
{
    public :
        // --------------------------------------------------------------------
        //  Constructors and Destructor
        // --------------------------------------------------------------------
        TAppCtrlServerImpl() = delete;

        TAppCtrlServerImpl
        (
                    TCQCTrayAppCtrlTab* const pwndTab
        );

        TAppCtrlServerImpl(const TAppCtrlServerImpl&) = delete;
        TAppCtrlServerImpl(TAppCtrlServerImpl&&) = delete;

        ~TAppCtrlServerImpl();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TAppCtrlServerImpl& operator=(const TAppCtrlServerImpl&) = delete;
        TAppCtrlServerImpl& operator=(TAppCtrlServerImpl&&) = delete;


        // --------------------------------------------------------------------
        //  Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid AddRecord
        (
            const   TString&                strMoniker
            , const TString&                strTitle
            , const TString&                strAppName
            , const TString&                strParams
            , const TString&                strWorkingDir
        )   final;

        tCIDLib::TBoolean bAppRegistered
        (
            const   TString&                strMoniker
            , const TString&                strAppTitle
        )   final;

        tCIDLib::TBoolean bIsRunning
        (
            const   TString&                strMoniker
        )   final;

        tCIDLib::TBoolean bIsVisible
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        )   final;

        tCIDLib::TBoolean bInvokeFile
        (
            const   TString&                strPath
            , const TString&                strShowType
        )   final;

        tCIDLib::TBoolean bStart
        (
            const   TString&                strMoniker
        )   final;

        tCIDLib::TBoolean bStartApp
        (
            const   TString&                strAppPath
            , const TString&                strParams
            , const TString&                strInitPath
        )   final;

        tCIDLib::TBoolean bStartWithParams
        (
            const   TString&                strMoniker
            , const TString&                strParams
        )   final;

        tCIDLib::TBoolean bStartAppViaOpen
        (
            const   TString&                strMoniker
            , const TString&                strDocPath
            , const TString&                strInitPath
            , const TString&                strShow
        )   final;

        tCIDLib::TBoolean bStop
        (
            const   TString&                strMoniker
        )   final;

        tCIDLib::TCard4 c4AddWindow
        (
            const   TString&                strMoniker
            , const TString&                strPath
        )   final;

        tCIDLib::TCard4 c4AddWindowByClass
        (
            const   TString&                strMoniker
            , const TString&                strClass
        )   final;

        tCIDLib::TCard4 c4QueryListSel
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        )   final;

        tCIDLib::TCard4 c4QueryText
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            ,       TString&                strToFill
        )   final;

        tCIDLib::TCard4 c4QueryWndStyles
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        )   final;

        tCIDLib::TInt4 i4QueryTrackBar
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        )   final;

        tCIDLib::TInt4 i4SendMsg
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCard4         c4ToSend
            , const tCIDLib::TCard4         c4Param1
            , const tCIDLib::TInt4          i4Param2
            , const tCIDLib::TBoolean       bAsync
        )   final;

        tCIDLib::TVoid Ping() final;

        tCIDLib::TVoid RemoveRecord
        (
            const   TString&                strMoniker
        )   final;

        tCIDLib::TVoid SendExtKey
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDCtrls::EExtKeys     eKeyToSend
            , const tCIDLib::TBoolean       bAltShifted
            , const tCIDLib::TBoolean       bCtrlShifted
            , const tCIDLib::TBoolean       bShifted
        )   final;

        tCIDLib::TVoid SendKey
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCh            chToSend
            , const tCIDLib::TBoolean       bAltShifted
            , const tCIDLib::TBoolean       bCtrlShifted
            , const tCIDLib::TBoolean       bShifted
        )   final;

        tCIDLib::TVoid SetFocus
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        )   final;

        tCIDLib::TVoid SetListSel
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCard4         c4Index
        )   final;

        tCIDLib::TVoid SetTrackBar
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TInt4          i4ToSet
        )   final;

        tCIDLib::TVoid SetWindowPos
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TInt4          i4X
            , const tCIDLib::TInt4          i4Y
        )   final;

        tCIDLib::TVoid SetWindowSize
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCard4         c4Width
            , const tCIDLib::TCard4         c4Height
        )   final;

        tCIDLib::TVoid SetWindowText
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const TString&                strText
        )   final;

        tCIDLib::TVoid ShowWindow
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TBoolean       bState
        )   final;

        tCIDLib::TVoid StandardOp
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCQCKit::EStdACOps      eOp
        )   final;


        // --------------------------------------------------------------------
        //  Public, non-virtual methods
        // --------------------------------------------------------------------
        TAppCtrlRec& acrFind
        (
            const   TString&                strMoniker
        );

        const tCQCTrayMon::TAppList& colAppList() const;

        TMutex& mtxSync();

        TAppCtrlRec* pacrFind
        (
            const   TString&                strMoniker
        );


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colApps
        //      The current list of applications. It is a keyed hash set with
        //      the key being the moniker, so that we can quickly map from
        //      an incoming command from a driver with a particular moniker
        //      to the application that that driver is controlling.
        //
        //  m_mtxSync
        //      We need to be able to sync this object, so that the window
        //      can access the app list data during updates, and be sure
        //      that incoming client calls won't modify it.
        //
        //  m_pwndTab
        //      A pointer to our associated tab, who we need to send commands
        //      to pretty often.
        // -------------------------------------------------------------------
        tCQCTrayMon::TAppList   m_colApps;
        TMutex                  m_mtxSync;
        TCQCTrayAppCtrlTab*     m_pwndTab;


        // --------------------------------------------------------------------
        // Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TAppCtrlServerImpl,TAppCtrlServerBase)
};


