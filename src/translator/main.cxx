#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>
#include <osgDB/ReaderWriter>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osg/Node>

#include <osgwTools/GeometryModifier.h>
#include <osgwTools/Trianglizer.h>

#include <iostream>
#include <string>
#include <sstream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "RemoveNodeNameVisitor.h"
#include "ShareNodes.h"
#include "RemoveByDesc.h"

#include <crunchstore/Persistable.h>
#include <crunchstore/Datum.h>
#include <crunchstore/DataManager.h>
#include <crunchstore/NullBuffer.h>
#include <crunchstore/NullCache.h>
#include <crunchstore/DataAbstractionLayer.h>
#include <crunchstore/SQLiteStore.h>

using namespace crunchstore;
///Setup the location of the db file
std::string SetupDBLocation()
{
    boost::filesystem::path tempPath;
    try
    {
        tempPath = boost::filesystem::temp_directory_path();
    }
    catch( boost::filesystem::filesystem_error& ec )
    {
        std::cout << ec.what() << std::endl;
    }

    std::string logNameStr( "jt2ive" );
    tempPath /= logNameStr;
    // Create subdir if needed
    if( !boost::filesystem::exists( tempPath ) )
    {
        boost::filesystem::create_directory( tempPath );
    }
    tempPath /= "jt2ive.db";
    std::string dbPath;
    dbPath = tempPath.string();
    
    std::cout << "Logging db location " << dbPath << std::endl;

    return dbPath;
}

int main( int argc, char* argv[] )
{
    if( argc < 2 )
    {
        std::cout << "Usage: jtToive <file to load> " << std::endl;
        return 1;
    }

    std::string outDir;
    if( argc == 3 )
    {
        outDir = argv[ 2 ];
    }

    boost::filesystem::path exePath;
    exePath = boost::filesystem::path( argv[ 0 ] );
    osgDB::FilePathList& fileList = osgDB::getDataFilePathList();
    fileList.push_back( exePath.parent_path().string() );

    osgDB::Registry::instance()->loadLibrary( "osgdb_PolyTrans.dll" );

    // Set up a datamanager to test persistence
    DataManager manager;
    DataAbstractionLayerPtr cache( new NullCache );
    DataAbstractionLayerPtr buffer( new NullBuffer );
    manager.SetCache( cache );
    manager.SetBuffer( buffer );
    
    // Add an SQLite store
    SQLiteStorePtr sqstore( new SQLiteStore );
    sqstore->SetStorePath( SetupDBLocation() );
    manager.AttachStore( sqstore, Store::WORKINGSTORE_ROLE );
    
    // Build up a persistable with some useful test types
    Persistable q;
    {
        q.SetTypeName( "FileConversion" );
        q.AddDatum( "input_file", std::string( argv[ 1 ] ) );
        
        const boost::posix_time::ptime now =
            boost::posix_time::second_clock::local_time();
        boost::posix_time::time_facet* const f =
            new boost::posix_time::time_facet("%d-%b-%Y %H:%M:%S");
        std::ostringstream msg;
        msg.imbue( std::locale( msg.getloc(), f ) );
        msg << now;
        q.AddDatum( "start_onversion_date", msg.str() );
    }
    
    boost::filesystem::path outPath;
    if( argc < 3 )
    {
        outPath = boost::filesystem::path( argv[ 1 ] );
        outPath.replace_extension( "ive" );
    }
    else
    {
        boost::filesystem::path p( argv[ 1 ] );
        outPath = boost::filesystem::path( argv[ 2 ] );
        if( !boost::filesystem::exists( outPath ) )
        {
            std::cout << "\nDirectory not found: "
                << outPath.string() << " so we are making it." << std::endl;
            try
            {
                boost::filesystem::create_directory( outPath );
            }
            catch( const std::exception& ex )
            {
                std::cout << ex.what() << std::endl;
            }
        }

        outPath /= p.filename();
        outPath.replace_extension( "ive" );
    }

    boost::filesystem::path tempDirPath = outPath.parent_path() / "tempIve";
    if( !boost::filesystem::exists( tempDirPath ) )
    {
        std::cout << "\nDirectory not found: "
            << tempDirPath.string() << " so we are making it." << std::endl;
        try
        {
            boost::filesystem::create_directory( tempDirPath );
        }
        catch( const std::exception& ex )
        {
            std::cout << ex.what() << std::endl;
        }
    }

    osg::ref_ptr< osgDB::ReaderWriter::Options > osgOpt = new osgDB::ReaderWriter::Options();
    //IntermediateFileNameBase
    std::string osgFileOutput = "IntermediateFileNameBase " + tempDirPath.generic_string();
    osgOpt->setOptionString( osgFileOutput );
    //ShowImportOptions N
    //ShowExportOptions N
    
    //read in osg file
    osg::ref_ptr< osg::Node > tempCADNode = osgDB::readNodeFile( argv[ 1 ], osgOpt.get() );
    if( !tempCADNode.valid() )
    {
        std::cout << "Invalid file loaded" << std::endl;
        return 1;
    }
    
    //Merge the primitive sets and make triangles
    {
        osgwTools::GeometryModifier gm( new osgwTools::Trianglizer() );
        tempCADNode->accept( gm );
    }
    
    //Remove repeated pt nodes
    {
        osg::Node::DescriptionList criteria;
        criteria.push_back( "NUGRAF___AccountedCounter" );
        //criteria.push_back( "AccountedCounter" );
        criteria.push_back( "ok_int: -1" );
        
        RemoveByDesc rbd;
        rbd.setDescriptions( criteria );
        unsigned int count( rbd.execute( tempCADNode.get() ) );
        std::cout << "Removed " << count << " nodes." << std::endl;
    }

    //Remove crufty names from polytrans conversion
    {
        ves::xplorer::scenegraph::util::RemoveNodeNameVisitor polyTransCleanup( tempCADNode.get(), "", "" );
    }

    //Finally reparent repeated nodes
    {
        ShareNodes snv;
        snv.execute( tempCADNode.get() );
    }

    if( !osgDB::writeNodeFile( *tempCADNode.get(), outPath.string() ) )
    {
        std::cout << "Unable to write converted jt file." << std::endl;
    }
    else
    {
        std::cout << "Writing file " << outPath.string() << std::endl;
    }

    // Build up a persistable with some useful test types
    {
        const boost::posix_time::ptime now =
            boost::posix_time::second_clock::local_time();
        boost::posix_time::time_facet* const f =
            new boost::posix_time::time_facet("%d-%b-%Y %H:%M:%S");
        std::ostringstream msg;
        msg.imbue( std::locale( msg.getloc(), f ) );
        msg << now;
        q.AddDatum( "end_onversion_date", msg.str() );
        
        manager.Save( q );
    }

    return 0;
}


