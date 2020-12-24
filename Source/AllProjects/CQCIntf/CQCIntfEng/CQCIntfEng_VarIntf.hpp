//
// FILE NAME: CQCIntfEng_VarIntf.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/03/2008
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
//  This is the header for the CQCIntfEng_Varntf.cpp file, which implements
//  a simple mixin interface that all dynamic widgets based on a single
//  variable value will use. It serves two purposes. It handles a wee bit
//  of grunt work for them, and it provides a standard interface that an
//  attribute tab can use to set the variable name and data type on the widget
//  when the user provides it.
//
// CAVEATS/GOTCHAS:
//
//  1)  Classes that use this interface must include the 'wants value updates'
//      capabilities flag, so that they will get calls to the value update
//      method, which will call one of our methods to see if anything needs
//      be done.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfVarIntf
//  PREFIX: miwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT MCQCIntfVarIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~MCQCIntfVarIntf();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHasGoodVarValue() const;

        tCIDLib::TBoolean bHasVariable() const;

        tCIDLib::TBoolean bSameVariable
        (
            const   MCQCIntfVarIntf&        miwdgSrc
        )   const;

        tCIDLib::TVoid ClearVarName();

        tCQCIntfEng::EVarStates eVarState() const;

        tCQCKit::EFldTypes eVarType() const;

        tCIDLib::TVoid QueryVarAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const;

        tCIDLib::TVoid ReadInVar
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid ReplaceVar
        (
            const   TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        const TString& strVarName() const;

        tCIDLib::TVoid SetVarAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        );

        tCIDLib::TVoid SetVarInfo
        (
            const   TString&                strVarName
        );

        tCIDLib::TVoid ValidateVar
        (
                    tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
            , const TCQCIntfWidget&         iwdgThis
        )   const;

        tCIDLib::TVoid VarUpdate
        (
            const   TStdVarsTar&            ctarGlobalVars
            , const tCIDLib::TBoolean       bNoRedraw
        );

        tCIDLib::TVoid WriteOutVar
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        MCQCIntfVarIntf();

        MCQCIntfVarIntf
        (
            const   MCQCIntfVarIntf&        iwdgToCopy
        );

        MCQCIntfVarIntf& operator=
        (
            const   MCQCIntfVarIntf&        iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Protected virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bCanAcceptVar
        (
            const   TCQCActVar&             varToTest
        ) = 0;

        virtual tCIDLib::TVoid VarInError
        (
            const   tCIDLib::TBoolean       bNoRedraw
        ) = 0;

        virtual tCIDLib::TVoid VarValChanged
        (
            const   TCQCActVar&             varNew
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        ) = 0;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4SerialNum
        //      The last serial number we saw for our variable, which lets us
        //      know if it's changed. This is runtime only.
        //
        //  m_c4VarId
        //      If the enclosing class accepts a variable, we store its var
        //      id. We pass this back in when checking the value. If the
        //      var exists, but has been deleted and re-created, the var id
        //      will not match and we'll get a NotFound result, which will
        //      make us recycle and get the enclsing class to check it again
        //      for acceptance.
        //
        //  m_eState
        //      The current state of the state. This tracks our progress in
        //      finding the variable, verifying its type and limits, and keeping
        //      up with its value. We notify the enclosing class when this
        //      state changes. We present it with the variable when found, to
        //      ask if it's OK to use it. If not, we stay in TypeError state.
        //
        //  m_eType
        //      The type of the last variable that the enclosing class accepted.
        //      If we aren't currently set up with a variable, then it will
        //      set to the _Count value.
        //
        //  m_strVarName
        //      The name of the global variable (with prefix) that we are
        //      associated with. This is the only persisted value.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4SerialNum;
        tCIDLib::TCard4         m_c4VarId;
        tCQCIntfEng::EVarStates m_eState;
        tCQCKit::EFldTypes      m_eType;
        TString                 m_strVarName;
};

#pragma CIDLIB_POPPACK

