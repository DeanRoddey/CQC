//
// FILE NAME: CQCVoice_Reminder.hpp
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
//  This is the header for the reminder class. We have a command lets the user ask
//  us to queue up a reminder for some number of minutes in the future.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCVoiceReminder
//  PREFIX: cvrem
// ---------------------------------------------------------------------------
class TCQCVoiceReminder : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompByTime
        (
            const   TCQCVoiceReminder&      cvrem1
            , const TCQCVoiceReminder&      cvrem2
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TCQCVoiceReminder();

        TCQCVoiceReminder
        (
            const   TString&                strText
            , const tCIDLib::TCard4         c4Minutes
            , const tCIDLib::TCard4         c4UniqueId
        );

        TCQCVoiceReminder
        (
            const   TCQCVoiceReminder&      cvremSrc
        );

        ~TCQCVoiceReminder();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCVoiceReminder& operator=
        (
            const   TCQCVoiceReminder&      cvremSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4UniqueId() const;

        tCIDLib::TEncodedTime enctAt() const;

        tCIDLib::TEncodedTime enctAt
        (
            const   tCIDLib::TEncodedTime   enctToSet
        );

        tCIDLib::TVoid FormatTime
        (
                    TString&                strToFill
        )   const;

        const TString& strText() const;

        tCIDLib::TVoid UpdateTime
        (
            const   tCIDLib::TCard4         c4Minutes
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4UniqueId
        //      Each new reminder is assigned a unique id.
        //
        //  m_enctAt
        //      The pre-calculated time that this reminder should be given. Once
        //      the current time is at or beyond this, it should be spoken.
        //
        //  m_strText
        //      The reminder text
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4UniqueId;
        tCIDLib::TEncodedTime   m_enctAt;
        TString                 m_strText;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCVoiceReminder, TObject);
};


#pragma CIDLIB_POPPACK
