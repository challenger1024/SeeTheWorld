import os
import openai

# 设置 API Key
openai.api_key = os.environ.get("ARK_API_KEY")
# 如果有自定义 base_url，需要设置 openai.api_base
openai.api_base = "https://ark.cn-beijing.volces.com/api/v3"

completion = openai.ChatCompletion.create(
    model="doubao-seed-1-6-lite-251015",
    messages=[
        {"role": "user", "content": "hello"}
    ]
)

# 输出模型回复
print(completion["choices"][0]["message"]["content"])
