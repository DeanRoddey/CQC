//
// FILE NAME: JAPwrSer_PortCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/12/2013
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
//  This is the header for the JAPwrSer_PortCfg.cpp file, which defines the
//  info that the containing app must provide us for each JAP device that we are
//  to allow access to.
//
//  The app provides a list of these to the factory object it registers with
//  the CIDComm facility. The factory uses these configured objects to tell
//  the app what ports are available and to gen up new ports.
//
//  For each device we need a name (used in the port name for that one to make
//  them identifiable), and an address for the device we talk to. The port is
//  fixed.
//
//  We allow for more port flags than currently any JSP supports, to allow
//  for future expansion. We just use a bitmapped Card4.
//
//  For the overall configuration we create a simple class that wraps a
//  vector of config objects, so that it can be streamable as a whole.
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
//  CLASS: TJAPwrCfg
// PREFIX: japcfg
// ---------------------------------------------------------------------------
class JAPWRSEREXPORT TJAPwrCfg : public TObject, public MStreamable
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompByName
        (
            const   TJAPwrCfg&              japcfg1
            , const TJAPwrCfg&              japcfg2
        );

        static const TString& strName
        (
            const   TJAPwrCfg&              japcfgSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TJAPwrCfg();

        TJAPwrCfg
        (
            const   TString&                strName
            , const TString&                strAddr
            , const tCIDLib::TCard4         c4Ports
        );

        TJAPwrCfg(const TJAPwrCfg&) = default;
        TJAPwrCfg(TJAPwrCfg&&) = default;

        ~TJAPwrCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TJAPwrCfg& operator=(const TJAPwrCfg&) = default;
        TJAPwrCfg& operator=(TJAPwrCfg&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TJAPwrCfg&              japcfgSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TJAPwrCfg&              japcfgSrc
        )   const;



        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4Ports() const;

        const TString& strAddr() const;

        const TString& strName() const;

        tCIDLib::TVoid Set
        (
            const   TString&                strName
            , const TString&                strAddr
            , const tCIDLib::TCard4         c4Ports
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
        //  m_c4Ports
        //      A bit map of ports to enable for this device. 0th bit is port 1,
        //      ect...
        //
        //  m_strAddr
        //      The target address of the device.
        //
        //  m_strName
        //      A short name that will be part of the port paths of this device.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4Ports;
        TString             m_strAddr;
        TString             m_strName;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TJAPwrCfg,TObject)
};


// ---------------------------------------------------------------------------
//  CLASS: TJAPwrCfgList
// PREFIX: japl
// ---------------------------------------------------------------------------
class JAPWRSEREXPORT TJAPwrCfgList : public TObject, public MStreamable
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TJAPwrCfgList();

        TJAPwrCfgList(const TJAPwrCfgList&) = default;
        TJAPwrCfgList(TJAPwrCfgList&&) = default;

        ~TJAPwrCfgList();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TJAPwrCfgList& operator=(const TJAPwrCfgList&) = default;
        TJAPwrCfgList& operator=(TJAPwrCfgList&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TJAPwrCfgList&          japlSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TJAPwrCfgList&          japlSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4AddNew
        (
            const   TJAPwrCfg&              japcfgNew
        );

        tCIDLib::TCard4 c4Count() const;

        const TJAPwrCfg& japcfgAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid RemoveAll();

        tCIDLib::TVoid RemoveAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid SetAt
        (
            const   tCIDLib::TCard4         c4At
            , const TJAPwrCfg&              japcfgToSet
        );

        tCIDLib::TVoid SortList();


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
        // -------------------------------------------------------------------
        TVector<TJAPwrCfg>  m_colCfgList;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TJAPwrCfgList,TObject)
};

#pragma CIDLIB_POPPACK


