//
// FILE NAME: CQCIntfEd_ErrList_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/28/2016
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
//  This is the header for the CQCIntfEd_ErrList.cpp file, which is inserted as the bottom
//  pane of the left side of the main pane container (under the editor.) Errors are output
//  to this window.
//
//  We define a simple publish info format and publish them when an error in our list is
//  either selected or double clicked on. We pass along the event type and the id of the
//  widget that issued the error (if any, it could be a general error.)
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
//  CLASS: TIntfErrListTopic
// ---------------------------------------------------------------------------
class TIntfErrListTopic  : public TObject
{
    public :
        TIntfErrListTopic() = delete;

        TIntfErrListTopic(  const   tCIDCtrls::EListEvents  eEvent
                            , const tCIDLib::TCard4         c4WidgetId) :

            m_c4WidgetId(c4WidgetId)
            , m_eEvent(eEvent)
        {
        }

        ~TIntfErrListTopic() {}

        tCIDLib::TCard4         m_c4WidgetId;
        tCIDCtrls::EListEvents  m_eEvent;

    private :
        RTTIDefs(TIntfErrlistTopic, TObject)
};



// ---------------------------------------------------------------------------
//  CLASS: TIntfErrListWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TIntfErrListWnd : public TGenericWnd
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfErrListWnd();

        TIntfErrListWnd(const TIntfErrListWnd&) = delete;

        ~TIntfErrListWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfErrListWnd& operator=(const TIntfErrListWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ChildFocusChange
        (
            const   TWindow&                wndParent
            , const tCIDCtrls::TWndId       widChild
            , const tCIDLib::TBoolean       bGotFocus
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ClearList();

        tCIDLib::TVoid CreateErrList
        (
            const   TWindow&                wndParent
            , const tCIDCtrls::TWndId       widToUse
        );

        tCIDLib::TVoid LoadErrors
        (
            const   tCQCIntfEng::TErrList&  colErrs
        );

        const TString& strPublishTopic() const
        {
            return m_strPublishTopic;
        }


    protected :
        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid SendErrNot
        (
            const   tCIDCtrls::EListEvents  eEvent
            , const tCIDLib::TCard4         c4Index
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colErrs
        //      Since we remove items as we process them, we have to keep the original list
        //      around. We store the original index as the list row id, so that we can map
        //      back to this list.
        //
        //  m_pstopErrs
        //      Our publish topic for error selection events.
        //
        //  m_pwndList
        //      We get a typed pointer to the list window, which is the only one we need.
        //
        //  m_pwndTitle
        //      Normally we wouldn't care about this static text, but we use it as pseudo
        //      focus indicator.
        //
        //  m_strPublishTopic
        //      We have to create a path for our topic dynamically since there can be
        //      multiple instances of the interface editor loaded at once. We make it
        //      available via a method for others to use to subscribe.
        //
        //  m_strType_xxx
        //      Some common strings we use in our display, so we pre-load them.
        // -------------------------------------------------------------------
        tCQCIntfEng::TErrList   m_colErrs;
        TPubSubTopic            m_pstopErrs;
        TMultiColListBox*       m_pwndList;
        TStaticText*            m_pwndTitle;
        TString                 m_strPublishTopic;
        const TString           m_strType_Error;
        const TString           m_strType_Warning;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfErrListWnd, TGenericWnd)
};

#pragma CIDLIB_POPPACK
