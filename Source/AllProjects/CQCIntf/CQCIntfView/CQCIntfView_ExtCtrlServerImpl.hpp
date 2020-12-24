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
        ~TIntfCtrlEv();


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
        //  Unimplemented
        // -------------------------------------------------------------------
        TIntfCtrlEv(const TIntfCtrlEv&);
        tCIDLib::TVoid operator=(const TIntfCtrlEv&);


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
        TIntfCtrlExtKeyEv
        (
            const   tCIDCtrls::EExtKeys     eToSend
            , const tCIDLib::TBoolean       bAltShift
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
        );

        ~TIntfCtrlExtKeyEv();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TMainFrameWnd&          wndMain
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TIntfCtrlExtKeyEv();
        TIntfCtrlExtKeyEv(const TIntfCtrlExtKeyEv&);
        tCIDLib::TVoid operator=(const TIntfCtrlExtKeyEv&);


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
        TIntfCtrlFrameOp
        (
            const   tCQCKit::EIVFrmOps      eOp
        );

        ~TIntfCtrlFrameOp();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TMainFrameWnd&          wndMain
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TIntfCtrlFrameOp();
        TIntfCtrlFrameOp(const TIntfCtrlFrameOp&);
        tCIDLib::TVoid operator=(const TIntfCtrlFrameOp&);


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
        TIntfCtrlLoadEv
        (
            const   TString&                strTemplate
        );

        TIntfCtrlLoadEv
        (
            const   TString&                strOvrName
            , const TString&                strTemplate
        );

        ~TIntfCtrlLoadEv();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TMainFrameWnd&          wndMain
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TIntfCtrlLoadEv();
        TIntfCtrlLoadEv(const TIntfCtrlLoadEv&);
        tCIDLib::TVoid operator=(const TIntfCtrlLoadEv&);


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
        TIntfCtrlMiscEv
        (
            const   tCQCKit::EIVMiscOps eOp
        );

        ~TIntfCtrlMiscEv();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TMainFrameWnd&          wndMain
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TIntfCtrlMiscEv();
        TIntfCtrlMiscEv(const TIntfCtrlMiscEv&);
        tCIDLib::TVoid operator=(const TIntfCtrlMiscEv&);


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
        TIntfCtrlSetVarEv
        (
            const   TString&                strVarName
            , const TString&                strValue
        );

        ~TIntfCtrlSetVarEv();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TMainFrameWnd&          wndMain
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TIntfCtrlSetVarEv();
        TIntfCtrlSetVarEv(const TIntfCtrlSetVarEv&);
        tCIDLib::TVoid operator=(const TIntfCtrlSetVarEv&);


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

        ~TIntfCtrlServer();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSpeakText
        (
            const   TString&                strToSpeak
        );

        tCIDLib::TCard4 c4Poll
        (
                    TString&                strBaseTmpl
            ,       TString&                strTopTmpl
            ,       tCIDLib::TBoolean&      bMaximized
        );

        tCIDLib::TVoid DoFrameOp
        (
            const   tCQCKit::EIVFrmOps      eOp
        );

        tCIDLib::TVoid DoMiscOp
        (
            const   tCQCKit::EIVMiscOps     eOp
        );

        tCIDLib::TVoid Invoke();

        tCIDLib::TVoid LoadOverlay
        (
            const   TString&                strOvrName
            , const TString&                strToLoad
        );

        tCIDLib::TVoid LoadTemplate
        (
            const   TString&                strToLoad
        );

        tCIDLib::TVoid Navigation
        (
            const   tCQCKit::EScrNavOps     eOps
        );

        tCIDLib::TVoid PlayWAV
        (
            const   TString&                strPath
        );

        tCIDLib::TVoid SetGlobalVar
        (
            const   TString&                strVarName
            , const TString&                strValue
        );


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
        tCIDLib::TVoid Initialize();

        tCIDLib::TVoid Terminate();


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


