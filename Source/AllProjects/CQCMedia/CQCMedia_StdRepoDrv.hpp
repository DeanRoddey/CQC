//
// FILE NAME: CQCMedia_StdRepoDrv.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/14/2006
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
//  This is the header for the CQCMedia_StdRepoDrv.cpp file. This file implements
//  a base class that many repository drivers can derive from. Since most of
//  the repository drivers use the standard in-memory database that is defined
//  here in this facility, that means that much of the code related to getting
//  data out of that database on request from clients wil be the same, and we
//  can provide it generically. Putting data into the database, from whatever
//  source the repository gets it from, we cannot do anything about. But, once
//  in the database, it's in a standard format. So the derived class has to
//  do the database loading.
//
//  In actual fact the code for managing client queries for media data is
//  separated out into a separate class, and we work in terms of that. This
//  is so non-drives can make use of that functionality, something sometimes
//  required. So this class is really just handling the backdoor driver
//  methods that clients use to query data, and passing them on to a repo
//  database engine provided by the derived driver class.
//
//  We could almost just use a single standard database engine and declare it
//  here in this class instead of having drivers provide it. But, we have to
//  deal with the possibility of some of the data having to be gotten after
//  the loading is done, such as cover art which is faulted in in many
//  cases, and might come from different sources. So the derived class can
//  create a derivative of the engine and give us one of those.
//
//  There isn't a lot of code in this class, but it's enough that we do not
//  want every media repo driver duplicating it. If they want to provide other
//  backdoor commands they can override those methods and only pass on to us
//  those that they don't handle themselves.
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
//  CLASS: TCQCStdMediaRepoDrv
// PREFIX: sdrv
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCStdMediaRepoDrv : public TCQCServerBase
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TCQCStdMediaRepoDrv() = delete;

        TCQCStdMediaRepoDrv(const TCQCStdMediaRepoDrv&) = delete;
        TCQCStdMediaRepoDrv(TCQCStdMediaRepoDrv&&) = delete;

        ~TCQCStdMediaRepoDrv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCStdMediaRepoDrv& operator=(const TCQCStdMediaRepoDrv&) = delete;
        TCQCStdMediaRepoDrv& operator=(TCQCStdMediaRepoDrv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryData
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TBoolean bQueryData2
        (
            const   TString&                strQueryType
            ,       tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
        );

        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        );

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        );

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4Bytes
            ,       THeapBuf&               mbufToSend
        );

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmd
            , const TString&                strParms
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQCStdMediaRepoDrv
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
            ,       TCQCStdMediaRepoEng* const psrdbToSet
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strDBSerialNum() const;

        tCIDLib::TVoid SetDBSerialNum
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid ThrowLoading();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strDBSerialNum
        //      This is the V2 repo database serial number. It changes any
        //      time the database changes.
        //
        //  m_psrdbEng
        //      We can use a standard repo database engine object for all of
        //      the stuff related to serving up data to clients. The derived
        //      class owns the object and is responsible for loading up the
        //      contents. We provide the support for serving up the data to
        //      clients via a standard interface (based on backdoor driver
        //      calls.)
        //
        //      The derived class provides us this pointer, and owns the
        //      engine.
        //
        //      If the derived class provides a direct interface into the
        //      repo driver, for management purposes, it's repo engine must
        //      sync access to the data. It should override the callbacks
        //      above, lock, then pass the call through.
        // -------------------------------------------------------------------
        TString                 m_strDBSerialNum;
        TCQCStdMediaRepoEng*    m_psrdbEng;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCStdMediaRepoDrv,TCQCServerBase)
};

#pragma CIDLIB_POPPACK



