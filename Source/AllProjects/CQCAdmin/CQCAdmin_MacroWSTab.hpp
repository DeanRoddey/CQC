//
// FILE NAME: CQCAdmin_MacroWSTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/15/2015
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
//  This is the header for the CQCClient_MacroWSTab.cpp file, which implements a
//  tab window that just contains a scroll area window, within which is a simple
//  static image window. This is used for read-only viewing of image.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


class TMacroDbgMainWnd;
class TCMLDbgChangeInfo;

// ---------------------------------------------------------------------------
//  CLASS: TMacroWSTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TMacroWSTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMacroWSTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TMacroWSTab(const TMacroWSTab&) = delete;
        TMacroWSTab(TMacroWSTab&&) = delete;

        ~TMacroWSTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMacroWSTab& operator=(const TMacroWSTab&) = delete;
        TMacroWSTab& operator=(TMacroWSTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bProcessAccel
        (
            const   tCIDLib::TVoid* const   pMsgData
        )   const final;

        const TString& strContName() const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TString&                strTabText
        );


    protected  :
        // -------------------------------------------------------------------
        //  Protected, inherited method
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   final;

        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TVoid Destroyed() final;

        tCIDLib::ESaveRes eDoSave
        (
                    TString&                strErr
            , const tCQCAdmin::ESaveModes   eMode
            ,       tCIDLib::TBoolean&      bChangedSaved
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eDbgHandler
        (
                    TCMLDbgChangeInfo&      wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_mecmToUse
        //  m_mefrToUse
        //      The class manager and file resolver taat we want the debugger to
        //      use. It just references objects that we own.
        //
        //  m_pwndIDE
        //      The macro debugger IDE window that does all of the work. We keep
        //      it sized to us.
        //
        //  m_strClassPathRoot
        //      All classes are loaded from a hierarchy underneath a given directory.
        //      This is set by a command line parm, or defaulted to \User if not
        //      provided.
        //
        //  m_strFileSysRoot
        //      We us a simple means of expanding macro paths. We just make them all
        //      exist below a particular directory. This is set by a command line
        //      parm or defaulted if not provided.
        // -------------------------------------------------------------------
        TCQCInterClassMgr           m_mecmToUse;
        TMEngFixedBaseFileResolver  m_mefrToUse;
        TMacroDbgMainWnd*           m_pwndIDE;
        TString                     m_strClassPathRoot;
        TString                     m_strFileSysRoot;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMacroWSTab, TContentTab)
};


