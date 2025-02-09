#ifndef JETSTREAM_MEMORY_VULKAN_BUFFER_HH
#define JETSTREAM_MEMORY_VULKAN_BUFFER_HH

#include <memory>

#include "jetstream/memory/devices/base/buffer.hh"

namespace Jetstream {

template<>
class TensorBuffer<Device::Vulkan> {
 public:
    explicit TensorBuffer(std::shared_ptr<TensorStorageMetadata>& storage,
                          const TensorPrototypeMetadata& prototype,
                          const bool& host_accessible = false,
                          const VkBufferUsageFlags& usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                            VK_BUFFER_USAGE_TRANSFER_DST_BIT);

#ifdef JETSTREAM_BACKEND_CPU_AVAILABLE
    explicit TensorBuffer(std::shared_ptr<TensorStorageMetadata>& storage,
                          const TensorPrototypeMetadata& prototype,
                          const std::shared_ptr<TensorBuffer<Device::CPU>>& root_buffer);
#endif

#ifdef JETSTREAM_BACKEND_METAL_AVAILABLE
    explicit TensorBuffer(std::shared_ptr<TensorStorageMetadata>& storage,
                          const TensorPrototypeMetadata& prototype,
                          const std::shared_ptr<TensorBuffer<Device::Metal>>& root_buffer);
#endif

#ifdef JETSTREAM_BACKEND_CUDA_AVAILABLE
    explicit TensorBuffer(std::shared_ptr<TensorStorageMetadata>& storage,
                          const TensorPrototypeMetadata& prototype,
                          const std::shared_ptr<TensorBuffer<Device::CUDA>>& root_buffer);
#endif

    ~TensorBuffer();

    TensorBuffer(const TensorBuffer&) = delete;
    TensorBuffer& operator=(const TensorBuffer&) = delete;

    constexpr const bool& host_accessible() const {
        return _host_accessible;
    }

    constexpr const VkDeviceMemory& memory() const noexcept {
        return _memory;
    }

    constexpr VkDeviceMemory& memory() noexcept {
        return _memory;
    }

    constexpr const VkBuffer& data() const noexcept {
        return _buffer;
    }

    constexpr VkBuffer& data() noexcept {
        return _buffer;
    }

 private:
    VkBuffer _buffer;
    VkDeviceMemory _memory;
    bool owns_data = false;
    bool _host_accessible = false;
};

}  // namespace Jetstream

#endif
