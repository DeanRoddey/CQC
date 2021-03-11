//
// FILE NAME: CQCTrayMon_AppRecord.hpp
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
//  This is the header for the CQCTrayMon_AppRecord.cpp file, which implements
//  the TAppCtrlRec class. This class maintains the information we need to
//  track about each application under control.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TAppCtrlRec
//  PREFIX: acr
// ---------------------------------------------------------------------------
class TAppCtrlRec : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TAppCtrlRec&            acrSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAppCtrlRec() = delete;

        TAppCtrlRec
        (
            const   TString&                strMoniker
            , const TString&                strTitle
            , const TString&                strApp
            , const TString&                strParams
            , const TString&                strWorkingDir
        );

        TAppCtrlRec(const TAppCtrlRec&) = delete;
        TAppCtrlRec(TAppCtrlRec&&) = delete;

        ~TAppCtrlRec();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAppCtrlRec operator=(const TAppCtrlRec) = delete;
        TAppCtrlRec operator=(TAppCtrlRec&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Activate();

        tCIDLib::TBoolean bIsRunning() const;

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
            , const tCIDLib::EExtProcShows  eShow
        );

        tCIDLib::TBoolean bStop();

        tCIDLib::TBoolean bWaitForDeath
        (
                    tCIDLib::EExitCodes&    eToFill
            , const tCIDLib::TCard4         c4MilliSecs = kCIDLib::c4MaxWait
        )   const;

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

        tCIDLib::EExtProcFlags eFlags() const;

        tCIDLib::EExtProcFlags eFlags
        (
            const   tCIDLib::EExtProcFlags  eToSet
        );

        tCIDLib::EPrioClasses ePriorityClass() const;

        tCIDLib::EPrioClasses ePriorityClass
        (
            const   tCIDLib::EPrioClasses   eNewClass
        );

        tCIDLib::EExitCodes eWaitForDeath
        (
            const   tCIDLib::TCard4         c4MilliSecs = kCIDLib::c4MaxWait
        )   const;

        TExternalProcess& extpThis();

        const TExternalProcess& extpThis() const;

        tCIDCtrls::TWndHandle hwndFindWindow
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

        const TString& strAppName() const;

        const TString& strAppPath() const;

        const TString& strFullAppPath() const;

        const TString& strMoniker() const;

        const TString& strParams() const;

        const TString& strTitle() const;

        const TString& strWorkingDir() const;

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

        tCIDLib::TVoid Update
        (
            const   TString&                strTitle
            , const TString&                strApp
            , const TString&                strParams
            , const TString&                strWorkingDir
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ReplaceTokens
        (
                    TString&                strTarget
        );


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        using TWndMap = TKeyedHashSet<TAppCtrlWndRec,TString,TStringKeyOps>;
        using TWndMapId = TRefVector<TAppCtrlWndRec>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::TWndHandle hwndParsePath
        (
            const   TString&                strPath
            , const TString&                strClass
            ,       tCIDCtrls::TWndId&      widToFill
        );

        tCIDCtrls::TWndHandle hwndTestWindow
        (
                    TAppCtrlWndRec&         acwrTest
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colWndMap
        //  m_colWndMapIds
        //      We have to keep a list of registered windows that the driver
        //      registers with us. We need two views of this. One is a keyed
        //      hash set and the other is a vector. As we add each item to
        //      the keyed hash set, for by window path lookup, we also put
        //      a pointer into the vector. The index into the vector is the
        //      id we give back to the driver, and that's what he passes back
        //      in when he wants to do things with windows.
        //
        //      Obviously the vector must be non-adopting, since it is just
        //      being used to hold a pointer to objects that are owned by
        //      the hash set.
        //
        //  m_eFlags
        //      The external process flags to use. It defaults to a non-
        //      detached process that shares the same console.
        //
        //  m_extpThis
        //      This is the external process object that we use to keep track
        //      of our process instance.
        //
        //  m_strAppName
        //  m_strAppPath
        //      The parts of the path name, broken out for convenience, since
        //      they are displayed separately in many cases. The full path
        //      is stored in m_strFullAppPath.
        //
        //  m_strFullAppPath
        //      The fully qualified path to the application.
        //
        //  m_strMoniker
        //      Since each application is a 'device' from CQC's point of view,
        //      and each device has a driver with a given moniker, this field
        //      stores the moniker of the driver that is controlling this
        //      app, so that we can match incoming commands to a given app.
        //      This is the key field for that reason.
        //
        //  m_strParams
        //      The parameters to pass to the application.
        //
        //  m_strTitle
        //      A title to give to the process for human indentification
        //      purposes.
        //
        //  m_strWorkingDir
        //      The default directory that this program should be started in.
        //      It can be empty if they don't indicate one explicitly.
        // -------------------------------------------------------------------
        TWndMap                 m_colWndMap;
        TWndMapId               m_colWndMapIds;
        tCIDLib::EExtProcFlags  m_eFlags;
        TExternalProcess        m_extpThis;
        TString                 m_strAppName;
        TString                 m_strAppPath;
        TString                 m_strFullAppPath;
        TString                 m_strMoniker;
        TString                 m_strParams;
        TString                 m_strTitle;
        TString                 m_strWorkingDir;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAppCtrlRec,TObject)
};



