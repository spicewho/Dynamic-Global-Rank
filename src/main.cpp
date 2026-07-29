#include <Geode/Geode.hpp>
#include <Geode/modify/GameLevelManager.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/loader/SettingV3.hpp>

#include "RankManager.hpp"
#include "RankPopup.hpp"
#include "RankRefreshScheduler.hpp"

using namespace geode::prelude; 

class $modify(MyGameLevelManager, GameLevelManager) {
    void onGetLeaderboardScoresCompleted(gd::string response, gd::string tag) {
        GameLevelManager::onGetLeaderboardScoresCompleted(response, tag);
        if (tag != "lb_2_0")
            return;

        auto glm = GameLevelManager::sharedState();
        auto accountID = GJAccountManager::sharedState()->m_accountID;
        auto scores = typeinfo_cast<CCArray*>(glm->m_storedLevels->objectForKey(tag.c_str()));

        if (!scores)
            return;
        for (auto score : CCArrayExt<GJUserScore*>(scores)) {
            if (score->m_accountID == accountID) {
                RankManager::get().updateRankFromScore(score);
                break;
            }
        }
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    void levelComplete() {
        auto gsm = GameStatsManager::sharedState();
        int oldStars = gsm->getStat("6");
        PlayLayer::levelComplete();
        int newStars = gsm->getStat("6");

        if (newStars > oldStars) {
            log::info(
                "Earned {} stars",
                newStars - oldStars
            );
            RankManager::get().markLevelCompleted();
        }
    }
};

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel * level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge))
            return false;
        RankManager::get().onLevelInfoOpened();
        return true;
    }
};


$on_mod(Loaded) {
    log::info("Dynamic Global Rank loaded");

    RankManager::get().load();
    RankPopup::get();
    RankRefreshScheduler::get();
}

$execute{
    listenForSettingChanges<int>("refresh-seconds", [](int value) {
        RankRefreshScheduler::get()->setInterval(value);
    });

    listenForSettingChanges<ccColor3B>("popup-background-color", [](ccColor3B) {
            if (RankPopup::get())
                RankPopup::get()->updateColor();
        }
    );

    listenForSettingChanges<int>("popup-background-opacity", [](int) {
        if (RankPopup::get())
            RankPopup::get()->updateColor();
        }
    );
}