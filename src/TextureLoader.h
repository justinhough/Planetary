//
//  TextureLoader.h
//  Kepler
//
//  Created by Tom Carden on 7/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#include "cinder/Vector.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/Function.h"
#include "cinder/Thread.h"

class TextureLoader {
public:

    TextureLoader() {}
    ~TextureLoader() {}
    
    int getTotal() { return mTotalRequests; }
    int getCount() { return mTextures.size(); }
    float getProgress() { return (float)getCount() / (float)getTotal(); }

    void addRequest( int texId, std::string fileName );
    void addRequest( int texId, std::string fileName, ci::gl::Texture::Format format );
    void addRequest( int texId, std::string compressedFileName, ci::Vec2i size );
    
    void update(); // call every frame or you won't get results!
    
    ci::gl::Texture operator[](const int &index){ return mTextures[index]; };
    
    template<typename T>
	ci::CallbackId registerComplete( T *obj, void ( T::*callback )( TextureLoader* ) ){
		return mCbComplete.registerCb(std::bind1st( std::mem_fun( callback ), obj ) );
	}

    // begin the thread
    void start();

private:
    
    struct Request {
        int mTexId;
        std::string mFileName;
        ci::Surface mSurface;
        ci::gl::Texture::Format mFormat;
        
        Request( int texId, std::string fileName, ci::gl::Texture::Format format ): mTexId(texId), mFileName(fileName), mFormat(format) {}
    };     
    
    std::mutex mRequestsMutex;
    std::vector<Request> mRequests;
    int mTotalRequests; // mRequests.size() but threadsafe
    int mRequestsComplete;

    std::map<int, ci::gl::Texture> mTextures;
    
    ci::CallbackMgr<void(TextureLoader*)> mCbComplete;        

    void loadSurfaces();

};