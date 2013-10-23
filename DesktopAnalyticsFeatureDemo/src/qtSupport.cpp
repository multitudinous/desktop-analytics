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

#define JAG3D_USE_GL3W
#include "jagmodel.h"
//#include "DemoInterface.h"
#include <jagDraw/ContextSupport.h>
#include <jagBase/Profile.h>

#include "qtGlWidget.h"
#include "WarrantyToolPlugin_UIDialog.h"
#include <QApplication>
#include <QGLFormat>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QTimer>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/foreach.hpp>

#include <math.h>
#include "ui_datest.h"
#include <iostream>

#include <jagMx/MxCore.h>
#include <jagMx/MxUtils.h>
#include <jagMx/MxGamePad.h>


#include <boost/thread/thread.hpp>

using namespace std;
namespace bpo = boost::program_options;
Ui_MainWindow ui;

JagModel* di( NULL );

void doCollection() {
//	di->doCollection();
	
}

GLWidget::GLWidget( const QGLFormat& format, QWidget* parent )
    : QGLWidget( format, parent )
{
	_rightDrag = false;
	_leftDrag = false;
	_lastNX = 0;
	_lastNY = 0;
}

QGLFormat getFormat() {
	QGLFormat glFormat( QGL::DoubleBuffer | QGL::Rgba | QGL::DepthBuffer | QGL::NoSampleBuffers );
	double version = 4.0;
	double versionMajor;
    modf( version, &versionMajor );
    float versionMinor = (float)( version * 10. - versionMajor * 10. );
    if( version >= 3.0 )
    {
        glFormat.setVersion( int( versionMajor ), int( versionMinor ) );
        //if( version >= 3.1 )
            // Qt doesn't appear to allow setting the forward compatible flag.
            // Do nothing in this case.
        if( version >= 3.2 )
            glFormat.setProfile( QGLFormat::CoreProfile );
    }
	return glFormat;
}

GLWidget::GLWidget(QWidget* parent)
	: QGLWidget( getFormat(), parent)
	{
	
}

void GLWidget::initializeGL()
{
    jagDraw::ContextSupport* cs( jagDraw::ContextSupport::instance() );
    const jagDraw::platformContextID pCtxId = reinterpret_cast< const jagDraw::platformContextID >( context() );
    jagDraw::jagDrawContextID contextID = cs->registerContext( pCtxId );

    cs->setActiveContext( contextID );
    cs->initContext();

    di->init();
	
}

void GLWidget::paintGL()
{
    jagDraw::ContextSupport* cs( jagDraw::ContextSupport::instance() ); 
    const jagDraw::platformContextID pCtxId = reinterpret_cast< const jagDraw::platformContextID >( context() );
    jagDraw::jagDrawContextID contextID = cs->getJagContextID( pCtxId );

    cs->setActiveContext( contextID );
    di->frame(gmtl::MAT_IDENTITY44D,gmtl::MAT_IDENTITY44D);

#ifdef JAG3D_ENABLE_PROFILING
    jagBase::ProfileManager::instance()->dumpAll();
#endif

	
}

void GLWidget::resizeGL( int w, int h )
{
    glViewport( 0, 0, w, h );
    di->getCollectionVisitor().setViewport( 0, 0, w, h );
    di->reshape( w, h );
}


void GLWidget::toggleNode() {
	//di->toggleSecondNodeMask();
//	di->toggleNodeByName(ui.lineEdit->text().toLatin1().constData());
	updateGL();
}

void GLWidget::keyPressEvent( QKeyEvent* e )
{
    switch ( e->key() )
    {
    case Qt::Key_Escape:
    case 'q':
        di->shutdown();
        delete di;
        QCoreApplication::instance()->quit();
        break;

    default:
        QGLWidget::keyPressEvent( e );
    }
}

void normXY( float& normX, float& normY, const int x, const int y, const int width, const int height )
{
    // Given a width x height window, convert pixel coords x and y
    // to normalized coords in the range -1 to 1. Invert Y so that
    // -1 is at the window bottom.
    const float halfW( (float)width * .5f );
    const float halfH( (float)height * .5f );

    normX = ( (float)x - halfW ) / halfW;
    normY = -( (float)y - halfH ) / halfH;
}

void GLWidget::mousePressEvent(QMouseEvent* e) {

	const int width( this->width() );
	const int height( this->height() );
	int x, y;
	x = e->x();
	y = e->y();

	normXY( _lastNX, _lastNY, x, y, width, height );
    _lastX = x;
    _lastY = y;

	
	_leftDrag = e->buttons()&Qt::LeftButton;

	
	_rightDrag = e->buttons()&Qt::RightButton;

	_midDrag = e->buttons()&Qt::MidButton;

	rayEvent(x,y);
	updateGL();
}



void GLWidget::mouseMoveEvent(QMouseEvent* e) {
	if( !_leftDrag && !_rightDrag &&  !_midDrag)
        return;

	int context = jagDraw::ContextSupport::instance()->getActiveContext();
    //const int window( glutGetWindow() - 1 );
    jagMx::MxCorePtr mxCore( di->getMxCore( context ) );
    if( mxCore == NULL )
        return;

	const int width( this->width() );
	const int height( this->height() );

    float nx, ny;
	int x, y;
	x = e->x();
	y = e->y();
    normXY( nx, ny, x, y, width, height );
    const float deltaX( nx - _lastNX );
    const float deltaY( ny - _lastNY );

    if( _rightDrag )
    {
        mxCore->moveOrbit( deltaY );
    }
    else if( _leftDrag )
    {
        double angle;
        gmtl::Vec3d axis;
        jagMx::computeTrackball( angle, axis,
            gmtl::Vec2d( _lastNX, _lastNY ), gmtl::Vec2d( deltaX, deltaY ),
            mxCore->getOrientationMatrix() );

        mxCore->rotateOrbit( angle, axis );
    }
	else if(_midDrag) 
	{
		/*//mxCore->
		//std::cout << "should move literal" << nx << " " << ny << std::endl;
		gmtl::Vec3d pos = mxCore->getPosition();
		gmtl::Vec3d center = mxCore->getOrbitCenterPoint();
		gmtl::Vec3d delta = pos - center;
		double distance = sqrt(pow(delta[0],2)+pow(delta[1],2)+pow(delta[2],2));
		//distance = distance/3.0;
		
		////std::cout << "TRANS:" << di->getRealTrans(gmtl::Vec2d(deltaX, deltaY)) << std::endl;
		//std::cout << "DISTANCE:" << distance;
		double ddX, ddY;
		if(this->width() > this->height()) {
			ddX = deltaX*((double)this->width())/((double)this->height());
			ddY= deltaY;
		}
		else {
			ddY = deltaY*((double)this->height())/((double)this->width());
			ddX = deltaX;
		}

		double scrdelta = sqrt(ddX*ddX + ddY*ddY);
		gmtl::Vec3d rtrans = di->getRealTrans(gmtl::Vec2d(deltaX, deltaY));
		gmtl::normalize(rtrans);

		//std::cout << rtrans << " RTRaNS " << std::endl;
		//mxCore->moveLiteral(di->getRealTrans(gmtl::Vec2d(deltaX, deltaY)));
		//mxCore->moveLiteral(gmtl::Vec3d(-deltaX*distance*0.26794919243, 0, -deltaY*distance*0.26794919243));
		mxCore->moveLiteral(rtrans*scrdelta*distance*0.26794919243);*/

		gmtl::Planed panPlane( -( mxCore->getDir() ), 0. );
        gmtl::Vec3d panDelta( jagMx::pan( mxCore.get(), panPlane, deltaX, deltaY ) );
        mxCore->moveLiteral( -panDelta );

	}

    _lastNX = nx;
    _lastNY = ny;
	
	updateGL();
    
	
}

void GLWidget::rayEvent(float x, float y) {
	gmtl::Vec4d ndcPos;
	ndcPos[0] = x;//2*x/((double)(width())) -1.0;
	ndcPos[1] = height()-y;///-2*y/((double)(height())) +1.0;
	ndcPos[2] = 0.0;//-1.0;
	ndcPos[3] = 1.0;
	std::cout << ndcPos << " ndcPos" << std::endl;


//	di->pickEvent(ndcPos, width(), height());

}


int main( int argc, char** argv )
{
    std::vector< int > winsize;
    bpo::options_description desc( "Options" );
    // Add test/demo options
    desc.add_options()
        ( "help,h", "Help text" )
        ( "version,v", bpo::value< double  >(), "OpenGL context version. Default: 4.0." )
        ( "nwin", bpo::value< int >(), "Number of windows. Default: 1." )
        ( "winsize,w", bpo::value< std::vector< int > >( &winsize )->multitoken(), "Window width and height. Default: 300 300." )
        ;

    // Create test/demo-specific DemoInterface, and allow it to
    // add test/demo-specific options.
    di = new JagModel();

    bpo::variables_map vm;
    bpo::store( bpo::parse_command_line( argc, argv, desc ), vm );
    bpo::notify( vm );

    if( !( di->parseOptions( vm ) ) ||
        ( vm.count( "help" ) > 0 ) )
    {
        std::cout << desc << std::endl;
        return( 1 );
    }

#if( POCO_OS == POCO_OS_MAC_OS_X )
    // In OSX 10.7/10.8, use GL 3.2 and GLSL 1.50
    double version( 3.2 );
#else
    double version( 4.0 );
#endif
    if( vm.count( "version" ) > 0 )
        version = vm[ "version" ].as< double >();
    double versionMajor;
    modf( version, &versionMajor );
    float versionMinor = (float)( version * 10. - versionMajor * 10. );

    int nwin( 1 );
    if( vm.count( "nwin" ) > 0 )
        nwin = vm[ "nwin" ].as< int >();

    if( winsize.size() != 2 )
    {
        winsize.clear();
        winsize.push_back( 300 ); winsize.push_back( 300 );
    }


    QApplication app( argc, argv );

    QGLFormat glFormat( QGL::DoubleBuffer | QGL::Rgba | QGL::DepthBuffer | QGL::SampleBuffers );
    if( version >= 3.0 )
    {
        glFormat.setVersion( int( versionMajor ), int( versionMinor ) );
        //if( version >= 3.1 )
            // Qt doesn't appear to allow setting the forward compatible flag.
            // Do nothing in this case.
        if( version >= 3.2 )
            glFormat.setProfile( QGLFormat::CoreProfile );
    }

    

	
	

   

	QMainWindow* window = new QMainWindow();
	ui.setupUi(window);
	ui.renderwidget->resize(1000,1000);
	 if( !( di->startup( 1 ) ) )
        return( 1 );
	ui.renderwidget->show();
	ui.pushButton->show();
	//QObject::connect(ui.pushButton, SIGNAL(clicked()), ui.renderwidget, SLOT(dosomething()));
	QObject::connect(ui.toggleNodeButton, SIGNAL(clicked()), ui.renderwidget, SLOT(toggleNode()));
	
	window->show();

	WarrantyToolPlugin_UIDialog* wtpuid = new WarrantyToolPlugin_UIDialog();
	wtpuid->show();


	boost::thread t(doCollection);
		
    return( app.exec() );

    
}
