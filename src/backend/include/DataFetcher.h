#include <string>
#include <vector>

#ifndef __ARKBEACON_DATAFETCHER_H__
#define __ARKBEACON_DATAFETCHER_H__

namespace ArkBeacon
{
    template<typename T>
    class DataFetcher
    {
    public:

        struct ValueNamePair
        {
            const T* value;
            const std::string* name;
        };

        virtual void AddData(std::string&& name, T&& data) = 0;
        virtual ValueNamePair GetData(int index) const { return ValueNamePair { nullptr, nullptr }; }
        virtual int GetLen() const = 0;
        virtual void Clear() = 0;

        void HandleArgs(const std::vector<std::string>& args);

        void AddArg(const std::string& arg);
        std::string GetArg(int index);
        int GetArgCount();
        void ClearArgs();

    private:
        std::vector<std::string> m_args;
    };

    template<typename T>
    inline void DataFetcher<T>::AddArg(const std::string& arg)
    {
        m_args.push_back(arg);
    }

    template <typename T>
    inline std::string DataFetcher<T>::GetArg(int index)
    {
        if (index >= 0 && index < m_args.size())
            return m_args.at(index);
        else
            return std::string();
    }

    template <typename T>
    inline int DataFetcher<T>::GetArgCount()
    {
        return m_args.size();
    }

    template <typename T>
    inline void DataFetcher<T>::ClearArgs()
    {
        m_args.clear();
    }

    template <typename T>
    inline void DataFetcher<T>::HandleArgs(const std::vector<std::string>& args)
    {
        for (int i = 0; i < args.size(); i++)
        {
            AddArg(args.at(i));
        }
    }
}

#endif