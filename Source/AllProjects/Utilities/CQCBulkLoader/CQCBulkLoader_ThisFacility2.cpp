//
// FILE NAME: CQCBulkLoader_ThisFacility2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2010
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
//  This file handles some of the grunt work of the facility class, keeping
//  the main file a little more reasonable and readable. We have the stuff
//  here related to the parsing of the configuration file in particular.
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
#include "CQCBulkLoader.hpp"


// ---------------------------------------------------------------------------
//  A macro to conveniently get the current line number into logged messages
// ---------------------------------------------------------------------------
#define CURLINE L"\n!![Line " << m_c4LineNum << L"] - "




// ---------------------------------------------------------------------------
//  TFacCQCBulkLoader: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Parses a line that contains a key=value line. If the key is the one
//  indicated returns true and the value, else returns false and pushes the
//  line back.
//
tCIDLib::TBoolean
TFacCQCBulkLoader::bCheckKey(const  TString&            strToCheck
                            ,       TString&            strValue
                            , const tCIDLib::TBoolean   bEmptyOk)
{
    static TString strKeyVal;
    static TString strSave;

    // If end of file, then obviously not
    if (!bReadLine(strKeyVal, kCIDLib::True))
        return kCIDLib::False;

    //
    //  Break the two parts out. Save the original line in case we need to
    //  undo this and push it back.
    //
    strSave = strKeyVal;
    if (!strKeyVal.bSplit(strValue, kCIDLib::chEquals)
    ||  (strKeyVal != strToCheck))
    {
        Pushback(strSave);
        return kCIDLib::False;
    }

    strValue.StripWhitespace();
    if (!bEmptyOk && strValue.bIsEmpty())
    {
        m_strmOut << CURLINE << L"The value for key '" << strToCheck
                  << L"' cannot be empty" << kCIDLib::EndLn;
        throw(EError_Syntax);
    }

    return kCIDLib::True;
}


//
//  In those cases where we want to check if the next line is something that
//  we want to look for specifically. If so, it returns true. Else it pushes
//  the line back and returns false.
//
tCIDLib::TBoolean TFacCQCBulkLoader::bCheckLine(const TString& strToCheck)
{
    static TString strLine;

    // If we hit the end of file, then obviously not
    if (!bReadLine(strLine, kCIDLib::True))
        return kCIDLib::False;

    // We got something, see if it's what we want
    if (strLine != strToCheck)
    {
        Pushback(strLine);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This method is called during init to parse the config file and load up
//  all the info. We load it all up before we do anything, because we need
//  to do a fair bit of checking before we start the work.
//
tCIDLib::TBoolean TFacCQCBulkLoader::bParseCfgFile()
{
    m_strmOut << L"Parsing cfg file: " << m_pathCfgFile << kCIDLib::EndLn;
    try
    {
        m_strmCfg.Open
        (
            m_pathCfgFile
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::None
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Excl_Read
        );

        // Reset the line number
        m_c4LineNum = 0;

        //
        //  Keep the path part of the config file's path. Expand it out if it is relative
        //  to the program path.
        //
        m_pathCfgRel = m_pathCfgFile;
        m_pathCfgRel.Normalize();
        m_pathCfgRel.bRemoveNameExt();

        // The first the we have to see is the main block
        ParseMainBlock();

        // Next we should see the block that lists stuff to unload
        ParseUnloadBlock();

        // Next we should see the block that defines drivers to load
        ParseDriversBlock();

        // Next we should see the block that defines events
        ParseEventsBlock();

        // Next we should see the block that defines images
        ParseImagesBlock();

        //
        //  And the optional ignore block, which allows them to keep stuff
        //  in the file but temporarily keep it from being used.
        //
        ParseIgnoreBlock();

        // And the rest should be comments or empty space
        EatToEnd();
    }

    catch(const TError& errToCatch)
    {
        m_strmOut << L"\n!!An exception occured while loading the config file\n"
                  << errToCatch << kCIDLib::NewLn
                  << kCIDLib::EndLn;
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Reads a line from the configuration source file, skipping empty lines
//  and comments. If bEndOK is true, then it just returns true or false
//  to indicate whether it hit the end or not. Else, it will throw if it
//  hits the end of stream.
//
tCIDLib::TBoolean
TFacCQCBulkLoader::bReadLine(       TString&            strToFill
                            , const tCIDLib::TBoolean   bEndOK)
{
    static const TString strCommentPref(L"//");

    // If we have a pushback line, give that first
    if (!m_strPushback.bIsEmpty())
    {
        m_c4LineNum++;
        strToFill = m_strPushback;
        m_strPushback.Clear();
        return kCIDLib::True;
    }

    while (!m_strmCfg.bEndOfStream())
    {
        m_strmCfg.c4GetLine(strToFill);
        m_c4LineNum++;

        // Get a line of text. Skip it if its empty
        strToFill.StripWhitespace();
        if (strToFill.bIsEmpty())
            continue;

        // If it starts with a //, skip it as a comment
        if (strToFill.bStartsWith(strCommentPref))
            continue;

        // Must be something useful, so break out
        break;
    }

    // If we got nothing, and it's not ok to hit the end, that's bad
    if (strToFill.bIsEmpty() && m_strmCfg.bEndOfStream())
    {
        if (!bEndOK)
        {
            m_strmOut << CURLINE << L"Unexpected end of file occurred\n"
                      << kCIDLib::EndLn;
            throw(EError_UnexpectedEOF);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Parses a line that contains a key=value line. Makes sure that the
//  indicated key was found, and gets the value and returns it as a Card4
//  value.
//
tCIDLib::TCard4 TFacCQCBulkLoader::c4CheckKey(const TString& strKey)
{
    // Call the basic key parser to get the value back
    static TString strValue;
    CheckKey(strKey, strValue);

    // And convert
    tCIDLib::TCard4 c4Ret;
    if (!strValue.bToCard4(c4Ret, tCIDLib::ERadices::Dec))
    {
        m_strmOut << CURLINE << L"Invalid value for key '" << strKey << L"'"
                  << kCIDLib::EndLn;
        throw(EError_Data);
    }
    return c4Ret;
}


//
//  Parses a line that contains a key=value line. Makes sure that the
//  indicated key was found, and returns the value part.
//
tCIDLib::TVoid
TFacCQCBulkLoader::CheckKey(const   TString&            strToCheck
                            ,       TString&            strValue
                            , const tCIDLib::TBoolean   bEmptyOk)
{
    static TString strLine;
    bReadLine(strLine);

    // Break the two parts out
    if (!strLine.bSplit(strValue, kCIDLib::chEquals)
    ||  (strLine != strToCheck))
    {
        m_strmOut << CURLINE << L"Expected to find '" << strToCheck
                  << L"=' here" << kCIDLib::EndLn;
        throw(EError_Syntax);
    }

    strValue.StripWhitespace();
    if (!bEmptyOk && strValue.bIsEmpty())
    {
        m_strmOut << CURLINE << L"The value for key '" << strToCheck
                  << L"' cannot be empty" << kCIDLib::EndLn;
        throw(EError_Syntax);
    }
}


//
//  In those cases where we have to see particular lines, this is a
//  convenience to check for those. It throws if it doesn't find that
//  line.
//
tCIDLib::TVoid TFacCQCBulkLoader::CheckLine(const TString& strToCheck)
{
    static TString strLine;

    bReadLine(strLine);
    if (strLine != strToCheck)
    {
        m_strmOut << CURLINE << L"Expected to see '" << strToCheck << L"'"
                  << kCIDLib::EndLn;
        throw(EError_Syntax);
    }
}


//
//  Handles parsing the data for an image. It's all on one line, comma
//  separated, with only the first value being required.
//
//  NOTE: For now, we aren't doing any checking for transparency color,
//  it's just there for when we enable it.
//
tCQCBulkLoader::EImgTrans
TFacCQCBulkLoader::eParseImageData( const   TString&    strImgData
                                    ,       TString&    strName
                                    ,       TPoint&     pntTransClr
                                    ,       TRGBClr&    rgbTransClr)
{
    static const TString strSep(L",");

    tCQCBulkLoader::EImgTrans eRet = tCQCBulkLoader::EImgTrans_None;

    // It's one or more comma separated tokens
    TStringTokenizer stokData(&strImgData, strSep);

    if (!stokData.bGetNextToken(strName))
    {
        m_strmOut << CURLINE << L"The image name must be provided"
                  << kCIDLib::EndLn;
        throw(EError_Syntax);
    }
    strName.StripWhitespace();

    return eRet;
}


//
//  After all legal content is seen, this is called to make sure no illegal
//  content is at the end of the file.
//
tCIDLib::TVoid TFacCQCBulkLoader::EatToEnd()
{
    TString strTmp;
    if (bReadLine(strTmp, kCIDLib::True))
    {
        // This isn't legal
        m_strmOut << CURLINE << L"Illegal trailing content in file\n"
                  << kCIDLib::EndLn;
        throw(EError_Syntax);
    }
}


//
//  Handles the parsing of a single driver info block. We then put that
//  info it a new driver object and add it to our list.
//
tCIDLib::TVoid
TFacCQCBulkLoader::ParseDriverBlock(const TString& strMoniker)
{
    TPathStr    pathFldExport;
    TString     strMake;
    TString     strModel;
    TString     strHost;
    TString     strConnInfo;

    CheckKey(m_strKey_Make, strMake);
    CheckKey(m_strKey_Model, strModel);
    CheckKey(m_strKey_Host, strHost);
    CheckKey(m_strKey_ConnInfo, strConnInfo, kCIDLib::True);

    // We can optionally get a fld export for variable drivers
    bCheckKey(m_strKey_FldExport, pathFldExport, kCIDLib::True);

    //
    //  If we got a field export, but it's not the variables driver, then
    //  that's an error.
    //
    if (!pathFldExport.bIsEmpty())
    {
        if ((strMake != L"Charmed Quark") || (strModel != L"VarDriver"))
        {
            m_strmOut << CURLINE
                      << L"The field export key is only valid for a variables "
                         L"driver instance" << kCIDLib::EndLn;
            throw(EError_Syntax);
        }
    }

    // If the path is relative, then make it relative to the configuration file path
    if (!pathFldExport.bIsEmpty() && !pathFldExport.bIsFullyQualified())
        pathFldExport.AddToBasePath(m_pathCfgRel);

    // If the host is 'localhost' then translate to this host
    if (strHost == L"localhost")
        strHost = TSysInfo::strIPHostName();

    // We keep a list of all of the unique hosts for later validation purposes
    if (!m_colHosts.bHasElement(strHost))
        m_colHosts.objAdd(strHost);

    // Looks ok, so let's store the info away
    TBLDrvInfo& drviNew = m_colDrivers.objAdd
    (
        TBLDrvInfo
        (
            strMoniker, strMake, strModel, strHost, strConnInfo, pathFldExport
        )
    );

    //
    //  Now we could have driver prompts to store. Clear the dup keys list
    //  and use it as a check for duplicate prompt keys.
    //
    if (bCheckLine(m_strBlk_Prompts))
    {
        m_colDupKeys.RemoveAll();

        TString strKey;
        TString strSubKey;
        TString strValue;

        // Loop while we have prompt keys
        while (bCheckKey(m_strKey_Prompt, strKey))
        {
            TBLPromptInfo piNew(strKey);

            // Make sure it's not already been used
            if (m_colDupKeys.bHasElement(strKey))
            {
                m_strmOut << L"!!Prompt key '" << strKey << L"' has already been "
                             L"used for driver '" << strMoniker << L"'"
                          << kCIDLib::EndLn;
                throw(EError_Data);
            }
            m_colDupKeys.objAdd(strKey);

            // And while we see subkeys
            tCIDLib::TCard4 c4SubKeyCnt = 0;
            while (bCheckKey(m_strKey_SubKey, strSubKey))
            {
                c4SubKeyCnt++;
                CheckKey(m_strKey_Value, strValue, kCIDLib::True);
                piNew.AddSubKey(strSubKey, strValue);
                CheckLine(m_strBlk_SubKeyEnd);
            }

            // We have to see at least one subkey
            if (!c4SubKeyCnt)
            {
                m_strmOut << CURLINE << L"Prompt '" << strKey
                          << L"' for driver '" << strMoniker
                          << L"' has no sub-keys" << kCIDLib::EndLn;
                throw(EError_Data);
            }

            // And add the overall prompt to our driver
            drviNew.AddPrompt(piNew);

            // And we have to see the end of the prompt
            CheckLine(m_strBlk_PromptEnd);
        }

        // And we have to see the end of prompts
        CheckLine(m_strBlk_PromptsEnd);
    }

    // And finally we have to see the end block
    CheckLine(m_strBlk_DriverEnd);

    // If we got a field extract, then remember that we saw at least one
    if (!pathFldExport.bIsEmpty())
        m_bLoadFldExtracts = kCIDLib::True;
}


//
//  Handles parsing the drivers block of the file. This defines which drivers
//  to install. We also check here to make sure that no duplicate driver
//  monikers are used in the file.
//
tCIDLib::TVoid TFacCQCBulkLoader::ParseDriversBlock()
{
    CheckLine(m_strBlk_Drivers);

    // We now loop until we hit the end of the driver block
    TString strMoniker;
    while (bCheckKey(m_strKey_Driver, strMoniker))
    {
        // Make sure it's not a dup
        if (m_colDupDrivers.bHasElement(strMoniker))
        {
            m_strmOut << CURLINE << L"Moniker '" << strMoniker
                      << L"' has already been used" << kCIDLib::EndLn;
            throw(EError_Data);
        }

        // It's not, so add it to the dup list and parse it
        m_colDupDrivers.objAdd(strMoniker);
        ParseDriverBlock(strMoniker);
    }

    // Have to see the end of the overall divers block
    CheckLine(m_strBlk_DriversEnd);
}


//
//  Called at the end of parsing, to parse an optional ignore block. This
//  allows content to be kept in the file but temporarily ignored. We just
//  loop till we see the end of the block, eating lines.
//
tCIDLib::TVoid TFacCQCBulkLoader::ParseIgnoreBlock()
{
    if (bCheckLine(m_strBlk_Ignore))
    {
        TString strLine;
        do
        {
            bReadLine(strLine);
        }   while(strLine != m_strBlk_IgnoreEnd);
    }
}


//
//  This method parses a single triggered or scheduled event block and
//  puts it in the list of event sets to load.
//
tCIDLib::TVoid TFacCQCBulkLoader::ParseEventTree(TBLEvScopeInfo& scpiParent)
{
    // We can't have any duplicate names at a given level
    TDupList    colDupNames(109, TStringKeyOps());
    TString     strName;

    //
    //  Loop until we hit our own level's end of scope. Until then, we can
    //  see event or subdir entries.
    //
    while(kCIDLib::True)
    {
        if (bCheckLine(m_strBlk_EventScopeEnd))
        {
            break;
        }
         else if (bCheckLine(m_strBlk_EventScope))
        {
            // Recurse on a child scope and add it to our parent
            CheckKey(m_strKey_Name, strName);

            // Validate, no extensiosn allowed here
            ValidateTarName(strName, kCIDLib::False);

            // Make sure that the target name hasn't been used yet
            if (colDupNames.bHasElement(strName))
            {
                m_strmOut << CURLINE
                          << L"!!The target name '" << strName
                          << L"' has already been used at this scope level"
                          << kCIDLib::EndLn;
                throw(EError_Syntax);
            }
            colDupNames.objAdd(strName);

            TBLEvScopeInfo& scpiNew = scpiParent.scpiAddSubDir(strName);
            ParseEventTree(scpiNew);
        }
         else if (bCheckKey(m_strKey_Event, strName))
        {
            // Make sure that the target name hasn't been used yet
            if (colDupNames.bHasElement(strName))
            {
                m_strmOut << CURLINE
                          << L"!!The target name '" << strName
                          << L"' has already been used at this scope level"
                          << kCIDLib::EndLn;
                throw(EError_Syntax);
            }
            colDupNames.objAdd(strName);
            scpiParent.AddEvent(strName);
        }
         else
        {
            //
            //  Has to be something illegal. Bump the line since we never
            //  actually consumed it.
            //
            m_c4LineNum++;
            m_strmOut << CURLINE
                      << L"!!Expected to see an event, a child event scope or"
                         L" the end of the current event scope here\n"
                      << kCIDLib::EndLn;
            throw(EError_Syntax);
        }
    }
}


//
//  Handles parsing the events block of the file. This defines which scheduled
//  and triggered events to load up.
//
tCIDLib::TVoid TFacCQCBulkLoader::ParseEventsBlock()
{
    // We have to see the start of the events block here
    CheckLine(m_strBlk_Events);

    // While we have image lists, process them
    TPathStr    pathSrcDir;
    TString     strTarScope;
    TString     strType;
    while (kCIDLib::True)
    {
        if (bCheckLine(m_strBlk_EventScope))
        {
            //
            //  At this level we have to get both a source directory and a target scope,
            //  which point at the starting points of the tree.
            //
            CheckKey(m_strKey_SrcDir, pathSrcDir);
            CheckKey(m_strKey_TarScope, strTarScope);

            // If the source is relative, make it relative to the config file location
            if (!pathSrcDir.bIsFullyQualified())
                pathSrcDir.AddToBasePath(m_pathCfgRel);

            //
            //  At this level we have to get an event type, which gets passed along down
            //  this tree.
            //
            tCQCRemBrws::EDTypes eType;
            CheckKey(m_strKey_Type, strType);
            if (strType.bCompareI(L"Triggered"))
                eType = tCQCRemBrws::EDTypes::TrgEvent;
            else if (strType.bCompareI(L"Scheduled"))
                eType = tCQCRemBrws::EDTypes::SchEvent;
            else
            {
                m_strmOut << CURLINE << L"!!Expected to see an event type here\n"
                          << kCIDLib::EndLn;
                throw(EError_Syntax);
            }

            // Make sure the target scope is reasonable
            ValidateTarPath(strTarScope);

            //
            //  Add a new scope info, and then call the recursive tree parsing method to
            //  load it up. It will come back after it's consumed the end line of our
            //  current events block.
            //
            TBLEvScopeInfo& scpiNew = m_colEvents.objAdd
            (
                TBLEvScopeInfo(pathSrcDir, strTarScope, eType)
            );
            ParseEventTree(scpiNew);
        }
         else if (bCheckLine(m_strBlk_EventsEnd))
        {
            // We are done with events
            break;
        }
         else
        {
            //
            //  Has to be something illegal. Bump the line since we never
            //  actually consumed it.
            //
            m_c4LineNum++;
            m_strmOut << CURLINE << L"!!Expected to see an event list or"
                         L" the end of the events block here\n"
                      << kCIDLib::EndLn;
            throw(EError_Syntax);
        }
    }
}


//
//  These methods handle parsing image blocks. The images block contains
//  one or more trees of images to load. The top level contains the full
//  source directory and target scope. After that it becomes a recursive
//  process which the ParseImageTree method handles, passing the new scope
//  info class down into each level.
//
tCIDLib::TVoid
TFacCQCBulkLoader::ParseImageTree(TBLImgScopeInfo& scpiParent)
{
    // We can't have any duplicate names at a given level
    TDupList    colDupNames(109, TStringKeyOps());
    TPoint      pntTransClr;
    TRGBClr     rgbTransClr;
    TString     strImgData;
    TString     strName;

    //
    //  Loop until we hit our own level's end of scope. Until then, we can
    //  see image or subdir entries.
    //
    while(kCIDLib::True)
    {
        if (bCheckLine(m_strBlk_ImageScopeEnd))
        {
            break;
        }
         else if (bCheckLine(m_strBlk_ImageScope))
        {
            //
            //  It's a nested scope. At this level it's just a single name used for both
            //  source and target.
            //
            CheckKey(m_strKey_Name, strName);

            // Validate, no extensiosn allowed here
            ValidateTarName(strName, kCIDLib::False);

            // Make sure that the name hasn't been used yet
            if (colDupNames.bHasElement(strName))
            {
                m_strmOut << CURLINE
                          << L"!!The target name '" << strName
                          << L"' has already been used at this scope level"
                          << kCIDLib::EndLn;
                throw(EError_Syntax);
            }
            colDupNames.objAdd(strName);

            TBLImgScopeInfo& scpiNew = scpiParent.scpiAddSubDir(strName);
            ParseImageTree(scpiNew);
        }
         else if (bCheckKey(m_strKey_Image, strImgData))
        {
            //
            //  It's an image entry for this level, so let's parse that
            //  out and store a new image on the parent scope.
            //
            const tCQCBulkLoader::EImgTrans eTrans = eParseImageData
            (
                strImgData, strName, pntTransClr, rgbTransClr
            );

            // Validate, extensions allowed here
            ValidateTarName(strName, kCIDLib::True);

            // Make sure that the target name hasn't been used yet
            if (colDupNames.bHasElement(strName))
            {
                m_strmOut << CURLINE
                          << L"!!The target name '" << strName
                          << L"' has already been used at this scope level"
                          << kCIDLib::EndLn;
                throw(EError_Syntax);
            }
            colDupNames.objAdd(strName);
            scpiParent.AddImage(eTrans, strName, pntTransClr, rgbTransClr);
        }
         else
        {
            //
            //  Has to be something illegal. Bump the line since we never
            //  actually consumed it.
            //
            m_c4LineNum++;
            m_strmOut << CURLINE
                      << L"!!Expected to see an image, a child image scope or"
                         L" the end of the current image scope here\n"
                      << kCIDLib::EndLn;
            throw(EError_Syntax);
        }
    }
}


tCIDLib::TVoid TFacCQCBulkLoader::ParseImagesBlock()
{
    // We have to see the start of the images block here
    CheckLine(m_strBlk_Images);

    // While we have image lists, process them
    TPathStr    pathSrcDir;
    TString     strTarScope;
    while (kCIDLib::True)
    {
        if (bCheckLine(m_strBlk_ImageScope))
        {
            //
            //  We have to have a source dir and target scope at this level. They both need
            //  to point at the starting points in the tree.
            //
            CheckKey(m_strKey_SrcDir, pathSrcDir);
            CheckKey(m_strKey_TarScope, strTarScope);

            // If the source is relative, make it relative to the config file location
            if (!pathSrcDir.bIsFullyQualified())
                pathSrcDir.AddToBasePath(m_pathCfgRel);

            //
            //  Make sure the target scope has no unwanted characters. At this point it's
            //  a path, not a name.
            //
            ValidateTarPath(strTarScope);

            //
            //  Add a new scope info, and then call the recursive tree parsing method
            //  to load it up. It will come back after it's consumed the end line of
            //  our current image list block.
            //
            TBLImgScopeInfo& scpiNew = m_colImages.objAdd(TBLImgScopeInfo(pathSrcDir, strTarScope));
            ParseImageTree(scpiNew);
        }
         else if (bCheckLine(m_strBlk_ImagesEnd))
        {
            // We are done with images
            break;
        }
         else
        {
            //
            //  Has to be something illegal. Bump the line since we never
            //  actually consumed it.
            //
            m_c4LineNum++;
            m_strmOut << CURLINE << L"!!Expected to see an image list or"
                         L" the end of the images block here\n"
                      << kCIDLib::EndLn;
            throw(EError_Syntax);
        }
    }
}



//
//  Handles parsing the main block of the file. This contains some overall
//  info.
//
tCIDLib::TVoid TFacCQCBulkLoader::ParseMainBlock()
{
    CheckLine(m_strBlk_CfgInfo);
    m_c4VersionNum = c4CheckKey(m_strKey_Version);
    CheckKey(m_strKey_Descr, m_strDescription);
    CheckLine(m_strBlk_CfgInfoEnd);
}


//
//  Handles parsing the block of stuff to unload. It contains multiple
//  sub-blocks.
//
tCIDLib::TVoid TFacCQCBulkLoader::ParseUnloadBlock()
{
    CheckLine(m_strBlk_Unload);

    // We should see a drivers child block
    CheckLine(m_strBlk_Drivers);

    // We now loop until we hit the end of the drivers block
    TString strMoniker;
    while (bReadLine(strMoniker, kCIDLib::False))
    {
        // Break out on the end line
        if (strMoniker == m_strBlk_DriversEnd)
            break;

        // If not a dup, add it to the list
        if (!m_colDupDrivers.bHasElement(strMoniker))
            m_colUnloadDrvs.objAdd(strMoniker);
    }

    // Have to see the end of the overall divers block
    CheckLine(m_strBlk_UnloadEnd);
}



// Allows for pushback of lines
tCIDLib::TVoid TFacCQCBulkLoader::Pushback(const TString& strToPush)
{
    CIDAssert
    (
        m_strPushback.bIsEmpty() && (m_c4LineNum != 0)
        , L"Line parser pushback error"
    );

    m_strPushback = strToPush;
    m_c4LineNum--;
}


// For scope names, and scope paths
tCIDLib::TVoid
TFacCQCBulkLoader::ValidateTarName(const TString& strName, const tCIDLib::TBoolean bExtOK)
{
    const tCIDLib::TCard4 c4Count = strName.c4Length();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const tCIDLib::TCh chCur = strName[c4Index];

        // We can't have any separators in names
        if ((chCur == kCIDLib::chBackSlash)
        ||  (chCur == kCIDLib::chForwardSlash))
        {
            m_strmOut << CURLINE << L"!!The item or scope name contains "
                         L"path separator characters" << kCIDLib::EndLn;
            throw(EError_Syntax);
        }

        // If extensions are not ok, then no periods
        if (chCur == kCIDLib::chPeriod)
        {
            if (!bExtOK)
            {
                m_strmOut << CURLINE << L"!!The item or scope name contains "
                             L"a file extension separator" << kCIDLib::EndLn;
                throw(EError_Syntax);
            }
        }
         else if (!TRawStr::bIsAlphaNum(chCur)
              &&  (chCur != kCIDLib::chSpace)
              &&  (chCur != kCIDLib::chHyphenMinus)
              &&  (chCur != kCIDLib::chUnderscore))
        {
            m_strmOut << CURLINE << L"!!The item or scope name contains "
                         L"illegal characters" << kCIDLib::EndLn;
            throw(EError_Syntax);
        }
    }
}

tCIDLib::TVoid TFacCQCBulkLoader::ValidateTarPath(const TString& strPath)
{
    const tCIDLib::TCard4 c4Count = strPath.c4Length();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const tCIDLib::TCh chCur = strPath[c4Index];

        if (!TRawStr::bIsAlphaNum(chCur)
        &&  (chCur != kCIDLib::chSpace)
        &&  (chCur != kCIDLib::chForwardSlash)
        &&  (chCur != kCIDLib::chHyphenMinus)
        &&  (chCur != kCIDLib::chUnderscore))
        {
            m_strmOut << CURLINE << L"!!The scope path contains "
                         L"illegal characters" << kCIDLib::EndLn;
            throw(EError_Syntax);
        }
    }
}

