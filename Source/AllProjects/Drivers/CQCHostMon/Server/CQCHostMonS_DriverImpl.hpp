//
// FILE NAME: CQCHostMonS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/13/2000
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
//  This is the header file for the actual driver class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: THostMonS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TCQCHostMonSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCHostMonSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TCQCHostMonSDriver();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TCQCHostMonSDriver();
        TCQCHostMonSDriver(const TCQCHostMonSDriver&);
        tCIDLib::TVoid operator=(const TCQCHostMonSDriver&);


        // -------------------------------------------------------------------
        //  We use this simple class to maintain a collection of volumes
        //  internally. We store one of these for each non-removeable volumes.
        //  We provide a string field to put the data in for access by clients.
        //
        //  We store the field id for the field this one maps to, to allow for
        //  quicker access when we have changes to store.
        // -------------------------------------------------------------------
        class TVolInfo
        {
            public :
                TVolInfo(const  TString&            strVol
                        , const tCIDLib::TCard4     c4TotalMB
                        , const tCIDLib::TCard4     c4FreeMB) :

                    m_c4FldId(kCIDLib::c4MaxCard)
                    , m_c4FreeMB(c4FreeMB)
                    , m_c4TotalMB(c4TotalMB)
                    , m_strVolume(strVol)
                {
                }

                ~TVolInfo()
                {
                }

                tCIDLib::TCard4 m_c4FldId;
                tCIDLib::TCard4 m_c4FreeMB;
                tCIDLib::TCard4 m_c4TotalMB;
                TString         m_strVolume;
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid InitFields();

        tCIDLib::TVoid LoadFields
        (
                    TVector<TCQCFldDef>&    colToFill
        );

        tCIDLib::TVoid RefreshVolumes
        (
            const   tCIDLib::TBoolean       bForce
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldIdXXX
        //      After we register our fields, we look up the ids of the ones
        //      that we have to keep updating, so that we can do it via the
        //      more efficient id rather than by name. Note that this doesn't
        //      include the volume field ids, which are stored in their
        //      respective volume info objects.
        //
        //  m_c4VolSerialNum
        //      The TFileSys class maintains a serial number that gets
        //      bumped any time the volume list is updated. We watch this
        //      so that we know when to update our list of volumes.
        //
        //  m_c8PhysicalMem
        //      In order to calc the percent usage of memory, we store the
        //      full amount here. It's a field as well, but the field is in
        //      K and the calculation we need this for is in MB and as a float,
        //      so it easier to just keep it around since it won't ever change.
        //
        //  m_colVolInfo
        //      This is where we store the volume info that was seen on the
        //      last cycle of the poll thread.
        //
        //  m_strFldXXX
        //      THe names of our fields, so that we don't have a bunch of
        //      hard coded names about.
        //
        //  m_strTmpFmt
        //      To avoid creating and destroying a string on every poll cycle
        //      we use this member during our poll cyle to format data.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldIdPerMemUsed;
        tCIDLib::TCard4     m_c4VolSerialNum;
        tCIDLib::TFloat8    m_f8PhysicalMB;
        TVector<TVolInfo>   m_colVolInfo;
        const TString       m_strFldCPUCount;
        const TString       m_strFldCPUType;
        const TString       m_strFldHostName;
        const TString       m_strFldOSVersion;
        const TString       m_strFldPerMemUsed;
        const TString       m_strFldPhysMemK;
        TString             m_strTmpFmt;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCHostMonSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK

