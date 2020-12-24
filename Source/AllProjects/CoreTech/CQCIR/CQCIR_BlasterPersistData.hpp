//
// FILE NAME: CQCIR_BlasterPersistData.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/12/2003
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
//  This is the header file for the CQCIR_PersistData.cpp file. This file
//  implements the TIRBlasterPerData class. The blaster drivers don't actually
//  store fully device models, which would be dumb. They just store a list
//  of models that they've been configured to load, and then reload those
//  models when they are loaded up by CQCServer.
//
//  Some blasters may derive from this and add some more info, but this is
//  what most will use.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TIRBlasterPerData
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class CQCIREXPORT TIRBlasterPerData : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TIRBlasterCfg::TModelCursor TBlasterCursor;
        typedef TVector<TString>            TModelList;
        typedef TModelList::TCursor         TModelCursor;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIRBlasterPerData();

        TIRBlasterPerData
        (
            const   TString&                strMoniker
        );

        TIRBlasterPerData(const TIRBlasterPerData&) = default;
        TIRBlasterPerData(TIRBlasterPerData&&) = default;

        ~TIRBlasterPerData();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIRBlasterPerData& operator=(const TIRBlasterPerData&) = default;
        TIRBlasterPerData& operator=(TIRBlasterPerData&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddModel
        (
            const   TString&                strToAdd
        );

        TModelCursor cursModels() const;

        tCIDLib::TVoid RemoveModel
        (
            const   TString&                strToRemove
        );

        const TString& strMoniker() const;

        tCIDLib::TVoid SetModels
        (
                    TBlasterCursor          cursModels
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
        //  m_colModels
        //      This is a list of the device models that we've been configured
        //      to load up. We just have to store the device model names, since
        //      we can use that to query these model's files from the data
        //      server.
        //
        //  m_strMoniker
        //      The moniker of the driver which saved us. The file name is
        //      actual based on the driver moniker, but this is just a sanity
        //      check that can be checked after the file is loaded.
        // -------------------------------------------------------------------
        TModelList  m_colModels;
        TString     m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIRBlasterPerData,TObject)
};

#pragma CIDLIB_POPPACK



