#include "ivxpch.h"
//
#include "layer.h"

namespace ivx {
Layer::Layer(std::string name) : debug_name_(std::move(name)) {
}

Layer::~Layer() {
}
} // namespace ivx