//
// FILE NAME: CQCIntfEng_Container.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/16/2006
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
//  This is the public header for the CQCIntfEng_Container.cpp file. This file
//  implements the base class from which any container of widgets is derived (such as
//  templates or overlays.)
//
//  We provide almost all the functionalty of containers, mostly related to
//  holding a list of children and providng ways to find them in various ways
//  and to iterate them. We we provide various methods that we just turn
//  around and recursively appy to our children.
//
//  A lot of our methods are recursive, which will do us and all of the nested stuff
//  that we contain. And we also override some of the virtuals of the base widget
//  class and make them recursive as well.
//
//  We hold the states, if any. In the case of overlays, it is set from the states
//  of the templates loaded into the overlay.
//
//  We provide various methods just for the designer, to add/remove widgets, modify
//  the states, and mostly the widget finding methods are just used by the designer.
//  But, to the degree that they are used by the viewer, some of them take a flag
//  that indicates recursive or non-recursive searching. In the designer it doesn't
//  matter since only one level is ever being edited in a given window. At viewing
//  time we often need to search the whole tree.
//
// CAVEATS/GOTCHAS:
//
//  1)  The order that the children are added to a container defines their
//      'z-order', with the first one being the furthest back, and later
//      ones being further 'forward' (i.e. would overdraw them if they
//      overlap.)
//
//      This is also the tab order for moving the focus, which moves forward
//      'upwards' in the z-order (back-tab does the opposite.)
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfContainer
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfContainer : public TCQCIntfWidget
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfContainer();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bHandleHotKey
        (
            const   tCQCIntfEng::EHotKeys   eKey
        );

        virtual tCIDLib::TBoolean bInvokeTrgEvents
        (
            const   tCIDLib::TBoolean       bIsNight
            , const tCQCKit::TCQCEvPtr&     cptrEv
            , const tCIDLib::TCard4         c4Hour
            , const tCIDLib::TCard4         c4Minute
            ,       TTrigEvCmdTarget&       ctarEvent
        );

        virtual TCQCIntfWidget* piwdgHit
        (
            const   TPoint&                 pntTest
            , const tCQCIntfEng::EHitFlags  eFlags = tCQCIntfEng::EHitFlags::Widgets
        );

        virtual tCIDLib::TVoid UpdateCmdTarget
        (
            const   TString&                strOldTargetId
            , const TString&                strNewTargetName
            , const TString&                strNewTargetId
            ,       tCQCIntfEng::TDesIdList& fcolAffected
        )   = 0;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TBoolean bReferencesTarId
        (
            const   TString&                strId
        )   const override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid QueryMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
        )   const override;

        tCIDLib::TVoid QueryReferencedImgs
        (
                    tCIDLib::TStrHashSet&   colToFill
            ,       tCIDLib::TStrHashSet&   colScopes
            , const tCIDLib::TBoolean       bIncludeSysImgs
            , const tCIDLib::TBoolean       bIncludeScopes
        )   const override;

        tCIDLib::TVoid RegisterFields
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        )   override;

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

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        TArea areaHull() const;

        tCIDLib::TVoid AddChild
        (
                    TCQCIntfWidget* const   piwdgToAdopt
            , const tCIDLib::EEnds          eEnd = tCIDLib::EEnds::Last
        );

        tCIDLib::TVoid AddChildAt
        (
                    TCQCIntfWidget* const   piwdgToAdopt
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid AdoptChildren
        (
                    tCQCIntfEng::TChildList& colChildren
        );

        tCIDLib::TBoolean bCheckForNonScrollables() const;

        tCIDLib::TBoolean bCheckForInvalidStates
        (
                    tCIDLib::TStrHashSet&   colBadList
        )   const;

        tCIDLib::TBoolean bChangeGroupZOrder
        (
            const   tCIDLib::TCard4         c4GroupId
            , const tCQCIntfEng::EZOrder    eMove
            ,       tCIDLib::TCard4&        c4NewZ
        );

        tCIDLib::TBoolean bChangeWidgetZOrder
        (
                    TCQCIntfWidget* const   piwdgToMove
            , const tCQCIntfEng::EZOrder    eMove
            ,       tCIDLib::TCard4&        c4NewZ
        );

        tCIDLib::TBoolean bContainsWidget
        (
            const   TCQCIntfWidget* const   piwdgFind
            , const tCIDLib::TBoolean       bRecurse = kCIDLib::False
        )   const;

        tCIDLib::TBoolean bDesIdExists
        (
            const   tCIDLib::TCard8         c8ToCheck
        )   const;

        tCIDLib::TBoolean bDrawChildren
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaTest
            , const TGUIRegion&             grgnClip
            , const TCQCIntfWidget* const   piwdgStopAt = nullptr
            , const tCIDLib::TBoolean       bStopBefore = kCIDLib::False
        );

        tCIDLib::TBoolean bDrawUnder
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaTest
            , const TCQCIntfWidget* const   piwdgStopAt = nullptr
            , const tCIDLib::TBoolean       bStopBefore = kCIDLib::False
        );

        tCIDLib::TBoolean bDrawUnder
        (
                    TGraphDrawDev&          gdevTarget
            , const TCQCIntfWidget* const   piwdgUnder
            , const tCIDLib::TBoolean       bStopBefore = kCIDLib::False
        );

        tCIDLib::TBoolean bFindGroupMembers
        (
            const   tCIDLib::TCard4         c4GroupId
            ,       tCQCIntfEng::TDesIdList& fcolToFill
            ,       tCIDLib::TCard4&        c4FirstZ
        )   const;

        tCIDLib::TBoolean bFindGroupMembers
        (
            const   tCIDLib::TCard4         c4GroupId
            ,       tCQCIntfEng::TChildList& colToFill
            ,       tCIDLib::TCard4&        c4FirstZ
        );

        tCIDLib::TBoolean bFindGroupMembers
        (
            const   TString&                strGrpName
            ,       tCQCIntfEng::TDesIdList& fcolToFill
            ,       tCIDLib::TCard4&        c4FirstZ
        );

        tCIDLib::TBoolean bFindGroupMembers
        (
            const   TString&                strGrpName
            ,       tCQCIntfEng::TChildList& colToFill
            ,       tCIDLib::TCard4&        c4FirstZ
        );

        tCIDLib::TBoolean bGroupNameUsed
        (
            const   TString&                strToCheck
            , const TCQCIntfWidget* const   piwdgExcept = nullptr
        )   const;

        tCIDLib::TBoolean bLoadSiblingOverlay
        (
            const   TString&                strOverlayId
            , const TString&                strToLoad
            ,       tCQCIntfEng::TErrList&  colErrs
        );

        tCIDLib::TBoolean bNextPrevInsertPnt
        (
            const   tCIDLib::TCard4         c4StartAt
            ,       tCIDLib::TCard4&        c4Insert
            , const tCIDLib::TBoolean       bForward
        )   const;

        tCIDLib::TBoolean bStateExists
        (
            const   TString&                strToFind
        )   const;

        tCIDLib::TBoolean bStateExists
        (
            const   tCIDLib::TCard4         c4Index
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bWidgetIdUsed
        (
            const   TString&                strToCheck
            , const TCQCIntfWidget* const   piwdgExcept = nullptr
        )   const;

        tCIDLib::TCard4 c4AssignDesIdsFrom
        (
            const   tCIDLib::TCard4         c4From
        );

        tCIDLib::TCard4 c4ChildCount() const;

        tCIDLib::TCard4 c4FindFirstGroupZ
        (
            const   tCIDLib::TCard4         c4GroupId
        )   const;

        tCIDLib::TCard4 c4WdgZOrder
        (
            const   TCQCIntfWidget* const   piwdgToFind
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        )   const;

        tCIDLib::TVoid ChangeZOrder
        (
                    TCQCIntfWidget* const   piwdgToMove
            , const tCIDLib::TCard4         c4To
        );

        tCIDLib::TVoid ClearStates();

        tCIDLib::TVoid DeleteStates
        (
            const   tCIDLib::TStrCollect&   colToRemove
        );

        tCIDLib::TVoid DoOnExits
        (
                    TCQCIntfView&           civOwner
        );

        tCIDLib::TVoid DoOnLoads
        (
                    TCQCIntfView&           civOwner
        );

        tCIDLib::TVoid DoPostInits();

        tCIDLib::TVoid DoPreloads
        (
                    TCQCIntfView&           civOwner
            , const TString&                strInvokeParms
        );

        tCQCKit::EUserRoles eMinRole() const;

        tCQCKit::EUserRoles eMinRole
        (
            const   tCQCKit::EUserRoles     eToSet
        );

        tCIDLib::TVoid ExpandTmplPath
        (
            const   TString&                strOrg
            ,       TString&                strExpPath
        )   const;

        tCIDLib::TVoid FindAllFocusable
        (
                    tCQCIntfEng::TChildList& colToFill
        );

        tCIDLib::TVoid FindAllTargets
        (
                    tCQCKit::TCmdTarList&   colToFill
            , const tCIDLib::TBoolean       bRecurse
            , const TCQCIntfWidget* const   piwdgIgnore = nullptr
        );

        tCIDLib::TVoid FindAllTargets
        (
                    tCQCKit::TCCmdTarList&  colToFill
            , const tCIDLib::TBoolean       bRecurse
            , const TCQCIntfWidget* const   piwdgIgnore = nullptr
        )   const;

        const TCQCIntfStateList& istlStates() const;

        const TCQCIntfStateList& istlStates
        (
            const   TCQCIntfStateList&      istlToSet
        );

        TCQCIntfWidget& iwdgAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TCQCIntfWidget& iwdgAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid InitialScan
        (
                    TCQCPollEngine&         polleToUse
            ,       TStdVarsTar&            ctarGlobals
        );

        tCIDLib::TVoid MakeRelTmplPath
        (
            const   TString&                strOrg
            ,       TString&                strRelPath
        )   const;

        tCIDLib::TVoid OrphanGrpMember
        (
                    TCQCIntfWidget* const   piwdgToOrphan
            ,       tCIDLib::TCard4&        c4GroupZ
            ,       tCIDLib::TBoolean&      bGrpDestroyed
        );

        TCQCIntfWidget* piwdgFindByDesId
        (
            const   tCIDLib::TCard8         c8ToFind
        );

        const TCQCIntfWidget* piwdgFindByDesId
        (
            const   tCIDLib::TCard8         c8ToFind
        )   const;

        TCQCIntfWidget* piwdgFindByDesId
        (
            const   tCIDLib::TCard8         c8ToFind
            ,       tCIDLib::TCard4&        c4ZOrder
        );

        const TCQCIntfWidget* piwdgFindByDesId
        (
            const   tCIDLib::TCard8         c8ToFind
            ,       tCIDLib::TCard4&        c4ZOrder
        )   const;

        const TCQCIntfWidget* piwdgFindByName
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bRecurse
        )   const;

        TCQCIntfWidget* piwdgFindByName
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bRecurse
        );

        TCQCIntfWidget* piwdgFindByUID
        (
            const   tCIDLib::TCard4         c4ToFind
            , const tCIDLib::TBoolean       bRecurse
        );

        TCQCIntfWidget* piwdgFindCurFocus();

        TCQCIntfWidget* piwdgFindCurFocus
        (
                    TCQCIntfContainer*&     piwdgParent
        );

        const TCQCIntfWidget* piwdgFindRunFlags
        (
            const   tCQCIntfEng::ERunFlags eToFind
            , const tCQCIntfEng::ERunFlags eMask
        )   const;

        const TCQCIntfWidget* piwdgFindRunFlags
        (
            const   tCQCIntfEng::ERunFlags  eToFind
            , const tCQCIntfEng::ERunFlags  eMask
            , const TCQCIntfContainer*&     piwdgParent
        )   const;

        TCQCIntfWidget* piwdgFindRunFlags
        (
            const   tCQCIntfEng::ERunFlags eToFind
            , const tCQCIntfEng::ERunFlags eMask
        );

        TCQCIntfWidget* piwdgFindRunFlags
        (
            const   tCQCIntfEng::ERunFlags  eToFind
            , const tCQCIntfEng::ERunFlags  eMask
            ,       TCQCIntfContainer*&     piwdgParent
        );

        TCQCIntfWidget* piwdgNextPrevFocus
        (
                    TCQCIntfWidget* const   piwdgCurrent
            , const tCIDLib::TBoolean       bNext
        );

        tCIDLib::TVoid PublishWidgetChanged
        (
            const   TCQCIntfWidget* const   piwdgChanged
        );

        tCIDLib::TVoid QueryAllDesIds
        (
                    tCQCIntfEng::TDesIdList& fcolToFill
            , const tCIDLib::TBoolean       bIncludeLocked
        );

        tCIDLib::TVoid QueryFldWidgets
        (
                    tCQCIntfEng::TWidgetList& colToFill
        );

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid ReplaceWidget
        (
            const   tCIDLib::TCard8         c8DesId
            , const TCQCIntfWidget&         iwdgReplacement
        );

        tCIDLib::TVoid RemoveByDesId
        (
            const   tCIDLib::TCard8         c8ToFind
        );

        tCIDLib::TVoid RemoveByName
        (
            const   TString&                strName
        );

        tCIDLib::TVoid RemoveWidget
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid RemoveWidget
        (
                    TCQCIntfWidget* const   piwdgToRemove
        );

        tCIDLib::TVoid SetGlobalRunFlags
        (
            const   tCQCIntfEng::ERunFlags  eBitsToSet
            , const tCQCIntfEng::ERunFlags  eMask
        );

        tCIDLib::TVoid SetAllZOrder
        (
            const tCQCIntfEng::TDesIdList&  fcolOrder
        );

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        TSize szMaxChildExtent();

        const TString& strPublishTopic() const
        {
            return m_colChildren.strTopicPath();
        }

        const TString& strTemplateName() const;

        const TString& strTemplateName
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid TakeFrom
        (
                    TCQCIntfContainer&      iwdgSrc
            , const tCIDLib::TBoolean       bSetParOrg
        );

        tCIDLib::TVoid TermWidgets();


        //
        //  A typesafe by name lookup helper. one allows optional recursion and
        //  the other never recurses (not needed at design time.)
        //
        template <typename T>
        T* piwdgCastByName( const   TString&            strName
                            , const tCIDLib::TBoolean   bRecurse)
        {
            TCQCIntfWidget* piwdgRet = piwdgFindByName(strName, bRecurse);
            if (piwdgRet == nullptr)
            {
                ThrowNotFound(strName);

                // Make the analyzer happy
                return nullptr;
            }
            if (!piwdgRet->bIsA(T::clsThis()))
                ThrowCastErr(strName, T::clsThis().strClassName());
            return static_cast<T*>(piwdgRet);
        }

        template <typename T> T* piwdgCastByName(const TString& strName)
        {
            TCQCIntfWidget* piwdgRet = piwdgFindByName(strName, kCIDLib::False);
            if (piwdgRet == nullptr)
            {
                ThrowNotFound(strName);

                // Make the analyzer happy
                return nullptr;
            }
            if (!piwdgRet->bIsA(T::clsThis()))
                ThrowCastErr(strName, T::clsThis().strClassName());
            return static_cast<T*>(piwdgRet);
        }


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfContainer
        (
            const   tCQCIntfEng::ECapFlags  eCaps
            , const TString&                strTypeName
            , const tCIDLib::TCard4         c4InitStyle = 0
        );

        TCQCIntfContainer
        (
            const   TString&                strName
            , const TArea&                  areaInit
            , const tCQCIntfEng::ECapFlags  eCaps
            , const TString&                strTypeName
            , const tCIDLib::TCard4         c4InitStyle = 0
        );

        TCQCIntfContainer
        (
            const   TCQCIntfContainer&      iwdgSrc
        );

        TCQCIntfContainer(TCQCIntfContainer&&) = delete;

        TCQCIntfContainer& operator=
        (
            const   TCQCIntfContainer&      iwdgSrc
        );

        TCQCIntfContainer& operator=(TCQCIntfContainer&&) = delete;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActiveUpdate
        (
            const   tCIDLib::TBoolean       bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
        )   override;

        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        )   override;

        tCIDLib::TVoid DisplayStateChanged()  override;

        tCIDLib::TVoid ScrPosChanged
        (
            const   TArea&                  areaTmplWnd
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid ValueUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bVarsUpdate
            , const TStdVarsTar&            ctarGlobalVars
            , const TGUIRegion&             grgnClip
        )   override;


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid AdjustChildPos
        (
            const   TPoint&                 pntNew
            ,       TPoint&                 pntAdj
        );

        virtual tCIDLib::TVoid DoOnExit
        (
                    TCQCIntfView&           civOwner
        ) = 0;

        virtual tCIDLib::TVoid DoOnLoad
        (
                    TCQCIntfView&           civOwner
        ) = 0;

        virtual tCIDLib::TVoid DoOnPreload
        (
                    TCQCIntfView&           civOwner
            , const TString&                strInvokeParms
        ) = 0;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunTrgEvents
        (
            const   tCQCEvCl::TTrgEvList&   colEvents
            , const tCIDLib::TBoolean       bIsNight
            , const tCQCKit::TCQCEvPtr&     cptrEv
            , const tCIDLib::TCard4         c4Hour
            , const tCIDLib::TCard4         c4Minute
            ,       TTrigEvCmdTarget&       ctarEvent
        );

        tCIDLib::TVoid QueryAllWithCapFlags
        (
            const   tCQCIntfEng::ECapFlags eToFind
            , const tCQCIntfEng::ECapFlags eMask
            ,       tCQCIntfEng::TChildList& colToFill
        );

        tCIDLib::TVoid RecursiveLoad
        (
                    TCQCIntfView* const     pcivOwner
            ,       TDataSrvClient&         dsclToUse
            , const TCQCFldCache&           cfcData
        );

        tCIDLib::TVoid RemoveAllChildren();

        tCIDLib::TVoid RemoveRecursiveLinks();

        tCIDLib::TVoid ValidateCmds
        (
            const   TString&                strActType
            , const TCQCIntfWidget&         iwdgSrc
            , const MCQCCmdSrcIntf&         mcsrcVal
            ,       tCQCIntfEng::TErrList&  colErrs
        )   const;


        //
        //  This is just for streaming in pre-2.5 stuff. It can be thrown away
        //  after the move to 3.0. The template class will, after streaming us
        //  in (and it'll know if we are of the pre-2.5 variety) translate these
        //  to an action based scheme on himself. So we don't write them back
        //  out in the 2.5 world.
        //
        TVector<TKeyValuePair>      m_colFXlats;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CommonInit();

        tCIDLib::TVoid EvaluateStates
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        );

        tCIDLib::TVoid ThrowCastErr
        (
            const   TString&                strName
            , const TString&                strClassName
        );

        tCIDLib::TVoid ThrowNotFound
        (
            const   TString&                strName
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colChildren
        //      The list of child widgets at this level. It is of course an adopting
        //      collection that owns the widgets. If we are in designer mode, we set
        //      up a unique topic path and enable publishing on this collection, so that
        //      the editor window can subscribe to changes.
        //
        //  m_pistlStates
        //      The state list, which holds the stuff we need to evaluate
        //      any states associated within this interface. This guys is
        //      a pointer so that we can keep all of the implementation
        //      internal. It's here since the overlay and template classes that
        //      derive from us both support them.
        //
        //  m_strTemplateName
        //      A runtime value, which is set to the path of the template that
        //      is loaded into the derived template or overlay class. This is
        //      used for error messages, for save operations in the editor
        //      and so forth.
        // -------------------------------------------------------------------
        tCQCIntfEng::TChildList m_colChildren;
        TCQCIntfStateList*      m_pistlStates;
        TString                 m_strTemplateName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfContainer, TCQCIntfWidget)
};


#pragma CIDLIB_POPPACK

