#include "../BetterSafe.hpp"
#include <Geode/cocos/layers_scenes_transitions_nodes/CCLayer.h>

using HoverCallback = geode::Function<void()>;

class BSHoverNode : public cocos2d::CCLayer {
protected:
    GJGameLevel* m_level;
    HoverCallback m_callback;

    bool init(const SafeLevel&, GJGameLevel*, HoverCallback);
public:
    static BSHoverNode* create(const SafeLevel&, GJGameLevel*, HoverCallback);

    void onClose(cocos2d::CCObject*);
    void onName(cocos2d::CCObject*);
    void onCreator(cocos2d::CCObject*);
    void keyBackClicked() override;
    void registerWithTouchDispatcher() override;
    bool ccTouchBegan(cocos2d::CCTouch*, cocos2d::CCEvent*) override;

    ~BSHoverNode() override;
};
