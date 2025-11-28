#define ASIO_STANDALONE

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio.hpp>
#include <nlohmann/json.hpp>
#include <functional>
#include <string>

#include "DataFetcher.h"

using json = nlohmann::json;

typedef websocketpp::server<websocketpp::config::asio> no_tls;
typedef websocketpp::server<websocketpp::config::asio_tls> tls;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

#ifndef __ARKBEACON_WEBSOCKETSERVER_H__
#define __ARKBEACON_WEBSOCKETSERVER_H__

namespace ArkBeacon
{
    template<class _DataFetcher, typename _DataFetcher_Type, typename _ServerType>
    class WebsocketServer
    {
    public:
        static_assert(std::is_base_of<DataFetcher<_DataFetcher_Type>, _DataFetcher>::value, "_DataFetcher must be of base class DataFetcher<T> with _Datafetcher_Type of type T");
        static_assert(std::is_same<_ServerType, no_tls>::value || std::is_same<_ServerType, tls>::value, "_ServerType must be of value \"no_tls\" or \"tls\"");

        struct SSLFiles
        {
            std::string certificate_chain_file;
            std::string private_key_file;
        };

        enum TLSMode
        {
            MOZILLA_INTERMEDIATE = 1,
            MOZILLA_MODERN = 2
        };

        WebsocketServer(int port) 
            : m_port(port)
            , m_output_parser_callback(nullptr)
        { 
            m_handle.clear_access_channels(websocketpp::log::alevel::all);
            m_handle.set_access_channels(websocketpp::log::alevel::connect); 
        }

        void SetOutputParserCallback(std::function<void(json&, typename DataFetcher<_DataFetcher_Type>::ValueNamePair&)> output_parser_callback) { m_output_parser_callback = output_parser_callback; }
        void SetClearAfterDataSent(bool clear_after_sent) { m_clear_after_sent = clear_after_sent; }
        void Init(std::function<void(json&, _DataFetcher*, std::function<void(_DataFetcher*)>)> message_callback);
        void Run();
        void Stop();
        void SetupTLS(TLSMode tls_mode, std::string chain_file, std::string private_file);

        std::vector<websocketpp::connection_hdl> m_active_connections;

    private:
        std::function<void(json&, typename DataFetcher<_DataFetcher_Type>::ValueNamePair&)> m_output_parser_callback;

        SSLFiles m_ssl_files;

        context_ptr OnTSLInit(TLSMode tsl_mode, websocketpp::connection_hdl hdl);

        _ServerType m_handle;
        _DataFetcher m_data_fetcher;
        int m_port;
        bool m_clear_after_sent;
        bool m_is_tls;
    };

    template <class _DataFetcher, typename _DataFetcher_Type, typename _ServerType>
    inline void WebsocketServer<_DataFetcher, _DataFetcher_Type, _ServerType>::Init(std::function<void(json&, _DataFetcher*, std::function<void(_DataFetcher*)>)> message_callback)
    {
        m_handle.init_asio();

        m_handle.set_open_handler([&](websocketpp::connection_hdl hdl) {
            m_active_connections.push_back(hdl);
        });

        m_handle.set_close_handler([&](websocketpp::connection_hdl hdl) {
            int index = -1;

            for (int i = 0; i < m_active_connections.size(); i++)
            {
                if (m_active_connections[i].lock() == hdl.lock())
                {
                    index = i;
                    break;
                }
            }

            if (index != -1)
            {
                m_active_connections.erase(m_active_connections.begin() + index);
            }
        });

        m_handle.set_message_handler([&](websocketpp::connection_hdl hdl, std::shared_ptr<websocketpp::config::core::message_type> msg) {
            try
            {
                json obj = json::parse(msg->get_payload());

                message_callback(obj, &m_data_fetcher, [&, msg, hdl](DataFetcher<_DataFetcher_Type>* _fetcher) {
                    DataFetcher<_DataFetcher_Type>* fetcher = dynamic_cast<DataFetcher<_DataFetcher_Type>*>(_fetcher);

                    json data;
                    data["error"] = 0;
                    data["data"] = json::array();

                    for (int i = 0; i < fetcher->GetLen(); i++)
                    {
                        typename DataFetcher<_DataFetcher_Type>::ValueNamePair pair = fetcher->GetData(i);
                        json obj = json::object();

                        if (m_output_parser_callback != nullptr)
                            m_output_parser_callback(obj, pair);
                        else
                        {
                            obj["value"] = *pair.value;
                            obj["name"] = *pair.name;
                        }

                        data["data"].push_back(obj);
                    }

                    if (m_clear_after_sent)
                    {
                        fetcher->Clear();
                        fetcher->ClearArgs();
                    }

                    m_handle.send(hdl, data.dump(), msg->get_opcode());
                });
            }
            catch(const json::exception& e)
            {
                json data;
                data["error"] = 1;
                data["error_msg"] = e.what();

                m_handle.send(hdl, data.dump(), msg->get_opcode());
            }
        });
    }

    template <class _DataFetcher, typename _DataFetcher_Type, typename _ServerType>
    inline void WebsocketServer<_DataFetcher, _DataFetcher_Type, _ServerType>::Run()
    {
        m_handle.listen(m_port);
        m_handle.start_accept();
        m_handle.run();
    }

    template <class _DataFetcher, typename _DataFetcher_Type, typename _ServerType>
    inline void WebsocketServer<_DataFetcher, _DataFetcher_Type, _ServerType>::Stop()
    {
        m_handle.stop_listening();
        for (auto hdl : m_active_connections) {
            m_handle.close(hdl, websocketpp::close::status::going_away, "Server shutting down");
        }
        m_handle.stop();
    }

    template <class _DataFetcher, typename _DataFetcher_Type, typename _ServerType>
    inline void WebsocketServer<_DataFetcher, _DataFetcher_Type, _ServerType>::SetupTLS(TLSMode tls_mode, std::string chain_file, std::string private_file)
    {
        m_is_tls = true;

        m_handle.set_tls_init_handler([&](websocketpp::connection_hdl hdl){
            return OnTSLInit(tls_mode, hdl);
        });

        m_ssl_files.certificate_chain_file = chain_file;
        m_ssl_files.private_key_file = private_file;
    }

    template <class _DataFetcher, typename _DataFetcher_Type, typename _ServerType>
    inline context_ptr WebsocketServer<_DataFetcher, _DataFetcher_Type, _ServerType>::OnTSLInit(TLSMode tsl_mode, websocketpp::connection_hdl hdl)
    {
        context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

        try {
            if (tsl_mode == MOZILLA_MODERN) {
                // Modern disables TLSv1
                ctx->set_options(asio::ssl::context::default_workarounds |
                                asio::ssl::context::no_sslv2 |
                                asio::ssl::context::no_sslv3 |
                                asio::ssl::context::no_tlsv1 |
                                asio::ssl::context::single_dh_use);
            } else {
                ctx->set_options(asio::ssl::context::default_workarounds |
                                asio::ssl::context::no_sslv2 |
                                asio::ssl::context::single_dh_use);
            }
            ctx->use_certificate_chain_file(m_ssl_files.certificate_chain_file);
            ctx->use_private_key_file(m_ssl_files.private_key_file, asio::ssl::context::file_format::pem);
        } catch (std::exception& e) {
            std::cout << "Exception: " << e.what() << std::endl;
        }

        return ctx;
    }
}

#endif