//
// FILE NAME: ElkM1V2C_ItemInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/06/2014
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
//  This is the header for the ElkM1V2C_ItemInfo.cpp file, which implements
//  the base class from which a set of classes are derived. These hold the config
//  data for each of the types of things we can configure.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TItemInfo
//  PREFIX: ii
// ---------------------------------------------------------------------------
class TItemInfo : public TObject, public MDuplicable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eSortByNum
        (
            const   TItemInfo&              ii1
            , const TItemInfo&              ii2
        );


        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        virtual ~TItemInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TBoolean operator==
        (
            const   TItemInfo&              iiToComp
        )   const;

        tCIDLib::TBoolean operator!=(const TItemInfo& iiToComp) const
        {
            return !operator==(iiToComp);
        }


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bComp
        (
            const   TItemInfo&              iiToComp
        )   const = 0;

        virtual tCIDLib::TVoid FormatCfg
        (
                    TTextOutStream&         strmTar
        )   const;

        virtual tCIDLib::TVoid FormatSettings
        (
                    TTextOutStream&         strmTar
            , const TStreamFmt&             sfmtName
            , const TStreamFmt&             sfmtValue
        )   const;

        virtual tCIDLib::TVoid ParseCfg
        (
            const   tCIDLib::TCard4         c4Id
            ,       tCIDLib::TStrList&      colTokens
        );


        // -------------------------------------------------------------------
        //  Public class members
        //
        //  m_c4ElkId
        //      The Elk item number for this item.
        //
        //  m_eType
        //      The derived class tells us the type, so that we can do some stuff
        //      on their behalf.
        //
        //  m_strName
        //      The user configured name for the item.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4ElkId;
        tElkM1V2C::EItemTypes   m_eType;
        TString                 m_strName;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TItemInfo
        (
            const   tElkM1V2C::EItemTypes   eType
        );
        TItemInfo(const TItemInfo&);
        tCIDLib::TVoid operator=(const TItemInfo&);


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TItemInfo,TObject)
};

#pragma CIDLIB_POPPACK


