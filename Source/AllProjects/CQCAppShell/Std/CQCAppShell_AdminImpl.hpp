//
// FILE NAME: CQCAppShell_AdminImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/09/2002
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
//  This is the header for the CQCAppShell_AdminImpl.cpp file. This file
//  implements our IDL admin interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCAppShellImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TCQCAppShellImpl : public TCQCAppShellAdminServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCAppShellImpl();

        TCQCAppShellImpl(const TCQCAppShellImpl&) = delete;
        TCQCAppShellImpl(TCQCAppShellImpl&&) = delete;

        ~TCQCAppShellImpl();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCAppShellImpl& operator=(const TCQCAppShellImpl&) = delete;
        TCQCAppShellImpl& operator=(TCQCAppShellImpl&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryNewMsgs
        (
                    tCIDLib::TCard4&        c4MsgIndex
            ,       tCIDLib::TStrList&      colMsgs
            , const tCIDLib::TBoolean       bAddNewLine
        )   final;

        tCIDLib::TCard4 c4QueryApps
        (
                    tCIDLib::TKVPList&      colApps
        )   final;

        tCIDLib::TVoid CycleApps() final;


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCAppShellImpl,TCQCAppShellAdminServerBase)
};

#pragma CIDLIB_POPPACK


