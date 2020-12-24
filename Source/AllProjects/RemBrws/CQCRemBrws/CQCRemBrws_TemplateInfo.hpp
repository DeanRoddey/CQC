//
// FILE NAME: CQCRemBrws_TemplateInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/22/2003
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
//  This is the header for the CQCKit_TemplateInfo.cpp file, which implements the
//  TCQCIntfTmplInfo class.
//
//  THIS FILE is now only kept for backwards compatibility and just enough to read in
//  the data. As of 5.3.903 we moved to a new, consistent file storage format based
//  on our new TChunkedFile class. We just need this to be able to read in the data
//  and get it converted over.
//
//  There's no need for them to be public anymore either, so they are no longer
//  exported.
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
//   CLASS: TCQCIntfTmplInfo
//  PREFIX: itfi
// ---------------------------------------------------------------------------
class TCQCIntfTmplInfo : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfTmplInfo();

        TCQCIntfTmplInfo(const TCQCIntfTmplInfo&) = default;
        TCQCIntfTmplInfo(TCQCIntfTmplInfo&&) = default;

        ~TCQCIntfTmplInfo();


        TCQCIntfTmplInfo& operator=(const TCQCIntfTmplInfo&) = default;
        TCQCIntfTmplInfo& operator=(TCQCIntfTmplInfo&&) = default;


        // -------------------------------------------------------------------
        //  No need for getters/setters for this legacy code so just make the data public
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4Bytes;
        tCQCKit::EUserRoles m_eMinRole;
        TString             m_strName;
        TString             m_strNotes;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Magic Intfs
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfTmplInfo,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTmplData
//  PREFIX: itfd
// ---------------------------------------------------------------------------
class TCQCIntfTmplData : public TCQCIntfTmplInfo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfTmplData();

        TCQCIntfTmplData(const TCQCIntfTmplData&) = default;
        TCQCIntfTmplData(TCQCIntfTmplData&&) = default;

        ~TCQCIntfTmplData();

        TCQCIntfTmplData& operator=(const TCQCIntfTmplData&) = default;
        TCQCIntfTmplData& operator=(TCQCIntfTmplData&&) = default;

        // -------------------------------------------------------------------
        //  Just make the data public for our legacy purposes
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4SerialNum;
        THeapBuf        m_mbufData;


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
        //  Magic Intfs
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfTmplData,TCQCIntfTmplInfo)
};

#pragma CIDLIB_POPPACK


