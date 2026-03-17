#pragma once

#include "imgui.h"
#include <functional>
#include <filesystem>

namespace KGR
{
    namespace Editor
    {
        /** @brief top-level menu bar rendered above the dockspace */
        class MenuBar
        {
        public:
            using SaveCallback = std::function<void(const std::filesystem::path&)>;
            using LoadCallback = std::function<void(const std::filesystem::path&)>;

            /**
             * @param onSave called with the chosen path on File > Save As
             * @param onLoad called with the chosen path on File > Open
             */
            MenuBar(SaveCallback onSave, LoadCallback onLoad);

            /**
             * @brief draws the main menu bar
             * @note call once per frame, before BeginDockspace()
             */
            void Render();

        private:
            SaveCallback m_onSave;
            LoadCallback m_onLoad;
        };
    }
}