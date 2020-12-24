//
// FILE NAME: CQCGenDrvS_AppCtrl_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2004
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
//  This if the header for the CQCGenDrvS_AppCtrl.cpp file, which implements
//  the TCQCAppCtrlVal and TCQCAppCtrlInfo classes. These are the two classes
//  required to create a new class at the CML level for accessing the app
//  control server. This CML class wraps the client proxy class for the app
//  control IDL interface. The server side of that interface is the app
//  control server program that manages the actual programs.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCAppCtrlVal
//  PREFIX: mecv
// ---------------------------------------------------------------------------
class TCQCAppCtrlVal : public TMEngClassVal
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructors
        // -------------------------------------------------------------------
        TCQCAppCtrlVal
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ClassId
            , const tCIDMacroEng::EConstTypes  eConst
        );

        ~TCQCAppCtrlVal();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDbgFormat
        (
                    TTextOutStream&         strmTarget
            , const TMEngClassInfo&         meciThis
            , const tCIDMacroEng::EDbgFmts     eFormat
            , const tCIDLib::ERadices       eRadix
            , const TCIDMacroEngine&        meOwner
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bConnect();

        tCIDLib::TBoolean bDisconnect
        (
            const   tCIDLib::TBoolean       bCleanup
        );

        tCIDLib::TBoolean bIsConnected();

        tCIDLib::TBoolean bIsRegistered();

        tCIDLib::TBoolean bIsRunning();

        tCIDLib::TBoolean bIsVisible
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        );

        tCIDLib::TBoolean bStart();

        tCIDLib::TBoolean bStart
        (
            const   TString&                strParams
        );

        tCIDLib::TBoolean bStartViaOpen
        (
            const   TString&                strDocFile
        );

        tCIDLib::TBoolean bStop();

        tCIDLib::TCard4 c4AddWindow
        (
            const   TString&                strPath
        );

        tCIDLib::TCard4 c4AddWindowByClass
        (
            const   TString&                strClass
        );

        tCIDLib::TCard4 c4QueryListSel
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        );

        tCIDLib::TCard4 c4QueryText
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            ,       TString&                strToFill
        );

        tCIDLib::TCard4 c4QueryWndStyles
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        );

        tCIDLib::TInt4 i4QueryTrackBar
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        );

        tCIDLib::TInt4 i4SendMsg
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCard4         c4ToSend
            , const tCIDLib::TCard4         c4Param1
            , const tCIDLib::TInt4          i4Param2
            , const tCIDLib::TBoolean       bAsync
        );

        tCIDLib::TVoid Initialize
        (
            const   TString&                strMoniker
            , const TString&                strBinding
            , const TString&                strAppTitle
            , const TString&                strAppName
            , const TString&                strAppPath
            , const TString&                strParams
            , const TString&                strWorkingDir
        );

        TAppCtrlClientProxy* porbcCtrl();

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid SendExtKey
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDCtrls::EExtKeys     eKeyToSend
            , const tCIDLib::TBoolean       bAltShifted
            , const tCIDLib::TBoolean       bCtrlShifted
            , const tCIDLib::TBoolean       bShifted
        );

        tCIDLib::TVoid SendKey
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCh            chToSend
            , const tCIDLib::TBoolean       bAltShifted
            , const tCIDLib::TBoolean       bCtrlShifted
            , const tCIDLib::TBoolean       bShifted
        );

        tCIDLib::TVoid SetFocus
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        );

        tCIDLib::TVoid SetListSel
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCard4         c4Index
        );

        tCIDLib::TVoid SetTrackBar
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TInt4          i4ToSet
        );

        tCIDLib::TVoid SetWindowPos
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TInt4          i4X
            , const tCIDLib::TInt4          i4Y
        );

        tCIDLib::TVoid SetWindowSize
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCard4         c4Width
            , const tCIDLib::TCard4         c4Height
        );

        tCIDLib::TVoid SetWindowText
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const TString&                strText
        );

        tCIDLib::TVoid ShowWindow
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TBoolean       bState
        );

        tCIDLib::TVoid StandardOp
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCQCKit::EStdACOps      eOp
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TCQCAppCtrlVal();
        TCQCAppCtrlVal(const TCQCAppCtrlVal&);
        tCIDLib::TVoid operator=(const TCQCAppCtrlVal&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckConnected();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_porbcCtrl
        //      A client proxy object we create to talk to the app control
        //      server.
        //
        //  m_strAppPath
        //      The full path (on the target machine) of the app to run.
        //
        //  m_strAppTitle
        //      The title to register the app under, for human consumption
        //      on the app control server interface.
        //
        //  m_strBinding
        //      we get a binding name to connect to in our constructor. It's
        //      just the end binding name. We append it to a known name
        //      server scope for app control servers to create the full
        //      binding path.
        //
        //  m_strMoniker
        //      The moniker of the driver that is using us. Since we pass it
        //      in most calls, he provides it to us during ctor to avoid the
        //      overhead of passing it in all the time.
        //
        //  m_strParams
        //      Any parameters that we are supposed to send to the app.
        //
        //  m_strWorkingDir
        //      The working dir (on the target machine) to start the app
        //      in.
        // -------------------------------------------------------------------
        TAppCtrlClientProxy*    m_porbcCtrl;
        TString                 m_strAppPath;
        TString                 m_strAppTitle;
        TString                 m_strBinding;
        TString                 m_strMoniker;
        TString                 m_strParams;
        TString                 m_strWorkingDir;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCAppCtrlVal,TMEngClassVal)
};


// ---------------------------------------------------------------------------
//  CLASS: TCQCAppCtrlInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class TCQCAppCtrlInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static data
        // -------------------------------------------------------------------
        static const TString& strPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCAppCtrlInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        ~TCQCAppCtrlInfo();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Init
        (
                    TCIDMacroEngine&        meOwner
        );

        TMEngClassVal* pmecvMakeStorage
        (
            const   TString&                strName
            ,       TCIDMacroEngine&        meOwner
            , const tCIDMacroEng::EConstTypes  eConst
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInvokeMethod
        (
                    TCIDMacroEngine&        meOwner
            , const TMEngMethodInfo&        methiTarget
            ,       TMEngClassVal&          mecvInstance
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TCQCAppCtrlInfo(const TCQCAppCtrlInfo&);
        tCIDLib::TVoid operator=(const TCQCAppCtrlInfo&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
            , const TString&                strText
        )   const;

        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
        )   const;

        tCIDLib::TVoid TestExcept
        (
                    TCIDMacroEngine&        meOwner
            , const TError&                 errCaught
            , const tCIDLib::TCard4         c4Default
            ,       TCQCAppCtrlVal&         mecvTarget
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2EnumId_XXXX
        //      The class ids for some nested enum classes we create.
        //
        //  m_c2MethId_XXX
        //      To speed up dispatch, we store the ids of the methods that
        //      we set up during init.
        //
        //  m_c4ErrId_XXX
        //      The ids for our error enumeration's values. We store them
        //      as we create the enum, so that we can easily use them later
        //      to throw CML level exceptions.
        //
        //  m_pmeciXXX
        //      The class info objects for some nested enum classes that we
        //      create.
        // -------------------------------------------------------------------
        tCIDLib::TCard2 m_c2EnumId_ButtStates;
        tCIDLib::TCard2 m_c2EnumId_Errors;
        tCIDLib::TCard2 m_c2EnumId_ExtKeys;
        tCIDLib::TCard2 m_c2EnumId_StdOps;
        tCIDLib::TCard2 m_c2MethId_AddWindow;
        tCIDLib::TCard2 m_c2MethId_AddWindowByClass;
        tCIDLib::TCard2 m_c2MethId_ClickButton;
        tCIDLib::TCard2 m_c2MethId_Connect;
        tCIDLib::TCard2 m_c2MethId_DefCtor;
        tCIDLib::TCard2 m_c2MethId_Disconnect;
        tCIDLib::TCard2 m_c2MethId_IsRegistered;
        tCIDLib::TCard2 m_c2MethId_IsRunning;
        tCIDLib::TCard2 m_c2MethId_IsVisible;
        tCIDLib::TCard2 m_c2MethId_QueryButtState;
        tCIDLib::TCard2 m_c2MethId_QueryListSel;
        tCIDLib::TCard2 m_c2MethId_QueryText;
        tCIDLib::TCard2 m_c2MethId_QueryTrackBar;
        tCIDLib::TCard2 m_c2MethId_QueryWndStyles;
        tCIDLib::TCard2 m_c2MethId_SendChildMsg;
        tCIDLib::TCard2 m_c2MethId_SendExtKey;
        tCIDLib::TCard2 m_c2MethId_SendKey;
        tCIDLib::TCard2 m_c2MethId_SendMenuItem;
        tCIDLib::TCard2 m_c2MethId_SendMsg;
        tCIDLib::TCard2 m_c2MethId_SetButtState;
        tCIDLib::TCard2 m_c2MethId_SetFocus;
        tCIDLib::TCard2 m_c2MethId_SetInfo;
        tCIDLib::TCard2 m_c2MethId_SetListSel;
        tCIDLib::TCard2 m_c2MethId_SetTrackBar;
        tCIDLib::TCard2 m_c2MethId_SetWindowPos;
        tCIDLib::TCard2 m_c2MethId_SetWindowSize;
        tCIDLib::TCard2 m_c2MethId_SetWindowText;
        tCIDLib::TCard2 m_c2MethId_ShowWindow;
        tCIDLib::TCard2 m_c2MethId_StandardOp;
        tCIDLib::TCard2 m_c2MethId_StartApp;
        tCIDLib::TCard2 m_c2MethId_StartAppWithParams;
        tCIDLib::TCard2 m_c2MethId_StartViaOpen;
        tCIDLib::TCard2 m_c2MethId_StopApp;
        tCIDLib::TCard4 m_c4ErrId_AddWindow;
        tCIDLib::TCard4 m_c4ErrId_Connect;
        tCIDLib::TCard4 m_c4ErrId_Disconnect;
        tCIDLib::TCard4 m_c4ErrId_NotConnected;
        tCIDLib::TCard4 m_c4ErrId_QueryText;
        tCIDLib::TCard4 m_c4ErrId_QueryVisibility;
        tCIDLib::TCard4 m_c4ErrId_QueryWndStyles;
        tCIDLib::TCard4 m_c4ErrId_Register;
        tCIDLib::TCard4 m_c4ErrId_SendKey;
        tCIDLib::TCard4 m_c4ErrId_SendMsg;
        tCIDLib::TCard4 m_c4ErrId_SetFocus;
        tCIDLib::TCard4 m_c4ErrId_SetSizePos;
        tCIDLib::TCard4 m_c4ErrId_SetText;
        tCIDLib::TCard4 m_c4ErrId_SetVisibility;
        tCIDLib::TCard4 m_c4ErrId_ShowWindow;
        tCIDLib::TCard4 m_c4ErrId_Start;
        tCIDLib::TCard4 m_c4ErrId_StdOp;
        tCIDLib::TCard4 m_c4ErrId_Stop;
        tCIDLib::TCard4 m_c4ErrId_Unknown;
        TMEngEnumInfo*  m_pmeciButtStates;
        TMEngEnumInfo*  m_pmeciErrors;
        TMEngEnumInfo*  m_pmeciExtKeys;
        TMEngEnumInfo*  m_pmeciStdOps;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCAppCtrlInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK


