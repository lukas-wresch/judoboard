#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <cassert>



namespace ZED
{
	//Mutex where multiple thread can read data at the same time,
	//but only one thread can write

    class IReadWriteMutex
    {
    public:
        class ScopedReadLock
        {
        public:
            ScopedReadLock(const IReadWriteMutex& Mutex) : m_Mutex(Mutex) { Mutex.LockRead(); }
            ~ScopedReadLock() { m_Mutex.UnlockRead(); }

        private:
            const IReadWriteMutex& m_Mutex;
        };


        class ScopedWriteLock
        {
        public:
            ScopedWriteLock(IReadWriteMutex& Mutex) : m_Mutex(Mutex) { Mutex.LockWrite(); }
            ~ScopedWriteLock() { m_Mutex.UnlockWrite(); }

        private:
            IReadWriteMutex& m_Mutex;
        };


        [[nodiscard]]
        ScopedReadLock  LockReadForScope() const { return ScopedReadLock(*this); }
        [[nodiscard]]
        ScopedWriteLock LockWriteForScope() { return ScopedWriteLock(*this); }

        //Read access
        virtual void LockRead() const    = 0;
        virtual void UnlockRead() const  = 0;
        virtual bool TryReadLock() const = 0;

        //Write access
        virtual void LockWrite()    = 0;
        virtual void UnlockWrite()  = 0;
        virtual bool TryWriteLock() = 0;
    };


    class ReadWriteMutex : public IReadWriteMutex
    {
    public:
        //Read access
        void LockRead() const
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            while (m_Writing || m_WaitingWriters > 0)
                m_Cond.wait(lock);
            m_ReadCount++;
        }

        void UnlockRead() const
        {
            assert(m_ReadCount > 0);
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_ReadCount--;
            if (m_ReadCount == 0 && m_WaitingWriters > 0)
                m_Cond.notify_all();
        }

        bool TryReadLock() const
        {
            std::unique_lock<std::mutex> lock(m_Mutex, std::try_to_lock);
            if (!lock || m_Writing)
                return false;
            m_ReadCount++;
            return true;
        }

        //Write access
        void LockWrite()
        {
            std::unique_lock<std::mutex> lock(m_Mutex);

            m_WaitingWriters++;
            while (m_ReadCount > 0 || m_Writing)
                m_Cond.wait(lock);
            m_WaitingWriters--;

            m_Writing = true;
        }

        void UnlockWrite()
        {
            assert(m_Writing);
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Writing = false;
            if (m_WaitingWriters > 0)
                m_Cond.notify_all();
            else
                m_Cond.notify_one();
        }

        bool TryWriteLock()
        {
            std::unique_lock<std::mutex> lock(m_Mutex, std::try_to_lock);
            if (!lock || m_ReadCount > 0 || m_Writing)
                return false;
            m_Writing = true;
            return true;
        }

    private:
        bool m_Writing = false;
        int m_WaitingWriters = 0;
        mutable int m_ReadCount = 0;
        mutable std::mutex m_Mutex;
        mutable std::condition_variable m_Cond;
	};


    class RecursiveReadWriteMutex : public IReadWriteMutex
    {
    public:
        //Read access
        void LockRead() const
        {
            std::unique_lock<std::recursive_mutex> lock(m_Mutex);

            if (HasReadLock() || HasWriteLock())
                return;

            while (m_WaitingWriters > 0 ||
                  ( m_WritingCount > 0 && m_OwnerID != std::this_thread::get_id()) )
                m_Cond.wait(lock);
            m_ReadCount++;
            m_ReaderIDs[std::this_thread::get_id()] = 1;
        }

        void UnlockRead() const
        {
            std::unique_lock<std::recursive_mutex> lock(m_Mutex);

            if (m_ReaderIDs[std::this_thread::get_id()] == 0)
                return;

            assert(m_ReadCount > 0);
            assert(m_ReaderIDs[std::this_thread::get_id()] == 1);
            m_ReadCount--;
            m_ReaderIDs[std::this_thread::get_id()] = 0;

            if (m_ReadCount == 0 && m_WaitingWriters > 0)
                m_Cond.notify_all();
        }

        bool TryReadLock() const
        {
            std::unique_lock<std::recursive_mutex> lock(m_Mutex, std::try_to_lock);
            if (!lock || (m_WritingCount > 0 && m_OwnerID != std::this_thread::get_id()) )
                return false;

            m_ReadCount++;
            m_ReaderIDs[std::this_thread::get_id()] = 1;
            return true;
        }

        bool HasReadLock() const
        {
            std::unique_lock<std::recursive_mutex> lock(m_Mutex);

            auto it = m_ReaderIDs.find(std::this_thread::get_id());
            const bool has_read_lock = (it != m_ReaderIDs.end()) && (it->second > 0);

            if (has_read_lock)
                assert(m_ReadCount > 0);//Assert that there are active readers if we have a lock
            return has_read_lock;
        }

        //Write access
        void LockWrite()
        {
            std::unique_lock<std::recursive_mutex> lock(m_Mutex);

            m_WaitingWriters++;

            int own_read_locks = 0;
            if (HasReadLock())
                own_read_locks = m_ReaderIDs.find(std::this_thread::get_id())->second;
            assert(m_ReadCount >= own_read_locks);

            while (m_ReadCount - own_read_locks > 0 || m_ReadCount > 0 || (m_WritingCount > 0 && m_OwnerID != std::this_thread::get_id()) )
                m_Cond.wait(lock);
            m_WaitingWriters--;

            m_WritingCount++;
            m_OwnerID = std::this_thread::get_id();
        }

        void UnlockWrite()
        {
            assert(m_WritingCount > 0);
            std::unique_lock<std::recursive_mutex> lock(m_Mutex);

            m_WritingCount--;
            if (m_WritingCount == 0)
                m_OwnerID = std::thread::id();

            if (m_WaitingWriters > 0)
                m_Cond.notify_all();
            else
                m_Cond.notify_one();
        }

        bool TryWriteLock()
        {
            std::unique_lock<std::recursive_mutex> lock(m_Mutex, std::try_to_lock);
            if (!lock || m_ReadCount > 0 || (m_WritingCount > 0 && m_OwnerID != std::this_thread::get_id()) )
                return false;

            m_WritingCount++;
            m_OwnerID = std::this_thread::get_id();
            return true;
        }

        bool HasWriteLock() const
        {
            std::unique_lock<std::recursive_mutex> lock(m_Mutex, std::try_to_lock);
            return m_OwnerID == std::this_thread::get_id();
        }

    private:
        int m_WritingCount = 0;
        int m_WaitingWriters = 0;
        mutable int m_ReadCount = 0;

        mutable std::recursive_mutex m_Mutex;
        mutable std::condition_variable_any m_Cond;

        std::thread::id m_OwnerID;
        mutable std::unordered_map<std::thread::id, int> m_ReaderIDs;
    };
}