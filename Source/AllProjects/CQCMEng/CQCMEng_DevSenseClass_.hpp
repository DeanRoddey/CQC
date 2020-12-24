//
// FILE NAME: CQCMEng_DevSenseClass_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/08/2014
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
//  This is the header for the CQCMacroEng_DevSenseClass.cpp, which implements
//  the info class for the device sensing CML base class. Derivatives of this class
//  are created by driver writers to support auto-sensing of devices.
//
//  This guy doesn't need any data at this level, so we just use the standard CML
//  value class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TCQCDevSenseInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TCQCDevSenseInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strClassPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDevSenseInfo() = delete;

        TCQCDevSenseInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TCQCDevSenseInfo(const TCQCDevSenseInfo&) = delete;
        TCQCDevSenseInfo(TCQCDevSenseInfo&&) = delete;

        ~TCQCDevSenseInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDevSenseInfo& operator=(const TCQCDevSenseInfo&) = delete;
        TCQCDevSenseInfo& operator=(TCQCDevSenseInfo&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Init
        (
                    TCIDMacroEngine&        meOwner
        )   final;

        TMEngClassVal* pmecvMakeStorage
        (
            const   TString&                strName
            ,       TCIDMacroEngine&        meOwner
            , const tCIDMacroEng::EConstTypes  eConst
        )   const final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInvokeMethod
        (
                    TCIDMacroEngine&        meOwner
            , const TMEngMethodInfo&        methiTarget
            ,       TMEngClassVal&          mecvInstance
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2MethId_XXX
        //      To speed up dispatch, we store the ids of the methods that
        //      we set up during init.
        // -------------------------------------------------------------------
        tCIDLib::TCard2 m_c2MethId_DefCtor;
        tCIDLib::TCard2 m_c2MethId_Initialize;
        tCIDLib::TCard2 m_c2MethId_Terminate;
        tCIDLib::TCard2 m_c2MethId_TestOther;
        tCIDLib::TCard2 m_c2MethId_TestSerial;
        tCIDLib::TCard2 m_c2MethId_TestSocket;
        tCIDLib::TCard2 m_c2TypeId_SockProtos;
        TMEngClassInfo* m_pmeciCommCfg;
        TMEngClassInfo* m_pmeciCommPort;
        TMEngClassInfo* m_pmeciEndPoint;
        TMEngClassInfo* m_pmeciStreamSocket;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDevSenseInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK


