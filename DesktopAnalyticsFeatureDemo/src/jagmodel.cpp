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

//#define JAG3D_USE_GL3W

//#include "DemoInterface.h"
#include "jagmodel.h"
#include <jagDraw/PlatformOpenGL.h>
//#include <jagDraw/gl3w.h>

#include <jagDraw/Common.h>
#include <jagDraw/PerContextData.h>
#include <jagSG/CollectionVisitor.h>
#include <jagSG/Node.h>
#include <jagSG/SmallFeatureCallback.h>
#include <jagDisk/ReadWrite.h>
#include <jagBase/Profile.h>
#include <jagBase/Version.h>
#include <jagBase/Log.h>
#include <jagBase/LogMacros.h>
#include <jagSG/Common.h>
#include <jagUtil/Shapes.h>
#include <jagSG/NodeMaskCullCallback.h>
#include <jagSG/IntersectVisitor.h>
#include <time.h>
#include <jagUtil/BufferAggregationVisitor.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <gmtl/gmtl.h>

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include <string>
#include <sstream>


using namespace std;
namespace bpo = boost::program_options;





DemoInterface* DemoInterface::create( bpo::options_description& desc )
{
	desc.add_options()
		( "file,f", bpo::value< std::string >(), "Model to load. Default: Jansens.osg" );

	return( new JagModel );
}

bool JagModel::parseOptions( bpo::variables_map& vm )
{
	if( vm.count( "file" ) > 0 )
		_fileName = vm[ "file" ].as< std::string >();
	return( true );
}





bool JagModel::startup( const unsigned int numContexts )
{
	DemoInterface::startup( numContexts );

	JAG3D_INFO_STATIC( _logName, _fileName );

	if( _fileName.empty() )
	{
		JAG3D_FATAL_STATIC( _logName, "Specify '--file <fileName>' on command line." );
		return( false );
	}


	// Prepare the draw graph.
	jagDraw::DrawGraphPtr drawGraphTemplate( new jagDraw::DrawGraph() );
	drawGraphTemplate->resize( 1 );
	getCollectionVisitor().setDrawGraphTemplate( drawGraphTemplate );

	jagDraw::BoundPtr smallBound = jagDraw::BoundPtr(new jagDraw::BoundSphere(gmtl::Sphere<double>(gmtl::Point3d(0,0,0),.5)));

	_root = jagSG::NodePtr ( new jagSG::Node() );

	jagSG::NodePtr xform1( boost::make_shared< jagSG::Node >()),xform2( boost::make_shared< jagSG::Node >()),xform3( boost::make_shared< jagSG::Node >());
	jagSG::NodePtr model1= DemoInterface::readSceneGraphNodeUtil( "1h.ive" );
	jagSG::NodePtr model2= DemoInterface::readSceneGraphNodeUtil( "2hp1.ive" );
	jagSG::NodePtr model3= DemoInterface::readSceneGraphNodeUtil( "2hp2a.ive" );
	jagSG::NodePtr model4= DemoInterface::readSceneGraphNodeUtil( "2hp2b.ive" );
	jagSG::NodePtr model5= DemoInterface::readSceneGraphNodeUtil( "3h.ive" );
	jagSG::NodePtr cow= DemoInterface::readSceneGraphNodeUtil( "cow.osg" );
	_root->addChild( model1 );
	_root->addChild( model2 );
	_root->addChild( model3 );
	_root->addChild( model4 );
	_root->addChild( model5 );


	{ \
		boost::any anyTemp( jagDisk::write( "out.txt", (void*)_root.get(), NULL ) ); \
		try { \
		bool didit = boost::any_cast< bool >( anyTemp ); \
	} \
	catch( boost::bad_any_cast bac ) \
	{ \
	bac.what(); \
	} \

	}





	if( _root == NULL )
	{
		JAG3D_FATAL_STATIC( _logName, "Can't load \"" + _fileName + "\"." );
		return( false );
	}

	jagSG::NodeMaskCullDistributionVisitor nmdv;
	_root->accept( nmdv );

	jagSG::SmallFeatureDistributionVisitor sfdv;

	_root->accept( sfdv );

	jagSG::FrustumCullDistributionVisitor fcdv;
	_root->accept( fcdv);

	jagUtil::BufferAggregationVisitor bav( _root );


	jagDraw::ShaderPtr vs( DemoInterface::readShaderUtil( "jagmodel.vert" ) );
	jagDraw::ShaderPtr fs( DemoInterface::readShaderUtil( "jagmodel.frag" ) );
	if( ( vs == NULL ) || ( fs == NULL ) )
	{
		JAG3D_INFO_STATIC( _logName, "Unable to load shaders. Set JAG3D_DATA_PATH in the environment." );
		return( false );
	}

	jagDraw::ProgramPtr prog;
	prog = jagDraw::ProgramPtr( new jagDraw::Program );
	prog->attachShader( vs );
	prog->attachShader( fs );

	jagDraw::CommandMapPtr commands( _root->getCommandMap() );
	if( commands == NULL )
	{
		commands = jagDraw::CommandMapPtr( new jagDraw::CommandMap() );
		_root->setCommandMap( commands );
	}
	commands->insert( prog );










	// Set up lighting uniforms
	jagDraw::UniformBlockPtr lights( jagDraw::UniformBlockPtr(
		new jagDraw::UniformBlock( "LightingLight" ) ) );
	gmtl::Vec3f dir( 0.f, 0.f, 1.f );
	gmtl::normalize( dir );
	gmtl::Point4f lightVec( dir[0], dir[1], dir[2], 0. );
	lights->addUniform( jagDraw::UniformPtr(
		new jagDraw::Uniform( "position", lightVec ) ) );
	lights->addUniform( jagDraw::UniformPtr(
		new jagDraw::Uniform( "ambient", gmtl::Point4f( 0.f, 0.f, 0.f, 1.f ) ) ) );
	lights->addUniform( jagDraw::UniformPtr(
		new jagDraw::Uniform( "diffuse", gmtl::Point4f( 1.f, 1.f, 1.f, 1.f ) ) ) );
	lights->addUniform( jagDraw::UniformPtr(
		new jagDraw::Uniform( "specular", gmtl::Point4f( 1.f, 1.f, 1.f, 1.f ) ) ) );

	jagDraw::UniformBlockPtr backMaterials( jagDraw::UniformBlockPtr(
		new jagDraw::UniformBlock( "LightingMaterialBack" ) ) );
	backMaterials->addUniform( jagDraw::UniformPtr(
		new jagDraw::Uniform( "ambient", gmtl::Point4f( .1f, .1f, .1f, 1.f ) ) ) );
	backMaterials->addUniform( jagDraw::UniformPtr(
		new jagDraw::Uniform( "diffuse", gmtl::Point4f( .7f, .7f, .7f, 1.f ) ) ) );
	backMaterials->addUniform( jagDraw::UniformPtr(
		new jagDraw::Uniform( "specular", gmtl::Point4f( .5f, .5f, .5f, 1.f ) ) ) );
	backMaterials->addUniform( jagDraw::UniformPtr(
		new jagDraw::Uniform( "shininess", 16.f ) ) );

	jagDraw::UniformBlockPtr frontMaterials( jagDraw::UniformBlockPtr(
		new jagDraw::UniformBlock( "LightingMaterialFront" ) ) );
	frontMaterials->addUniform( jagDraw::UniformPtr(
		new jagDraw::Uniform( "ambient", gmtl::Point4f( .1f, .1f, .1f, 1.f ) ) ) );
	frontMaterials->addUniform( jagDraw::UniformPtr(
		new jagDraw::Uniform( "diffuse", gmtl::Point4f( .7f, .7f, .7f, 1.f ) ) ) );
	frontMaterials->addUniform( jagDraw::UniformPtr(
		new jagDraw::Uniform( "specular", gmtl::Point4f( .5f, .5f, .5f, 1.f ) ) ) );
	frontMaterials->addUniform( jagDraw::UniformPtr(
		new jagDraw::Uniform( "shininess", 16.f ) ) );

	jagDraw::UniformBlockSetPtr ubsp( jagDraw::UniformBlockSetPtr(
		new jagDraw::UniformBlockSet() ) );
	ubsp->insert( lights );
	ubsp->insert( backMaterials );
	ubsp->insert( frontMaterials );
	commands->insert( ubsp );


	//_pass1 = jagSG::NodePtr(new jagSG::Node());
	//_pass1->addChild(_root);

	// We keep a different project matrix per context (to support different
	// window sizes). Initialize them all to a reasonable default.
	// We have potentially different views per window, so we keep an MxCore
	// per context. Initialize the MxCore objects and create default views.
	const jagDraw::BoundPtr bound( _root->getBound() );
	const gmtl::Point3d pos( bound->getCenter() + gmtl::Vec3d( 0., -1., 0. ) );
	for( unsigned int idx( 0 ); idx<numContexts; ++idx )
	{
		jagMx::MxCorePtr mxCore( new jagMx::MxCore() );
		mxCore->setAspect( 1. );
		mxCore->setFovy( 30. );
		mxCore->setPosition( pos );
		mxCore->setOrbitCenterPoint( bound->getCenter() );
		mxCore->lookAtAndFit( bound->asSphere() );
		_mxCore._data.push_back( mxCore );
	}

	{
		//commands = _pass1->getOrCreateCommandMap();
		//	_texWidth = 1000;
		//_texHeight = 1000;
		jagDraw::ImagePtr image( new jagDraw::Image() );
		image->set( 0, GL_RGBA, _texWidth, _texHeight, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
		jagDraw::TexturePtr tex( new jagDraw::Texture( GL_TEXTURE_2D, image,
			jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
		tex->getSampler()->getSamplerState()->_minFilter = GL_LINEAR;

		// Create an FBO and attach the texture.
		_textureFBO = jagDraw::FramebufferPtr( new jagDraw::Framebuffer(GL_DRAW_FRAMEBUFFER ) );
		_textureFBO->setViewport( 0, 0, _texWidth, _texHeight );
		_textureFBO->setClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		_textureFBO->addAttachment( GL_COLOR_ATTACHMENT0, tex );

		_texWidth = 1000;
		_texHeight = 1000;
		//create the second texture to render into
		jagDraw::ImagePtr image2( new jagDraw::Image() );
		image2->set( 0, GL_RGBA, _texWidth, _texHeight, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
		jagDraw::TexturePtr tex2( new jagDraw::Texture( GL_TEXTURE_2D, image2,
			jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
		tex2->getSampler()->getSamplerState()->_minFilter = GL_LINEAR;


		jagDraw::ImagePtr image3( new jagDraw::Image() );
		image3->set( 0, GL_RGBA, _texWidth, _texHeight, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
		jagDraw::TexturePtr tex3( new jagDraw::Texture( GL_TEXTURE_2D, image3,
			jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
		tex3->getSampler()->getSamplerState()->_minFilter = GL_LINEAR;



		//attach the second texture
		_textureFBO->addAttachment( GL_COLOR_ATTACHMENT1, tex2 );



		//create the second texture to render into
		/*jagDraw::ImagePtr image2( new jagDraw::Image() );
		image2->set( 0, GL_DEPTH_COMPONENT, _texWidth, _texHeight, 1, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL );

		jagDraw::TexturePtr tex2( new jagDraw::Texture( GL_TEXTURE_2D, image2,
		jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
		tex2->getSampler()->getSamplerState()->_minFilter = GL_NEAREST;*/

		jagDraw::RenderbufferPtr rbdb = jagDraw::RenderbufferPtr(new jagDraw::Renderbuffer(GL_DEPTH_COMPONENT, 1000, 1000, 0, ""));
		_textureFBO->addAttachment(GL_DEPTH_ATTACHMENT, rbdb);
		//commands->insert(rbdb);

		// attach the second texture
		// _textureFBO->addAttachment( GL_DEPTH_ATTACHMENT, tex2 );


		commands->insert(_textureFBO);

		_pass2FBO = jagDraw::FramebufferPtr( new jagDraw::Framebuffer(GL_DRAW_FRAMEBUFFER ) ); 
		_pass2FBO->setViewport( 0, 0, _texWidth, _texHeight );
		_pass2FBO->setClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
		_pass2FBO->addAttachment( GL_COLOR_ATTACHMENT0, tex3 );

		_pass3FBO = jagDraw::FramebufferPtr( new jagDraw::Framebuffer(GL_DRAW_FRAMEBUFFER ) ); 
		_pass3FBO->setViewport( 0, 0, _texWidth, _texHeight );
		_pass3FBO->setClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		_pass3FBO->addAttachment( GL_COLOR_ATTACHMENT0, tex2 );

		_pass2 = jagSG::NodePtr(new jagSG::Node());
		_pass2->addChild(_root);
		commands = _pass2->getOrCreateCommandMap();

		const char* blurFS = 
			"#version 400 \n"
			"out vec4 colorOut; \n"

			"uniform sampler2D texture, texture2; \n"
			"in vec2 tcOut; \n"
			"uniform float PixOffset[5] = float[](0.0,3.0,6.0,9.0,12.0);\n"
			"uniform float Weight[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216,\n"
			"0.0540540541, 0.0162162162 );\n"
			"uniform int Height = 1000;\n"
			"uniform int Width = 1000;\n"

			" vec4 pass2()\n"
			"{\n"
			" float dy = 1.0 / float(Height);\n"

			"vec4 sum = texture2D(texture2, tcOut) * Weight[0];\n"
			"for( int i = 1; i < 5; i++ )\n"
			"{\n"
			"sum += texture2D( texture2, tcOut + vec2(0.0,PixOffset[i]) * dy ) * Weight[i];\n"
			"sum += texture2D( texture2, tcOut - vec2(0.0,PixOffset[i]) * dy ) * Weight[i];\n"
			"}\n"
			"return sum;\n"
			"}\n"
			"void main() { \n"
			"    colorOut = pass2(); \n"
			//"    colorOut = texture2D( texture, tcOut )+texture2D(texture2, tcOut);// + vec4( .5, 0., 0., 0. ); \n"
			//"    colorOut = vec4( tcOut, 0., 1. ); \n"
			"}";



		jagDraw::ShaderPtr blfs( new jagDraw::Shader( GL_FRAGMENT_SHADER ) );
		blfs->addSourceString( std::string( blurFS ) );



		const char* blurFSH = 
			"#version 400 \n"
			"out vec4 colorOut; \n"

			"uniform sampler2D texture, texture2, texture3; \n"
			"in vec2 tcOut; \n"
			"uniform float PixOffset[5] = float[](0.0,3.0,6.0,9.0,12.0);\n"
			"uniform float Weight[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216,\n"
			"0.0540540541, 0.0162162162 );\n"
			"uniform int Height = 1000;\n"
			"uniform int Width = 1000;\n"

			" vec4 pass2()\n"
			"{\n"
			" float dy = 1.0 / float(Height);\n"

			"vec4 sum = texture2D(texture3, tcOut) * Weight[0];\n"
			"for( int i = 1; i < 5; i++ )\n"
			"{\n"
			"sum += texture2D( texture3, tcOut + vec2(PixOffset[i],0.0) * dy ) * Weight[i];\n"
			"sum += texture2D( texture3, tcOut - vec2(PixOffset[i],0.0) * dy ) * Weight[i];\n"
			"}\n"
			"return sum;\n"
			"}\n"
			"void main() { \n"
			"    colorOut = pass2(); \n"
			//"    colorOut = texture2D( texture, tcOut )+texture2D(texture2, tcOut);// + vec4( .5, 0., 0., 0. ); \n"
			//"    colorOut = vec4( tcOut, 0., 1. ); \n"
			"}";



		jagDraw::ShaderPtr blfsh( new jagDraw::Shader( GL_FRAGMENT_SHADER ) );
		blfsh->addSourceString( std::string( blurFSH ) );

		// Now set up for drawing a textured quad.

		const char* vertSource =
#if( POCO_OS == POCO_OS_MAC_OS_X )
			// In OSX 10.7/10.8, use GL 3.2 and GLSL 1.50
			"#version 150 \n"
#else
			"#version 400 \n"
#endif
			"in vec3 vertex; \n"
			"in vec2 texcoord; \n"
			"out vec2 tcOut; \n"
			"void main() { \n"
			"    gl_Position = vec4( vertex, 1. ); \n"
			"    tcOut = texcoord; \n"
			"}";
		jagDraw::ShaderPtr vs( new jagDraw::Shader( GL_VERTEX_SHADER ) );
		vs->addSourceString( std::string( vertSource ) );

		jagDraw::ProgramPtr blurprog( new jagDraw::Program );
		blurprog->attachShader( vs );
		blurprog->attachShader( blfs );

		jagDraw::ProgramPtr blurprogh( new jagDraw::Program );
		blurprogh->attachShader( vs );
		blurprogh->attachShader( blfsh );



		const char* fragSource =
#if( POCO_OS == POCO_OS_MAC_OS_X )
			// In OSX 10.7/10.8, use GL 3.2 and GLSL 1.50
			"#version 150 \n"
#else
			"#version 400 \n"
#endif
			"uniform sampler2D texture, texture2, texture3; \n"
			"in vec2 tcOut; \n"
			"out vec4 colorOut; \n"
			"void main() { \n"
			"vec4 glow = texture2D(texture2, tcOut);\n" 
			"    colorOut = texture2D(texture, tcOut);//*(1.0-glow[3]) + glow;// + .5*texture2D( texture2, tcOut ); \n"
			//"    colorOut = texture2D( texture, tcOut )+texture2D(texture2, tcOut);// + vec4( .5, 0., 0., 0. ); \n"
			//"    colorOut = vec4( tcOut, 0., 1. ); \n"
			"}";
		jagDraw::ShaderPtr fs( new jagDraw::Shader( GL_FRAGMENT_SHADER ) );
		fs->addSourceString( std::string( fragSource ) );

		jagDraw::ProgramPtr prog( new jagDraw::Program );
		prog->attachShader( vs );
		prog->attachShader( fs );
		// Create an FBO for the default framebuffer (the window)
		_defaultFBO = jagDraw::FramebufferPtr( new jagDraw::Framebuffer( GL_DRAW_FRAMEBUFFER ) );
		_defaultFBO->setViewport( 0, 0, 1000, 1000 );

		const float z = .5f;
		float vertices[] = {
			-1., -1., z,
			0., 0.,
			1., -1., z,
			1., 0.,
			-1., 1., z,
			0., 1.,
			1., 1., z,
			1., 1. };
		jagBase::BufferPtr ibp( new jagBase::Buffer( sizeof( vertices ), (void*)vertices ) );
		jagDraw::BufferObjectPtr ibop( new jagDraw::BufferObject( GL_ARRAY_BUFFER, ibp ) );

		const GLsizei stride = sizeof( GLfloat ) * 5;
		jagDraw::VertexAttribPtr iVerts( new jagDraw::VertexAttrib(
			"vertex", 3, GL_FLOAT, GL_FALSE, stride, 0 ) );
		jagDraw::VertexAttribPtr iTexCoord( new jagDraw::VertexAttrib(
			"texcoord", 2, GL_FLOAT, GL_FALSE, stride, sizeof( GLfloat ) * 3 ) );

		jagDraw::VertexArrayObjectPtr vaop( new jagDraw::VertexArrayObject );
		vaop->addVertexArrayCommand( ibop, jagDraw::VertexArrayObject::Vertex );
		vaop->addVertexArrayCommand( iVerts, jagDraw::VertexArrayObject::Vertex );
		vaop->addVertexArrayCommand( iTexCoord );

		jagDraw::DrawablePtr drawable( new jagDraw::Drawable() );
		jagDraw::DrawArraysPtr drawArrays( new jagDraw::DrawArrays( GL_TRIANGLE_STRIP, 0, 4 ) );
		drawable->addDrawCommand( drawArrays );

		// And a uniform for each sampler / texture unit.
		jagDraw::UniformPtr textureUniform( new jagDraw::Uniform( "texture", GL_SAMPLER_2D, (GLint)0 ) );
		jagDraw::UniformPtr textureUniform2( new jagDraw::Uniform( "texture2", GL_SAMPLER_2D, (GLint)1 ) );
		jagDraw::UniformPtr textureUniform3( new jagDraw::Uniform( "texture3", GL_SAMPLER_2D, (GLint)2 ) );


		jagDraw::UniformSetPtr uniformSet( jagDraw::UniformSetPtr( new jagDraw::UniformSet() ) );
		(*uniformSet)[ textureUniform->getNameHash() ] = textureUniform;
		(*uniformSet)[ textureUniform2->getNameHash() ] = textureUniform2;
		(*uniformSet)[ textureUniform3->getNameHash() ] = textureUniform3;

		jagDraw::TextureSetPtr textureSet( jagDraw::TextureSetPtr( new jagDraw::TextureSet() ) );

		//add both textures to the texture set
		(*textureSet)[ GL_TEXTURE0 ] = tex;
		(*textureSet)[ GL_TEXTURE1 ] = tex2;
		(*textureSet)[ GL_TEXTURE2 ] = tex3;
		std::cout << "_texHeight " << _texHeight << std::endl;
		std::cout << "_texWidth " << _texWidth << std::endl;

		jagDraw::CommandMapPtr quadCommands( jagDraw::CommandMapPtr( new jagDraw::CommandMap() ) );
		jagDraw::CommandMapPtr pass3Commands( jagDraw::CommandMapPtr( new jagDraw::CommandMap() ) );


		commands->insert(blurprog);
		commands->insert(vaop);
		commands->insert(_pass2FBO);
		commands->insert(textureSet);
		commands->insert(uniformSet);

		pass3Commands->insert(blurprogh);
		pass3Commands->insert(vaop);
		pass3Commands->insert(_pass3FBO);
		pass3Commands->insert(textureSet);
		pass3Commands->insert(uniformSet);

		jagDraw::DrawNodePtr pass2DrawNode = jagDraw::DrawNodePtr( new jagDraw::Node( commands ));
		pass2DrawNode->addDrawable(drawable);
		_pass2Nodes.push_back(pass2DrawNode);

		jagDraw::DrawNodePtr pass3DrawNode = jagDraw::DrawNodePtr( new jagDraw::Node( pass3Commands ));
		pass3DrawNode->addDrawable(drawable);
		_pass3Nodes.push_back(pass3DrawNode);


		quadCommands->insert( prog );
		quadCommands->insert( vaop );
		quadCommands->insert( _defaultFBO );
		std::cout << "COMMAND TYPE "<< textureSet->getCommandTypeString(textureSet->getCommandType()) << std::endl;
		quadCommands->insert( textureSet );
		quadCommands->insert( uniformSet );
		jagDraw::DrawNodePtr quadDrawNode = jagDraw::DrawNodePtr( new jagDraw::Node( quadCommands ));
		quadDrawNode->addDrawable( drawable );
		_quadNodes.push_back( quadDrawNode );
		_quadNodes.setMaxContexts(numContexts);
		_textureFBO->setMaxContexts(numContexts);
		_pass2Nodes.setMaxContexts(numContexts);
		_pass3Nodes.setMaxContexts(numContexts);

	}

	currentDrawGraph=false;

	// Tell all Jag3D objects how many contexts to expect.
	_root->setMaxContexts( numContexts );

	wt = new warrantytool::WarrantyToolGP(_root);

	wt->InitializeNode();
	//launchThread();
	return( true );
}

bool JagModel::init()
{
	glClearColor( 1.f, 1.f, 1.f, 0.f );

	glEnable( GL_DEPTH_TEST );

	// Auto-log the version string.
	jagBase::getVersionString();

	// Auto-log the OpenGL version string.
	jagDraw::getOpenGLVersionString();

	return( true );
}

// Don't bother until we have something worth sorting.
//#define ENABLE_SORT

bool JagModel::frame( const gmtl::Matrix44d& view, const gmtl::Matrix44d& proj)
{
	if( !getStartupCalled() )
		return( true );
	//if(!currentDrawGraph)
	//	return true;
	boost::mutex::scoped_lock scoped_lock(_cvSwapMutex);
	JAG3D_PROFILE( "frame" );

#ifdef ENABLE_SORT
	jagDraw::DrawablePrep::CommandTypeVec plist;
	plist.push_back( jagDraw::DrawablePrep::UniformSet_t );
#endif

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	const jagDraw::jagDrawContextID contextID( jagDraw::ContextSupport::instance()->getActiveContext() );
	jagDraw::DrawInfo& drawInfo( getDrawInfo( contextID ) );

	jagMx::MxCorePtr mxCore( _mxCore._data[ contextID ] );

	jagSG::CollectionVisitor& collect( getCollectionVisitor() );

	//_first = false;
	if(_first)
		collect.reset();

	gmtl::Matrix44d viewMatrix;
	{
		JAG3D_PROFILE( "Collection" );

		// Set view and projection to define the collection frustum.
		viewMatrix = mxCore->getInverseMatrix();
		collect.setViewProj( viewMatrix, mxCore->computeProjection( .1, 500. ) );

		{
			JAG3D_PROFILE( "Collection traverse" );
			// Collect a draw graph.
			if(_first) {
				_root->accept( collect );
				//		_first = false;
			}
		}
#ifdef ENABLE_SORT
		{
			JAG3D_PROFILE( "Collection sort" );
			jagDraw::DrawGraphPtr drawGraph( collect.getDrawGraph() );
			BOOST_FOREACH( jagDraw::NodeContainer& nc, *drawGraph )
			{
				std::sort( nc.begin(), nc.end(), jagDraw::DrawNodeCommandSorter( plist ) );
			}
		}
#endif
	}

	{
		JAG3D_PROFILE( "Render" );

		// Execute the draw graph.

		jagDraw::DrawGraphPtr drawGraph=currentDrawGraph;//( collect.getDrawGraph() );
		if(_first) {
			drawGraph = collect.getDrawGraph();
			_first = false;
			currentDrawGraph = collect.getDrawGraph();
		}
		// Set view and projection to use for drawing. Create projection using
		// the computed near and far planes.
		double minNear, maxFar;
		//collect.getNearFar( minNear, maxFar );
		minNear=.1;
		maxFar=1000;

		drawGraph->setViewProj( viewMatrix, mxCore->computeProjection( minNear, maxFar ) );
		//collect.setViewProj(viewMatrix, mxCore->computeProjection( minNear, maxFar ) );
		tform = drawGraph->getTransformCallback()->getTransform();
		//std::cout << "RIGHT BEFORE FRAME EXECUTe" << std::endl;
		drawGraph->execute( drawInfo );
		//std::cout << "RIGHT AFTER FRAME EXEECUTE" << std::endl;

		//std::cout << (void*)currentDrawGraph.get() << "drawgraphaddress" << std::endl;

		const jagDraw::jagDrawContextID contextID( jagDraw::ContextSupport::instance()->getActiveContext() );
		jagDraw::DrawInfo& drawInfo( getDrawInfo( contextID ) );

		// Render all Drawables.
		glEnable(GL_DEPTH_TEST);
		_nodes.execute( drawInfo );
		glFlush();
		//glDisable(GL_DEPTH_TEST);
		_pass2Nodes.execute(drawInfo);
		//_pass3Nodes.execute(drawInfo);
		_quadNodes.execute(drawInfo);
	}

	glFlush();

	JAG3D_ERROR_CHECK( "jagmodel display()" );
	//jagBase::ProfileManager::get 
	return( true );
}

//this needs major rework to work properly with RTT and MRT - currently just breaks everything.
void JagModel::reshape( const int w, const int h )
{
	if( !getStartupCalled() )
		return;

	const jagDraw::jagDrawContextID contextID( jagDraw::ContextSupport::instance()->getActiveContext() );
	std::cout << "RESHAPE CALLED" << std::endl;
	_mxCore._data[ contextID ]->setAspect( ( double ) w / ( double ) h );
}


//no longer used
/*gmtl::Matrix44d JagModel::computeProjection( double aspect )
{
const gmtl::Sphered s( _root->getBound()->asSphere() );

if( s.getRadius() <= 0.f )
return( gmtl::MAT_IDENTITY44D );

gmtl::Matrix44d proj;
const double zNear = 3.5 * s.getRadius();
const double zFar = 5.75 * s.getRadius();
gmtl::setPerspective< double >( proj, 30., aspect, zNear, zFar );

return( proj );
}*/

//these are not used any more
/*
gmtl::Matrix44d JagModel::computeView()
{


const gmtl::Sphered s( _root->getBound()->asSphere() );
const gmtl::Point3d center( s.getCenter() );
//std::cout << "center: " << s.getCenter()  << std::endl;
//std::cout << "radius: " << s.getRadius() << std::endl;
const double radius( (float)s.getRadius() );

const gmtl::Point3d eye( center + ( gmtl::Point3d( 0, -4., 0 ) * radius ) );
const gmtl::Vec3d up( 0.f, 0.f, 1.f );

gmtl::Matrix44d view;
gmtl::setLookAt( view, eye, center, up );
//return( view );
return computeView(currentAngle);
}

gmtl::Matrix44d JagModel::computeView( const double angleRad )
{
const gmtl::Sphered s( _root->getBound()->asSphere() );
const gmtl::Point3d center( s.getCenter() );
const double radius( (float)s.getRadius() );

gmtl::Matrix33d rot;
setRot( rot, gmtl::AxisAngle< double >( angleRad, 0., 1., 0. ) );
gmtl::Point3d eyeOffset( rot * gmtl::Point3d( 0., 0., -4. ) * radius );

const gmtl::Point3d eye( center + eyeOffset );
const gmtl::Vec3d up( 0.f, 1.f, 0.f );

gmtl::Matrix44d view;
gmtl::setLookAt( view, eye, center, up );
//std::cout << view << std::endl;
return( view );
}
*/


void JagModel::pickEvent(gmtl::Vec4d pos, int w, int h) {




	gmtl::Matrix44d fulltform = tform.getModelViewProjInv();
	gmtl::Vec4d a, b;
	pos[0] = 2*pos[0]/((double)w) -1.0;
	pos[1] = 2*pos[1]/((double)(h)) -1.0;
	pos[2] = -1.0;// - 2.0*z;
	//pos[2] = 1.0;
	//gmtl::xform(a, fulltform, pos4d);
	//a = pos4d * fulltform;
	a = fulltform * pos;
	pos[2] = 1.0;
	//gmtl::xform(b, fulltform, pos4d);
	b = fulltform * pos;
	a = a/a[3];
	b = b/b[3];


	gmtl::Vec3d aa, bb;
	aa[0] = a[0];
	aa[1] = a[1];
	aa[2] = a[2];
	bb[0] = b[0];
	bb[1] = b[1];
	bb[2] = b[2];

	const jagDraw::jagDrawContextID contextID( jagDraw::ContextSupport::instance()->getActiveContext() );
	jagDraw::DrawInfo& drawInfo( getDrawInfo( contextID ) );

	jagMx::MxCorePtr mxCore( _mxCore._data[ contextID ] );
	gmtl::Vec3d camPos = mxCore->getPosition();
	std::cout << camPos << " camPos " << std::endl;
	jagSG::IntersectVisitor iv(_root, gmtl::Rayd(camPos,bb-aa));
	gmtl::Vec3d transPos;
	if (iv.getHits().size() >0) {
		std::cout << iv.getHits().size() << std::endl;
		gmtl::Vec3f posf = iv.getHits()[0].hitPosition;
		transPos[0] = posf[0];
		transPos[1] = posf[1];
		transPos[2] = posf[2];
		//transPos[3] = 1.0;
		std::cout << transPos << " TRANSPOS" << std::endl;


	}

	//these are unrelated and are only used for debug information about the path of picking
	std::cout << iv.getNumTriangles() << " TRIANGLES " << std::endl;
	std::cout << iv.getNumNodes() << " NODES " << std::endl;
	std::cout << iv.getNumDrawables() << " DRAWCOMMANDS " << std::endl;



}

/* This represents the initial crude implementation of a multithreaded renderer using jag. 
 * Basically the collection visitor for  the root scene (including the correseponding draw graph) 
 * is double buffered. The collection thread continually does collection based on updated camera info
 * and the draw thread continually draws the most recently fully collected draw graph.
 * The only critical section is the buffer swap.
*/
void JagModel::doCollection() {
	
	jagSG::CollectionVisitor cva, cvb;;// = this->getBackCollectionVisitor();
		

	while(true) {
		jagDraw::DrawGraphPtr drawGraphTemplatea( new jagDraw::DrawGraph() );
		jagDraw::DrawGraphPtr drawGraphTemplateb( new jagDraw::DrawGraph() );
		drawGraphTemplatea->resize( 1 );
		drawGraphTemplateb->resize( 1 );
		//getCollectionVisitor().setDrawGraphTemplate( drawGraphTemplate );
		cva.setDrawGraphTemplate(drawGraphTemplatea);
		cvb.setDrawGraphTemplate(drawGraphTemplateb);
		cva.setViewport( 0, 0, 1000, 1000 );
		cvb.setViewport(0,0,1000,1000);
		
		gmtl::Matrix44d viewMatrix;
		viewMatrix = tform.getView();

		
		if(this->_useFirst) {
			boost::mutex::scoped_lock(updateMutex);
			cva.reset();
			cva.setViewProj( viewMatrix, tform.getProj() );
			_root->accept(cva);
			boost::mutex::scoped_lock(_cvSwapMutex);
			currentDrawGraph = cva.getDrawGraph();
			_useFirst = false;
		}
		else{
			boost::mutex::scoped_lock(updateMutex);
			cvb.reset();
			cvb.setViewProj( viewMatrix, tform.getProj() );
			_root->accept(cvb);
			boost::mutex::scoped_lock(_cvSwapMutex);
			
			currentDrawGraph = cvb.getDrawGraph();
			
			_useFirst = true;
		}
		//return;
		/*cv.reset();
		gmtl::Matrix44d viewMatrix;


		{
		//JAG3D_PROFILE( "Collection traverse" );
		// Collect a draw graph.

		//_root->accept( cv );
		}

		//std::cout << "DOING COLLECTION" << std::endl;
		*/

	}
}

gmtl::Vec3d JagModel::getRealTrans(gmtl::Vec2d delta) {
	gmtl::Matrix44d fulltform = tform.getModelViewProjInv();
	gmtl::Vec4d a(0,0,0,1), b(delta[0],delta[1],0,1);
	a = fulltform*a;
	b = fulltform*b;
	a = a/a[3];
	b = b/b[3];
	gmtl::Vec4d ret = a - b;
	gmtl::Vec3d ret3;
	ret3[0] = ret[0];
	ret3[1] = ret[1];
	ret3[2] =  ret[2];

	return ret3;


	
}