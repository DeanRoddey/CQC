//
// FILE NAME: CQCAdmin_StateInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/16/2002
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
//  This is the header for the CQCAdmin_StateInfo.cpp file, which implements some
//  derivatives of the standard TCQCFrameState class, to which we add some info for our
//  needs and use to store our per-user/app object store. We have one for the main
//  frame window state (for restore upon startup) and others for things like opened
//  client side drivers and such.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCCAppState
//  PREFIX: fst
// ---------------------------------------------------------------------------
class TCQCCAppState : public TCQCFrameState
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCCAppState();

        TCQCCAppState
        (
            const   TArea&                  areaFrame
        );

        TCQCCAppState(const TCQCCAppState&) = default;
        TCQCCAppState(TCQCCAppState&&) = default;

        ~TCQCCAppState();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCCAppState& operator=(const TCQCCAppState&) = default;
        TCQCCAppState& operator=(TCQCCAppState&&) = default;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCCAppState,TCQCFrameState)
};

#pragma CIDLIB_POPPACK

