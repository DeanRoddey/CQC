//
// FILE NAME: CQCHostMonC_VolInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/25/2001
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
//  This is a little class where we save info on the volumes we are
//  monitoring. We need a place to know what volumes the server is offering
//  to us, and a place to store the info that we query for the fields.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCVolPollInfo
//  PREFIX: vpi
// ---------------------------------------------------------------------------
class TCQCVolPollInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCVolPollInfo();

        TCQCVolPollInfo
        (
            const   TCQCVolPollInfo&        vpiToCopy
        );

        TCQCVolPollInfo
        (
            const   TString&                strInitValue
        );

        ~TCQCVolPollInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCVolPollInfo& operator=
        (
            const   TCQCVolPollInfo&        vpiToCopy
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCVolPollInfo&        vpiToCompare
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bChanged() const;

        tCIDLib::TCard4 c4PercentUsed() const;

        tCIDLib::TCard4 c4PercentUsed
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4PercentUsed2() const;

        tCIDLib::TCard4 c4PercentUsed2
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4TotalMB() const;

        tCIDLib::TVoid ParseValue
        (
            const   TString&                strNewValue
        );

        const TString& strVolume() const;

        const TString& strVolume
        (
            const   TString&                strToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4PercentUsed
        //  m_c4PercentUsed2
        //      This is the percent of the volume space used. The 2 version
        //      is the latest value set by the poll thread. Once the GUI
        //      updates, it copies this to the other version, then it knows
        //      that is up to date.
        //
        //  m_c4TotalMB
        //      The total amount of storage on the volume, in MB.
        //
        //  m_strVolume
        //      The name for to the volume that this info represents. For
        //      Win32, this is like C:\. For other platforms it might be
        //      some other form.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4PercentUsed;
        tCIDLib::TCard4 m_c4PercentUsed2;
        tCIDLib::TCard4 m_c4TotalMB;
        TString         m_strVolume;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCVolPollInfo,TObject)
};

#pragma CIDLIB_POPPACK


