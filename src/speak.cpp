#include <cstdlib>
#include <string>

int main() {
    std::string text = "前方是一只黑色的猫";

    // 指定使用 card 1, device 0（耳机输出）
    std::string cmd = "espeak-ng -v cmn-latn-pinyin -s 150 \"" + text + "\" --stdout | aplay -D plughw:1,0";

    system(cmd.c_str());
    return 0;
}

