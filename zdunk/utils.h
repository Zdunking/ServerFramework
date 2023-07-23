#pragma once
#include <cxxabi.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include "util/hash_util.h"
#include "util/json_util.h"
#include "util/crypto_util.h"

namespace zdunk
{
    pid_t GetThreadId();
    uint32_t GetFiberID();

    void BackTrace(std::vector<std::string> &bt, int size, int skip = 1);
    std::string BacktraceToString(int size, int skip = 2, std::string prefix = "");

    uint64_t GetCurrentMS();
    uint64_t GetCurrentUS();

    std::string ToUpper(const std::string &name);

    std::string ToLower(const std::string &name);

    std::string Time2Str(time_t ts = time(0), const std::string &format = "%Y-%m-%d %H:%M:%S");
    time_t Str2Time(const char *str, const char *format = "%Y-%m-%d %H:%M:%S");

    class FSUtil
    {
    public:
        static void ListAllFile(std::vector<std::string> &files, const std::string &path, const std::string &subfix);
        static bool Mkdir(const std::string &dirname);
        static bool IsRunningPidfile(const std::string &pidfile);
        static bool Rm(const std::string &path);
        static bool Mv(const std::string &from, const std::string &to);
        static bool Realpath(const std::string &path, std::string &rpath);
        static bool Symlink(const std::string &frm, const std::string &to);
        static bool Unlink(const std::string &filename, bool exist = false);
        static std::string Dirname(const std::string &filename);
        static std::string Basename(const std::string &filename);
        static bool OpenForRead(std::ifstream &ifs, const std::string &filename, std::ios_base::openmode mode);
        static bool OpenForWrite(std::ofstream &ofs, const std::string &filename, std::ios_base::openmode mode);
    };

    template <class V, class Map, class K>
    V GetParamValue(const Map &m, const K &k, const V &def = V())
    {
        auto it = m.find(k);
        if (it == m.end())
        {
            return def;
        }
        try
        {
            return boost::lexical_cast<V>(it->second);
        }
        catch (...)
        {
        }
        return def;
    }

    template <class V, class Map, class K>
    bool CheckGetParamValue(const Map &m, const K &k, V &v)
    {
        auto it = m.find(k);
        if (it == m.end())
        {
            return false;
        }
        try
        {
            v = boost::lexical_cast<V>(it->second);
            return true;
        }
        catch (...)
        {
        }
        return false;
    }

    class TypeUtil
    {
    public:
        static int8_t ToChar(const std::string &str);
        static int64_t Atoi(const std::string &str);
        static double Atof(const std::string &str);
        static int8_t ToChar(const char *str);
        static int64_t Atoi(const char *str);
        static double Atof(const char *str);
    };

    class Atomic
    {
    public:
        template <class T, class S = T>
        static T addFetch(volatile T &t, S v = 1)
        {
            return __sync_add_and_fetch(&t, (T)v);
        }

        template <class T, class S = T>
        static T subFetch(volatile T &t, S v = 1)
        {
            return __sync_sub_and_fetch(&t, (T)v);
        }

        template <class T, class S>
        static T orFetch(volatile T &t, S v)
        {
            return __sync_or_and_fetch(&t, (T)v);
        }

        template <class T, class S>
        static T andFetch(volatile T &t, S v)
        {
            return __sync_and_and_fetch(&t, (T)v);
        }

        template <class T, class S>
        static T xorFetch(volatile T &t, S v)
        {
            return __sync_xor_and_fetch(&t, (T)v);
        }

        template <class T, class S>
        static T nandFetch(volatile T &t, S v)
        {
            return __sync_nand_and_fetch(&t, (T)v);
        }

        template <class T, class S>
        static T fetchAdd(volatile T &t, S v = 1)
        {
            return __sync_fetch_and_add(&t, (T)v);
        }

        template <class T, class S>
        static T fetchSub(volatile T &t, S v = 1)
        {
            return __sync_fetch_and_sub(&t, (T)v);
        }

        template <class T, class S>
        static T fetchOr(volatile T &t, S v)
        {
            return __sync_fetch_and_or(&t, (T)v);
        }

        template <class T, class S>
        static T fetchAnd(volatile T &t, S v)
        {
            return __sync_fetch_and_and(&t, (T)v);
        }

        template <class T, class S>
        static T fetchXor(volatile T &t, S v)
        {
            return __sync_fetch_and_xor(&t, (T)v);
        }

        template <class T, class S>
        static T fetchNand(volatile T &t, S v)
        {
            return __sync_fetch_and_nand(&t, (T)v);
        }

        template <class T, class S>
        static T compareAndSwap(volatile T &t, S old_val, S new_val)
        {
            return __sync_val_compare_and_swap(&t, (T)old_val, (T)new_val);
        }

        template <class T, class S>
        static bool compareAndSwapBool(volatile T &t, S old_val, S new_val)
        {
            return __sync_bool_compare_and_swap(&t, (T)old_val, (T)new_val);
        }
    };

    template <class T>
    void nop(T *) {}

    template <class T>
    void delete_array(T *v)
    {
        if (v)
        {
            delete[] v;
        }
    }

    template <class T>
    class SharedArray
    {
    public:
        explicit SharedArray(const uint64_t &size = 0, T *p = 0)
            : m_size(size), m_ptr(p, delete_array<T>)
        {
        }
        template <class D>
        SharedArray(const uint64_t &size, T *p, D d)
            : m_size(size), m_ptr(p, d){};

        SharedArray(const SharedArray &r)
            : m_size(r.m_size), m_ptr(r.m_ptr)
        {
        }

        SharedArray &operator=(const SharedArray &r)
        {
            m_size = r.m_size;
            m_ptr = r.m_ptr;
            return *this;
        }

        T &operator[](std::ptrdiff_t i) const
        {
            return m_ptr.get()[i];
        }

        T *get() const
        {
            return m_ptr.get();
        }

        bool unique() const
        {
            return m_ptr.unique();
        }

        long use_count() const
        {
            return m_ptr.use_count();
        }

        void swap(SharedArray &b)
        {
            std::swap(m_size, b.m_size);
            m_ptr.swap(b.m_ptr);
        }

        bool operator!() const
        {
            return !m_ptr;
        }

        operator bool() const
        {
            return !!m_ptr;
        }

        uint64_t size() const
        {
            return m_size;
        }

    private:
        uint64_t m_size;
        std::shared_ptr<T> m_ptr;
    };

    class StringUtil
    {
    public:
        static std::string Format(const char *fmt, ...);
        static std::string Formatv(const char *fmt, va_list ap);

        static std::string UrlEncode(const std::string &str, bool space_as_plus = true);
        static std::string UrlDecode(const std::string &str, bool space_as_plus = true);

        static std::string Trim(const std::string &str, const std::string &delimit = " \t\r\n");
        static std::string TrimLeft(const std::string &str, const std::string &delimit = " \t\r\n");
        static std::string TrimRight(const std::string &str, const std::string &delimit = " \t\r\n");

        static std::string WStringToString(const std::wstring &ws);
        static std::wstring StringToWString(const std::string &s);
    };

    template <class T>
    const char *TypeToName()
    {
        static const char *s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
        return s_name;
    }

}