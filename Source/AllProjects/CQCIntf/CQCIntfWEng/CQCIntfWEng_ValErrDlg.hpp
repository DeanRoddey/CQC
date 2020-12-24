//
// FILE NAME: CQCIntfWEng_ValErrDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/23/2008
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
//  This is the header for the CQCIntfWEng_VarErrDlg.cpp file, which
//  implements a dialog box that is used to display validation errors to
//  the user and let him correct them.
//
//  We will also send a code to the target window any time the user cursors
//  over one of the errors or double clicks on it, so that the containg
//  app can respond to this. The editor uses this to show the user which
//  widget had the error or to allow him to edit the attributes. We use
//  message ids set on our facility class by the containing app.
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
//   CLASS: TCQCValErrDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class CQCINTFWENGEXPORT TCQCValErrDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCValErrDlg();

        TCQCValErrDlg(const TCQCValErrDlg&) = delete;

        ~TCQCValErrDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCValErrDlg& operator=(const TCQCValErrDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
            const   TWindow&                wndOwner
            ,       TWindow* const          pwndReport
            , const tCQCIntfEng::TErrList&  colErrs
            , const TString&                strTmplName
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        const TCQCIntfValErrInfo* pveiFindById
        (
            const   tCIDLib::TCard4         c4Id
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_i4XXXOfs
        //      We remember the offsets of some of the controls from the
        //      bottom of the dialog so that we can correctly move them as
        //      the dialog is resized.
        //
        //  m_pcolErrs
        //      A pointer to the incoing list of errors until we can get it
        //      loaded into the list box.
        //
        //  m_pwndXXXX
        //      Some typed pointers to our widgets. We don't own them, we just
        //      want to avoid lots of downcasting when we use them.
        //
        //  m_pwndReport
        //      The target window that we will post a message back to when
        //      the user select or invokes an item in the error list. The
        //      target window can do whatever it wants with these. They give
        //      us an id to use when we post it back.
        //
        //  m_strTmplName
        //      Storage for the incoming template name until we can get it
        //      loaded into the title bar.
        //
        //  m_szMinTrack
        //      We save the initial size as the minimum size we'll allow our
        //      self to be sized to. It's used in bQuerySizeLimits().
        //
        //  m_szPrevSz
        //      The last size we were, so that we know which way we are moving
        //      when we are resized.
        // -------------------------------------------------------------------
        tCIDLib::TInt4                  m_i4ButtonOfs;
        tCIDLib::TInt4                  m_i4ListHOfs;
        tCIDLib::TInt4                  m_i4ListVOfs;
        tCIDLib::TInt4                  m_i4IconHOfs;
        tCIDLib::TInt4                  m_i4IconVOfs;
        const tCQCIntfEng::TErrList*    m_pcolErrs;
        TPushButton*                    m_pwndCloseButton;
        TListBox*                       m_pwndList;
        TWindow*                        m_pwndReport;
        TString                         m_strTmplName;
        TSize                           m_szPrev;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCValErrDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


