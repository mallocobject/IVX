#!/usr/bin/env bash
# setup_deps.sh

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEPS="$ROOT/deps"

echo "========================================"
echo " IVX Dependency Setup"
echo "========================================"
echo

if [ ! -d "$DEPS" ]; then
    echo "Creating deps directory..."
    mkdir -p "$DEPS"
fi

cd "$DEPS" || { echo "[ERROR] Cannot enter $DEPS"; exit 1; }

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

# 1. GLM
echo
echo "[1/4] Cloning GLM..."
clone_if_missing glm "https://github.com/icaven/glm.git" master || exit 1

# 2. ImGui (docking branch)
echo
echo "[2/4] Cloning ImGui (docking branch)..."
clone_if_missing imgui "https://github.com/mallocobject/imgui.git" docking || exit 1

# 3. BqLog
echo
echo "[3/4] Cloning BqLog..."
clone_if_missing BqLog "https://github.com/mallocobject/BqLog.git" || exit 1

# 4. SDL3: the only windowing/input back end
echo
echo "[4/4] Cloning SDL3..."
clone_if_missing SDL "https://github.com/libsdl-org/SDL.git" release-3.4.0 || exit 1

echo
echo "========================================"
echo " All dependencies are ready in:"
echo " $DEPS"
echo "========================================"
