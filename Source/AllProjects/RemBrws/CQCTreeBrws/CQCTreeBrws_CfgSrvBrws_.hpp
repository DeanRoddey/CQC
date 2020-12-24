//
// FILE NAME: CQCTreeBrws_CfgSrvBrws.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/11/2015
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
//  This file implements a browswer window plugin for the /Configure section of the
//  browser window. Most of the stuff this guy deals with comes from the config server
//  on the master server, but some stuff is gotten through some other interfaces such as
//  the installation server, but really constitutes configuration data, so we expose it
//  via this browser.
//
//  Of course we don't actually access the data, we just provide the means to expose it
//  via the tree control that we implement to manage all of the remote data browser
//  objects. The containing application just responds to our notifications of the user's
//  intent to operate on these things. We do have to access the servers in order to
//  load the tree up.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCfgSrvAccClientProxy;


// ---------------------------------------------------------------------------
//   CLASS: TCQCCfgSrvBrws
//  PREFIX: rbrws
// ---------------------------------------------------------------------------
class TCQCCfgSrvBrws : public TCQCTreeBrwsIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCCfgSrvBrws();

        TCQCCfgSrvBrws(const TCQCCfgSrvBrws&) = delete;
        TCQCCfgSrvBrws(TCQCCfgSrvBrws&&) = delete;

        ~TCQCCfgSrvBrws();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCCfgSrvBrws& operator=(const TCQCCfgSrvBrws&) = delete;
        TCQCCfgSrvBrws& operator=(TCQCCfgSrvBrws&&) = delete;


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
            const   TCQCUserCtx&            cuctxToUse
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



    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDeleteFile
        (
            const  TString&                 strPath
        );

        tCIDLib::TBoolean bMakeNewFile
        (
            const   TString&                strParHPath
            , const tCQCRemBrws::EDTypes    eDType
            ,       TTreeBrowseInfo&        wnotToSend
        );

        tCIDLib::TBoolean bMakeNewEmailAcct
        (
            const   TString&                strParHPath
            , const TString&                strParRelPath
            , const TString&                strNewName
        );

        tCIDLib::TBoolean bMakeNewUserAcct
        (
            const   TString&                strParHPath
            , const TString&                strParRelPath
            , const TString&                strNewName
        );

        tCQCRemBrws::EDTypes ePathToDType
        (
            const   TString&                strPath
            ,       tCIDLib::TBoolean&      bAtTop
        )   const;

        tCIDLib::TVoid LoadEMails
        (
                    TTreeView&              wndTar
        );

        tCIDLib::TVoid LoadUsers
        (
                    TTreeView&              wndTar
        );

        tCIDLib::TVoid UpdateScope
        (
            const   TString&                strPath
        );


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCCfgSrvBrws,TCQCTreeBrwsIntf)
};

#pragma CIDLIB_POPPACK



