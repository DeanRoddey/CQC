//
// FILE NAME: CQCTreeBrws_BrowseIntf.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/25/2015
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
//  This file defines an abstract based class that is used for all browsing of
//  remote (CQC back end) based resources, and possibly for some other types of
//  resources as well since it is so generic.
//
//  The way browsers interact with the admin client is defined as:
//
//  1.  The browsers handle the addition, deletion, and renaming of tree items in
//      their section of the hierarchy. If the application provides a callback handler
//      it will be informed about these changes. In the case of delete and rename, it
//      will be given a chance to reject the operation, via the callback. It may choose
//      to reject them if files are open that would be affected.
//
//  2.  Browser objects tell the browser window if they can accept new files, which is
//      used to enable drag and drop. bAcceptsNew() is called to ask if the browser
//      object can accept files of particular extensions. If so, then the drop operation
//      is initiated. At the end, AcceptFiles() is called to let it import any of the
//      dropped files it can handle.
//
//  3.  When the browser window sees a menu request from the user, it will call the
//      bDoMenuAction() method. The browser can return false if it doesn't support a
//      menu or a menu on the thing clicked or in the current state. Or, it can present
//      a menu and handle the result internally. If this returns true, that indicates
//      that the called browser has filled in the provided notification event, which
//      will be sent to the contaiing application.
//
//  3.  If the browser window gets an accelerator message, it will pass it on to the
//      browser object for the part of the tree that has the selection. It calls the
//      bProcessMenuSelection method, which decides what to do. bDoMenuAction() calls
//      this method as , so that all of the menu/accel command drive operations can be
//      kept in one place. So the last thing bDoMenuAction does is call this. Here
//      gain, if it returns true that means that the notification event has been filled
//      in and it will be sent to the containing application.
//
//  4.  Browser are called to load the contents of scopes in their part of the tree.
//      The top level scope for each browser object is initially added, and set to
//      virtual. When expanded, LoadScope() is called on the handling browser object.
//      It can load that scope only, and leave any child scopes virtual for later
//      handling (via LoadScope), or load its whole contents, as desired.
//
//  5.  bDoRename is called if the user tries to invoke in-place editing of an item.
//      There is a default implementation of this that most browser will use. It will
//      first call bCanRename() to ask the derived browser if that item can be renamed
//      at all. If it says no, then false is returned. If it's allowed, then it
//      invokes the inplace editing operation. If the user commits to that, it then
//      calls bRenameItem() to let the browser do the rename operation.
//
//      If that succeeds, returns true, then the containing application is called back
//      to let it know.
//
//      If for some reason is is required, the browser object could override the
//      bDoRename() and change this default behavior.
//
//  6.  Some browsers have bgn threads that monitor a server or servers. These may
//      have to interact with the tree view to update content or status of existing
//      content. So the browser window provides a 'post-back' scheme. Drived browser
//      can call QueueChange() on the browser window and it will queue those changes
//      up. Then ProcessChanges() can be called and that will make the browser window
//      post itself an async code, in which it will then grab those changes and call
//      back on the browser object to let it handle the change in the GUI thread
//      context. It calls ProcessChange() and passes the next change to handle.
//
//      Even if no bgn thread is involved, a browser object may have a reason to have
//      an async post back to itself in some cases.
//
//  7.  If the user double clicks an item in the tree, then the handling browser object
//      for the clicked item will be called on bReportInvocation(). If it returns
//      true, then the containing application is called back, which can react to that.
//      Usually it's treated the same as an edit type menu selection, though it could
//      be treated as view menu selection if there is no edit operation available for
//      the thing selected.
//
//      If the thing selected cannot be edited or viewed, it can just return false to
//      do nothing in response to the double click.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TTreeBrowseInfo;
class TCQCTreeBrowser;


// ---------------------------------------------------------------------------
//   CLASS: TCQCTreeBrws
//  PREFIX: rbrws
// ---------------------------------------------------------------------------
class CQCTREEBRWSEXP TCQCTreeBrwsIntf : public TObject, public MIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCTreeBrwsIntf() = delete;

        TCQCTreeBrwsIntf(const TCQCTreeBrwsIntf&) = delete;
        TCQCTreeBrwsIntf(TCQCTreeBrwsIntf&&) = delete;

        ~TCQCTreeBrwsIntf();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCTreeBrwsIntf& operator=(const TCQCTreeBrwsIntf&) = delete;
        TCQCTreeBrwsIntf& operator=(TCQCTreeBrwsIntf&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIPEValidate
        (
            const   TString&                strSrc
            ,       TAttrData&              adatTar
            , const TString&                strNewVal
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard8&        c8UserId
        )   const override;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid AcceptFiles
        (
            const   TString&                strParScope
            , const tCIDLib::TStrList&      colTarNames
            , const tCIDLib::TStrList&      colSrcPaths
        )   = 0;

        virtual tCIDLib::TBoolean bAcceptsNew
        (
            const   TString&                strPath
            , const tCIDLib::TStrHashSet&   colExts
        )   const = 0;

        virtual tCIDLib::TBoolean bDoMenuAction
        (
            const   TString&                strPath
            ,       TTreeBrowseInfo&        wnotToSend
        ) = 0;

        virtual tCIDLib::TBoolean bDoRename
        (
            const   TString&                strPath
            ,       TTreeBrowseInfo&        wnotToSend
        );

        virtual tCIDLib::TBoolean bIsConnected() const = 0;

        virtual tCIDLib::TBoolean bProcessMenuSelection
        (
            const   tCIDLib::TCard4         c4CmdId
            , const TString&                strPath
            ,       TTreeBrowseInfo&        wnotToSend
        ) = 0;

        virtual tCIDLib::TBoolean bReportInvocation
        (
            const   TString&                strPath
            ,       tCIDLib::TBoolean&      bAsEdit
        )   const = 0;

        virtual tCIDLib::TVoid Initialize
        (
            const   TCQCUserCtx&            cuctxUser
        );

        virtual tCIDLib::TVoid LoadScope
        (
            const   TString&                strPath
        ) = 0;

        virtual tCIDLib::TVoid MakeDefName
        (
            const   TString&                strParScope
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bIsFile
        )   const = 0;

        virtual tCIDLib::TVoid ProcessChange
        (
            const   TString&                strPath
            , const TString&                strOpName
            , const TString&                strData
        );

        virtual tCIDLib::TVoid QueryAccelTable
        (
            const   TString&                strPath
            ,       TAccelTable&            accelToFill
        )   const = 0;

        virtual tCIDLib::TVoid UpdateFile
        (
            const   TString&                strPath
        );

        virtual tCIDLib::TVoid Terminate() = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strName() const;

        const TString& strRootPath() const;

        const TString& strTitle() const;

        tCIDLib::TVoid SetWindows
        (
                    TCQCTreeBrowser* const  pwndBrowser
        );

        TCQCTreeBrowser& wndBrowser() const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQCTreeBrwsIntf
        (
            const   TString&                strRootPath
            , const TString&                strRootName
            , const TString&                strTitle
        );


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bCanRename
        (
            const   TString&                strPath
        )   const;

        virtual tCIDLib::TBoolean bRenameItem
        (
            const   TString&                strParPath
            , const TString&                strOldName
            , const TString&                strNewName
            , const tCIDLib::TBoolean       bIsScope
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSendQuestionNot
        (
            const   tCQCTreeBrws::EEvents   eEvent
            , const TString&                strPath
            , const tCQCRemBrws::EDTypes    eType
            , const tCIDLib::TBoolean       bDefault
        );

        const TCQCUserCtx& cuctxUser() const
        {
            return m_cuctxUser;
        }

        const TCQCSecToken& sectUser() const
        {
            return m_cuctxUser.sectUser();
        }


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cuctxUser
        //      The browser plugins often need security credentials, so the app provides
        //      one for us to use.
        //
        //  m_pwndBrowser
        //      We get a pointer to the browser window that owns us, so that we can
        //      update the tree as required and get info from it.
        //
        //  m_strName
        //      The display name for the top level scope that this browser represents.
        //      The derived class provides this.
        //
        //  m_strRootPath
        //      The root path for this browser. They cannot go up above this level.
        //      The derived class provides this.
        //
        //  m_strTitle
        //      A title for this browser, to be used in msg popups and and such.
        //      Provided by the derived class.
        // -------------------------------------------------------------------
        TCQCUserCtx         m_cuctxUser;
        TCQCTreeBrowser*    m_pwndBrowser;
        const TString       m_strName;
        const TString       m_strRootPath;
        const TString       m_strTitle;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCTreeBrwsIntf,TObject)
};

#pragma CIDLIB_POPPACK



