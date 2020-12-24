//
// FILE NAME: CQCKit_StdActEngine.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/01/2008
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
//  This class just provides a very simple action command processing engine
//  interface. This is used by CQC to run CQC actions, via some derivative or
//  other of this class.
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
//   CLASS: TCQCActEngine
//  PREFIX: acte
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCActEngine : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCActEngine(const TCQCActEngine&) = delete;
        TCQCActEngine(TCQCActEngine&&) = delete;

        ~TCQCActEngine();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCActEngine& operator=(const TCQCActEngine&) = delete;
        TCQCActEngine& operator=(TCQCActEngine&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCQCKit::TCmdTarList& colExtra() = 0;

        virtual const TStdVarsTar& ctarLocals() const = 0;

        virtual TStdVarsTar& ctarLocals() = 0;

        virtual tCIDLib::TVoid Init
        (
            const   TString&                strActParms
        );

        virtual MCQCCmdTracer* const pcmdtDebug() = 0;

        virtual tCIDLib::TVoid Reset() = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCUserCtx& cuctxToUse() const;

        tCQCKit::EUserRoles eUserRole() const;

        tCIDLib::TVoid QueryActParms
        (
                    TStdVarsTar&            ctarLocals
        )   const;

        const TCQCSecToken& sectUser() const;

        const TString& strUserName() const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQCActEngine
        (
            const   TCQCUserCtx&            cuctxToUse
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colParms
        //      A list of invocation parameters can be set on us, which will
        //      be placed into the local vars list under known names. We
        //      handle the setting/parsing on behalf of our derivatives.
        //
        //  m_cuctxToUse
        //      The user context that this action is being invoked within.
        //      The app provides it to us and we pass it on to any stuff that
        //      needs it that we invoke.
        // -------------------------------------------------------------------
        tCIDLib::TKVPList   m_colParms;
        const TCQCUserCtx&  m_cuctxToUse;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCActEngine,TObject)
};


#pragma CIDLIB_POPPACK

