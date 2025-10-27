#include <cstdlib>
#include <string>
#include"SeeTheWorld.h"

void SeeTheWorld::speak(const std::string& text) {
    // 指定使用 card 1, device 0（耳机输出）
    std::string cmd = "espeak-ng -v cmn-latn-pinyin -s 200 -p 60 \"" + text + "\" --stdout | aplay -D plughw:1,0";

    system(cmd.c_str());
}
