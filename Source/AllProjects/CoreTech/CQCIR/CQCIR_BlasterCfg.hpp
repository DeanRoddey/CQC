//
// FILE NAME: CQCIR_BlasterCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/24/2002
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
//  This class represents the overall configuration data for IR device
//  drivers. It contains some top level data, and a list of TIRDevModel
//  objects, each of which in turn contains a list of TIRBlasterCmd
//  objects.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TIRBlasterCfg
//  PREFIX: irbc
// ---------------------------------------------------------------------------
class CQCIREXPORT TIRBlasterCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Class specific types
        // -------------------------------------------------------------------
        using TModelMap    = TKeyedHashSet<TIRBlasterDevModel, TString, TStringKeyOps>;
        using TModelCursor = TModelMap::TCursor;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIRBlasterCfg();

        TIRBlasterCfg(const TIRBlasterCfg&) = default;
        TIRBlasterCfg(TIRBlasterCfg&&) = default;

        ~TIRBlasterCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIRBlasterCfg& operator=(const TIRBlasterCfg&) = default;
        TIRBlasterCfg& operator=(TIRBlasterCfg&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bModelExists
        (
            const   TString&                strModelToFind
        )   const;

        tCIDLib::TBoolean bRemoveModel
        (
            const   TString&                strDevModel
        );

        TModelCursor cursModels() const;

        TIRBlasterCmd& irbcAddOrUpdate
        (
            const   TString&                strDevModel
            , const TIRBlasterCmd&          irbcToAdd
            ,       tCIDLib::TBoolean&      bAdded
        );

        const TIRBlasterCmd& irbcFromName
        (
            const   TString&                strDevModel
            , const TString&                strCmdName
            ,       tCIDLib::TCard4&        c4RepeatCount
        )   const;

        TIRBlasterDevModel& irbdmAdd
        (
            const   TString&                strDevModel
            , const TString&                strDevMake
            , const TString&                strDescr
            , const tCQCKit::EDevCats       eCategory
        );

        TIRBlasterDevModel& irbdmAddOrUpdate
        (
            const   TIRBlasterDevModel&     irbdmToAdd
            ,       tCIDLib::TBoolean&      bAdded
        );

        const TIRBlasterDevModel& irbdmFromName
        (
            const   TString&                strDevModel
        )   const;

        TIRBlasterDevModel& irbdmFromName
        (
            const   TString&                strDevModel
        );

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
        //  Private data members
        //
        //  m_colModelMap
        //      A collection of IR device models, keyed on the model name.
        // -------------------------------------------------------------------
        TModelMap   m_colModelMap;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TIRBlasterCfg,TObject)
};

#pragma CIDLIB_POPPACK


