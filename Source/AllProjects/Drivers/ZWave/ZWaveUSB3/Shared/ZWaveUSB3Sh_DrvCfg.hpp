//
// FILE NAME: ZWaveUSB3Sh_DrvCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2017
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
//  This is the header file for the ZWaveUSB3Sh_DrvConfig.cpp file, which implements the
//  TZWaveDrvCfg class. This is a class that holds all our persistent configuration info.
//  Mostly it's our list of unit info objects. These are stored sorted by unit name, for
//  fast by name lookup (for things like insuring no dups. We also have to do by id lookups
//  but that's a lot quicker since it's just a numeric comparison so we just do an exhaustive
//  search in that case.
//
//  We remember the last home id and unit id assigned to our controller. This lets us know
//  if something has changed the next time we start up. We start off with a stored name of
//  CQCZWCtrl, but will accept a new name via the naming class, which will happen at least
//  during our initial inclusion into the network.
//
//  To support efficient updates by the client driver each unit info object keeps a
//  serial number which it bumps any time any change is made to it. This may actually
//  bump it multiple times, which doesn't make any difference, as long as it changes.
//  We have a method that will generate a hash of the current unit serial numbers. The
//  client can do the same with his. If any have changed he knows he needs to query the
//  latest info from the server. This means that the unit info class MUST increment its
//  serial number on any change.
//
//  Normally this would only happen if two different admins accessed the client side
//  driver at the same time. But in this driver we can get battery powered devices
//  waking up and we can then go out and auto-discover them. That will change the info
//  for that unit. If the user saves his local changes, he will overwrite this again.
//  So we need to be able to let him know when changes have occurred.
//
//  State persistence
//
//  The unit info objects must stream their state, so that it gets to the client when
//  it requests config or unit info. But, on each startup, we want to push each unit
//  back to the initial state so that it will re-evaluate itself and catch any changes.
//  So we provide a state reset method that the driver will call after it loads up the
//  config. It will also do this after it does a replication.
//
//  This doesn't mean it resets other stuff. If the unit already has make/model info
//  that will be seen and it will just skip forward to re-verifying the man ids.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveDrvCfg
//  PREFIX: dcfg
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWaveDrvCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveDrvCfg();

        TZWaveDrvCfg
        (
            const   TZWaveDrvCfg&        dcfgSrc
        );

        ~TZWaveDrvCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWaveDrvCfg& operator=
        (
            const   TZWaveDrvCfg&        dcfgSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TZWaveDrvCfg&        dcfgSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWaveDrvCfg&        dcfgSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAddUnit
        (
            const   TZWUnitInfo&            unitiToAdd
        );

        tCIDLib::TBoolean bInNetwork() const;

        tCIDLib::TBoolean bInNetwork
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIsEmpty() const;

        tCIDLib::TBoolean bUnitsToApprove() const;

        tCIDLib::TBoolean bUnitExists
        (
            const   TString&                strNameToCheck
        )   const;

        tCIDLib::TBoolean bUpdateUnit
        (
            const   TZWUnitInfo&            unitNew
            ,       tCIDLib::TCard4&        c4At
        );

        tCIDLib::TBoolean bValidate
        (
                    TString&                strErr
        );

        tCIDLib::TCard1 c1CtrlId() const
        {
            return m_c1CtrlId;
        }

        tCIDLib::TCard1 c1MasterId() const;

        tCIDLib::TCard1 c1MasterId
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        tCIDLib::TCard4 c4CalcSNHash() const;

        tCIDLib::TCard4 c4HomeId() const
        {
            return m_c4HomeId;
        }

        tCIDLib::TCard8 c8ManIds() const
        {
            return m_c8ManIds;
        }

        tCIDLib::TCard4 c4UnitCount() const
        {
            return m_colUnits.c4ElemCount();
        }

        const tZWaveUSB3Sh::TUnitInfoList& colUnits() const;

        tZWaveUSB3Sh::TUnitInfoList& colUnits();

        tCIDLib::TVoid FormatReport
        (
                    TTextOutStream&         strmTar
            , const TString&                strMoniker
        )   const;

        tCIDLib::TVoid LoadControllerInfo
        (
                    MZWaveCtrlIntf&         mzwciIO
        );

        const TZWUnitInfo* punitiFindById
        (
            const   tCIDLib::TCard1         c1ToFind
        )   const;

        TZWUnitInfo* punitiFindById
        (
            const   tCIDLib::TCard1         c1ToFind
        );

        const TZWUnitInfo* punitiFindByName
        (
            const   TString&                strToCheck
        )   const;

        TZWUnitInfo* punitiFindByName
        (
            const   TString&                strToCheck
        );

        tCIDLib::TVoid QueryZWLibVer
        (
                    tCIDLib::TCard4&        c4ZWLibMajVer
            ,       tCIDLib::TCard4&        c4ZWLibMinVer
        );

        tCIDLib::TVoid PrepForUse
        (
                    MZWaveCtrlIntf* const   pmzwciOwner
        );

        tCIDLib::TVoid ProbeNewUnits
        (
                    MZWaveCtrlIntf* const   pmzwciOwner
        );

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid ResetUnitList();

        const TString& strCtrlModel() const
        {
            return m_strCtrlModel;
        }

        const TString& strCtrlName() const;

        const TString& strCtrlName
        (
            const   TString&                strToSet
        );

        const TString& strFWVersion() const
        {
            return m_strFWVersion;
        }

        const TZWUnitInfo& unitiAt
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        const TZWUnitInfo& unitiFindById
        (
            const   tCIDLib::TCard1         c1ToFind
        )   const;

        TZWUnitInfo& unitiFindById
        (
            const   tCIDLib::TCard1         c1ToFind
        );

        const TZWUnitInfo& unitiFindByName
        (
            const   TString&                strToFind
        )   const;

        TZWUnitInfo& unitiFindByName
        (
            const   TString&                strToFind
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bInNetwork
        //      Last we knew we were in the network.
        //
        //  m_bIsSUC
        //      Last we knew we were the system update controller.
        //
        //  m_bsUnitMap
        //      A map of bits for each unit that was found. The index of a bit corresponds
        //      with its unit id minus 1 (unit ids are 1 based.)
        //
        //  m_c1CtrlId
        //  m_c4HomeId
        //      The last unit id and home id that was assigned to our controller.
        //
        //  m_c1MasterId
        //      If we are in the network, then this is the master id we remembered from
        //      the replication process.
        //
        //  m_c4SerAPIVersion
        //      The last serial API vresion we got from the controller.
        //
        //  m_c4ZWLibMajVer
        //  m_c4ZWLibMinVer
        //      The Z-Wave library version info we last got from the controller.
        //
        //  m_c8ManIds
        //      The encoded manufacturer ids that we last saw for our controller. This
        //      lets us know if the hardware has changed (potentially.)
        //
        //  m_colUnits
        //      The list of unit info objects that we are configured for. We keep it
        //      sorted by unit id.
        //
        //  m_strCtrlModel
        //      Our controller's model if we can figure it out.
        //
        //  m_strCtrlName
        //      The module name assigned to us by the master. We default it initially, but
        //      accept a new name via the naming cmd class.
        //
        //  m_strFWVersion
        //      Our controller's library version formatted out for human consumption, which
        //      we use as a firmware version for V2 device info purposes.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bInNetwork;
        tCIDLib::TBoolean           m_bIsSUC;
        TBitset                     m_bsUnitMap;
        tCIDLib::TCard1             m_c1CtrlId;
        tCIDLib::TCard1             m_c1MasterId;
        tCIDLib::TCard4             m_c4HomeId;
        tCIDLib::TCard4             m_c4SerAPIVersion;
        tCIDLib::TCard4             m_c4ZWLibMajVer;
        tCIDLib::TCard4             m_c4ZWLibMinVer;
        tCIDLib::TCard8             m_c8ManIds;
        tZWaveUSB3Sh::TUnitInfoList m_colUnits;
        TString                     m_strCtrlModel;
        TString                     m_strCtrlName;
        TString                     m_strFWVersion;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveUnit,TObject)
};

#pragma CIDLIB_POPPACK

