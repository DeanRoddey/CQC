//
// FILE NAME: CQCKit_DriverStats.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/23/2016
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
//  This is the header for the CQCKit_DriverStats.cpp file, which implements a very simple
//  class that is used to return driver statistics from CQCServer via the CQCSErver admin
//  IDL interface.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCDrvStates
//  PREFIX: cdstats
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCDrvStats : public TObject, public MStreamable
{
    public :
        // ---------------------------------------------------------------------------
        //  Constructors and Destructor
        // ---------------------------------------------------------------------------
        TCQCDrvStats();

        TCQCDrvStats(const TCQCDrvStats&) = default;
        TCQCDrvStats(TCQCDrvStats&&) = delete;

        ~TCQCDrvStats();


        // ---------------------------------------------------------------------------
        //  Public operators
        // ---------------------------------------------------------------------------
        TCQCDrvStats& operator=(const TCQCDrvStats&) = default;
        TCQCDrvStats& operator=(TCQCDrvStats&&) = delete;

        tCIDLib::TBoolean operator==
        (
            const   TCQCDrvStats&           cdstatsSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCDrvStats&           cdstatsSrc
        )   const;


        // ---------------------------------------------------------------------------
        //  Public, non-virtual methods
        // ---------------------------------------------------------------------------
        tCIDLib::TVoid Reset();


        // ---------------------------------------------------------------------------
        //  Public data members
        // ---------------------------------------------------------------------------
        tCQCKit::EDrvStates     m_eStatus;
        tCQCKit::EVerboseLvls   m_eVerboseLvl;
        tCIDLib::TCard4         m_c4ThreadId;
        tCIDLib::TCard4         m_c4BadMsgs;
        tCIDLib::TCard4         m_c4FailedWrites;
        tCIDLib::TCard8         m_c8LoadTime;
        tCIDLib::TCard4         m_c4LostCommRes;
        tCIDLib::TCard4         m_c4LostConns;
        tCIDLib::TCard4         m_c4Naks;
        tCIDLib::TCard4         m_c4Reconfigures;
        tCIDLib::TCard4         m_c4Timeouts;
        tCIDLib::TCard4         m_c4UnknownMsgs;
        tCIDLib::TCard4         m_c4UnknownWrites;


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
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDrvStats,TObject)
};

#pragma CIDLIB_POPPACK
