#!/bin/bash
# set_ark_env.sh
# 一键设置 Xunfei TTS 凭证环境变量

# ----- 请在下面填写你的凭证 -----

APPID="your appid"
API_KEY="your api_key"
API_SECRET="your api_secret"
ARK_API_KEY="your ark_api_key"

BASHRC="$HOME/.bashrc"

# 检查 ~/.bashrc 是否已存在这些环境变量
grep -q "export APPID=" "$BASHRC" && echo "APPID 已存在，覆盖..." || echo "添加 APPID"
grep -q "export API_KEY=" "$BASHRC" && echo "API_KEY 已存在，覆盖..." || echo "添加 API_KEY"
grep -q "export API_SECRET=" "$BASHRC" && echo "API_SECRET 已存在，覆盖..." || echo "添加 API_SECRET"
grep -q "export ARK_API_KEY=" "$BASHRC" && echo "ARK_API_KEY 已存在，覆盖..." || echo "添加 ARK_API_KEY"

# 用 sed 覆盖已有值或追加新行
sed -i "/export APPID=/d" "$BASHRC"
sed -i "/export API_KEY=/d" "$BASHRC"
sed -i "/export API_SECRET=/d" "$BASHRC"
sed -i "/export ARK_API_KEY=/d" "$BASHRC"

echo "export APPID=\"$APPID\"" >> "$BASHRC"
echo "export API_KEY=\"$API_KEY\"" >> "$BASHRC"
echo "export API_SECRET=\"$API_SECRET\"" >> "$BASHRC"
echo "export ARK_API_KEY=\"$ARK_API_KEY\"" >> "$BASHRC"

# 立即生效
source "$BASHRC"

echo "✅ 已成功设置环境变量并生效："
echo "APPID=$APPID"
echo "API_KEY=$API_KEY"
echo "API_SECRET=$API_SECRET"
echo "ARK_API_KEY=$API_SECRET"
