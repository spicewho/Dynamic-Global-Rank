#include "RankPopup.hpp"

using namespace geode::prelude;


RankPopup* RankPopup::get() {
    static RankPopup* instance = nullptr;
    if (!instance) {
        auto ret = new RankPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            instance = ret;
        }
        else {
            CC_SAFE_DELETE(ret);
        }
    }

    return instance;
}


bool RankPopup::init() {
    if (!CCNode::init())
        return false;

    auto winsize = CCDirector::get()->getWinSize();
    this->setID("dynamic-rank-popup");
    this->setAnchorPoint({0.5f, 0.5f});
    this->setContentSize({150.f, 120.f});
    auto poshelp = ccp(winsize.width +80.f, winsize.height / 2);
    this->setPosition(poshelp);
    this->setZOrder(999);

    auto bg = CCScale9Sprite::create("square02b_001.png");
    bg->setContentSize({55.f, 100.f});
    bg->setPosition({40.f, 60.f});    
    m_background = bg;
    this->addChild(bg);
    updateColor();

    m_title = CCLabelBMFont::create("Rank", "bigFont.fnt");
    m_title->setScale(0.45f);
    m_title->setPosition({40.f, 100.f});
    m_title->setAnchorPoint({0.5f, 0.5f});
    this->addChild(m_title);

    float scales[5] = {
        0.20f,
        0.275f,
        0.3f, 
        0.275f, 
        0.20f
    };
    for (int i = 0; i < 5; i++) {
        auto label = CCLabelBMFont::create("", "bigFont.fnt");
        label->setScale(scales[i]);
        label->setPosition({40.f, 85.f - (i * 10.f)});
        if (i == 2)
            label->setColor({255,255,255});
        else
            label->setColor({120,120,120});
        label->setAnchorPoint({0.5f, 0.5f});

        this->addChild(label);
        m_rankLabels[i] = label;
    }
    m_change = CCLabelBMFont::create("+15", "bigFont.fnt");
    m_change->setScale(0.5f);
    m_change->setPosition({40.f, 25.f});
    m_change->setColor({0, 255, 120});

    this->scheduleUpdate();
    this->addChild(m_change);
    OverlayManager::get()->addChild(this);
    setVisible(false);
    setDisplayedRank(0);
    return true;
}

void RankPopup::updateColor() {
    if (!m_background)
        return;
    auto color = Mod::get()->getSettingValue<ccColor3B>("popup-background-color");
    auto opacity = Mod::get()->getSettingValue<int>("popup-background-opacity");

    m_background->setColor(color);
    m_background->setOpacity(opacity);
}

bool RankPopup::canShow() {
    auto scene = CCDirector::get()->getRunningScene();
    if (!scene)
        return false;
    for (auto child : CCArrayExt<CCNode*>(scene->getChildren())) {
        if (typeinfo_cast<PlayLayer*>(child))
            return false;
        if (typeinfo_cast<LevelEditorLayer*>(child))
            return false;
    }
    return true;
}

void RankPopup::hidePopup() {
    setVisible(false);
    m_animating = false;
}

void RankPopup::update(float dt) {
    if (!canShow()) {
        if (m_animating) {
            stopAllActions();
            unschedule(schedule_selector(RankPopup::scrollStep));
            m_animating = false;
        }
        setVisible(false);
        return;
    }
    if (!m_animating && m_pendingOldRank != -1) {
        int oldRank = m_pendingOldRank;
        int newRank = m_pendingNewRank;
        m_pendingOldRank = -1;
        m_pendingNewRank = -1;
        showRankChange(oldRank, newRank);
    }
}

void RankPopup::scrollStep(float) {
    m_currentStep++;
    float t = static_cast<float>(m_currentStep) / static_cast<float>(m_totalSteps);
    float eased = 0.5f - std::cos(t * 3.14159265f) * 0.5f;
    int shownRank = m_oldRank + static_cast<int>((m_targetRank - m_oldRank) * eased);
    setDisplayedRank(shownRank);
    auto winsize = CCDirector::get()->getWinSize();
    if (m_currentStep >= m_totalSteps) {
        unschedule(schedule_selector(RankPopup::scrollStep));
        setDisplayedRank(m_targetRank);
        runAction(
            CCSequence::create(
                CCDelayTime::create(1.80f),
                CCEaseSineIn::create(
                    CCMoveTo::create(0.35f, {winsize.width + 50.f, winsize.height / 2 })
                ),
                CCCallFunc::create(this, callfunc_selector(RankPopup::hidePopup)),
                nullptr
            )
        );
    }
}

void RankPopup::beginScrollAnimation() {
    schedule(schedule_selector(RankPopup::scrollStep), 0.06f);
}


void RankPopup::showRankChange(int oldRank, int newRank) {
    if (!canShow()) {
        m_pendingOldRank = oldRank;
        m_pendingNewRank = newRank;
        return;
    }
    if (m_animating)
        return;
    m_animating = true;
    m_oldRank = oldRank;
    m_targetRank = newRank;
    m_currentStep = 0;
    int distance = abs(newRank - oldRank);
    m_totalSteps = std::min(distance, 15);
    if (m_totalSteps == 0)
        m_totalSteps = 1;

    setDisplayedRank(oldRank);
    int difference = newRank - oldRank;
    if (difference < 0) {
        m_change->setString(
            fmt::format("+{}", -difference).c_str()
        );
        m_change->setColor({0,255,120});
    }
    else if (difference > 0) {
        m_change->setString(
            fmt::format("-{}", difference).c_str()
        );
        m_change->setColor({255,80,80});
    }
    else {
        m_change->setString("+0");
        m_change->setColor({255,255,255});
    }

    stopAllActions();
    setVisible(true);
    auto winsize = CCDirector::get()->getWinSize();
    auto y = winsize.height * 0.50f;
    setPosition({winsize.width + 80.f, y});
    runAction(
        CCSequence::create(
            CCEaseSineOut::create(
                CCMoveTo::create(0.35f, {winsize.width, y})
            ),
            CCDelayTime::create(0.40f),
            CCCallFunc::create(this, callfunc_selector(RankPopup::beginScrollAnimation)),
            nullptr
        )
    );
}

void RankPopup::setDisplayedRank(int centerRank) {
    m_displayedRank = centerRank;
    int displayedRanks[5] = {
        centerRank - 2,
        centerRank - 1,
        centerRank,
        centerRank + 1,
        centerRank + 2
    };
    for (int i = 0; i < 5; i++) {
        std::string text;
        if (i == 2)
            text = fmt::format("#{}", displayedRanks[i]);
        else
            text = fmt::format("{}", displayedRanks[i]);
        m_rankLabels[i]->setString(text.c_str());
    }
}