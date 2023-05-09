#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <ij/RunGame.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer.h>
#include <iostream>

namespace ij
{
    using UniqueTexture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;
    using UniqueFont = std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)>;
    using UniqueSurface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>;

    struct SdlTextureManager final : TextureLoader
    {
        explicit SdlTextureManager(SDL_Renderer &renderer)
            : _renderer(renderer)
        {
        }

        [[nodiscard]] std::optional<TextureId> LoadFromFile(const std::filesystem::path &textureFile) override;

    private:
        SDL_Renderer &_renderer;
        std::vector<UniqueTexture> _textures;
    };

    std::optional<TextureId> SdlTextureManager::LoadFromFile(const std::filesystem::path &textureFile)
    {
        UniqueSurface surface(IMG_Load(textureFile.string().c_str()), &SDL_FreeSurface);
        if (!surface)
        {
            return std::nullopt;
        }
        UniqueTexture texture(SDL_CreateTextureFromSurface(&_renderer, surface.get()), &SDL_DestroyTexture);
        if (!texture)
        {
            return std::nullopt;
        }
        const TextureId result{AssertCast<UInt32>(_textures.size())};
        _textures.emplace_back(std::move(texture));
        return result;
    }

    struct SdlCanvas final : Canvas
    {
        explicit SdlCanvas(SDL_Window &window)
            : _window(window)
        {
        }

        [[nodiscard]] Vector2u GetSize() override
        {
            int width = 0;
            int height = 0;
            SDL_GetWindowSize(&_window, &width, &height);
            return Vector2u(AssertCast<Uint32>(width), AssertCast<Uint32>(height));
        }

        void DrawDot(const Vector2i &position, const Color color) override
        {
        }

        void DrawRectangle(const Vector2i &topLeft, const Vector2u &size, const Color outline, const Color fill,
                           float outlineThickness) override
        {
        }

        void DrawSprite(const Sprite &sprite) override
        {
        }

        [[nodiscard]] Text CreateText(const std::string &content, FontId font, UInt32 size, const Vector2f &position,
                                      Color fillColor, Color outlineColor, float outlineThickness) override
        {
            return Text(*this, 0);
        }

        void SetTextPosition(TextId id, const Vector2f &position) override
        {
        }

        Vector2f GetTextPosition(TextId id) override
        {
            return Vector2f(0, 0);
        }

        void DeleteText(TextId id) override
        {
        }

        void DrawText(TextId id) override
        {
        }

    private:
        SDL_Window &_window;
    };

    struct SdlWindowFunctions : WindowFunctions
    {
        explicit SdlWindowFunctions(SDL_Renderer &renderer)
            : _renderer(renderer)
        {
        }

        [[nodiscard]] bool IsOpen() override
        {
            return _isOpen;
        }

        void ProcessEvents(Input &input, const Camera &camera, World &world) override
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL2_ProcessEvent(&event);

                if (event.type == SDL_QUIT)
                {
                    _isOpen = false;
                }
            }
        }

        void UpdateGui(TimeSpan deltaTime) override
        {
            ImGui::GetIO().DeltaTime = AssertCast<float>(deltaTime.Milliseconds) / 1000.0f;
            ImGui_ImplSDLRenderer_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
        }

        void Clear() override
        {
            SDL_RenderClear(&_renderer);
        }

        void SetView(const Rectangle<float> &view) override
        {
        }

        void RenderGui() override
        {
            ImGui::Render();
            ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        }

        void Display() override
        {
            SDL_RenderPresent(&_renderer);
            SDL_Delay(15);
        }

        [[nodiscard]] TimeSpan RestartDeltaClock() override
        {
            const Uint64 now = SDL_GetTicks64();
            const Uint64 delta = (now - _lastClockRestart);
            _lastClockRestart = now;
            return TimeSpan::FromMilliseconds(AssertCast<Int64>(delta));
        }

    private:
        SDL_Renderer &_renderer;
        bool _isOpen = true;
        Uint64 _lastClockRestart = SDL_GetTicks64();
    };
} // namespace ij

int main(int argc, char **argv)
{
    using UniqueWindow = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
    using UniqueRenderer = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;

    using namespace ij;

    if (SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed\n";
        return 1;
    }
    if (TTF_Init())
    {
        std::cerr << "TTF_Init failed\n";
        return 1;
    }
    const std::filesystem::path assets =
        std::filesystem::current_path().parent_path().parent_path() / "improved-journey" / "assets";

    const UniqueWindow window(SDL_CreateWindow("Improved Journey", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200,
                                               800, SDL_WINDOW_SHOWN),
                              &SDL_DestroyWindow);
    if (!window)
    {
        std::cerr << "SDL_CreateWindow failed\n";
        return 1;
    }
    const UniqueRenderer renderer(SDL_CreateRenderer(window.get(), -1, 0), &SDL_DestroyRenderer);
    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer failed\n";
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    if (!ImGui_ImplSDL2_InitForSDLRenderer(window.get(), renderer.get()))
    {
        std::cerr << "ImGui_ImplSDL2_InitForSDLRenderer failed\n";
        return 1;
    }

    if (!ImGui_ImplSDLRenderer_Init(renderer.get()))
    {
        std::cerr << "ImGui_ImplSDLRenderer_Init failed\n";
        return 1;
    }

    SdlCanvas canvas(*window);
    {
        const Vector2f windowSize = AssertCastVector<float>(canvas.GetSize());
        ImGui::GetIO().DisplaySize = ImVec2{windowSize.x, windowSize.y};
    }

    SdlTextureManager textures(*renderer);
    SdlWindowFunctions windowFunctions(*renderer);
    const bool success = RunGame(textures, canvas, assets, windowFunctions);
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    TTF_Quit();
    SDL_Quit();
    return !success;
}
