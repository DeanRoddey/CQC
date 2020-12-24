//
// FILE NAME: CQCTreeBrws_DataSrvBrws.hpp
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
//  This file implements a remote browser in terms of the ORB based interface
//  defined in CQCTreeBrws_DataSrcBrws.CIDIDL. The data server implements the other
//  end of that interface and allows us to browse the hierarchy of information
//  available from the data server.
//
//  We derive from the abstract base class in CQCTreeBrws_RemBrwsIntf.hpp, which
//  has a fairly simple interface that we have to turn around an implement in
//  terms of the ORB interface we use internally.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TDataSrvAccClientProxy;
class TEventSrvClientProxy;


// ---------------------------------------------------------------------------
//   CLASS: TCQCDataSrvBrws
//  PREFIX: rbrws
// ---------------------------------------------------------------------------
class TCQCDataSrvBrws : public TCQCTreeBrwsIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDataSrvBrws
        (
            const   tCQCRemBrws::EDTypes    eFilterType = tCQCRemBrws::EDTypes::Count
        );

        TCQCDataSrvBrws(const TCQCDataSrvBrws&) = delete;
        TCQCDataSrvBrws(TCQCDataSrvBrws&&) = delete;

        ~TCQCDataSrvBrws();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDataSrvBrws& operator=(const TCQCDataSrvBrws&) = delete;
        TCQCDataSrvBrws& operator=(TCQCDataSrvBrws&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AcceptFiles
        (
            const   TString&                strParScope
            , const tCIDLib::TStrList&      colTarNames
            , const tCIDLib::TStrList&      colSrcPaths
        )   final;

        tCIDLib::TBoolean bAcceptsNew
        (
            const   TString&                strPath
            , const tCIDLib::TStrHashSet&   colExts
        )   const final;

        tCIDLib::TBoolean bDoMenuAction
        (
            const   TString&                strPath
            ,       TTreeBrowseInfo&        wnotToSend
        )   final;

        tCIDLib::TBoolean bIsConnected() const final;

        tCIDLib::TBoolean bIPEValidate
        (
            const   TString&                strSrc
            ,       TAttrData&              adatTar
            , const TString&                strNewVal
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard8&        c8UserId
        )   const final;

        tCIDLib::TBoolean bProcessMenuSelection
        (
            const   tCIDLib::TCard4         c4CmdId
            , const TString&                strPath
            ,       TTreeBrowseInfo&        wnotToSend
        )   final;

        tCIDLib::TBoolean bReportInvocation
        (
            const   TString&                strPath
            ,       tCIDLib::TBoolean&      bAsEdit
        )   const final;

        tCIDLib::TVoid Initialize
        (
            const   TCQCUserCtx&            cuctxUser
        )   final;

        tCIDLib::TVoid LoadScope
        (
            const   TString&                strPath
        )   final;

        tCIDLib::TVoid MakeDefName
        (
            const   TString&                strParScope
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bIsFile
        )   const final;

        tCIDLib::TVoid QueryAccelTable
        (
            const   TString&                strPath
            ,       TAccelTable&            accelToFill
        )   const final;

        tCIDLib::TVoid UpdateFile
        (
            const   TString&                strPath
        )   final;

        tCIDLib::TVoid Terminate() final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanRename
        (
            const   TString&                strPath
        )   const final;

        tCIDLib::TBoolean bRenameItem
        (
            const   TString&                strParPath
            , const TString&                strOldName
            , const TString&                strNewName
            , const tCIDLib::TBoolean       bIsScope
        )   final;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid UploadEvent
        (
            const   TString&                strSrcPath
            , const TString&                strParScope
            , const TString&                strTarName
            , const tCIDLib::TBoolean       bTrigType
            ,       TDataSrvClient&         dsclToUse
        );

        tCIDLib::TVoid UploadImage
        (
            const   TString&                strSrcPath
            , const TString&                strParScope
            , const TString&                strTarName
            , const TString&                strExt
            ,       TDataSrvClient&         dsclToUse
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddScope
        (
            const   TString&                strParHPath
            , const tCIDLib::TBoolean       bVirtual
        );

        tCIDLib::TBoolean bDeleteFile
        (
            const  TString&                 strHPath
            , const TDSBrowseItem&          dsbiPath
        );

        tCIDLib::TBoolean bExportFiles
        (
            const   TString&                strParHPath
            , const tCQCRemBrws::EDTypes    eDType
        );

        tCIDLib::TBoolean bImportFiles
        (
            const   TString&                strParHPath
            , const tCQCRemBrws::EDTypes    eDType
        );

        tCIDLib::TBoolean bMakeNewFile
        (
            const   TString&                strParHPath
            , const tCQCRemBrws::EDTypes    eDType
            , const tCIDLib::TKVPFList&     colExtraMeta
            ,       TTreeBrowseInfo&        wnotToSend
        );

        tCIDLib::TBoolean bMakeNewTemplate
        (
            const   TString&                strParHPath
            , const TString&                strParRelPath
            , const TString&                strNewName
            , const tCIDLib::TKVPFList&     colExtraMeta
        );

        tCIDLib::TBoolean bPauseResumeEvent
        (
            const   TString&                strHPath
            , const TDSBrowseItem&          dsbiPath
            , const tCIDLib::TBoolean       bPause
            ,       TTreeBrowseInfo&        wnotToSend
        );

        tCIDLib::TBoolean bPastePath
        (
            const   TString&                strTarPath
            , const TDSBrowseItem&          dsbiPath
            ,       TString&                strSrcPath
        );

        tCIDLib::TVoid SetupMenu
        (
                    TMenu&                  menuTar
            , const TString&                strPath
            , const TDSBrowseItem&          dsbiCtrl
        )   const;

        tCIDLib::TVoid UpdateScope
        (
            const   TString&                strPath
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eFilterType
        //      Because we sometimes want to allow for browsing only of a specific
        //      file type, but we want to avoid a lot of complications, we allow
        //      the client code to indicate a file type. We will use the base path
        //      of that file to insure that we only return stuff that matches that
        //      path until we get to children of the path, at which time we start
        //      returning everything. This way, the tree browser doesn't have to
        //      do anything to support this, and we still expose all of the scopes
        //      of the per-file type paths in a natural way.
        //
        //      It's not very wasteful, since the amount of stuff returned that we
        //      throw away is very small, mostly just the other handful of file type
        //      top level scopes. None of the contents of the other scopes ever
        //      get queried, since they never are never seen.
        //
        //      If not set, it's the _Count value.
        //
        //  m_strFilterPath
        //      To avoid overhead, we set this during ctor, based on the incoming
        //      filter type (see m_eFilterType above.) If the incoming value is the
        //      _Count value, then this is not used and will be empty.
         // -------------------------------------------------------------------
        tCQCRemBrws::EDTypes        m_eFilterType;
        TString                     m_strFilterPath;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDataSrvBrws,TCQCTreeBrwsIntf)
};

#pragma CIDLIB_POPPACK

