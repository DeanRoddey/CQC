//
// FILE NAME: CQCKit_ConnCfg.hpp
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
//  Devices can be connected to (by the CQCServer instance that controls them)
//  in any number of possible ways. This class is the base from which all of
//  the possible configuration schemes are derived. The obvious ones are
//  RS-232 style serial connections, IP based connections, and in the future
//  probably USB and Firewire connections. There are derivatives provided,
//  in other files, for these types of connections.
//
// CAVEATS/GOTCHAS:
//
//  1)  These connection configuration objects are used as part of the driver
//      configuration class, TCQCDriverCfg. These objects must be streamable
//      so this class implements MStreamable and MDuplicable so that its
//      instances can be polymorphically streamed and duplicated as required.
//      Derived classes must provide the concrete implementations for these
//      mixin interfaces, and must support advanced CIDLib RTTI (which is
//      required for polymorphic streaming)
//
//  2)  As always for polymorphic streaming, the derived classes must provide
//      a default constructor. If they don't want to make this public, then
//      they should use the BefriendFactory() macro which allows the default
//      ctor to be private.
//
//  3)  We define a virtual here, ParseConnInfo(), which is a convenience
//      for test harnesses which need to get conn info generically as a string
//      in many cases. The derived class will try to parse it in a standard
//      format for its type of connection.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TXMLTreeElement;

// ---------------------------------------------------------------------------
//   CLASS: TCQCConnCfg
//  PREFIX: conncfg
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCConnCfg : public TObject, public MDuplicable, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        ~TCQCConnCfg();


        // -------------------------------------------------------------------
        //  Public, pure virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bIsSame
        (
            const   TCQCConnCfg&            conncfgSrc
        )   const = 0;

        virtual tCIDLib::TBoolean bParseConnInfo
        (
            const   TString&                strInfo
        ) = 0;

        virtual tCIDLib::TVoid LoadFromXML
        (
            const   TXMLTreeElement&        xtnodeSrc
        ) = 0;

        virtual TString strConnInfo() const = 0;


    protected :
        // -------------------------------------------------------------------
        //  Hidden ctors and operators
        // -------------------------------------------------------------------
        TCQCConnCfg();

        TCQCConnCfg(const TCQCConnCfg&) = default;
        TCQCConnCfg(TCQCConnCfg&&) = default;
        TCQCConnCfg& operator=(const TCQCConnCfg&) = default;
        TCQCConnCfg& operator=(TCQCConnCfg&&) = default;


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
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCConnCfg,TObject)
};

#pragma CIDLIB_POPPACK

