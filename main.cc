#define RENDER_DEBUG

#include "render/base.hpp"
#include "samurai/samurai.hpp"
#include "jetstream/fft/base.hpp"
#include "jetstream/lineplot/base.hpp"

struct State {
    bool streaming = false;

    // Render
    std::shared_ptr<Render::Instance> render;


    // Samurai
    Samurai::ChannelId rx;
    std::shared_ptr<Samurai::Airspy::Device> device;


    // Jetstream
    struct {
        Jetstream::FFT::Config fft;
        Jetstream::Lineplot::Config lpt;
    } cfg;

    Jetstream::cpu::arr::c32 input;
    std::shared_ptr<Jetstream::FFT::CPU> fft;
    std::shared_ptr<Jetstream::Lineplot::CPU> lpt;

};

auto state = std::make_shared<State>();

void dsp_loop(std::shared_ptr<State> state) {
    while (state->streaming) {
        state->device->ReadStream(state->rx, state->input.data.data(), state->input.data.size(), 1000);
        JETSTREAM_ASSERT_SUCCESS(state->fft->compute());
        JETSTREAM_ASSERT_SUCCESS(state->lpt->compute());
        JETSTREAM_ASSERT_SUCCESS(state->lpt->barrier());
    }
}

void render_loop() {
    state->render->start();

    state->lpt->present();

    ImGui::Begin("Lineplot");
    ImGui::GetWindowDrawList()->AddImage(
        (void*)state->lpt->tex()->raw(), ImVec2(ImGui::GetCursorScreenPos()),
        ImVec2(ImGui::GetCursorScreenPos().x + state->lpt->conf().width/2.0,
        ImGui::GetCursorScreenPos().y + state->lpt->conf().height/2.0), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();

    ImGui::Begin("Samurai Info");
    ImGui::Text("Buffer Fill: %ld", state->device->BufferOccupancy(state->rx));
    ImGui::End();

    state->render->end();
}

int main() {
    std::cout << "Welcome to CyberEther!" << std::endl;

    Render::Instance::Config renderCfg;
    renderCfg.width = 3000;
    renderCfg.height = 1080;
    renderCfg.resizable = true;
    renderCfg.enableImgui = true;
    renderCfg.enableDebug = true;
    renderCfg.enableVsync = true;
    renderCfg.title = "CyberEther";
    state->render = Render::Instantiate(Render::API::GLES, renderCfg);

    state->device = std::make_shared<Samurai::Airspy::Device>();

    Samurai::Device::Config deviceConfig;
    deviceConfig.sampleRate = 10e6;
    state->device->Enable(deviceConfig);

    Samurai::Channel::Config channelConfig;
    channelConfig.mode = Samurai::Mode::RX;
    channelConfig.dataFmt = Samurai::Format::F32;
    state->device->EnableChannel(channelConfig, &state->rx);

    Samurai::Channel::State channelState;
    channelState.enableAGC = true;
    channelState.frequency = 545.5e6;
    state->device->UpdateChannel(state->rx, channelState);

    state->input.data.resize(8192*8);

    state->fft = std::make_shared<Jetstream::FFT::CPU>(state->cfg.fft, state->input);

    state->cfg.lpt.render = state->render;
    state->lpt = std::make_shared<Jetstream::Lineplot::CPU>(state->cfg.lpt, state->fft, state->fft->out());

    state->render->create();
    state->device->StartStream();

    state->streaming = true;
    std::thread dsp(dsp_loop, state);

    while(state->render->keepRunning()) {
        render_loop();
    }

    state->streaming = false;
    dsp.join();

    state->device->StopStream();
    state->render->destroy();

    std::cout << "Goodbye from CyberEther!" << std::endl;
}
