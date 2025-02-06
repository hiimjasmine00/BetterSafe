#include "../BetterSafe.hpp"
#include <cocos2d.h>
#include <cocos-ext.h>

class BSHoverNode : public cocos2d::CCLayer {
protected:
    cocos2d::extension::CCScale9Sprite* m_background;
    std::function<void()> m_callback;

    bool init(const SafeLevel&, GJGameLevel*, const std::function<void()>&);
public:
    static BSHoverNode* create(const SafeLevel&, GJGameLevel*, const std::function<void()>&);

    void close();
    void keyBackClicked() override;
    void registerWithTouchDispatcher() override;
    bool ccTouchBegan(cocos2d::CCTouch*, cocos2d::CCEvent*) override;

    ~BSHoverNode() override;
};
