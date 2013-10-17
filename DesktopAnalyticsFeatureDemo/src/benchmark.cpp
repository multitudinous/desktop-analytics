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

#include <demoSupport/DemoInterface.h>

#include <jagDraw/Common.h>
#include <jagDraw/types.h>
#include <jagBase/Profile.h>
#include <jagBase/Version.h>
#include <jagBase/Log.h>
#include <jagBase/LogMacros.h>
#include <Poco/Foundation.h>
#include <boost/program_options/options_description.hpp>
#include <gmtl/gmtl.h>

#include <boost/foreach.hpp>

#include <string>



const size_t numTestCases( 5 );
const unsigned int numVerts( 1000000 );
const unsigned int trisPerCommand( 6 );


using namespace std;
namespace bpo = boost::program_options;


class BenchmarkTest : public DemoInterface
{
public:
    BenchmarkTest()
      : DemoInterface( "jag.test.bench" )
    {}
    virtual ~BenchmarkTest() {}

    virtual bool startup( const unsigned int numContexts );
    virtual bool init();
    virtual bool frame( const gmtl::Matrix44d& view, const gmtl::Matrix44d& proj );
    virtual bool shutdown()
    {
        return( true );
    }

protected:
    jagDraw::DrawableVec _drawables;
    jagDraw::CommandMap _commands;
};


DemoInterface* DemoInterface::create( bpo::options_description& desc )
{
    jagBase::Log::instance()->setPriority( jagBase::Log::PrioDebug, jagBase::Log::Console );

    return( new BenchmarkTest );
}


jagDraw::BufferObjectPtr createTriangleVertices( unsigned int n,
    const float x = .05f, const float y = .05f, const float z = .5f )
{
    const float xMin=-.9f;
    const float xMax=.9f;
    const float yMin=-.9f;
    const float yMax=.9f;
    jagBase::Point3fVec v3fa( n );

    float yVal( yMin );
    float yInc( y );
    for( unsigned int idx=0; idx<n; ++idx )
    {
        float xVal( ( idx & 1 ) ? ( xMin + x ) : xMin );
        xVal = std::min( xVal, xMax );

        v3fa[ idx ].set( xVal, yVal, z );

        if( idx & 1 )
        {
            yVal += yInc;
            if( yVal > yMax )
            {
                yVal = yMax;
                yInc = -yInc;
            }
            else if( yVal < yMin )
            {
                yVal = yMin;
                yInc = -yInc;
            }
        }
    }
    jagBase::BufferPtr vbp( new jagBase::Buffer( v3fa.size() * sizeof( gmtl::Point3f ), (void*)&v3fa[0] ) );

    return( jagDraw::BufferObjectPtr( new jagDraw::BufferObject( GL_ARRAY_BUFFER, vbp ) ) );
}

jagDraw::ProgramPtr createProgram()
{
    const char* vShaderSource =
#if( POCO_OS == POCO_OS_MAC_OS_X )
        // In OSX 10.7/10.8, use GL 3.2 and GLSL 1.50
        "#version 150 \n"
#else
        "#version 400 \n"
#endif
        "in vec3 vertex; \n"
        "void main() { \n"
        "    gl_Position = vec4( vertex, 1. ); \n"
        "}";
    jagDraw::ShaderPtr vs( new jagDraw::Shader( GL_VERTEX_SHADER ) );
    vs->addSourceString( std::string( vShaderSource ) );

    const char* fShaderSource =
#if( POCO_OS == POCO_OS_MAC_OS_X )
        // In OSX 10.7/10.8, use GL 3.2 and GLSL 1.50
        "#version 150 \n"
#else
        "#version 400 \n"
#endif
        "out vec4 colorOut; \n"
        "void main() { \n"
        "    colorOut = vec4( 1. ); \n"
        "}";
    jagDraw::ShaderPtr fs( new jagDraw::Shader( GL_FRAGMENT_SHADER ) );
    fs->addSourceString( std::string( fShaderSource ) );

    jagDraw::ProgramPtr prog;
    prog = jagDraw::ProgramPtr( new jagDraw::Program );
    prog->attachShader( vs );
    prog->attachShader( fs );

    return( prog );
}

void createSingleTriStrip( const unsigned int numVerts, jagDraw::DrawablePtr& drawable )
{
    drawable = jagDraw::DrawablePtr( new jagDraw::Drawable() );

    jagDraw::GLuintVec elements;
    unsigned int idx;
    for( idx=0; idx<numVerts; ++idx )
        elements.push_back( idx );
    jagBase::BufferPtr elbp( new jagBase::Buffer( elements.size() * sizeof( GLint ), (void*)&elements[0] ) );
    jagDraw::BufferObjectPtr elbop( new jagDraw::BufferObject( GL_ELEMENT_ARRAY_BUFFER, elbp ) );
    jagDraw::DrawElementsPtr drawElements( new jagDraw::DrawElements( GL_TRIANGLE_STRIP, (const GLsizei) elements.size(), GL_UNSIGNED_INT, 0, elbop ) );

    drawable->addDrawCommand( drawElements );

    drawable->setMaxContexts( 1 );
}
void createSeveralTriStrips( const unsigned int numVerts, const unsigned int trisPerCommand, jagDraw::DrawablePtr& drawable )
{
    drawable = jagDraw::DrawablePtr( new jagDraw::Drawable() );

    jagDraw::GLuintVec elements;
    const unsigned int vertsPerCommand( trisPerCommand + 2 );
    const unsigned int totalTris( numVerts - 2 );
    const unsigned int intCommandCount( totalTris/trisPerCommand );
    const unsigned int totalCommands( ( (float)totalTris/(float)trisPerCommand == intCommandCount ) ?
        intCommandCount : intCommandCount+1 );

    unsigned int vertsRemaining( numVerts );
    unsigned int startIdx( 0 );
    for( unsigned int numCommands = totalCommands; numCommands > 0; --numCommands )
    {
        unsigned int idx;
        unsigned int currentVertCount( std::min( vertsPerCommand, vertsRemaining ) );
        for( idx=0; idx < currentVertCount; idx++ )
            elements.push_back( idx + startIdx );
        vertsRemaining -= trisPerCommand;
        startIdx += trisPerCommand;
    }

    jagBase::BufferPtr elbp( new jagBase::Buffer( elements.size() * sizeof( GLint ), (void*)&elements[0] ) );
    jagDraw::BufferObjectPtr elbop( new jagDraw::BufferObject( GL_ELEMENT_ARRAY_BUFFER, elbp ) );

    vertsRemaining = numVerts;
    startIdx = 0;
    for( unsigned int numCommands = totalCommands; numCommands > 0; --numCommands )
    {
        unsigned int currentVertCount( std::min( vertsPerCommand, vertsRemaining ) );
        jagDraw::DrawElementsPtr drawElements( new jagDraw::DrawElements( GL_TRIANGLE_STRIP,
            (const GLsizei)currentVertCount, GL_UNSIGNED_INT, (const GLvoid*)(startIdx*sizeof(GLint)), elbop ) );
        drawable->addDrawCommand( drawElements );
        vertsRemaining -= trisPerCommand;
        startIdx += vertsPerCommand;
    }
    std::cout << drawable->getDrawCommandVec().size() << std::endl;

    drawable->setMaxContexts( 1 );
}
void createSingleTriStripRestart( const unsigned int numVerts, const unsigned int trisPerCommand, jagDraw::DrawablePtr& drawable )
{
    drawable = jagDraw::DrawablePtr( new jagDraw::Drawable() );

    const unsigned int vertsPerCommand( trisPerCommand + 2 );

    jagDraw::GLuintVec elements;
    unsigned int vertsRemaining( numVerts );
    unsigned int idx;
    for( idx=0; idx<numVerts;  )
    {
        unsigned int currentVertCount( std::min( vertsPerCommand, vertsRemaining ) );
        for( unsigned int n=0; n<currentVertCount; ++n )
        {
            elements.push_back( idx++ );
        }
        if( idx >= numVerts )
            break;
        elements.push_back( 0xffffffff );
        idx -= 2;
        vertsRemaining -= trisPerCommand;
    }
    jagBase::BufferPtr elbp( new jagBase::Buffer( elements.size() * sizeof( GLint ), (void*)&elements[0] ) );
    jagDraw::BufferObjectPtr elbop( new jagDraw::BufferObject( GL_ELEMENT_ARRAY_BUFFER, elbp ) );

    jagDraw::PrimitiveRestartPtr restart( new jagDraw::PrimitiveRestart( 0xffffffff ) );
    drawable->addDrawCommand( restart );

    jagDraw::DrawElementsPtr drawElements( new jagDraw::DrawElements( GL_TRIANGLE_STRIP, (const GLsizei) elements.size(), GL_UNSIGNED_INT, 0, elbop ) );
    drawable->addDrawCommand( drawElements );

    restart.reset( new jagDraw::PrimitiveRestart( 0, false ) );
    drawable->addDrawCommand( restart );

    drawable->setMaxContexts( 1 );
}
void createMultiTriStrip( const unsigned int numVerts, const unsigned int trisPerCommand, jagDraw::DrawablePtr& drawable )
{
    drawable = jagDraw::DrawablePtr( new jagDraw::Drawable() );

    const unsigned int vertsPerCommand( trisPerCommand + 2 );
    const unsigned int totalTris( numVerts - 2 );
    const unsigned int intCommandCount( totalTris/trisPerCommand );
    const GLsizei primCount( ( (float)totalTris/(float)trisPerCommand == intCommandCount ) ?
        intCommandCount : intCommandCount+1 );

    jagDraw::GLuintVec elements;
    jagDraw::GLsizeiVec counts( primCount );
    jagDraw::GLvoidPtrVec indices( primCount );

    unsigned int vertsRemaining( numVerts );
    unsigned int idx;
    unsigned int prim( 0 );
    for( idx=0; idx<numVerts;  )
    {
        unsigned int currentVertCount( std::min( vertsPerCommand, vertsRemaining ) );

        indices[ prim ] = (GLvoid*)( elements.size() * sizeof( GLint ) );
        counts[ prim ] = currentVertCount;
        for( unsigned int n=0; n<currentVertCount; ++n )
        {
            elements.push_back( idx++ );
        }
        if( idx >= numVerts )
            break;
        idx -= 2;
        vertsRemaining -= trisPerCommand;
        ++prim;
    }
    jagBase::BufferPtr elbp( new jagBase::Buffer( elements.size() * sizeof( GLint ), (void*)&elements[0] ) );
    jagDraw::BufferObjectPtr elbop( new jagDraw::BufferObject( GL_ELEMENT_ARRAY_BUFFER, elbp ) );

    jagDraw::MultiDrawElementsPtr multiDrawElements(
        new jagDraw::MultiDrawElements( GL_TRIANGLE_STRIP, counts, GL_UNSIGNED_INT, indices, primCount, elbop ) );
    drawable->addDrawCommand( multiDrawElements );

    drawable->setMaxContexts( 1 );
}
void createSingleTriangles( const unsigned int numVerts, jagDraw::DrawablePtr& drawable )
{
    drawable = jagDraw::DrawablePtr( new jagDraw::Drawable() );

    jagDraw::GLuintVec elements;
    unsigned int idx;
    for( idx=0; idx<(numVerts-2); idx+=2 )
    {
        elements.push_back( idx );
        elements.push_back( idx+1 );
        elements.push_back( idx+2 );
        elements.push_back( idx+2 );
        elements.push_back( idx+1 );
        elements.push_back( idx+3 );
    }
    jagBase::BufferPtr elbp( new jagBase::Buffer( elements.size() * sizeof( GLint ), (void*)&elements[0] ) );
    jagDraw::BufferObjectPtr elbop( new jagDraw::BufferObject( GL_ELEMENT_ARRAY_BUFFER, elbp ) );
    jagDraw::DrawElementsPtr drawElements( new jagDraw::DrawElements( GL_TRIANGLES, (const GLsizei) elements.size(), GL_UNSIGNED_INT, 0, elbop ) );

    drawable->addDrawCommand( drawElements );

    drawable->setMaxContexts( 1 );
}



bool BenchmarkTest::startup( const unsigned int numContexts )
{
    DemoInterface::startup( numContexts );

    // Set up CommandMap with Program and VAO.
    {
        _commands.insert( createProgram() );

        jagDraw::BufferObjectPtr vbop( createTriangleVertices( numVerts ) );
        jagDraw::VertexArrayObjectPtr vaop( new jagDraw::VertexArrayObject );
        vaop->addVertexArrayCommand( vbop, jagDraw::VertexArrayObject::Vertex );
        jagDraw::VertexAttribPtr verts( new jagDraw::VertexAttrib(
            "vertex", 3, GL_FLOAT, GL_FALSE, 0, 0 ) );
        vaop->addVertexArrayCommand( verts, jagDraw::VertexArrayObject::Vertex );
        _commands.insert( vaop );

        _commands.setMaxContexts( 1 );
    }

    // Create Drawables for all test cases
    _drawables.resize( numTestCases );
    createSingleTriStrip( numVerts, _drawables[ 0 ] );
    createSeveralTriStrips( numVerts, trisPerCommand, _drawables[ 1 ] );
    createSingleTriStripRestart( numVerts, trisPerCommand, _drawables[ 2 ] );
    createMultiTriStrip( numVerts, trisPerCommand, _drawables[ 3 ] );
    createSingleTriangles( numVerts, _drawables[ 4 ] );

    return( true );
}

bool BenchmarkTest::init()
{
    glClearColor( 0.f, 0.f, 0.f, 0.f );

    // Auto-log the version string.
    jagBase::getVersionString();

    // Auto-log the OpenGL version string.
    jagDraw::getOpenGLVersionString();

    JAG3D_ERROR_CHECK( "BenchmarkTest init()" );
    return( true );
}

bool BenchmarkTest::frame( const gmtl::Matrix44d& view, const gmtl::Matrix44d& proj )
{
    if( !getStartupCalled() )
        return( true );

    glClear( GL_COLOR_BUFFER_BIT );

    const jagDraw::jagDrawContextID contextID( jagDraw::ContextSupport::instance()->getActiveContext() );
    jagDraw::DrawInfo& drawInfo( getDrawInfo( contextID ) );

    _commands.execute( drawInfo );
    // Clear GL pipe before timing anything.
    glFinish();

    if( true )
    {
        JAG3D_PROFILE( "Single GL_TRIANGLE_STRIP command" );
        _drawables[ 0 ]->execute( drawInfo );
        glFinish();
    }
    if( true )
    {
        JAG3D_PROFILE( "Several GL_TRIANGLE_STRIP commands" );
        _drawables[ 1 ]->execute( drawInfo );
        glFinish();
    }
    if( true )
    {
        JAG3D_PROFILE( "Single GL_TRIANGLE_STRIP command with restart" );
        _drawables[ 2 ]->execute( drawInfo );
        glFinish();
    }
    if( true )
    {
        JAG3D_PROFILE( "Multi GL_TRIANGLE_STRIP command" );
        _drawables[ 3 ]->execute( drawInfo );
        glFinish();
    }
    if( true )
    {
        JAG3D_PROFILE( "Single GL_TRIANGLES command" );
        _drawables[ 4 ]->execute( drawInfo );
        glFinish();
    }
    
    glFlush();
    JAG3D_ERROR_CHECK( "BenchmarkTest display()" );

    return( true );
}
