#pragma once


#ifndef FS_HASH_MAP_HPP
#define FS_HASH_MAP_HPP


#include <FsContainer/Include/HashMap.h>

#include <FsContainer/Include/Hash.hpp>
#include <FsContainer/Include/StaticBitArray.hpp>
#include <FsContainer/Include/Vector.hpp>


namespace fs
{
    template<typename Key, typename Value>
    inline HashMap<Key, Value>::HashMap()
        : _bucketArray{ kSegmentLength }
        , _bucketCount{ 0 }
    {
        __noop;
    }

    template<typename Key, typename Value>
    inline HashMap<Key, Value>::~HashMap()
    {
        __noop;
    }

    template<typename Key, typename Value>
    inline const bool HashMap<Key, Value>::contains(const Key& key) const noexcept
    {
        const uint64 keyHash = Hasher<Key>()(key);
        const uint32 startBucketIndex = computeStartBucketIndex(keyHash);
        return containsInternal(startBucketIndex, key);
    }

    template<typename Key, typename Value>
    inline const bool HashMap<Key, Value>::containsInternal(const uint32 startBucketIndex, const Key& key) const noexcept
    {
        auto& startBucket = _bucketArray[startBucketIndex];
        for (uint32 hopAt = 0; hopAt < kHopRange; ++hopAt)
        {
            if (startBucket._hopInfo.get(hopAt) == true && _bucketArray[startBucketIndex + hopAt]._key == key)
            {
                return true;
            }
        }
        return false;
    }

    template<typename Key, typename Value>
    inline void HashMap<Key, Value>::insert(const Key& key, const Value& value) noexcept
    {
        const uint64 keyHash = Hasher<Key>()(key);
        const uint32 startBucketIndex = computeStartBucketIndex(keyHash);

        if (containsInternal(startBucketIndex, key) == true)
        {
            return;
        }

        auto& startBucket = _bucketArray[startBucketIndex];
        if (false == startBucket._isUsed)
        {
            setBucket(startBucketIndex, 0, key, value);
            return;
        }

        // Find empty slot!!!
        bool foundEmptySlot = false;
        uint32 hopDistance = 0;
        for (; hopDistance < kAddRange; ++hopDistance)
        {
            if (_bucketArray.size() <= startBucketIndex + hopDistance)
            {
                break;
            }

            if (_bucketArray[startBucketIndex + hopDistance]._isUsed == false)
            {
                foundEmptySlot = true;
                break;
            }
        }

        if (foundEmptySlot == true)
        {
            // Check if it is closest
            
            if (hopDistance < kHopRange)
            {
                setBucket(startBucketIndex, hopDistance, key, value);
                return;
            }

            do
            {
                if (displace(startBucketIndex, hopDistance) == false)
                {
                    break;
                }
            } while (kHopRange <= hopDistance);
            
            if (hopDistance < kHopRange)
            {
                setBucket(startBucketIndex, hopDistance, key, value);
                return;
            }
        }

        resize();
        insert(key, value);
    }

    template<typename Key, typename Value>
    inline const KeyValuePair<Key, Value> HashMap<Key, Value>::find(const Key& key) const noexcept
    {
        const uint64 keyHash = Hasher<Key>()(key);
        const uint32 startBucketIndex = computeStartBucketIndex(keyHash);
        auto& startBucket = _bucketArray[startBucketIndex];
        KeyValuePair<Key, Value> keyValuePair;
        for (uint32 hopAt = 0; hopAt < kHopRange; ++hopAt)
        {
            if (startBucket._hopInfo.get(hopAt) == true && _bucketArray[startBucketIndex + hopAt]._key == key)
            {
                keyValuePair._key = &_bucketArray[startBucketIndex + hopAt]._key;
                keyValuePair._value = &_bucketArray[startBucketIndex + hopAt]._value;
                break;
            }
        }
        return keyValuePair;
    }

    template<typename Key, typename Value>
    inline void HashMap<Key, Value>::erase(const Key& key) noexcept
    {
        const uint64 keyHash = Hasher<Key>()(key);
        const uint32 startBucketIndex = computeStartBucketIndex(keyHash);
        auto& startBucket = _bucketArray[startBucketIndex];
        int32 hopDistance = -1;
        for (uint32 hopAt = 0; hopAt < kHopRange; ++hopAt)
        {
            if (startBucket._hopInfo.get(hopAt) == true && _bucketArray[startBucketIndex + hopAt]._key == key)
            {
                hopDistance = static_cast<int32>(hopAt);
                break;
            }
        }
        if (0 <= hopDistance)
        {
            startBucket._hopInfo.set(hopDistance, false);

            _bucketArray[startBucketIndex + hopDistance]._isUsed = false;

            --_bucketCount;
        }
    }

    template<typename Key, typename Value>
    inline void HashMap<Key, Value>::clear() noexcept
    {
        _bucketArray.clear();
        _bucketArray.resize(kSegmentLength);
        
        _bucketCount = 0;
    }

    template<typename Key, typename Value>
    inline void HashMap<Key, Value>::resize() noexcept
    {
        const float load = static_cast<float>(_bucketCount) / static_cast<float>(_bucketArray.size());
        FS_LOG("김장원", "HashMap resizes with load [%f, %d/%d]", load, _bucketCount, _bucketArray.size());

        fs::Vector<Bucket> oldBucketArray = _bucketArray;

        _bucketCount = 0;

        _bucketArray.clear();
        _bucketArray.resize(oldBucketArray.size() * 2);

        const uint32 oldBucketCount = oldBucketArray.size();
        for (uint32 oldBucketIndex = 0; oldBucketIndex < oldBucketCount; ++oldBucketIndex)
        {
            if (oldBucketArray[oldBucketIndex]._isUsed == true)
            {
                insert(oldBucketArray[oldBucketIndex]._key, oldBucketArray[oldBucketIndex]._value);
            }
        }
    }

    template<typename Key, typename Value>
    inline void HashMap<Key, Value>::setBucket(const uint32 bucketIndex, const uint32 hopDistance, const Key& key, const Value& value) noexcept
    {
        _bucketArray[bucketIndex]._hopInfo.set(hopDistance, true);

        _bucketArray[bucketIndex + hopDistance]._isUsed = true;
        _bucketArray[bucketIndex + hopDistance]._key = key;
        _bucketArray[bucketIndex + hopDistance]._value = value;

        ++_bucketCount;
    }

    template<typename Key, typename Value>
    inline const bool HashMap<Key, Value>::displace(const uint32 startBucketIndex, uint32& hopDistance) noexcept
    {
        const uint32 bucketH_1Index = startBucketIndex + hopDistance - (kHopRange - 1);
        auto& bucketH_1 = _bucketArray[bucketH_1Index];
        for (uint32 hopAt = 0; hopAt < kHopRange; ++hopAt)
        {
            if (bucketH_1._hopInfo.get(hopAt) == true)
            {
                displaceBucket(bucketH_1Index, hopAt, 3);
                hopDistance = bucketH_1Index + hopAt - startBucketIndex;
                return true;
            }
        }
        return false;
    }

    template<typename Key, typename Value>
    inline void HashMap<Key, Value>::displaceBucket(const uint32 bucketIndex, const uint32 hopDistanceA, const uint32 hopDistanceB) noexcept
    {
        auto& baseBucket = _bucketArray[bucketIndex];
        auto& bucketA = _bucketArray[bucketIndex + hopDistanceA];
        auto& bucketB = _bucketArray[bucketIndex + hopDistanceB];
        FS_ASSERT("김장원", hopDistanceA < kHopRange, "HopDistance 는 반드시 HopRange 안에 있어야 합니다!!!");
        FS_ASSERT("김장원", hopDistanceB < kHopRange, "HopDistance 는 반드시 HopRange 안에 있어야 합니다!!!");
        FS_ASSERT("김장원", bucketB._isUsed == false, "BucketB 는 비어 있어야만 합니다!!!");
        
        baseBucket._hopInfo.set(hopDistanceA, false);
        baseBucket._hopInfo.set(hopDistanceB, true);

        bucketA._isUsed = false;
        
        bucketB._isUsed = true;
        bucketB._key = std::move(bucketA._key);
        bucketB._value = std::move(bucketA._value);
    }

    template<typename Key, typename Value>
    inline const uint32 HashMap<Key, Value>::computeSegmentIndex(const uint64 keyHash) const noexcept
    {
        return keyHash % (_bucketArray.size() / kSegmentLength);
    }

    template<typename Key, typename Value>
    inline const uint32 HashMap<Key, Value>::computeStartBucketIndex(const uint64 keyHash) const noexcept
    {
        return (kSegmentLength * computeSegmentIndex(keyHash)) + keyHash % kSegmentLength;
        //return keyHash % _bucketArray.size();
    }
}


#endif // !FS_HASH_MAP_HPP
