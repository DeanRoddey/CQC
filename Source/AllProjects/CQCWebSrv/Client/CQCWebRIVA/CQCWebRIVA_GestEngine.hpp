//
// FILE NAME: CQCWebRIVA_GestEngine.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/26/2013
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
//  This is the header for our RIVA specific gesture engine. We just feed it incoming RIVA
//  mouse movement, and it passes them on to the underlying gesture engine class. The IV
//  engine knows it's in remote mode, and any time the gesture engine asks a widget if it
//  wants to do a gesture, they will indicate flick type gestures when remoted, instead
//  of inertial drags.
//
//  And the widgets, once flicked, will not do a sliding scroll in remote mode, they will
//  just redraw in the new position.
//
//  This is an adaptation of the original version of this class for the old RIVA server
//  before we moved it to the web server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

class TWorkerThread;


// ---------------------------------------------------------------------------
//   CLASS: TRIVAGestHandler
//  PREFIX: gesth
// ---------------------------------------------------------------------------
class CQCWEBRIVAEXP TRIVAGestHandler : public TCIDGestHandler
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TRIVAGestHandler
        (
                    MCIDGestTarget* const    pmgesthTar
        );

        TRIVAGestHandler(const TRIVAGestHandler&) = delete;
        TRIVAGestHandler(TRIVAGestHandler&&) = delete;

        ~TRIVAGestHandler();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TRIVAGestHandler& operator==(const TRIVAGestHandler&) = delete;
        TRIVAGestHandler& operator==(TRIVAGestHandler&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid HandleMsg
        (
            const   tCQCWebRIVA::EGestEvs   eEv
            , const TPoint&                 pntAt
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset
        (
            const   tCIDCtrls::EGestReset   eType
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        //
        //  We need a small state enum to keep up with what we are doing.
        // -------------------------------------------------------------------
        enum class EInpStates
        {
            Idle
            , GotPress
            , InGesture
        };


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaBreakout
        //      The area around the touch point that we have to move out of
        //      to be considered a gesture. If not, it's treated as a click.
        //
        //  m_eCurState
        //      Our current state, so we know what we need to do when we
        //      receive messages.
        //
        //  m_pntStart
        //      When we see a button down, we remember where it happened. If
        //      we never move out of a small area around this point before
        //      releasing, we treat it as a click, so we send a press,
        //      move, and release all at once.
        //
        //      Else, we assume it's a gesture, and start that process.
        //      Just for temp use doing our message handling
        // -------------------------------------------------------------------
        TArea       m_areaBreakout;
        EInpStates  m_eCurState;
        TPoint      m_pntStart;
};


#pragma CIDLIB_POPPACK
