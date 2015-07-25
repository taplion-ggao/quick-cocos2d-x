//
//  ZYWebView.cpp
//  HelloWorld
//
//  Created by VincentChou on 14-8-6.
//
//

#include "ZYWebView.h"
#include "CCEGLView.h"

ZYWebView::ZYWebView()
{
    
}

bool ZYWebView::init()
{
	return true;
}

void ZYWebView::showWebView(const char* url, float x, float y, float width, float height)
{
    CCSize designsize = CCEGLView::sharedOpenGLView()->getDesignResolutionSize();
    CCSize framesize = CCEGLView::sharedOpenGLView()->getFrameSize();
    float sx = CCEGLView::sharedOpenGLView()->getScaleX();
    float sy = CCEGLView::sharedOpenGLView()->getScaleY();
    CCSize designframe(framesize.width / sx, framesize.height / sy);
    
    // 这里可能需要根据ResolutionPolicy进行修改。
    // Modify this ratio equation depend on your ResolutionPolicy.
    float ratioY = designsize.height / framesize.height;
    float ratioX = designsize.width / framesize.width;
    
    CCPoint orig((designframe.width - designsize.width) / 2, (designframe.height - designsize.height) / 2);
    
    x = x / ratioX + orig.x / ratioX;
    y = y / ratioY + orig.y / ratioY;
    width /= ratioX;
    height /= ratioY;
    
    _privateShowWebView(url, x, y, width, height);
}

