#include "DemoInterface.h"
#include <gmtl/Ray.h>
class JagModel : public DemoInterface
{
public:
    JagModel()
      : DemoInterface( "jag.ex.jagmodel" ),
        _fileName( "memphis_accum.ive" ),
		_imageName( "tex.png" )
		
    {
		currentAngle=0.0;
		setContinuousRedraw();
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

	virtual void toggleSecondNodeMask() {
		secondNode->setNodeMask(!(secondNode->getNodeMask()));
	}
	
	//virtual void performRayIntersection(gmtl::Ray<double> currentRay);
	void JagModel::pickEvent(gmtl::Vec4d pos, int w, int h);
	void doSomething() {
		
	}

protected:
    gmtl::Matrix44d computeProjection( double aspect );
    gmtl::Matrix44d computeView();
	gmtl::Matrix44d computeView( const double angleRad );
	gmtl::Matrix44d lastProj;
	double currentAngle;
    std::string _fileName, _imageName;

    jagSG::NodePtr _root;
	jagSG::NodePtr secondNode;
	jagSG::NodePtr thirdNode;
	typedef jagDraw::PerContextData< double > PerContextAspect;
    PerContextAspect _aspect;
	jagBase::TransformD tform;
    typedef jagDraw::PerContextData< gmtl::Matrix44d > PerContextMatrix44d;
    PerContextMatrix44d _proj;
	jagDraw::NodeContainer _nodes;
};