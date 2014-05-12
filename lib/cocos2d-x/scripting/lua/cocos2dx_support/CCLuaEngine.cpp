/****************************************************************************
 Copyright (c) 2011 cocos2d-x.org

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "CCLuaEngine.h"
#include "cocos2d.h"
#include "cocoa/CCArray.h"
#include "cocoa/CCScriptEventDispatcher.h"
#include "CCScheduler.h"
#include "cocos-ext.h"

NS_CC_BEGIN

CCLuaEngine* CCLuaEngine::m_defaultEngine = NULL;

CCLuaEngine* CCLuaEngine::defaultEngine(void)
{
    if (!m_defaultEngine)
    {
        m_defaultEngine = new CCLuaEngine();
        m_defaultEngine->init();
    }
    return m_defaultEngine;
}

CCLuaEngine::~CCLuaEngine(void)
{
    CC_SAFE_RELEASE(m_stack);
    m_defaultEngine = NULL;
}

bool CCLuaEngine::init(void)
{
    m_stack = CCLuaStack::create();
    m_stack->retain();
    return true;
}

void CCLuaEngine::addSearchPath(const char* path)
{
    m_stack->addSearchPath(path);
}

void CCLuaEngine::addLuaLoader(lua_CFunction func)
{
    m_stack->addLuaLoader(func);
}

void CCLuaEngine::removeScriptObjectByCCObject(CCObject* pObj)
{
    m_stack->removeScriptObjectByCCObject(pObj);
}

void CCLuaEngine::removeScriptHandler(int nHandler)
{
    m_stack->removeScriptHandler(nHandler);
}

int CCLuaEngine::executeString(const char *codes)
{
    int ret = m_stack->executeString(codes);
    m_stack->clean();
    return ret;
}

int CCLuaEngine::executeScriptFile(const char* filename)
{
    int ret = m_stack->executeScriptFile(filename);
    m_stack->clean();
    return ret;
}

int CCLuaEngine::executeGlobalFunction(const char* functionName, int numArgs /* = 0 */)
{
    int ret = m_stack->executeGlobalFunction(functionName, numArgs);
    m_stack->clean();
    return ret;
}

int CCLuaEngine::executeNodeEvent(CCNode* pNode, int nAction)
{
    m_stack->clean();
    switch (nAction)
    {
        case kCCNodeOnEnter:
            m_stack->pushString("enter");
            break;

        case kCCNodeOnExit:
            m_stack->pushString("exit");
            break;

        case kCCNodeOnEnterTransitionDidFinish:
            m_stack->pushString("enterTransitionFinish");
            break;

        case kCCNodeOnExitTransitionDidStart:
            m_stack->pushString("exitTransitionStart");
            break;

        case kCCNodeOnCleanup:
            m_stack->pushString("cleanup");
            break;

        default:
            return 0;
    }

    CCScriptEventListenersForEvent &listeners = pNode->getScriptEventListenersByEvent(NODE_EVENT);
    CCScriptEventListenersForEventIterator it = listeners.begin();
    for (; it != listeners.end(); ++it)
    {
        m_stack->copyValue(1);
        m_stack->executeFunctionByHandler(it->listener, 1);
        m_stack->settop(1);
    }
    m_stack->clean();
    return 0;
}

int CCLuaEngine::executeNodeEnterFrameEvent(CCNode* pNode, float dt)
{
    CCScriptEventListenersForEvent &listeners = pNode->getScriptEventListenersByEvent(NODE_ENTER_FRAME_EVENT);
    CCScriptEventListenersForEventIterator it = listeners.begin();
    for (; it != listeners.end(); ++it)
    {
        m_stack->pushFloat(dt);
        m_stack->executeFunctionByHandler(it->listener, 1);
        m_stack->clean();
    }
    return 0;
}

int CCLuaEngine::executeMenuItemEvent(CCMenuItem* pMenuItem)
{
    CCScriptEventListenersForEvent &listeners = pMenuItem->getScriptEventListenersByEvent(MENU_ITEM_CLICKED_EVENT);
    CCScriptEventListenersForEventIterator it = listeners.begin();
    for (; it != listeners.end(); ++it)
    {
        m_stack->pushInt(pMenuItem->getTag());
        m_stack->pushCCObject(pMenuItem, "CCMenuItem");
        m_stack->executeFunctionByHandler(it->listener, 2);
        m_stack->clean();
    }
    return 0;
}

int CCLuaEngine::executeNotificationEvent(CCNotificationCenter* pNotificationCenter, const char* pszName, CCObject *obj /* = NULL */)
{
    int nHandler = pNotificationCenter->getObserverHandlerByName(pszName);
    if (!nHandler) return 0;

    m_stack->pushString(pszName);

    if (obj)
        m_stack->pushCCObject(obj, "CCObject");
    else
        m_stack->pushNil();

    int ret = m_stack->executeFunctionByHandler(nHandler, 2);
    m_stack->clean();
    return ret;
}

int CCLuaEngine::executeCallFuncActionEvent(CCCallFunc* pAction, CCObject* pTarget/* = NULL*/)
{
    int nHandler = pAction->getScriptHandler();
    if (!nHandler) return 0;

    if (pTarget)
    {
        m_stack->pushCCObject(pTarget, "CCNode");
    }
    int ret = m_stack->executeFunctionByHandler(nHandler, pTarget ? 1 : 0);
    m_stack->clean();
    return ret;
}

int CCLuaEngine::executeSchedule(int nHandler, float dt, CCNode* pNode/* = NULL*/)
{
    if (!nHandler) return 0;
    m_stack->pushFloat(dt);
    int ret = m_stack->executeFunctionByHandler(nHandler, 1);
    m_stack->clean();
    return ret;
}

int CCLuaEngine::executeNodeTouchEvent(CCNode* pNode, int eventType, CCTouch *pTouch)
{
    CCScriptEventListenersForEvent &listeners = pNode->getScriptEventListenersByEvent(NODE_TOUCH_EVENT);
    if (listeners.size() == 0) return 0;

    lua_State *L = getLuaStack()->getLuaState();
    lua_settop(L, 0);
    lua_newtable(L);
    switch (eventType)
    {
        case CCTOUCHBEGAN:
            lua_pushliteral(L, "began");
            break;

        case CCTOUCHMOVED:
            lua_pushliteral(L, "moved");
            break;

        case CCTOUCHENDED:
            lua_pushliteral(L, "ended");
            break;

        case CCTOUCHCANCELLED:
            lua_pushliteral(L, "cancelled");
            break;

        default:
            lua_pop(L, 1);
            return 0;
    }
    lua_setfield(L, -2, "name");

    const CCPoint pt = CCDirector::sharedDirector()->convertToGL(pTouch->getLocationInView());
    lua_pushnumber(L, pt.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, pt.y);
    lua_setfield(L, -2, "y");
    const CCPoint prev = CCDirector::sharedDirector()->convertToGL(pTouch->getPreviousLocationInView());
    lua_pushnumber(L, prev.x);
    lua_setfield(L, -2, "prevX");
    lua_pushnumber(L, prev.y);
    lua_setfield(L, -2, "prevY");

    CCScriptEventListenersForEventIterator it = listeners.begin();
    for (; it != listeners.end(); ++it)
    {
        if (eventType == CCTOUCHBEGAN || eventType == CCTOUCHENDED || eventType == CCTOUCHCANCELLED)
        {
            (*it).enabled = true;
        }
        else if (!(*it).enabled)
        {
            continue;
        }

        lua_pushvalue(L, -1);
        int ret = m_stack->executeFunctionByHandler((*it).listener, 1);
        lua_settop(L, 1);

        if ((eventType == CCTOUCHBEGAN) && (ret == false))
        {
            // false = ignore
            (*it).enabled = false;
        }
    }

    return 1;
}

int CCLuaEngine::executeNodeTouchesEvent(CCNode* pNode, int eventType, CCSet *pTouches)
{
    CCScriptEventListenersForEvent &listeners = pNode->getScriptEventListenersByEvent(NODE_TOUCH_EVENT);
    if (listeners.size() == 0) return 0;

    CCDirector* pDirector = CCDirector::sharedDirector();
    lua_State *L = m_stack->getLuaState();

    CCScriptEventListenersForEventIterator it = listeners.begin();
    for (; it != listeners.end(); ++it)
    {
        if (eventType == CCTOUCHBEGAN || eventType == CCTOUCHENDED || eventType == CCTOUCHCANCELLED)
        {
            (*it).enabled = true;
        }
        else if (!(*it).enabled)
        {
            continue;
        }

        switch (eventType)
        {
            case CCTOUCHBEGAN:
                m_stack->pushString("began");
                break;

            case CCTOUCHMOVED:
                m_stack->pushString("moved");
                break;

            case CCTOUCHENDED:
                m_stack->pushString("ended");
                break;

            case CCTOUCHCANCELLED:
                m_stack->pushString("cancelled");
                break;

            default:
                return 0;
        }

        lua_newtable(L);
        lua_newtable(L);
        int i = 1;
        for (CCSetIterator it2 = pTouches->begin(); it2 != pTouches->end(); ++it2)
        {
            CCTouch* pTouch = (CCTouch*)*it2;
            const CCPoint pt = pDirector->convertToGL(pTouch->getLocationInView());
            lua_pushnumber(L, pt.x);
            lua_rawseti(L, -3, i);
            lua_pushnumber(L, pt.y);
            lua_rawseti(L, -3, i + 1);
            lua_pushinteger(L, pTouch->getID());
            lua_rawseti(L, -3, i + 2);

            const CCPoint prev = pDirector->convertToGL(pTouch->getPreviousLocationInView());
            lua_pushnumber(L, prev.x);
            lua_rawseti(L, -2, i);
            lua_pushnumber(L, prev.y);
            lua_rawseti(L, -2, i + 1);
            lua_pushinteger(L, pTouch->getID());
            lua_rawseti(L, -2, i + 2);
            
            i += 3;
        }
        int ret = m_stack->executeFunctionByHandler((*it).listener, 3);
        m_stack->clean();

        if (ret == 0)
        {
            // false = ignore
            (*it).enabled = false;
        }
    }
    
    return 1;
}

int CCLuaEngine::executeLayerKeypadEvent(CCLayer* pLayer, int eventType)
{
    m_stack->clean();
    switch (eventType)
    {
        case kTypeBackClicked:
            m_stack->pushString("back");
            break;

        case kTypeMenuClicked:
            m_stack->pushString("menu");
            break;

        default:
            return 0;
    }

    CCScriptEventListenersForEvent &listeners = pLayer->getScriptEventListenersByEvent(KEYPAD_EVENT);
    CCScriptEventListenersForEventIterator it = listeners.begin();
    for (; it != listeners.end(); ++it)
    {
        m_stack->copyValue(1);
        m_stack->executeFunctionByHandler(it->listener, 1);
        m_stack->settop(1);
    }
    m_stack->clean();
    return 0;
}

int CCLuaEngine::executeAccelerometerEvent(CCLayer* pLayer, CCAcceleration* pAccelerationValue)
{
    CCScriptEventListenersForEvent &listeners = pLayer->getScriptEventListenersByEvent(ACCELERATE_EVENT);
    CCScriptEventListenersForEventIterator it = listeners.begin();
    for (; it != listeners.end(); ++it)
    {
        m_stack->pushFloat(pAccelerationValue->x);
        m_stack->pushFloat(pAccelerationValue->y);
        m_stack->pushFloat(pAccelerationValue->z);
        m_stack->pushFloat(pAccelerationValue->timestamp);
        m_stack->executeFunctionByHandler(it->listener, 4);
        m_stack->clean();
    }
    return 0;
}

int CCLuaEngine::executeEvent(int nHandler, const char* pEventName, CCObject* pEventSource /* = NULL*/, const char* pEventSourceClassName /* = NULL*/)
{
    m_stack->pushString(pEventName);
    if (pEventSource)
    {
        m_stack->pushCCObject(pEventSource, pEventSourceClassName ? pEventSourceClassName : "CCObject");
    }
    int ret = m_stack->executeFunctionByHandler(nHandler, pEventSource ? 2 : 1);
    m_stack->clean();
    return ret;
}

bool CCLuaEngine::handleAssert(const char *msg)
{
    bool ret = m_stack->handleAssert(msg);
    m_stack->clean();
    return ret;
}

int CCLuaEngine::reallocateScriptHandler(int nHandler)
{
    int nRet = m_stack->reallocateScriptHandler(nHandler);
    m_stack->clean();
    return nRet;
}

#ifndef QUICK_MINI_TARGET

int CCLuaEngine::executeTableViewEvent(int nEventType,cocos2d::extension::CCTableView* pTableView,void* pValue, CCArray* pResultArray)
{
    if (NULL == pTableView)
        return 0;

    int nHanlder = pTableView->getScriptHandler(nEventType);
    if (0 == nHanlder)
        return 0;

    int nRet = 0;
    switch (nEventType)
    {
        case cocos2d::extension::CCTableView::kTableViewScroll:
        case cocos2d::extension::CCTableView::kTableViewZoom:
        {
            m_stack->pushCCObject(pTableView, "CCTableView");
            nRet = m_stack->executeFunctionByHandler(nHanlder, 1);
        }
            break;
        case cocos2d::extension::CCTableView::kTableCellTouched:
        case cocos2d::extension::CCTableView::kTableCellHighLight:
        case cocos2d::extension::CCTableView::kTableCellUnhighLight:
        case cocos2d::extension::CCTableView::kTableCellWillRecycle:
        {
            m_stack->pushCCObject(pTableView, "CCTableView");
            m_stack->pushCCObject(static_cast<cocos2d::extension::CCTableViewCell*>(pValue), "CCTableViewCell");
            nRet = m_stack->executeFunctionByHandler(nHanlder, 2);
        }
            break;
        case cocos2d::extension::CCTableView::kTableCellSizeForIndex:
        {
            m_stack->pushCCObject(pTableView, "CCTableView");
            m_stack->pushInt(*((int*)pValue));
            nRet = m_stack->executeFunctionReturnArray(nHanlder, 2, 2, pResultArray);
        }
            break;
        case cocos2d::extension::CCTableView::kTableCellSizeAtIndex:
        {
            m_stack->pushCCObject(pTableView, "CCTableView");
            m_stack->pushInt(*((int*)pValue));
            nRet = m_stack->executeFunctionReturnArray(nHanlder, 2, 1, pResultArray);
        }
            break;
        case cocos2d::extension::CCTableView::kNumberOfCellsInTableView:
        {
            m_stack->pushCCObject(pTableView, "CCTableView");
            nRet = m_stack->executeFunctionReturnArray(nHanlder, 1, 1, pResultArray);
        }
            break;
        default:
            break;
    }
    return nRet;
}

#endif // QUICK_MINI_TARGET

int CCLuaEngine::executeEventWithArgs(int nHandler, CCArray* pArgs)
{
    if (NULL == pArgs)
        return 0;

    CCObject*   pObject = NULL;

    CCInteger*  pIntVal = NULL;
    CCString*   pStrVal = NULL;
    CCDouble*   pDoubleVal = NULL;
    CCFloat*    pFloatVal = NULL;
    CCBool*     pBoolVal = NULL;


    int nArgNums = 0;
    for (unsigned int i = 0; i < pArgs->count(); i++)
    {
        pObject = pArgs->objectAtIndex(i);
        if (NULL != (pIntVal = dynamic_cast<CCInteger*>(pObject)))
        {
            m_stack->pushInt(pIntVal->getValue());
            nArgNums++;
        }
        else if (NULL != (pStrVal = dynamic_cast<CCString*>(pObject)))
        {
            m_stack->pushString(pStrVal->getCString());
            nArgNums++;
        }
        else if (NULL != (pDoubleVal = dynamic_cast<CCDouble*>(pObject)))
        {
            m_stack->pushFloat(pDoubleVal->getValue());
            nArgNums++;
        }
        else if (NULL != (pFloatVal = dynamic_cast<CCFloat*>(pObject)))
        {
            m_stack->pushFloat(pFloatVal->getValue());
            nArgNums++;
        }
        else if (NULL != (pBoolVal = dynamic_cast<CCBool*>(pObject)))
        {
            m_stack->pushBoolean(pBoolVal->getValue());
            nArgNums++;
        }
        else if(NULL != pObject)
        {
            m_stack->pushCCObject(pObject, "CCObject");
            nArgNums++;
        }
    }

    return  m_stack->executeFunctionByHandler(nHandler, nArgNums);
}

bool CCLuaEngine::parseConfig(CCScriptEngineProtocol::ConfigType type, const std::string& str)
{
    lua_getglobal(m_stack->getLuaState(), "__onParseConfig");
    if (!lua_isfunction(m_stack->getLuaState(), -1))
    {
        CCLOG("[LUA ERROR] name '%s' does not represent a Lua function", "__onParseConfig");
        lua_pop(m_stack->getLuaState(), 1);
        return false;
    }
    
    m_stack->pushInt((int)type);
    m_stack->pushString(str.c_str());
    
    return m_stack->executeFunction(2);
}

NS_CC_END
