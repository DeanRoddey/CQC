//
// FILE NAME: CQCVoice_BTReminderNodes.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/09/2017
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
//  This is the header for reminder oriented nodes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdCancelAllReminders
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdCancelAllReminders : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdCancelAllReminders
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdCancelAllReminders(const TBTCmdCancelAllReminders&) = delete;

        ~TBTCmdCancelAllReminders();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdCancelAllReminders& operator=(const TBTCmdCancelAllReminders&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdCancelLastReminder
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdCancelLastReminder : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdCancelLastReminder
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdCancelLastReminder(const TBTCmdCancelLastReminder&) = delete;

        ~TBTCmdCancelLastReminder();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdCancelLastReminder& operator=(const TBTCmdCancelLastReminder&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdRemindMe
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdRemindMe : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdRemindMe
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdRemindMe(const TBTCmdRemindMe&) = delete;

        ~TBTCmdRemindMe();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdRemindMe& operator=(const TBTCmdRemindMe&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};



#pragma CIDLIB_POPPACK
