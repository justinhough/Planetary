/*
 *  NodeTrack.h
 *  Bloom
 *
 *  Created by Robert Hodgin on 1/21/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "Node.h"
#include "CinderIPod.h"

class NodeTrack : public Node
{
  public:
	NodeTrack( Node *parent, int index, const ci::Font &font );
    void initVertexArray();
	void updateAudioData( double currentPlayheadTime );
	void update( const ci::Matrix44f &mat, const ci::Surface &surfaces );
	void drawEclipseGlow();
	void drawPlanet( const std::vector< ci::gl::Texture> &planets );
	void drawClouds( const std::vector< ci::gl::Texture> &clouds );
	void drawOrbitRing( float pinchAlphaOffset, float camAlpha, const ci::gl::Texture &tex, GLfloat *ringVertsLowRes, GLfloat *ringTexLowRes, GLfloat *ringVertsHighRes, GLfloat *ringTexHighRes );
	void buildPlayheadProgressVertexArray();
	void drawPlayheadProgress( float pinchAlphaPer, float camAlpha, const ci::gl::Texture &tex, const ci::gl::Texture &originTex );
	void drawAtmosphere( const ci::gl::Texture &tex, const ci::gl::Texture &directionalTex, float pinchAlphaPer );
	void setData( ci::ipod::TrackRef track, ci::ipod::PlaylistRef album );
	ci::Vec3f getStartRelPos(){ return mMatrix * mStartRelPos; }
	ci::Vec3f getRelPos(){ return mMatrix * mRelPos; }
	void setStartAngle();

	string getName();
    uint64_t getId();
	bool isMostPlayed() { return mIsMostPlayed; }

	// TODO: should this be from a getData() function? or private?
	ci::ipod::TrackRef      mTrack;
	ci::ipod::PlaylistRef   mAlbum;	
	
private:
	float		mAsciiPer;
	float		mEclipseStrength;
	float		mTrackLength;
	int			mPlayCount;
	float		mNormPlayCount;
	int			mStarRating;
	int			mNumTracks;
	ci::Vec3f	mStartPos, mTransStartPos, mStartRelPos;
	vector<ci::Vec3f> mOrbitPath;
	
	float		mPrevTime, mCurrentTime, mMyTime;
	double		mStartTime;
	double		mPlaybackTime;
	double		mPercentPlayed;
	
	bool		mHasClouds;
	bool		mIsPopulated;
	bool		mIsMostPlayed;
	bool		mHasAlbumArt;
	bool		mHasCreatedAlbumArt;
	ci::gl::Texture mAlbumArt;
	
	float		mCloudLayerRadius;
	
	int			mTotalOrbitVertices;
    int			mPrevTotalOrbitVertices;
	GLfloat		*mOrbitVerts;
	GLfloat		*mOrbitTexCoords;
	GLfloat		*mOrbitColors;
};