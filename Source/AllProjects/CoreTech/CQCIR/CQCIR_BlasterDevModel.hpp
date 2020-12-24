//
// FILE NAME: CQCIR_BlasterDevModel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2002
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
//  This class represents an IR device model, and holds a set of commands
//  that are relevant to that device. At this level we need the name of the
//  device (which is the key field), a description of the device for display
//  purposes, and a keyed collection of TIRBlasterCmd objects.
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
//   CLASS: TIRBlasterDevModel
//  PREFIX: irbdm
// ---------------------------------------------------------------------------
class CQCIREXPORT TIRBlasterDevModel : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Class specific types
        // -------------------------------------------------------------------
        using TCmdMap    = TKeyedHashSet<TIRBlasterCmd, TString, TStringKeyOps>;
        using TCmdCursor = TCmdMap::TCursor;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TIRBlasterDevModel&     irbdmSrc
        );

        static tCIDLib::TVoid StreamHeader
        (
                    TBinInStream&           strmSrc
            ,       tCIDLib::TCard4&        c4RepeatCount
            ,       TString&                strDevModel
            ,       TString&                strDevMake
            ,       TString&                strDescr
            ,       tCQCKit::EDevCats&      eCategory
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIRBlasterDevModel();

        TIRBlasterDevModel
        (
            const   TString&                strModel
            , const tCQCKit::EDevCats       eCategory
        );

        TIRBlasterDevModel
        (
            const   TString&                strModel
            , const TString&                strMake
            , const TString&                strDescr
            , const tCQCKit::EDevCats       eCategory
        );

        TIRBlasterDevModel(const TIRBlasterDevModel&) = default;
        TIRBlasterDevModel(TIRBlasterDevModel&&) = default;

        ~TIRBlasterDevModel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIRBlasterDevModel& operator=(const TIRBlasterDevModel&) = default;
        TIRBlasterDevModel& operator=(TIRBlasterDevModel&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCmdExists
        (
            const   TString&                strCmdToFind
        )   const;

        tCIDLib::TCard4 c4RepeatCount() const;

        tCIDLib::TCard4 c4RepeatCount
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        TCmdCursor cursCmds() const;

        tCQCKit::EDevCats eCategory() const;

        tCQCKit::EDevCats eCategory
        (
            const   tCQCKit::EDevCats       eToSet
        );

        TIRBlasterCmd& irbcAdd
        (
            const   TString&                strCmdName
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4DataBytes
        );

        TIRBlasterCmd& irbcAddOrUpdate
        (
            const   TIRBlasterCmd&          irbcToAdd
            ,       tCIDLib::TBoolean&      bAdded
        );

        const TIRBlasterCmd& irbcFromName
        (
            const   TString&                strCmdName
        )   const;

        TIRBlasterCmd& irbcFromName
        (
            const   TString&                strCmdName
        );

        tCIDLib::TVoid LoadFromFile
        (
            const   TString&                strSrcFile
            , const TString&                strExpType
            , const tCIDLib::TBoolean       bAllowDups
        );

        tCIDLib::TVoid RemoveCmd
        (
            const   TString&                strCmdName
        );

        const TString& strDescription() const;

        const TString& strDescription
        (
            const   TString&                strNew
        );

        const TString& strMake() const;

        const TString& strMake
        (
            const   TString&                strNew
        );

        const TString& strModel() const;

        const TString& strModel
        (
            const   TString&                strNew
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
        //  Private, non-virtual mehods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Parse
        (
            const   TXMLTreeElement&        xtnodeRoot
            , const tCIDLib::TBoolean       bAllowDups
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4RepeatCount
        //      The per-device model repeat count value. This is used by some
        //      blasters to indicate how many times to repeat the command,
        //      to make sure the device sees it.
        //
        //  m_colCmdMap
        //      A collection of rat commands, keyed on the command name, of
        //      commands that are relevant to this device.
        //
        //  m_eCategory
        //      The device category of this device. This is just to allow users
        //      to more easily locate particular types of devices.
        //
        //  m_strDescr
        //      The descriptive name of the command, which should be short and
        //      to the point, usually 10 to 15 words.
        //
        //  m_strMake
        //      The make indentification of the device. This will be used for
        //      sorting of the list of devices for selection by human.
        //
        //  m_strModel
        //      The model of the device. This is the key field, and it is case
        //      sensitive. All model names must be unique.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4RepeatCount;
        TCmdMap             m_colCmdMap;
        tCQCKit::EDevCats   m_eCategory;
        TString             m_strDescr;
        TString             m_strMake;
        TString             m_strModel;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TIRBlasterDevModel,TObject)
};

#pragma CIDLIB_POPPACK


