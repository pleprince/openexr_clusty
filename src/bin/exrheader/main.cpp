//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) Contributors to the OpenEXR Project.
//

//-----------------------------------------------------------------------------
//
//	Utility program to print an image file's header
//
//-----------------------------------------------------------------------------

#include "ImfNamespace.h"
#include <ImfBoxAttribute.h>
#include <ImfChannelListAttribute.h>
#include <ImfChromaticitiesAttribute.h>
#include <ImfCompressionAttribute.h>
#include <ImfDoubleAttribute.h>
#include <ImfEnvmapAttribute.h>
#include <ImfFloatAttribute.h>
#include <ImfHeader.h>
#include <ImfIntAttribute.h>
#include <ImfKeyCodeAttribute.h>
#include <ImfLineOrderAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfMultiPartInputFile.h>
#include <ImfPreviewImageAttribute.h>
#include <ImfRationalAttribute.h>
#include <ImfStringAttribute.h>
#include <ImfStringVectorAttribute.h>
#include <ImfTileDescriptionAttribute.h>
#include <ImfTimeCodeAttribute.h>
#include <ImfVecAttribute.h>
#include <ImfVersion.h>
#include <ImfMisc.h>
#include <OpenEXRConfig.h>

#include <iomanip>
#include <iostream>

using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;

void
printCompression (Compression c)
{
    std::string desc;
    getCompressionDescriptionFromId(c, desc);
    cout << desc.c_str();
}

void
printLineOrder (LineOrder lo)
{
    switch (lo)
    {
        case INCREASING_Y: cout << "increasing y"; break;

        case DECREASING_Y: cout << "decreasing y"; break;

        case RANDOM_Y: cout << "random y"; break;

        default: cout << int (lo); break;
    }
}

void
printPixelType (PixelType pt)
{
    switch (pt)
    {
        case UINT: cout << "32-bit unsigned integer"; break;

        case HALF: cout << "16-bit floating-point"; break;

        case FLOAT: cout << "32-bit floating-point"; break;

        default: cout << "type " << int (pt); break;
    }
}

void
printLevelMode (LevelMode lm)
{
    switch (lm)
    {
        case ONE_LEVEL: cout << "single level"; break;

        case MIPMAP_LEVELS: cout << "mip-map"; break;

        case RIPMAP_LEVELS: cout << "rip-map"; break;

        default: cout << "level mode " << int (lm); break;
    }
}

void
printLevelRoundingMode (LevelRoundingMode lm)
{
    switch (lm)
    {
        case ROUND_DOWN: cout << "down"; break;

        case ROUND_UP: cout << "up"; break;

        default: cout << "mode " << int (lm); break;
    }
}

void
printTimeCode (TimeCode tc)
{
    cout << "    "
            "time "
         << setfill ('0') <<
#ifndef HAVE_COMPLETE_IOMANIP
        setw (2) << tc.hours () << ":" << setw (2) << tc.minutes () << ":"
         << setw (2) << tc.seconds () << ":" << setw (2) << tc.frame () << "\n"
         <<
#else
        setw (2) << right << tc.hours () << ":" << setw (2) << right
         << tc.minutes () << ":" << setw (2) << right << tc.seconds () << ":"
         << setw (2) << right << tc.frame () << "\n"
         <<
#endif
        setfill (' ')
         << "    "
            "drop frame "
         << tc.dropFrame ()
         << ", "
            "color frame "
         << tc.colorFrame ()
         << ", "
            "field/phase "
         << tc.fieldPhase ()
         << "\n"
            "    "
            "bgf0 "
         << tc.bgf0 ()
         << ", "
            "bgf1 "
         << tc.bgf1 ()
         << ", "
            "bgf2 "
         << tc.bgf2 ()
         << "\n"
            "    "
            "user data 0x"
         << hex << tc.userData () << dec;
}

void
printEnvmap (const Envmap& e)
{
    switch (e)
    {
        case ENVMAP_LATLONG: cout << "latitude-longitude map"; break;

        case ENVMAP_CUBE: cout << "cube-face map"; break;

        default: cout << "map type " << int (e); break;
    }
}

void
printChannelList (const ChannelList& cl)
{
    for (ChannelList::ConstIterator i = cl.begin (); i != cl.end (); ++i)
    {
        cout << "\n    " << i.name () << ", ";

        printPixelType (i.channel ().type);

        cout << ", sampling " << i.channel ().xSampling << " "
             << i.channel ().ySampling;

        if (i.channel ().pLinear) cout << ", plinear";
    }
}

void
printInfo (const char fileName[])
{
    MultiPartInputFile in (fileName);
    int                parts = in.parts ();

    //
    // Check to see if any parts are incomplete
    //

    bool fileComplete = true;

    for (int i = 0; i < parts && fileComplete; ++i)
        if (!in.partComplete (i)) fileComplete = false;

    //
    // Print file name and file format version
    //

    cout << "\nfile " << fileName << (fileComplete ? "" : " (incomplete)")
         << ":\n\n";

    cout << "file format version: " << getVersion (in.version ())
         << ", "
            "flags 0x"
         << setbase (16) << getFlags (in.version ()) << setbase (10) << "\n";

    //
    // Print the header of every part in the file
    //

    for (int p = 0; p < parts; ++p)
    {
        const Header& h = in.header (p);

        if (parts != 1)
        {
            cout << "\n\n part " << p
                 << (in.partComplete (p) ? "" : " (incomplete)") << ":\n";
        }

        for (Header::ConstIterator i = h.begin (); i != h.end (); ++i)
        {
            const Attribute* a = &i.attribute ();
            cout << i.name () << " (type " << a->typeName () << ")";

            if (const Box2iAttribute* ta =
                    dynamic_cast<const Box2iAttribute*> (a))
            {
                cout << ": " << ta->value ().min << " - " << ta->value ().max;
            }

            else if (
                const Box2fAttribute* ta =
                    dynamic_cast<const Box2fAttribute*> (a))
            {
                cout << ": " << ta->value ().min << " - " << ta->value ().max;
            }
            else if (
                const ChannelListAttribute* ta =
                    dynamic_cast<const ChannelListAttribute*> (a))
            {
                cout << ":";
                printChannelList (ta->value ());
            }
            else if (
                const ChromaticitiesAttribute* ta =
                    dynamic_cast<const ChromaticitiesAttribute*> (a))
            {
                cout << ":\n"
                        "    red   "
                     << ta->value ().red
                     << "\n"
                        "    green "
                     << ta->value ().green
                     << "\n"
                        "    blue  "
                     << ta->value ().blue
                     << "\n"
                        "    white "
                     << ta->value ().white;
            }
            else if (
                const CompressionAttribute* ta =
                    dynamic_cast<const CompressionAttribute*> (a))
            {
                cout << ": ";
                printCompression (ta->value ());
            }
            else if (
                const DoubleAttribute* ta =
                    dynamic_cast<const DoubleAttribute*> (a))
            {
                cout << ": " << ta->value ();
            }
            else if (
                const EnvmapAttribute* ta =
                    dynamic_cast<const EnvmapAttribute*> (a))
            {
                cout << ": ";
                printEnvmap (ta->value ());
            }
            else if (
                const FloatAttribute* ta =
                    dynamic_cast<const FloatAttribute*> (a))
            {
                cout << ": " << ta->value ();
            }
            else if (
                const IntAttribute* ta = dynamic_cast<const IntAttribute*> (a))
            {
                cout << ": " << ta->value ();
            }
            else if (
                const KeyCodeAttribute* ta =
                    dynamic_cast<const KeyCodeAttribute*> (a))
            {
                cout << ":\n"
                        "    film manufacturer code "
                     << ta->value ().filmMfcCode ()
                     << "\n"
                        "    film type code "
                     << ta->value ().filmType ()
                     << "\n"
                        "    prefix "
                     << ta->value ().prefix ()
                     << "\n"
                        "    count "
                     << ta->value ().count ()
                     << "\n"
                        "    perf offset "
                     << ta->value ().perfOffset ()
                     << "\n"
                        "    perfs per frame "
                     << ta->value ().perfsPerFrame ()
                     << "\n"
                        "    perfs per count "
                     << ta->value ().perfsPerCount ();
            }
            else if (
                const LineOrderAttribute* ta =
                    dynamic_cast<const LineOrderAttribute*> (a))
            {
                cout << ": ";
                printLineOrder (ta->value ());
            }
            else if (
                const M33fAttribute* ta =
                    dynamic_cast<const M33fAttribute*> (a))
            {
                cout << ":\n"
                        "   ("
                     << ta->value ()[0][0] << " " << ta->value ()[0][1] << " "
                     << ta->value ()[0][2] << "\n    " << ta->value ()[1][0]
                     << " " << ta->value ()[1][1] << " " << ta->value ()[1][2]
                     << "\n    " << ta->value ()[2][0] << " "
                     << ta->value ()[2][1] << " " << ta->value ()[2][2] << ")";
            }
            else if (
                const M44fAttribute* ta =
                    dynamic_cast<const M44fAttribute*> (a))
            {
                cout << ":\n"
                        "   ("
                     << ta->value ()[0][0] << " " << ta->value ()[0][1] << " "
                     << ta->value ()[0][2] << " " << ta->value ()[0][3]
                     << "\n    " << ta->value ()[1][0] << " "
                     << ta->value ()[1][1] << " " << ta->value ()[1][2] << " "
                     << ta->value ()[1][3] << "\n    " << ta->value ()[2][0]
                     << " " << ta->value ()[2][1] << " " << ta->value ()[2][2]
                     << " " << ta->value ()[2][3] << "\n    "
                     << ta->value ()[3][0] << " " << ta->value ()[3][1] << " "
                     << ta->value ()[3][2] << " " << ta->value ()[3][3] << ")";
            }
            else if (
                const PreviewImageAttribute* ta =
                    dynamic_cast<const PreviewImageAttribute*> (a))
            {
                cout << ": " << ta->value ().width () << " by "
                     << ta->value ().height () << " pixels";
            }
            else if (
                const StringAttribute* ta =
                    dynamic_cast<const StringAttribute*> (a))
            {
                cout << ": \"" << ta->value () << "\"";
            }
            else if (
                const StringVectorAttribute* ta =
                    dynamic_cast<const StringVectorAttribute*> (a))
            {
                cout << ":";

                for (StringVector::const_iterator i = ta->value ().begin ();
                     i != ta->value ().end ();
                     ++i)
                {
                    cout << "\n    \"" << *i << "\"";
                }
            }
            else if (
                const RationalAttribute* ta =
                    dynamic_cast<const RationalAttribute*> (a))
            {
                cout << ": " << ta->value ().n << "/" << ta->value ().d << " ("
                     << double (ta->value ()) << ")";
            }
            else if (
                const TileDescriptionAttribute* ta =
                    dynamic_cast<const TileDescriptionAttribute*> (a))
            {
                cout << ":\n    ";

                printLevelMode (ta->value ().mode);

                cout << "\n    tile size " << ta->value ().xSize << " by "
                     << ta->value ().ySize << " pixels";

                if (ta->value ().mode != ONE_LEVEL)
                {
                    cout << "\n    level sizes rounded ";
                    printLevelRoundingMode (ta->value ().roundingMode);
                }
            }
            else if (
                const TimeCodeAttribute* ta =
                    dynamic_cast<const TimeCodeAttribute*> (a))
            {
                cout << ":\n";
                printTimeCode (ta->value ());
            }
            else if (
                const V2iAttribute* ta = dynamic_cast<const V2iAttribute*> (a))
            {
                cout << ": " << ta->value ();
            }
            else if (
                const V2fAttribute* ta = dynamic_cast<const V2fAttribute*> (a))
            {
                cout << ": " << ta->value ();
            }
            else if (
                const V3iAttribute* ta = dynamic_cast<const V3iAttribute*> (a))
            {
                cout << ": " << ta->value ();
            }
            else if (
                const V3fAttribute* ta = dynamic_cast<const V3fAttribute*> (a))
            {
                cout << ": " << ta->value ();
            }

            cout << '\n';
        }
    }

    cout << endl;
}

void
usageMessage (ostream& stream, const char* program_name, bool verbose = false)
{
    stream << "Usage: " << program_name << " imagefile [imagefile ...]\n";

    if (verbose)
        stream << "\n"
            "Read exr files and print the values of header attributes.\n"
            "\n"
            "Options:\n"
            "  -h, --help        print this message\n"
            "      --version     print version information\n"
            "\n"
            "Report bugs via https://github.com/AcademySoftwareFoundation/openexr/issues or email security@openexr.com\n"
            "";
}

int
main (int argc, char** argv)
{
    if (argc < 2)
    {
        usageMessage (cerr, argv[0], false);
        return -1;
    }

    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp (argv[i], "-h") || !strcmp(argv[1], "--help"))
        {
            usageMessage (cout, "exrheader", true);
            return 0;
        }
        else if (!strcmp (argv[i], "--version"))
        {
            const char* libraryVersion = getLibraryVersion();
            
            cout << "exrheader (OpenEXR) " << OPENEXR_VERSION_STRING;
            if (strcmp(libraryVersion, OPENEXR_VERSION_STRING))
                cout << "(OpenEXR version " << libraryVersion << ")";
            cout << " https://openexr.com" << endl;
            cout << "Copyright (c) Contributors to the OpenEXR Project" << endl;
            cout << "License BSD-3-Clause" << endl;
            return 0;
        }
    }

    try
    {
        for (int i = 1; i < argc; ++i)
            printInfo (argv[i]);
    }
    catch (const exception& e)
    {
        cerr << argv[0] << ": " << e.what () << endl;
        return 1;
    }

    return 0;
}
