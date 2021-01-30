//
// FILE NAME: CQCKit_VarDrvCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/05/2003
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
//  This is the header for the CQCKit.cpp file, which implements a config data class
//  shared by the client and server sides of the 'variable driver', which is a device
//  driver that talks to no device and just serves as a means for users to define system
//  wide variables that they can use in their macros.
//
//  Basically it's a list of field definition objects that describe the fields
//  that the driver should load. We use the 'extra config' field of the
//  objects to store a string representation of the initial value each field
//  should be given, if any.
//
//  There's not a particularly good place to put this, but it needs to be in something
//  that both sides would have to use anyway, so we put it here.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TVarDrvCfg
//  PREFIX: cfg
// ---------------------------------------------------------------------------
class CQCKITEXPORT TVarDrvCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TFldInfoList  = TVector<TCQCFldDef>;
        using TFldCursor    = TFldInfoList::TCursor;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TVarDrvCfg();

        TVarDrvCfg(const TVarDrvCfg&) = default;
        TVarDrvCfg(TVarDrvCfg&&) = default;

        ~TVarDrvCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TVarDrvCfg& operator=(const TVarDrvCfg&) = default;
        TVarDrvCfg& operator=(TVarDrvCfg&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TVarDrvCfg&             cfgToComp
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TVarDrvCfg&             cfgToComp
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bFieldExists
        (
            const   TString&                strToFind
        )   const;

        tCIDLib::TCard4 c4AddField
        (
            const   TCQCFldDef&             flddToAdd
        );

        [[nodiscard]] tCIDLib::TCard4 c4Count() const;

        const TFldInfoList& colFields() const;

        [[nodiscard]] TFldInfoList& colFields();

        [[nodiscard]] TFldCursor cursFields() const;

        const TCQCFldDef& flddAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TCQCFldDef& flddAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid ParseFromXML
        (
            const   TString&                strSrcFile
        );

        TCQCFldDef* pflddFind
        (
            const   TString&                strToFind
        );

        tCIDLib::TVoid RemoveAt
        (
            const   tCIDLib::TCard4         c4At
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ParseFromNode
        (
            const   TXMLTreeElement&        xtnodeRoot
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colFlds
        //      A vector of field definition objects that describes the the fields the
        //      driver should create, and we use the extra config field to store an
        //      initial value for each field if the user wants it to have one.
        // -------------------------------------------------------------------
        TFldInfoList    m_colFlds;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TVarDrvCfg,TObject)
};

#pragma CIDLIB_POPPACK


