//
// FILE NAME: CQCIGKit_ImportPackDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/23/2016
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
//  This is the header for the CQCIGKit_ImportPackDlg.cpp file, which provides a dialog
//  that lets a user import a CQC package. The caller has let the user select the
//  file, and passes it to us.
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
//   CLASS: TImportPackDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TImportPackDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TImportPackDlg();

        TImportPackDlg(const TImportPackDlg&) = delete;
        TImportPackDlg(TImportPackDlg&&) = delete;

        ~TImportPackDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TImportPackDlg& operator=(const TImportPackDlg&) = delete;
        TImportPackDlg& operator=(TImportPackDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strPackFile
            , const TCQCUserCtx&            cuctxToUse
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bLoadDriverMap();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::TVoid ImportTemplate
        (
                    TDataSrvClient&         dsclLoad
            , const TCQCPackageFl&          pflTmpl
        );

        tCIDLib::TVoid UploadFile
        (
                    TDataSrvClient&         dsclLoad
            , const TCQCPackageFl&          pflLoad
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colDevMap
        //      If it's a template pack, we process the included device map and leave it
        //      around for later processing of the template. It is a list of key/values
        //      objects. The key of each one is the original referenced moniker. The first
        //      value is the make, the second is the model, and the third is a space
        //      separated list of the drivers in this target system of the same make/model.
        //
        //  m_cuctxToUse
        //      The caller has to provide a user context that we can use to access server
        //      side resources.
        //
        //  m_imgToLoad
        //      To avoid creating one of these many times when importing template or
        //      image packs.
        //
        //  m_packImport
        //      Before we even create the dialog we read in teh package and make sure
        //      it can be read in.
        //
        //  m_pwndXXXX
        //      Some typed pointers to our widgets. We don't own them, we just want to
        //      avoid lots of downcasting when we use them.
        //
        //  m_strPackFile
        //      The caller passes us the path to the file, which we also need in order
        //      to pass to the helper method that does the actual work.
        //
        //  m_strmImg
        //  m_strmThumb
        //      A couple output streams to use during image upload.
        // -------------------------------------------------------------------
        tCIDLib::TKValsList     m_colDevMap;
        TCQCUserCtx             m_cuctxToUse;
        TPNGImage               m_imgToLoad;
        TCQCPackage             m_packImport;
        TPushButton*            m_pwndCancel;
        TPushButton*            m_pwndImport;
        TMultiColListBox*       m_pwndList;
        TString                 m_strPackFile;
        TBinMBufOutStream       m_strmImg;
        TBinMBufOutStream       m_strmThumb;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TImportPackDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK

