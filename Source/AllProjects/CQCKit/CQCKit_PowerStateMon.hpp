//
// FILE NAME: CQCKit_PowerStateMon.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/03/2014
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
//  This class is used when a client needs to wait for some arbitrary set of V2
//  drivers (which implement the Power class) to reach a particular power state.
//  When it's required to power on a set of drivers, it's necessary to check which
//  ones are not in the required state and ask them to transition, then to wait for
//  them to get there.
//
//  It's also possible that some of them might be heading towards the opposite state,
//  in which case we have to wait for those to get to that other state, and then
//  ask for the transition to the desired state.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCPwrStateMon
//  PREFIX: cpsm
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCPwrStateMon : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCPwrStateMon() = delete;

        TCQCPwrStateMon
        (
            const   TString&                strName
        );

        TCQCPwrStateMon(const TCQCPwrStateMon&) = delete;
        TCQCPwrStateMon(TCQCPwrStateMon&&) = delete;

        ~TCQCPwrStateMon();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCPwrStateMon& operator=(const TCQCPwrStateMon&) = delete;
        TCQCPwrStateMon& operator=(TCQCPwrStateMon&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EAsyncWaitRes eCheckResult();

        tCIDLib::TVoid Cleanup();

        tCIDLib::TVoid Start
        (
            const   tCIDLib::TKVPCollect&   colDrivers
            , const tCIDLib::TBoolean       bNewState
        );


    private :
        // -------------------------------------------------------------------
        //  Private data types
        //
        //  We need an overall state
        // -------------------------------------------------------------------
        enum class EStates
        {
            Idle
            , Waiting
            , Complete
            , Timeout
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes ePollThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eCurState
        //      Our current state, which is initialized by the Start() and
        //      Cleanup() methods, and the polling thread when active.
        //
        //  m_eTarPwrState
        //      The new target status we are going for
        //
        //  m_colDrivers
        //      The key/value list (driver/sub-unit) that we are to wait on.
        //
        //  m_thrPoll
        //      The thread that does the waiting in the background.
        //
        //  m_strName
        //      A name given to this object by the client code.
        // -------------------------------------------------------------------
        EStates                 m_eCurState;
        tCQCKit::EPowerStatus   m_eTarPwrState;
        tCIDLib::TKVPList       m_colDrivers;
        TThread                 m_thrPoll;
        TString                 m_strName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCPwrStateMon,TObject)
};

#pragma CIDLIB_POPPACK

