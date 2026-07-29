#include "RankRefreshScheduler.hpp"
#include "RankManager.hpp"
#include <algorithm>

using namespace geode::prelude;

RankRefreshScheduler* RankRefreshScheduler::get() {
    static RankRefreshScheduler* instance = nullptr;
    if (!instance) {
        auto ret = new RankRefreshScheduler();
        if (ret && ret->init()) {
            ret->autorelease();
            OverlayManager::get()->addChild(ret);
            instance = ret;
        }
        else {
            CC_SAFE_DELETE(ret);
        }
    }
    return instance;
}


bool RankRefreshScheduler::init() {
    if (!CCNode::init())
        return false;
    m_interval = static_cast<float>(std::clamp(Mod::get()->getSettingValue<int>("refresh-seconds"), 120, 600)
        );
    this->scheduleUpdate();
    return true;
}

void RankRefreshScheduler::setInterval(float interval) {
    m_interval = interval;
}


void RankRefreshScheduler::update(float dt) {
    m_timer += dt;
    if (m_timer < m_interval)
        return;
    m_timer = 0.f;

    if (!Mod::get()->getSettingValue<bool>("background-refresh"))
        return;
    if (RankManager::get().shouldSkipBackgroundRefresh())
        return;

    log::info("Background rank refresh");

    auto accountID = GJAccountManager::sharedState()->m_accountID;
    if (accountID == 0) {
        log::warn("Background rank refresh: User not logged in.");
        return;
    }
    RankManager::get().requestRankUpdate();
}