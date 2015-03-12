#include "CCCursorMultiTextFieldTTF.h"

#include "CCDirector.h"
#include "touch_dispatcher/CCTouchDispatcher.h"
#include "CCActionInterval.h"
#include "platform/CCImage.h"

NS_CC_BEGIN
static int _calcCharCount(const char * pszText)
{
    int n = 0;
    char ch = 0;
    while ((ch = *pszText))
    {
        CC_BREAK_IF(! ch);
        
        if (0x80 != (0xC0 & ch))
        {
            ++n;
        }
        ++pszText;
    }
    return n;
}

//const static float DELTA = 20.0f;

CCCursorMultiTextFieldTTF::CCCursorMultiTextFieldTTF()
{
    CCTextFieldTTF();
    
    m_pCursorSprite = NULL;
    m_pCursorAction = NULL;
    
//    m_pInputText = NULL;
    isPsw = false;
    m_limitNum = 30000;
    m_pDelegate = this;
}

CCCursorMultiTextFieldTTF::~CCCursorMultiTextFieldTTF()
{
   
}

void CCCursorMultiTextFieldTTF::onEnter()
{
    CCTextFieldTTF::onEnter();
}

//静态生成函数
CCCursorMultiTextFieldTTF * CCCursorMultiTextFieldTTF::textFieldWithPlaceHolder(const char *placeholder, const char *fontName, float fontSize)
{
    CCCursorMultiTextFieldTTF *pRet = new CCCursorMultiTextFieldTTF();
    
    if(pRet && pRet->initWithString("", fontName, fontSize))
    {
        pRet->autorelease();
        if (placeholder)
        {
            pRet->setPlaceHolder(placeholder);
        }
        pRet->initCursorSprite(fontSize);
        
        return pRet;
    }
    
    CC_SAFE_DELETE(pRet);
    
    return NULL;
}

void CCCursorMultiTextFieldTTF::initCursorSprite(int nHeight)
{
    // 初始化光标
    int column = 4;
    int pixels[nHeight][column];
    for (int i=0; i<nHeight; ++i) {
        for (int j=0; j<column; ++j) {
             pixels[i][j] = 0xffffffff;
        }
    }

    CCTexture2D *texture = new CCTexture2D();
    texture->initWithData(pixels, kCCTexture2DPixelFormat_RGB888, 1, 1, CCSizeMake(column, nHeight));
    
    m_pCursorSprite = CCSprite::createWithTexture(texture);
    CCSize winSize = getContentSize();
    CCLOG(" winSizewinSizewinSize is %f,%f",winSize.width,winSize.height);
    m_cursorPos = CCPoint(0, 0);
    m_pCursorSprite->setPosition(m_cursorPos);
    this->addChild(m_pCursorSprite);
    
    m_pCursorAction = CCRepeatForever::create((CCActionInterval *) CCSequence::create(CCFadeOut::create(0.25f), CCFadeIn::create(0.25f), NULL));
    
    m_pCursorSprite->runAction(m_pCursorAction);
    
    m_pCursorSprite->setVisible(false);  //init is false
//    m_pInputText = new std::string();
}

//bool CCCursorMultiTextFieldTTF::ccTouchBegan(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent)
//{    
////    m_beginPos = pTouch->getLocationInView();
////    m_beginPos = CCDirector::sharedDirector()->convertToGL(m_beginPos);
//    
//    return true;
//}

CCRect CCCursorMultiTextFieldTTF::getRect()
{
    CCSize size;
    if (&m_designedSize != NULL) {
         size = m_designedSize;
    }else {
        size = getContentSize();
    }
   
    CCRect rect = CCRectMake(0 - size.width * getAnchorPoint().x, 0 - size.height * getAnchorPoint().y, size.width, size.height);
    CCLOG("pToushPospToushPos %f,%f,%f,%f",0 - size.width * getAnchorPoint().x,0 - size.height * getAnchorPoint().y,size.width,size.height);
    return  rect;
}
//设置触摸弹出输入法的区域大小
void CCCursorMultiTextFieldTTF::setDesignedSize(cocos2d::CCSize size)
{
    m_designedSize = size;
}

CCSize CCCursorMultiTextFieldTTF::getDesignedSize()
{
    return m_designedSize;
}

bool CCCursorMultiTextFieldTTF::isInTextField(cocos2d::CCTouch *pTouch)
{   
    CCPoint pToushPos = convertTouchToNodeSpaceAR(pTouch);
    CCLOG("pToushPospToushPos %f,%f",pToushPos.x,pToushPos.y);
    bool inPoint = getRect().containsPoint(pToushPos);
    CCLOG("containsPointcontainsPoint is %s",inPoint);
    return true;
    //return CCRect::CCRectContainsPoint(getRect(), pToushPos);
}

//void CCCursorMultiTextFieldTTF::ccTouchEnded(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent)
//{
////    CCPoint endPos = pTouch->getLocationInView();
////    endPos = CCDirector::sharedDirector()->convertToGL(endPos);
////    
////    // 判断是否为点击事件
////    if (::abs(endPos.x - m_beginPos.x) > DELTA || 
////        ::abs(endPos.y - m_beginPos.y) > DELTA) 
////    {
////        // 不是点击事件
////        m_beginPos.x = m_beginPos.y = -1;
////        
////        return;
////    }
////    
////    CCLOG("width: %f, height: %f.", getContentSize().width, getContentSize().height);
////    
////    // 判断是打开输入法还是关闭输入法
////    isInTextField(pTouch) ? openIME() : closeIME();
//}

bool CCCursorMultiTextFieldTTF::onTextFieldAttachWithIME(cocos2d::CCTextFieldTTF *pSender)
{
    updateCursorPosition();

    if (m_pInputText->empty()) {
        return false;
    }
    
//    m_pCursorSprite->setPositionX(getContentSize().width);
    
    return false;
}
void CCCursorMultiTextFieldTTF::keyboardWillHide(CCIMEKeyboardNotificationInfo& info)
{
     CCLOG("CCCursorMultiTextFieldTTF::keyboardWillHide");
    m_pCursorSprite->setVisible(false);

}

void CCCursorMultiTextFieldTTF::keyboardDidHide(CCIMEKeyboardNotificationInfo& info)
{
    CCLOG("CCCursorMultiTextFieldTTF::keyboardDidHide");

}
bool CCCursorMultiTextFieldTTF::onTextFieldInsertText(cocos2d::CCTextFieldTTF *pSender, const char *text, int nLen)
{
    CCLOG("Width: %f", pSender->getContentSize().width);
    CCLOG("Text: %s", text);
    CCLOG("Length: %d", nLen);
    std::string tempStr = m_pInputText->substr();
    tempStr.append(text);
    if (tempStr.length() > m_limitNum) {
        return true;
    }
//    CCLOG("m_pInputTextm_pInputTextm_pInputText %s",m_pInputText->c_str());
//    
    m_pInputText->append(text);
//   textWidthFormat(text);
    CCLOG("m_pInputTextm_p_pInputText %s",m_pInputText->c_str());
    
    std::string sText(*m_pInputText);
//  sText.append(sInsert);
    setString(sText.c_str());
    
//    m_pCursorSprite->setPositionX(getContentSize().width);
    updateCursorPosition();
    return true;
}

std::string CCCursorMultiTextFieldTTF::textWidthFormat(const char *text)
{
    std::string sourceStr = m_pInputText->substr();
    std::string targetStr = "";
    std::string tempStr = "";
    
    while(sourceStr.length())
    {
        int nLastLen = sourceStr.length();
        int nDeleteLen = 1;
        while((nDeleteLen < nLastLen) && (0x80 == (0xC0 & sourceStr.at(nDeleteLen))))
        {
            ++nDeleteLen;
        }
        std::string word = sourceStr.substr(0,nDeleteLen);
        sourceStr.erase(0,nDeleteLen);
        
        tempStr.append(word);
        float stringWith = CCImage::getStringWithByFontAndSize(tempStr.c_str(),getFontName(),getFontSize());
        if(stringWith > getDimensions().width)
        {
            targetStr.append("\n");
        }
        targetStr.append(word);
        tempStr = targetStr;
    }
    return targetStr;
}
void CCCursorMultiTextFieldTTF::updateCursorPosition()
{
    CCSize textSize = getTexture()->getContentSizeInPixels();
    CCLOG("CCCursorMultiTextFieldTTF textSize %f,%f ",textSize.width, textSize.height);
    float height = 0;
    float width = 0;
    float merHeight = CCImage::getMerHeightByFontAndName(getFontName(),getFontSize());
    std::string inputText = textWidthFormat(m_pInputText->c_str());
    CCImage::getLastWordPositionX(inputText.c_str(),getFontName(),getFontSize(),getDimensions().width,getDimensions().height,&height,&width);
    CCLOG("CCCursorMultiTextFieldTTF height %f,width %f ,%s",height, width,m_pInputText->c_str());
    int row = ceil(height/merHeight);
    height = merHeight * row;
    m_cursorPos = CCPoint(width,getDimensions().height - abs(height - merHeight/2));
    m_pCursorSprite->setPosition(m_cursorPos);
//    return textSize;
//
}

bool CCCursorMultiTextFieldTTF::onTextFieldDeleteBackward(cocos2d::CCTextFieldTTF *pSender, const char *delText, int nLen)
{
    m_pInputText->resize(m_pInputText->size() - nLen);
    //CCLog(m_pInputText->c_str());

    std::string sText(*m_pInputText);
    setString(sText.c_str());
    
//    m_pCursorSprite->setPositionX(getContentSize().width);
    updateCursorPosition();

    if (m_pInputText->empty()) {
//        m_pCursorSprite->setPositionX(0);
    }
    
    return true;
}

bool CCCursorMultiTextFieldTTF::onTextFieldDetachWithIME(cocos2d::CCTextFieldTTF *pSender)
{
    return false;
}

void CCCursorMultiTextFieldTTF::openIME()
{
    m_pCursorSprite->setVisible(true);
    this->attachWithIME();
}

void CCCursorMultiTextFieldTTF::closeIME()
{
    m_pCursorSprite->setVisible(false);
    this->detachWithIME();
}

void CCCursorMultiTextFieldTTF::onExit()
{
    this->detachWithIME();
    CCTextFieldTTF::onExit();
}

bool CCCursorMultiTextFieldTTF::getIsPsw()
{
    return isPsw;
}
//设置星号显示否
void CCCursorMultiTextFieldTTF::setIsPsw( bool bFlag)
{
    isPsw = bFlag;
}

int CCCursorMultiTextFieldTTF::getLimitNum()
{
    return m_limitNum;
}
//设置字符长度
void CCCursorMultiTextFieldTTF::setLimitNum(int limitNum)
{
    m_limitNum = limitNum;
}

void CCCursorMultiTextFieldTTF::setString(const char *inputTx)
{
    CC_SAFE_DELETE(m_pInputText);
    
    if (inputTx)
    {
        m_pInputText = new std::string(inputTx);
    }
    else
    {
        m_pInputText = new std::string;
    }
    
    // if there is no input text, display placeholder instead
    if (! m_pInputText->length())
    {
        CCLabelTTF::setString(m_pPlaceHolder->c_str());
    }
    else
    {
        CCLabelTTF::setString(inputTx);
    }
    m_nCharCount = _calcCharCount(m_pInputText->c_str());
}

void CCCursorMultiTextFieldTTF::setColor(const ccColor3B& color3)
{
//    m_sColor = m_sColorUnmodified = color3;
//    
//    if (m_bOpacityModifyRGB)
//    {
//        m_sColor.r = color3.r * m_nOpacity/255.0f;
//        m_sColor.g = color3.g * m_nOpacity/255.0f;
//        m_sColor.b = color3.b * m_nOpacity/255.0f;
//    }
//    
//    updateColor();
//    m_pCursorSprite->setColor(color3);
}

NS_CC_END