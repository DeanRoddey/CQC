//
// FILE NAME: ZWaveLevi2Sh_Option.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/28/2014
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
//  This is the header file for the ZWaveLevi2Sh_Option.cpp file, which implements
//  the TZWOption class. Each command class can ask that the user provide some number
//  of configuration options. So the unit will give them a chance to create the
//  options they want.
//
//  The option itself and each potential value has a 'private key' that the command
//  class sets. The results of selections are stored in the unit class as key/value
//  pairs. The ke is the private key of the option and the value is the private key
//  of the selected value.
//
//  This means that new selectable values can be added to an option over time without
//  worrying about their order. And we don't have to worry about indices or any of
//  that. The selection list in the unit is keyed on the optoin private key for fast
//  lookup.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWOption
//  PREFIX: zwopt
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWOption : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWOption();

        ~TZWOption();


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_colValues
        //      The option values. The key is the unique value for each option value
        //      and the value is the display text for the user. This display text can
        //      change, since only the key is used internally. They keys only have to be
        //      unique within this one option.
        //
        //  m_strInstDescr
        //      If this is an option for a multi-instance class, then this is the
        //      instance name, so that it can be part of the displayed info.
        //
        //  m_strKey
        //      The key for this option. The command class should make sure it's
        //      unique by including some prefix related to its command class name.
        //
        //  m_strPrompt
        //      The text we display tot he user for this option.
        // -------------------------------------------------------------------
        tCIDLib::TKVPList   m_colValues;
        TString             m_strInstDescr;
        TString             m_strKey;
        TString             m_strPrompt;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWOption,TObject)
};




// ---------------------------------------------------------------------------
//   CLASS: TZWOptionVal
//  PREFIX: zwoptv
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWOptionVal : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TZWOptionVal&           zwoptvSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWOptionVal
        (
            const   tCIDLib::TBoolean       bFixed = kCIDLib::False
        );

        TZWOptionVal
        (
            const   TString&                strKey
            , const TString&                strValue
            , const tCIDLib::TBoolean       bFixed
            , const TString&                strAllowedVals
        );

        TZWOptionVal
        (
            const   TZWOptionVal&           zwoptvSrc
        );

        ~TZWOptionVal();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWOptionVal& operator=
        (
            const   TZWOptionVal&           zwoptvSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Set
        (
            const   TString&                strKey
            , const TString&                strValue
            , const tCIDLib::TBoolean       bFixed
            , const TString&                strAllowedVals
        );


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_bFixed
        //      This let's the dev info files force fixed configuration values.
        //      When it's know that certain options must be a particular way, it
        //      be defaulted and marked fixed in the dev info file. We won't let
        //      the user edit these.
        //
        //  m_colAllowedVals
        //      In some cases, we can't fix an option in the device info, but we
        //      don't want to allow all possible values because only some of them
        //      could possibly be valid. So this can be a comma separated list of
        //      option value keys of the valid ones. If this is empty, then all
        //      of them are presented.
        //
        //  m_strKey
        //      The key for this option.
        //
        //  m_strValue
        //      The selected value for this option.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bFixed;
        tCIDLib::TStrList   m_colAllowedVals;
        TString             m_strKey;
        TString             m_strValue;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StoreAllowedVals
        (
            const   TString&                strVals
        );


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWOptionVal,TObject)
};

#pragma CIDLIB_POPPACK


