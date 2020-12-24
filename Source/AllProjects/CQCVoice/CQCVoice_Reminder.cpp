//
// FILE NAME: CQCVoice_Reminder.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/08/2017
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
//  This is the implementation for the reminder class.
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
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCVoiceReminder, TObject)



// ---------------------------------------------------------------------------
//   CLASS: TCQCVoiceReminder
//  PREFIX: cvrem
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TCQCVoiceReminder::eCompByTime( const   TCQCVoiceReminder&    cvrem1
                                , const TCQCVoiceReminder&    cvrem2)
{
    if (cvrem1.m_enctAt < cvrem2.m_enctAt)
        return tCIDLib::ESortComps::FirstLess;
    else if (cvrem1.m_enctAt > cvrem2.m_enctAt)
        return tCIDLib::ESortComps::FirstGreater;

    return tCIDLib::ESortComps::Equal;
}


// ---------------------------------------------------------------------------
//  TCQCVoiceReminder: Constructors and destructor
// ---------------------------------------------------------------------------
TCQCVoiceReminder::TCQCVoiceReminder() :

    m_enctAt(0)
    , m_c4UniqueId(0)
{
}


TCQCVoiceReminder::TCQCVoiceReminder(const  TString&        strText
                                    , const tCIDLib::TCard4 c4Minutes
                                    , const tCIDLib::TCard4 c4UniqueId) :

    m_c4UniqueId(c4UniqueId)
    , m_enctAt(TTime::enctNowPlusMins(c4Minutes))
    , m_strText(strText)
{
}

TCQCVoiceReminder::TCQCVoiceReminder(const TCQCVoiceReminder& cvremSrc) :

    m_c4UniqueId(cvremSrc.m_c4UniqueId)
    , m_enctAt(cvremSrc.m_enctAt)
    , m_strText(cvremSrc.m_strText)
{
}

TCQCVoiceReminder::~TCQCVoiceReminder()
{
}


// ---------------------------------------------------------------------------
//  TCQCVoiceReminder: Public operators
// ---------------------------------------------------------------------------

TCQCVoiceReminder& TCQCVoiceReminder::operator=(const TCQCVoiceReminder& cvremSrc)
{
    if (this != &cvremSrc)
    {
        m_c4UniqueId  = cvremSrc.m_c4UniqueId;
        m_enctAt      = cvremSrc.m_enctAt;
        m_strText     = cvremSrc.m_strText;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCVoiceReminder: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Return our unique id
tCIDLib::TCard4 TCQCVoiceReminder::c4UniqueId() const
{
    return m_c4UniqueId;
}


// Set or get our run at time
tCIDLib::TEncodedTime TCQCVoiceReminder::enctAt() const
{
    return m_enctAt;
}

tCIDLib::TEncodedTime TCQCVoiceReminder::enctAt(const tCIDLib::TEncodedTime enctToSet)
{
    m_enctAt = enctToSet;
    return m_enctAt;
}


// Format our time out in HH/MM time in the appropriate locale form
tCIDLib::TVoid TCQCVoiceReminder::FormatTime(TString& strToFill) const
{
    TTime tmAt(m_enctAt);
    if (TLocale::b24Hour())
        tmAt.FormatToStr(strToFill, TTime::str24HHMM());
    else
        tmAt.FormatToStr(strToFill, TTime::strHHMM());
}


// Make the text available for speaking
const TString& TCQCVoiceReminder::strText() const
{
    return m_strText;
}


// Set the at time to now plus the new minutes
tCIDLib::TVoid TCQCVoiceReminder::UpdateTime(const tCIDLib::TCard4 c4Minutes)
{
    m_enctAt = TTime::enctNowPlusMins(c4Minutes);
}
