#include "DemoInterface.h"

class JagModel : public DemoInterface
{
public:
    JagModel()
      : DemoInterface( "jag.ex.jagmodel" ),
        _fileName( "dumptruck.osg" )
		
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

protected:
    gmtl::Matrix44d computeProjection( double aspect );
    gmtl::Matrix44d computeView();
	gmtl::Matrix44d computeView( const double angleRad );
	double currentAngle;
    std::string _fileName;

    jagSG::NodePtr _root;

	typedef jagDraw::PerContextData< double > PerContextAspect;
    PerContextAspect _aspect;

    typedef jagDraw::PerContextData< gmtl::Matrix44d > PerContextMatrix44d;
    PerContextMatrix44d _proj;
};