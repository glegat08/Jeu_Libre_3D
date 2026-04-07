#include <filesystem>
#include "Core/Window.h"
#include "Game.h"

int main(int argc, char** argv)
{
    std::filesystem::path projectRoot = std::filesystem::path(argv[0])
        .parent_path().parent_path().parent_path().parent_path().parent_path();

    KGR::RenderWindow::Init();

    std::unique_ptr<KGR::RenderWindow> window = std::make_unique<KGR::RenderWindow>(
        glm::vec2{ 1920, 1080 }, "KGR Engine", projectRoot / "Ressources");

    window->GetInputManager()->SetMode(GLFW_CURSOR_DISABLED);

    RunGame(window);

    window->Destroy();
    KGR::RenderWindow::End();
}