//
// FILE NAME: CQCKit_Package.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/05/2004
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
//  This class manages 'packages', which is a way to package up driver files or bundles
//  of macro files, so that they can be delivered separately from the official release,
//  and easily imported into the user's CQC system.
//
//  So we have a class that represents such a package overall, a helper class that
//  represents a single file, so that we have a convenient way to store info about the
//  various files that go into the package.
//
//  If it is a driver type package, the driver manifest file must be the first file added.
//
//  Initially packages were given a minimum and maximum CQC version they would work with,
//  but this was silly and it was changed so that the package is stamped with the version
//  of CQC it was created with and each version of CQC just makes sure it can import that
//  version of a package or rejects it.
//
//
//  AS OF 5.3.903, we completeyl changed how these types of files are stored, moving
//  from an ad hoc scheme to one based on a new TChunkedFile format. This is far more
//  flexible and consistent.
//
//  But, all the same types of files are still needed. So, we just added a bIsChunked
//  flag to the package file class. So the type plus the is chunked flag indicates
//  how to read in a package file. All that matters if reading. All newly created
//  packages from 5.3.903 forward will be written in the new format.
//
//  Any previously existing packages will, upon being read in, have this flag defaulted
//  to false.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCPackageFl
//  PREFIX: pfl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCPackageFl : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  We can take files in two forms. One is a raw binary which represents the
        //  file as read from the disk. And the other is a text file that has been
        //  read into a TString object, which will ultimately get flattened to UTF-8
        //  when the string is streamed out.
        // -------------------------------------------------------------------
        enum class EFmtTypes
        {
            Binary
            , Text
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCPackageFl
        (
            const   tCIDLib::TBoolean       bCompress = kCIDLib::True
        );

        TCQCPackageFl
        (
            const   tCQCKit::EPackFlTypes   eType
            , const TString&                strFileName
            , const TChunkedFile&           chflData
            , const tCIDLib::TBoolean       bCompress
            , const tCIDLib::TBoolean       bEncrypt
        );

        TCQCPackageFl
        (
            const   tCQCKit::EPackFlTypes   eType
            , const TString&                strFileName
            , const TString&                strSource
            , const tCIDLib::TBoolean       bCompress
            , const tCIDLib::TBoolean       bEncrypt
        );

        TCQCPackageFl
        (
            const   tCQCKit::EPackFlTypes   eType
            , const TString&                strFileName
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Bytes
            , const tCIDLib::TBoolean       bCompress
            , const tCIDLib::TBoolean       bEncrypt
        );

        TCQCPackageFl
        (
            const   TCQCPackageFl&          pflSrc
        );

        TCQCPackageFl
        (
                    TCQCPackageFl&&         pflSrc
        );

        ~TCQCPackageFl();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCPackageFl& operator=
        (
            const   TCQCPackageFl&          pflSrc
        );

        TCQCPackageFl& operator=
        (
                    TCQCPackageFl&&         pflSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bIsChunked() const
        {
            return m_bIsChunked;
        }

        [[nodiscard]] tCIDLib::TBoolean bCompress() const;

        tCIDLib::TBoolean bCompress
        (
            const   tCIDLib::TBoolean       bToSet
        );

        [[nodiscard]] tCIDLib::TBoolean bEncrypt() const;

        tCIDLib::TBoolean bEncrypt
        (
            const   tCIDLib::TBoolean       bToSet
        );

        [[nodiscard]] tCIDLib::TCard4 c4Size() const
        {
            return m_c4Size;
        }

        [[nodiscard]] tCQCKit::EPackFlTypes eFileType() const
        {
            return m_eFileType;
        }

        [[nodiscard]] EFmtTypes eFmtType() const
        {
            return m_eFmtType;
        }

        [[nodiscard]] const THeapBuf& mbufData() const;

        [[nodiscard]] const TString& strName() const
        {
            return m_strName;
        }

        [[nodiscard]] const TString& strUserData() const;

        const TString& strUserData
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetData
        (
            const   TString&                strFileName
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4DataSz
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
        //  m_bCompress
        //      We can be told if the file would benefit from compression, and if so will
        //      compress it when it is streamed out. For instance, a PNG file or JPEG won't
        //      compress hardly at all so it isn't worth doing.
        //
        //  m_bEncrypt
        //      This is used for macros when the user asks to create an encrypted driver
        //      pack. We set this on each file so that when it's imported, he will ask the
        //      data server to save it enrypted.
        //
        //  m_bIsChunked
        //      See file comments. This is true on any packages created from 5.3.903
        //      and forward, and false for any packages created before that. It's not
        //      even persisted. We just bumped the persistent format version for this
        //      class, and we know if it was any version before that it's not chunked
        //      and any version after that it is, so this is set when the package is
        //      read in, and defaulted to true in the ctors since any new packages
        //      should be in the new format.
        //
        //  m_c4Size
        //      The size of the file data in m_mbufData.
        //
        //  m_eFileType
        //      The type of file this is. We don't store any extensions, to make display
        //      easier, and just keep up with the type of file in this way.
        //
        //  m_eFmtType
        //      Indicates how the data was provided to us. The caller has to get it out the
        //      same way it was put in.
        //
        //  m_mbufData
        //      For any new (post 5.3.900 or beyond) packages this is the flattened
        //      TChunkedFile object. For ones created in earlier versions it is the old
        //      type specific formats. The client code will have to convert, which it can
        //      do using methods in the CQCRemBrws facility. Check the bIsChunked flag.
        //
        //  m_strName
        //      The name of the file. For manifests and protocol files it is just the base
        //      name and extension. For data server based stuff, it's the type relative
        //      path. The m_eFileType member tells us what type of file it is.
        //
        //  m_strUserData
        //      This allows the client code that's packing/unpacking the file to put some
        //      sort of marker info into the package. For instance, with drivers, this is
        //      used to indicate whether it is a System or User driver.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bCompress;
        tCIDLib::TBoolean       m_bEncrypt;
        tCIDLib::TBoolean       m_bIsChunked;
        tCIDLib::TCard4         m_c4Size;
        tCQCKit::EPackFlTypes   m_eFileType;
        EFmtTypes               m_eFmtType;
        THeapBuf                m_mbufData;
        TString                 m_strName;
        TString                 m_strUserData;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCPackageFl,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCPackage
//  PREFIX: pack
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCPackage : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TVoid LoadPackage
        (
            const   TString&                strPath
            ,       TCQCPackage&            packToFill
        );

        static tCIDLib::TVoid MakePackage
        (
            const   TString&                strPath
            , const TCQCPackage&            packSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCPackage();

        TCQCPackage
        (
            const   tCQCKit::EPackTypes     eType
            , const tCIDLib::TCard8         c8CreateVer
        );

        TCQCPackage
        (
            const   TCQCPackage&            packSrc
        );

        TCQCPackage
        (
                    TCQCPackage&&           packSrc
        );

        ~TCQCPackage();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCPackage& operator=
        (
            const   TCQCPackage&            packSrc
        );

        TCQCPackage& operator=
        (
                    TCQCPackage&&           packSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanDelete
        (
            const   tCIDLib::TCard4         c4AtIndex
        )   const;

        [[nodiscard]] tCIDLib::TBoolean bFileIsInPackage
        (
            const   TString&                strToFind
            , const tCQCKit::EPackFlTypes   eType
            , COP   tCIDLib::TCard4&        c4AtIndex
        )   const;

        [[nodiscard]] tCIDLib::TBoolean bIsDriverType() const;

        tCIDLib::TBoolean bRemoveFile
        (
            const   TString&                strFileName
        );

        tCIDLib::TCard4 c4AddFile
        (
            const   tCQCKit::EPackFlTypes   eType
            , const TString&                strFileName
            , const TChunkedFile&           chflToAdd
            , const tCIDLib::TBoolean       bCompress
            , const tCIDLib::TBoolean       bEncrypt
        );

        tCIDLib::TCard4 c4AddFile
        (
            const   tCQCKit::EPackFlTypes   eType
            , const TString&                strFileName
            , const TString&                strText
            , const tCIDLib::TBoolean       bEncrypt
        );

        tCIDLib::TCard4 c4AddFile
        (
            const   tCQCKit::EPackFlTypes   eType
            , const TString&                strFileName
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Bytes
            , const tCIDLib::TBoolean       bCompress
            , const tCIDLib::TBoolean       bEncrypt
        );

        [[nodiscard]] tCIDLib::TCard4 c4FileCount() const;

        [[nodiscard]] tCIDLib::TCard8 c8CreateVer() const;

        const TCQCPackageFl& pflAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        [[nodiscard]] tCQCKit::EPackTypes eType() const;

        tCIDLib::TVoid Initialize
        (
            const   tCQCKit::EPackTypes     eType
            , const tCIDLib::TCard8         c8CreateVer
        );

        tCIDLib::TVoid RemoveFileAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TString& strUserData() const;

        const TString& strUserData
        (
            const   TString&                strToSet
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
        //  m_c8CreateVer
        //      The version of CQC under which this package was created. This
        //      is used to know how to interpret it as the package format
        //      evolves.
        //
        //  m_colFiles
        //      All the files in this package.
        //
        //  m_eType
        //      The type of package. It indicates whether it's a macro driver,
        //      protocol driver, macro bundle, image bundle, interface
        //      template, etc...
        //
        //  m_strUserData
        //      This allows the client code that's packing/unpacking the file
        //      to put some sort of marker info into the package. For instance,
        //      with drivers, this is used to indicate whether it is a System
        //      or User driver.
        // -------------------------------------------------------------------
        tCIDLib::TCard8         m_c8CreateVer;
        TVector<TCQCPackageFl>  m_colFiles;
        tCQCKit::EPackTypes     m_eType;
        TString                 m_strUserData;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCPackage,TObject)
};

#pragma CIDLIB_POPPACK


