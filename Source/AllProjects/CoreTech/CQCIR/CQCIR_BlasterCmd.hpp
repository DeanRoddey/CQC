//
// FILE NAME: CQCIR_BlasterCmd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2002
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
//  This class represents a single IR command mapping. It includes the name
//  of the command, which is the key (e.g. Play, Stop, Power, etc...), and
//  the IR data string that we send to the blaster when this command is
//  invoked.
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
//   CLASS: TIRBlasterCmd
//  PREFIX: irbc
// ---------------------------------------------------------------------------
class CQCIREXPORT TIRBlasterCmd : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TIRBlasterCmd&          irbcSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIRBlasterCmd();

        TIRBlasterCmd
        (
            const   TString&                strName
        );

        TIRBlasterCmd
        (
            const   TString&                strName
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Len
        );

        TIRBlasterCmd
        (
            const   TString&                strName
            , const tCIDLib::TSCh* const    pchData
        );

        TIRBlasterCmd(const TIRBlasterCmd&) = default;
        TIRBlasterCmd(TIRBlasterCmd&&) = default;

        ~TIRBlasterCmd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIRBlasterCmd& operator=(const TIRBlasterCmd&) = default;
        TIRBlasterCmd& operator=(TIRBlasterCmd&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4DataLen() const;

        tCIDLib::TVoid FormatData
        (
                    TString&                strToFill
        )   const;

        const TMemBuf& mbufData() const;

        const TString& strName() const;

        const TString& strName
        (
            const   TString&                strNew
        );

        tCIDLib::TVoid Set
        (
            const   TString&                strNew
            , const TString&                strData
        );

        tCIDLib::TVoid SetData
        (
            const   TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Len
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4DataLen
        //      The length of data in the data buffer, i.e. the number of
        //      bytes that make up the IR command.
        //
        //  m_mbufData
        //      A buffer of data for this IR command. m_c4DataLen tells
        //      us how many bytes we have, since they can vary in length.
        //
        //  m_strName
        //      The name of the command. This is the key field, and it is case
        //      sensitive. You should always be consistent about using common
        //      names for common actions, i.e. always use Play or Stop for
        //      those common operations. This will will it easier for a user
        //      to change to a new device, since the commands will be the same,
        //      and just the device name needs to change.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4DataLen;
        THeapBuf            m_mbufData;
        TString             m_strName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TIRBlasterCmd,TObject)
};

#pragma CIDLIB_POPPACK


