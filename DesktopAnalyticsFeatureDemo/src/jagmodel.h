#include "DemoInterface.h"
#include <gmtl/Ray.h>
#include "ToggleByNameVisitor.h"
#include <jagDraw/Common.h>
#include "WarrantyToolGP.h"
#include <jagBase\Transform.h>

#include <jagUtil/ABuffer.h>
#include <jagUtil/Blur.h>
#include <jagDisk/ReadWrite.h>
#include <jagBase/Profile.h>
#include <jagUtil/DrawGraphCountVisitor.h>
#include <jagSG/CollectionVisitor.h>

#pragma once

class JagModel : public DemoInterface
{
public:
    JagModel()
      : DemoInterface( "jag.ex.abuf" ),
        _fileName( "cow.osg" ),
        _moveRate( 1. ),
        _width( 1600 ),
        _height( 1200 ),
		_firstFrame(true)
    {}
    virtual ~JagModel() {}

    virtual bool parseOptions( boost::program_options::variables_map& vm );

    virtual bool startup( const unsigned int numContexts );
    virtual bool init();
    virtual bool frame( const gmtl::Matrix44d& view, const gmtl::Matrix44d& proj );
    virtual void reshape( const int w, const int h );
    virtual bool keyCommand( const int command );
	virtual void setABufferParams() {
		_aBuffer->setMaxFragments(150);
		_aBuffer->setFragmentAlpha(.05);
	
	}
	boost::mutex& getUpdateMutex()
	{
		return _updateMutex;
	}

	void toggleNodeByName(std::string name) {
		jagSG::ToggleByNameVisitor tbnm(this->_root, name);
	}

	void JagModel::pickEvent(gmtl::Vec4d pos, int w, int h);

    // Return a value to bontrol base gamepad move rate in the scene.
    virtual double getMoveRate() const
    {
        return( _moveRate );
    }

	/* Launch double buffered threaded collection
	*/
	void doThreadedCollection();

	jagUtil::ABufferPtr getABuffer() {
		return _aBuffer;
	}

protected:
    std::string _fileName;
	bool _useFirst;
	jagBase::TransformD tform;
    jagSG::NodePtr _root;

    double _moveRate;

    jagUtil::ABufferPtr _aBuffer;

    jagDraw::FramebufferPtr _opaqueFBO;
    jagDraw::TexturePtr _opaqueBuffer, _secondaryBuffer, _glowBuffer, _depthBuffer;
    jagUtil::BlurPtr _blur;

	//double buffered collection visitors
	jagSG::CollectionVisitor cva, cvb, currentCV;
	boost::mutex _updateMutex, _fm, _sm;
	bool _firstFrame;
	
    int _width, _height;

	//warranty tool graphical side object;
	warrantytool::WarrantyToolGP * wt;
};