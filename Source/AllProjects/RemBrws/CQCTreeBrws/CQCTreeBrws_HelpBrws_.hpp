//
// FILE NAME: CQCTreeBrws_HelpBrws.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/18/2016
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
//  This file implements a browswer window plugin for the /Help section of the browser
//  window. This one is very simple and exists just to invoke the Help tab. So all we
//  do is react to a double click or View from the popup menu, and send a notification.
//  We also provide a couple sub-items to invoke specific help topics that might be
//  very commonly used. But all of them are the same basically the same, just sending
//  a notification to the containing app to open the help content.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCHelpBrws
//  PREFIX: rbrws
// ---------------------------------------------------------------------------
class TCQCHelpBrws : public TCQCTreeBrwsIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCHelpBrws();

        TCQCHelpBrws(const TCQCHelpBrws&) = delete;
        TCQCHelpBrws(TCQCHelpBrws&&) = delete;

        ~TCQCHelpBrws();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCHelpBrws& operator=(const TCQCHelpBrws&) = delete;
        TCQCHelpBrws& operator=(TCQCHelpBrws&&) = delete;


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

        tCIDLib::TVoid Terminate() final;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCHelpBrws,TCQCTreeBrwsIntf)
};

#pragma CIDLIB_POPPACK


