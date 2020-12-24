//
// FILE NAME: CQCKit_DriverObjCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/02/2001
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
//  This is the header for the CQCKit_DriverObjCfg.cpp file, which implements
//  the TCQCDriverObjCfg class. This class derives from the TCQCDriverCfg
//  class. The parent class contains the info parsed from the manifest file,
//  and so doesn't have any 'live configuration'. It just provides the manifest
//  driven info, which among other things defines what configurable options are
//  available. This class is used to hold the user configured stuff, the moniker,
//  comm configuration, driver prompts, etc...
//
//  As of 1.4.25 and beyond, this class also holds the 'event trigger' info
//  for the driver. This is information that indicates which, if any, fields
//  of the driver should generate an event, and the criteria for generating
//  it, i.e. field value changed, field equals some value, field is < or >
//  some value, etc... The field name is the key that links a trigger event
//  to a field. The driver doesn't have to deal with these. CQCServer is
//  responsible for invoking triggers when fields change, actually the base
//  driver class that all drivers are derived from.
//
//  The trigger config class is in the field storage class file, since it's
//  also used there, because the event for each field is put into the storage
//  object along with the field value, since that's where triggering is done
//  with the stored value changes. So we hold the static configuration of
//  events here, but at runtime they are loaded into their associated storage
//  objects.
//
//  As of 4.5.9 we also store a 'paused' flag here. Drivers can be paused, to
//  temporarily turn them off. Previously that was a runtime thing only, but now
//  we want to allow it to persist. That means we have to remember paused drivers
//  across invocations of CQCServer. So it's stored here. CQCServer checks this
//  flag when it reloads drivers and just puts any paused ones directly into
//  paused state without trying to initialize them.
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
//   CLASS: TCQCDriverObjCfg
//  PREFIX: cqcdc
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCDriverObjCfg : public TCQCDriverCfg
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TTriggerList = TVector<TCQCFldEvTrigger>;


        // -------------------------------------------------------------------
        //  Static comparators and key extraction methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eComp
        (
            const   TCQCDriverObjCfg&       cqcdc1
            , const TCQCDriverObjCfg&       cqcdc2
        );

        static tCIDLib::ESortComps eIComp
        (
            const   TCQCDriverObjCfg&       cqcdc1
            , const TCQCDriverObjCfg&       cqcdc2
        );

        static const TString& strKey
        (
            const   TCQCDriverObjCfg&       cqcdcSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDriverObjCfg();

        TCQCDriverObjCfg
        (
            const   TCQCDriverCfg&          cqcdcSrc
        );

        TCQCDriverObjCfg
        (
            const   TCQCDriverCfg&          cqcdcSrc
            , const TString&                strMoniker
        );

        TCQCDriverObjCfg
        (
            const   TCQCDriverObjCfg&       cqcdcSrc
        );

        TCQCDriverObjCfg(TCQCDriverObjCfg&&) = delete;

        ~TCQCDriverObjCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDriverObjCfg& operator=
        (
            const   TCQCDriverObjCfg&       cqcdcSrc
        );

        TCQCDriverObjCfg& operator=(TCQCDriverObjCfg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid NewConnCfg
        (
                    TCQCConnCfg* const      pconncfgNew
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFindPromptValue
        (
            const   TString&                strKey
            , const TString&                strSubKey
            ,       TString&                strValue
        )   const;

        tCIDLib::TBoolean bPaused() const;

        tCIDLib::TBoolean bPaused
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bSameBase
        (
            const   TCQCDriverCfg&          cqcdcToCheck
        )   const;

        const TTriggerList& colTriggers() const;

        const TCQCConnCfg& conncfgReal() const;

        TCQCConnCfg& conncfgReal();

        TPromptCursor cursPromptVals() const;

        TValCursor cursPromptSubVals
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TVoid FaultInPromptVals
        (
            const   tCIDLib::TBoolean       bForce = kCIDLib::False
        )   const;

        const TPromptVals& nvmPromptVals() const;

        const TCQCFldEvTrigger* pfetFind
        (
            const   TString&                strFldName
        )   const;

        tCIDLib::TVoid RemoveAllTriggers();

        tCIDLib::TVoid RemoveTrigger
        (
            const   TString&                strFldName
        );

        const TString& strMoniker() const;

        const TString& strMoniker
        (
            const   TString&                strToSet
        );

        const TString& strPromptValue
        (
            const   TString&                strKey
            , const TString&                strSubKey
        )   const;

        tCIDLib::TVoid SetPromptVal
        (
            const   TString&                strKey
            , const TString&                strSubKey
            , const TString&                strNewValue
        );

        tCIDLib::TVoid SetTrigger
        (
            const   TCQCFldEvTrigger&       fetToAdd
        );

        tCIDLib::TVoid SetTriggers
        (
            const   TTriggerList&           colToSet
        );

        tCIDLib::TVoid UpdateBase
        (
            const   TCQCDriverCfg&          cqcdcToSet
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
        //  m_bPaused
        //      Used to persistently if a driver is paused. See file level comments above.
        //
        //  m_colTriggers
        //      Any event triggers that are defined for this driver instance. They are keyed
        //      by field name.
        //
        //  m_nvmPromptVals
        //      The optional list of prompted values gotten from the user on driver install,
        //      as driven by the manifest file. It is mutable because we fault them in,
        //      driven by the prompt cfg data in the base class, upon use. So it will always
        //      look to the outside that it has as many items as are in the underlying prompt
        //      cfg list.
        //
        //  m_pconncfgReal
        //      The base class has the default connection config that was read from the
        //      manifest file. This is the actual per-instance config that can be edited (if
        //      allowed by the manifest) and used to configure the driver. It will be
        //      initialized to the defaults on first use, then can be modified if needed.
        //
        //  m_strMoniker
        //      This is the human readable identifier for the driver. It is also the
        //      persistent identifier. This is also the key under which the object id
        //      binding is done in the name server.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bPaused;
        TTriggerList        m_colTriggers;
        mutable TPromptVals m_nvmPromptVals;
        TCQCConnCfg*        m_pconncfgReal;
        TString             m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDriverObjCfg,TCQCDriverCfg)
};

#pragma CIDLIB_POPPACK


