//
// FILE NAME: ZWaveUSB3Sh_Unit_NChOutlet_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2017
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
//  There are various devices out there are provide N number of controllable outlets
//  in a single box. Most of these will use the multi-channel capabilities of Z-Wave
//  to allow each of them to be separately addressed.
//
//  This class should be able to deal with all of them, as long as they follow the
//  normal scheme of things, not at all a given in Z-Wave world of course.
//
//  For each outlet we create a field named: [name]_[epname], so the name given to the
//  the unit by the user, followed by a name given to each end point. The latter is
//  done via a unit option, one per end point. We set an initial value of OutletX where
//  X is the end point id.
//
//  The extra info for our module types has these values, in this orrder.
//
//      Extra1=EP count
//      Extra2=Command Class
//      Extra3=StartNum
//
//  The end point is the number of outlets. The class is the class that represents the
//  values for the outlets. Usually it'll be binary switch, but it might be basic. Those
//  are currently the only ones we watch for. StartNum is the first end point number.
//  Normally it should be 1, but just in case. We assume they are assigned contiguously
//  from there.
//
//  For incoming messages it doesn't matter. We'll respond to either. But we need to
//  know what to send out in response to field writes.
//
//
//  For now, we assume these are read/write. We could add a unit level extra info
//  value later to control access. We don't want to do it in the CCs since we'd
//  have to redundantly do it for every end point, when they are all going to be
//  the same. So, for now, we force the access when we create the CCs.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TNChOutletUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TNChOutletUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TNChOutletUnit() = delete;

        TNChOutletUnit
        (
                TZWUnitInfo* const      punitiOwner
        );

        TNChOutletUnit(const TNChOutletUnit&) = delete;

        ~TNChOutletUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TNChOutletUnit& operator=(const TNChOutletUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseExtraInfo() override;

        tCIDLib::TBoolean bPrepare() override;

        tCIDLib::TVoid QueryUnitAttrs
        (
                    tCIDMData::TAttrList&   colToFill
            ,       TAttrData&              attrTmp
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4EPCount
        //      The number of outlets (End Points) our device has. This comes from the
        //      device info file.
        //
        //  m_c4FirstEPNum
        //      The number of the first end point in the set. See the class level
        //      comments above.
        //
        //  m_eClass
        //      The class we should send out in response to field writes. See the
        //      class comments above.
        //
        //  m_fcolImplIds
        //      The ids of the CC impl objects we register on our base class to handle
        //      each end point.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4EPCount;
        tCIDLib::TCard4         m_c4FirstEPNum;
        tZWaveUSB3Sh::ECClasses m_eClass;
        tCIDLib::TCardList      m_fcolImplIds;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TNChOutletUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
