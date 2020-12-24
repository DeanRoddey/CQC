//
// FILE NAME: CQCTreeBrws_RemBrowseDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/24/2015
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
//  This is the header for the browser dialog. It provides a way to select individual
//  resource types. We need to reuse the tree browser, since we can't afford to create
//  a completely different mechanism for that. So this guy uses the tree browser in a
//  dialog, and allows for browsing within just one section of the hierarchy, i.e. one
//  data server data type. So mainly this just uses features provided by the
//  CQCTreeBrws/CQCRemBrws facilities.
//
//  Since we only deal with single data types, a fully qualified path for us starts with
//  /User or /System. We get told the data type we are dealing with, and that is used
//  to set up a filter that will cause only that part of the overall data server hierarchy
//  to be loaded.
//
//  The paths we get and set on the browser window will be in the form:
//
//      /Customize/[datatype]/User
//      /Customize/[datatype]/System
//
//  Since it always works in terms of the whole hierarchy. But we just insure that only
//  the part of it that represents out data gets loaded into the browser. When the user
//  makes a selection, we get the type relative part of that to give back.
//
//  We optionally take a 'relative to' path. If provided, then we show a check box that
//  let's the user choose whether to get a full path back, or a path relative to the
//  the relative to path.
//
//  SO ANYWAY, there are two different levels of fully qualified vs. relative going on
//  here. Those between what the browser window displays and what we get/give back, to
//  callers who are always wanting the type relative part. And between those type relative
//  paths which may be fully qualified (/User or /System) or relative to another type
//  relative path (so /User/MyTemplates as a base and ./Popups/Confirm relative to that.)
//
//  If the incoming path doesn't start with /User or /System, they MUST provide a relative
//  path so that we can figure out what the starting path actually is. If the incoming is
//  relative we default our check box to return a relative path, else we default it to
//  full.
//
//
//  For images, we display an image preview to the right and a button to let the user
//  display the currently selected image in the preview control. If not images, we delete
//  that guy and size the dialog down to show just the browser control.
//
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
//   CLASS: TRemBrwsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TRemBrwsDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TRemBrwsDlg();

        TRemBrwsDlg(const TRemBrwsDlg&) = delete;

        ~TRemBrwsDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TRemBrwsDlg& operator=(const TRemBrwsDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const TString&                strPath
            , const TCQCUserCtx&            cuctxToUse
            , const tCQCRemBrws::EDTypes    eDType
            , const tCQCTreeBrws::EFSelFlags eFlags
            ,       TString&                strSelected
            ,       MRemBrwsBrwsCB* const   prbbcEvents
            , const TString* const          pstrRelToPath = nullptr
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
        tCIDLib::TBoolean bStorePath();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eBrwsHandler
        (
                    TTreeBrowseInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bOrgRelative
        //      Upon entry, if the incoming path is relative we have to expand it before
        //      we store it in m_strPath. But we need to remember if it was relative since
        //      that becomes our default return format.
        //
        //  m_eFlags
        //      The various flags that the caller passed us to control how we operate.
        //
        //  m_eDType
        //      The caller has to tell us the data type we are to handle, which we will
        //      verify by looking at the path.
        //
        //  m_eRes
        //      To store the result code for return.
        //
        //  m_prbbcEvents
        //      The containing application can provide us a callback handler. We'll use
        //      this to pass on any events we get from the browser window.
        //
        //  m_pstrRelToPath
        //      This is optional. If provided, we allow the caller to select whether he
        //      wants to get a fully qualified path back, or one relative to the this
        //      relative to path. If this is not provided, we always return a fully
        //      qualified path.
        //
        //  m_pwndXXX
        //      Type pointers to some of our controls, for convenience. We don't own them.
        //
        //  m_strPath
        //      This is the initial path we start on. It's either a default if non was
        //      provided by the caller, or it's a path that is expanded relative to the
        //      provided 'relative to' path. Then we expand it out to a full hierarchial
        //      path which is what the browser window wants.
        //
        //  m_strSelected
        //      The selected path is returned here.
        //
        //  m_strTitle
        //      They give us the title text to display. We store it here until we can get
        //      it loaded.
        //
        //  m_strTopLimit
        //      The selected path must be here or down in the hierarchy, i.e. it must
        //      start with this path prefix.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bOrgRelative;
        TCQCUserCtx                 m_cuctxToUse;
        tCQCTreeBrws::EFSelFlags    m_eFlags;
        tCQCRemBrws::EDTypes        m_eDType;
        tCIDLib::EOpenRes           m_eRes;
        MRemBrwsBrwsCB*             m_prbbcEvents;
        const TString*              m_pstrRelToPath;
        TPushButton*                m_pwndAccept;
        TPushButton*                m_pwndCancel;
        TPushButton*                m_pwndCopyPath;
        TStaticText*                m_pwndCurPath;
        TStaticText*                m_pwndCurPreview;
        TImgPreviewWnd*             m_pwndImgPreview;
        TCheckBox*                  m_pwndRelPath;
        TPushButton*                m_pwndShowPreview;
        TCQCTreeTypeBrowser*        m_pwndTree;
        TString                     m_strPath;
        TString                     m_strSelected;
        TString                     m_strTitle;
        TString                     m_strTopLimit;


        // -------------------------------------------------------------------
        //  Private, static data members
        //
        //  m_colLastTypePaths
        //      We remember the last selected path of each data type. If there's no
        //      explicit path provided as a starting path, then use this one. This will
        //      very often make a big difference in usability as a practical matter.
        //
        //      Some of the types will never be accessed via this dialog, but that's fine
        //      they'll just stay empty.
        // -------------------------------------------------------------------
        static TVector<TString, tCQCRemBrws::EDTypes> s_colLastTypePaths;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TRemBrwsDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



