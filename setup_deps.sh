#!/usr/bin/env bash
# setup_deps.sh - 自动克隆 IVX 所需的三个第三方仓库到 deps/

set -u

# 获取脚本所在目录（项目根目录）
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEPS="$ROOT/deps"

echo "========================================"
echo " IVX Dependency Setup"
echo "========================================"
echo

# 创建 deps 目录
if [ ! -d "$DEPS" ]; then
    echo "Creating deps directory..."
    mkdir -p "$DEPS"
fi

cd "$DEPS" || { echo "[ERROR] Cannot enter $DEPS"; exit 1; }

# 克隆函数：$1=目录名 $2=仓库URL $3=分支（可选）
clone_if_missing() {
    local name="$1"
    local url="$2"
    local branch="${3:-}"

    if [ -d "$name" ]; then
        echo "[SKIP] $name already exists"
        return 0
    fi

    echo "[CLONE] $name ..."
    if [ -n "$branch" ]; then
        git clone -b "$branch" "$url" "$name"
    else
        git clone "$url" "$name"
    fi

    if [ $? -ne 0 ]; then
        echo "[ERROR] Failed to clone $name"
        return 1
    fi
}

# 1. GLFW
echo
echo "[1/3] Cloning GLFW..."
clone_if_missing glfw "https://github.com/mallocobject/glfw.git" master || exit 1

# 2. GLM
echo
echo "[2/3] Cloning GLM..."
clone_if_missing glm "https://github.com/icaven/glm.git" master || exit 1

# 3. ImGui (docking 分支)
echo
echo "[3/3] Cloning ImGui (docking branch)..."
clone_if_missing imgui "https://github.com/mallocobject/imgui.git" docking || exit 1

echo
echo "========================================"
echo " All dependencies are ready in:"
echo " $DEPS"
echo "========================================"