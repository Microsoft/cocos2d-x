/*
* cocos2d-x   http://www.cocos2d-x.org
*
* Copyright (c) 2010-2014 - cocos2d-x community
*
* Portions Copyright (c) Microsoft Open Technologies, Inc.
* All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and limitations under the License.
*/

#include "Cocos2dRenderer.h"
#include "AppDelegate.h"
#include "CCEglView-Win8_1.h"
#include "CCApplication.h"

// These are used by the shader compilation methods.
#include <vector>
#include <iostream>
#include <fstream>

using namespace Platform;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::Graphics::Display;

USING_NS_CC;

Cocos2dRenderer::Cocos2dRenderer(int width, int height, float dpi, DisplayOrientations orientation, CoreDispatcher^ dispatcher, Panel^ panel)
    : m_app(nullptr)
    , m_width(width)
    , m_height(height)
    , m_dpi(dpi)
    , m_dispatcher(dispatcher)
    , m_panel(panel)
    , m_orientation(orientation)
{
    m_app = new AppDelegate();
}

Cocos2dRenderer::~Cocos2dRenderer()
{
    delete m_app;
}

void Cocos2dRenderer::Resume()
{
    CCEGLView* glView = CCEGLView::sharedOpenGLView();

    if (!glView)
    {
        CCEGLView* glView = new CCEGLView();
        glView->Create(m_width, m_height);
        glView->setViewName("Cocos2d-x");
        glView->setDispatcher(m_dispatcher.Get());
        glView->setPanel(m_panel.Get());
        CCApplication::sharedApplication()->run();
    }
    else
    {
        CCApplication::sharedApplication()->applicationWillEnterForeground();
    }
}

void Cocos2dRenderer::Pause()
{
    CCApplication::sharedApplication()->applicationDidEnterBackground();

}

bool Cocos2dRenderer::AppShouldExit()
{
    return CCEGLView::sharedOpenGLView()->AppShouldExit();
}

void Cocos2dRenderer::DeviceLost()
{
    Pause();

#if 0
    auto director = cocos2d::Director::getInstance();
    if (director->getOpenglView()) {
        cocos2d::GL::invalidateStateCache();
        cocos2d::GLProgramCache::getInstance()->reloadDefaultGLPrograms();
        cocos2d::DrawPrimitives::init();
        cocos2d::VolatileTextureMgr::reloadAllTextures();

        cocos2d::EventCustom recreatedEvent(EVENT_RENDERER_RECREATED);
        director->getEventDispatcher()->dispatchEvent(&recreatedEvent);
        director->setGLDefaultValues();

        Application::getInstance()->applicationWillEnterForeground();
        cocos2d::EventCustom foregroundEvent(EVENT_COME_TO_FOREGROUND);
        cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&foregroundEvent);
    }
#endif
}

void Cocos2dRenderer::Draw(GLsizei width, GLsizei height, float dpi, DisplayOrientations orientation)
{
    CCEGLView* glView = CCEGLView::sharedOpenGLView();

    if (orientation != m_orientation)
    {
        m_orientation = orientation;
        glView->UpdateOrientation(orientation);
    }

    if (width != m_width || height != m_height)
    { 
        m_width = width;
        m_height = height;
        glView->UpdateForWindowSizeChange(static_cast<float>(width), static_cast<float>(height));
    }

    if (dpi != m_dpi)
    {
        m_dpi = dpi;
        //glView->SetDPI(m_dpi);
    }

    glView->Render();
}

void Cocos2dRenderer::QueuePointerEvent(cocos2d::PointerEventType type, Windows::UI::Core::PointerEventArgs^ args)
{
    CCEGLView::sharedOpenGLView()->QueuePointerEvent(type, args);
}

void Cocos2dRenderer::QueueBackButtonEvent()
{
    CCEGLView::sharedOpenGLView()->QueueBackKeyPress();
}

void Cocos2dRenderer::QueueKeyboardEvent(cocos2d::Cocos2dKeyEvent type, Windows::UI::Core::KeyEventArgs^ args)
{
    // keyboard events are handled internally
	//CCEglView::sharedOpenglView()->QueueWinRTKeyboardEvent(type, args);
}



