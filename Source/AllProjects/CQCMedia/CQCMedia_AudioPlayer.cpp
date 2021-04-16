//
// FILE NAME: CQCMedia_AudioPlayer.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/21/2007
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
//  This file just contains the small bit of non-pure virtual stuff that
//  the audio player base class has.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCMedia_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCAudioPlayer,TObject)



// ---------------------------------------------------------------------------
//  CLASS: TCQCAudioPlayer::TEvData
// PREFIX: evd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCAudioPlayer::TEvData: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCAudioPlayer::TEvData::TEvData(const EEvTypes eType) :

    m_bVal(kCIDLib::False)
    , m_c4Val(0)
    , m_eType(eType)
    , m_evWait(tCIDLib::EEventStates::Reset)
    , m_pstrVal1(0)
    , m_pstrVal2(0)
    , m_perrFailure(0)
{
}

TCQCAudioPlayer::TEvData::TEvData(  const   EEvTypes eType
                                    , const TString& strVal1
                                    , const TString& strVal2) :

    m_bVal(kCIDLib::False)
    , m_c4Val(0)
    , m_eType(eType)
    , m_evWait(tCIDLib::EEventStates::Reset)
    , m_pstrVal1(nullptr)
    , m_pstrVal2(nullptr)
    , m_perrFailure(nullptr)
{
    m_pstrVal1 = new TString(strVal1);
    m_pstrVal2 = new TString(strVal2);
}

TCQCAudioPlayer::TEvData::~TEvData()
{
    // Delete the string members if we allocated them
    if (m_pstrVal1)
    {
        try
        {
            delete m_pstrVal1;
            m_pstrVal1 = nullptr;
        }

        catch(...)
        {
        }
    }

    if (m_pstrVal2)
    {
        try
        {
            delete m_pstrVal2;
            m_pstrVal2 = nullptr;
        }

        catch(...)
        {
        }
    }

    // And the error object
    if (m_perrFailure)
    {
        try
        {
            delete m_perrFailure;
            m_perrFailure = nullptr;
        }

        catch(...)
        {
        }
    }
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCAudioPlayer
// PREFIX: cwmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCAudioPlayer: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCAudioPlayer::bFindDevGUID(const TString& strName, TString& strGUID)
{
    // Get a list of of devices with the name as the key
    tCIDLib::TKVPList colList;
    const tCIDLib::TCard4 c4Count = TAudio::c4EnumAudioDevices(colList);

    //
    //  And now run through them and see if the passed name is present. We
    //  check the passed name and if it starts with DirectShow: we strip it
    //  off because our enumeration method doesn't return that form of
    //  name, but the we we used to do it did, so names in that form are
    //  still stored in driver config.
    //
    TString strActualName(strName);
    if (strActualName.bStartsWith(L"DirectShow: "))
        strActualName.Cut(0, 12);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (colList[c4Index].strKey() == strActualName)
        {
            strGUID = colList[c4Index].strValue();
            return kCIDLib::True;
        }
    }

    // Never found it
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCQCAudioPlayer: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TCQCAudioPlayer::c4DefVolume() const noexcept
{
    return m_c4DefVolume;
}


// ---------------------------------------------------------------------------
//  TCQCAudioPlayer: Destructor
// ---------------------------------------------------------------------------
TCQCAudioPlayer::~TCQCAudioPlayer()
{
}


// ---------------------------------------------------------------------------
//  TCQCAudioPlayer: Hidden Constructors
// ---------------------------------------------------------------------------
TCQCAudioPlayer::TCQCAudioPlayer(const tCIDLib::TCard4 c4DeVolume) :

    m_c4DefVolume(c4DeVolume)
{
}

