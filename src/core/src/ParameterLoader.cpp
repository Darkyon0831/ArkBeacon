#include "ParameterLoader.h"
#include "PythonHandler.h" 
#include "DataFetcher.h"

#include <pybind11/pybind11.h>

PYTHON_MODULE_START(ParameterLoader, ArkBeacon::ParameterLoader)
PYTHON_MODULE_ADD_METHOD(AddData)
PYTHON_MODULE_ADD_METHOD(GetArg)
PYTHON_MODULE_ADD_METHOD(GetArgCount)
PYTHON_MODULE_END()

void ArkBeacon::ParameterLoader::AddData(std::string&& name, std::string&& data)
{
    m_data.push_back(data);
    m_names.push_back(name);
}

ArkBeacon::ParameterLoader::ValueNamePair ArkBeacon::ParameterLoader::GetData(int index) const
{
    ValueNamePair pair;
    pair.name = nullptr;
    pair.value = nullptr;

    if (index >= 0 && index < m_names.size())
    {
        pair.name = &m_names.at(index);
        pair.value = &m_data.at(index);
    }

    return pair;
}

int ArkBeacon::ParameterLoader::GetLen() const
{
    return m_names.size();
}

void ArkBeacon::ParameterLoader::Clear()
{
    m_names.clear();
    m_data.clear();
}
