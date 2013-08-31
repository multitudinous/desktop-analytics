#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>
#include <osgDB/ReaderWriter>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osg/Node>

#include <iostream>
#include <string>

#include <boost/filesystem/path.hpp>

#include "RemoveNodeNameVisitor.h"

#include <crunchstore/Persistable.h>
#include <crunchstore/Datum.h>
#include <crunchstore/DataManager.h>
#include <crunchstore/NullBuffer.h>
#include <crunchstore/NullCache.h>
#include <crunchstore/DataAbstractionLayer.h>
#include <crunchstore/SQLiteStore.h>

using namespace crunchstore;

int main( int argc, char* argv[] )
{
    if( argc < 2 )
    {
        std::cout << "Usage: jtToive <file to load> " << std::endl;
        return 1;
    }
    
    osgDB::Registry::instance()->loadLibrary( "osgdb_PolyTrans.dll" );
    
    //read in osg file
    osg::ref_ptr< osg::Node > tempCADNode = osgDB::readNodeFile( argv[ 1 ] );
    if( !tempCADNode.valid() )
    {
        std::cout << "Invalid file loaded" << std::endl;
        return 1;
    }
    
    boost::filesystem::path p( argv[ 1 ] );
    p.replace_extension( "ive" );
    //Remove crufty names from polytrans conversion
    ves::xplorer::scenegraph::util::RemoveNodeNameVisitor polyTransCleanup( tempCADNode.get(), "", "" );

    bool success = osgDB::writeNodeFile( *tempCADNode.get(), p.string() );
    if( !success )
    {
        std::cout << "Unable to write converted jt file." << std::endl;
    }
    
    // Set up a datamanager to test persistence
    DataManager manager;
    DataAbstractionLayerPtr cache( new NullCache );
    DataAbstractionLayerPtr buffer( new NullBuffer );
    manager.SetCache( cache );
    manager.SetBuffer( buffer );
    
#ifndef USE_MONGODB
    // Add an SQLite store
    //DataAbstractionLayerPtr sqstore( new SQLiteStore );
    //static_cast<SQLiteStore*>(sqstore.get())->SetStorePath( "/tmp/DALTest.db" );
    SQLiteStorePtr sqstore( new SQLiteStore );
    sqstore->SetStorePath( "/tmp/DALTest.db" );
    manager.AttachStore( sqstore, Store::WORKINGSTORE_ROLE );
#else
    // Add a mongoDB store
    DataAbstractionLayerPtr mongostore( new MongoStore );
    static_cast<MongoStore*>(mongostore.get())->SetStorePath("localhost");
    //manager.AttachStore( mongostore, Store::BACKINGSTORE_ROLE );
    manager.AttachStore( mongostore, Store::WORKINGSTORE_ROLE );
#endif
    
    // Build up a persistable with some useful test types
    Persistable q;
    q.SetTypeName( "TestType" );
    q.AddDatum( "Num", 1234.98735 );
    q.AddDatum( "ABool", true );
    q.AddDatum( "AString", std::string("This is a test") );
    q.AddDatum( "AnInt", 19 );


    return 0;
}


