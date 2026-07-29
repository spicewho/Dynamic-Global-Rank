#pragma once

#include <Geode/Geode.hpp>


class RankPopup : public cocos2d::CCNode {
public:
    static RankPopup* get();

    void showRankChange(int oldRank, int newRank);
    void setDisplayedRank(int centerRank);

    void beginScrollAnimation();
    void scrollStep(float);
    void hidePopup();
    void update(float dt) override;

    void updateColor();

private:
    bool init() override;
    bool canShow();
    bool m_animating = false;

    int m_displayedRank = -1;
    int m_oldRank = 0;
    int m_targetRank = 0;

    int m_currentStep = 0;
    int m_totalSteps = 0;

    int m_pendingOldRank = -1;
    int m_pendingNewRank = -1;

    cocos2d::CCLabelBMFont* m_rankLabels[5] = {};
    cocos2d::CCLabelBMFont* m_title;
    cocos2d::CCLabelBMFont* m_change;

    cocos2d::extension::CCScale9Sprite* m_background;
};