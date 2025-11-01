from flask import Flask, jsonify, send_from_directory
import subprocess
import os

app = Flask(__name__, static_folder='.')

@app.route('/')
def index():
    # 返回当前目录下的 index.html
    return send_from_directory('.', 'index.html')

@app.route('/capture', methods=['GET'])
def capture_image():
    try:
        # 调用 C++ 程序
        subprocess.run(["./see_the_world"], check=True)
        return jsonify({"status": "success", "message": "拍照成功，已保存 image.jpg"})
    except subprocess.CalledProcessError:
        return jsonify({"status": "error", "message": "拍照失败"})

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
