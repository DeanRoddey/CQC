//
// FILE NAME: CQCIntfEng_Template.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/16/2001
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
//  This class is a special widget type that implements the actual template
//  itself, i.e. the background of the template. It provides some methods that
//  apply to the template as a whole, though a lot of that stuff is provided
//  by our parent container class. So this guy is a container but it always
//  represents the top level container, either the base template or a popup
//  template. This is also the guy that gets persisted when templates are
//  saved.
//
//  There's lot of recursive stuff going on (at runtime, not at design time),
//  since templates can contain overlays, into which other templates get
//  loaded at runtime. Overlays are also derived from the container class.
//  They have no content at design time, but at runtime they are loaded with
//  other templates.
//
//  We provide one of the focus management methods, but it mostly is just
//  using stuff in the underlying container method to do it's work. We provide
//  a flag for supressing focus (set by default) that the containing window
//  will look at (in the base-most template) and just decline to pass on any
//  keyboard operations if focus is disabled. The setting of this flag in
//  any templates loaded into overlays or popups os overridden by the setting
//  in the base-most template. So it's either all on or all off.
//
//  We can support a background image, so we use the image interface mixin. However,
//  to avoid excessive overhead, we make a copy of our own which is pre-scaled to
//  the needed size/AR. We have to watch for commands that would change the image
//  and update our separate one.
//
//  We support a few different types of schemes for invoking actions:
//
//  1)  A set of triggered events that can be invoked by incoming events from
//      drivers and other sources.
//  2)  A set of template level actions which can be invoked by name from the
//      child widgets, to allow for the creation of reusable logic. Input to
//      and output from these actions is via global variables.
//  3)  A set of hot keys that can be mapped to actions, which is mainly
//      intended to support RIVA clients running on systems that have hard
//      buttons that need to be able drive actions. There is also an optional
//      'otherwise' action that is run if the incoming hot key is not mapped
//      in that template.
//
//  The triggered and hot key events are invoked recursively through overlays,
//  but never go further than the top-most template/popup. So they can only
//  target the active template. The template level actions are invoked by
//  the widgets of that template themselves, and they can only do that if
//  the user interacts with them, so that has to be the active template.
//
//  We also provide a timer that will periodically go off and cause an
//  OnTimeout event to be generated on the template. This can be just every
//  X seconds or after X seconds of inactivity. So you can configure an
//  action in the OnTimeout event of the template and have it run periodically
//  to do something.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


class TCQCIntfTmplCfg;


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTemplate
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfTemplate

    : public TCQCIntfContainer
    , public MCQCIntfImgIntf
    , public MCQCCmdSrcIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfTemplate();

        TCQCIntfTemplate
        (
            const   TString&                strName
        );

        TCQCIntfTemplate
        (
            const   TCQCIntfTemplate&       iwdgSrc
        );

        TCQCIntfTemplate(TCQCIntfTemplate&&) = delete;

        ~TCQCIntfTemplate();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfTemplate& operator=
        (
            const   TCQCIntfTemplate&       iwdgSrc
        );

        TCQCIntfTemplate& operator=(TCQCIntfTemplate&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSizeTo() const override;

        tCIDLib::TBoolean bHandleHotKey
        (
            const   tCQCIntfEng::EHotKeys   eKey
        )   override;

        tCIDLib::TBoolean bInvokeTrgEvents
        (
            const   tCIDLib::TBoolean        bIsNight
            , const tCQCKit::TCQCEvPtr&      cptrEv
            , const tCIDLib::TCard4          c4Hour
            , const tCIDLib::TCard4          c4Minute
            ,       TTrigEvCmdTarget&        ctarEvent
        )   override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TBoolean bReferencesTarId
        (
            const   TString&                strId
        )   const override;

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

        tCIDLib::TVoid QueryContentSize
        (
                    TSize&                  szToFill
        )   override;

        tCIDLib::TVoid QueryReferencedImgs
        (
                    tCIDLib::TStrHashSet&   colToFill
            ,       tCIDLib::TStrHashSet&   colScopes
            , const tCIDLib::TBoolean       bIncludeSysImgs
            , const tCIDLib::TBoolean       bIncludeScopes
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid RefreshImages
        (
                    TDataSrvClient&         dsclToUse
        )   override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        tCIDLib::TVoid ResetWidget() override;

        TSize szDefaultSize() const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid UpdateCmdTarget
        (
            const   TString&                strOldTargetId
            , const TString&                strNewTargetName
            , const TString&                strNewTargetId
            ,       tCQCIntfEng::TDesIdList& fcolAffected
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bActivityBasedTO() const;

        tCIDLib::TBoolean bEventsEnabled() const;

        tCIDLib::TBoolean bEventsEnabled
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIsPopupType() const;

        tCIDLib::TBoolean bUserFlag() const
        {
            return m_bUserFlag;
        }

        tCIDLib::TBoolean bUserFlag(const tCIDLib::TBoolean bToSet)
        {
            m_bUserFlag = bToSet;
            return m_bUserFlag;
        }

        tCIDLib::TBoolean bInitTemplate
        (
                    TCQCIntfView* const     civOwner
            ,       TCQCPollEngine&         polleToUse
            ,       TDataSrvClient&         dsclInit
            , const TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            , const TString&                strInvokeParms
        );

        tCIDLib::TBoolean bNoFocus() const;

        tCIDLib::TBoolean bNoFocus
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard1 c1ConstOpacity() const;

        tCIDLib::TCard1 c1ConstOpacity
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        tCIDLib::TCard4 c4AppData() const
        {
            return m_c4AppData;
        }

        tCIDLib::TCard4 c4AppData(const tCIDLib::TCard4 c4ToSet)
        {
            m_c4AppData = c4ToSet;
            return m_c4AppData;
        }

        tCIDLib::TCard4 c4GetNextGroupId();

        tCIDLib::TCard4 c4HandlerId() const
        {
            return m_c4HandlerId;
        }

        tCIDLib::TCard4 c4HandlerId(const tCIDLib::TCard4 c4ToSet)
        {
            m_c4HandlerId = c4ToSet;
            return m_c4HandlerId;
        }

        tCIDLib::TCard4 c4TimeoutSecs() const
        {
            return m_c4TimeoutSecs;
        }

        tCIDLib::TCard4 c4TimeoutSecs(const tCIDLib::TCard4 c4ToSet)
        {
            m_c4TimeoutSecs = c4ToSet;
            return m_c4TimeoutSecs;
        }

        const tCQCIntfEng::TActList& colActions() const;

        tCQCIntfEng::TActList& colActions();

        const tCQCIntfEng::THKActList& colHotKeyActions() const;

        tCQCIntfEng::THKActList& colHotKeyActions();

        const tCQCEvCl::TTrgEvList& colTrgEvents() const;

        tCQCEvCl::TTrgEvList& colTrgEvents();

        tCIDLib::TVoid DoExtClick
        (
            const   TPoint&                 pntAt
        );

        tCIDLib::TVoid DoUpdateFixes();

        tCIDLib::TEncodedTime enctLastTimeout() const;

        tCIDLib::TEncodedTime enctLastTimeout
        (
            const   tCIDLib::TEncodedTime   enctToSet
        );

        tCQCIntfEng::EValTypes eDoValidation
        (
                tCQCIntfEng::TErrList&      colErrs
        );

        tCQCKit::EUserRoles eMinRole() const;

        tCQCKit::EUserRoles eMinRole
        (
            const   tCQCKit::EUserRoles     eToSet
        );

        tCQCIntfEng::ETmplTypes eType() const;

        tCQCIntfEng::ETmplTypes eType
        (
            const   tCQCIntfEng::ETmplTypes eType
        );

        TCQCIntfWidget* piwdgMoveFocus
        (
            const   tCQCKit::EScrNavOps     eOp
            ,       TCQCIntfWidget*&        piwdgOld
        );

        tCIDLib::TVoid ResetTemplate
        (
                    TCQCIntfView* const     pcivOwner
        );

        const TString& strNotes() const;

        const TString& strNotes
        (
            const   TString&                strToSet
        );

        const TString& strLoadWAV() const;

        const TString& strLoadWAV
        (
            const   TString&                strToSet
        );

        const TString& strUnloadWAV() const;

        const TString& strUnloadWAV
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        )   override;

        tCIDLib::TVoid DoOnExit
        (
                    TCQCIntfView&           civOwner
        )   override;

        tCIDLib::TVoid DoOnLoad
        (
                    TCQCIntfView&           civOwner
        )   override;

        tCIDLib::TVoid DoOnPreload
        (
                    TCQCIntfView&           civOwner
            , const TString&                strInvokeParms
        )   ;

        tCIDLib::TVoid SizeChanged
        (
            const   TSize&                  szNew
            , const TSize&                  szOld
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SwizzleCmds();

        tCIDLib::TVoid SwizzleOne
        (
            const   TString&                strTarId
            ,       MCQCCmdSrcIntf&         mcsrcToUpdate
        );

        // Temporary to handle 2.5 conversion stuff
        tCIDLib::TVoid ConvertXlats
        (
            const   TVector<TKeyValuePair>& colFXlats
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bActivityBasedTO
        //      Used in conjunction with m_enctLastTimeout below. Indicates if the timeout
        //      seconds means that many seconds after no user activity, or that many seconds
        //      since the last timeout, which will be stored in m_enctLastTimeout.
        //
        //  m_bNoFocus
        //      The interface system supports a focus mechanism, so that a particular
        //      input widget has the focus, and will display some indication of that status.
        //      But for some purposes, that focus mechanism isn't wanted, and just adds
        //      complexity, so it can be turned off.
        //
        //      This applies to the top level template currently loaded and will affect any
        //      overlays loaded into it, whatever their own setting for this flag.
        //
        //  m_bUserFlag
        //      A runtime only flag for applications to use, such as the editor or auto-
        //      generation tools.
        //
        //  m_c1ConstOpacity
        //      If we are a popup, we can be rendered with a constant opacity value in addition
        //      to any per-pixel opacity. This is runtime only, set on us by the window that
        //      creates us, and ignored if we aren't a popup.
        //
        //  m_c4AppData
        //      This is for the application's use of it is convenient. It's not persisted.
        //
        //  m_c4HandlerId
        //      The action running window that we use to invoke actions will take a handler
        //      id that it just passes back out in some call backs. We use it so that we can
        //      match incoming callbacks to process cmd against an interface target to the
        //      template it belongs to (base or one of the popups.) So the windows sets
        //      this on us as required and we pass it in when posting actions. This is
        //      runtime only, not persisted.
        //
        //  m_c4NextGroupId
        //      The next id to be used if widgets are grouped. We set this after streaming in
        //      our contents, by just looking through the list and finding the highest
        //      previously used id number and setting this to that plus 1. They only have to
        //      be unique within each template.
        //
        //  m_c4TimeoutSecs
        //      An action command is provided to set an inactivity timer that can auto-close
        //      a popup after a certain period. By default it's set to 0 which means it's
        //      ignored. It's not persisted, it's only used at runtime.
        //
        //  m_colActions
        //      Our list of template level actions, if any. Often it will be empty since
        //      none will be configured.
        //
        //  m_colHotKeyActions
        //      A list of actions that map to the hot keys. We just create the list the
        //      same size as the hot ke enum, so the enum is the index into the list, and
        //      use the same standard command src that we use for template level actions.
        //
        //  m_colTrgEvents
        //      We can have one or more triggered events that can be kicked off by incoming
        //      event triggers.
        //
        //  m_enctLastTimeout
        //      This is set any time a timeout event is issued. The view that checks for
        //      them will update this if it issues such an event. It's used to remember
        //      when it's time to do the next one (when they are not based on inactivity
        //      time.) This is not persisted. It's used with m_bActivityBasedTO.
        //
        //  m_eMinRole
        //      The minimum user role required to access this template. This is used to
        //      manage access to templates. It is set by the administrator during design
        //      of the template. It is persisted.
        //
        //  m_eType
        //      The caller who created us will tell us if we are a regular template, a
        //      popup, or a popout. We store it here for any situations where we, our
        //      contained widgets, or the window that owns us needs to react differently
        //      based on that. This is runtime only, not persisted.
        //
        //  m_strNotes
        //      A text field for the designer of the template to remind himself of various
        //      things. This is persisted of course.
        //
        //  m_strLoadWAV
        //  m_strUnloadWAV
        //      WAV files can be configured to play on open/close of the template. These are
        //      persisted.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bActivityBasedTO;
        tCIDLib::TBoolean       m_bNoFocus;
        tCIDLib::TBoolean       m_bUserFlag;
        tCIDLib::TCard1         m_c1ConstOpacity;
        tCIDLib::TCard4         m_c4AppData;
        tCIDLib::TCard4         m_c4HandlerId;
        tCIDLib::TCard4         m_c4NextGroupId;
        tCIDLib::TCard4         m_c4TimeoutSecs;
        tCQCIntfEng::TActList   m_colActions;
        tCQCIntfEng::THKActList m_colHotKeyActions;
        tCQCEvCl::TTrgEvList    m_colTrgEvents;
        tCIDLib::TEncodedTime   m_enctLastTimeout;
        tCQCKit::EUserRoles     m_eMinRole;
        tCQCIntfEng::ETmplTypes m_eType;
        TString                 m_strNotes;
        TString                 m_strLoadWAV;
        TString                 m_strUnloadWAV;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfTemplate,TCQCIntfContainer)
        DefPolyDup(TCQCIntfTemplate)
};


#pragma CIDLIB_POPPACK

