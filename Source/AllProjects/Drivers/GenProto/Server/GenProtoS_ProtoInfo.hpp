//
// FILE NAME: GenProtoS_ProtoInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This is the header file for the GenProtoS_ProtoInfo.cpp file, which
//  implements the main class defines a protocol. It contains the overall
//  protocol information and is the first thing streamed out when a protocol
//  is stored.
//
//  The information at this level is stuff like whether the protocol is
//  binary or text, how the messages are terminated, and whatnot.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoInfo
//  PREFIX: gpinfo
// ---------------------------------------------------------------------------
class TGenProtoInfo : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoInfo();

        TGenProtoInfo
        (
            const   TGenProtoInfo&          gpinfoToCopy
        );

        ~TGenProtoInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoInfo& operator=
        (
            const   TGenProtoInfo&          gpinfoToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const tCIDLib::TCard1* ac1StartDelim() const;

        const tCIDLib::TCard1* ac1TermDelim() const;

        tCIDLib::TBoolean bLittleEndian() const;

        tCIDLib::TCard4 c4EndDelimCount() const;

        tCIDLib::TCard4 c4StartDelimCount() const;

        tCIDLib::TCard4 c4LenOfs() const;

        tGenProtoS::EMsgDelims eDelim() const;

        tGenProtoS::EProtoFmts eFormat() const;


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
        //  Private data members
        //
        //  m_ac1StartDelim
        //  m_ac1TermDelim
        //      If the m_eDelim member indicates this protocol uses either
        //      start or term delimters, then these hold them.
        //
        //  m_bLittleEndian
        //      Indicates if the protocol is big or little endian. This is
        //      not relevant for an ASCII protocol or strictly byte oriented
        //      protocol. But where binary values larger than a byte are
        //      embedded indicates how to pull them out to our local host
        //      endianess. Since there are only two options, we just use a
        //      boolean.
        //
        //  m_c4EndDelimCount
        //      The number of bytes in the end delimiter, if any. The
        //      m_eDelim member indicates whether we have one or not.
        //
        //  m_c4StartDelimCount
        //      The number of bytes in the start delimiter, if any. The
        //      m_eDelim member indicates whether we have one or not.
        //
        //  m_c4LenOfs
        //      If the delimter type indicates embedded length, then this
        //      indicates the offset from the start of the message where
        //      the length is at. If the delimiter type indicates fixed
        //      length, this is the fixed length. Else ignored since term
        //      and/or start sequences are used for delimitation.
        //
        //  m_eDelim
        //      The message delimiter type used for this protocol. This value
        //      controls which of the other delimiter values are used.
        //
        //  m_eFormat
        //      The protocol format (e.g. binary or ASCII text.)
        // -------------------------------------------------------------------
        tCIDLib::TCard1         m_ac1StartDelim[kGenProtoS::c4MaxSeqBytes];
        tCIDLib::TCard1         m_ac1TermDelim[kGenProtoS::c4MaxSeqBytes];
        tCIDLib::TBoolean       m_bLittleEndian;
        tCIDLib::TCard4         m_c4EndDelimCount;
        tCIDLib::TCard4         m_c4StartDelimCount;
        tCIDLib::TCard4         m_c4LenOfs;
        tGenProtoS::EMsgDelims  m_eDelim;
        tGenProtoS::EProtoFmts  m_eFormat;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoInfo,TObject)
};

#pragma CIDLIB_POPPACK


