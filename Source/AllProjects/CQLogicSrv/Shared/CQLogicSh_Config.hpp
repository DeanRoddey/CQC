//
// FILE NAME: CQLogicSh_Config.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/08/2009
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
//  This is the header for the CQLogicSh_Config.cpp file, which
//  implements the class that is used to store and pass around the driver
//  configuration data. The user has to tell us what types of virtual fields
//  he wants the driver to have.
//
//  We support various types of fields, which are driven by the values of other
//  fields (mostly, we do have passive ones as well.) So we have configuration
//  classes for each type of field, and store a polymorphic list of them.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQLogicSrvCfg
//  PREFIX: lscfg
// ---------------------------------------------------------------------------
class CQLOGICSHEXPORT TCQLogicSrvCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TCQLogicSrvCfg();

        TCQLogicSrvCfg
        (
            const   TCQLogicSrvCfg&       lscfgSrc
        );

        TCQLogicSrvCfg(TCQLogicSrvCfg&&) = delete;

        ~TCQLogicSrvCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQLogicSrvCfg& operator=
        (
            const   TCQLogicSrvCfg&       lscfgSrc
        );

        TCQLogicSrvCfg& operator=(TCQLogicSrvCfg&&) = delete;

        tCIDLib::TBoolean operator==
        (
            const   TCQLogicSrvCfg&       lscfgComp
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQLogicSrvCfg&       lscfgComp
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddField
        (
                    TCQLSrvFldType* const   pclsftToAdd
        );

        tCIDLib::TBoolean bFindByName
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4Index
        )   const;

        tCIDLib::TBoolean bFindById
        (
            const   tCIDLib::TCard4         c4ToFind
            ,       tCIDLib::TCard4&        c4Index
        )   const;

        tCIDLib::TCard4 c4FldCount() const;

        const tCQLogicSh::TFldList& colFldTypes() const;

        tCQLogicSh::TFldList& colFldTypes();

        tCIDLib::TVoid DeleteField
        (
            const   tCIDLib::TCard4         c4At
        );

        TCQLSrvFldType* pclsftAt
        (
            const   tCIDLib::TCard4         c4Index
        );

        const TCQLSrvFldType* pclsftAt
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        tCIDLib::TVoid Reset();


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
        //  Private, non-virtual methods
        //
        //  m_colFldTypes
        //      This is our main bit of configuration. It's a polymorphic list
        //      of objects derived from the field type class. Each one
        //      represents a configured virtual fields.
        // -------------------------------------------------------------------
        tCQLogicSh::TFldList m_colFldTypes;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQLogicSrvCfg,TObject)
};


#pragma CIDLIB_POPPACK

