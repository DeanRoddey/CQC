//
// FILE NAME: CQCIR_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/29/2003
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
//  This file defines facility class for this facility. This guy provides some
//  commmon functionality related to device types and accessing the IR
//  device model repository and for writing out common IRExport file foramt
//  data.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCIR
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCIREXPORT TFacCQCIR : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        using TModelInfoList = TVector<TIRBlasterDevModelInfo>;


        // -------------------------------------------------------------------
        //  Public, static data
        // -------------------------------------------------------------------
        static const TString strFldName_FirmwareVer;
        static const TString strFldName_Invoke;
        static const TString strFldName_TrainingState;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCIR();

        TFacCQCIR(const TFacCQCIR&) = delete;
        TFacCQCIR(TFacCQCIR&&) = delete;

        ~TFacCQCIR();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCIR& operator=(const TFacCQCIR&) = delete;
        TFacCQCIR& operator=(TFacCQCIR&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsValidGC100BlastData
        (
            const   TString&                strTest
        )   const;

        tCIDLib::TBoolean bIsValidR2DIBlastData
        (
            const   TString&                strTest
        )   const;

        tCIDLib::TBoolean bIsValidUIRTBlastData
        (
            const   TString&                strTest
        )   const;

        tCIDLib::TBoolean bIsValidNameChar
        (
            const   tCIDLib::TCh            chToCheck
            , const tCIDLib::TCard4         c4Index
        )   const;

        tCIDLib::TCard4 c4ParseInvokeCmd
        (
            const   TString&                strToParse
            ,       TString&                strDevName
            ,       TString&                strCmdName
            , const tCIDLib::TCard4         c4ZoneCount
            , const TString&                strMoniker
        )   const;

        tCIDLib::TCard4 c4QueryDevIRModels
        (
            const   TString&                strMoniker
            ,       TModelInfoList&         colToFill
        );

        tCIDLib::TCard4 c4QueryDevIRModels
        (
                    tCQCKit::TCQCSrvProxy&  orbcDev
            , const TString&                strMoniker
            ,       TModelInfoList&         colToFill
        );

        tCQCIR::EIRDevCaps eExtractDevInfo
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
            ,       TString&                strDevType
            ,       TString&                strDevDescr
        );

        TStdVarsTar& ctarGVars();

        tCQCIR::TRepoSrvProxy orbcRepoSrvProxy
        (
            const   tCIDLib::TCard4         c4WaitUpTo = 0
        )   const;

        tCIDLib::TVoid WriteExpFooter
        (
                    TTextOutStream&         strmOut
            , const TString&                strIRDevName
            , const TString&                strMake
            , const TString&                strModel
        );

        tCIDLib::TVoid WriteExpHeader
        (
                    TTextOutStream&         strmOut
            , const TString&                strIRDevName
            , const TString&                strMake
            , const TString&                strModel
            , const TString&                strDescription
            , const TString&                strCategory
        );

        tCIDLib::TVoid ProntoToGC100
        (
            const   TString&                strSrc
            ,       TTextOutStream&         strTarget
        );

        tCIDLib::TVoid ProntoToR2DI
        (
            const   TString&                strSrc
            ,       TTextOutStream&         strTarget
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_ctarGVars
        //      The background threads that process IR invoked actions will
        //      use this single object for their global variables. This will
        //      allow all of the actions to share a set of global variables.
        // -------------------------------------------------------------------
        TStdVarsTar m_ctarGVars;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCIR,TFacility)
};

#pragma CIDLIB_POPPACK

