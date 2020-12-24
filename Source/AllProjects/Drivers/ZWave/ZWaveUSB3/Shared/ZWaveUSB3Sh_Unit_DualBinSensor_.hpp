//
// FILE NAME: ZWaveUSB3Sh_Unit_DualBinSensor_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/09/2018
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
//  This is a handler for a type of unit that is unfortunately sort of common, where
//  it has two binary sensors but doesn't implement multi-channel. Instead they are
//  only distinguished by the CC that they use. So we have to special case these and
//  manage the CC impls ourself and route msgs to them selectively.
//
//  For each one we have extra data to indicate which CC is used to report the state
//  of the sensor, and the type of the sensor so that we can create V2 compliant fields
//  where possible.
//
//  They have to use separate CCs. If one is notification, then we have to be told what
//  notification ids are relevant.
//
//  The 'extra' configuration data has at least the following data.
//
//      Class[1|2] : Basic | BinSensor | Notification
//       Type[1|2] : CO | CO2 | Door | Glass | Heat | Intrusion | Motion | Smoke | Tilt| Water
//
//  So it's the same info twice, with a 1 or 2 appended to the key.
//
//  NOTE THAT the Type value must match the EBinSensorTypes enum in the IDL file.
//
//
//  If the class indicates this one is notification based, then the correct notification
//  ids need to be set on the Notification CC.
//
//  For motions sensors, we create a V2 compliant field and send out motion triggers
//  if configured to do so. For the others we create a field in the form:
//
//      [unitname]_StateX
//
//  where X is 1 or 2, depending on if it is the first or second sensor.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TDualBinSensorUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TDualBinSensorUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDualBinSensorUnit() = delete;

        TDualBinSensorUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );

        TDualBinSensorUnit(const TDualBinSensorUnit&) = delete;

        ~TDualBinSensorUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDualBinSensorUnit& operator=(const TDualBinSensorUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const final;

        tCIDLib::TBoolean bParseExtraInfo() final;

        tCIDLib::TBoolean bPrepare() final;

        tCIDLib::TVoid FormatCCsInfo
        (
                    TTextOutStream&         strmTar
        )   const final;

        tCIDLib::TVoid HandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        )   final;

        tCIDLib::TVoid ImplValueChanged
        (
            const   tCIDLib::TCard4         c4ImplId
            , const tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TBoolean       bWasInErr
            , const tCIDLib::TInt4          i4ExtraInfo
        )   final;

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const final;

        tCIDLib::TVoid QueryUnitAttrs
        (
                    tCIDMData::TAttrList&   colToFill
            ,       TAttrData&              attrTmp
        )   const final;

        tCIDLib::TVoid Process() final;

        tCIDLib::TVoid StoreFldIds() final;

        tCIDLib::TVoid UnitIsOnline() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bMakeSensorCC
        (
            const   tZWaveUSB3Sh::ECClasses     eHandlerClass
            , const tZWaveUSB3Sh::EBinSensorTypes& eSensorType
            ,       TString&                    strFldName
            ,       TZWCCImplBin*&              pzwcciVal
            , const tCIDLib::TBoolean           bFirst
        );



        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eHandlerClass1
        //  m_eHandlerClass2
        //      Indicates what type of CC impl to create to handle each sensor, i.e. what
        //      types of reports we should expect. This is driven by the device info.
        //
        //  m_eSensorType1
        //  m_eSensorType2
        //      The specific type of binary sensors. This tells us more about the unit
        //      which may be important in setting up the CC impl.
        //
        //  m_pzwcciVal1
        //  m_pzwcciVal2
        //      We manage our own CC impls so that we can route msgs appropriately. We
        //      store them via the common base binary CC class, and cast as required.
        //
        //  m_strFldName1
        //  m_strFldName2
        //      The binary base classes we use to store the impls don't know from field
        //      names, so we keep these around for log and error msgs.
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::ECClasses         m_eHandlerClass1;
        tZWaveUSB3Sh::ECClasses         m_eHandlerClass2;
        tZWaveUSB3Sh::EBinSensorTypes   m_eSensorType1;
        tZWaveUSB3Sh::EBinSensorTypes   m_eSensorType2;
        TZWCCImplBin*                   m_pzwcciVal1;
        TZWCCImplBin*                   m_pzwcciVal2;
        TString                         m_strFldName1;
        TString                         m_strFldName2;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDualBinSensorUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
