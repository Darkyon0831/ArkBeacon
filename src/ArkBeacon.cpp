#include <unordered_map>
#include <format>
#include <signal.h>
#include <thread>
#include <string>
#include <filesystem>

#include "WebsocketServer.h"
#include "PythonHandler.h"
#include "ParameterLoader.h"
#include "WebsocketServerFactory.h"
#include "ArgumentProcessor.h"
#include "InputHandler.h"
#include "CallQueue.h"
#include "ConfigLoader.h"

std::shared_ptr<ArkBeacon::PythonHandler<ArkBeacon::ParameterLoader>> m_python_handler;
std::unique_ptr<std::thread> m_command_line_io_thread;
std::unique_ptr<std::thread> m_python_call_thread;

ArkBeacon::CallQueue<ArkBeacon::PythonHandler<ArkBeacon::ParameterLoader>> m_call_queue;

ArkBeacon::WebsocketServerFactory::WebsocketServerHolder m_server_holder;
ArkBeacon::ArgumentProcessor m_argument_processor;

ArkBeacon::InputHandler m_input_handler;

std::string m_executable_path = std::filesystem::canonical("/proc/self/exe").parent_path().string();
std::string m_script_path = m_executable_path + "/Scripts";

bool m_running = true;
bool m_use_tls = false;

void Message_Callback(json& j_obj, ArkBeacon::ParameterLoader* parameter_loader, std::function<void(ArkBeacon::ParameterLoader*)> after)
{
    std::string script_name = j_obj["script_name"];

    if (j_obj.contains("args"))
        parameter_loader->HandleArgs(j_obj["args"]);

    m_call_queue.Push({
        .call = [script_name, parameter_loader, after](std::shared_ptr<ArkBeacon::PythonHandler<ArkBeacon::ParameterLoader>> handler) {
            handler->RunScript(parameter_loader, script_name);
            after(parameter_loader);
        }
    });
}

void StopServer()
{
    std::cout << "Stopping server and exiting..." << std::endl;
    m_input_handler.StopInput(); 

    if (m_use_tls)
        m_server_holder.tls_server->Stop();
    else
        m_server_holder.no_tls_server->Stop();
}

void Defines()
{
    m_argument_processor.AddArgumentDefiner("ScriptsPath", {"ScriptPath"});
    m_argument_processor.AddArgumentDefiner("Port", {"Port"});
    m_argument_processor.AddArgumentDefiner("UseTLS", {"SSLChainFile", "SSLPrivateKeyFile"});
    m_argument_processor.AddArgumentDefiner("AutoReloadScripts", {"true_or_false"});

    m_argument_processor.AddDefaultArgument("ScriptsPath", {m_script_path});
    m_argument_processor.AddDefaultArgument("Port", {"9002"});

    m_input_handler.AddCommand({
        .action = [&]() { 
            StopServer(); 
        },
        .name = "exit",
        .description = "Stop the server and exit the application."
    });

    m_input_handler.AddCommand({
        .action = [&]() {
            if (m_python_handler)
            {
                std::cout << "Reloading Python modules..." << std::endl;
                
                m_call_queue.Push({
                    .call = [](std::shared_ptr<ArkBeacon::PythonHandler<ArkBeacon::ParameterLoader>> handler) {
                        handler->ReloadModules();
                        std::cout << "Modules reloaded." << std::endl;
                    }
                });


            }
        },
        .name = "reload",
        .description = "Reload Python modules."
    });

    m_input_handler.AddCommand({
        .action = [&]() { m_input_handler.PrintDescription(); },
        .name = "help",
        .description = "Print command descriptions."
    });
}

void PythonCallThread()
{
    ArkBeacon::CallQueue<ArkBeacon::PythonHandler<ArkBeacon::ParameterLoader>>::CallRequest request;

    while (m_call_queue.Pop(request))
    {
        if (request.call)
            request.call(m_python_handler);
        else
        {
            #ifdef DEBUG
            std::cerr << "Call request is empty." << std::endl;
            #endif
        }
    }
}

void BeforeInputCallback()
{
    const std::string& port = m_argument_processor.GetArgument("Port")->values.at(0);

    if (m_use_tls)
        std::cout << "ArkBeacon TLS Server is running with port " << port << ". Type 'help' to view commands." << std::endl;
    else
        std::cout << "ArkBeacon Server is running with port " << port << ". Type 'help' to view commands." << std::endl;
}

void AdditionalPathsCallback(std::vector<std::string>& additional_paths)
{
    additional_paths.push_back(m_executable_path);
}

void OnSIGINT(int signal_code) 
{
    StopServer();
}

int main(int argc, char* argv[])
{
    Defines();
    signal(SIGINT, OnSIGINT);

    m_argument_processor.HandleArguments(argc, argv);

    int port = std::stoi(std::string(m_argument_processor.GetArgument("Port")->values.at(0)));
    const ArkBeacon::ArgumentProcessor::Argument* use_tls = m_argument_processor.GetArgument("UseTLS");

    m_python_handler = std::make_unique<ArkBeacon::PythonHandler<ArkBeacon::ParameterLoader>>(m_argument_processor.GetArgument("ScriptsPath")->values.at(0), "ParameterLoader");
    m_python_handler->SetAdditionalIncludePaths(AdditionalPathsCallback);

    if (m_argument_processor.GetArgument("AutoReloadScripts") != nullptr)
        m_python_handler->SetAutoReloadModule();

    if (use_tls != nullptr)
        m_use_tls = true;

    m_call_queue.SetStartCallback([]() {
        m_call_queue.Push({
            .call = [](std::shared_ptr<ArkBeacon::PythonHandler<ArkBeacon::ParameterLoader>> handler) {
                handler->Init();
            }
        });
    });

    m_call_queue.SetStopCallback([]() {
        m_call_queue.Push({
            .call = [](std::shared_ptr<ArkBeacon::PythonHandler<ArkBeacon::ParameterLoader>> handler) {
                handler->Finalize();
            }
        });
    });

    m_call_queue.Init(m_python_handler);
    m_input_handler.SetBeforeInputCallback(BeforeInputCallback);
    m_command_line_io_thread = std::make_unique<std::thread>(&ArkBeacon::InputHandler::StartInputLoop, &m_input_handler);
    m_python_call_thread = std::make_unique<std::thread>(PythonCallThread);

    auto run = ArkBeacon::WebsocketServerFactory::CreateRunFunction<ArkBeacon::ParameterLoader, std::string>(
        m_use_tls,
        port,
        m_server_holder,
        [&](json& j_obj, ArkBeacon::ParameterLoader* parameter_loader, std::function<void(ArkBeacon::ParameterLoader*)> after) { Message_Callback(j_obj, parameter_loader, after); },
        true,
        nullptr,
        m_use_tls ? use_tls->values.at(0) : "",
        m_use_tls ? use_tls->values.at(1) : ""
    );

    run();
 
    m_command_line_io_thread->join();
    m_call_queue.Stop();

    m_python_call_thread->join();

    return 0;
} 