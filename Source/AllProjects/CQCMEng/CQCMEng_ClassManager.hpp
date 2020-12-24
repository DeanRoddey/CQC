//
// FILE NAME: CQCMEng_ClassManager.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/24/2001
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
//  This is the header for the CQCMacroEng_ClassManager.cpp file, which
//  implements a derivative of the CIDLib Macro Engine's class manager
//  interface. This allows CQC apps that use the macro engine to have a pre-
//  fab class manager to plug into their engine instance, so that it can
//  pull macros from the CQC macro repository.
//
//  This one is purely for non-interactive use, since it does not implement
//  the bDoSelect() method. Another one will be implemented in the GUI
//  specific code to use in interactive situations. This one just let's the
//  engine pull macro classes from the repository to compile and run.
//
//  We also provide a standard external class loader for all CQC apps to use.
//  This one handles making our CQC specific built in macro classes available
//  to the macro engine.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TCQCMEngClassMgr
// PREFIX: mecm
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TCQCMEngClassMgr : public TObject, public MMEngClassMgr
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCMEngClassMgr() = delete;

        TCQCMEngClassMgr
        (
            const   TCQCSecToken&           sectUser
        );

        TCQCMEngClassMgr(const TCQCMEngClassMgr&) = delete;
        TCQCMEngClassMgr(TCQCMEngClassMgr&&) = delete;

        ~TCQCMEngClassMgr();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCMEngClassMgr& operator=(const TCQCMEngClassMgr&) = delete;
        TCQCMEngClassMgr& operator=(TCQCMEngClassMgr&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCSecToken& sectUser() const;

        tCIDLib::TVoid Shutdown();


    protected :
        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckIfExists
        (
            const   TString&                strToCheck
        )   final;

        tCIDLib::EOpenRes eDoSelect
        (
                    TString&                strToFill
            , const tCIDMacroEng::EResModes    eMode
        )   final;

        tCIDLib::TVoid DoStore
        (
            const   TString&                strClassPath
            , const TString&                strText
        )   final;

        tCIDLib::TVoid DoUndoWriteMode
        (
            const   TString&                strClassPath
        )   final;

        TTextInStream* pstrmDoLoad
        (
            const   TString&                strClassPath
            , const tCIDMacroEng::EResModes    eMode
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_sectUser
        //      The caller has to provide us with a security token for the
        //      user account the action is running within, since we need it
        //      to do some things.
        // -------------------------------------------------------------------
        TCQCSecToken    m_sectUser;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCMEngClassMgr,TObject)
};


// ---------------------------------------------------------------------------
//  CLASS: TCQCRTClassLoader
// PREFIX: mecl
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TCQCRTClassLoader : public MMEngExtClassLoader
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCRTClassLoader() = delete;

        TCQCRTClassLoader
        (
            const   TCQCSecToken&           sectUser
        );

        TCQCRTClassLoader(const TCQCRTClassLoader&) = delete;
        TCQCRTClassLoader(TCQCRTClassLoader&&) = delete;

        ~TCQCRTClassLoader();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCRTClassLoader& operator=(const TCQCRTClassLoader&) = delete;
        TCQCRTClassLoader& operator=(TCQCRTClassLoader&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        TMEngClassInfo* pmeciLoadClass
        (
                    TCIDMacroEngine&        meTarget
            , const TString&                strClassPath
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_sectUser
        //      The caller has to provide us with a security token since we have to
        //      pass one on to some of the class classes.
        // -------------------------------------------------------------------
        TCQCSecToken        m_sectUser;
};

#pragma CIDLIB_POPPACK

