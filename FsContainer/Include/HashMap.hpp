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
    inline Bucket<Key, Value>::Bucket()
        : _isUsed{ false }
    {
        __noop;
    }

    template<typename Key, typename Value>
    inline Bucket<Key, Value>::Bucket(const Bucket& rhs)
        : _hopInfo{ rhs._hopInfo }
        , _isUsed{ rhs._isUsed }
        , _key{ rhs._key }
    {
        _value = rhs._value;
    }


    template<typename Key, typename Value>
    inline BucketViewer<Key, Value>::BucketViewer(const HashMap<Key, Value>* const hashMap)
        : _hashMap{ hashMap }
        , _bucketIndex{ 0 }
    {
        const int32 bucketArraySize = static_cast<int32>(_hashMap->_bucketArray.size());
        for (int32 bucketIndex = 0; bucketIndex < bucketArraySize; ++bucketIndex)
        {
            if (_hashMap->_bucketArray[bucketIndex]._isUsed == true)
            {
                _bucketIndex = bucketIndex;
                break;
            }
        }
    }

    template<typename Key, typename Value>
    inline const bool BucketViewer<Key, Value>::isValid() const noexcept
    {
        return static_cast<uint32>(_bucketIndex) < _hashMap->_bucketArray.size();
    }

    template<typename Key, typename Value>
    inline void BucketViewer<Key, Value>::next() noexcept
    {
        const int32 bucketArraySize = static_cast<int32>(_hashMap->_bucketArray.size());
        for (int32 bucketIndex = _bucketIndex + 1; bucketIndex < bucketArraySize; ++bucketIndex)
        {
            if (_hashMap->_bucketArray[bucketIndex]._isUsed == true)
            {
                _bucketIndex = bucketIndex;
                return;
            }
        }
        _bucketIndex = -1;
    }

    template<typename Key, typename Value>
    inline KeyValuePairConst<Key, Value> BucketViewer<Key, Value>::view() noexcept
    {
        const Bucket<Key, Value>& bucket = _hashMap->_bucketArray[_bucketIndex];
        KeyValuePairConst<Key, Value> keyValuePairConst;
        keyValuePairConst._key = &bucket._key;
        keyValuePairConst._value = &bucket._value;
        return keyValuePairConst;
    }


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
                keyValuePair._value = const_cast<Value*>(&_bucketArray[startBucketIndex + hopAt]._value);
                break;
            }
        }
        return keyValuePair;
    }

    template<typename Key, typename Value>
    inline const Value& HashMap<Key, Value>::at(const Key& key) const noexcept
    {
        const Value* const value = find(key)._value;
        return (value == nullptr) ? getInvalidValue() : *value;
    }

    template<typename Key, typename Value>
    inline Value& HashMap<Key, Value>::at(const Key& key) noexcept
    {
        Value* const value = find(key)._value;
        return (value == nullptr) ? getInvalidValue() : *value;
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
    inline BucketViewer<Key, Value> HashMap<Key, Value>::getBucketViewer() const noexcept
    {
        return BucketViewer<Key, Value>(this);
    }

    template<typename Key, typename Value>
    inline Value& HashMap<Key, Value>::getInvalidValue() noexcept
    {
        static Value invalidValue;
        return invalidValue;
    }

    template<typename Key, typename Value>
    inline const uint32 HashMap<Key, Value>::size() const noexcept
    {
        return _bucketCount;
    }

    template<typename Key, typename Value>
    inline const bool HashMap<Key, Value>::empty() const noexcept
    {
        return 0 == _bucketCount;
    }

    template<typename Key, typename Value>
    inline void HashMap<Key, Value>::resize() noexcept
    {
        const float load = static_cast<float>(_bucketCount) / static_cast<float>(_bucketArray.size());
        FS_LOG("�����", "HashMap resizes with load [%f, %d/%d]", load, _bucketCount, _bucketArray.size());

        fs::Vector<Bucket<Key, Value>> oldBucketArray = _bucketArray;

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
        FS_ASSERT("�����", hopDistanceA < kHopRange, "HopDistance �� �ݵ�� HopRange �ȿ� �־�� �մϴ�!!!");
        FS_ASSERT("�����", hopDistanceB < kHopRange, "HopDistance �� �ݵ�� HopRange �ȿ� �־�� �մϴ�!!!");
        FS_ASSERT("�����", bucketB._isUsed == false, "BucketB �� ��� �־�߸� �մϴ�!!!");
        
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
    }
}


#endif // !FS_HASH_MAP_HPP
