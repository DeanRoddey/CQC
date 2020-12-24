//
// FILE NAME: CQCGKit_FrameState.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/08/2002
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
//  This is the header for the CQCGKit_FrameState.cpp file, which implements
//  the TCQCFrameState class. This class defines the persistence data that we
//  want to store about a frame window in a CQC client app, so that we can
//  restore it when we start again. Each app will create it's own derivatives
//  that store extra data above and beyond this basic info.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCFrameState
//  PREFIX: fst
// ---------------------------------------------------------------------------
class CQCGKITEXPORT TCQCFrameState : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFrameState();

        TCQCFrameState
        (
            const   TArea&                  areaFrame
        );

        TCQCFrameState
        (
            const   TArea&                  areaFrame
            , const tCIDCtrls::EPosStates   ePosState
        );

        TCQCFrameState(const TCQCFrameState&) = default;
        TCQCFrameState(TCQCFrameState&&) = default;

        ~TCQCFrameState();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFrameState& operator=(const TCQCFrameState&) = default;
        TCQCFrameState& operator=(TCQCFrameState&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TArea& areaFrame() const;

        const TArea& areaFrame
        (
            const   TArea&                  areaToSet
        );

        tCIDCtrls::EPosStates ePosState() const;

        tCIDCtrls::EPosStates ePosState
        (
            const   tCIDCtrls::EPosStates   eToSet
        );

        tCIDLib::TVoid SetArea
        (
            const   TArea&                  areaToSet
            , const tCIDCtrls::EPosStates   eToSet
        );

        tCIDLib::TVoid SetFrom
        (
            const   TFrameWnd&              wndSrc
        );

        tCIDLib::TVoid SetOrg
        (
            const   TPoint&                 pntToSet
            , const tCIDCtrls::EPosStates   eToSet
        );

        tCIDLib::TVoid SetSize
        (
            const   TSize&                  szToSet
            , const tCIDCtrls::EPosStates   eToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaFrame
        //      The last normal (restored) area of the frame window.
        //
        //  m_ePosState
        //      The position state of the window, i.e. minimized, restored,
        //      or maximized. We need to store this in order to get the
        //      windows back where they were.
        // -------------------------------------------------------------------
        TArea                   m_areaFrame;
        tCIDCtrls::EPosStates   m_ePosState;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFrameState,TObject)
};

#pragma CIDLIB_POPPACK


