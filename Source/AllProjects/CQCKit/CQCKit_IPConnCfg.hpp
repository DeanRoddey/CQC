//
// FILE NAME: CQCKit_IPConnCfg.hpp
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
//  This class provides the IP specific derivative of the connection
//  configuration class. It primarily exists as a transport mechanism for the
//  flags required to set up an IP connection, and to parse out the
//  contents of an XML tree node which contains the configuration flags.
//
//  It expects the tree element it gets to be of type: CQCCfg:IPConn. And
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
//   CLASS: TCQCIPConnCfg
//  PREFIX: conncfg
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCIPConnCfg : public TCQCConnCfg
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIPConnCfg();

        TCQCIPConnCfg(const TCQCIPConnCfg&) = default;
        TCQCIPConnCfg(TCQCIPConnCfg&&) = delete;

        ~TCQCIPConnCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIPConnCfg& operator=(const TCQCIPConnCfg&) = default;
        TCQCIPConnCfg& operator=(TCQCIPConnCfg&&) = delete;

        tCIDLib::TBoolean operator==
        (
            const   TCQCIPConnCfg&          conncfgSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCIPConnCfg&          conncfgSrc
        )   const;


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
        tCIDSock::EAddrTypes eAddrType() const;

        tCIDSock::EAddrTypes eAddrType
        (
            const   tCIDSock::EAddrTypes    eToSet
        );

        tCIDSock::ESockProtos eProto() const;

        tCIDSock::ESockProtos eProto
        (
            const   tCIDSock::ESockProtos   eProto
        );

        const TIPEndPoint& ipepConn() const;

        tCIDLib::TIPPortNum ippnTarget() const;

        tCIDLib::TIPPortNum ippnTarget
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        const TString& strAddr() const;

        const TString& strAddr
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid Set
        (
            const   TString&                strAddrToSet
            , const tCIDLib::TIPPortNum     ippnToSet
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
        //  m_bResolved
        //      Remember if we have successfully resolved our end point yet,
        //      so that we can stop doing it, since it's just more overhead
        //      and the driver will repeatedly get the end point if it cannot
        //      connect. This is runtime only, and has to be mutable since it
        //      is part of faulting in the resolved address.
        //
        //  m_eAddrType
        //      The desired address type (IPV4/6) that is desired (or let the
        //      system choose.) This defaults to let the system choose but it
        //      can be overridden in the manifest and by the user (if editing
        //      of the settings is allowed.)
        //
        //  m_eProto
        //      The protocol to use with this socket connection.
        //
        //  m_ipepTarget
        //      The end point. We fault this in the first time it is accessed,
        //      and return that thereafter. The reason being that we don't want
        //      to constantly rebuild an end point and force it to
        //      do a resolution. We remember if it has resolved successfully
        //      or not, and will do it again if subsequently accessed. This
        //      is runtime only. It has to be mutable because it's faulted in
        //      within a constant getter method.
        //
        //  m_ippnTarget
        //      The port number to which we will connect.
        //
        //  m_strAddr
        //      The target address to which we will connect.
        // -------------------------------------------------------------------
        mutable tCIDLib::TBoolean   m_bResolved;
        tCIDSock::EAddrTypes        m_eAddrType;
        tCIDSock::ESockProtos       m_eProto;
        mutable TIPEndPoint         m_ipepTarget;
        tCIDLib::TIPPortNum         m_ippnTarget;
        TString                     m_strAddr;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        DefPolyDup(TCQCIPConnCfg)
        RTTIDefs(TCQCIPConnCfg,TCQCConnCfg)
};

#pragma CIDLIB_POPPACK

