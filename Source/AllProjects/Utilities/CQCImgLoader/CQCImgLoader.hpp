//
// FILE NAME: CQCImgLoader.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/17/2006
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
//  This is the main header for the facility. It includes anything that is
//  needed by our program.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//

// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CIDXML.hpp"

#include    "CIDImage.hpp"

#include    "CIDPNG.hpp"

#include    "CIDJPEG.hpp"

#include    "CIDGraphDev.hpp"

#include    "CIDCtrls.hpp"

#include    "CQCKit.hpp"

#include    "CQCRemBrws.hpp"

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCImgLoader
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCImgLoader : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCImgLoader();

        ~TFacCQCImgLoader();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented ctors and operators
        // -------------------------------------------------------------------
        TFacCQCImgLoader(const TFacCQCImgLoader&);
        tCIDLib::TVoid operator=(const TFacCQCImgLoader&);


        // -------------------------------------------------------------------
        //  Local types
        // -------------------------------------------------------------------
        enum class ECmds
        {
            GenSysImages
            , ExtractIcons
            , Package
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFindSysImages
        (
            const   TXMLTreeElement&        xtnodeCur
            , const TString&                strSrcRoot
        );

        tCIDLib::TBoolean bRecurseSysImages
        (
            const   TXMLTreeElement&        xtnodeScope
            ,       TPathStr&               pathSrcRoot
            ,       TPathStr&               pathTarRoot
        );

        tCIDLib::TBoolean bParseParms();

        tCIDLib::TVoid CreatePackage();

        tCIDLib::TVoid ExtractIcons();

        tCIDLib::TVoid GenAnImage
        (
            const   TPathStr&               pathSrcRoot
            , const TPathStr&               pathTarRoot
            , const TString&                strName
            , const tCIDLib::TBoolean       bTrans
            , const TPoint&                 pntTransClrAt
        );

        tCIDLib::TVoid GenSystemImages();

        tCIDLib::TVoid RecursiveImgPack
        (
            const   TXMLTreeElement&        xtnodeScope
            ,       TCQCPackage&            packTar
            ,       TPathStr&               pathSrcRoot
            ,       THeapBuf&               mbufImg
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        // -------------------------------------------------------------------
        ECmds           m_eCmd;
        TInConsole      m_conIn;
        TOutConsole     m_conOut;
        TChunkedFile    m_chflImg;
        TPNGImage       m_imgiLoad;
        TPathStr        m_pathInFile;
        TPathStr        m_pathOutFile;
        TPathStr        m_pathBase;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCImgLoader,TFacility)
};


// ---------------------------------------------------------------------------
//  A simple janitor class that lets us expand strings before we recurse,
//  and clip them back to the original length when we get back.
// ---------------------------------------------------------------------------
class TPathExpJan
{
    public :
        TPathExpJan(TPathStr* const pstrExp, const TString& strToAdd) :
            m_c4OrgLen(pstrExp->c4Length())
            , m_pathExp(pstrExp)
        {
            m_pathExp->AddLevel(strToAdd);
        }

        ~TPathExpJan()
        {
            m_pathExp->CapAt(m_c4OrgLen);
        }

        TPathStr& pathExp()
        {
            return *m_pathExp;
        }

        tCIDLib::TCard4 m_c4OrgLen;
        TPathStr*       m_pathExp;
};


// ---------------------------------------------------------------------------
//  Some stuff that needs to be seen throughout this facility
// ---------------------------------------------------------------------------
extern TFacCQCImgLoader facCQCImgLoader;

