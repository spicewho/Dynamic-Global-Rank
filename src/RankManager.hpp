#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class RankManager {
public:
    static RankManager& get();

    void load();
    void requestRankUpdate();
    void updateRankFromScore(GJUserScore* score);

    void markLevelCompleted();
    void onLevelInfoOpened();

    bool shouldSkipBackgroundRefresh() const;
    int getCurrentRank() const;

private:
    int m_currentRank = -1;

    bool m_pendingLevelComplete = false;
    bool m_skipBackgroundRefresh = false;

    void updateRank(int newRank);
};