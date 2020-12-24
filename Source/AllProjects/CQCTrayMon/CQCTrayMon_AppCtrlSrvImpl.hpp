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

        ~TAppCtrlServerImpl();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TAppCtrlServerImpl& operator=(const TAppCtrlServerImpl&) = delete;


        // --------------------------------------------------------------------
        //  Public, inherited methods
        // --------------------------------------------------------------------
        TAppCtrlRec& acrFind
        (
            const   TString&                strMoniker
        );

        tCIDLib::TVoid AddRecord
        (
            const   TString&                strMoniker
            , const TString&                strTitle
            , const TString&                strAppName
            , const TString&                strParams
            , const TString&                strWorkingDir
        );

        tCIDLib::TBoolean bAppRegistered
        (
            const   TString&                strMoniker
            , const TString&                strAppTitle
        );

        tCIDLib::TBoolean bIsRunning
        (
            const   TString&                strMoniker
        );

        tCIDLib::TBoolean bIsVisible
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        );

        tCIDLib::TBoolean bInvokeFile
        (
            const   TString&                strPath
            , const TString&                strShowType
        );

        tCIDLib::TBoolean bStart
        (
            const   TString&                strMoniker
        );

        tCIDLib::TBoolean bStartApp
        (
            const   TString&                strAppPath
            , const TString&                strParams
            , const TString&                strInitPath
        );

        tCIDLib::TBoolean bStartWithParams
        (
            const   TString&                strMoniker
            , const TString&                strParams
        );

        tCIDLib::TBoolean bStartAppViaOpen
        (
            const   TString&                strMoniker
            , const TString&                strDocPath
            , const TString&                strInitPath
            , const TString&                strShow
        );

        tCIDLib::TBoolean bStop
        (
            const   TString&                strMoniker
        );

        tCIDLib::TCard4 c4AddWindow
        (
            const   TString&                strMoniker
            , const TString&                strPath
        );

        tCIDLib::TCard4 c4AddWindowByClass
        (
            const   TString&                strMoniker
            , const TString&                strClass
        );

        tCIDLib::TCard4 c4QueryListSel
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        );

        tCIDLib::TCard4 c4QueryText
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            ,       TString&                strToFill
        );

        tCIDLib::TCard4 c4QueryWndStyles
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        );

        tCIDLib::TInt4 i4QueryTrackBar
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        );

        tCIDLib::TInt4 i4SendMsg
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCard4         c4ToSend
            , const tCIDLib::TCard4         c4Param1
            , const tCIDLib::TInt4          i4Param2
            , const tCIDLib::TBoolean       bAsync
        );

        TAppCtrlRec* pacrFind
        (
            const   TString&                strMoniker
        );

        tCIDLib::TVoid Ping();

        tCIDLib::TVoid RemoveRecord
        (
            const   TString&                strMoniker
        );

        tCIDLib::TVoid SendExtKey
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDCtrls::EExtKeys     eKeyToSend
            , const tCIDLib::TBoolean       bAltShifted
            , const tCIDLib::TBoolean       bCtrlShifted
            , const tCIDLib::TBoolean       bShifted
        );

        tCIDLib::TVoid SendKey
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCh            chToSend
            , const tCIDLib::TBoolean       bAltShifted
            , const tCIDLib::TBoolean       bCtrlShifted
            , const tCIDLib::TBoolean       bShifted
        );

        tCIDLib::TVoid SetFocus
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        );

        tCIDLib::TVoid SetListSel
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCard4         c4Index
        );

        tCIDLib::TVoid SetTrackBar
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TInt4          i4ToSet
        );

        tCIDLib::TVoid SetWindowPos
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TInt4          i4X
            , const tCIDLib::TInt4          i4Y
        );

        tCIDLib::TVoid SetWindowSize
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCard4         c4Width
            , const tCIDLib::TCard4         c4Height
        );

        tCIDLib::TVoid SetWindowText
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const TString&                strText
        );

        tCIDLib::TVoid ShowWindow
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TBoolean       bState
        );

        tCIDLib::TVoid StandardOp
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCQCKit::EStdACOps      eOp
        );


        // --------------------------------------------------------------------
        //  Public, non-virtual methods
        // --------------------------------------------------------------------
        const tCQCTrayMon::TAppList& colAppList() const;

        TMutex& mtxSync();


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid Initialize();

        tCIDLib::TVoid Terminate();


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


