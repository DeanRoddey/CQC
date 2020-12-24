//
// FILE NAME: CQCTreeBrws_SystemBrws.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/06/2016
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
//  This file implements a browswer window plugin for the /System section of the browser
//  window. This one is very simple and exists just send notifications to the containing
//  application to let it know what was invoked. We send view notifications for all of these
//  with the path of the thing invoked.
//
//  We have these items, all of which are loaded up front.
//
//      /System
//      /System/Explore Logs
//      /System/License Info
//      /System/Monitor
//      /System/Monitor/Event Triggers
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCSystemBrws
//  PREFIX: rbrws
// ---------------------------------------------------------------------------
class TCQCSystemBrws : public TCQCTreeBrwsIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSystemBrws();

        TCQCSystemBrws(const TCQCSystemBrws&) = delete;
        TCQCSystemBrws(TCQCSystemBrws&&) = delete;

        ~TCQCSystemBrws();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSystemBrws& operator=(const TCQCSystemBrws&) = delete;
        TCQCSystemBrws& operator=(TCQCSystemBrws&&) = delete;


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
        RTTIDefs(TCQCSystemBrws,TCQCTreeBrwsIntf)
};

#pragma CIDLIB_POPPACK


