#include "jetstream/instance.hh"
#include "jetstream/store.hh"

namespace Jetstream {

Result Instance::buildDefaultInterface(const Device& preferredDevice,
                                       const Backend::Config& backendConfig,
                                       const Viewport::Config& viewportConfig,
                                       const Render::Window::Config& renderConfig) {
    JST_DEBUG("[INSTANCE] Building default viewport and render.");

    if (_viewport || _window) {
        JST_ERROR("[INSTANCE] Viewport or render already built.");
        return Result::ERROR;
    }

    std::vector<Device> devicePriority = {
        preferredDevice,
        Device::Metal,
        Device::Vulkan,
        Device::WebGPU,
    };

    if (backendConfig.headless) {
        for (const auto& device : devicePriority) {
            switch (device) {
#ifdef JETSTREAM_VIEWPORT_HEADLESS_AVAILABLE
#if   defined(JETSTREAM_BACKEND_VULKAN_AVAILABLE)
                case Device::Vulkan:
                    JST_CHECK(Backend::Initialize<Device::Vulkan>(backendConfig));
                    JST_CHECK(this->buildViewport<Viewport::Headless<Device::Vulkan>>(viewportConfig));
                    JST_CHECK(this->buildRender<Device::Vulkan>(renderConfig));
                    return Result::SUCCESS;
#endif
#endif
                default:
                    continue;
            }
        }

        JST_ERROR("[INSTANCE] No headless viewport backend available.");
        return Result::ERROR;
    } else {
        for (const auto& device : devicePriority) {
            switch (device) {
#ifdef JETSTREAM_VIEWPORT_GLFW_AVAILABLE
#ifdef JETSTREAM_BACKEND_METAL_AVAILABLE
                case Device::Metal:
                    JST_CHECK(Backend::Initialize<Device::Metal>(backendConfig));
                    JST_CHECK(this->buildViewport<Viewport::GLFW<Device::Metal>>(viewportConfig));
                    JST_CHECK(this->buildRender<Device::Metal>(renderConfig));
                    return Result::SUCCESS;
#endif
#ifdef JETSTREAM_BACKEND_VULKAN_AVAILABLE
                case Device::Vulkan:
                    JST_CHECK(Backend::Initialize<Device::Vulkan>(backendConfig));
                    JST_CHECK(this->buildViewport<Viewport::GLFW<Device::Vulkan>>(viewportConfig));
                    JST_CHECK(this->buildRender<Device::Vulkan>(renderConfig));
                    return Result::SUCCESS;
#endif
#ifdef JETSTREAM_BACKEND_WEBGPU_AVAILABLE
                case Device::WebGPU:
                    JST_CHECK(Backend::Initialize<Device::WebGPU>(backendConfig));
                    JST_CHECK(this->buildViewport<Viewport::GLFW<Device::WebGPU>>(viewportConfig));
                    JST_CHECK(this->buildRender<Device::WebGPU>(renderConfig));
                    return Result::SUCCESS;
#endif
#endif
                default:
                    continue;
            }
        }

        JST_ERROR("[INSTANCE] No viewport backend available.");
        return Result::ERROR;
    }
}

Result Instance::reloadBlock(Locale locale) {
    JST_DEBUG("[INSTANCE] Reloading block '{}'.", locale.block());

    if (!_flowgraph.nodes().contains(locale.block())) {
        JST_ERROR("[INSTANCE] Block '{}' doesn't exist.", locale.block());
        return Result::ERROR;
    }

    JST_CHECK(blockUpdater(locale.block(), [](std::shared_ptr<Flowgraph::Node>&){
        return Result::SUCCESS;
    }));

    return Result::SUCCESS;
}

Result Instance::removeBlock(Locale locale) {
    JST_DEBUG("[INSTANCE] Trying to remove block '{}'.", locale);

    // Verify input parameters.

    if (!_flowgraph.nodes().contains(locale.block())) {
        JST_ERROR("[INSTANCE] Block '{}' doesn't exist.", locale);
        return Result::ERROR;
    }

    // Unlink all modules linked to the module being removed.

    std::vector<std::pair<Locale, Locale>> unlinkList;
    // TODO: This is exponential garbage. Implement cached system.
    for (const auto& [outputPinId, outputRecord] : _flowgraph.nodes().at(locale)->outputMap) {
        for (const auto& [inputLocale, inputState] : _flowgraph.nodes()) {
            for (const auto& [inputPinId, inputRecord] : inputState->inputMap) {
                if (inputRecord.locale == outputRecord.locale) {
                    unlinkList.push_back({{inputLocale.blockId, inputLocale.moduleId, inputPinId}, outputRecord.locale});
                }
            }
        }
    }

    for (const auto& [inputLocale, outputLocale] : unlinkList) {
        JST_CHECK(unlinkBlocks(inputLocale.block(), outputLocale.block()));
    }

    // Delete module.
    JST_CHECK(eraseBlock(locale.block()));

    return Result::SUCCESS;
}

Result Instance::unlinkBlocks(Locale inputLocale, Locale outputLocale) {
    JST_DEBUG("[INSTANCE] Unlinking '{}' -> '{}'.", outputLocale, inputLocale);

    // Verify input parameters.

    if (!_flowgraph.nodes().contains(inputLocale.block())) {
        JST_ERROR("[INSTANCE] Link input block '{}' doesn't exist.", inputLocale);
        return Result::ERROR;
    }

    if (!_flowgraph.nodes().contains(outputLocale.block())) {
        JST_ERROR("[INSTANCE] Link output block '{}' doesn't exist.", outputLocale);
        return Result::ERROR;
    }

    if (inputLocale.pinId.empty() || outputLocale.pinId.empty()) {
        JST_ERROR("[INSTANCE] The pin ID of the input and output locale must not be empty.");
        return Result::ERROR;
    }

    const auto& rawInputLocale = _flowgraph.nodes().at(inputLocale.block())->inputMap.at(inputLocale.pinId).locale;
    if (rawInputLocale.block() != outputLocale.block()) {
        JST_ERROR("[INSTANCE] Link '{}' -> '{}' doesn't exist.", outputLocale, inputLocale);
        return Result::ERROR;
    }

    // Update module.

    JST_CHECK(blockUpdater(inputLocale.block(), [&](std::shared_ptr<Flowgraph::Node>& node) {
        // Delete link from input block inputs.
        node->inputMap.erase(inputLocale.pinId);

        return Result::SUCCESS;
    }));

    return Result::SUCCESS;
}

Result Instance::linkBlocks(Locale inputLocale, Locale outputLocale) {
    JST_DEBUG("[INSTANCE] Linking '{}' -> '{}'.", outputLocale, inputLocale);

    // Verify input parameters.

    if (!_flowgraph.nodes().contains(inputLocale.block())) {
        JST_ERROR("[INSTANCE] Link input block '{}' doesn't exist.", inputLocale);
        return Result::ERROR;
    }

    if (!_flowgraph.nodes().contains(outputLocale.block())) {
        JST_ERROR("[INSTANCE] Link output block '{}' doesn't exist.", outputLocale);
        return Result::ERROR;
    }

    if (inputLocale.pinId.empty() || outputLocale.pinId.empty()) {
        JST_ERROR("[INSTANCE] The pin ID of the input and output locale must not be empty.");
        return Result::ERROR;
    }

    const auto& rawInputLocale = _flowgraph.nodes().at(inputLocale.block())->inputMap.at(inputLocale.pinId).locale;
    if (rawInputLocale.block() == outputLocale.block()) {
        JST_ERROR("[INSTANCE] Link '{}' -> '{}' already exists.", outputLocale, inputLocale);
        return Result::ERROR;
    }

    const auto& inputRecordMap = _flowgraph.nodes().at(inputLocale.block())->inputMap;
    if (inputRecordMap.contains(inputLocale.pinId) && inputRecordMap.at(inputLocale.pinId).hash != 0) {
        JST_ERROR("[INSTANCE] Input '{}' is already linked with something else.", inputLocale);
        return Result::ERROR;
    }

    // Update module.

    JST_CHECK(blockUpdater(inputLocale.block(), [&](std::shared_ptr<Flowgraph::Node>& node) {
        // Add link between input and output blocks.
        const auto& outputMap = _flowgraph.nodes().at(outputLocale.block())->outputMap;
        node->inputMap[inputLocale.pinId] = outputMap.at(outputLocale.pinId);

        return Result::SUCCESS;
    }));

    return Result::SUCCESS;
}

Result Instance::changeBlockBackend(Locale input, Device device) {
    JST_DEBUG("[INSTANCE] Changing module '{}' backend to '{}'.", input, device);

    // Verify input parameters.

    if (!_flowgraph.nodes().contains(input.block())) {
        JST_ERROR("[INSTANCE] Module '{}' doesn't exist.", input);
        return Result::ERROR;
    }

    // Update module.

    JST_CHECK(blockUpdater(input.block(), [&](std::shared_ptr<Flowgraph::Node>& node) {
        // Change backend.
        node->fingerprint.device = GetDeviceName(device);

        return Result::SUCCESS;
    }));

    return Result::SUCCESS;
}

Result Instance::changeBlockDataType(Locale input, std::tuple<std::string, std::string> type) {
    JST_DEBUG("[INSTANCE] Changing module '{}' data type to '{}'.", input, type);

    // Verify input parameters.

    if (!_flowgraph.nodes().contains(input.block())) {
        JST_ERROR("[INSTANCE] Module '{}' doesn't exist.", input);
        return Result::ERROR;
    }

    // Update module.

    JST_CHECK(blockUpdater(input.block(), [&](std::shared_ptr<Flowgraph::Node>& node) {
        // Change data type.
        const auto& [inputDataType, outputDataType] = type;
        node->fingerprint.inputDataType = inputDataType;
        node->fingerprint.outputDataType = outputDataType;

        // Clean-up every state because they will most likely be incompatible.
        node->inputMap.clear();
        node->outputMap.clear();
        node->configMap.clear();

        return Result::SUCCESS;
    }));

    return Result::SUCCESS;
}

Result Instance::renameBlock(Locale input, const std::string& id) {
    JST_DEBUG("[INSTANCE] Renaming block '{}' to '{}'.", input.blockId, id);

    // Verify input parameters.

    if (!_flowgraph.nodes().contains(input.block())) {
        JST_ERROR("[INSTANCE] Block '{}' doesn't exist.", input);
        return Result::ERROR;
    }

    // Update block.

    JST_CHECK(blockUpdater(input.block(), [&](std::shared_ptr<Flowgraph::Node>& node) {
        // Change ID.
        node->id = id;

        return Result::SUCCESS;
    }));

    return Result::SUCCESS;
}

Result Instance::blockUpdater(Locale locale, 
                              const std::function<Result(std::shared_ptr<Flowgraph::Node>&)>& updater) {
    // List all dependencies.

    std::vector<Locale> dependencyTree;
    JST_CHECK(fetchDependencyTree(locale.block(), dependencyTree));

    // Copy and update dependency blocks nodes and then delete.

    std::vector<std::shared_ptr<Flowgraph::Node>> dependencyRecords;
    for (const auto& dependencyLocale : dependencyTree | std::ranges::views::reverse) {
        // Fetch dependency record.
        auto& record = _flowgraph.nodes().at(dependencyLocale);

        // Update dependency block records.
        JST_CHECK(record->updateMaps());

        // Copy dependency block records.
        dependencyRecords.push_back(record);

        // Delete dependency block.
        JST_CHECK(eraseBlock(dependencyLocale));
    }

    // Copy input block record.
    auto record = _flowgraph.nodes().at(locale.block());

    // Update input block records.
    JST_CHECK(record->updateMaps());

    // Delete input block.
    JST_CHECK(eraseBlock(locale.block()));

    // Backup original record.
    Flowgraph::Node recordBackup = *record;

    // Call user defined record updater.
    JST_CHECK(updater(record));

    auto res = Result::SUCCESS;

    // Check if the module store has such a record fingerprint.
    if (!Store::BlockConstructorList().contains(record->fingerprint)) {
        JST_ERROR("[INSTANCE] Module fingerprint doesn't exist: '{}'.", record->fingerprint);        
        return Result::ERROR;
    } else {
        res = Store::BlockConstructorList().at(record->fingerprint)(*this,
                                                                    record->id,
                                                                    record->configMap,
                                                                    record->inputMap,
                                                                    record->stateMap);
    }

    // Create new input block using saved records.
    const auto errorCodeBackup = JST_LOG_LAST_ERROR();

    // If recreation fails, rewind the module state.
    if (res != Result::SUCCESS) {
        JST_TRACE("[INSTANCE] Module recreation failed. Rewinding state.");
        JST_CHECK(Store::BlockConstructorList().at(recordBackup.fingerprint)(*this,
                                                                             recordBackup.id,
                                                                             recordBackup.configMap,
                                                                             recordBackup.inputMap,
                                                                             recordBackup.stateMap));
    }

    // Create new dependency block using saved records.
    for (auto& dependencyRecord : dependencyRecords | std::ranges::views::reverse) {
        // Update input of dependency block record.
        for (auto& [inputName, inputRecord] : dependencyRecord->inputMap) {
            auto inputLocale = inputRecord.locale;

            // Update block ID if necessary.
            if (inputLocale.blockId == recordBackup.id) {
                JST_TRACE("[INSTANCE] Updating block ID of input '{}' to '{}'.", inputLocale.blockId, record->id)
                inputLocale.blockId = record->id;
            }

            const auto& outputRecord = _flowgraph.nodes().at(inputLocale.block());
            inputRecord = outputRecord->outputMap.at(inputLocale.pinId);
        }

        // Create new dependency block with updated inputs.
        JST_CHECK(Store::BlockConstructorList().at(dependencyRecord->fingerprint)(*this,
                                                                                  dependencyRecord->id,
                                                                                  dependencyRecord->configMap,
                                                                                  dependencyRecord->inputMap,
                                                                                  dependencyRecord->stateMap));
    }

    JST_LOG_LAST_ERROR() = errorCodeBackup;

    return res;
}

Result Instance::eraseModule(Locale locale) {
    // Verify input parameters.
    if (!_flowgraph.nodes().contains(locale)) {
        return Result::SUCCESS;
    }

    // Remove block from schedule.
    JST_CHECK(_scheduler.removeModule(locale));

    // Remove module from state.
    auto state = _flowgraph.nodes().extract(locale).mapped();

    // Destroy the present logic.
    if (state->present) {
        JST_CHECK(state->present->destroyPresent());
    }

    // Destroy the module or bundle.
    JST_CHECK(state->module->destroy());

    // Remove module from order.
    _flowgraph.nodesOrder().erase(std::find(_flowgraph.nodesOrder().begin(), _flowgraph.nodesOrder().end(), locale));

    return Result::SUCCESS;
}

Result Instance::eraseBlock(Locale locale) {
    // Verify input parameters.
    if (!_flowgraph.nodes().contains(locale)) {
        JST_ERROR("[INSTANCE] Block '{}' doesn't exist.", locale);
        return Result::ERROR;
    }

    // Remove block from compositor.
    JST_CHECK(_compositor.removeBlock(locale));

    // Remove module from state.
    auto state = _flowgraph.nodes().extract(locale).mapped();

    // Destroy the module or bundle.
    JST_CHECK(state->block->destroy());

    // Remove block from order.
    _flowgraph.nodesOrder().erase(std::find(_flowgraph.nodesOrder().begin(), _flowgraph.nodesOrder().end(), locale));

    return Result::SUCCESS;
}

Result Instance::reset() {
    JST_DEBUG("[INSTANCE] Reseting instance.");

    // Destroying compositor and scheduler.

    JST_CHECK(_compositor.destroy());
    JST_CHECK(_scheduler.destroy());

    // Destroying blocks.

    std::vector<Locale> block_erase_list;

    for (const auto& [locale, state] : _flowgraph.nodes()) {
        if (state->block) {
            block_erase_list.push_back(locale);
        }
    }

    for (const auto& locale : block_erase_list) {
        JST_TRACE("[INSTANCE] Resetting block '{}'.", locale);
        JST_CHECK(eraseBlock(locale));
    }

    return Result::SUCCESS;
}

Result Instance::fetchDependencyTree(Locale locale, std::vector<Locale>& storage) {
    std::stack<Locale> stack;
    std::unordered_set<Locale, Locale::Hasher> seenLocales;

    stack.push(locale);
    seenLocales.insert(locale);

    while (!stack.empty()) {
        Locale currentLocale = stack.top();
        stack.pop();

        // TODO: Kill this shit with fire. This is an exponential hack. Implement cached system.
        for (const auto& [outputPinId, outputRecord] : _flowgraph.nodes().at(currentLocale)->outputMap) {
            for (const auto& [inputLocale, inputState] : _flowgraph.nodes()) {
                for (const auto& [inputPinId, inputRecord] : inputState->inputMap) {
                    if (inputRecord.locale == outputRecord.locale) {
                        Locale nextLocale = inputLocale.block();
                        if (seenLocales.find(nextLocale) == seenLocales.end()) {
                            storage.push_back(nextLocale);
                            stack.push(nextLocale);
                            seenLocales.insert(nextLocale);
                        }
                    }
                }
            }
        }
    }

    return Result::SUCCESS;
}

Result Instance::destroy() {
    JST_DEBUG("[INSTANCE] Destroying instance.");

    // Clear modules.
    JST_CHECK(reset());

    // Destroy window and viewport.

    if (_window) {
        JST_CHECK(_window->destroy());
    }

    if (_viewport) {
        JST_CHECK(_viewport->destroy());
    }

    return Result::SUCCESS;
}

Result Instance::compute() {
    // Update the modules compute logic.
    JST_CHECK(_scheduler.compute());

    return Result::SUCCESS;
}

Result Instance::begin() {
    // Create new render frame.
    JST_CHECK(_window->begin());

    return Result::SUCCESS;
}

Result Instance::present() {
    // Update the modules present logic.
    JST_CHECK(_scheduler.present());

    return Result::SUCCESS;
}

Result Instance::end() {
    // Draw the main interface.
    JST_CHECK(_compositor.draw());

    // Finish the render frame.
    JST_CHECK(_window->end());

    // Process interactions after finishing frame.
    JST_CHECK(_compositor.processInteractions());

    return Result::SUCCESS;
}

}  // namespace Jetstream
