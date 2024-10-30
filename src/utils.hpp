#pragma once

#include <Geode/Geode.hpp>
#include <random>

class Utils {
    public:
    static float calculateScale(const std::string& text, int minLength, int maxLength, float minScale, float maxScale) {
        int length = text.length();
        if (length <= minLength) {
            return minScale;
        } else if (length >= maxLength) {
            return maxScale;
        } else {
            float scale = minScale - ((length - minLength) * (minScale - maxScale) / (maxLength - minLength));
            return scale;
        }
    }
    static float calculateScale(int length, int minLength, int maxLength, float minScale, float maxScale) {
        if (length <= minLength) {
            return minScale;
        } else if (length >= maxLength) {
            return maxScale;
        } else {
            float scale = minScale - ((length - minLength) * (minScale - maxScale) / (maxLength - minLength));
            return scale;
        }
    }
    cocos2d::CCPoint generateRandomPosition(cocos2d::CCPoint center, float range) {
        float xOffset = ((float)rand() / RAND_MAX) * range - (range / 2);
        float yOffset = ((float)rand() / RAND_MAX) * range - (range / 2);
        
        return ccp(center.x + xOffset, center.y + yOffset);
    }
    static int generateRandomNumber(int min, int max) {
        if (min >= max) {
            throw std::invalid_argument("min must be less than max");
        }
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);
        return dist(rng);
    }
    static void setPractice(bool value);
    static bool isPractice();
};
