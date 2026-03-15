#pragma once
#include <map>
#include <string>
#include <filesystem>

#include "Tools/TypeWrapper.h"

namespace KGR
{
    /**
     * @brief Forward declaration of the ResourceManager template.
     *
     * This class manages loading, caching, reloading, and unloading of resources.
     * It is specialized below for functions returning std::unique_ptr.
     */
    template<typename TextureType, typename ConstructArgs, auto FN>
    class ResourceManager;


    /**
     * @brief Generic resource manager for loading and caching resources.
     *
     * @tparam TextureType Type of the resource (e.g., Texture, Mesh, Collider).
     * @tparam ConstructArgs Parameter pack wrapped in TypeWrapper.
     * @tparam FN Function pointer used to load the resource.
     *
     * FN must have the signature:
     * @code
     * std::unique_ptr<TextureType> FN(const std::string& path, ConstructArgs... args);
     * @endcode
     *
     * Features:
     * - Loads resources on demand and caches them.
     * - Ensures each resource is loaded only once.
     * - Supports unloading and reloading.
     * - Uses a global root directory for resolving relative paths.
     */
    template<typename TextureType,
        typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    class ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>
    {
    public:
        using TexturePtr = std::unique_ptr<TextureType>;

        /**
         * @brief Sets the global root directory for resource loading.
         *
         * Must be called once before loading any resource.
         *
         * @throws std::runtime_error If called more than once.
         */
        static void SetGlobalFIlePath(const std::filesystem::path& racinePath);

        ~ResourceManager();

        /**
         * @brief Loads a resource or returns the cached version if already loaded.
         *
         * @param relativePath Path relative to the global root directory.
         * @param args Additional arguments forwarded to the loading function.
         * @return Reference to the loaded resource.
         */
        static TextureType& Load(const std::string& relativePath, ConstructArgs... args);

        /**
         * @brief Unloads a specific resource.
         *
         * @throws std::out_of_range If the resource is not loaded.
         */
        static void Unload(const std::string& relativePath);

        /**
         * @brief Reloads a specific resource.
         */
        static void Reload(const std::string& relativePath, ConstructArgs... args);

        /**
         * @brief Reloads all currently loaded resources.
         */
        static void ReloadAll(ConstructArgs... args);

        /**
         * @brief Unloads all resources.
         */
        static void UnloadAll();

        /**
         * @brief Checks if a resource exists in the cache.
         */
        static bool Exists(const std::string& relativePath);

        /**
         * @brief Checks if a resource pointer exists in the cache.
         */
        static bool Exists(TextureType* texture);

        /**
         * @brief Returns the relative path associated with a loaded resource.
         *
         * @throws std::out_of_range If the resource is not found.
         */
        std::string GetRelativePath(TextureType& texture);

    private:
        /**
         * @brief Converts a relative path to an absolute path using the global root.
         */
        static std::string GetAbsoluteFilePath(const std::string& relativePath);

        /**
         * @brief Converts an absolute path back to a relative path.
         */
        static std::string GetRelativeFilePath(const std::string& AbsolutePath);

        static std::filesystem::path m_absoluteFilePath; ///< Global root directory.
        static std::map<std::string, TexturePtr> m_textureMap; ///< Cached resources.
    };


    // -------------------------------------------------------------------------
    // Static member definitions
    // -------------------------------------------------------------------------

    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    std::filesystem::path
        ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::m_absoluteFilePath =
        std::filesystem::path{};

    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    std::map<std::string, std::unique_ptr<TextureType>>
        ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::m_textureMap =
        std::map<std::string, std::unique_ptr<TextureType>>{};


    // -------------------------------------------------------------------------
    // Implementation
    // -------------------------------------------------------------------------

    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    TextureType&
        ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::Load(
            const std::string& relativePath, ConstructArgs... args)
    {
        auto fullPath = GetAbsoluteFilePath(relativePath);

        // Return cached resource if already loaded
        auto it = m_textureMap.find(fullPath);
        if (it != m_textureMap.end())
            return *it->second;

        // Load new resource
        m_textureMap[fullPath] =
            std::move(FN(fullPath, std::forward<ConstructArgs>(args)...));

        return *m_textureMap[fullPath];
    }


    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    void ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::SetGlobalFIlePath(
        const std::filesystem::path& racinePath)
    {
        static bool init = false;
        if (init)
            throw std::runtime_error("Global file path can only be set once");

        m_absoluteFilePath = racinePath;
        init = true;
    }


    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::~ResourceManager()
    {
        m_textureMap.clear();
    }


    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    void ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::UnloadAll()
    {
        m_textureMap.clear();
    }


    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    void ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::Reload(
        const std::string& relativePath, ConstructArgs... args)
    {
        Unload(relativePath);
        Load(relativePath, std::forward<ConstructArgs>(args)...);
    }


    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    void ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::ReloadAll(
        ConstructArgs... args)
    {
        for (auto it = m_textureMap.begin(); it != m_textureMap.end(); )
        {
            auto relativePath = GetRelativeFilePath(it->first);
            it = m_textureMap.erase(it);
            Load(relativePath, std::forward<ConstructArgs>(args)...);
        }
    }


    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    bool ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::Exists(
        const std::string& relativePath)
    {
        return m_textureMap.contains(GetAbsoluteFilePath(relativePath));
    }


    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    bool ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::Exists(
        TextureType* texture)
    {
        auto it = std::find_if(
            m_textureMap.begin(), m_textureMap.end(),
            [&](const auto& pair) { return pair.second.get() == texture; });

        return it != m_textureMap.end();
    }


    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    std::string ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::GetRelativePath(
            TextureType& texture)
    {
        auto it = std::find_if(
            m_textureMap.begin(), m_textureMap.end(),
            [&](const auto& pair) { return pair.second.get() == &texture; });

        if (it == m_textureMap.end())
            throw std::out_of_range("Resource does not exist");

        return GetRelativeFilePath(it->first);
    }


    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    void ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::Unload(
        const std::string& relativePath)
    {
        if (!Exists(relativePath))
            throw std::out_of_range("Resource is not loaded");

        m_textureMap.erase(GetAbsoluteFilePath(relativePath));
    }


    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    std::string ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::GetAbsoluteFilePath(
            const std::string& relativePath)
    {
        auto filePath = m_absoluteFilePath / std::filesystem::path(relativePath);
        return filePath.string();
    }


    template<typename TextureType, typename... ConstructArgs,
        std::unique_ptr<TextureType>(*FN)(const std::string&, ConstructArgs...)>
    std::string ResourceManager<TextureType, TypeWrapper<ConstructArgs...>, FN>::GetRelativeFilePath(
            const std::string& AbsolutePath)
    {
        std::filesystem::path full = AbsolutePath;
        std::filesystem::path local = relative(full, m_absoluteFilePath);
        return local.string();
    }

} // namespace KGR
