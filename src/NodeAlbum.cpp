/*
 *  NodeAlbum.cpp
 *  Bloom
 *
 *  Created by Robert Hodgin on 1/21/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "cinder/app/AppBasic.h"
#include "NodeArtist.h"
#include "NodeAlbum.h"
#include "NodeTrack.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/PolyLine.h"
#include "Globals.h"

using namespace ci;
using namespace ci::ipod;
using namespace std;

NodeAlbum::NodeAlbum( Node *parent, int index, const Font &font )
	: Node( parent, index, font )
{
	mIsHighlighted	= true;
	
	// FIXME: bad c++?
	float numAlbums = ((NodeArtist*)mParentNode)->mNumAlbums + 2.0f;
	
	float invAlbumPer = 1.0f/(float)numAlbums;
	float albumNumPer = (float)mIndex * invAlbumPer;
	
	float minAmt		= mParentNode->mRadiusDest * 1.0f;
	float maxAmt		= mParentNode->mRadiusDest * 3.0f;
	float deltaAmt		= maxAmt - minAmt;
	mOrbitRadiusDest	= minAmt + deltaAmt * albumNumPer + Rand::randFloat( maxAmt * invAlbumPer * 0.35f );

	mIdealCameraDist = mRadius * 6.0f;
    mPlanetTexIndex = 0;
	
    mRadius         *= 0.85f;
	mSphere			= Sphere( mPos, mRadius * 7.5f );
}


void NodeAlbum::setData( PlaylistRef album )
{
	mAlbum				= album;
	mNumTracks			= mAlbum->size();
	mHighestPlayCount	= 0;
	mLowestPlayCount	= 10000;
	
	float hue			= Rand::randFloat( 0.15f, 0.75f );
	float sat			= Rand::randFloat( 0.15f, 0.25f );
	float val			= Rand::randFloat( 0.85f, 1.00f );
	mColor				= Color( CM_HSV, hue, sat, val );
	mEclipseColor       = mColor;

    mAxialVel       = Rand::randFloat( 10.0f, 45.0f );
    
    mPlanetTexIndex = ( mIndex + 6 )%( G_NUM_PLANET_TYPES * G_NUM_PLANET_TYPE_OPTIONS );//(int)( normPlayCount * ( G_NUM_PLANET_TYPES - 1 ) );
	mCloudTexIndex  = Rand::randInt( G_NUM_CLOUD_TYPES );

	for (int i = 0; i < mNumTracks; i++) {
		float numPlays = (*mAlbum)[i]->getPlayCount();
		if( numPlays < mLowestPlayCount )
			mLowestPlayCount = numPlays;
		if( numPlays > mHighestPlayCount )
			mHighestPlayCount = numPlays;
	}
}


void NodeAlbum::update( const Matrix44f &mat )
{
	double playbackTime		= app::getElapsedSeconds();
	double percentPlayed	= playbackTime/mOrbitPeriod;
	double orbitAngle		= percentPlayed * TWO_PI + mStartAngle;
    
	mPrevPos	= mTransPos;
	
	mRelPos		= Vec3f( cos( orbitAngle ), sin( orbitAngle ), 0.0f ) * mOrbitRadius;
	mPos		= mParentNode->mPos + mRelPos;
    
    float eclipseDist = 1.0f;
    if( mIsSelected && mParentNode->mDistFromCamZAxisPer > 0.0f ){
        float dist = mScreenPos.distance( mParentNode->mScreenPos );
        eclipseDist = constrain( dist/200.0f, 0.0f, 1.0f );
    }
	mEclipseColor = mColor * eclipseDist;
    
	repelOrbitTracks();
    
	Node::update( mat );
	
	mVel		= mTransPos - mPrevPos;	
}

void NodeAlbum::repelOrbitTracks()
{
	if( mIsSelected ){
		float minAmt		= mRadius * 2.0f;
		float maxAmt		= mRadius * 8.0f;
		for( vector<Node*>::iterator p1 = mChildNodes.begin(); p1 != mChildNodes.end(); ++p1 ){
			
			vector<Node*>::iterator p2 = p1;
			for( ++p2; p2 != mChildNodes.end(); ++p2 ) {
				float dist = abs( (*p2)->mOrbitRadiusDest - (*p1)->mOrbitRadiusDest );
				//float radiusTotal = (*p1)->mRadius + (*p2)->mRadius;
				if( dist > 0 ){
					float F = 1.0f/(dist*10.0f);
					
					(*p1)->mOrbitRadiusDestAcc -= ( F / ((*p2)->mMass * 10000000.0f ) );
					(*p2)->mOrbitRadiusDestAcc += ( F / ((*p1)->mMass * 10000000.0f ) );
				}
			}
		}
		
		(*mChildNodes.front()).mOrbitRadiusDest = minAmt;
		(*mChildNodes.back()).mOrbitRadiusDest = maxAmt;
	}
}


void NodeAlbum::drawOrbitRing( NodeTrack *playingNode, GLfloat *ringVertsLowRes, GLfloat *ringVertsHighRes )
{
	if( mIsSelected ){
		gl::color( ColorA( 0.15f, 0.2f, 0.4f, 0.5f ) );
	} else {
		gl::color( ColorA( 0.15f, 0.2f, 0.4f, 0.2f ) );
	}
	gl::pushModelView();
	gl::translate( mParentNode->mTransPos );
	gl::scale( Vec3f( mOrbitRadius, mOrbitRadius, mOrbitRadius ) );
	gl::rotate( mMatrix );
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, ringVertsHighRes );
	glDrawArrays( GL_LINE_STRIP, 0, G_RING_HIGH_RES );
	glDisableClientState( GL_VERTEX_ARRAY );
	gl::popModelView();
	
	Node::drawOrbitRing( playingNode, ringVertsLowRes, ringVertsHighRes );
}

void NodeAlbum::drawPlanet( const vector<gl::Texture> &planets )
{	
	glEnable( GL_RESCALE_NORMAL );
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	int numVerts;
	if( mIsSelected ){
		glVertexPointer( 3, GL_FLOAT, 0, mSphereVertsHiRes );
		glTexCoordPointer( 2, GL_FLOAT, 0, mSphereTexCoordsHiRes );
		glNormalPointer( GL_FLOAT, 0, mSphereNormalsHiRes );
		numVerts = mTotalVertsHiRes;
	} else {
		glVertexPointer( 3, GL_FLOAT, 0, mSphereVertsLoRes );
		glTexCoordPointer( 2, GL_FLOAT, 0, mSphereTexCoordsLoRes );
		glNormalPointer( GL_FLOAT, 0, mSphereNormalsLoRes );
		numVerts = mTotalVertsLoRes;
	}
	
	gl::disableAlphaBlending();
	
    gl::pushModelView();
	gl::translate( mTransPos );
	gl::scale( Vec3f( mRadius, mRadius, mRadius ) );
	gl::rotate( mMatrix );
	gl::rotate( Vec3f( 90.0f, app::getElapsedSeconds() * mAxialVel, 0.0f ) );
	gl::color( mEclipseColor );
	planets[mPlanetTexIndex].enableAndBind();
	glDrawArrays( GL_TRIANGLES, 0, numVerts );
	gl::popModelView();
	
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );	
	
	
	
	/*
	gl::disableAlphaBlending();
	
    gl::pushModelView();
	gl::translate( mTransPos );
	gl::scale( Vec3f( 2.0f, 2.0f, 2.0f ) ) ;
	gl::rotate( mMatrix );
	gl::rotate( Vec3f( 90.0f, app::getElapsedSeconds() * mAxialVel, 0.0f ) );
	gl::color( mEclipseColor );
	planets[mPlanetTexIndex].enableAndBind();
	gl::drawSphere( Vec3f::zero(), mRadius, mSphereResInt );
	gl::popModelView();
    */
	
	
	Node::drawPlanet( planets );
}

void NodeAlbum::drawClouds( const vector<gl::Texture> &clouds )
{
    if( mCamDistAlpha > 0.05f ){
		glEnableClientState( GL_VERTEX_ARRAY );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
		glEnableClientState( GL_NORMAL_ARRAY );
		int numVerts;
		if( mIsSelected ){
			glVertexPointer( 3, GL_FLOAT, 0, mSphereVertsHiRes );
			glTexCoordPointer( 2, GL_FLOAT, 0, mSphereTexCoordsHiRes );
			glNormalPointer( GL_FLOAT, 0, mSphereNormalsHiRes );
			numVerts = mTotalVertsHiRes;
		} else {
			glVertexPointer( 3, GL_FLOAT, 0, mSphereVertsLoRes );
			glTexCoordPointer( 2, GL_FLOAT, 0, mSphereTexCoordsLoRes );
			glNormalPointer( GL_FLOAT, 0, mSphereNormalsLoRes );
			numVerts = mTotalVertsLoRes;
		}
		
		gl::disableAlphaBlending();
		
		gl::pushModelView();
		gl::translate( mTransPos );
		gl::pushModelView();
		float radius = mRadius + 0.00025f;
		gl::scale( Vec3f( radius, radius, radius ) );
		glEnable( GL_RESCALE_NORMAL );
		gl::rotate( mMatrix );
		gl::rotate( Vec3f( 90.0f, app::getElapsedSeconds() * mAxialVel * 0.75f, 0.0f ) );
// SHADOW CLOUDS
		glDisable( GL_LIGHTING );
		gl::disableAlphaBlending();
		gl::enableAlphaBlending();
		gl::color( ColorA( 0.0f, 0.0f, 0.0f, mCamDistAlpha ) );
		clouds[mCloudTexIndex].enableAndBind();
		glDrawArrays( GL_TRIANGLES, 0, numVerts );
		gl::popModelView();
		glEnable( GL_LIGHTING );		
// LIT CLOUDS
		gl::pushModelView();
		radius = mRadius + 0.0008f;
		gl::scale( Vec3f( radius, radius, radius ) );
		glEnable( GL_RESCALE_NORMAL );
		gl::rotate( mMatrix );
		gl::rotate( Vec3f( 90.0f, app::getElapsedSeconds() * mAxialVel * 0.75f, 0.0f ) );
		gl::enableAdditiveBlending();
		gl::color( ColorA( ( mEclipseColor + Color::white() ) * 0.5f, mCamDistAlpha ) );
		glDrawArrays( GL_TRIANGLES, 0, numVerts );
		gl::popModelView();
		gl::popModelView();
		
		glDisableClientState( GL_VERTEX_ARRAY );
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		glDisableClientState( GL_NORMAL_ARRAY );
	}
    
	Node::drawClouds( clouds );
}

void NodeAlbum::select()
{
	if( !mIsSelected && mChildNodes.size() == 0 ){
		for (int i = 0; i < mNumTracks; i++) {
			TrackRef track		= (*mAlbum)[i];
			string name			= track->getTitle();
			std::cout << "trackname = " << name << std::endl;
			NodeTrack *newNode	= new NodeTrack( this, i, mFont );
			mChildNodes.push_back( newNode );
			newNode->setData( track, mAlbum );
		}
		
		for( vector<Node*>::iterator it = mChildNodes.begin(); it != mChildNodes.end(); ++it ){
			(*it)->setSphereData( mTotalVertsHiRes, mSphereVertsHiRes, mSphereTexCoordsHiRes, mSphereNormalsHiRes,
								  mTotalVertsLoRes, mSphereVertsLoRes, mSphereTexCoordsLoRes, mSphereNormalsLoRes );
		}
	}	
	Node::select();
}

string NodeAlbum::getName()
{
	string name = mAlbum->getAlbumTitle();
	if( name.size() < 1 ) name = "Untitled";
	return name;
}
