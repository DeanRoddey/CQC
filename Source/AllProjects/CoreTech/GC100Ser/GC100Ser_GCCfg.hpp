//
// FILE NAME: GC100Ser_GCCfg.hpp
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
//  This is the header for the GC100Ser_GCCfg.cpp file, which defines the
//  info that the containing app must provide us for each GC that we are
//  to allow access to.
//
//  The app provides a list of these to the factory object it registers with
//  the CIDComm facility. The factory uses these configured objects to tell
//  the app what ports are available and to gen up new ports.
//
//  For each GC we need a name (used in the port names for that GC to make
//  them identifiable), and an address for the GC we talk to, and which of the
//  ports on that GC are to be made available. Ports are implict, being
//  assigned sequentially from 4999.
//
//  We allow for more port flags than currently any GC supports, to allow
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
//  CLASS: TGC100Cfg
// PREFIX: gccfg
// ---------------------------------------------------------------------------
class GC100SEREXPORT TGC100Cfg : public TObject, public MStreamable
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompByName
        (
            const   TGC100Cfg&              gccfg1
            , const TGC100Cfg&              gccfg2
        );

        static const TString& strName
        (
            const   TGC100Cfg&              gccfgSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGC100Cfg();

        TGC100Cfg
        (
            const   TString&                strName
            , const TString&                strAddr
            , const tCIDLib::TCard4         c4Ports
        );

        TGC100Cfg(const TGC100Cfg&) = default;
        TGC100Cfg(TGC100Cfg&&) = default;

        ~TGC100Cfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGC100Cfg& operator=(const TGC100Cfg&) = default;
        TGC100Cfg& operator=(TGC100Cfg&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TGC100Cfg&              gccfgSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TGC100Cfg&              gccfgSrc
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
        //      A bit map of ports to enable for this GC. 0th bit is port 1,
        //      ect...
        //
        //  m_strAddr
        //      The target address of the GC-100.
        //
        //  m_strName
        //      A short name that will be part of the port paths of this GC.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4Ports;
        TString             m_strAddr;
        TString             m_strName;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGC100Cfg,TObject)
};


// ---------------------------------------------------------------------------
//  CLASS: TGC100CfgList
// PREFIX: gccl
// ---------------------------------------------------------------------------
class GC100SEREXPORT TGC100CfgList : public TObject, public MStreamable
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGC100CfgList();

        TGC100CfgList(const TGC100CfgList&) = default;
        TGC100CfgList(TGC100CfgList&&) = default;

        ~TGC100CfgList();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGC100CfgList& operator=(const TGC100CfgList&) = default;
        TGC100CfgList& operator=(TGC100CfgList&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TGC100CfgList&          gcclSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TGC100CfgList&          gcclSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4AddNew
        (
            const   TGC100Cfg&              gccfgNew
        );

        tCIDLib::TCard4 c4Count() const;

        const TGC100Cfg& gccfgAt
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
            , const TGC100Cfg&              gccfgToSet
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
        TVector<TGC100Cfg>  m_colCfgList;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGC100CfgList,TObject)
};

#pragma CIDLIB_POPPACK


