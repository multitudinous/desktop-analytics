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

#include "ABufferCollectionVisitor.h"
#include <jagSG/Node.h>
#include <jagDraw/Node.h>
#include <jagBase/gmtlSupport.h>
#include <jagDraw/Error.h>
#include <jagBase/Profile.h>
#include <jagBase/LogMacros.h>

#include <gmtl/gmtl.h>

#include <cfloat>


namespace jagSG {


ABufferCollectionVisitor::ABufferCollectionVisitor()
  : Visitor( "collect" ),
    _currentNodes( NULL ),
    _nearFarOps( Default ),
    _frustumPlanes( LeftRightBottomTopPlanes )
{
    reset();
}
ABufferCollectionVisitor::ABufferCollectionVisitor( jagSG::Node& node )
  : Visitor( "collect" ),
    _currentNodes( NULL ),
    _nearFarOps( Default ),
    _frustumPlanes( LeftRightBottomTopPlanes )
{
    reset();
    node.accept( *this );
}
ABufferCollectionVisitor::ABufferCollectionVisitor( const ABufferCollectionVisitor& rhs )
  : Visitor( rhs ),
    _currentID( rhs._currentID ),
    _currentNodes( NULL ),
    _transform( rhs._transform ),
    _nearFarOps( rhs._nearFarOps ),
    _frustumPlanes( rhs._frustumPlanes )
{
    setDrawGraphTemplate( rhs._drawGraphTemplate );

    reset();
}
ABufferCollectionVisitor::~ABufferCollectionVisitor()
{
}


void ABufferCollectionVisitor::setViewProj( const gmtl::Matrix44d& view, const gmtl::Matrix44d& proj )
{
    _transform.setView( view );
    _transform.setProj( proj );
}
void ABufferCollectionVisitor::setViewport( const GLint x, const GLint y, const GLsizei width, const GLsizei height )
{
    _transform.setViewport( x, y, width, height );
}


void ABufferCollectionVisitor::reset()
{
    JAG3D_TRACE( "reset()" );

    resetCommandMap();
    resetMatrix();

    resetPlanes();

    // TBD This seems very bad that we're allocating a new one of
    // these in reset(), which means once per frame.
    if( _drawTransformCallback == NULL )
    {
        _drawTransformCallback = jagDraw::TransformCallbackPtr(
            new jagDraw::TransformCallback() );
    }
    else
    {
        // Use copy constructor to preserve any pre-existing settings
        // (like custom matrix uniform names).
        _drawTransformCallback.reset(
            new jagDraw::TransformCallback( *_drawTransformCallback ) );
    }

    _currentID = 0;
    if( _drawGraph != NULL )
    {
        BOOST_FOREACH( jagDraw::DrawGraph::value_type& data, *_drawGraph )
        {
            data.reset();
        }
        if( _currentID < _drawGraph->size() )
            _currentNodes = &( (*_drawGraph)[ _currentID ] );

        _drawGraph->setTransformCallback( _drawTransformCallback );
    }

    _infoPtr = CollectionInfoPtr( new CollectionInfo( _transform ) );

    _minECZ = DBL_MAX;
    _maxECZ = -DBL_MAX;
}


void ABufferCollectionVisitor::visit( jagSG::Node& node )
{
    CommandMapStackHelper cmsh( *this, node.getCommandMap() );

    {
    JAG3D_TRACE( "visit()" );
    JAG3D_PROFILE( "setup" );

    _infoPtr->setNode( &node );
    jagDraw::BoundOwner* boundOwner( &node );
    _infoPtr->setBound( boundOwner->getBound( _commandStack.back() ).get() );
    _infoPtr->setContainmentPlanes( getCurrentPlanes() );
    }
    Node::CallbackInfo* info( _infoPtr.get() );

    bool collect( true );
    {
    JAG3D_PROFILE( "callbacks" );
    jagSG::Node::CollectionCallbacks& callbacks( node.getCollectionCallbacks() );
    BOOST_FOREACH( jagSG::Node::CallbackPtr cb, callbacks )
    {
        if( !( (*cb)( this, info ) ) )
        {
            collect = false;
            break;
        }
    }
    }
    if( collect )
    {
        collectAndTraverse( node );
    }
}

void ABufferCollectionVisitor::collectAndTraverse( jagSG::Node& node )
{
    MatrixStackHelper msh( *this, node.getTransform() );
    pushPlanes();

    {
    JAG3D_PROFILE( "collect operations" );
    if( msh.getDirty() )
    {
        _transform.setModel( _matrixStack.back() );
    }

    // Calculate maxECZ, used to determine 1) if all Drawables
    // are behind the viewer, and 2) possible modification to the
    // computed _maxECZ value.
    const double maxDrawableECZ( _infoPtr->getECBoundDistance() + _infoPtr->getECBoundRadius() );
    // We will collectDrawables if either 1) we don't care about them
    // being behind the viewer, or 2) at least some of the Drawables are
    // in front of the viewer.
    const bool collectDrawables( ( ( _nearFarOps & DiscardNegativeECZDrawables ) == 0 ) ||
        ( maxDrawableECZ > 0.0 ) );

    const unsigned int numDrawables( node.getNumDrawables() );
    if( collectDrawables && ( numDrawables > 0 ) )
    {
        if( ( _nearFarOps & AutoCompute ) != 0 )
        {
            // Record changes to min near / max far.
            _minECZ = std::min< double >( _minECZ, _infoPtr->getECBoundDistance() - _infoPtr->getECBoundRadius() );
            _maxECZ = std::max< double >( _maxECZ, maxDrawableECZ );
        }

        // Determine if we really need to move to a new Draw Node, or can we
        // just reuse the last Draw Node. If we do need a new Draw Node,
        // determine whether we need an execute callback to specify the transform.
        bool needNewDrawNode;
        if( ( _currentNodes == NULL ) || _currentNodes->empty() )
        {
            if( _currentNodes == NULL )
                setCurrentNodeContainer( 0 );
            needNewDrawNode = true;
        }
        else
        {
            jagDraw::DrawNodePtr& lastDrawNode( (*_currentNodes)[ _currentNodes->size()-1 ] );
            const bool matrixChanged( _matrixStack.back() != lastDrawNode->getTransform() );

            const jagDraw::CommandMap& lastCommands( *( lastDrawNode->getCommandMap() ) );
            needNewDrawNode = ( matrixChanged ||
                ( _commandStack.back() != lastCommands ) );
        }

        if( needNewDrawNode )
            _currentNodes->push_back( jagDraw::DrawNodePtr( new jagDraw::Node() ) );
        jagDraw::DrawNodePtr& drawNode( (*_currentNodes)[ _currentNodes->size()-1 ] );

        if( needNewDrawNode )
        {
            // Set eye coord distance (for distance-based render order control ).
            drawNode->setDistance( _infoPtr->getECBoundDistance() );

            // Model matrix changed; add callback to update the transform uniforms.
            drawNode->getExecuteCallbacks().push_back( _drawTransformCallback );
            // Set the model matrix.
            drawNode->setTransform( _matrixStack.back() );

            //JAG3D_WARNING( "TBD Must allocate new CommandMapPtr?" );
            jagDraw::CommandMapPtr commands( new jagDraw::CommandMap(
                    _commandStack.back() ) );
            drawNode->setCommandMap( commands );
        }

        for( unsigned int idx=0; idx < numDrawables; ++idx )
        {
            drawNode->addDrawable( node.getDrawable( idx ) );
        }
    }
    }

    Visitor::visit( node );

    popPlanes();
    if( msh.getDirty() )
    {
        _transform.setModel( _matrixStack[ _matrixStack.size()-2 ] );
    }
}


jagDraw::DrawGraphPtr ABufferCollectionVisitor::getDrawGraph() const
{
    return( _drawGraph );
}

void ABufferCollectionVisitor::setDrawGraphTemplate( jagDraw::DrawGraphPtr drawGraphTemplate )
{
    _drawGraphTemplate = drawGraphTemplate;
    _drawGraph = jagDraw::DrawGraphPtr( new jagDraw::DrawGraph( *_drawGraphTemplate ) );

    if( _drawTransformCallback != NULL )
        _drawGraph->setTransformCallback( _drawTransformCallback );
}
const jagDraw::DrawGraphPtr ABufferCollectionVisitor::getDrawGraphTemplate() const
{
    return( _drawGraphTemplate );
}


void ABufferCollectionVisitor::setNearFarOps( const NearFarOps nearFarOps )
{
    _nearFarOps = nearFarOps;
}
ABufferCollectionVisitor::NearFarOps ABufferCollectionVisitor::getNearFarOps() const
{
    return( _nearFarOps );
}

void ABufferCollectionVisitor::getNearFar( double& minECNear, double& maxECFar, const double ratio )
{
    if( _minECZ > _maxECZ )
    {
        // This typically happens when everything is culled and no bounds
        // were used to modify the min and max eye coord z values. In
        // this case, the returned values are irrelevant.
        minECNear = 1.;
        maxECFar = 100.;
        return;
    }
         
    const double scaledNear( _maxECZ * ratio );
    if( _minECZ < scaledNear )
        minECNear = scaledNear;
    else
        minECNear = _minECZ;
    maxECFar = _maxECZ;
}
void ABufferCollectionVisitor::recomputeNearFar( double& minECNear, double& maxECFar, const gmtl::Matrix44d& view, const double ratio )
{
    // The transform we're interested in is Vd = V * Vn(-1), where:
    //   V      = Original view matrix used during collection
    //   Vn(-1) = New view matrix the app is using for next draw traversal.
    //   Vd     = Inverse of the difference between the two matrices.
    // Multiplying the eye coord origin (0,0,0) by Vd produces the
    // change in view position from the original view matrix to the new
    // view matrix. We add the z component of that change to the old
    // computed near/far values to yield an approximation of the new
    // near/far values.

    gmtl::Matrix44d viewInv;
    gmtl::invert( viewInv, view );
    const gmtl::Point4d deltaEC = _transform.getView() * viewInv * gmtl::Point4d( 0., 0., 0., 1. );
    const double newMinZ = _minECZ + deltaEC[ 2 ];
    const double newMaxZ = _maxECZ + deltaEC[ 2 ];

    // Scale and clamp as usual.
    const double scaledNear( newMaxZ * ratio );
    if( newMinZ < scaledNear )
        minECNear = scaledNear;
    else
        minECNear = newMinZ;
    maxECFar = newMaxZ;
}


void ABufferCollectionVisitor::setFrustumPlanes( const FrustumPlanes frustumPlanes )
{
    _frustumPlanes = frustumPlanes;
    resetPlanes();
}
ABufferCollectionVisitor::FrustumPlanes ABufferCollectionVisitor::getFrustumPlanes() const
{
    return( _frustumPlanes );
}


void ABufferCollectionVisitor::setCurrentNodeContainer( const unsigned int currentID )
{
    _currentID = currentID;

    if( ( _drawGraph == NULL ) ||
        ( _currentID >= _drawGraph->size() ) )
    {
        if( _drawGraph == NULL )
            // No draw graph template specified.
            _drawGraph = jagDraw::DrawGraphPtr( new jagDraw::DrawGraph() );

        // Either off the end of the draw graph,
        // or no draw graph template specified.
        _drawGraph->resize( _currentID + 1 );
    }
    _currentNodes = &( (*_drawGraph)[ _currentID ] );
}
unsigned int ABufferCollectionVisitor::getCurrentNodeContainer() const
{
    return( _currentID );
}




ABufferCollectionVisitor::CollectionInfo::CollectionInfo( jagBase::TransformD& transform )
    : Node::CallbackInfo(),
      _transform( transform ),
      _bound( NULL )
    // Initialization of these variables is handled in setBound():
    //   _ecDistance
    //   _ecDistanceDirty
    //   _ecRadius
    //   _ecRadiusDirty
    //   _wcLengthCoeff
    //   _wcLengthCoeffDirty
    // Initialization of these variables is handled in setContainmentPlanes():
    //   _planes
    //   _indices
{
}
ABufferCollectionVisitor::CollectionInfo::CollectionInfo( const CollectionInfo& rhs )
    : Node::CallbackInfo( rhs ),
      _transform( rhs._transform ),
      _bound( rhs._bound )
    // Initialization of these variables is handled in setBound():
    //   _ecDistance
    //   _ecDistanceDirty
    //   _ecRadius
    //   _ecRadiusDirty
    //   _wcLengthCoeff
    //   _wcLengthCoeffDirty
    // Initialization of these variables is handled in setContainmentPlanes():
    //   _planes
    //   _indices
{
}
ABufferCollectionVisitor::CollectionInfo::~CollectionInfo()
{
}


void ABufferCollectionVisitor::CollectionInfo::setBound( jagDraw::Bound* bound )
{
    _ecDistanceDirty = true;
    _ecRadiusDirty = true;
    _wcLengthCoeffDirty = true;

    _bound = bound;
}

double ABufferCollectionVisitor::CollectionInfo::getECBoundDistance() const
{
    if( _ecDistanceDirty )
    {
        const gmtl::Matrix44d& mv( _transform.getModelView() );
        const gmtl::Point3d& c( _bound->getCenter() );
        _ecDistance = -( mv( 2, 0 ) * c[ 0 ] +
            mv( 2, 1 ) * c[ 1 ] +
            mv( 2, 2 ) * c[ 2 ] +
            mv( 2, 3 ) );
        _ecDistanceDirty = false;
    }
    return( _ecDistance );
}
double ABufferCollectionVisitor::CollectionInfo::getECBoundRadius() const
{
    if( _ecRadiusDirty )
    {
        const gmtl::Vec3d radPrime( _transform.getModelView() *
            gmtl::Vec3d( _bound->getRadius(), 0., 0. ) );
        _ecRadius = gmtl::length< double >( radPrime );
        _ecRadiusDirty = false;
    }
    return( _ecRadius );
}
double ABufferCollectionVisitor::CollectionInfo::getWinCLength( double ecSegmentLength ) const
{
    if( _wcLengthCoeffDirty || _ecDistanceDirty )
    {
        // Formula to compute pixel distance from ec length:
        // ( d * 0.5 * w * p0 ) / ( z * p11 + p15 )
        //   d: ec length (parameter)
        //   z: ec z value
        //   w: viewport width
        //   p: projection matrix (16 elements, 0-based)

        int x, y, w, h;
        _transform.getViewport( x, y, w, h );
        const double* p( _transform.getProj().getData() );
        double ecZ( -getECBoundDistance() );

        if( ecZ >= 0. )
            _wcLengthCoeff = FLT_MAX;
        else
            _wcLengthCoeff = ( 0.5 * w * p[0] ) / ( ecZ * p[11] + p[15] );
        _wcLengthCoeffDirty = false;
    }
    return( _wcLengthCoeff * ecSegmentLength );
}

void ABufferCollectionVisitor::CollectionInfo::setContainmentPlanes( ABufferCollectionVisitor::IndexList* indices )
{
    _indices = indices;

    const gmtl::Frustumd& f( _transform.getFrustum() );
    _planes.clear();
    for( unsigned int idx=0; idx<6; ++idx )
        _planes.push_back( f.mPlanes[ idx ] );
}
bool ABufferCollectionVisitor::CollectionInfo::isContained() const
{
    const jagBase::TransformD::FTYPE frustum( _transform.getFrustum() );
    if( _bound->getType() == jagDraw::Bound::Box_t )
        return( gmtl::contains( _planes, *_indices, _bound->asAABox() ) );
    else
        return( gmtl::contains( _planes, *_indices, _bound->asSphere() ) );
}



// jagSG
}
