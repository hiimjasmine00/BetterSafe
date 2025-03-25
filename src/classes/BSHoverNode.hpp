#include "../BetterSafe.hpp"
#include <cocos2d.h>

using HoverCallback = std::function<void()>;

class BSHoverNode : public cocos2d::CCLayer {
protected:
    HoverCallback m_callback;

    bool init(const SafeLevel&, GJGameLevel*, HoverCallback);
public:
    static BSHoverNode* create(const SafeLevel&, GJGameLevel*, HoverCallback);

    void close();
    void keyBackClicked() override;
    void registerWithTouchDispatcher() override;
    bool ccTouchBegan(cocos2d::CCTouch*, cocos2d::CCEvent*) override;

    ~BSHoverNode() override;
};
