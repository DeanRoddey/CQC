//
// FILE NAME: CQCSysCfg_Parts.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/02/2014
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
//  This file defines some bits and pieces that make up the configuration for rooms
//  or the overall config list. These are simple classes that need very little
//  actual code. Mostly they are just for streaming purposes.
//
//  However we do need to support polymorphic copying, so we define a CopyFrom
//  method. This allows us to have single methods in the layout class that can copy
//  and assign these objects as required.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TSCfgPart
//  PREFIX: scli
// ---------------------------------------------------------------------------
class CQCSYSCFGEXP TSysCfgListItem

    : public TObject, public MStreamable, public MDuplicable
{
    public :
        static tCIDLib::ESortComps eCompByTitle
        (
            const   TSysCfgListItem&        scliL
            , const TSysCfgListItem&        scliR
        );

        ~TSysCfgListItem();

        TSysCfgListItem& operator=(const TSysCfgListItem&) = default;
        TSysCfgListItem& operator=(TSysCfgListItem&&) = default;

        tCIDLib::TBoolean operator==(const TSysCfgListItem& scliSrc) const
        {
            return bCompare(scliSrc);
        }
        tCIDLib::TBoolean operator!=(const TSysCfgListItem& scliSrc) const
        {
            return !bCompare(scliSrc);
        }

        virtual tCIDLib::TBoolean bCompare(const TSysCfgListItem& scliSrc) const;
        virtual tCIDLib::TVoid CopyFrom(const TSysCfgListItem& scliSrc);
        virtual tCIDLib::TVoid Reset();

        TString         m_strAltName;
        TString         m_strTitle;
        TString         m_strUniqueId;

    protected :
        TSysCfgListItem();
        TSysCfgListItem(const TSysCfgListItem& scliSrc) = default;
        TSysCfgListItem(TSysCfgListItem&&) = default;

        tCIDLib::TVoid StreamFrom(TBinInStream& strmTar) override;
        tCIDLib::TVoid StreamTo(TBinOutStream& strmTar) const override;

    private :
        RTTIDefs(TSysCfgListItem,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TSCGlobActInfo
//  PREFIX: scli
//
//  A simple class to hold information about global actions that each room can have.
//  It's used both for a fixed set of actions, and to support a set of 'room modes'
//  that the user can set, each of which is just the invocation of a global action.
//
//  In the latter case the enabled flag isn't used. If it's defined, it's enabled.
// ---------------------------------------------------------------------------
class CQCSYSCFGEXP TSCGlobActInfo : public TSysCfgListItem
{
    public :
        static tCIDLib::TBoolean bComp
        (
            const TSCGlobActInfo&, const TSCGlobActInfo&
        );

        TSCGlobActInfo();
        TSCGlobActInfo(const TSCGlobActInfo&) = default;
        TSCGlobActInfo(TSCGlobActInfo&&) = default;
        ~TSCGlobActInfo();

        tCIDLib::TBoolean operator==(const TSCGlobActInfo& scliSrc) const
        {
            return bCompare(scliSrc);
        }
        TSCGlobActInfo& operator=(const TSCGlobActInfo&) = default;
        TSCGlobActInfo& operator=(TSCGlobActInfo&&) = default;
        tCIDLib::TBoolean bCompare(const TSysCfgListItem& scliSrc) const override;
        tCIDLib::TVoid CopyFrom(const TSysCfgListItem& scliSrc) override;
        tCIDLib::TVoid Reset() override;

        tCIDLib::TBoolean   m_bEnabled;
        tCIDLib::TStrArray  m_colParms;
        TString             m_strPath;


    protected :
        tCIDLib::TVoid StreamFrom(TBinInStream& strmTar) final;
        tCIDLib::TVoid StreamTo(TBinOutStream& strmTar) const final;

    private :
        RTTIDefs(TSCGlobActInfo,TSysCfgListItem)
        DefPolyDup(TSCGlobActInfo);
};



// ---------------------------------------------------------------------------
//   CLASS: TSCHVACInfo
//  PREFIX: seci
// ---------------------------------------------------------------------------
class CQCSYSCFGEXP TSCHVACInfo : public TSysCfgListItem
{
    public :
        TSCHVACInfo();
        TSCHVACInfo(const TSCHVACInfo&) = default;
        TSCHVACInfo(TSCHVACInfo&&) = default;
        ~TSCHVACInfo();

        TSCHVACInfo& operator=(const TSCHVACInfo&) = default;
        TSCHVACInfo& operator=(TSCHVACInfo&&) = default;

        tCIDLib::TBoolean operator==(const TSCHVACInfo& scliSrc) const
        {
            return bCompare(scliSrc);
        }
        tCIDLib::TBoolean bCompare(const TSysCfgListItem& scliSrc) const override;
        tCIDLib::TVoid CopyFrom(const TSysCfgListItem& scliSrc) override;
        tCIDLib::TVoid Reset() override;

        TString m_strMoniker;
        TString m_strThermoSub;


    protected :
        tCIDLib::TVoid StreamFrom(TBinInStream& strmTar) final;
        tCIDLib::TVoid StreamTo(TBinOutStream& strmTar) const final;

    private :
        RTTIDefs(TSCHVACInfo,TSysCfgListItem)
        DefPolyDup(TSCHVACInfo);
};




// ---------------------------------------------------------------------------
//   CLASS: TSCLoadInfo
//  PREFIX: li
// ---------------------------------------------------------------------------
class CQCSYSCFGEXP TSCLoadInfo : public TSysCfgListItem
{
    public :
        TSCLoadInfo();
        TSCLoadInfo(const TSCLoadInfo&) = default;
        TSCLoadInfo(TSCLoadInfo&&) = default;
        ~TSCLoadInfo();

        TSCLoadInfo& operator=(const TSCLoadInfo&) = default;
        TSCLoadInfo& operator=(TSCLoadInfo&&) = default;

        tCIDLib::TBoolean operator==(const TSCLoadInfo& scliSrc) const
        {
            return bCompare(scliSrc);
        }
        tCIDLib::TBoolean bCompare(const TSysCfgListItem& scliSrc) const override;
        tCIDLib::TVoid CopyFrom(const TSysCfgListItem& scliSrc) override;
        tCIDLib::TVoid Reset() override;

        tCIDLib::TBoolean bIsDimmer() const;

        TString m_strClrField;
        TString m_strMoniker;
        TString m_strField;
        TString m_strField2;

    protected :
        tCIDLib::TVoid StreamFrom(TBinInStream& strmTar) final;
        tCIDLib::TVoid StreamTo(TBinOutStream& strmTar) const final;

    private :
        RTTIDefs(TSCLoadInfo,TSysCfgListItem)
        DefPolyDup(TSCLoadInfo);
};


// ---------------------------------------------------------------------------
//   CLASS: TSCMediaInfo
//  PREFIX: mi
// ---------------------------------------------------------------------------
class CQCSYSCFGEXP TSCMediaInfo : public TSysCfgListItem
{
    public :
        TSCMediaInfo();
        TSCMediaInfo(const TSCMediaInfo&) = default;
        TSCMediaInfo(TSCMediaInfo&&) = default;
        ~TSCMediaInfo();

        TSCMediaInfo& operator=(const TSCMediaInfo&) = default;
        TSCMediaInfo& operator=(TSCMediaInfo&&) = default;

        tCIDLib::TBoolean operator==(const TSysCfgListItem& scliSrc) const
        {
            return bCompare(scliSrc);
        }
        tCIDLib::TBoolean bCompare(const TSysCfgListItem& scliSrc) const override;
        tCIDLib::TVoid CopyFrom(const TSysCfgListItem& scliSrc) override;
        tCIDLib::TVoid Reset() override;

        TString m_strAudioMon;
        TString m_strAudioSub;
        TString m_strRendMon;
        TString m_strRepoMon;

    protected :
        tCIDLib::TVoid StreamFrom(TBinInStream& strmTar) final;
        tCIDLib::TVoid StreamTo(TBinOutStream& strmTar) const final;

    private :
        RTTIDefs(TSCMediaInfo,TSysCfgListItem)
        DefPolyDup(TSCMediaInfo);
};



// ---------------------------------------------------------------------------
//   CLASS: TSCSecInfo
//  PREFIX: seci
// ---------------------------------------------------------------------------
class CQCSYSCFGEXP TSCSecInfo : public TSysCfgListItem
{
    public :
        TSCSecInfo();
        TSCSecInfo(const TSCSecInfo&) = default;
        TSCSecInfo(TSCSecInfo&&) = default;
        ~TSCSecInfo();

        TSCSecInfo& operator=(const TSCSecInfo&) = default;
        TSCSecInfo& operator=(TSCSecInfo&&) = default;

        tCIDLib::TBoolean operator==(const TSCSecInfo& scliSrc) const
        {
            return bCompare(scliSrc);
        }
        tCIDLib::TBoolean bCompare(const TSysCfgListItem& scliSrc) const override;
        tCIDLib::TVoid CopyFrom(const TSysCfgListItem& scliSrc) override;
        tCIDLib::TVoid Reset() override;

        TString m_strAreaName;
        TString m_strArmingCode;
        TString m_strMoniker;


    protected :
        tCIDLib::TVoid StreamFrom(TBinInStream& strmTar) final;
        tCIDLib::TVoid StreamTo(TBinOutStream& strmTar) const final;

    private :
        RTTIDefs(TSCSecInfo,TSysCfgListItem)
        DefPolyDup(TSCSecInfo);
};



// ---------------------------------------------------------------------------
//   CLASS: TSCWeatherInfo
//  PREFIX: seci
// ---------------------------------------------------------------------------
class CQCSYSCFGEXP TSCWeatherInfo : public TSysCfgListItem
{
    public :
        TSCWeatherInfo();
        TSCWeatherInfo(const TSCWeatherInfo&) = default;
        TSCWeatherInfo(TSCWeatherInfo&&) = default;
        ~TSCWeatherInfo();

        TSCWeatherInfo& operator=(const TSCWeatherInfo&) = default;
        TSCWeatherInfo& operator=(TSCWeatherInfo&&) = default;
        tCIDLib::TBoolean operator==(const TSCWeatherInfo& scliSrc) const
        {
            return bCompare(scliSrc);
        }
        tCIDLib::TBoolean bCompare(const TSysCfgListItem& scliSrc) const override;
        tCIDLib::TVoid CopyFrom(const TSysCfgListItem& scliSrc) override;
        tCIDLib::TVoid Reset() override;

        tCIDLib::TBoolean   m_bFScale;
        TString             m_strMoniker;


    protected :
        tCIDLib::TVoid StreamFrom(TBinInStream& strmTar) final;
        tCIDLib::TVoid StreamTo(TBinOutStream& strmTar) const final;

    private :
        RTTIDefs(TSCWeatherInfo,TSysCfgListItem)
        DefPolyDup(TSCWeatherInfo);
};



// ---------------------------------------------------------------------------
//   CLASS: TSCXOverInfo
//  PREFIX: seci
// ---------------------------------------------------------------------------
class CQCSYSCFGEXP TSCXOverInfo : public TSysCfgListItem
{
    public :
        TSCXOverInfo();
        TSCXOverInfo(const TSCXOverInfo&) = default;
        TSCXOverInfo(TSCXOverInfo&&) = default;
        ~TSCXOverInfo();

        TSCXOverInfo& operator=(const TSCXOverInfo&) = default;
        TSCXOverInfo& operator=(TSCXOverInfo&&) = default;
        tCIDLib::TBoolean operator==(const TSCXOverInfo& scliSrc) const
        {
            return bCompare(scliSrc);
        }
        tCIDLib::TBoolean bCompare(const TSysCfgListItem& scliSrc) const override;
        tCIDLib::TVoid CopyFrom(const TSysCfgListItem& scliSrc) override;
        tCIDLib::TVoid Reset() override;

        TString     m_strContentTmpl;
        TString     m_strTileTmpl;

    protected :
        tCIDLib::TVoid StreamFrom(TBinInStream& strmTar) final;
        tCIDLib::TVoid StreamTo(TBinOutStream& strmTar) const final;

    private :
        RTTIDefs(TSCXOverInfo,TSysCfgListItem)
        DefPolyDup(TSCXOverInfo);
};

#pragma CIDLIB_POPPACK

