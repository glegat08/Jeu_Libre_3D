#include "Serialiser.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

namespace KGR
{
    namespace Editor
    {
        void Serializer::Save(Scene& scene, const std::filesystem::path& path)
        {
            const SceneRegistry& reg = scene.GetRegistry();

            // --- Step 1: collect entities in depth-first order ---
            // Parents must come before their children so that Load can process
            // the parent's HierarchyComponent before the child tries to register
            // itself in the parent's children list.
            std::vector<SceneEntity> ordered;

            for (SceneEntity e : reg.GetAllEntities())
            {
                if (!reg.HasComponent<HierarchyComponent>(e))
                    continue;

                if (reg.GetComponent<HierarchyComponent>(e).m_parent == NullEntity)
                    CollectDFS(e, reg, ordered);
            }

            // Entities without a HierarchyComponent go at the end (unlikely but safe).
            for (SceneEntity e : reg.GetAllEntities())
                if (!reg.HasComponent<HierarchyComponent>(e))
                    ordered.push_back(e);

            // --- Step 2: build handle -> index map ---
            // Each entity gets a stable integer index (0, 1, 2...) that will be
            // stored in the file. This is the only way to encode entity references
            // (like parent handles) since live handles change every session.
            HandleToIndex toIndex;
            for (int i = 0; i < static_cast<int>(ordered.size()); ++i)
                toIndex[ordered[i]] = i;

            // --- Step 3: serialise ---
            nlohmann::json root;
            root["version"] = 1;
            root["entities"] = nlohmann::json::array();

            for (SceneEntity e : ordered)
            {
                nlohmann::json entityJson;
                entityJson["index"] = toIndex.at(e);

                for (const ComponentSerializerEntry& entry : scene.GetSerializerRegistry())
                    entry.write(e, reg, entityJson, toIndex);

                root["entities"].push_back(entityJson);
            }

            // --- Step 4: write to disk ---
            std::ofstream file(path);
            if (!file.is_open())
                throw std::runtime_error("SceneSerializer: cannot write to: " + path.string());

            file << root.dump(4); // 4-space indent keeps the file readable in a text editor
        }

        void Serializer::Load(Scene& scene, const std::filesystem::path& path)
        {
            std::ifstream file(path);
            if (!file.is_open())
                throw std::runtime_error("Serializer: cannot read: " + path.string());

            nlohmann::json root = nlohmann::json::parse(file);

            if (root.value("version", 0) != 1)
                throw std::runtime_error("Serializer: unsupported file version.");

            // --- Pass 1: create all empty entities ---
            // We create every entity up front so that every handle exists before
            // any read lambda tries to look up a parent reference via the index map.
            IndexToHandle toHandle;
            std::vector<std::pair<SceneEntity, nlohmann::json>> pending;

            for (const nlohmann::json& entityJson : root["entities"])
            {
                int         index = entityJson["index"].get<int>();
                SceneEntity e = scene.CreateEmptyEntity();

                toHandle[index] = e;
                pending.emplace_back(e, entityJson);
            }

            // --- Pass 2: populate components ---
            // Because the file was written in DFS order, parents always appear before
            // children in the pending list. When a child's HierarchyComponent read
            // lambda adds the child to its parent's children list, the parent's
            // HierarchyComponent is guaranteed to already exist.
            SceneRegistry& reg = scene.GetRegistry();

            for (auto& [e, entityJson] : pending)
            {
                for (const ComponentSerializerEntry& entry : scene.GetSerializerRegistry())
                    entry.read(e, reg, entityJson, toHandle);
            }
        }

        void Serializer::CollectDFS(SceneEntity e, const SceneRegistry& reg,
            std::vector<SceneEntity>& out)
        {
            out.push_back(e);

            if (!reg.HasComponent<HierarchyComponent>(e))
                return;

            for (SceneEntity child : reg.GetComponent<HierarchyComponent>(e).m_children)
                CollectDFS(child, reg, out);
        }
    }
}