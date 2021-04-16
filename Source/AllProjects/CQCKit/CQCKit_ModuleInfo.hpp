//
// FILE NAME: CQCKit_DriverCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/23/2000
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
//  This is the header for the CQCKit_ModuleInfo.cpp file, which implements
//  a simple derivative of the TCIDModuleInfo class, which is used to hold info
//  about extra facilities that a driver might create and need to be copied
//  along with it. We just add to the base class an indicator of which sides
//  use the driver, client, server, or both, so that the apps that download
//  the drivers know where to put them.
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
//   CLASS: TCQCModuleInfo
//  PREFIX: modi
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCModuleInfo : public TCIDModuleInfo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCModuleInfo();

        TCQCModuleInfo
        (
            const   TString&                strBaseName
            , const tCIDLib::ECSSides       eSides
            , const tCIDLib::EModTypes      eModType
            , const tCIDLib::TCard4         c4MajVersion
            , const tCIDLib::TCard4         c4MinVersion
            , const tCIDLib::EModFlags      eFlags
        );

        TCQCModuleInfo
        (
            const   TString&                strBaseName
            , const tCIDLib::ECSSides       eSides
            , const tCIDLib::EModTypes      eModType
            , const tCIDLib::EModFlags      eFlags
        );

        TCQCModuleInfo(const TCQCModuleInfo&) = default;
        TCQCModuleInfo(TCQCModuleInfo&&) = default;

        ~TCQCModuleInfo() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCModuleInfo& operator=(const TCQCModuleInfo&) = default;
        TCQCModuleInfo& operator=(TCQCModuleInfo&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCModuleInfo&         modiSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCModuleInfo&         modiSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::ECSSides eSides() const;


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
        //  m_eSides
        //      Indicates which sides use this module, the client, server, or
        //      both.
        // -------------------------------------------------------------------
        tCIDLib::ECSSides   m_eSides;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCModuleInfo,TCIDModuleInfo)
};

#pragma CIDLIB_POPPACK


