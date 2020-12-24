//
// FILE NAME: CQCKit_UPnPConnCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/19/2011
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
//  This class provides a connection configuration for UPnP devices. It gets
//  a device type URN, and that is used to let the user select a specific
//  instance of that device via a UPnP search.
//
//  It expects the tree element it gets to be of type: CQCCfg:UPnPConn. And
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

class TXMLTreeElement;

// ---------------------------------------------------------------------------
//   CLASS: TCQCUPnPConnCfg
//  PREFIX: conncfg
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCUPnPConnCfg : public TCQCConnCfg
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCUPnPConnCfg();

        TCQCUPnPConnCfg
        (
            const   TString&                strCfgInfo
        );

        TCQCUPnPConnCfg(const TCQCUPnPConnCfg&) = default;
        TCQCUPnPConnCfg(TCQCUPnPConnCfg&&) = default;

        ~TCQCUPnPConnCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCUPnPConnCfg& operator=(const TCQCUPnPConnCfg&) = default;
        TCQCUPnPConnCfg& operator=(TCQCUPnPConnCfg&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCUPnPConnCfg&        conncfgSrc
        )   const;

        tCIDLib::TBoolean operator!=(const TCQCUPnPConnCfg& conncfgSrc) const
        {
            return !operator==(conncfgSrc);
        }


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCConnCfg&            conncfgSrc
        )   const final;

        tCIDLib::TBoolean bParseConnInfo
        (
            const   TString&                strInfo
        )   final;

        tCIDLib::TVoid LoadFromXML
        (
            const   TXMLTreeElement&        xtnodeSrc
        )   final;

        TString strConnInfo() const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strDevName() const;

        const TString& strDevName
        (
            const   TString&                strToSet
        );

        const TString& strDevType() const;

        const TString& strDevUID() const;

        const TString& strDevUID
        (
            const   TString&                strToSet
        );

        const TString& strDispSvcType() const;

        const TString& strDispVarName() const;

        const TString& strTypeDescr() const;


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
        //  m_strDevName
        //      For display purposes to the user we also remember the device's
        //      friendly name, so we can display it again when they reconfigure
        //      the driver.
        //
        //  m_strDevType
        //      The device type that was indicated in the connection info. This
        //      is not updated by the user, just stored and used for the search.
        //
        //  m_strDevUID
        //      The unique device ID we get from the user.
        //
        //  m_strDispSvcType
        //  m_strDispVarName
        //      They can optionally indicate that we should get the display
        //      name shown in the driver install panel from a contained service's
        //      state variable instead of using the friendly name of the
        //      actual device. This is optional and these may be blank.
        //
        //  m_strTypeDescr
        //      A short description provided in the manifest. Not for the user
        //      to modify, just for display purposes in the driver installer
        //      panel to indicate the type of device.
        // -------------------------------------------------------------------
        TString m_strDevName;
        TString m_strDevType;
        TString m_strDevUID;
        TString m_strDispSvcType;
        TString m_strDispVarName;
        TString m_strTypeDescr;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        DefPolyDup(TCQCUPnPConnCfg)
        RTTIDefs(TCQCUPnPConnCfg,TCQCConnCfg)
};

#pragma CIDLIB_POPPACK

