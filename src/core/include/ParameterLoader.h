#include <unordered_map>
#include <Python.h>
#include <string>
#include <vector>

#include "DataFetcher.h"

#ifndef __ARKBEACON_PARAMETERLOADER_H__
#define __ARKBEACON_PARAMETERLOADER_H__

namespace ArkBeacon
{
    class ParameterLoader : public DataFetcher<std::string>
    {
    public:

        ParameterLoader() {}

        void AddData(std::string&& data, std::string&& name) override;
        ValueNamePair GetData(int index) const override;
        int GetLen() const override;
        void Clear() override;
        
    private:
        std::vector<std::string> m_data;
        std::vector<std::string> m_names;
    };
}

#endif