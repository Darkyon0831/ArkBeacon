#include <vector>
#include <string>
#include <filesystem>
#include <Python.h>
#include <iostream>
#include <sstream>
#include <pybind11/pybind11.h>
#include <chrono>
#include <mutex>

#include <sys/stat.h>
#include <sys/types.h>

#ifndef __ARKBEACON_PYTHONHANDLER_H__
#define __ARKBEACON_PYTHONHANDLER_H__

#define PYTHON_MODULE_START(module, class) \
    typedef class class_t; \
    PYBIND11_MODULE(module, m) \
    { \
        pybind11::class_<class_t>(m, #module) \
        .def(pybind11::init<>())
#define PYTHON_MODULE_ADD_METHOD(function_name) \
    .def(#function_name, &class_t::function_name)
#define PYTHON_MODULE_END() ;}

namespace fs = std::filesystem;

namespace ArkBeacon
{
    template<class _dh>
    class PythonHandler
    {
    public:

        struct ErrorObject
        {
            std::string script_name;
            std::string error;
        };

        PythonHandler(std::string_view scripts_path, std::string_view module_name);
        ~PythonHandler();

        void Init();
        void Finalize();

        bool RunScript(_dh* data_handler, std::string_view script_name);
        void SetEntryPoint(std::string_view entry_point) { m_entry_point = entry_point; }

        void SetAutoReloadModule() { m_auto_reload_module = true; }

        void ReloadModule(std::string_view module_name);
        void ReloadModules();

        void SetAdditionalIncludePaths(std::function<void(std::vector<std::string>&)> additional_include_paths) { m_additional_include_paths = additional_include_paths; }

    private:
        void SetupEnvironment(std::string_view scripts_path);
        void RecordError(std::string_view script_name);
        PyObject* TryGetModule(std::string_view module_name);

        void CheckDoReload(std::string_view module_name);

        std::string m_entry_point;
        std::string m_module_name;
        std::string m_scripts_path;

        bool m_auto_reload_module;

        std::function<void(std::vector<std::string>&)> m_additional_include_paths;
        std::vector<ErrorObject> m_errors;
        std::unordered_map<std::string, time_t> m_loaded_modules;
        std::mutex m_mutex;
    };
    
    template <class _dh>
    inline PythonHandler<_dh>::PythonHandler(std::string_view scripts_path, std::string_view module_name)
        : m_entry_point("Run")
        , m_module_name(module_name)
        , m_auto_reload_module(false)
        , m_scripts_path(scripts_path)
        , m_additional_include_paths(nullptr)
    {
        
    }

    template <class _dh>
    inline PythonHandler<_dh>::~PythonHandler()
    {
        if (Py_IsInitialized())
            Py_Finalize();
    }

    template <class _dh>
    inline void PythonHandler<_dh>::Init()
    {
        SetupEnvironment(m_scripts_path);
    }

    template <class _dh>
    inline void PythonHandler<_dh>::Finalize()
    {
        if (Py_IsInitialized())
            Py_Finalize();
    }

    template <class _dh>
    inline bool PythonHandler<_dh>::RunScript(_dh* data_handler, std::string_view script_name)
    {
        if (Py_IsInitialized() && script_name != "")
        {
            if (m_auto_reload_module)
                CheckDoReload(script_name);

            PyObject* p_module = TryGetModule(script_name);

            if (p_module == nullptr)
            {
                PyObject* p_name = PyUnicode_DecodeFSDefault(script_name.data());
                p_module = PyImport_Import(p_name);

                Py_DECREF(p_name);

                if (p_module == nullptr)
                {
                    RecordError(script_name.data());
                    return false;
                }

                std::string file_path = m_scripts_path + std::string("/") + std::string(script_name) + ".py";
                struct stat result;
                stat(file_path.c_str(), &result);
                time_t timestamp = result.st_mtime;

                m_loaded_modules.insert(std::pair<std::string, time_t>(std::string(script_name), timestamp));
            }

            PyObject* p_func = PyObject_GetAttrString(p_module, m_entry_point.c_str());

            if (p_func == nullptr || PyCallable_Check(p_func) == 0)
            {
                RecordError(script_name.data());
                return false;
            }

            pybind11::object data_handler_reference = pybind11::cast(data_handler);

            PyObject* p_args = PyTuple_Pack(1, data_handler_reference.ptr());
            PyObject* p_value = PyObject_CallObject(p_func, p_args);

            Py_DECREF(p_module);
            Py_DECREF(p_func);
            Py_DECREF(p_args);
            Py_XDECREF(p_value);
        }

        return true;
    }

    template <class _dh>
    inline void PythonHandler<_dh>::ReloadModule(std::string_view module_name)
    {
        PyObject* p_module = TryGetModule(module_name);

        if (p_module != nullptr)
        {
            PyObject* new_module = PyImport_ReloadModule(p_module);
        }
    }

    template <class _dh>
    inline void PythonHandler<_dh>::ReloadModules()
    {
        for (const auto& [module_name, _] : m_loaded_modules)
        {
            ReloadModule(module_name);
        }
    }

    template <class _dh>
    inline void PythonHandler<_dh>::SetupEnvironment(std::string_view scripts_path)
    {
        Py_Initialize();

        if (Py_IsInitialized())
        {

            std::ostringstream oss;
            std::ostringstream oss_executable_path;
            oss << "sys.path.append(\"" << scripts_path << "\")";

            std::vector<std::string> additional_paths;
            if (m_additional_include_paths)
            {
                m_additional_include_paths(additional_paths);
                for (const auto& path : additional_paths)
                {
                    oss << "\nsys.path.append(\"" << path << "\")";
                }
            }

            PyRun_SimpleString("import sys");
            PyRun_SimpleString(oss.str().c_str());
            PyRun_SimpleString(oss_executable_path.str().c_str());

            pybind11::module::import(m_module_name.c_str());
        }
    }

    template <class _dh>
    inline void PythonHandler<_dh>::RecordError(std::string_view script_name)
    {
        if (Py_IsInitialized())
        {
            PyObject *p_type_o, *p_error_o, *p_traceback_o;
            PyErr_Fetch(&p_type_o, &p_error_o, &p_traceback_o);

            if (p_error_o)
            {
                ErrorObject object;
                PyObject* str_ext = PyObject_Str(p_error_o);
                object.error = PyUnicode_AsUTF8(str_ext);
                object.script_name = script_name;
                m_errors.push_back(object);
                Py_XDECREF(str_ext);
            }

            Py_XDECREF(p_type_o);
            Py_XDECREF(p_error_o);
            Py_XDECREF(p_traceback_o);
        }
    }

    template <class _dh>
    inline PyObject* PythonHandler<_dh>::TryGetModule(std::string_view module_name)
    {
        if (Py_IsInitialized())
        {
            PyObject* sys = PyImport_ImportModule("sys");
            PyObject* modulesDict = PyObject_GetAttrString(sys, "modules");
            PyObject* result = PyDict_GetItemString(modulesDict, module_name.data());

            Py_DECREF(sys);
            Py_DECREF(modulesDict);

            return result;
        }

        return nullptr;
    }

    template <class _dh>
    inline void PythonHandler<_dh>::CheckDoReload(std::string_view module_name)
    {
        if (auto it = m_loaded_modules.find(module_name.data()); it != m_loaded_modules.end())
        {
            time_t timestamp = it->second;
            std::string file_path = m_scripts_path + std::string("/") + std::string(module_name) + ".py";
            struct stat result;
            stat(file_path.c_str(), &result);
            time_t current_timestamp = result.st_mtime;

            if (current_timestamp > timestamp)
                ReloadModule(module_name);
        }
    }
}

#endif