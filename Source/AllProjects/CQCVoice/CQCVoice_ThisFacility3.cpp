//
// FILE NAME: CQCVoice_ThisFacility3.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/02/2016
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
//  This file handles some of the facility methods, to keep it from getting too
//  large. This one handles code related to loading up the content of dynamic
//  grammar rules and manipulating the state of rules.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"


// ---------------------------------------------------------------------------
//  TFacCQCVoice: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called when we are asked to load up our grammar. This handles up loading up any of
//  our dynamic rules. It calls other local helpers for each of the rules it updates.
//
//  The basic caps flags are set by now, so we can check those, though we may turn
//  some back off if we can't get to the information they refer to. Since the basic
//  flags are set, we can call the helper methods to get our room's config for those
//  things that we know by now are available.
//
tCIDLib::TVoid TFacCQCVoice::LoadDynRules()
{
    tCIDLib::TKVPList colValues;

    //
    //  First let's update the rule that lets the user select a light. This is not
    //  actually room specific. We make them all availble for control. In this case
    //  have both the main title and the spoken name potentially available. So we
    //  can have two entries per light in some cases, both of which map back to the
    //  title.
    //
    {
        const tCIDLib::TCard4 c4Count = m_scfgCurrent.c4ListIdCount(tCQCSysCfg::ECfgLists::Lighting);

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSCLoadInfo& scliCur = m_scfgCurrent.scliLoadAt(c4Index);

            // Add one for the main title, which maps to itself
            colValues.objPlace(scliCur.m_strTitle, scliCur.m_strTitle);

            // If there is an alt name and it's different from the title, add that
            if (!scliCur.m_strAltName.bIsEmpty()
            &&  !scliCur.m_strAltName.bCompareI(scliCur.m_strTitle))
            {
                colValues.objPlace(scliCur.m_strAltName, scliCur.m_strTitle);
            }
        }

        //
        //  We don't set a semantic name, so the semantic value just gets set to
        //  out, the rule output, which is what is expected in the grammar.
        //
        m_sprecoLocal.ReplaceOneOfRule
        (
            kCQCVoice::strRule_LightList, TString::strEmpty(), colValues
        );
    }


    //
    //  If media playback is configured for our room, then let's load up the available
    //  playlists and albums. If we ultimately can't get access to the media database,
    //  we will turn the flag back off.
    //
    if (bCapFlag(tCQCVoice::ECapFlags::MusicData))
    {
        const TSCMediaInfo& scliOurs = scliMusic();

        //
        //  We can use the media repo moniker to get access to the media repository data
        //  and load up any music related inf we require.
        //
        TFacCQCMedia::TMDBPtr cptrNew;
        const tCIDLib::ELoadRes eRes = facCQCMedia().eGetMediaDB
        (
            scliOurs.m_strRepoMon, m_strMusicDBSerNum, cptrNew
        );

        //
        //  If we got data, then let's store the new DB pointer away and load up the
        //  music related rules.
        //
        if (eRes == tCIDLib::ELoadRes::NotFound)
        {
            SetCapFlag(tCQCVoice::ECapFlags::MusicData, kCIDLib::False);
        }
         else
        {
            // We either found data or already had the most recent
            m_cptrMusic = cptrNew;

            // Let's try to load up the rule for playlists and categories
            LoadPlaylists();
            LoadMusicCats();
        }
    }

    // If we have security configured for our room
    if (bCapFlag(tCQCVoice::ECapFlags::SecData))
    {
        const TSCSecInfo& scliOurs = scliSecurity();

        // Let's try to load the arming modes and security zones rules
        LoadSecArmModes(scliOurs);
        LoadSecZones();
    }

    // If we have any room modes, load them up
    if (bCapFlag(tCQCVoice::ECapFlags::RoomModes))
        LoadRoomModes();

    // We always set up the custom reminders if there are any
    LoadCustReminders();

    //
    //  Update those rules that provide the leading keyword.
    //
    {
        const TString& strCfgKeyWord = scrmiOurs().scvcVoice().strKeyWord();

        // Set the top level command prefix version of it
        tCIDLib::TStrList colSemVals(1);
        colSemVals.objAdd(strCfgKeyWord);
        m_sprecoLocal.ReplaceOneOfRule(kCQCVoice::strRule_KeyWord, colSemVals);

        //
        //  Setup the wakeup keyword phrase. This guy needs the standard semantic values,
        //  but only the action is set.
        //
        colValues.RemoveAll();
        colValues.objPlace(kCQCVoice::strVal_Action, kCQCVoice::strAction_WakeupKeyWord);
        colValues.objPlace(kCQCVoice::strVal_DataType, TString::strEmpty());
        colValues.objPlace(kCQCVoice::strVal_TarType, TString::strEmpty());
        colValues.objPlace(kCQCVoice::strVal_Target, TString::strEmpty());
        colValues.objPlace(kCQCVoice::strVal_State, TString::strEmpty());
        colValues.objPlace(kCQCVoice::strVal_Prefixed, TString::strEmpty());

        //
        //  This one just needs the action to reflect the hello keyword rule. We setup
        //  up a few options
        //
        tCIDLib::TStrList colRuleVals;
        TString strVal = L"Wake up ";
        strVal.Append(strCfgKeyWord);
        colRuleVals.objAdd(strVal);

        strVal = L"Help me ";
        strVal.Append(strCfgKeyWord);
        colRuleVals.objAdd(strVal);

        strVal = L"Hello ";
        strVal.Append(strCfgKeyWord);
        colRuleVals.objAdd(strVal);

        m_sprecoLocal.ReplaceOneOfRule
        (
            kCQCVoice::strRule_WakeupKeyWord, colRuleVals, colValues
        );
    }
}


//
//  The main media rules loading method calls these to update the playlists and music
//  categories rules. We set Name and Id semantic values. The latter is to find the
//  thing. The former is for if we need to clarify, so we don't need to look it up just
//  to get the name for the clarification reply, and then look it up again if we have to
//  clarify.
//
tCIDLib::TVoid TFacCQCVoice::LoadMusicCats()
{
    const TMediaDB& mdbSrc = m_cptrMusic->mdbData();

    // Get the count of music categories
    const tCIDLib::TCard4 c4Count = mdbSrc.c4CatCnt(tCQCMedia::EMediaTypes::Music);

    if (c4Count)
    {
        //
        //  Loop through the categories. For each one, check if they have any titles
        //  in them, keep it.
        //
        tCIDLib::TKValsList colSemVals(c4Count);
        TString strId;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaCat& mcatCur = mdbSrc.mcatAt(tCQCMedia::EMediaTypes::Music, c4Index);

            if (mdbSrc.bCatHasSomeSets(mcatCur))
            {
                colSemVals.objPlace
                (
                    mcatCur.strName(), mcatCur.strName(), mcatCur.strUniqueId()
                );
            }
        }

        if (!colSemVals.bIsEmpty())
        {
            const TString strSemName(L"Name");
            const TString strSemId(L"Id");

            m_sprecoLocal.ReplaceOneOfRule
            (
                kCQCVoice::strRule_MusicCategories, strSemName, strSemId, colSemVals
            );

            // Remember that we have music categories
            SetCapFlag(tCQCVoice::ECapFlags::MusicCats, kCIDLib::True);
        }
    }
}


tCIDLib::TVoid TFacCQCVoice::LoadPlaylists()
{
    const TMediaDB& mdbSrc = m_cptrMusic->mdbData();

    // There is a helper to find all playlists, so just use that to load our list
    tCQCMedia::TMediaIdList fcolIds;
    tCIDLib::TStrList colNames;
    tCIDLib::TCard4 c4Count = mdbSrc.c4QueryPlayLists(colNames, fcolIds);

    //
    //  If there are not any, don't do anything. We set two semantic values, Id= and
    //  Name=. The latter so that we don't have to look up the playlist just to find
    //  the name if we need a clarification.
    //
    if (c4Count)
    {
        const TString strSemName(L"Name");
        const TString strSemId(L"Id");

        // Transfer the information to the appropriate format
        tCIDLib::TKValsList colSemVals(c4Count);

        TString strId;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strId.SetFormatted(fcolIds[c4Index]);
            colSemVals.objPlace(colNames[c4Index], colNames[c4Index], strId);
        }

        // We put the names in value 1, so Names first then Id
        m_sprecoLocal.ReplaceOneOfRule
        (
            kCQCVoice::strRule_PlayLists, strSemName, strSemId, colSemVals
        );

        // Remember we have playlists
        SetCapFlag(tCQCVoice::ECapFlags::PlayLists, kCIDLib::True);
    }
}


//
//  Load up any custom reminders if there are any.
//
tCIDLib::TVoid TFacCQCVoice::LoadCustReminders()
{

    //
    //  And now let's set it. We don't set a semantic value name, so the selected
    //  value is just the basic 'out' result of the rul.
    //
    // m_sprecoLocal.ReplaceOneOfRule(kCQCVoice::strRule_CustReminders, colValues);
}



//
//  If we haev room modes defined, this is called to load them up. We set two semantic
//  values, the UID is the unique id for the mode, and the name is the human readable
//  name and is set the same as the spoken text value.
//
tCIDLib::TVoid TFacCQCVoice::LoadRoomModes()
{
    const TCQCSysCfgRmInfo& scrmiUs = scrmiOurs();

    // Get the count of modes and loop through them
    const tCIDLib::TCard4 c4Count = scrmiUs.c4RoomModeCount();
    if (c4Count)
    {
        tCIDLib::TKValsList colSemVals(c4Count);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSCGlobActInfo& scliMode = scrmiUs.scliRoomModeAt(c4Index);
            colSemVals.objPlace
            (
                scliMode.m_strTitle, scliMode.m_strTitle, scliMode.m_strUniqueId
            );
        }

        // The name is the first value and the id is the second
        const TString strSemName(L"Name");
        const TString strSemId(L"Id");
        m_sprecoLocal.ReplaceOneOfRule
        (
            kCQCVoice::strRule_RoomModes, strSemName, strSemId, colSemVals
        );
    }
}


tCIDLib::TVoid TFacCQCVoice::LoadSecArmModes(const TSCSecInfo& scliSec)
{
    //
    //  In this one we have to ask the driver for available modes. It provides a
    //  backdoor command to query them, as a comma separated list.
    //
    try
    {
        const TString& strMoniker = scliSec.m_strMoniker;
        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

        TString strModes;
        if (orbcDrv->bQueryTextVal(strMoniker, L"QSecPanelInfo", L"SecArmInfo", strModes))
        {
            // Parse out the values
            tCIDLib::TStrList colModes;
            TStringTokenizer stokModes(&strModes, L",");
            const tCIDLib::TCard4 c4Count = stokModes.c4BreakApart(colModes);
            if (c4Count)
            {
                m_sprecoLocal.ReplaceOneOfRule(kCQCVoice::strRule_ArmModes, colModes);

                // Remember that we have arming info
                SetCapFlag(tCQCVoice::ECapFlags::SecArmModes, kCIDLib::True);
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


//
//  This guy is called to load up any security zones configured for this room. The
//  text is either the alt name (if available) or the actual zone name otherwise.
//
tCIDLib::TVoid TFacCQCVoice::LoadSecZones()
{
    const TCQCSysCfgRmInfo& scrmiUs = scrmiOurs();

    // Get the count of zones and loop through them
    const tCIDLib::TCard4 c4Count = scrmiUs.c4SecZoneCount();
    if (c4Count)
    {
        tCIDLib::TKVPList colSemVals;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TString& strName = scrmiUs.strZoneNameAt(c4Index);
            const TString& strAltName = scrmiUs.strZoneAltNameAt(c4Index);
            colSemVals.objPlace
            (
                strAltName.bIsEmpty() ? strName : strAltName, strName
            );
        }

        // No semantic name, they just set the basic 'out' value
        m_sprecoLocal.ReplaceOneOfRule
        (
            kCQCVoice::strRule_SecZones, TString::strEmpty(), colSemVals
        );

        // Indicate we have at least one security zone
        SetCapFlag(tCQCVoice::ECapFlags::SecZones, kCIDLib::True);
    }
}


//
//  This is called when we enter and leave conversation mode. Currently we aren't doing
//  anything, but it could be useful.
//
tCIDLib::TVoid TFacCQCVoice::SetConversationMode(const tCIDLib::TBoolean bToSet)
{
}
