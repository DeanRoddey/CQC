//
// FILE NAME: GC100S_AddrMapItem.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/18/2003
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
//  This is the header file for the GC-100_AddrMapItem.cpp file, which
//  implements a helper class used to map between our field ids and the
//  target addresses on the GC-100. The GC boxes are hardware configurable
//  and we figure out what options it has, and what address values we use
//  to address each one, at runtime.
//
//  So we need to keep up with a field id, a module number, and a connector
//  number, and a connector type. A GC-100 target connector is indicated
//  by m:c, i.e. module number and connector number.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TGC100Addr
//  PREFIX: gca
// ---------------------------------------------------------------------------
class TGC100Addr : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGC100Addr();

        TGC100Addr
        (
            const   TGC100Addr&             gcaToCopy
        );

        ~TGC100Addr();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGC100Addr& operator=
        (
            const   TGC100Addr&             gcaToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ConnNum() const;

        tCIDLib::TCard4 c4FldId() const;

        tCIDLib::TCard4 c4FldId
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4ModuleNum() const;

        tGC100::EModTypes eModType() const;

        const TString& strName() const;

        tCIDLib::TVoid Set
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4ConnNum
            , const tCIDLib::TCard4         c4ModuleNum
            , const tGC100::EModTypes       eModType
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4ConnNum
        //      The GC-100 connector number (within the module) that this
        //      address belongs to.
        //
        //  m_c4FldId
        //      The id of the CQC field associated with this GC-100 address.
        //
        //  m_c4ModuleNum
        //      The GC-100 module number this address belongs to.
        //
        //  m_eModType
        //      The type of module this address is for. This let's us do some
        //      sanity checking that the correct type of operation is
        //      requested.
        //
        //  m_strName
        //      The name we gave to this field. They are generated based on
        //      the connector type and number.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4ConnNum;
        tCIDLib::TCard4     m_c4FldId;
        tCIDLib::TCard4     m_c4ModuleNum;
        tGC100::EModTypes   m_eModType;
        TString             m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGC100Addr,TObject)
};

#pragma CIDLIB_POPPACK



