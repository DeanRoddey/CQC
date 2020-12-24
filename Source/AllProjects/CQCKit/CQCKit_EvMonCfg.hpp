//
// FILE NAME: CQCKit_EvMonCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/08/2014
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
//  This file defines a class used to contain the configuration for event monitors.
//  This is what we store away in the config server and use to reload the monitors
//  upon system restart.
//
//  We define a simple derivative that includes the path it was loaded from, which is
//  often useful, such as by the event server, when a list of events keyed by the path
//  is needed.
//
//
// CAVEATS/GOTCHAS:
//
//  1)  As of 5.0, we moved them to the standard hierarchical storage. They used to have
//      a unique id that was the actual id, plus a human readable name, and actions
//      referred to them via the id. Now the path is the unique id. During the upgrade
//      process the old id will be read in and the events will be moved to their new
//      location under the /User section of the triggered events scope, with the id
//      as the name.
//
//      Actions that reference the ids will be updated so put that path prefix before
//      the id, making them now correctly refer to them in the new scheme. The user can
//      rename them after the ugprade if they want to give them more meaningful names.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCEvMonCfg
//  PREFIX: emonc
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCEvMonCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCEvMonCfg();

        TCQCEvMonCfg
        (
            const   TCQCEvMonCfg&           emoncSrc
        );

        ~TCQCEvMonCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TBoolean operator==
        (
            const   TCQCEvMonCfg&           emoncSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCEvMonCfg&           emoncSrc
        )   const;

        TCQCEvMonCfg& operator=
        (
            const   TCQCEvMonCfg&           emoncSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFailed() const;

        tCIDLib::TBoolean bFailed
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bPaused() const;

        tCIDLib::TBoolean bPaused
        (
            const   tCIDLib::TBoolean       bToSet
        );

        const TString& strClassPath() const;

        const TString& strClassPath
        (
            const   TString&                strToSet
        );

        const TString& strDescr() const;

        const TString& strParams() const;

        tCIDLib::TVoid Set
        (
            const   TString&                strClassPath
            , const TString&                strDescr
            , const TString&                strParams
        );


        // -------------------------------------------------------------------
        //  Only required for the 5.0 upgrade process
        // -------------------------------------------------------------------
        const TString& strId() const;


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
        //  m_bFailed
        //      This is only used at runtime by the server. Since this is the info
        //      returned to clients by the server, we want to be able to return this
        //      info as well, so it is streamed, but the actual persisted value
        //      doesn't have any meaning.
        //
        //  m_bPaused
        //      To allow monitors to be paused/resumed as needed, without having to
        //      remove them from the list of configured monitors.
        //
        //  m_strClassPath
        //      The CML class path to the class that provides the code for this
        //      monitor. It is in the hierarchical format.
        //
        //  m_strDescr
        //      A short description, for human consumption, to make it easier to
        //      remember what a given monitor is for.
        //
        //  m_strId
        //      In the pre-5.0 world, events were identified by a unique generated
        //      id. Now they are hierarchical. We just keep this around in order to
        //      do the upgrade process. See the class notes above.
        //
        //  m_strParams
        //      Parameters that can be passed to the monitor class.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bFailed;
        tCIDLib::TBoolean   m_bPaused;
        TString             m_strClassPath;
        TString             m_strDescr;
        TString             m_strId;
        TString             m_strParams;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCEvMonCfg, TObject)
};


// ---------------------------------------------------------------------------
//  CLASS: TKeyedCQCEvMonCfg
// PREFIX: csrc
//
//  A convenience for folks who need to keep a list of these events, keyed by
//  the path.
// ---------------------------------------------------------------------------
class CQCKITEXPORT TKeyedCQCEvMonCfg : public TCQCEvMonCfg
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TKeyedCQCEvMonCfg();

        TKeyedCQCEvMonCfg
        (
            const   TString&                strPath
            , const TCQCEvMonCfg&           csrcBase
        );

        TKeyedCQCEvMonCfg
        (
            const   TKeyedCQCEvMonCfg&      csrcSrc
        );

        ~TKeyedCQCEvMonCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TKeyedCQCEvMonCfg& operator=
        (
            const   TKeyedCQCEvMonCfg&      csrcSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strPath() const;

        const TString& strPath
        (
            const   TString&                strToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strPath
        //      The path from which this event was loaded. It could change if the
        //      user does a rename.
        // -------------------------------------------------------------------
        TString     m_strPath;
};

#pragma CIDLIB_POPPACK


