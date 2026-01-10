#pragma once
#include <cstdint>

namespace AssetManager {
    template <typename Tag> struct Handle {
        explicit Handle(const int64_t id) : id(id) {}
        Handle() : id(-1) {}

        int64_t id; // uint64 and flag for validness

        bool operator==(const Handle &rhs) const {
            return id == rhs.id;
        }
        bool operator!=(const Handle &rhs) const {
            return id != rhs.id;
        }
        bool operator!() const {
            return id == -1;
        }
        explicit operator bool() const {
            return id != -1;
        }
        explicit operator int64_t() const {
            return id;
        }
    };

    using ShaderHandle = Handle<struct Shader>;
} // namespace AssetManager
