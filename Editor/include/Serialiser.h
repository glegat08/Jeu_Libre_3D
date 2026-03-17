#pragma once

#include "Scene/Scene.h"
#include <filesystem>

namespace KGR
{
    namespace Editor
    {
        /** @brief saves and loads a scene to/from a JSON file on disk */
        class Serializer
        {
        public:
            /**
             * @brief writes the scene to a .kscene file
             * @param scene the scene to save
             * @param path  output path (e.g. "MyProject/main.kscene")
             */
            static void Save(Scene& scene, const std::filesystem::path& path);

            /**
             * @brief clears the scene and rebuilds it from a .kscene file
             * @param scene the target scene, cleared before loading
             * @param path  path to an existing .kscene file
             */
            static void Load(Scene& scene, const std::filesystem::path& path);

        private:
            /**
             * @brief appends entity e and all its descendants to out (depth-first)
             * @param e   root entity
             * @param reg the registry that owns the entities
             * @param out output vector to fill
             */
            static void CollectDFS(SceneEntity e, const SceneRegistry& reg,
                std::vector<SceneEntity>& out);
        };
    }
}