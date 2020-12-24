//
// FILE NAME: CQCSysCfg_VoiceCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/29/2017
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
//  This file defines the per-room configuration for voice control. Currently that
//  is Kinect based.
//
//  We let the user enable or disable voice control, select a Kinect (by host name)
//  for that room, set the trigger keyword, etc...
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCSysCfgVoice
//  PREFIX: scri
// ---------------------------------------------------------------------------
class CQCSYSCFGEXP TCQCSysCfgVoice : public TObject, public MStreamable
{
    public :
        TCQCSysCfgVoice();

        TCQCSysCfgVoice(const TCQCSysCfgVoice&) = default;
        TCQCSysCfgVoice(TCQCSysCfgVoice&&) = default;

        ~TCQCSysCfgVoice();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSysCfgVoice& operator=(const  TCQCSysCfgVoice&) = default;
        TCQCSysCfgVoice& operator=(TCQCSysCfgVoice&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCSysCfgVoice&       scvcSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCSysCfgVoice&       scvcSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bEnable() const;

        tCIDLib::TBoolean bEnable
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid Reset();

        const TString& strHost() const;

        const TString& strHost
        (
            const   TString&                strToSet
        );

        const TString& strKeyWord() const;

        const TString& strKeyWord
        (
            const   TString&                strToSet
        );


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
        //  Private data members
        //
        //  m_bEnable
        //      Enable or disable voice control for the room.
        //
        //  m_strHost
        //      The name of the host machine that is attached to the Kinect that should
        //      control this room.
        //
        //  m_strKeyWord
        //      The keyword to use for this room.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bEnable;
        TString             m_strHost;
        TString             m_strKeyWord;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSysCfgVoice, TObject)
};

#pragma CIDLIB_POPPACK

