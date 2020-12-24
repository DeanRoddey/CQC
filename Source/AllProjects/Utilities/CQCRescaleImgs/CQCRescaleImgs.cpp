//
// FILE NAME: CQCRescaleImgs.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/02/2005
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
//  This is the main module for this program. This one is very simple and
//  just has a single cpp file that contains everything. This program is
//  a utility that is used to rescale images in a media repository.
//
//  In many cases, the images are far larger than they need to be, and this
//  makes cover art browsing sluggish. This program can be pointed at a
//  directory and it will run through the directory hierarchy from that point
//  down and will find any bmp, PNG, or JPEG images and will rescale any
//  of them that are larger than a given size, and spit them back out. For
//  JPEGS it will use fairly high compression to lower the size. For PNGs it
//  will use the most agreessive filtering for the same reason.
//
//  The options are:
//
//      CQCRecaleImgs path [options]
//
//          /MaxH=xx    - The max horz size, beyond which scale down is done
//          /MaxV=xx    - The max vert size, beyond which scale down is done
//          /Recurse    - Do target dir and recurse through child dirs
//
//  If any image is larger than either of the indicated h/v values, it will
//  be scaled, keeping the aspect ratio. If it is larger in both dimensions,
//  the larger axis is scaled, with the other adjusted for AR.
//
//  Defaults are 192x192 and no recursion.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CIDJPEG.hpp"

#include    "CIDPNG.hpp"

#include    "CQCMedia.hpp"



// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc
(
        TThread&            thrThis
        , tCIDLib::TVoid*   pData
);


// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
CIDLib_MainModule(TThread(L"CQCRescaleImgsMainThread", eMainThreadFunc))


// ---------------------------------------------------------------------------
//  Local data
//
//  imgXXX
//      Some image objects that we use to stream in/out the image data. We
//      don't want to create these large objects over and over so we make
//      them global.
// ---------------------------------------------------------------------------
TBitmapImage    imgBMP;
TJPEGImage      imgJPEG;
TPNGImage       imgPNG;
TMediaImg       mimgCQSL;
THeapBuf        mbufTmp(64 * 1024UL);


// ---------------------------------------------------------------------------
//  Local functions
// ---------------------------------------------------------------------------

//
//  Shows the parameter usage for the program.
//
static tCIDLib::TVoid ShowUsage()
{
    TSysInfo::strmOut()
            << L"Usage: CQCRescaleImgs path [/MaxH=xx /MaxV=xx /Recurse]\n\n"
            << L"       If an image is larger in either H or V directions than\n"
            << L"       the indicated sizes (defaults to 192 for both), then\n"
            << L"       the image is scaled down so that the offending axis\n"
            << L"       fits. If both are over, then the larger one is scaled\n"
            << L"       down. In either case, the aspect ratio is retained.\n\n"
            << L"       If the images are not in one directory, the /Recurse\n"
            << L"       option causes the whole tree under the starting path\n"
            << L"       are scanned. Any BMP, JPEG, JPG, or PNG file is checked.\n"
            << kCIDLib::EndLn;
}


static tCIDLib::TBoolean bParseParms(tCIDLib::TBoolean& bRecurse
                                    , tCIDLib::TCard4&  c4MaxH
                                    , tCIDLib::TCard4&  c4MaxV
                                    , TString&          strStartPath)
{
    // Check out the command line parms for valid count
    tCIDLib::TCard4 c4Args = TSysInfo::c4CmdLineParmCount();
    if ((c4Args < 1) || (c4Args > 4))
        return kCIDLib::False;
    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();

    // The first one should be the path
    TFileSys::QueryFullPath(*cursParms++, strStartPath);
    if (!TFileSys::bIsDirectory(strStartPath))
    {
        TSysInfo::strmOut() << L"The starting path was not found\n\n"
                            << kCIDLib::FlushIt;
        return kCIDLib::False;
    }

    // If there are any more parms, check them out
    tCIDLib::TBoolean   bOk;
    TString strCur;
    for (; cursParms; ++cursParms)
    {
        strCur = *cursParms;
        if (strCur.bStartsWithI(L"/MaxH="))
        {
            strCur.Cut(0, 6);
            c4MaxH = TRawStr::c4AsBinary(strCur.pszBuffer(), bOk, tCIDLib::ERadices::Dec);
            if (!bOk || (c4MaxH < 64) || (c4MaxH > 1024))
            {
                TSysInfo::strmOut() << L"MaxH must be from 64 to 1024\n\n"
                                    << kCIDLib::FlushIt;
                return kCIDLib::False;
            }
        }
         else if (strCur.bStartsWithI(L"/MaxV="))
        {
            strCur.Cut(0, 6);
            c4MaxV = TRawStr::c4AsBinary(strCur.pszBuffer(), bOk, tCIDLib::ERadices::Dec);
            if (!bOk || (c4MaxV < 64) || (c4MaxV > 1024))
            {
                TSysInfo::strmOut() << L"MaxH must be from 64 to 1024\n\n"
                                    << kCIDLib::FlushIt;
                return kCIDLib::False;
            }
        }
         else if (strCur.bCompareI(L"/Recurse"))
        {
            bRecurse = kCIDLib::True;
        }
         else
        {
            TSysInfo::strmOut() << L"'" << strCur
                                << L"' is not a valid parameter\n\n"
                                << kCIDLib::FlushIt;
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


static tCIDLib::TVoid DoAFile(  const   TString&        strPath
                                , const TString&        strExt
                                , const tCIDLib::TCard4 c4MaxH
                                , const tCIDLib::TCard4 c4MaxV)
{
    try
    {
        tCIDLib::TCard4 c4OrgSz;

        // Open a binary input file stream over this file
        TCIDImage* pimgTar = 0;
        {
            TBinFileInStream strmSrc
            (
                strPath
                , tCIDLib::ECreateActs::OpenIfExists
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );
            c4OrgSz = tCIDLib::TCard4(strmSrc.c8CurSize());

            if (strExt == L"BMP")
            {
                strmSrc >> imgBMP;
                pimgTar = &imgBMP;
            }
            else if ((strExt == L"JPEG")
                 ||   (strExt == L"JPG")
                 ||  (strExt == L"MIMG"))
            {
                strmSrc >> imgJPEG;
                pimgTar = &imgJPEG;
            }
            else if (strExt == L"PNG")
            {
                strmSrc >> imgPNG;
                pimgTar = &imgPNG;
            }
        }

        if (pimgTar
        &&  ((pimgTar->c4Width() > c4MaxH)
        ||   (pimgTar->c4Height() > c4MaxV)))
        {
            // We need to scale it
            TSize szNew(pimgTar->szImage());

            if ((pimgTar->c4Width() > c4MaxH) && (pimgTar->c4Height() > c4MaxV))
            {
                // It's bigger in both directions, so take the larger one
                if (pimgTar->c4Width() > pimgTar->c4Height())
                    szNew.ScaleToWidthAR(c4MaxH);
                else
                    szNew.ScaleToHeightAR(c4MaxV);
            }
             else if (pimgTar->c4Width() > c4MaxH)
            {
                szNew.ScaleToWidthAR(c4MaxH);
            }
             else
            {
                szNew.ScaleToHeightAR(c4MaxV);
            }
            pimgTar->Scale(szNew, 2);

            //
            //  Ok, now we can now stream it back to a temp file. We'll
            //  rename it back to the original if all goes well.
            //
            tCIDLib::TCard4 c4OutBytes;
            TPathStr pathOut(strPath);
            pathOut.Append(L"_New");
            {
                TBinFileOutStream strmOut
                (
                    pathOut
                    , tCIDLib::ECreateActs::CreateAlways
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                );

                strmOut << *pimgTar << kCIDLib::FlushIt;
                c4OutBytes = strmOut.c4CurPos();
            }

            //
            //  Ok, so let's delete the old one and rename the new one to
            //  the old name.
            //
            TFileSys::DeleteFile(strPath);
            TFileSys::Rename(pathOut, strPath);

            //
            //  Display the percent we reduced the file and the path of
            //  the file we just did. We cut off the leading part if it
            //  is more than 60 chars long, so that they don't wrap.
            //
            const tCIDLib::TCard4 c4Per = 100 - tCIDLib::TCard4
            (
                tCIDLib::TFloat8(c4OutBytes) / tCIDLib::TFloat8(c4OrgSz)
                * 100
            );

            const tCIDLib::TCard4 c4OrgLen = strPath.c4Length();
            pathOut.CapAt(c4OrgLen);
            if (c4OrgLen > 60)
            {
                pathOut.Cut(0, c4OrgLen - 60);
                pathOut.Prepend(L"...");
            }
            TSysInfo::strmOut() << L"(Reduced ";
            if (c4Per < 10)
                TSysInfo::strmOut() << L"0";
            TSysInfo::strmOut() << c4Per;
            TSysInfo::strmOut() << L"%) " << pathOut << kCIDLib::EndLn;
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TSysInfo::strmOut() << L"Failed to convert file: " << strPath
                            << kCIDLib::NewLn;
    }
}


static tCIDLib::TVoid DoADir(const  TString&            strCurPath
                            , const tCIDLib::TCard4     c4MaxH
                            , const tCIDLib::TCard4     c4MaxV
                            , const tCIDLib::TBoolean   bRecurse)
{
    // If no files at this level, then we are done
    TDirIter diterLevel;
    TFindBuf fndbSearch;
    if (!diterLevel.bFindFirst( strCurPath
                                , kCIDLib::pszAllFilesSpec
                                , fndbSearch
                                , tCIDLib::EDirSearchFlags::All))
    {
        return;
    }

    TString strTmp;
    do
    {
        // If it's a dir and we are recursing, then do that
        if (fndbSearch.bIsDirectory() && bRecurse)
            DoADir(fndbSearch.pathFileName(), c4MaxH, c4MaxV, bRecurse);

        //
        //  If it's a file, then pull the extension and see if it's an image
        //  file type that we care about.
        //
        if (fndbSearch.bIsNormalFile())
        {
            const TPathStr& pathCur = fndbSearch.pathFileName();
            pathCur.bQueryExt(strTmp);
            strTmp.ToUpper();


            if ((strTmp == L"BMP")
            ||  (strTmp == L"JPEG")
            ||  (strTmp == L"JPG")
            ||  (strTmp == L"PNG")
            ||  (strTmp == L"MIMG"))
            {
                DoAFile(pathCur, strTmp, c4MaxH, c4MaxV);
            }
        }
    }   while (diterLevel.bFindNext(fndbSearch));
}


//
//  This is the the thread function for the main thread.
//
tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    //
    //  Since this is a demo and testing program, we'd like to catch
    //  all exceptions cleanly and report them. So put the whole thing
    //  in a try.
    //
    try
    {

        // Get the start path out and make sure it exists
        tCIDLib::TBoolean   bRecurse = kCIDLib::False;
        tCIDLib::TCard4     c4MaxH = 192;
        tCIDLib::TCard4     c4MaxV = 192;
        TString             strStartPath;

        if (!bParseParms(bRecurse, c4MaxH, c4MaxV, strStartPath))
        {
            ShowUsage();
            return tCIDLib::EExitCodes::BadParameters;
        }

        // Set options on the image objects
        imgJPEG.bFastDecode(kCIDLib::False);
        imgJPEG.bOptimalEncoding(kCIDLib::True);
        imgJPEG.c4CompQuality(100);
        imgJPEG.eOutSample(tCIDJPEG::EOutSamples::F4_4_4);

        DoADir(strStartPath, c4MaxH, c4MaxV, bRecurse);
    }

    //
    //  Catch any unhandled CIDLib runtime errors
    //
    catch(TError& errToCatch)
    {
        // If this hasn't been logged already, then log it
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TSysInfo::strmOut()
                    <<  L"A CIDLib runtime error occured during processing. "
                    <<  L"\nError: " << errToCatch.strErrText()
                    << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::FatalError;
    }

    //
    //  Kernel errors should never propogate out of CIDLib, but I test
    //  for them in my test programs so I can catch them if they do
    //  and fix them.
    //
    catch(const TKrnlError& kerrToCatch)
    {
        TSysInfo::strmOut()
                    << L"A kernel error occured during processing.\n  Error="
                    << kerrToCatch.errcId() << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::FatalError;
    }

    // Catch a general exception
    catch(...)
    {
        TSysInfo::strmOut()
                    << L"A general exception occured during processing\n"
                    << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::SystemException;
    }

    return tCIDLib::EExitCodes::Normal;
}


