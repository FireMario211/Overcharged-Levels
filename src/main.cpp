#include "utils.hpp"

bool s_doTogglePracticeMode = false;
void Utils::setPractice(bool value) {
    s_doTogglePracticeMode = value;
}
bool Utils::isPractice() {
    return s_doTogglePracticeMode;
}
