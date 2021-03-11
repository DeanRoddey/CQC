//
// FILE NAME: CQCTrayMon_AppCtrlEvents.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/06/2004
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
//  This is the header for the CQCTrayMon_AppCtrlEvents.cpp file, which
//  implements a set of event classes that the background (ORB) server
//  threads handling incoming app control requests will queue up on the
//  app control tab window to be processed. Most of the things that the
//  app control interface has to do can only be done within a GUI thread
//  context.
//
//  Because of the variety of them, we can't have just one big base class,
//  so we have to have a polymorphic hierarchy and use a by ref queue to
//  store them until processed. Each derived class overrides the ProcessEv()
//  method and does whatever is appropriate.
//
//  Note that almost all of these are oriented towards an installed app
//  control driver, i.e. it's an app control driver sending a command to
//  the app it controls. So most of them take a moniker. But a few things
//  are for general use, such as invoking files or starting apps generically
//  without a driver. Some are supported by both and will have ctors that
//  both do and don't take a moniker.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


class TCQCTrayAppCtrlTab;


// ---------------------------------------------------------------------------
//   CLASS: TAppUtilEv
//  PREFIX: aue
// ---------------------------------------------------------------------------
class TAppUtilEv : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TAppUtilEv() = delete;

        TAppUtilEv(const TAppUtilEv&) = delete;
        TAppUtilEv(TAppUtilEv&&) = delete;

        ~TAppUtilEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAppUtilEv& operator=(const TAppUtilEv&) = delete;
        TAppUtilEv& operator=(TAppUtilEv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid ProcessEv
        (
                    TCQCTrayAppCtrlTab&     wndTab
            ,       TAppCtrlServerImpl&     orbsAC
        ) = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ChildId() const;

        tCIDLib::TCard4 c4Id() const;

        const TString& strMoniker() const;

    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TAppUtilEv
        (
            const   TString&                strMoniker
        );

        TAppUtilEv
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4ChildId
        //  m_c4Id
        //      If the operation targets a given window or child of a given
        //      window, these are set to indicate the relevant ids.
        //
        //  m_strMoniker
        //      The moniker of the application for which this change even was
        //      posted. This lets the window know which item in it's list box
        //      to update.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4ChildId;
        tCIDLib::TCard4         m_c4Id;
        TString                 m_strMoniker;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAppUtilEv,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TAppFocusEv
//  PREFIX: aue
// ---------------------------------------------------------------------------
class TAppFocusEv : public TAppUtilEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAppFocusEv() = delete;

        TAppFocusEv
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
        );

        TAppFocusEv(const TAppFocusEv&) = delete;
        TAppFocusEv(TAppFocusEv&&) = delete;

        ~TAppFocusEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAppFocusEv& operator=(const TAppFocusEv&) = delete;
        TAppFocusEv& operator=(TAppFocusEv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TCQCTrayAppCtrlTab&     wndTab
            ,       TAppCtrlServerImpl&     orbsAC
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAppFocusEv, TAppUtilEv)
};


// ---------------------------------------------------------------------------
//   CLASS: TAppInvFileEv
//  PREFIX: aue
//
//  This supports either a regular invoke of a file, or a start of a driver
//  by invoking some file that will cause it to run.
// ---------------------------------------------------------------------------
class TAppInvFileEv : public TAppUtilEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAppInvFileEv() = delete;

        TAppInvFileEv
        (
            const   TString&                strMoniker
            , const TString&                strPath
            , const tCIDLib::EExtProcShows  eShow
        );

        TAppInvFileEv
        (
            const   TString&                strPath
            , const tCIDLib::EExtProcShows  eShow
        );

        TAppInvFileEv(const TAppInvFileEv&) = delete;
        TAppInvFileEv(TAppInvFileEv&&) = delete;

        ~TAppInvFileEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAppInvFileEv& operator=(const TAppInvFileEv&) = delete;
        TAppInvFileEv& operator=(TAppInvFileEv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TCQCTrayAppCtrlTab&     wndTab
            ,       TAppCtrlServerImpl&     orbsAC
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eShow
        //      The initial show state to use.
        //
        //  m_strMoniker
        //      If this is intended to start up a defined app, then this will
        //      be the moniker. Otherwise, it's just for a generic operation.
        //
        //  m_strPath
        //      The path to the file to invoke
        // -------------------------------------------------------------------
        tCIDLib::EExtProcShows  m_eShow;
        TString                 m_strMoniker;
        TString                 m_strPath;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAppInvFileEv, TAppUtilEv)
};



// ---------------------------------------------------------------------------
//   CLASS: TAppRemStartEv
//  PREFIX: aue
//
//  This one handles the generic invocation of a program, never associated
//  with a driver. And we don't keep up with it after it is started.
// ---------------------------------------------------------------------------
class TAppRemStartEv : public TAppUtilEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAppRemStartEv() = delete;

        TAppRemStartEv
        (
            const   TString&                strPath
            , const TString&                strParams
            , const TString&                strInitPath
        );

        TAppRemStartEv(const TAppRemStartEv&) = delete;
        TAppRemStartEv(TAppRemStartEv&&) = delete;

        ~TAppRemStartEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAppRemStartEv& operator=(const TAppRemStartEv&) = delete;
        TAppRemStartEv& operator=(TAppRemStartEv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TCQCTrayAppCtrlTab&     wndTab
            ,       TAppCtrlServerImpl&     orbsAC
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strInitPath
        //      The optional initial path (on the target machine) to star the
        //      app in.
        //
        //  m_strPath
        //  m_strParams
        //      The path to run (on the target machine) and any parameters to
        //      pass to it.
        // -------------------------------------------------------------------
        TString m_strInitPath;
        TString m_strPath;
        TString m_strParams;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAppRemStartEv, TAppUtilEv)
};



// ---------------------------------------------------------------------------
//   CLASS: TAppSendExtKeyEv
//  PREFIX: aue
// ---------------------------------------------------------------------------
class TAppSendExtKeyEv : public TAppUtilEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAppSendExtKeyEv() = delete;

        TAppSendExtKeyEv
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDCtrls::EExtKeys     eToSend
            , const tCIDLib::TBoolean       bAltShift
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
        );

        TAppSendExtKeyEv(const TAppSendExtKeyEv&) = delete;
        TAppSendExtKeyEv(TAppSendExtKeyEv&&) = delete;

        ~TAppSendExtKeyEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAppSendExtKeyEv& operator=(const TAppSendExtKeyEv&) = delete;
        TAppSendExtKeyEv& operator=(TAppSendExtKeyEv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TCQCTrayAppCtrlTab&     wndTab
            ,       TAppCtrlServerImpl&     orbsAC
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
        RTTIDefs(TAppSendExtKeyEv, TAppUtilEv)
};



// ---------------------------------------------------------------------------
//   CLASS: TAppSendKeyEv
//  PREFIX: aue
// ---------------------------------------------------------------------------
class TAppSendKeyEv : public TAppUtilEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAppSendKeyEv() = delete;

        TAppSendKeyEv
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDLib::TCh            chToSend
            , const tCIDLib::TBoolean       bAltShift
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
        );

        TAppSendKeyEv(const TAppSendKeyEv&) = delete;
        TAppSendKeyEv(TAppSendKeyEv&&) = delete;

        ~TAppSendKeyEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAppSendKeyEv& operator=(const TAppSendKeyEv&) = delete;
        TAppSendKeyEv& operator=(TAppSendKeyEv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TCQCTrayAppCtrlTab&     wndTab
            ,       TAppCtrlServerImpl&     orbsAC
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAltShift
        //  m_bCtrlShift
        //      Indicates whether the key should be sent with shift keys
        //      pressed.
        //
        //  m_chToSend
        //      The character to send.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bAltShift;
        tCIDLib::TBoolean   m_bCtrlShift;
        tCIDLib::TBoolean   m_bShift;
        tCIDLib::TCh        m_chToSend;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAppSendKeyEv, TAppUtilEv)
};



// ---------------------------------------------------------------------------
//   CLASS: TAppSendMsgEv
//  PREFIX: aue
// ---------------------------------------------------------------------------
class TAppSendMsgEv : public TAppUtilEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAppSendMsgEv() = delete;

        TAppSendMsgEv
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCIDCtrls::TWndMsg      wmsgToSend
            , const tCIDCtrls::TWParam      wParm
            , const tCIDCtrls::TLParam      lParm
        );

        TAppSendMsgEv(const TAppSendMsgEv&) = delete;
        TAppSendMsgEv(TAppSendMsgEv&&) = delete;

        ~TAppSendMsgEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAppSendMsgEv& operator=(const TAppSendMsgEv&) = delete;
        TAppSendMsgEv& operator=(TAppSendMsgEv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TCQCTrayAppCtrlTab&     wndTab
            ,       TAppCtrlServerImpl&     orbsAC
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_lParm
        //  m_wParm
        //      The parameters that we should send.
        //
        //  m_wmsgToSend
        //      The message id to send.
        // -------------------------------------------------------------------
        tCIDCtrls::TLParam  m_lParm;
        tCIDCtrls::TWParam  m_wParm;
        tCIDCtrls::TWndMsg  m_wmsgToSend;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAppSendMsgEv, TAppUtilEv)
};



// ---------------------------------------------------------------------------
//   CLASS: TAppStartEv
//  PREFIX: aue
// ---------------------------------------------------------------------------
class TAppStartEv : public TAppUtilEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAppStartEv() = delete;

        TAppStartEv
        (
            const   TString&                strMoniker
        );

        TAppStartEv
        (
            const   TString&                strMoniker
            , const TString&                strParams
        );

        TAppStartEv(const TAppStartEv&) = delete;
        TAppStartEv(TAppStartEv&&) = delete;

        ~TAppStartEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAppStartEv& operator=(const TAppStartEv&) = delete;
        TAppStartEv& operator=(TAppStartEv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TCQCTrayAppCtrlTab&     wndTab
            ,       TAppCtrlServerImpl&     orbsAC
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  They can optionall provide parameters that override the ones
        //  registered with the app.
        // -------------------------------------------------------------------
        TString m_strParams;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAppStartEv, TAppUtilEv)
};



// ---------------------------------------------------------------------------
//   CLASS: TAppStdOpEv
//  PREFIX: aue
// ---------------------------------------------------------------------------
class TAppStdOpEv : public TAppUtilEv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAppStdOpEv() = delete;

        TAppStdOpEv
        (
            const   TString&                strMoniker
            , const tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ChildId
            , const tCQCKit::EStdACOps      eOp
        );

        TAppStdOpEv(const TAppStdOpEv&) = delete;
        TAppStdOpEv(TAppStdOpEv&&) = delete;

        ~TAppStdOpEv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAppStdOpEv& operator=(const TAppStdOpEv&) = delete;
        TAppStdOpEv& operator=(TAppStdOpEv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessEv
        (
                    TCQCTrayAppCtrlTab&     wndTab
            ,       TAppCtrlServerImpl&     orbsAC
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eOp
        //      The standard operation that we are to do
        // -------------------------------------------------------------------
        tCQCKit::EStdACOps  m_eOp;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAppStdOpEv, TAppUtilEv)
};

