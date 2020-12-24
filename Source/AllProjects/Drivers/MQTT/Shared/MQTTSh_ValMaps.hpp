//
// FILE NAME: MQTTSh_ValMaps.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/10/2019
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
//  This is the header for the value mappings classes, or the core ones, there
//  may be others later. These are driven by config and each field's payload format
//  object will create an appropriate type and let it parse its data out of the
//  XML.
//
//  We have a base abstract class that provides the interface, and a derivative
//  for each map type defined in the DTD.
//
//  Some maps are single level and some are dual level. Ther former are single pass
//  where the incoming MQTT value is mapped to a field value, and an outgoing field
//  value is mapped to an MQTT value.
//
//  The latter are things like JSON or multi-line text, where the actual value we
//  want is embedded in a larger value and needs to be extracted (or needs to be
//  embedded into a larger value for sending out.) These have a nested map that does
//  the above single pass job, and they themselves do the extraction/embedding
//  part. For now, all dual level ones are text.
//
//  So the process on incoming publish msgs is:
//
//      ePreMapInVal -  For single pass maps it's just a pass-through, i.e. the incoming
//                      value is copied to the outgoing value. For dual level
//                      maps this will extract the actual MQTT value from the raw
//                      payload text.
//      eMapInVal    -  Takes the MQTT value and maps it to the field value, same for
//                      both (the dual ones using their nested map.)
//
//  For output it is:
//
//      bMapOutVal    - Maps the field value to the MQTT value, same for both types with
//                      the dual level ones using their nested map.
//      bPostMapOutVal- For single pass this is a pass-through. For dual level ones
//                      this embeds the MQTT value into the larger output text.
//
//
//  To avoid redundancy for the JSON/multi-line type maps, we create a TMQTTComplexMap
//  class that most of those types of maps will derive from. It will provide some
//  common functionality for massaging the raw value after it is extracted from the
//  larger text content. The derived classes will call it to do that work after they
//  do the format specific extraction.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TMQTTValMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTValMap : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTValMap(const TMQTTValMap&) = default;

        ~TMQTTValMap() {}


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTValMap& operator=(const TMQTTValMap&) = default;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bIsValidFor
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TMQTTPLFmt&             mqplfVals
        )   const = 0;

        virtual tCIDLib::TBoolean bMapOutVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strFldVal
            ,       TString&                strMQTTVal
            , const TMQTTPLFmt&             mqplfVals
        )   const = 0;

        virtual tCIDLib::TBoolean bPostMapOutVal
        (
            const   TString&                strMQTTVal
            ,       TString&                strOutVal
        )   const;

        virtual tCIDLib::TBoolean bParseFrom
        (
            const   TString&                strFldName
            , const tCQCKit::EFldAccess     eAccess
            , const TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        ) = 0;

        virtual tMQTTSh::EInMapRes eMapInVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strMQTTVal
            ,       TString&                strFldVal
            , const TMQTTPLFmt&             mqplfVals
        )   const = 0;

        virtual tMQTTSh::EInMapRes ePreMapInVal
        (
            const   TString&                strInVal
            ,       TString&                strMQTTVal
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TMQTTValMap() {}


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddErr
        (
                    tCIDLib::TStrCollect&   colErrs
            , const TString&                strFldName
            , const TString&                strErr
            , const MFormattable&           mfmtblToken1 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken2 = MFormattable::Nul_MFormattable()
        );


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTValMap, TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TMQTTComplexMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTComplexMap : public TMQTTValMap
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        ~TMQTTComplexMap();


        // -------------------------------------------------------------------
        //
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseFrom
        (
            const   TString&                strFldName
            , const tCQCKit::EFldAccess     eAccess
            , const TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        )   override;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TMQTTComplexMap();
        TMQTTComplexMap(const TMQTTComplexMap&) = default;
        TMQTTComplexMap& operator=(const TMQTTComplexMap&) = default;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tMQTTSh::EInMapRes eProcessValue
        (
                    TString&                strToProcess
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Protected class types
        // -------------------------------------------------------------------
        enum class EExFroms
        {
            Start
            , Char
            , Offset
        };

        enum class EExTos
        {
            End
            , Char
            , Count
            , Offset
        };


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bStripWS
        //      Indicates if we should strip whitespace from the value after we extract
        //      it, which can make the extraction a lot easier to set up.
        //
        //  m_c4From/To
        //  m_chFrom/To
        //      If the from or two extraction types are Char or Offset, these hold the
        //      configured values to use.
        //
        //  m_eFrom
        //  m_eTo
        //      How we extract the actual value from the line we find. They default to
        //      Start/End, i.e. the whole line. Else, we use the From/To values above
        //      to decide what to extract.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bStripWS;
        tCIDLib::TCard4     m_c4FromOfs;
        tCIDLib::TCard4     m_c4ToCount;
        tCIDLib::TCard4     m_c4ToOfs;
        tCIDLib::TCh        m_chFrom;
        tCIDLib::TCh        m_chTo;
        EExFroms            m_eFrom;
        EExTos              m_eTo;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTComplexMap, TMQTTValMap)
};



// ---------------------------------------------------------------------------
//   CLASS: TMQTTBoolGenMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTBoolGenMap : public TMQTTValMap
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTBoolGenMap();

        TMQTTBoolGenMap(const TMQTTBoolGenMap&) = default;

        ~TMQTTBoolGenMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTBoolGenMap& operator=(const TMQTTBoolGenMap&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsValidFor
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bMapOutVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strFldVal
            ,       TString&                strMQTTVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bParseFrom
        (
            const   TString&                strFldName
            , const tCQCKit::EFldAccess     eAccess
            , const TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        )   override;

        tMQTTSh::EInMapRes eMapInVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strMQTTVal
            ,       TString&                strFldVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strOutFalse
        //  m_strOutTrue
        //      The fixed values we send out for true/false.
        // -------------------------------------------------------------------
        TString     m_strOutFalse;
        TString     m_strOutTrue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTBoolGenMap, TMQTTValMap)
};



// ---------------------------------------------------------------------------
//   CLASS: TMQTTBoolNumMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTBoolNumMap : public TMQTTValMap
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTBoolNumMap();

        TMQTTBoolNumMap(const TMQTTBoolNumMap&) = default;

        ~TMQTTBoolNumMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTBoolNumMap& operator=(const TMQTTBoolNumMap&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsValidFor
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bMapOutVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strFldVal
            ,       TString&                strMQTTVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

         tCIDLib::TBoolean bParseFrom
        (
            const   TString&                strFldName
            , const tCQCKit::EFldAccess     eAccess
            , const TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        )   override;

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TInt8          i8FalseVal
            , const tCIDLib::TInt8          i8TrueVal
        );

        tMQTTSh::EInMapRes eMapInVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strMQTTVal
            ,       TString&                strFldVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  A small class to hold the individual true/false to range mappings.
        // -------------------------------------------------------------------
        class TARange
        {
            public :
                TARange();

                TARange(const   tCIDLib::TBoolean   bTarVal
                        , const tCIDLib::TInt8      i8MinVal
                        , const tCIDLib::TInt8      i8MaxVal) :

                    m_bTarVal(bTarVal)
                    , m_i8MaxVal(i8MaxVal)
                    , m_i8MinVal(i8MinVal)
                {
                }

                TARange(const TARange&) = default;

                ~TARange() {}

                tCIDLib::TBoolean   m_bTarVal;
                tCIDLib::TInt8      m_i8MaxVal;
                tCIDLib::TInt8      m_i8MinVal;
        };
        using TRangeList = TVector<TARange>;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colMapIn
        //      A list of TARange objects to map incoming values to a true or false
        //      value.
        //
        //  m_i8FalseVal
        //  m_i8TrueVal
        //      The outgoing values for true and false, stored as Int8 so that we can
        //      handle either signed or unsigned 32 bit values.
        // -------------------------------------------------------------------
        TRangeList          m_colMapIn;
        tCIDLib::TInt8      m_i8FalseVal;
        tCIDLib::TInt8      m_i8TrueVal;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTBoolNumMap, TMQTTValMap)
};



// ---------------------------------------------------------------------------
//   CLASS: TMQTTBoolTextMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTBoolTextMap : public TMQTTValMap
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTBoolTextMap();

        TMQTTBoolTextMap(const TMQTTBoolTextMap&) = default;

        ~TMQTTBoolTextMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTBoolTextMap& operator=(const TMQTTBoolTextMap&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsValidFor
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bMapOutVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strFldVal
            ,       TString&                strMQTTVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bParseFrom
        (
            const   TString&                strFldName
            , const tCQCKit::EFldAccess     eAccess
            , const TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        )   override;

        tCIDLib::TVoid Reset
        (
            const   TString&                strFalseOut
            , const TString&                strTrueOut
        );

        tMQTTSh::EInMapRes eMapInVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strMQTTVal
            ,       TString&                strFldVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colInFalseMap
        //  m_colInTrueMap
        //      We just store the incoming values that map to true and that map to
        //      false, for quick lookup.
        //
        //  m_strOutFalse
        //  m_strOutTrue
        //      The fixed values we send out for true/false.
        // -------------------------------------------------------------------
        tCIDLib::TStrHashSet    m_colInFalseMap;
        tCIDLib::TStrHashSet    m_colInTrueMap;
        TString                 m_strOutFalse;
        TString                 m_strOutTrue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTBoolTextMap, TMQTTValMap)
};



// ---------------------------------------------------------------------------
//   CLASS: TMQTTEnumMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTEnumMap : public TMQTTValMap
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTEnumMap();

        TMQTTEnumMap(const TMQTTEnumMap&) = default;

        ~TMQTTEnumMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTEnumMap& operator=(const TMQTTEnumMap&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsValidFor
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bMapOutVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strFldVal
            ,       TString&                strMQTTVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bParseFrom
        (
            const   TString&                strFldName
            , const tCQCKit::EFldAccess     eAccess
            , const TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        )   override;

        tMQTTSh::EInMapRes eMapInVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strMQTTVal
            ,       TString&                strFldVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colMapIn
        //  m_colMapOut
        //      We set up two keyed hash sets over key/value pairs. In one we have the
        //      MQTT value as the key and in the other the fld value, so we can map
        //      quickly either way.
        // -------------------------------------------------------------------
        tCIDLib::TKVHashSet m_colMapIn;
        tCIDLib::TKVHashSet m_colMapOut;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTEnumMap, TMQTTValMap)
};



// ---------------------------------------------------------------------------
//   CLASS: TMQTTJSONMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTJSONMap : public TMQTTComplexMap
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTJSONMap();

        TMQTTJSONMap(const TMQTTJSONMap&) = default;

        ~TMQTTJSONMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTJSONMap& operator=(const TMQTTJSONMap&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsValidFor
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bMapOutVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strFldVal
            ,       TString&                strMQTTVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bPostMapOutVal
        (
            const   TString&                strMQTTVal
            ,       TString&                strOutVal
        )   const override;

        tCIDLib::TBoolean bParseFrom
        (
            const   TString&                strFldName
            , const tCQCKit::EFldAccess     eAccess
            , const TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        )   override;

        tMQTTSh::EInMapRes ePreMapInVal
        (
            const   TString&                strInVal
            ,       TString&                strMQTTVal
        )   const override;

        tMQTTSh::EInMapRes eMapInVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strMQTTVal
            ,       TString&                strFldVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bIgnoreNotFnd
        //      If we get a payload and don't find our configured value path in it,
        //      we treat that as an error. But, in some cases that can't be done
        //      because of stupid devices that send varying payloads. So they can
        //      mark us to just ignore those and just take the ones that have our
        //      value.
        //
        //  m_jprsIn
        //      A JSON parser we can use for parsing the incoming JSON info.
        //
        //  m_pmqvmpValue
        //      We have another map as a child. We use it to do the actual value
        //      mapping, and we handle the JSON bits around it.
        //
        //  m_strInPath
        //      The path to look in the incoming JSON to find the value.
        //
        //  m_strOutJSON
        //      The JSON in which we embed the outgoing value (translated by our
        //      internal map.)
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bIgnoreNotFnd;
        mutable TJSONParser m_jprsIn;
        TMQTTValMap*        m_pmqvmpValue;
        TString             m_strInPath;
        TString             m_strOutJSON;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTJSONMap, TMQTTComplexMap)
};


// ---------------------------------------------------------------------------
//   CLASS: TMQTTMLineMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTMLineMap : public TMQTTComplexMap
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTMLineMap();

        TMQTTMLineMap(const TMQTTMLineMap&) = default;

        ~TMQTTMLineMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTMLineMap& operator=(const TMQTTMLineMap&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsValidFor
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bMapOutVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strFldVal
            ,       TString&                strMQTTVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bPostMapOutVal
        (
            const   TString&                strMQTTVal
            ,       TString&                strOutVal
        )   const override;

        tCIDLib::TBoolean bParseFrom
        (
            const   TString&                strFldName
            , const tCQCKit::EFldAccess     eAccess
            , const TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        )   override;

        tMQTTSh::EInMapRes ePreMapInVal
        (
            const   TString&                strInVal
            ,       TString&                strMQTTVal
        )   const override;

        tMQTTSh::EInMapRes eMapInVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strMQTTVal
            ,       TString&                strFldVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4LineNum
        //      The number of the line to extract, one based.
        //
        //  m_pmqvmpValue
        //      We have another map as a child. We use it to do the actual value
        //      mapping, and we handle extracting it from the input.
        //
        //  m_strOutText
        //      The text in which we embed the outgoing value (translated by our
        //      internal map.) It has a %(v) replacement token in it.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4LineNum;
        TMQTTValMap*        m_pmqvmpValue;
        TString             m_strOutText;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTMLineMap, TMQTTComplexMap)
};



// ---------------------------------------------------------------------------
//   CLASS: TMQTTPassthroughMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTPassthroughMap : public TMQTTValMap
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTPassthroughMap();

        TMQTTPassthroughMap(const TMQTTPassthroughMap&) = default;

        ~TMQTTPassthroughMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTPassthroughMap& operator=(const TMQTTPassthroughMap&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsValidFor
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bMapOutVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strFldVal
            ,       TString&                strMQTTVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bParseFrom
        (
            const   TString&                strFldName
            , const tCQCKit::EFldAccess     eAccess
            , const TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        )   override;

        tMQTTSh::EInMapRes eMapInVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strMQTTVal
            ,       TString&                strFldVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTPassthroughMap, TMQTTValMap)
};



// ---------------------------------------------------------------------------
//   CLASS: TMQTTScaleRangeMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTScaleRangeMap : public TMQTTValMap
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTScaleRangeMap();

        TMQTTScaleRangeMap(const TMQTTScaleRangeMap&) = default;

        ~TMQTTScaleRangeMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTScaleRangeMap& operator=(const TMQTTScaleRangeMap&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsValidFor
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bMapOutVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strFldVal
            ,       TString&                strMQTTVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bParseFrom
        (
            const   TString&                strFldName
            , const tCQCKit::EFldAccess     eAccess
            , const TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        )   override;

        tMQTTSh::EInMapRes eMapInVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strMQTTVal
            ,       TString&                strFldVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        using TMapVals  = tCIDLib::TStrHashSet;
        using TValMap   = THashMap<TMapVals, TString, TStringKeyOps>;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_f8XXX
        //      The ranges of values available. We store as float for the worst case
        //      scenario. We pre-calc the range between the min/max values for use in
        //      scaling calculations.
        // -------------------------------------------------------------------
        tCIDLib::TFloat8    m_f8FldMax;
        tCIDLib::TFloat8    m_f8FldMin;
        tCIDLib::TFloat8    m_f8FldRange;
        tCIDLib::TFloat8    m_f8MQTTMax;
        tCIDLib::TFloat8    m_f8MQTTMin;
        tCIDLib::TFloat8    m_f8MQTTRange;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTScaleRangeMap, TMQTTValMap)
};


// ---------------------------------------------------------------------------
//   CLASS: TMQTTTextMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTTextMap : public TMQTTValMap
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTTextMap();

        TMQTTTextMap(const TMQTTTextMap&) = default;

        ~TMQTTTextMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTTextMap& operator=(const TMQTTTextMap&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsValidFor
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bMapOutVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strFldVal
            ,       TString&                strMQTTVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;

        tCIDLib::TBoolean bParseFrom
        (
            const   TString&                strFldName
            , const tCQCKit::EFldAccess     eAccess
            , const TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        )   override;

        tMQTTSh::EInMapRes eMapInVal
        (
            const   tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldSTypes     eSemType
            , const TString&                strMQTTVal
            ,       TString&                strFldVal
            , const TMQTTPLFmt&             mqplfVals
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        using TMapVals  = THashSet<TString, TStringKeyOps>;
        using TValMap   = THashMap<TMapVals, TString, TStringKeyOps>;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colMapIn
        //  m_colMapOut
        //      A hash map for the in values and one for the out values.
        // -------------------------------------------------------------------
        TValMap     m_colMapIn;
        TValMap     m_colMapOut;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTTextMap, TMQTTValMap)
};

#pragma CIDLIB_POPPACK
