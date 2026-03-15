#pragma once
#include <stdexcept>
#include <vector>

/**
 * @brief Generic structure holding a value and a dirty flag.
 *
 * @tparam T Type of the stored value.
 *
 * This structure is useful when a system needs to know whether a value
 * has changed and must be re-uploaded or synchronized.
 */
template<typename T>
struct DataDirty
{
    bool isDirty = false;  ///< Indicates whether the value has been modified.
    T data = T();          ///< Stored value.
};



// -----------------------------------------------------------------------------
//  PadData
// -----------------------------------------------------------------------------

/**
 * @brief Generic declaration of PadData.
 *
 * @tparam T Type of the stored value.
 * @tparam targetSize Desired total size of the structure.
 *
 * PadData ensures a fixed memory footprint, often required for GPU layouts
 * or strict alignment constraints.
 */
template<typename T, size_t targetSize>
struct PadData;



/**
 * @brief Specialization of PadData when targetSize > sizeof(T).
 *
 * Adds internal padding to reach the desired size.
 *
 * @tparam T Type of the stored value.
 * @tparam targetSize Desired total size (must be > sizeof(T)).
 */
template<typename T, size_t targetSize>
    requires(targetSize > sizeof(T))
struct PadData<T, targetSize>
{
    /** @brief Default constructor. */
    PadData() : val(), tabPad() {}

    /** @brief Copy constructor. */
    PadData(const T& copy) : val(copy), tabPad() {}

    /** @brief Move constructor. */
    PadData(T&& move) noexcept : val(std::move(move)), tabPad() {}

    /** @brief Copy assignment. */
    PadData& operator=(const T& data)
    {
        val = data;
        return *this;
    }

    /** @brief Move assignment. */
    PadData& operator=(T&& data) noexcept
    {
        val = std::move(data);
        return *this;
    }

    /// Implicit conversion to T&
    operator T& () { return val; }

    /// Implicit conversion to const T&
    operator const T& () const { return val; }

    /// Member access operator
    T* operator->() { return &val; }

    /// Const member access operator
    const T* operator->() const { return &val; }

    T val; ///< Stored value.

private:
    char tabPad[targetSize - sizeof(T)]; ///< Padding to reach targetSize.
};



/**
 * @brief Specialization of PadData when targetSize == sizeof(T).
 *
 * No padding is added.
 */
template<typename T, size_t targetSize>
    requires(targetSize == sizeof(T))
struct PadData<T, targetSize>
{
    PadData() : val() {}
    PadData(const T& copy) : val(copy) {}
    PadData(T&& move) noexcept : val(std::move(move)) {}

    PadData& operator=(const T& data)
    {
        val = data;
        return *this;
    }

    PadData& operator=(T&& data) noexcept
    {
        val = std::move(data);
        return *this;
    }

    operator T& () { return val; }
    operator const T& () const { return val; }

    T* operator->() { return &val; }
    const T* operator->() const { return &val; }

    T val; ///< Stored value.
};



// -----------------------------------------------------------------------------
//  StorageContainer
// -----------------------------------------------------------------------------

/**
 * @brief Static container storing up to maxCount elements.
 *
 * @tparam type Element type.
 * @tparam maxCount Maximum number of elements.
 *
 * Useful for GPU uploads or any system requiring a fixed-size buffer
 * while still tracking the actual number of used elements.
 */
template<typename type, size_t maxCount>
struct StorageContainer
{
    /**
     * @brief Creates a StorageContainer from a std::vector.
     *
     * @param data Source vector.
     * @return A filled StorageContainer.
     *
     * @throws std::out_of_range if data.size() > maxCount.
     */
    static StorageContainer FromVec(const std::vector<type>& data)
    {
        if (data.size() > maxCount)
            throw std::out_of_range("too much");

        StorageContainer result;
        result.size = data.size();

        int i = 0;
        for (auto& it : data)
            result.tab[i++] = it;

        return result;
    }

    /**
     * @brief Returns the total memory capacity in bytes.
     */
    static uint32_t Capacity()
    {
        return sizeof(type) * maxCount;
    }

    /**
     * @brief Returns a pointer to the stored data.
     */
    type* Data() { return tab; }

    /**
     * @brief Returns a const pointer to the stored data.
     */
    const type* Data() const { return tab; }

    /**
     * @brief Returns the size in bytes of the used portion.
     */
    size_t UploadSize() const
    {
        return size * sizeof(type);
    }

    /**
     * @brief Returns the number of used elements.
     */
    size_t GetSize() const
    {
        return size;
    }

    /**
     * @brief Returns a pointer to the size value (useful for GPU uploads).
     */
    uint32_t* GetSizeData()
    {
        return &size;
    }

    /**
     * @brief Returns a const pointer to the size value.
     */
    const uint32_t* GetSizeData() const
    {
        return &size;
    }

private:
    uint32_t size = 0;       ///< Number of used elements.
    type tab[maxCount];      ///< Static storage array.
};