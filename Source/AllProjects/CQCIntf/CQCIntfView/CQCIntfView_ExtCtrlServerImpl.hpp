//
// FILE NAME: CQCIntfView_ExtCtrlServerImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/01/2004
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
//  This is the implementation of our server side control interface. We provide
//  a simple ORB-based protocol via which we can be controlled, and a CQC
//  driver is provided that works in terms of this interface so that the
//  interface can be controlled via IR remote.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlEv
//  PREFIX: ice
// ---------------------------------------------------------------------------
class TIntfCtrlEv : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TIntfCtrlEv(const TIntfCtrlEv&) = delete;
        TIntfCtrlEv(TIntfCtrlEv&&) = delete;
        ~TIntfCtrlEv();


        // -------------------------------------------------------------------
        //  Public opeartors
        // -------------------------------------------------------------------
        TIntfCtrlEv& operator=(const TIntfCtrlEv&) = delete;
        TIntfCtrlEv& operator=(TIntfCtrlEv&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid ProcessEv
        (
                    TMainFrameWnd&          wndMain
        ) = 0;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TIntfCtrlEv();


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfCtrlEv,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlExtKeyEv
//  PREFIX: ice
// ---------------------------------------------------------------------------
class TIntfCtrlExtKeyEv : public TIntfCtrlEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfCtrlExtKeyEv() = delete;

        TIntfCtrlExtKeyEv
        (
            const   tCIDCtrls::EExtKeys     eToSend
            , const tCIDLib::TBoolean       bAltShift
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
        );

        TIntfCtrlExtKeyEv(const TIntfCtrlExtKeyEv&) = delete;
        TIntfCtrlExtKeyEv(TIntfCtrlExtKeyEv&&) = delete;

        ~TIntfCtrlExtKeyEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfCtrlExtKeyEv& operator=(const TIntfCtrlExtKeyEv&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TMainFrameWnd&          wndMain
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAltShift
        //  m_bCtrlShift
        //  m_bShift
        //      Indicates whether the key should be sent with shift keys
        //      pressed.
        //
        //  m_eToSend
        //      The extended character to send.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bAltShift;
        tCIDLib::TBoolean   m_bCtrlShift;
        tCIDLib::TBoolean   m_bShift;
        tCIDCtrls::EExtKeys m_eToSend;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfCtrlExtKeyEv, TIntfCtrlEv)
};


// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlFrameOp
//  PREFIX: ice
// ---------------------------------------------------------------------------
class TIntfCtrlFrameOp : public TIntfCtrlEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfCtrlFrameOp() = delete;

        TIntfCtrlFrameOp
        (
            const   tCQCKit::EIVFrmOps      eOp
        );

        TIntfCtrlFrameOp(const TIntfCtrlFrameOp&) = delete;
        TIntfCtrlFrameOp(TIntfCtrlFrameOp&&) = delete;

        ~TIntfCtrlFrameOp();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfCtrlFrameOp& operator=(const TIntfCtrlFrameOp&) = delete;
        TIntfCtrlFrameOp& operator=(TIntfCtrlFrameOp&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TMainFrameWnd&          wndMain
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eOp
        //      The frame operation to do
        // -------------------------------------------------------------------
        tCQCKit::EIVFrmOps  m_eOp;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfCtrlMiscEv, TIntfCtrlEv)
};


// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlLoadEv
//  PREFIX: ice
// ---------------------------------------------------------------------------
class TIntfCtrlLoadEv : public TIntfCtrlEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfCtrlLoadEv() = delete;

        TIntfCtrlLoadEv
        (
            const   TString&                strTemplate
        );

        TIntfCtrlLoadEv
        (
            const   TString&                strOvrName
            , const TString&                strTemplate
        );

        TIntfCtrlLoadEv(const TIntfCtrlLoadEv&) = delete;
        TIntfCtrlLoadEv(TIntfCtrlLoadEv&&) = delete;

        ~TIntfCtrlLoadEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfCtrlLoadEv& operator=(const TIntfCtrlLoadEv&) = delete;
        TIntfCtrlLoadEv& operator=(TIntfCtrlLoadEv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TMainFrameWnd&          wndMain
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bOverlay
        //      Indicates whether we are loading an overlay or the main
        //      template.
        //
        //  m_strOvrName
        //  m_strTemplate
        //      If we are doing an overlay, we get an overlay name and a
        //      template to load. Else we just get a template to load.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bOverlay;
        TString             m_strOvrName;
        TString             m_strTemplate;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfCtrlLoadEv, TIntfCtrlEv)
};



// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlMiscEv
//  PREFIX: ice
// ---------------------------------------------------------------------------
class TIntfCtrlMiscEv : public TIntfCtrlEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfCtrlMiscEv() = delete;

        TIntfCtrlMiscEv
        (
            const   tCQCKit::EIVMiscOps eOp
        );

        TIntfCtrlMiscEv(const TIntfCtrlMiscEv&) = delete;
        TIntfCtrlMiscEv(TIntfCtrlMiscEv&&) = delete;

        ~TIntfCtrlMiscEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfCtrlMiscEv& operator=(const TIntfCtrlMiscEv&) = delete;
        TIntfCtrlMiscEv& operator=(TIntfCtrlMiscEv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TMainFrameWnd&          wndMain
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eEvent
        //      The miscellaneous operation to invoke.
        // -------------------------------------------------------------------
        tCQCKit::EIVMiscOps   m_eOp;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfCtrlMiscEv, TIntfCtrlEv)
};


// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlSetVarEv
//  PREFIX: ice
// ---------------------------------------------------------------------------
class TIntfCtrlSetVarEv : public TIntfCtrlEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfCtrlSetVarEv() = delete;

        TIntfCtrlSetVarEv
        (
            const   TString&                strVarName
            , const TString&                strValue
        );

        TIntfCtrlSetVarEv(const TIntfCtrlSetVarEv&) = delete;
        TIntfCtrlSetVarEv(TIntfCtrlSetVarEv&&) = delete;

        ~TIntfCtrlSetVarEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfCtrlSetVarEv& operator=(const TIntfCtrlSetVarEv&) = delete;
        TIntfCtrlSetVarEv& operator=(TIntfCtrlSetVarEv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TMainFrameWnd&          wndMain
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strVarName
        //      The global variable to set (has to have the GVar: prefix.
        //
        //  m_strValue
        //      The variable value to set.
        // -------------------------------------------------------------------
        TString m_strVarName;
        TString m_strValue;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfCtrlSetVarEv, TIntfCtrlEv)
};



// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlServer
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TIntfCtrlServer : public TIntfCtrlServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfCtrlServer();

        TIntfCtrlServer(const TIntfCtrlServer&) = delete;
        TIntfCtrlServer(TIntfCtrlServer&&) = delete;

        ~TIntfCtrlServer();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfCtrlServer& operator=(const TIntfCtrlServer&) = delete;
        TIntfCtrlServer& operator=(TIntfCtrlServer&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSpeakText
        (
            const   TString&                strToSpeak
        )   final;

        tCIDLib::TCard4 c4Poll
        (
                    TString&                strBaseTmpl
            ,       TString&                strTopTmpl
            ,       tCIDLib::TBoolean&      bMaximized
        )   final;

        tCIDLib::TVoid DoFrameOp
        (
            const   tCQCKit::EIVFrmOps      eOp
        )   final;

        tCIDLib::TVoid DoMiscOp
        (
            const   tCQCKit::EIVMiscOps     eOp
        )   final;

        tCIDLib::TVoid Invoke() final;

        tCIDLib::TVoid LoadOverlay
        (
            const   TString&                strOvrName
            , const TString&                strToLoad
        )   final;

        tCIDLib::TVoid LoadTemplate
        (
            const   TString&                strToLoad
        )   final;

        tCIDLib::TVoid Navigation
        (
            const   tCQCKit::EScrNavOps     eOps
        )   final;

        tCIDLib::TVoid PlayWAV
        (
            const   TString&                strPath
        )   final;

        tCIDLib::TVoid SetGlobalVar
        (
            const   TString&                strVarName
            , const TString&                strValue
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreStatus
        (
            const   TString&                strBaseTmpl
            , const TString&                strTopTmpl
            , const tCIDLib::TCard4         c4LayerCnt
            , const tCIDLib::TBoolean       bMaximized
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bMaximized
        //      Set if the main frame window is maximized/full screen. The
        //      main frame window sets this on us periodically by calling our
        //      StoreStatus() method.
        //
        //  m_c4LayerCount
        //      The number of layers currently displayed in the interface
        //      window. This is set on us periodically by the main frame
        //      window calling our StoreStatus() method.
        //
        //  m_mtxSync
        //      We have to do a little synchronization between the main frame
        //      window timer calling into here to store the latest state info,
        //      and the ORB threads calling our c4Poll() method to get that
        //      info.
        //
        //  m_strBaseTemplate
        //  m_strTopTemplate
        //      The base and topmost templates currently displayed. These are
        //      set on us periodically by the main frame window calling our
        //      StoreStatus() method.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bMaximized;
        tCIDLib::TCard4     m_c4LayerCount;
        TMutex              m_mtxSync;
        TString             m_strBaseTemplate;
        TString             m_strTopTemplate;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfCtrlServer, TIntfCtrlServerBase)
};

#pragma CIDLIB_POPPACK


