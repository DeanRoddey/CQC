//
// FILE NAME: CQCKit_OtherConnCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2001
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
//  This class provides a generic connection configuration. For those devices
//  that need no connection (because it runs in the same host that is running
//  the CQCServer process that is controlling it), or which have some sort of
//  configuration that isn't yet directly supported, this class provides a
//  fallback.
//
//  It expects the tree element it gets to be of type: CQCCfg:OtherConn. And
//  it assumes that this tree has be validated with the configuration DTD
//  before being passed in.
//
//  This guy stores no data, and it assumed that any info needed will be
//  gotten through prompted values in the manifest file. So basically it's
//  just a place holder, because we ahve to have some kind of config.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TXMLTreeElement;

// ---------------------------------------------------------------------------
//   CLASS: TCQCOtherConnCfg
//  PREFIX: conncfg
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCOtherConnCfg : public TCQCConnCfg
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCOtherConnCfg();

        TCQCOtherConnCfg
        (
            const   TString&                strCfgInfo
        );

        TCQCOtherConnCfg(const TCQCOtherConnCfg&) = default;
        TCQCOtherConnCfg(TCQCOtherConnCfg&&) = default;

        ~TCQCOtherConnCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCOtherConnCfg& operator=(const TCQCOtherConnCfg&) = default;
        TCQCOtherConnCfg& operator=(TCQCOtherConnCfg&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCOtherConnCfg&       conncfgSrc
        )   const;

        tCIDLib::TBoolean operator!=(const TCQCOtherConnCfg& conncfgSrc)const
        {
            return !operator==(conncfgSrc);
        }


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCConnCfg&            conncfgSrc
        )   const override;

        tCIDLib::TBoolean bParseConnInfo
        (
            const   TString&                strInfo
        )   override;

        tCIDLib::TVoid LoadFromXML
        (
            const   TXMLTreeElement&        xtnodeSrc
        )   override;

        TString strConnInfo() const override;


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
        //  Magic Macros
        // -------------------------------------------------------------------
        DefPolyDup(TCQCOtherConnCfg)
        RTTIDefs(TCQCOtherConnCfg,TCQCConnCfg)
};

#pragma CIDLIB_POPPACK


