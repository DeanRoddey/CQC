//
// FILE NAME: CQCImgLoader.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2006
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
//  This is the main file of the image loader utility. It provides the main
//  program startup code.
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
#include    "CQCImgLoader.hpp"



// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCImgLoader,TFacility)
tCIDLib::EExitCodes eMainThreadFunc
(
        TThread&            thrThis
    ,   tCIDLib::TVoid*     pData
);



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"CQCImgLoaderMainThread"
        , TMemberFunc<TFacCQCImgLoader>(&facCQCImgLoader, &TFacCQCImgLoader::eMainThread)
    )
)


// ----------------------------------------------------------------------------
//  Declare the one global object, our facility object
// ----------------------------------------------------------------------------
TFacCQCImgLoader facCQCImgLoader;




// ---------------------------------------------------------------------------
//   CLASS: TFacCQCImgLoader
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCImgLoader: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we use the same version constants as the toolkit facility. Since
//  they must be used together, this makes sense.
//
TFacCQCImgLoader::TFacCQCImgLoader() :

    TFacility
    (
        L"CQCImgLoader"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::None
    )
{
}

TFacCQCImgLoader::~TFacCQCImgLoader()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCImgLoader: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes
TFacCQCImgLoader::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();
    try
    {
        m_conOut << L"CQC Image Loader\n"
                 << L"Copyright (c) Charmed Quark Systems\n\n"
                 << kCIDLib::FlushIt;

        //
        //  Ask CQCKit to load up environment info, which we'll need for
        //  almost anything.
        //
        TString strFailReason;
        if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
            return tCIDLib::EExitCodes::BadEnvironment;

        // Parse the command line parms
        if (!bParseParms())
        {
            m_conOut << L"\nUsage:\n"
                        L"   CQCImgLoader /Package xmlfile outfile tarscope\n\n"
                        L"   xmlfile  - The description of files to pack\n"
                        L"   outfile  - The package file, without extension\n"
                        L"   tarscope - The root scope at which the packaged files should\n"
                        L"              end up when the user imports the pckage\n"
                     << kCIDLib::EndLn;
            return tCIDLib::EExitCodes::BadParameters;
        }

        switch(m_eCmd)
        {
            case ECmds::GenSysImages :
                GenSystemImages();
                break;

            case ECmds::ExtractIcons :
                break;

            case ECmds::Package :
                CreatePackage();
                break;
        };
    }

    // Catch any CIDLib runtime errors
    catch(const TError& errToCatch)
    {
        m_conOut << L"A CIDLib runtime error occured during processing.\n"
                 << L"Error: " << errToCatch << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::RuntimeError;
    }

    //
    //  Kernel errors should never propogate out of CIDLib, but test for
    //  them here just in case.
    //
    catch(const TKrnlError& kerrToCatch)
    {
        m_conOut << L"A kernel error occured during processing.\nError="
                 << kerrToCatch.errcId() << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::FatalError;
    }

    // Catch a general exception
    catch(...)
    {
        m_conOut << L"A general exception occured during processing"
                 << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::SystemException;
    }
    return tCIDLib::EExitCodes::Normal;
}


// ---------------------------------------------------------------------------
//  TFacCQCImgLoader: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Some of the commands use a basically similar format, that describe the
//  images in a hierarchical format that maps to the repository hierarchy.
//  This method will search the XML data and, given the root of the source
//  direcory of images (also in the same structure under the given root), will
//  make sure all the image exist.
//
tCIDLib::TBoolean
TFacCQCImgLoader::bFindSysImages(const  TXMLTreeElement&    xtnodeCur
                                , const TString&            strSrcRoot)
{
    //
    //  If we have a prefixes attribute, get it out and break it into it's
    //  individual values.
    //
    TVector<TString> colPrefixes;
    tCIDLib::TCard4 c4At;
    if (xtnodeCur.bAttrExists(L"IntfImg:Prefixes", c4At))
    {
        const TString& strPrefixes(xtnodeCur.xtattrAt(c4At).strValue());
        TStringTokenizer stokPref(&strPrefixes, L", ");
        TString strTok;
        while (stokPref.bGetNextToken(strTok))
            colPrefixes.objAdd(strTok);
    }

    //
    //  Iterate the chidren of this node. If we hit a scope, then we recurse
    //  on that node. If we hit images, we process them.
    //
    TString  strTmp;
    TPathStr pathTmp;
    TFindBuf fndbTmp;
    const tCIDLib::TCard4 c4ChildCount = xtnodeCur.c4ChildCount();
    for (tCIDLib::TCard4 c4ChildInd = 0; c4ChildInd < c4ChildCount; c4ChildInd++)
    {
        // Get the next element
        const TXMLTreeElement& xtnodeChild
        (
            xtnodeCur.xtnodeChildAtAsElement(c4ChildInd)
        );

        const TString& strName = xtnodeChild.xtattrNamed(L"IntfImg:Name").strValue();
        if (xtnodeChild.strQName() == L"IntfImg:Scope")
        {
            // Build up the scope at the new level and recurse.
            pathTmp = strSrcRoot;
            pathTmp.AddLevel(strName);
            if (!bFindSysImages(xtnodeChild, pathTmp))
                return kCIDLib::False;
        }
         else
        {
            //
            //  If we have a prefixes list, then we have to loop through
            //  this file for each prefix, else the name is literal and we
            //  do it once. To save code, if the prefix list is empty, just
            //  add an empty string, and then we can use the same code in
            //  both cases.
            //
            if (colPrefixes.bIsEmpty())
                colPrefixes.objAdd(TString::strEmpty());

            const tCIDLib::TCard4 c4PrefCount = colPrefixes.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PrefCount; c4Index++)
            {
                pathTmp = strSrcRoot;
                if (colPrefixes[c4Index].bIsEmpty())
                {
                    pathTmp.AddLevel(strName);
                }
                 else
                {
                    pathTmp.AddLevel(colPrefixes[c4Index]);
                    pathTmp.Append(strName);
                }

                if (!TFileSys::bExists(pathTmp, fndbTmp))
                {
                    m_conOut << L"Could not find source image file '"
                             << pathTmp << L"'. Continue?(Y/N): "
                             << kCIDLib::FlushIt;
                    m_conIn >> strTmp;
                    strTmp.ToUpper();
                    if (strTmp != L"Y")
                        return kCIDLib::False;
                }
                 else
                {
                    fndbTmp.pathFileName().bQueryExt(strTmp);
                    strTmp.ToUpper();
                    if ((strTmp != L"BMP") && (strTmp != L"PNG"))
                    {
                        m_conOut << L"Source image file '" << pathTmp
                                 << L"' is not a BMP or PNG file"
                                 << kCIDLib::EndLn;
                        return kCIDLib::False;
                    }
                }
            }
        }
    }
    return kCIDLib::True;
}


//
//  Called from the startup to get the pareameters. The parameters are
//  used differently according to the command being invoked.
//
tCIDLib::TBoolean TFacCQCImgLoader::bParseParms()
{
    TString strVal;
    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    for (; cursParms; ++cursParms)
    {
        strVal = *cursParms;
        if (strVal.bStartsWithI(L"/Cmd="))
        {
            strVal.Cut(0, 5);

            if (strVal == L"GenSysImages")
                m_eCmd = ECmds::GenSysImages;
            else if (strVal == L"ExtractIcons")
                m_eCmd = ECmds::ExtractIcons;
            else if (strVal == L"Package")
                m_eCmd = ECmds::Package;
            else
            {
                m_conOut << L"'" << strVal << L"' is an unknown command"
                         << kCIDLib::EndLn;
                return kCIDLib::False;
            }
        }
         else if (strVal.bStartsWithI(L"/InFile="))
        {
            strVal.Cut(0, 8);
            m_pathInFile = strVal;
        }
         else if (strVal.bStartsWithI(L"/OutFile="))
        {
            strVal.Cut(0, 9);
            m_pathOutFile = strVal;
            m_pathOutFile.bRemoveTrailingSeparator();
        }
         else
        {
            m_conOut << L"'" << strVal << L"' is not a valid parameter"
                     << kCIDLib::EndLn;
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}



