#include <cstdlib>
#include <string>
#include"SeeTheWorld.h"
#include"tts_ws_client.h"

void SeeTheWorld::speak(const std::string& text) {
    // 指定使用 card 1, device 0（耳机输出）
    //std::string cmd = "espeak-ng -v cmn-latn-pinyin -s 200 -p 60 \"" + text + "\" --stdout | aplay -D plughw:1,0";
    tts_speak(text);
    std::string cmd = "aplay -D hw:1,0 -f S16_LE -r 16000 -c 1 demo.pcm";
//    std::string cmd = "aplay -D hw:0,0 -f S16_LE -r 16000 -c 1 demo.pcm";

    system(cmd.c_str());
}
