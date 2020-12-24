//
// FILE NAME: CQCDataSrv_IRServerImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/18/2002
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
//  This is the header for the CQCDataSrv_IRServerImpl.cpp file, which
//  provides an implementation of the data server's IR repository server
//  interface. Each file we manage represents the information for a
//  particular model of IR controlled device, so we expose the repository
//  in terms of 'device models'.
//
//  Since we could have a large number of files over time, we maintain an
//  in-memory key, which consists of the header information, represented by
//  the TIRDevModelInfo class. This allows us to provide info for the
//  c4QueryDevModelList() method quickly, because its a just a dump of the
//  in-memory key. They key fields indicate the model name (the unique key),
//  the make, a description, a device category, and the name of the file,
//  which is a unique generated file name.
//
//  Since this information is embedded in the file, we can rebuild the key
//  on each load, and don't have to maintain a separate key file and all of
//  the possibilities for disaster that that implies. Since some folks won't
//  use the IR system we don't load the key info until someone actually
//  accesses the repository. It can be done fairly quickly, since it just
//  requries that we read a little header info from each file.
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
//   CLASS: TCQCIRSrvImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TCQCIRSrvImpl : public TIRRepoServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        //
        //  TModInfoList -
        //      A typedef for the list of device model info objects that we
        //      return to clients in a couple of server side calls.
        //
        //  TBlasterDevList -
        //      The list of models currently installed for one particular
        //      blaster type. Needs to be a keyed hash set, keyed on the model
        //      name, for fast access.
        //
        //  TKeyList -
        //      The list of currently loaded blaster device lists. I.e. for
        //      each blast device that's accessed to load one of it's device
        //      modes, we add an entry here, keyed by the device type name.
        // -------------------------------------------------------------------
        using TModInfoList = TVector<TIRBlasterDevModelInfo>;
        using TBlasterDevList = TKeyedHashSet<TIRBlasterDevModelInfo, TString, TStringKeyOps>;
        using TKeyList = THashMap<TBlasterDevList, TString, TStringKeyOps>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIRSrvImpl();

        TCQCIRSrvImpl(const TCQCIRSrvImpl&) = delete;
        TCQCIRSrvImpl(TCQCIRSrvImpl&&) = delete;

        ~TCQCIRSrvImpl();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIRSrvImpl& operator=(const TCQCIRSrvImpl&) = delete;
        TCQCIRSrvImpl& operator=(TCQCIRSrvImpl&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bModelExists
        (
            const   TString&                strDevModel
            , const TString&                strBlasterName
        )   override;

        tCIDLib::TCard4 c4QueryDevCatList
        (
            const   tCQCKit::EDevCats       eCategory
            , const TString&                strBlasterName
            ,       TModInfoList&           colToFill
        )   override;

        tCIDLib::TCard4 c4QueryDevModelList
        (
            const   TString&                strBlasterName
            ,       TModInfoList&           colToFill
        )   override;

        tCIDLib::TVoid DeleteDevModel
        (
            const   TString&                strDevModel
            , const TString&                strBlasterName
        )   override;

        tCIDLib::TVoid QueryDevModel
        (
            const   TString&                strDevModel
            , const TString&                strBlasterName
            ,       TIRBlasterDevModel&     irbdmToFill
        )   override;

        tCIDLib::TVoid WriteDevModel
        (
            const   TString&                strDevModel
            , const TString&                strBlasterName
            , const TIRBlasterDevModel&     irbdmToWrite
        )   override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadKey
        (
                    TBlasterDevList&        colToFill
            , const TString&                strBlasterName
        );

        [[nodiscard]] TBlasterDevList* pcolFindKey
        (
            const   TString&                strBlasterName
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colKey
        //      This is our key collection. This allows us to get quickly
        //      from a device model name to the file that holds the data.
        //      There is one element per possible blaster type and each
        //      element is faulted in on first use. It's not thread safe
        //      since we have a separate mutex for sync.
        //
        //  m_mtxSync
        //      This is used to synchronize access to the repository.
        //
        //  m_strRepoDir
        //      The directory where our files are. We set this up during the
        //      ctor.
        // -------------------------------------------------------------------
        TKeyList    m_colKey;
        TMutex      m_mtxSync;
        TString     m_strRepoDir;


        // -------------------------------------------------------------------
        //  Magic Intfs
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIRSrvImpl,TIRRepoServerBase)
};

#pragma CIDLIB_POPPACK


