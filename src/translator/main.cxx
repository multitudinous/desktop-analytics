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

int main( int argc, char* argv[] )
{
    if( argc < 2 )
    {
        std::cout << "Usage: jtToive <file to load> " << std::endl;
        return 1;
    }
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

    return 0;
}


