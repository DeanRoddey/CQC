//
// FILE NAME: CQCIntfEd_Undo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/03/2016
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
//  This is the header for the file CQCIntfEd_Undo.cpp file, which implements some
//  derivatives of the standard undo framework's undo item class. These allow us to put
//  interface editor specific undo operations onto the undo stack.
//
//  We have a simple abstract base class that adds our application specific Undo() method,
//  and the others derive from that and implement their undo by doing the appropriate
//  thing to the passed interface template.
//
//
//  The bulk of undo is just based on restoring attributes since the bulk of editing is done
//  via the attribute editor window. This makes it considerably easier because it provides
//  a standard interface by which almost all changes are made. The only things not done
//  via attributes are cut/paste/delete, z-order type stuff, and a few special cases of
//  things too complex to do via attributes, though they are still done via the attribute
//  interface, so we can still catch them at the same place in the editor window.
//
//  It also makes restoral of the change vastly simpler since we would otherwize have to
//  either just save the entire widget or have specialized methods for every type of thing
//  we'd want to be able to put back.
//
//  The editor doesn't allow any operations that would break up a group. Only an ungroup
//  can do that. So it never allows things like doing a delete when part of a group is
//  selected. They have to ungroup in order to delete group members. This avoids a lot of
//  potential confusion.
//
//
//  !!!!
//  We cannot keep around pointers to widgets, since at any point further up the undo stack
//  a widget could be removed and then put back in. So, though we got back to the same
//  place, a previous reference to that widget pushed onto the undo stack would now be
//  invalid.
//
//  So a set of design-time ids are used. These are assigned to the widgets by the designer
//  (from a running counter) so that any widget will have a unique id at design time. Any
//  time a previously created widget is pasted into the template it gets a new one. If a
//  widget is removed and then undo puts it back, it still has its original design time id,
//  so subsequent undo operations that reference it will work correctly.
//
//  In this cases where a widget is cut or copied, we have to make a copy of it, not reference
//  the original, since we can't depend on the original still being there by the time it's
//  pasted back into a template. It may be a completely different template after the original
//  has been closed.
//
//  However there is the 'parent container' pointer that is in the base widget class and
//  used to work back up the parent chain. This is a viewing time thing mostly but it still
//  will be invoked at various points. So any time a widget is either pasted back in or
//  even just an undo of a delete that puts it back in, the parent container pointer should
//  be updated to point to the target template. In theory a delete undo is safe, but don't
//  take any chances.
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
//  CLASS: TBaseIntfUndo
// PREFIX: ui
// ---------------------------------------------------------------------------
class TBaseIntfUndo : public TUndoCmd
{
    public :
        // -------------------------------------------------------------------
        //  Public constructors and destructor
        // -------------------------------------------------------------------
        TBaseIntfUndo(const TBaseIntfUndo&) = delete;

        ~TBaseIntfUndo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBaseIntfUndo& operator=(const TBaseIntfUndo&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TUndoCmd&               uiSrc
        )   const override;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid Undo
        (
                    TAttrEditWnd&           wndAttrEd
            ,       TCQCIntfTemplate&       iwdgTar
        ) = 0;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructor
        // -------------------------------------------------------------------
        TBaseIntfUndo();


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ThrowIdNotFound
        (
            const   tCIDLib::TCard8         c8DesId
            , const tCIDLib::TCard4         c4Line
        );

        tCIDLib::TVoid ThrowZNotFound
        (
            const   tCIDLib::TCard4         c4ZOrder
            , const tCIDLib::TCard4         c4Line
        );


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TBaseIntfUndo,TUndoCmd)
};


// ---------------------------------------------------------------------------
//  CLASS: TIntfAttrUndo
// PREFIX: ui
//
//  This is the most commonly used undo. We just store the original attribute value for
//  later restoral.
// ---------------------------------------------------------------------------
class TIntfAttrUndo : public TBaseIntfUndo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfAttrUndo
        (
            const   tCIDLib::TCard8         c8DesId
            , const TAttrData&              adatNew
            , const TAttrData&              adatOrg
        );

        TIntfAttrUndo(const TIntfAttrUndo&) = delete;

        ~TIntfAttrUndo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfAttrUndo& operator=(const TIntfAttrUndo&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TUndoCmd&               uiSrc
        )   const override;

        tCIDLib::TVoid Undo
        (
                    TAttrEditWnd&           wndAttrEd
            ,       TCQCIntfTemplate&       iwdgTar
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_adatNew
        //  m_adatOrg
        //      The original attribute data that we'll put back, and the one we'll be
        //      putting back. We have to pass the previous value to the widget as well
        //      as the new, and he may check for changes.
        //
        //  m_c8DesId
        //      The design id of the widget affected so we can find it later.
        // -------------------------------------------------------------------
        TAttrData       m_adatNew;
        TAttrData       m_adatOrg;
        tCIDLib::TCard8 m_c8DesId;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfAttrUndo,TBaseIntfUndo)
};


// ---------------------------------------------------------------------------
//  CLASS: TIntfCutUndo
// PREFIX: ui
// ---------------------------------------------------------------------------
class TIntfCutUndo : public TBaseIntfUndo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfCutUndo() = delete;

        TIntfCutUndo
        (
            const   TCQCIntfTemplate&       iwdgSrc
            , const tCQCIntfEng::TDesIdList& fcolCut
        );

        TIntfCutUndo(const TIntfCutUndo&) = delete;

        ~TIntfCutUndo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfCutUndo& operator=(const TIntfCutUndo&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TUndoCmd&               uiSrc
        )   const override;

        tCIDLib::TVoid Undo
        (
                    TAttrEditWnd&           wndAttrEd
            ,       TCQCIntfTemplate&       iwdgTar
        )   override;


    private :
        // -------------------------------------------------------------------
        //  We need to sort the list by their z-order, so that when we re-
        //  insert them via their z-order, the indices will be correct.
        // -------------------------------------------------------------------
        typedef TKeyObjPair<const TCQCIntfWidget*, tCIDLib::TCard4>   TSortItem;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompSortItem
        (
            const   TSortItem&              kobj1
            , const TSortItem&              kobj2
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colCut
        //      A collection of widgets that were cut. We make copies of the widgets and
        //      keep them around. We own them and so this guy is an adopting collection.
        //
        //  m_fcolZOrders
        //      The original Z-Orders of the widgets that were cut, so that we can
        //      put them back in in those positions.
        // -------------------------------------------------------------------
        tCQCIntfEng::TChildList         m_colCut;
        tCIDLib::TCardList              m_fcolZOrders;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfCutUndo,TBaseIntfUndo)
};



// ---------------------------------------------------------------------------
//  CLASS: TIntfFullUndo
// PREFIX: ui
//
//  This one is used to save and restore the entire contents of a template. Which we use
//  for things that would require more individual bits than is worth it.
// ---------------------------------------------------------------------------
class TIntfFullUndo : public TBaseIntfUndo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfFullUndo
        (
            const   TCQCIntfTemplate&       iwdgSave
        );

        TIntfFullUndo(const TIntfFullUndo&) = delete;

        ~TIntfFullUndo();


        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TIntfFullUndo& operator=(const TIntfFullUndo&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TUndoCmd&               uiSrc
        )   const override;

        tCIDLib::TVoid Undo
        (
                    TAttrEditWnd&           wndAttrEd
            ,       TCQCIntfTemplate&       iwdgTar
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_iwdgSave
        //      We just make a copy of the whole template.
        // -------------------------------------------------------------------
        TCQCIntfTemplate    m_iwdgSave;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfFullUndo,TBaseIntfUndo)
};



// ---------------------------------------------------------------------------
//  CLASS: TIntfInsertUndo
// PREFIX: ui
// ---------------------------------------------------------------------------
class TIntfInsertUndo : public TBaseIntfUndo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfInsertUndo() = delete;

        TIntfInsertUndo
        (
            const   tCQCIntfEng::TDesIdList& fcolInserted
        );

        TIntfInsertUndo(const TIntfInsertUndo&) = delete;

        TIntfInsertUndo
        (
            const   TCQCIntfWidget&         iwdgInserted
        );

        ~TIntfInsertUndo();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TIntfInsertUndo& operator=(const TIntfInsertUndo&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TUndoCmd&               uiSrc
        )   const override;

        tCIDLib::TVoid Undo
        (
                    TAttrEditWnd&           wndAttrEd
            ,       TCQCIntfTemplate&       iwdgTar
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fcolInserted
        //      The list of design ids for the widgets that were inserted, so
        //      tht we can find them later to remove them.
        // -------------------------------------------------------------------
        tCQCIntfEng::TDesIdList m_fcolInserted;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfInsertUndo,TBaseIntfUndo)
};



// ---------------------------------------------------------------------------
//  CLASS: TIntfLockUndo
// PREFIX: ui
//
//  This one is used to restore one or more widget's lock state. What we do is just
//  store the design time ids and the state to restore.
// ---------------------------------------------------------------------------
class TIntfLockUndo : public TBaseIntfUndo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfLockUndo() = delete;

        TIntfLockUndo
        (
            const   tCQCIntfEng::TDesIdList& fcolChanged
            , const tCIDLib::TBoolean       bState
        );

        TIntfLockUndo
        (
            const   tCIDLib::TCard8         c8Changed
            , const tCIDLib::TBoolean       bState
        );

        TIntfLockUndo(const TIntfLockUndo&) = delete;

        ~TIntfLockUndo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfLockUndo& operator=(const TIntfLockUndo&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TUndoCmd&               uiSrc
        )   const override;

        tCIDLib::TVoid Undo
        (
                    TAttrEditWnd&           wndAttrEd
            ,       TCQCIntfTemplate&       iwdgTar
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bState
        //      The state to restore.
        //
        //  m_fcolChanged
        //      The design ids of all of the widgets in their original order.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bState;
        tCQCIntfEng::TDesIdList m_fcolChanged;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfLockUndo,TBaseIntfUndo)
};



// ---------------------------------------------------------------------------
//  CLASS: TIntfMultiPosUndo
// PREFIX: ui
//
//  This is one is used when multiple widgets are sized/moved at once. We just store
//  their original positions and design time ids. On undo, we just update the areas
//  of the widgets.
// ---------------------------------------------------------------------------
class TIntfMultiPosUndo : public TBaseIntfUndo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfMultiPosUndo() = delete;

        TIntfMultiPosUndo
        (
                    TCQCIntfTemplate&       iwdgTar
            , const tCQCIntfEng::TDesIdList& fcolChanged
        );

        TIntfMultiPosUndo(const TIntfMultiPosUndo&) = delete;

        ~TIntfMultiPosUndo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfMultiPosUndo& operator=(const TIntfMultiPosUndo&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TUndoCmd&               uiSrc
        )   const override;

        tCIDLib::TVoid Undo
        (
                    TAttrEditWnd&           wndAttrEd
            ,       TCQCIntfTemplate&       iwdgTar
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fcolChanged
        //      The design ids of all of the affected widgets.
        //
        //  m_colAreas
        //      The original areas of the widgets we are going to undo. We have one
        //      for each selected widget, in the same order.
        // -------------------------------------------------------------------
        tCQCIntfEng::TDesIdList     m_fcolChanged;
        TVector<TArea>              m_colAreas;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfMultiPosUndo,TBaseIntfUndo)
};


// ---------------------------------------------------------------------------
//  CLASS: TIntfSelectUndo
// PREFIX: ui
//
//  This is one is used when the selection changes. We don't need to do anything, since
//  the base class saves the selected widgets, and the editor window restores that saved
//  list of selected widgets after each undo step.
// ---------------------------------------------------------------------------
class TIntfSelectUndo : public TBaseIntfUndo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfSelectUndo() = delete;

        TIntfSelectUndo
        (
            const   tCQCIntfEng::TDesIdList& fcolSelected
        );

        TIntfSelectUndo(const TIntfSelectUndo&) = delete;

        ~TIntfSelectUndo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfSelectUndo& operator=(const TIntfSelectUndo&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TUndoCmd&               uiSrc
        )   const override;

        tCIDLib::TVoid Undo
        (
                    TAttrEditWnd&           wndAttrEd
            ,       TCQCIntfTemplate&       iwdgTar
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const tCQCIntfEng::TDesIdList& fcolSelected() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fcolSelected
        //      The list of selected items at the time this was pushed.
        // -------------------------------------------------------------------
        tCQCIntfEng::TDesIdList m_fcolSelected;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfSelectUndo,TBaseIntfUndo)
};



// ---------------------------------------------------------------------------
//  CLASS: TIntfZOrderUndo
// PREFIX: ui
//
//  This one is used to restore one or more widget's zorder. What we do is just store the
//  design time ids of all of the widgets in z-order. So, to restore, no matter what the
//  cirumstances, we can just put them back in that order. This avoids copying widgets
//  and specialized operations. One generic scheme handles anything.
// ---------------------------------------------------------------------------
class TIntfZOrderUndo : public TBaseIntfUndo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfZOrderUndo() = delete;

        TIntfZOrderUndo
        (
            const   TCQCIntfTemplate&       iwdgTmpl
        );

        TIntfZOrderUndo(const TIntfZOrderUndo&) = delete;

        ~TIntfZOrderUndo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfZOrderUndo& operator=(const TIntfZOrderUndo&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TUndoCmd&               uiSrc
        )   const override;

        tCIDLib::TVoid Undo
        (
                    TAttrEditWnd&           wndAttrEd
            ,       TCQCIntfTemplate&       iwdgTar
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fcolOrgOrder
        //      The design ids of all of the widgets in their original order.
        // -------------------------------------------------------------------
        tCQCIntfEng::TDesIdList     m_fcolOrgOrder;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfZOrderUndo,TBaseIntfUndo)
};

#pragma CIDLIB_POPPACK

