//
// FILE NAME: CQCRPortSrv_SerialPortInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2005
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
//  This is the header for the CQCRPortSrv_SerialPortInfo.cpp file, which implements
//  a class that is used to track of ports that we serve up. We scan all the local ports
//  and create one of these for each port. We only create a local serial port object
//  when a client has opened a port.
//
//  We only expose those that are enabled by the user in the allowed ports list.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//   CLASS: TCQCSerPortInfo
//  PREFIX: spi
// ---------------------------------------------------------------------------
class TCQCSerPortInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSerPortInfo
        (
            const   TString&                strLocalPath
        );

        ~TCQCSerPortInfo();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAllow() const;

        tCIDLib::TBoolean bAllow
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4Cookie() const;

        tCIDLib::TCard4 c4Cookie
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TEncodedTime enctLast() const;

        tCIDLib::TEncodedTime enctLast
        (
            const   tCIDLib::TEncodedTime   enctToSet
        );

        TCommPortBase* pcommPort() const;

        TCommPortBase* pcommPort();

        const TString& strLocalPath() const;

        const TString& strRemotePath() const;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented ctors and operators
        // -------------------------------------------------------------------
        TCQCSerPortInfo(const TCQCSerPortInfo&);
        tCIDLib::TVoid operator=(const TCQCSerPortInfo&);


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAllow
        //      The user selects which available local ports he wants to make available
        //      to remote clients. Any not marked allow are not reported as existing.
        //
        //  m_c4Cookie
        //      The cookie we give back when this port is opened. We generate a unique
        //      id for each one.
        //
        //  m_enctLast
        //      We'll drop the port if it's not accessed within 10 minutes. The user can
        //      force a port down if required.
        //
        //  m_pcommPort
        //      The local port that we are providing as a remote port. We create the port
        //      object upon construction, but open/close it based on client requests.
        //
        //  m_strLocalPath
        //      The local path of this port.
        //
        //  m_strRemotePath
        //      The path that we show this port to the client side as.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bAllow;
        tCIDLib::TCard4         m_c4Cookie;
        tCIDLib::TEncodedTime   m_enctLast;
        TCommPortBase*          m_pcommPort;
        TString                 m_strLocalPath;
        TString                 m_strRemotePath;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSerPortInfo,TObject)
};


