//
// FILE NAME: CQCIR_BlasterDevModelInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/30/2002
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
//  This is just a simple class used to transmit information to the client
//  about the available devices. We don't want to transmit the entire files,
//  which can be large since they contain IR data, so we just send some header
//  information for each device.
//
//  To support an in-memory lookup key, we also store the name of the file
//  that holds the data.
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
//   CLASS: TIRBlasterDevModelInfo
//  PREFIX: irbdmi
// ---------------------------------------------------------------------------
class CQCIREXPORT TIRBlasterDevModelInfo : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TIRBlasterDevModelInfo& irbdmiSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIRBlasterDevModelInfo();

        TIRBlasterDevModelInfo
        (
            const   TString&                strModel
            , const TString&                strMake
            , const TString&                strDescr
            , const TString&                strDataFile
            , const tCQCKit::EDevCats       eCategory
        );

        TIRBlasterDevModelInfo
        (
            const   TString&                strDataFile
            , const TIRBlasterDevModel&     irbdmSrc
        );

        TIRBlasterDevModelInfo(const TIRBlasterDevModelInfo&) = default;
        TIRBlasterDevModelInfo(TIRBlasterDevModelInfo&&) = default;

        ~TIRBlasterDevModelInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIRBlasterDevModelInfo& operator=(const TIRBlasterDevModelInfo&) = default;
        TIRBlasterDevModelInfo& operator=(TIRBlasterDevModelInfo&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::EDevCats eCategory() const;

        tCQCKit::EDevCats eCategory
        (
            const   tCQCKit::EDevCats       eToSet
        );

        const TString& strDataFile() const;

        const TString& strDataFile
        (
            const   TString&                strToSet
        );

        const TString& strDescription() const;

        const TString& strDescription
        (
            const   TString&                strToSet
        );

        const TString& strMake() const;

        const TString& strMake
        (
            const   TString&                strToSet
        );

        const TString& strModel() const;

        const TString& strModel
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid UpdateFrom
        (
            const   TIRBlasterDevModel&     irbdmSrc
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
        //  m_eCategory
        //      The category of device. This is just for users to more quickly
        //      find devices.
        //
        //  m_strDataFile
        //      The file that holds the IR data for this device.
        //
        //  m_strDescr
        //      The descriptive name of the command, which should be short and
        //      to the point, usually 10 to 15 words.
        //
        //  m_strMake
        //      The make indentification of the device. This will be used for
        //      sorting of the list of devices for selection by human.
        //
        //  m_strModel
        //      The model of the device. This is the key field, and it is case
        //      sensitive. All model names must be unique.
        // -------------------------------------------------------------------
        tCQCKit::EDevCats   m_eCategory;
        TString             m_strDataFile;
        TString             m_strDescr;
        TString             m_strMake;
        TString             m_strModel;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TIRBlasterDevModelInfo,TObject)
};

#pragma CIDLIB_POPPACK


