#include "RankManager.hpp"
#include "RankPopup.hpp"

#include <Geode/modify/GameLevelManager.hpp>

using namespace geode::prelude;


RankManager& RankManager::get() {
    static RankManager instance;
    return instance;
}

bool RankManager::shouldSkipBackgroundRefresh() const {
    return m_skipBackgroundRefresh;
}

void RankManager::load() {
    m_currentRank = Mod::get()->getSavedValue<int>("last-rank",-1);
    log::info("Loaded saved rank: {}", m_currentRank);
}

void RankManager::requestRankUpdate() {
    auto glm = GameLevelManager::sharedState();
    glm->updateUserScore();
    auto key = fmt::format("lb_{}_{}", (int)LeaderboardType::Global, (int)LeaderboardStat::Stars);
    glm->m_storedLevels->removeObjectForKey(key.c_str());
    glm->getLeaderboardScores(LeaderboardType::Global, LeaderboardStat::Stars);
}


int RankManager::getCurrentRank() const {
    return m_currentRank;
}

void RankManager::updateRank(int newRank) {
    if (m_currentRank == -1) {
        log::info("Found rank: {}", newRank);

        m_currentRank = newRank;
        Mod::get()->setSavedValue("last-rank", newRank);
        return;
    }
    else if (m_currentRank != newRank) {
        int difference = newRank - m_currentRank;
        if (difference < 0) {
            log::info("Rank improved by {} places", -difference);
        }
        else {
            log::info("Rank dropped by {} places", difference);
        }
        RankPopup::get()->showRankChange(m_currentRank, newRank);
    }

    m_currentRank = newRank;
    Mod::get()->setSavedValue("last-rank", m_currentRank);
}

void RankManager::updateRankFromScore(GJUserScore* score) {
    int newRank = score->m_playerRank;
    if (newRank <= 0)
        return;

    updateRank(newRank);
    if (m_pendingLevelComplete) {
        m_pendingLevelComplete = false;
        m_skipBackgroundRefresh = false;
    }
}

void RankManager::markLevelCompleted() {
    m_pendingLevelComplete = true;
    m_skipBackgroundRefresh = true;

    log::info("Level completion pending rank refresh");
}

void RankManager::onLevelInfoOpened() {
    if (!m_pendingLevelComplete)
        return;
    if (!Mod::get()->getSettingValue<bool>("show-after-level-complete"))
        return;
    m_pendingLevelComplete = false;

    log::info("Refreshing rank after level completion");
    requestRankUpdate();
}
