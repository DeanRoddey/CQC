//
// FILE NAME: MediaRepoMgr_RepoSettings.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/11/2006
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
//  This is the header for MediaRepoMgr_RepoSettings.cpp file, which defines a class
//  that we persiste to maintain repository options that the user can choose. It is
//  stores in the master server config repository.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TMediaRepoSettings
//  PREFIX: mrs
// ---------------------------------------------------------------------------
class TMediaRepoSettings : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMediaRepoSettings();

        TMediaRepoSettings
        (
            const   TMediaRepoSettings&     mrsSrc
        );

        ~TMediaRepoSettings();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMediaRepoSettings& operator=
        (
            const   TMediaRepoSettings&     mrsSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual method
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bJitterCorrection() const;

        tCIDLib::TBoolean bJitterCorrection
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bRipEject() const;

        tCIDLib::TBoolean bRipEject
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bShowAdult() const;

        tCIDLib::TBoolean bShowAdult
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bShowEnglish() const;

        tCIDLib::TBoolean bShowEnglish
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bUseMyMovies() const;

        tCIDLib::TBoolean bUseMyMovies
        (
            const   tCIDLib::TBoolean       bToSet
        );

        const TString& strCodec() const;

        const TString& strCodec
        (
            const   TString&                strToSet
        );

        const TString& strCodecFmt() const;

        const TString& strCodecFmt
        (
            const   TString&                strToSet
        );

        const TString& strEncoder() const;

        const TString& strEncoder
        (
            const   TString&                strToSet
        );

        const TString& strMMName() const;

        const TString& strMMName
        (
            const   TString&                strToSet
        );

        const TString& strMMPassword() const;

        const TString& strMMPassword
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid Reset();


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
        //  Private data members
        //
        //  m_bJitterCorrection
        //      The current default jitter correction option. This is stored
        //      in the master server config repository and reloaded again next
        //      time so that we always present the last set rip mode.
        //
        //  m_bRipEject
        //      Indicates whether we should issue an eject command after
        //      ripping and upload is completed.
        //
        //  m_bShowAdult
        //  m_bShowEnglish
        //      Where metadata sources have options to show adult titles or
        //      to optionally leave out English titles, these will be used.
        //
        //  m_bUseMyMovies
        //      They can optionally configure us with username/password for the
        //      My Movies web service and can use that to look up movie metadata.
        //
        //  m_strCodec
        //  m_strCodecFmt
        //      The current default output format. This is stored in the
        //      master server config repository and is used by all repository
        //      managers. We store a codec name and a codec format name which
        //      are passed back in to the encoder and it uses this info to
        //      do the right thing. We don't know what it means, it's just
        //      selected by the user (based on info provided by the encoder)
        //      and we pass them back in to it again later.
        //
        //  m_strEncoder
        //      The name of the encoder to use. The use selects an encoder
        //      and we look up an encoder of that type to use when we need
        //      to rip.
        //
        //      Note that, for now, we just support WMA so we are hard coded
        //      to use that, but we go ahead and store the WM encoder name
        //      for now so that it's there and we won't require a data upgrade
        //      when the time codes to support others.
        //
        //  m_strMMName
        //  m_strMMPassword
        //      If m_bUseMyMovies is set, then this is the name and password
        //      we should send in our queries.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bJitterCorrection;
        tCIDLib::TBoolean   m_bRipEject;
        tCIDLib::TBoolean   m_bShowAdult;
        tCIDLib::TBoolean   m_bShowEnglish;
        tCIDLib::TBoolean   m_bUseMyMovies;
        TString             m_strCodec;
        TString             m_strCodecFmt;
        TString             m_strEncoder;
        TString             m_strMMName;
        TString             m_strMMPassword;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMediaRepoSettings,TObject)
};


#pragma CIDLIB_POPPACK
