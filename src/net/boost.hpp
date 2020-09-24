#ifndef SERVER_NET_BOOST_HPP_
#define SERVER_NET_BOOST_HPP_

#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;

#endif // SERVER_NET_BOOST_HPP_