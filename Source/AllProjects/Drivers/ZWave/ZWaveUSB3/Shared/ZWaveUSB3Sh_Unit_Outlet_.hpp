//
// FILE NAME: ZWaveUSB3Sh_Unit_Outlet.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/02/2018
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
//  This is a handler for switchable outlets. We have a write only or a read/write
//  field that reflects (or controls) the state of the outlet. The extra info in
//  the device info file tells us whether its write only or read/write, and it indicates
//  the CC to use.
//
//      IOType=[Write | ReadWrite]
//      OutletCC=[Basic | BinSwitch]
//
//  We can optionally accept async reports of status changes via another CC, currently
//  either notification or the outlet CC not used above for control and query.
//
//      AsyncNot=[Notification | Basic | BinSwitch]
//
//  If notify, the CC's extra info sets the notification ids.
//
//  We provide an option that lets the user tell us to set ourself up as a light
//  switch, in case this guy is basically just driving a lamp or something.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TOutletUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TOutletUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TOutletUnit() = delete;

        TOutletUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );

        TOutletUnit(const TOutletUnit&) = delete;

        ~TOutletUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TOutletUnit& operator=(const TOutletUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseExtraInfo() override;

        tCIDLib::TBoolean bPrepare() override;

        tCIDLib::TVoid ImplValueChanged
        (
            const   tCIDLib::TCard4         c4ImplId
            , const tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TBoolean       bWasInErr
            , const tCIDLib::TInt4          i4ExtraInfo
        )   override;

        tCIDLib::TVoid QueryUnitAttrs
        (
                    tCIDMData::TAttrList&   colToFill
            ,       TAttrData&              adatTmp
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAsLight
        //      Driven by a unit option, indicates if we are to create our field as a
        //      light switch, or jsut a general binary switch.
        //
        //  m_eAccess
        //      Driven by extra info, used in field creation and setting access
        //      flags on the CCs.
        //
        //  m_eNotifyClass
        //      Can be _Count if not used, else the CC to get notifications on. We will
        //      push changes from it to the outlet CC.
        //
        //  m_eOutletClass
        //      The user can tell us which class to to interact with the outlet by.
        //
        //  m_pzwcciNotify
        //      Optional async notification CC if we don't get them via the outlet CC.
        //      We don't own it, it's set on the base class.
        //
        //  m_pzwcciOutlet
        //      We keep a typed pointer to our outlet CC impl so that we can interact
        //      with it, but we don't own it. We store it via the bin impl base class.
        //
        //  m_strFldName
        //      We keep our built field name for use in logging or event trigers (the
        //      latter only if we are set up to be a light switch.)
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bAsLight;
        tZWaveUSB3Sh::EUnitAcc  m_eAccess;
        tZWaveUSB3Sh::ECClasses m_eNotifyClass;
        tZWaveUSB3Sh::ECClasses m_eOutletClass;
        TZWCCImplBin*           m_pzwcciNotify;
        TZWCCImplBin*           m_pzwcciOutlet;
        TString                 m_strFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TOutletUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
