#pragma once

#include <Geode/Geode.hpp>

class RankRefreshScheduler : public cocos2d::CCNode {
public:
    static RankRefreshScheduler* get();
    void setInterval(float interval);

private:
    bool init() override;
    void update(float dt);
    float m_timer = 0.f;
    float m_interval = 240.f;
};