//
// FILE NAME: CQCWebRIVA_WorkerIntf.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/18/2017
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
//  This header defines a simple mixin interface that we define so that we can access
//  required functionality in the Web Server client facility's worker thread. It will
//  implement this interface and provide the IV view object it creates a pointer to
//  itself via this interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TWebRIVAWIntf
//  PREFIX: wrwi
// ---------------------------------------------------------------------------
class CQCWEBRIVAEXP TWebRIVAWIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TWebRIVAWIntf(const TWebRIVAWIntf&) = delete;
        TWebRIVAWIntf(TWebRIVAWIntf&&) = delete;

        ~TWebRIVAWIntf();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TWebRIVAWIntf& operator=(const TWebRIVAWIntf&) = delete;
        TWebRIVAWIntf& operator=(TWebRIVAWIntf&&) = delete;


        // -------------------------------------------------------------------
        //  Virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid DispatchActEvent
        (
                    tCQCIntfEng::TIntfCmdEv& iceToDo
        )   = 0;

        virtual tCIDLib::TVoid FauxGUILoop
        (
                    tCIDLib::TBoolean&      bBreakFlag
        )   = 0;

        virtual tCIDLib::TVoid PauseTimers
        (
            const   tCIDLib::TBoolean       bPauseState
        )   = 0;

        virtual tCIDLib::TVoid SendGraphicsMsg
        (
                    TMemBuf* const          pmbufToAdopt
            , const tCIDLib::TCard4         c4Size
        )   = 0;

        virtual tCIDLib::TVoid SendImgMsg
        (
            const   TString&                strFullPath
            , const TBitmap&                bmpToSend
            ,       TMemBuf* const          pmbufToAdopt
            , const tCIDLib::TCard4         c4Size
        )   = 0;

        virtual tCIDLib::TVoid SendMsg
        (
                    TMemBuf* const          pmbufToAdopt
            , const tCIDLib::TCard4         c4Size
        )   = 0;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TWebRIVAWIntf();
};

#pragma CIDLIB_POPPACK
