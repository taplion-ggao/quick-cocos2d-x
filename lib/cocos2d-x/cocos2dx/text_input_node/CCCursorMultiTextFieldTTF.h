/****************************************************************************
Copyright (c) 2010 cocos2d-x.org

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

#ifndef __CC_CMTEXT_FIELD_H__
#define __CC_CMTEXT_FIELD_H__

#include "text_input_node/CCTextFieldTTF.h"
#include "touch_dispatcher/CCTouchDelegateProtocol.h"

NS_CC_BEGIN


class CC_DLL CCCursorMultiTextFieldTTF : public CCTextFieldTTF , public CCTextFieldDelegate
{
private:
    // 点击开始位置
    CCPoint m_beginPos;
    
    // 光标精灵
    CCSprite *m_pCursorSprite;
    
    // 光标动画
    CCAction *m_pCursorAction;
                 
    // 光标坐标
    CCPoint m_cursorPos;
    
    // 是否加密显示
    bool isPsw;
    int m_limitNum;
    // 输入框内容
    CCSize m_designedSize;
    
    void updateCursorPosition();
    std::string textWidthFormat(const char *text);
    
public:
    CCCursorMultiTextFieldTTF();
    ~CCCursorMultiTextFieldTTF();
    
    // static，暂时不能使用
//    static DTCursorTextField * textFieldWithPlaceHolder(const char *placeholder, const CCSize& dimensions, CCTextAlignment alignment, const char *fontName, float fontSize);
    
    /** creates a CCLabelTTF from a fontname and font size */
    static CCCursorMultiTextFieldTTF * textFieldWithPlaceHolder(const char *placeholder, const char *fontName, float fontSize);
    
    // CCLayer
    void onEnter();
    void onExit();
    
    // 初始化光标精灵
    void initCursorSprite(int nHeight);
    void setColor(const ccColor3B& color3);
    // CCTextFieldDelegate
    virtual bool onTextFieldAttachWithIME(CCTextFieldTTF *pSender);
    virtual bool onTextFieldDetachWithIME(CCTextFieldTTF * pSender);
    virtual bool onTextFieldInsertText(CCTextFieldTTF * pSender, const char * text, int nLen);
    virtual bool onTextFieldDeleteBackward(CCTextFieldTTF * pSender, const char * delText, int nLen);
    virtual void keyboardWillHide(CCIMEKeyboardNotificationInfo& info);
    virtual void keyboardDidHide(CCIMEKeyboardNotificationInfo& info);

    // CCLayer Touch
//    bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent);
//    void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent);
    
    // 判断是否点击在TextField处
    bool isInTextField(CCTouch *pTouch);
    // 得到TextField矩形
    CCRect getRect();
    
    // 打开输入法
    void openIME();
    // 关闭输入法
    void closeIME();
    
    //设置是否星号显示
    bool getIsPsw();
    void setIsPsw(bool bFlag);
    //设置字符长度限制，一个汉字三个字符
    void setLimitNum(int limitNum);
    int getLimitNum();
    //重载原函数，用来显示星号
    void setString(const char* inputTx);
    //点击弹出输入法的尺寸范围
    void setDesignedSize(CCSize size);
    CCSize getDesignedSize();
};

// end of input group
/// @}

NS_CC_END

#endif    // __CC_TEXT_FIELD_H__
