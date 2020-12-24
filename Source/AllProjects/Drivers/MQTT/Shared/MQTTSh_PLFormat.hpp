//
// FILE NAME: MQTTSh_PLFormat.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/31/2019
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
//  This is the header for the subscription info related classes. This is driven
//  by the configuration file, which is an XML file that we parse in.
//
//  * This configuration info will grow over time. Currently it only allows you to
//    define single topic to field relationships. Later, we'll probably allow for
//    more 'device oriented' configuration as well, based on the standard device
//    classes, so you can create things like thermos or media players in one shot.
//
//  Single Fields
//
//  For fields though each particular field type in the config (which are driven
//  by the semantic field types) has a particular set of configuable values, there's
//  only a fairly small set overall. So we have a field config class that has
//  enough members to deal with all of them, and the appropriate ones are set based
//  on config (some being provided automatically based semantic type.)
//
//  We also need information about how to interpret the payload data in the publish
//  msg. We support a number of schemes that have to be indicated. That is defined
//  in the 'data rep' class. Each field config contains one of these.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TMQTTPLFmt
//  PREFIX: mqplf
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTPLFmt : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTPLFmt();

        TMQTTPLFmt(const TMQTTPLFmt&) = default;

        ~TMQTTPLFmt();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTPLFmt& operator=(const TMQTTPLFmt&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsNFNumericPL() const;

        tCIDLib::TBoolean bIsNumericPL() const;

        tCIDLib::TVoid ParseFrom
        (
            const   TString&                strFldName
            , const tCQCKit::EFldSTypes     eSemType
            , const tCQCKit::EFldTypes      eFldType
            , const tCQCKit::EFldAccess     eAccess
            , const TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        );


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_bBigEndian
        //      If a binary type, this is the endianness if m_c4PLBytes > 1
        //
        //  m_c4Bytes
        //      If a binary type this is the number of bytes in the value.
        //
        //  m_c4DecDigits
        //      Floating point values are assumed to be text. This indicates how many
        //      decimal digits to format on outgoing values of that type. Cannto be
        //      used with m_c4Precision. If set, implies that the value is float, though
        //      m_eType is text.
        //
        //  m_c4Offset
        //      Indicates how many bytes into the payload the value is. Defaults to
        //      zero if not explicitly provided.
        //
        //  m_c4Precision
        //      If a floating point value encoded in text without an explicit
        //      decimal, this is the precision of the value, i.e. we divid by 10^prec.
        //      Cannot be set if m_c4DecDigits is set. If so, implies that the value is
        //      float (m_eType indicate text.)
        //
        //  m_eType
        //      Is it card,  int or text format (with special text formats like JSON or
        //      XML.) Float values are assumed to be text encoded, or as text/cards/ints
        //      with a precision value.
        //
        //  m_cptrEncode
        //      If it's 'binary text' then we need a converter for the encoding that it
        //      is in. If not, this is null. We use a unique pointer to manage it so that
        //      we can still have default copy/assign. The driver will only use one copy
        //      of the config at once, so it doesn't matter if the original and the
        //      copy of this object reference the same object.
        //
        //      Has to be mutable because we use it in the otherwise const in/out mapping
        //      methods.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean               m_bBigEndian;
        tCIDLib::TCard4                 m_c4Bytes;
        tCIDLib::TCard4                 m_c4DecDigits;
        tCIDLib::TCard4                 m_c4Offset;
        tCIDLib::TCard4                 m_c4Precision;
        tMQTTSh::EPLTypes               m_eType;
        mutable tCIDEncode::TTCvtPtr    m_cptrEncode;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTPLFmt, TObject)
};

#pragma CIDLIB_POPPACK
