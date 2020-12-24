//
// FILE NAME: CQCIR_BlasterCfg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/24/2002
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
//  This class implements the IR blaster driver configuration class. This is
//  how we store the data on disk.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIR_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TIRBlasterCfg,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIR_BlasterCfg
{
    namespace
    {
        // -------------------------------------------------------------------
        //  The current version of this class' format
        // -------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion  = 1;


        // -------------------------------------------------------------------
        //  The hash modulus we use for our device map
        // -------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4Modulus = 29;
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TIRBlasterCfg
//  PREFIX: irbc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIRBlasterCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TIRBlasterCfg::TIRBlasterCfg() :

    m_colModelMap
    (
        CQCIR_BlasterCfg::c4Modulus, TStringKeyOps(), &TIRBlasterDevModel::strKey
    )
{
}

TIRBlasterCfg::~TIRBlasterCfg()
{
}


// ---------------------------------------------------------------------------
//  TIRBlasterCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIRBlasterCfg::bModelExists(const TString& strModelToFind) const
{
    return m_colModelMap.bKeyExists(strModelToFind);
}


tCIDLib::TBoolean TIRBlasterCfg::bRemoveModel(const TString& strDevModel)
{
    return m_colModelMap.bRemoveKeyIfExists(strDevModel);
}


TIRBlasterCfg::TModelCursor TIRBlasterCfg::cursModels() const
{
    return TIRBlasterCfg::TModelCursor(&m_colModelMap);
}


TIRBlasterCmd&
TIRBlasterCfg::irbcAddOrUpdate( const   TString&            strDevModel
                                , const TIRBlasterCmd&      irbcToAdd
                                ,       tCIDLib::TBoolean&  bAdded)
{
    TIRBlasterDevModel* pirbdmTarget = m_colModelMap.pobjFindByKey(strDevModel);
    if (!pirbdmTarget)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcCfg_UnknownDevModel
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strDevModel
        );
    }
    return pirbdmTarget->irbcAddOrUpdate(irbcToAdd, bAdded);
}


const TIRBlasterCmd&
TIRBlasterCfg::irbcFromName( const  TString&            strDevModel
                            , const TString&            strCmdName
                            ,       tCIDLib::TCard4&    c4RepeatCount) const
{
    const TIRBlasterDevModel& irbdmRet = irbdmFromName(strDevModel);
    c4RepeatCount = irbdmRet.c4RepeatCount();
    return irbdmRet.irbcFromName(strCmdName);
}


TIRBlasterDevModel&
TIRBlasterCfg::irbdmAdd(const   TString&            strDevModel
                        , const TString&            strDevMake
                        , const TString&            strDescr
                        , const tCQCKit::EDevCats   eCategory)
{
    TIRBlasterDevModel irbdmTmp(strDevModel, strDevMake, strDescr, eCategory);
    return m_colModelMap.objAdd(irbdmTmp);
}


TIRBlasterDevModel&
TIRBlasterCfg::irbdmAddOrUpdate(const   TIRBlasterDevModel& irbdmToAdd
                                ,       tCIDLib::TBoolean&  bAdded)
{
    return m_colModelMap.objAddOrUpdate(irbdmToAdd, bAdded);
}


const TIRBlasterDevModel&
TIRBlasterCfg::irbdmFromName(const TString& strDevModel) const
{
    const TIRBlasterDevModel* pirbdmRet = m_colModelMap.pobjFindByKey(strDevModel);
    if (!pirbdmRet)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcCfg_UnknownDevModel
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strDevModel
        );
    }
    return *pirbdmRet;
}

TIRBlasterDevModel& TIRBlasterCfg::irbdmFromName(const TString& strDevModel)
{
    TIRBlasterDevModel* pirbdmRet = m_colModelMap.pobjFindByKey(strDevModel);
    if (!pirbdmRet)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcCfg_UnknownDevModel
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strDevModel
        );
    }
    return *pirbdmRet;
}


tCIDLib::TVoid TIRBlasterCfg::Reset()
{
    m_colModelMap.RemoveAll();
}


// ---------------------------------------------------------------------------
//  TIRBlasterCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIRBlasterCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start obj marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCIR_BlasterCfg::c2FmtVersion)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Now pull in the devices, if any. Flush current items first
    m_colModelMap.RemoveAll();
    strmToReadFrom >> m_colModelMap;

    // Make sure we get the end obj marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TIRBlasterCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do a marker first
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIR_BlasterCfg::c2FmtVersion;

    // Just ask the device map to write itself out
    strmToWriteTo << m_colModelMap;

    // End off with an end object marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


