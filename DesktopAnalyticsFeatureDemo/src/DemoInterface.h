/*************** <auto-copyright.pl BEGIN do not edit this line> **************
*
* jag3d is (C) Copyright 2011-2012 by Kenneth Mark Bryden and Paul Martz
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License version 2.1 as published by the Free Software Foundation.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*
*************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef __JAG3D_DEMO_SUPPORT_DEMO_INTERFACE_H__
#define __JAG3D_DEMO_SUPPORT_DEMO_INTERFACE_H__ 1


#include <jagDraw/DrawInfo.h>
#include <jagDraw/Shader.h>
#include <jagDraw/Image.h>
#include <jagSG/Node.h>
#include <jagSG/CollectionVisitor.h>
#include <jagMx/MxCore.h>
#include <jagDisk/ReadWrite.h>
#include <jagBase/LogMacros.h>
#include "ToggleByNameVisitor.h"

#include <gmtl/gmtl.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/foreach.hpp>
#include <boost/any.hpp>


/** \class DemoInterface DemoInterface.h DemoInterface.h
\brief Facilitates building examples, tests, and demos with any
windowing system API, e.g., freeglut, Qt, etc.
*/
class DemoInterface
{
public:
    DemoInterface( const std::string& logName )
      : _logName( logName ),
        _startupCalled( false ),
        _continuousRedraw( false )
    {}
    virtual ~DemoInterface() {}

    static DemoInterface* create( boost::program_options::options_description& desc );
    virtual bool parseOptions( boost::program_options::variables_map& vm ) { return( true ); }

    /** Called before any contexts are created. */
    virtual bool startup( const unsigned int numContexts )
    {
        _startupCalled = true;

        _drawInfo._data.resize( numContexts );
        unsigned int id( 0 );
        BOOST_FOREACH( jagDraw::DrawInfo& drawInfo, _drawInfo._data )
        {
            drawInfo._id = id++;
        }

        return( true );
    }
    bool getStartupCalled() { return( _startupCalled ); }

    void setContinuousRedraw( bool redraw=true ) { _continuousRedraw = redraw; }
    bool getContinuousRedraw() { return( _continuousRedraw ); }
	
    /** Called for each context, after the context has been created. */
    virtual bool init() = 0;
    /** Called to render a frame. */
    virtual bool frame( const gmtl::Matrix44d& view=gmtl::MAT_IDENTITY44D, const gmtl::Matrix44d& proj=gmtl::MAT_IDENTITY44D ) = 0;
    /** Called on window resize. */
    virtual void reshape( const int w, const int h ) {}
    /** Called prior to exit. */
    virtual bool shutdown() { return( true ); }

    /** Simple interface for passing key press events. Return true to redraw. */
    virtual bool keyCommand( const int command ) { return( false ); }

    jagDraw::DrawInfo& getDrawInfo( const jagDraw::jagDrawContextID contextID )
    {
        return( _drawInfo._data[ contextID ] );
    }
    jagSG::CollectionVisitor& getCollectionVisitor()
    {
        return( _collectionVisitor );
    }

    jagMx::MxCorePtr getMxCore( const jagDraw::jagDrawContextID contextID )
    {
        if( _mxCore._data.empty() )
            return( jagMx::MxCorePtr( (jagMx::MxCore*)NULL ) );
        else
            return( _mxCore._data[ contextID ] );
    }

    // Return a value to bontrol base gamepad move rate in the scene.
    virtual double getMoveRate() const { return( 1. ); }

protected:
    std::string _logName;
    bool _startupCalled;

    bool _continuousRedraw;

    jagDraw::PerContextDrawInfo _drawInfo;
    jagSG::CollectionVisitor _collectionVisitor;

    typedef jagDraw::PerContextData< jagMx::MxCorePtr > PerContextMxCore;
    PerContextMxCore _mxCore;



#define __READ_UTIL( _RESULT, _TYPE, _NAME, _OPT ) \
    { \
        boost::any anyTemp( jagDisk::read( _NAME, _OPT ) ); \
        try { \
            _RESULT = boost::any_cast< _TYPE >( anyTemp ); \
        } \
        catch( boost::bad_any_cast bac ) \
        { \
            bac.what(); \
        } \
        if( _RESULT == NULL ) \
        { \
            JAG3D_FATAL_STATIC( _logName, std::string("Can't load \"") + std::string(_NAME) + std::string("\".") ); \
        } \
    }

    jagSG::NodePtr readSceneGraphNodeUtil( const std::string& fileName, const jagDisk::Options* options=NULL )
    {
        jagSG::NodePtr result;
        __READ_UTIL( result, jagSG::NodePtr, fileName, options );
        return( result );
    }
    jagDraw::ImagePtr readImageUtil( const std::string& fileName, const jagDisk::Options* options=NULL )
    {
        jagDraw::ImagePtr result;
        __READ_UTIL( result, jagDraw::ImagePtr, fileName, options );
        return( result );
    }
    jagDraw::ShaderPtr readShaderUtil( const std::string& fileName, const jagDisk::Options* options=NULL )
    {
        jagDraw::ShaderPtr result;
        __READ_UTIL( result, jagDraw::ShaderPtr, fileName, options );
        return( result );
    }
};


// __JAG3D_DEMO_SUPPORT_DEMO_INTERFACE_H__
#endif
