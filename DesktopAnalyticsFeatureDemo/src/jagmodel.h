#include "DemoInterface.h"
#include <gmtl/Ray.h>
#include "ToggleByNameVisitor.h"
#include <jagDraw/Common.h>
#include "WarrantyToolGP.h"


#include <jagUtil/ABuffer.h>
#include <jagDisk/ReadWrite.h>
#include <jagBase/Profile.h>
#include <jagUtil/DrawGraphCountVisitor.h>

class JagModel : public DemoInterface
{
public:
    JagModel()
      : DemoInterface( "jag.ex.jagmodel" ),
        _fileName( "memphis_accum.ive" ),
		_imageName( "tex.png" )
		
    {
		_texWidth = 1000;
		_texHeight = 1000;
		currentAngle=0.0;
		setContinuousRedraw();
		_first = true;
	}
    virtual ~JagModel() {}

    virtual bool parseOptions( boost::program_options::variables_map& vm );

    virtual bool startup( const unsigned int numContexts );
    virtual bool init();
    virtual bool frame( const gmtl::Matrix44d& view=gmtl::MAT_IDENTITY44D, const gmtl::Matrix44d& proj=gmtl::MAT_IDENTITY44D );
    virtual void reshape( const int w, const int h );
    virtual bool shutdown()
    {
        return( true );
    }
	virtual void incrementAngle(double inc) {
		currentAngle+=inc;
	}

	void doCollection(); 

	gmtl::Vec3d getRealTrans(gmtl::Vec2d delta);

	void launchThread() {
		_thread = boost::thread(boost::bind(&JagModel::doCollection, this));
	}

	virtual void toggleSecondNodeMask() {
		//secondNode->setNodeMask(!(secondNode->getNodeMask()));
	}
	virtual void toggleNodeByName(std::string name) {
		_first = true;
		jagSG::ToggleByNameVisitor tbnv(_root, name);
		std::cout << " GOT THIS FAR " << std::endl;
		//_root->setMaxContexts(1);
		//std::cout << "SET MAX CONTEXTS" << std::endl;
	}
	
	//virtual void performRayIntersection(gmtl::Ray<double> currentRay);
	void JagModel::pickEvent(gmtl::Vec4d pos, int w, int h);
	void doSomething() {
		
	}

	boost::mutex& getUpdateMutex() {
		return updateMutex;
	}
protected:
    //no longer used
	//gmtl::Matrix44d computeProjection( double aspect );
   
	//these are no longer used
	//gmtl::Matrix44d computeView();
	//gmtl::Matrix44d computeView( const double angleRad );
	gmtl::Matrix44d lastProj;
	double currentAngle;
    std::string _fileName, _imageName;

    jagSG::NodePtr _root;
	jagSG::NodePtr _pass1, _pass2, _pass3;
	
	typedef jagDraw::PerContextData< double > PerContextAspect;
    PerContextAspect _aspect;
	jagBase::TransformD tform;
    typedef jagDraw::PerContextData< gmtl::Matrix44d > PerContextMatrix44d;
    PerContextMatrix44d _proj;
	jagDraw::NodeContainer _nodes;
	boost::thread _thread;
	boost::mutex updateMutex;

	jagDraw::NodeContainer _windowNodes, _rttNodes, _quadNodes, _pass2Nodes, _pass3Nodes;
    jagDraw::FramebufferPtr _textureFBO, _defaultFBO, _pass2FBO, _pass3FBO;
	jagDraw::DrawGraphPtr currentDrawGraph;
	bool _first;
	warrantytool::WarrantyToolGP * wt;
     GLsizei _texWidth, _texHeight;


	 jagUtil::ABufferPtr _aBuffer;

    jagDraw::FramebufferPtr _opaqueFBO;
    jagDraw::TexturePtr _opaqueBuffer, _secondaryBuffer, _depthBuffer;

};