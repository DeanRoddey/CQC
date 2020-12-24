//
// FILE NAME: CQCVoice_BTMediaNodes.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/16/2017
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
//  This file implements a set of nodes that handle media related operations.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"
#include    "CQCVoice_BTMediaNodes.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdClearMusicPLNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdClearMusicPLNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdClearMusicPLNode::TBTCmdClearMusicPLNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdClearMusicPL, 0, 0)
{
}

TBTCmdClearMusicPLNode::~TBTCmdClearMusicPLNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdClearMusicPLNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdClearMusicPLNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // If no music info, then we can't do anything
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::MusicData))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoMusicData, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    // Get the music data for our room
    const TSCMediaInfo& scliMusic = facCQCVoice.scliMusic();
    const TString& strMoniker = scliMusic.m_strRendMon;

    // See if there's anything in the playlist
    try
    {
        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

        tCIDLib::TCard4 c4SerNum = 0;
        tCIDLib::TCard4 c4PLCnt;
        orbcDrv->bReadCardByName(c4SerNum, strMoniker, L"MREND#PLItemCnt", c4PLCnt);

        if (c4PLCnt == 0)
        {
            facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_PLAlreadyEmpty);
            return tCIDAI::ENodeStates::Success;
        }

        //
        //  And let's do the field write, doesn't matter what we write, as long as it's
        //  a valid boolean field write value.
        //
        const TString strFld(L"MREND#ClearPlaylist");
        facCQCVoice.bWriteField(strMoniker, strFld, kCQCVoice::strVal_OnState, kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_ClearPL, kCQCVoice::strVal_Music);
        return tCIDAI::ENodeStates::Failure;
    }

    // Tell the user what we did
    facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_PLCleared, kCQCVoice::strVal_Music);
    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdNextMusicItemNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdNextMusicItemNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdNextMusicItemNode::TBTCmdNextMusicItemNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdNextMusicItem, 0, 0)
{
}

TBTCmdNextMusicItemNode::~TBTCmdNextMusicItemNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdNextMusicItemNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdNextMusicItemNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // If no music info, then we can't do anything
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::MusicData))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoMusicData, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    // Get the music data for our room
    const TSCMediaInfo& scliMusic = facCQCVoice.scliMusic();
    const TString& strMoniker = scliMusic.m_strRendMon;

    // See if there's anything in the playlist
    try
    {
        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

        tCIDLib::TCard4 c4SerNum = 0;
        tCIDLib::TCard4 c4PLCnt;
        orbcDrv->bReadCardByName(c4SerNum, strMoniker, L"MREND#PLItemCnt", c4PLCnt);

        if (c4PLCnt == 0)
        {
            facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_PLIsEmpty);
            return tCIDAI::ENodeStates::Success;
        }

        //
        //  And let's do the field write, d
        //
        const TString strFld(L"MTRANS#Transport");
        facCQCVoice.bWriteField(strMoniker, strFld, L"Next", kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NextPLItem, kCQCVoice::strVal_Music);
        return tCIDAI::ENodeStates::Failure;
    }

    // Tell the user what we did
    facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_MovedToNextPLItem, kCQCVoice::strVal_Music);
    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdPlayMusicFromCatNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdPlayMusicFromCatNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdPlayMusicFromCatNode::TBTCmdPlayMusicFromCatNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdPlayMusicFromCat, 0, 0)
{
}

TBTCmdPlayMusicFromCatNode::~TBTCmdPlayMusicFromCatNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdPlayMusicFromCatNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdPlayMusicFromCatNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    //
    //  If no categories were found during setup, then just say so now and give up. If
    //  we do, then this also implies we have music info as well, which we'll access
    //  below.
    //
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::MusicCats))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoCategories);
        return tCIDAI::ENodeStates::Failure;
    }

    TString strCatId = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);

    // If the target isn't medium confidence, ask them to confirm
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_Target))
    {
        // Get the Info semantic value out to speak to the user, that's the PL name
        facCQCVoice.SpeakReply
        (
            kCQCVoiceMsgs::midQ_MediaCatConfirm
            , ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Info)
        );

        // We will accept either yes, cancel, or a category clarification
        tCIDLib::TStrList colWaitFor(2);
        colWaitFor.objAdd(kCQCVoice::strAction_Yes);
        colWaitFor.objAdd(kCQCVoice::strAction_ClarifyMusicCat);

        // Enable these rules temporarily here
        TCQCVoiceRuleJan janRule1(kCQCVoice::strAction_ClarifyMusicCat, kCIDLib::True);
        TCQCVoiceRuleJan janRule2(kCQCVoice::strAction_Yes, kCIDLib::True);

        tCIDLib::TCard4 c4At;
        TCIDSpeechRecoEv sprecevClarify;
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eWaitForAction
        (
            colWaitFor
            , L"category name clarification"
            , sprecevClarify
            , c4At
        );

        // If not successful a cancel or timeout, we just give up
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;

        // If it was a cancel, then we give up
        if (c4At == 0)
            return tCIDAI::ENodeStates::Failure;

        // If it was a clarify, get the new category id out, else keep what we have
        if (c4At == 1)
            strCatId = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_Target);
    }

    // OK, let's do it. Get our music info and the music database
    const TSCMediaInfo& scliMusic = facCQCVoice.scliMusic();
    const TMediaDB& mdbMusic = facCQCVoice.mdbMusic();


    // Look up the category to make sure we find it
    const TMediaCat* pmcatTar = mdbMusic.pmcatByUniqueId
    (
        tCQCMedia::EMediaTypes::Music, strCatId, kCIDLib::False
    );

    // This shouldn't happen, but just in case
    if (!pmcatTar)
    {
        facCQCVoice.SpeakReply(L"The category could not be found");
        return tCIDAI::ENodeStates::Failure;
    }

    // Now build up the cookie for this category
    TString strCookie;
    facCQCMedia().FormatCatCookie(tCQCMedia::EMediaTypes::Music, pmcatTar->c2Id(), strCookie);

    //
    //  We have to make sure the renderer's default repository is set to the repo this
    //  category is from, so set that first.
    //
    const TString& strRendMon =  scliMusic.m_strRendMon;
    if (!facCQCVoice.bWriteField(strRendMon, L"MREND#SetRepository", scliMusic.m_strRepoMon))
    {
        facCQCVoice.SpeakReply(L"The player's default repository could not be set.");
        return tCIDAI::ENodeStates::Failure;
    }

    // And now write to the
    if (!facCQCVoice.bWriteField(strRendMon, L"MREND#SetRandomCat", strCookie))
    {
        facCQCVoice.SpeakReply(L"The new music category could not be set");
        return tCIDAI::ENodeStates::Failure;
    }

    // And finally set the playlist mode to random category mode
    if (!facCQCVoice.bWriteField(strRendMon, L"MREND#PlaylistMode", L"Random Category"))
    {
        facCQCVoice.SpeakReply(L"The random category playlist mode could not be set");
        return tCIDAI::ENodeStates::Failure;
    }

    // It worked, so acknowledge that
    facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_RandomCatPBSet, pmcatTar->strName());

    return tCIDAI::ENodeStates::Success;
}




// ---------------------------------------------------------------------------
//   CLASS: TBTCmdPlayMusicPLNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdPlayMusicPLNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdPlayMusicPLNode::TBTCmdPlayMusicPLNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdPlayMusicPL, 0, 0)
{
}

TBTCmdPlayMusicPLNode::~TBTCmdPlayMusicPLNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdPlayMusicPLNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdPlayMusicPLNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    //
    //  If no playlists were found during setup, then just say so now and give up. If
    //  we do, then this also implies we have music info as well, which we'll access
    //  below.
    //
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::PlayLists))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoMusicPLs, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    // If the playlist id is low confidence, then let's confirm that with the user
    TString strPLId = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);

    // Get the spoken PL name
    TString strPLName = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Info);

    // If the playlist isn't at least medium confidence, ask them to confirm
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_Target))
    {
        // Get the name semantic value out to speak to the user

        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_MediaPLConfirm, strPLName);

        // We will accept either yes, or a playlist clarification
        tCIDLib::TStrList colWaitFor(2);
        colWaitFor.objAdd(kCQCVoice::strAction_Yes);
        colWaitFor.objAdd(kCQCVoice::strAction_ClarifyMusicPL);

        // Enable these rules temporarily here
        TCQCVoiceRuleJan janRule1(kCQCVoice::strAction_ClarifyMusicPL, kCIDLib::True);
        TCQCVoiceRuleJan janRule2(kCQCVoice::strAction_Yes, kCIDLib::True);

        tCIDLib::TCard4 c4At;
        TCIDSpeechRecoEv sprecevClarify;
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eWaitForAction
        (
            colWaitFor
            , L"playlist name clarification"
            , sprecevClarify
            , c4At
        );

        // If not successful a cancel or timeout, we just give up
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;

        // If it was a clarify, get the new playlist id out, else keep what we have
        if (c4At == 1)
        {
            strPLId = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_Target);
            strPLName = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_Info);
        }
    }

    // Just in case, make sure it's a valid number
    tCIDLib::TCard2 c2PLId;
    if (!strPLId.bToCard2(c2PLId, tCIDLib::ERadices::Dec))
    {
        facCQCVoice.SpeakReply(L"The playlist id was not valid");
        return tCIDAI::ENodeStates::Failure;
    }

    // Look up the collection and get it's name so that we can ask the user
    const TMediaCollect* pmcolTar = facCQCVoice.mdbMusic().pmcolById
    (
        tCQCMedia::EMediaTypes::Music, c2PLId, kCIDLib::False
    );

    // This shouldn't happen, but just in case
    if (!pmcolTar)
    {
        facCQCVoice.SpeakReply
        (
            kCQCVoiceMsgs::midErr_PLNotFound, kCQCVoice::strVal_Music, strPLName
        );
        return tCIDAI::ENodeStates::Failure;
    }

    // OK, let's do it. Get our music info and the music database
    const TSCMediaInfo& scliMusic = facCQCVoice.scliMusic();
    const TMediaDB& mdbMusic = facCQCVoice.mdbMusic();

    //
    //  We need to build up the collection cookie for this playlist. First we need to
    //  find the parent container for our collection.
    //
    const TMediaTitleSet* pmtsParent = nullptr;
    tCIDLib::TCard2 c2ColInd;
    const tCIDLib::TBoolean bRes = mdbMusic.bFindColContainer
    (
        tCQCMedia::EMediaTypes::Music, pmcolTar->c2Id(), c2ColInd, pmtsParent
    );

    // Shouldn't happen but just in case
    if (!bRes)
    {
        facCQCVoice.SpeakReply(L"The playlist's parent container could not be found");
        return tCIDAI::ENodeStates::Failure;
    }

    // Now build up the cookie for this collection
    TString strCookie = mdbMusic.strAllCatCookieFor(tCQCMedia::EMediaTypes::Music);
    strCookie.Append(kCIDLib::chComma);
    strCookie.AppendFormatted(pmtsParent->c2Id(), tCIDLib::ERadices::Hex);
    strCookie.Append(kCIDLib::chComma);

    // For the last bit, it's the index into the title set, not its DB id
    strCookie.AppendFormatted(tCIDLib::TCard4(c2ColInd + 1), tCIDLib::ERadices::Hex);

    // And make sure the repo is correct by passing it in
    strCookie.Append(L" ");
    strCookie.Append(scliMusic.m_strRepoMon);

    //
    //  Now we write to either the play or enqueue field, depending on which mode
    //  they indicated. That's stored in the standard State semantic value. It
    //  has the value we need to build up the field.
    //
    const TString& strMode = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_State);

    TString strField = L"MREND#";
    strField.Append(strMode);
    strField.Append(L"Media");

    if (!facCQCVoice.bWriteField(scliMusic.m_strRendMon, strField, strCookie))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_LoadPL, kCQCVoice::strVal_Music);
        return tCIDAI::ENodeStates::Failure;
    }

    // It worked, so acknowledge that
    if (strMode == kCQCVoice::strVal_Enqueue)
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_PLQueued, strPLName, kCQCVoice::strVal_Music);
    else
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_PLPlayed, kCQCVoice::strVal_Music, strPLName);

    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdMusicTransportNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdMusicTransportNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdMusicTransportNode::TBTCmdMusicTransportNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdMusicTransport, 0, 0)
{
}

TBTCmdMusicTransportNode::~TBTCmdMusicTransportNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdMusicTransportNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdMusicTransportNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // If no music info, then we can't do anything
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::MusicData))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoMusicData, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    // Get the state, which is the transport command
    const TString& strState = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_State);

    // The info value is the display name for the command
    const TString& strDesc = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Info);

    // Get the music data for our room
    const TSCMediaInfo& scliMusic = facCQCVoice.scliMusic();

    // And let's do the field write, the state value is set up to use as is
    const TString strTransFld(L"MTRANS#Transport");
    if (!facCQCVoice.bWriteField(scliMusic.m_strRendMon, strTransFld, strState))
    {
        facCQCVoice.SpeakReply
        (
            kCQCVoiceMsgs::midErr_TransportCmd
            , kCQCVoice::strVal_Music
            , strState
        );
        return tCIDAI::ENodeStates::Failure;
    }

    // Tell the user what we did
    facCQCVoice.SpeakReply
    (
        kCQCVoiceMsgs::midSpeak_TransportCmd, kCQCVoice::strVal_Music, strDesc
    );

    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetMusicMuteNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdSetMusicMuteNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdSetMusicMuteNode::TBTCmdSetMusicMuteNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdSetMusicMute, 0, 0)
{
}

TBTCmdSetMusicMuteNode::~TBTCmdSetMusicMuteNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdSetMusicMuteNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdSetMusicMuteNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // If no music info, then we can't do anything
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::MusicData))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoMusicData, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    // Get the state, which is 0 or 1, unmuted or muted
    TString strState = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_State);

    // If the state isn't at least med confidence, get a clarification
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_State))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_MuteConfirm);

        // We will accept either a mute clarification or a cancel
        tCIDLib::TStrList colWaitFor(1);
        colWaitFor.objAdd(kCQCVoice::strAction_ClarifyMute);

        // Enable this rule temporarily here
        TCQCVoiceRuleJan janRule(kCQCVoice::strAction_ClarifyMute, kCIDLib::True);

        tCIDLib::TCard4 c4At;
        TCIDSpeechRecoEv sprecevClarify;
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eWaitForAction
        (
            colWaitFor
            , L"mute state clarification"
            , sprecevClarify
            , c4At
        );

        // If not successful, a cancel or timeout, we just give up
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;

        // Get the new state out, else keep what we have
        strState = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_State);
    }

    // Get the music data for our room
    const TSCMediaInfo& scliMusic = facCQCVoice.scliMusic();

    //
    //  And let's do the field write. Remember this is not necessarily the renderer
    //  though it could be. We get the AUD class handler as a separate thing.
    //
    TString strMuteFld(L"AUD#");
    if (!scliMusic.m_strAudioSub.bIsEmpty())
    {
        strMuteFld.Append(scliMusic.m_strAudioSub);
        strMuteFld.Append(kCIDLib::chTilde);
    }
    strMuteFld.Append(L"Mute");

    // We can write the 0/1 state directly to the field
    const tCIDLib::TBoolean bState(strState == kCQCVoice::strVal_OnState);
    if (!facCQCVoice.bWriteField(scliMusic.m_strAudioMon, strMuteFld, strState))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_MuteSet, kCQCVoice::strVal_Music);
        return tCIDAI::ENodeStates::Failure;
    }

    // Tell the user what we did

    facCQCVoice.SpeakReply
    (
        bState ? kCQCVoiceMsgs::midSpeak_MuteSet : kCQCVoiceMsgs::midSpeak_MuteCleared
        , kCQCVoice::strVal_Music
    );

    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetMusicPLModeNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdSetMusicPLModeNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdSetMusicPLModeNode::
TBTCmdSetMusicPLModeNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdSetMusicPLMode, 0, 0)
{
}

TBTCmdSetMusicPLModeNode::~TBTCmdSetMusicPLModeNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdSetMusicPLModeNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdSetMusicPLModeNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // If no music info, then we can't do anything
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::MusicData))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoMusicData, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    // Get the state, which is the mode to set
    TString strMode = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_State);

    // If the mode isn't at least me confidence, get a clarification
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_State))
    {
        // <TBD>
    }

    // If it's random cateogry, tell them there's another command for that
    if (strMode == kCQCVoice::strVal_RandomCat)
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_UseLoadPLCmd);
        return tCIDAI::ENodeStates::Failure;
    }

    // Get the music data for our room
    const TSCMediaInfo& scliMusic = facCQCVoice.scliMusic();

    // And let's do the field write
    TString strFld(L"MREND#PlaylistMode");
    if (!facCQCVoice.bWriteField(scliMusic.m_strRendMon, strFld, strMode))
    {
        facCQCVoice.SpeakReply
        (
            kCQCVoiceMsgs::midErr_PLModeSet, kCQCVoice::strVal_Music, strMode
        );
        return tCIDAI::ENodeStates::Failure;
    }

    // Tell the user what we did
    facCQCVoice.SpeakReply
    (
        kCQCVoiceMsgs::midSpeak_PLModeSet, kCQCVoice::strVal_Music, strMode
    );

    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetMusicVolumeNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdSetMusicVolumeNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdSetMusicVolumeNode::TBTCmdSetMusicVolumeNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdSetMusicVolume, 0, 0)
{
}

TBTCmdSetMusicVolumeNode::~TBTCmdSetMusicVolumeNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdSetMusicVolumeNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdSetMusicVolumeNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // If no music info, then we can't do anything
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::MusicData))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoMusicData, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    // Get the state, which is the volume percentage
    TString strLevel = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_State);

    // If the percentage isn't full confidence, get a clarification
    if (!facCQCVoice.bIsFullConfidence(btreeOwner, kCQCVoice::strSemVal_State))
    {
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eClarifyPercentage(strLevel);
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;
    }

    // Make sure it's a valid percentage
    tCIDLib::TCard4 c4Level;
    if (!strLevel.bToCard4(c4Level, tCIDLib::ERadices::Dec) || (c4Level > 100))
    {
        facCQCVoice.SpeakReply
        (
            L"The received volume level could not be converted to a percent."
        );
        return tCIDAI::ENodeStates::Failure;
    }

    // Get the music data for our room
    const TSCMediaInfo& scliMusic = facCQCVoice.scliMusic();

    //
    //  And let's do the field write. Remember this is not necessarily the renderer
    //  though it could be. We get the AUD class handler as a separate thing.
    //
    TString strVolFld(L"AUD#");
    if (!scliMusic.m_strAudioSub.bIsEmpty())
    {
        strVolFld.Append(scliMusic.m_strAudioSub);
        strVolFld.Append(kCIDLib::chTilde);
    }
    strVolFld.Append(L"Volume");
    if (!facCQCVoice.bWriteField(scliMusic.m_strAudioMon, strVolFld, strLevel))
    {
        facCQCVoice.SpeakReply
        (
            kCQCVoiceMsgs::midErr_VolumeSet, kCQCVoice::strVal_Music, strLevel
        );
        return tCIDAI::ENodeStates::Failure;
    }

    // Tell the user what we did
    facCQCVoice.SpeakReply
    (
        kCQCVoiceMsgs::midSpeak_VolumeSet, kCQCVoice::strVal_Music, strLevel
    );

    return tCIDAI::ENodeStates::Success;
}
