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
	DemoInterface::startup( numContexts );

    JAG3D_INFO_STATIC( _logName, _fileName );

    if( _fileName.empty() )
    {
        JAG3D_FATAL_STATIC( _logName, "Specify '--file <fileName>' on command line." );
        return( false );
    }


    // Create the texture used by _opaqueFBO to store the opaque
    // color buffer. After our opaque NodeContainer #0 renders into it, 
    // the ABuffer class's third NodeContainer uses it during the
    // abuffer resolve.
    jagDraw::ImagePtr image( new jagDraw::Image() );
    image->set( 0, GL_RGBA, _width, _height, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    _opaqueBuffer.reset( new jagDraw::Texture( GL_TEXTURE_2D, image,
        jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
    _opaqueBuffer->getSampler()->getSamplerState()->_minFilter = GL_NEAREST;
    _opaqueBuffer->getSampler()->getSamplerState()->_magFilter = GL_NEAREST;
    _opaqueBuffer->setMaxContexts( numContexts );

    // Create second color buffer for glow effect.
    _secondaryBuffer.reset( new jagDraw::Texture( GL_TEXTURE_2D, image,
        jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
    _secondaryBuffer->getSampler()->getSamplerState()->_minFilter = GL_NEAREST;
    _secondaryBuffer->getSampler()->getSamplerState()->_magFilter = GL_NEAREST;
    _secondaryBuffer->setMaxContexts( numContexts );

    // Create glow effect output buffer
    _glowBuffer.reset( new jagDraw::Texture( GL_TEXTURE_2D, image,
        jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
    _glowBuffer->getSampler()->getSamplerState()->_minFilter = GL_NEAREST;
    _glowBuffer->getSampler()->getSamplerState()->_magFilter = GL_NEAREST;
    _glowBuffer->setMaxContexts( numContexts );

    // Depth texture used as depth buffer for opaque pass.
    // Also used during abuffer render to discard occluded fragments.
    image->set( 0, GL_DEPTH_COMPONENT, _width, _height, 1, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL );
    _depthBuffer.reset( new jagDraw::Texture( GL_TEXTURE_2D, image,
        jagDraw::SamplerPtr( new jagDraw::Sampler() ) ) );
    _depthBuffer->getSampler()->getSamplerState()->_minFilter = GL_NEAREST;
    _depthBuffer->getSampler()->getSamplerState()->_magFilter = GL_NEAREST;
    _depthBuffer->setMaxContexts( numContexts );

    // Create the ABuffer management object.
    _aBuffer.reset( new jagUtil::ABuffer( _depthBuffer, _opaqueBuffer, _glowBuffer ) );
    _aBuffer->setMaxContexts( numContexts );

    // Obtain the draw graph from the ABuffer object.
    // Default behavior is that the ABuffer owns NodeContainers 1-3, and we put
    // all opaque geometry in NodeContainer 0.
    jagDraw::DrawGraphPtr drawGraph( _aBuffer->createDrawGraphTemplate( 2 ) );

    // Create blur effect NodeContainer
    _blur.reset( new jagUtil::Blur( _secondaryBuffer, _glowBuffer ) );
    _blur->setMaxContexts( numContexts );
    (*drawGraph)[ 1 ] = _blur->getNodeContainer();

    getCollectionVisitor().setDrawGraphTemplate( drawGraph );

    // Allow ABuffer object to specify which matrices it needs.
    jagDraw::TransformCallback* xformCB( getCollectionVisitor().getTransformCallback() );
    xformCB->setRequiredMatrixUniforms(
        xformCB->getRequiredMatrixUniforms() |
        _aBuffer->getRequiredMatrixUniforms() );


    // Prepare the scene graph.
    /*jagSG::NodePtr model( DemoInterface::readSceneGraphNodeUtil( _fileName ) );
    if( model == NULL )
        return( false );

    // Add model instance as opaque
    _root.reset( new jagSG::Node );
    _root->addChild( model );

    // Add translated model instance for ABuffer transparency
    jagSG::NodePtr xformNode( jagSG::NodePtr( new jagSG::Node() ) );
    gmtl::setTrans( xformNode->getTransform(), gmtl::Vec3d( 0., model->getBound()->getRadius() * -1.5, 0. ) );
    _root->addChild( xformNode );
    xformNode->addChild( model );*/
    _aBuffer->setTransparencyEnable( *_root );


    // For gamepad speed control
    //_moveRate = _root->getBound()->getRadius();


    // Prepare for culling.
    jagSG::FrustumCullDistributionVisitor fcdv;
    _root->accept( fcdv );
    jagSG::SmallFeatureDistributionVisitor sfdv;
    _root->accept( sfdv );

    // Optimize VAO and element buffers.
    jagUtil::BufferAggregationVisitor bav( _root );


    // The scene graph will have two CommandMaps, one for opaque
    // rendering and the other for ABuffer transparency. The opaque
    // CommandMap will sit at the root of the scene graph, and the
    // application will add or remove the ABuffer CommandMap as
    // needed to toggle transparency on and off.

    // Create opaque CommandMap.
    jagDraw::ShaderPtr vs( DemoInterface::readShaderUtil( "jagmodel.vert" ) );
    jagDraw::ShaderPtr fs( DemoInterface::readShaderUtil( "abufferJagGlow.frag" ) );

    jagDraw::ProgramPtr prog;
    prog = jagDraw::ProgramPtr( new jagDraw::Program );
    prog->attachShader( vs );
    prog->attachShader( fs );

    jagDraw::CommandMapPtr commands( _root->getOrCreateCommandMap() );
    commands->insert( prog );

    // Create a framebuffer object. The color texture will store opaque
    // rendering. The depth texture is shared with ABuffer to avoid
    // rendering behind opaque geometry.
    _opaqueFBO.reset( new jagDraw::Framebuffer( GL_DRAW_FRAMEBUFFER ) );
    _opaqueFBO->setViewport( 0, 0, _width, _height );
    _opaqueFBO->setClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    _opaqueFBO->setClearColor( .15f, .1f, .5f, 0.f ); // Must clear alpha to 0 for glow.
    _opaqueFBO->addAttachment( GL_DEPTH_ATTACHMENT, _depthBuffer );
    _opaqueFBO->addAttachment( GL_COLOR_ATTACHMENT0, _opaqueBuffer );
    _opaqueFBO->addAttachment( GL_COLOR_ATTACHMENT1, _secondaryBuffer );
    commands->insert( _opaqueFBO );

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


    // Tell scene graph how many contexts to expect.
    _root->setMaxContexts( numContexts );


    

	wt = new warrantytool::WarrantyToolGP(_root);
	currentDrawGraph = false;
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
			_first = true;
			currentDrawGraph = collect.getDrawGraph();
			//currentCv = jagSG::CollectionVisitorPtr(&collect);
		}
		// Set view and projection to use for drawing. Create projection using
		// the computed near and far planes.
		/*double minNear, maxFar;
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
		_quadNodes.execute(drawInfo);*/


		 JAG3D_PROFILE( "Render" );

        // Execute the draw graph.
        jagDraw::DrawGraphPtr drawGraph2( collect.getDrawGraph() );

        // Set view and projection to use for drawing. Create projection using
        // the computed near and far planes.
        double minNear, maxFar;
        collect.getNearFar( minNear, maxFar );
        drawGraph2->setViewProj( viewMatrix, mxCore->computeProjection( minNear, maxFar ) );

        // The ABuffer object handles rendering.
        // This line replaces drawGraph->execute( drawInfo );
        _aBuffer->renderFrame( collect, drawInfo );
    }
#ifdef JAG3D_ENABLE_PROFILING
    {
        // If profiling, dump out draw graph info.
        jagUtil::DrawGraphCountVisitor dgcv;
        dgcv.traverse( *( collect.getDrawGraph() ) );
        dgcv.dump( std::cout );
    }
#endif

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
	
	jagSG::CollectionVisitorPtr cva, cvb;;// = this->getBackCollectionVisitor();
	cva = jagSG::CollectionVisitorPtr( new jagSG::CollectionVisitor());
	cvb = jagSG::CollectionVisitorPtr( new jagSG::CollectionVisitor());
	return;
	while(true) {
		
		jagDraw::DrawGraphPtr drawGraphTemplatea( new jagDraw::DrawGraph() );
		jagDraw::DrawGraphPtr drawGraphTemplateb( new jagDraw::DrawGraph() );
		drawGraphTemplatea->resize( 1 );
		drawGraphTemplateb->resize( 1 );
		//getCollectionVisitor().setDrawGraphTemplate( drawGraphTemplate );

		cva->setDrawGraphTemplate(drawGraphTemplatea);
		cvb->setDrawGraphTemplate(drawGraphTemplateb);
		cva->setViewport( 0, 0, 1000, 1000 );
		cvb->setViewport(0,0,1000,1000);
		std::cout << "got here" << std::endl;
		gmtl::Matrix44d viewMatrix;
		viewMatrix = tform.getView();

		
		if(this->_useFirst) {
			boost::mutex::scoped_lock(updateMutex);
			cva.reset();
			cva->setViewProj( viewMatrix, tform.getProj() );
			_root->accept(*cva);
			boost::mutex::scoped_lock(_cvSwapMutex);
			currentDrawGraph = cva->getDrawGraph();
			currentCv = cva;
			_useFirst = false;
		}
		else{
			boost::mutex::scoped_lock(updateMutex);
			cvb.reset();
			cvb->setViewProj( viewMatrix, tform.getProj() );
			_root->accept(*cvb);
			boost::mutex::scoped_lock(_cvSwapMutex);
			
			currentDrawGraph = cvb->getDrawGraph();
			currentCv = cvb;
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