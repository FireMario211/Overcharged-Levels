#include "utils.hpp"

bool doTogglePracticeMode = false;
void Utils::setPractice(bool value) {
    doTogglePracticeMode = value;
}
bool Utils::isPractice() {
    return doTogglePracticeMode;
}
/*
$execute {
    listenForSettingChanges("enabled", +[](bool value) {
        enableMod = value;
    });
}
*/
