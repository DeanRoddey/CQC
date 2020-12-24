//
// FILE NAME: CQCKit_SerialConnCfg.hpp
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
//  This class provides the RS-232 specific derivative of the connection
//  configuration class. It primarily exists as a transport mechanism for the
//  flags required to set up a serial connection, and to parse out the
//  contents of an XML tree node which contains the configuratoin flags.
//
//  It expects the tree element it gets to be of type: CQCCfg:SerialConn. And
//  it assumes that this tree has be validated with the configuration DTD
//  before being passed in.
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
//   CLASS: TCQCSerialConnCfg
//  PREFIX: conncfg
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCSerialConnCfg : public TCQCConnCfg
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSerialConnCfg();

        TCQCSerialConnCfg
        (
            const   TCommPortCfg&           cpcfgToUse
            , const TString&                strPortToUse
            , const tCIDComm::EPortTypes    eType
        );

        TCQCSerialConnCfg(const TCQCSerialConnCfg&) = default;

        ~TCQCSerialConnCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSerialConnCfg& operator=(const TCQCSerialConnCfg&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCSerialConnCfg&      conncfgSrc
        )   const;

        tCIDLib::TBoolean operator!=(const TCQCSerialConnCfg& conncfgSrc) const
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


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCommPortCfg& cpcfgSerial() const;

        const TCommPortCfg& cpcfgSerial
        (
            const   TCommPortCfg&           cpcfgToSet
        );

        const TString& strPortPath() const;

        const TString& strPortPath
        (
            const   TString&                strToSet
        );

        tCIDComm::EPortTypes ePortType() const;


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
        //  Private data members
        //
        //  m_cpcfgSerial
        //      If this device has a serial port connection, this is the
        //      configuration for that device. It is set up from the passed in
        //      XML node.
        //
        //  m_ePortType
        //      The type of port, i.e. 232 or 422.
        //
        //  m_strPortPath
        //      The path that represents the port the driver should use.
        // -------------------------------------------------------------------
        TCommPortCfg            m_cpcfgSerial;
        tCIDComm::EPortTypes    m_ePortType;
        TString                 m_strPortPath;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        DefPolyDup(TCQCSerialConnCfg)
        RTTIDefs(TCQCSerialConnCfg,TCQCConnCfg)
};

#pragma CIDLIB_POPPACK

