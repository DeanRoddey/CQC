//
// FILE NAME: CQCIntfEd_AttrEditWnd_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/01/2015
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
//  This is the header for the CQCIntfEd_AttrEditWnd.cpp file, which implements a
//  simple derivate of the attribute editor, specialized for our needs. We primarily
//  need to override the bVisEdit() method to provide visual editing for some of our
//  specialized types of data. For the rest we pass them through to the parent.
//
//  We also store the design time id of the widget whose contents we are displaying.
//  This helps avoid some unnecessary full reloading of the window. If we get the
//  same widget again, we just do a refresh instead of a full reload.
//
//  The design id is also set as the user id on our parent, so that it gets passed back
//  in any notifications. This lets the handler find the original widget that the
//  notification is for.
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
//  CLASS: TIntfAttrEdWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TIntfAttrEdWnd : public TAttrEditWnd, private MPubSubscription
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfAttrEdWnd() = delete;

        TIntfAttrEdWnd
        (
                    TCQCIntfEdView* const   pcivToUse
            ,       MIPEIntf* const         pmipeToUse
        );

        TIntfAttrEdWnd(const TIntfAttrEdWnd&) = delete;

        ~TIntfAttrEdWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfAttrEdWnd& operator=(const TIntfAttrEdWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard8 c8CurDesId() const;

        tCIDLib::TVoid ClearWidgetAttrs();

        tCIDLib::TVoid LoadWidgetAttrs
        (
            const   TCQCIntfWidget&         iwdgToLoad
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TBoolean bVisEdit
        (
                    TAttrData&              adatEdit
            , const TArea&                  areaCellScr
            , const tCIDLib::TCard8         c8UserId
        )   override;

        tCIDLib::TVoid ProcessPubMsg
        (
            const   TPubSubMsg&             psmsgIn
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        //
        //  m_c8DesId
        //      The design id of the widget we are displaying, MaxCard if
        //
        //  m_pcivToUse
        //      The main window gives us a pointer to his view object, since we
        //      to access it for some info.
        // -------------------------------------------------------------------
        tCIDLib::TCard8 m_c8DesId;
        TCQCIntfEdView* m_pcivToUse;
};

#pragma CIDLIB_POPPACK

