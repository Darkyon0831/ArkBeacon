#ifndef __ARKBEACON_WEBSOCKETSERVERFACTORY_H__
#define __ARKBEACON_WEBSOCKETSERVERFACTORY_H__

#include "WebsocketServer.h"
#include "ParameterLoader.h"

namespace ArkBeacon
{
    class WebsocketServerFactory
    {
    public:

        struct WebsocketServerHolder
        {
            std::unique_ptr<WebsocketServer<ParameterLoader, std::string, no_tls>> no_tls_server = nullptr;
            std::unique_ptr<WebsocketServer<ParameterLoader, std::string, tls>> tls_server = nullptr;
        };
        
        template <typename _DataFetcher, typename _DataFetcher_Type, typename _ServerType>
        static std::unique_ptr<ArkBeacon::WebsocketServer<_DataFetcher, _DataFetcher_Type, _ServerType>> Create(int port)
        {
            return std::make_unique<ArkBeacon::WebsocketServer<_DataFetcher, _DataFetcher_Type, _ServerType>>(port);
        }

        template <typename _DataFetcher, typename _DataFetcher_Type>
        static std::unique_ptr<ArkBeacon::WebsocketServer<_DataFetcher, _DataFetcher_Type, no_tls>> CreateNoTLS(int port)
        {
            return Create<_DataFetcher, _DataFetcher_Type, no_tls>(port);
        }

        template <typename _DataFetcher, typename _DataFetcher_Type>
        static std::unique_ptr<ArkBeacon::WebsocketServer<_DataFetcher, _DataFetcher_Type, tls>> CreateTLS(int port)
        {
            return Create<_DataFetcher, _DataFetcher_Type, tls>(port);
        }

        template <typename _DataFetcher, typename _DataFetcher_Type>
        static std::function<void()> CreateRunFunction(
            bool use_tls, 
            int port, 
            WebsocketServerHolder& server_holder, 
            std::function<void(json&, _DataFetcher*, std::function<void(_DataFetcher*)>)> message_callback,
            bool clear_data_after_sent = false,
            std::function<void(json&, typename DataFetcher<_DataFetcher_Type>::ValueNamePair&)> output_parser_callback = nullptr,
            std::string ssl_chain_file = "", 
            std::string ssl_private_file = "")
        {
            return [use_tls, port, &server_holder, clear_data_after_sent, output_parser_callback, message_callback, ssl_chain_file, ssl_private_file]() {
                if (!use_tls)
                {
                    server_holder.no_tls_server = CreateNoTLS<ArkBeacon::ParameterLoader, std::string>(port);
                    server_holder.no_tls_server->SetClearAfterDataSent(clear_data_after_sent);
                    server_holder.no_tls_server->SetOutputParserCallback(output_parser_callback);
                    server_holder.no_tls_server->Init(message_callback);
                    server_holder.no_tls_server->Run();
                }
                else
                {
                    server_holder.tls_server = CreateTLS<ArkBeacon::ParameterLoader, std::string>(port);
                    server_holder.tls_server->SetupTLS(ArkBeacon::WebsocketServer<ArkBeacon::ParameterLoader, std::string, tls>::MOZILLA_MODERN, 
                                    ssl_chain_file, ssl_private_file);
                    server_holder.tls_server->SetClearAfterDataSent(clear_data_after_sent);
                    server_holder.tls_server->SetOutputParserCallback(output_parser_callback);
                    server_holder.tls_server->Init(message_callback);
                    server_holder.tls_server->Run();
                }
            };
        }

    private:
        WebsocketServerFactory() = default;
    };
}

#endif // __ARKBEACON_WEBSOCKETSERVERFACTORY_H__