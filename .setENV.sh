#!/bin/bash
# Copy this file to setENV.sh, fill in your own credentials, then run:
#   source ./setENV.sh

APPID="your_xunfei_appid"
API_KEY="your_xunfei_api_key"
API_SECRET="your_xunfei_api_secret"
ARK_API_KEY="your_volcengine_ark_api_key"

export APPID
export API_KEY
export API_SECRET
export ARK_API_KEY

echo "Environment variables loaded:"
echo "APPID is set: $([[ -n "$APPID" ]] && echo yes || echo no)"
echo "API_KEY is set: $([[ -n "$API_KEY" ]] && echo yes || echo no)"
echo "API_SECRET is set: $([[ -n "$API_SECRET" ]] && echo yes || echo no)"
echo "ARK_API_KEY is set: $([[ -n "$ARK_API_KEY" ]] && echo yes || echo no)"
