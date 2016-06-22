//
// sender.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <sstream>
#include <string>
#include <boost/asio.hpp>
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

const short multicast_port = 30001;
const int max_message_count = 120*1000/33;

class sender
{
public:
  sender(boost::asio::io_service& io_service,
      const boost::asio::ip::address& multicast_address,
      const int packet_size)
    : endpoint_(multicast_address, multicast_port),
      socket_(io_service, endpoint_.protocol()),
      timer_(io_service),
      message_count_(0),
      _packet_size(packet_size)
  {
    std::ostringstream os;
    os << "Message " << message_count_++;
    message_ = os.str();
//    std::ostream request_stream(&request_);
    message_buf = (char*)malloc(_packet_size);
    memset(message_buf, '-', _packet_size);
//    memcpy(message_buf, message_.c_str(), message_.length());
    message_.append(message_buf, _packet_size);
//    request_stream.write(message_buf, 200*1024);
//    boost::asio::streambuf::mutable_buffers_type mutableBuffer =
//              request_.prepare(200*1024);

    socket_.async_send_to(
        boost::asio::buffer(message_), endpoint_,
        boost::bind(&sender::handle_send_to, this,
          boost::asio::placeholders::error));
  }

  void handle_send_to(const boost::system::error_code& error)
  {
    if (!error && message_count_ < max_message_count)
    {
      timer_.expires_from_now(boost::posix_time::milliseconds(33));
      timer_.async_wait(
          boost::bind(&sender::handle_timeout, this,
            boost::asio::placeholders::error));
    }
    else {
      std::cerr << error.message() << std::endl;
    }
  }

  void handle_timeout(const boost::system::error_code& error)
  {
    if (!error)
    {
      std::ostringstream os;
      os << "Message " << message_count_++;
      message_ = os.str();
//      std::ostream request_stream(&request_);
//      memcpy(message_buf, message_.c_str(), message_.length());
//      request_stream.write(message_buf, 200*1024);
//      boost::asio::streambuf::mutable_buffers_type mutableBuffer =
//            request_.prepare(200*1024);
      message_.append(message_buf, _packet_size);

      socket_.async_send_to(
          boost::asio::buffer(message_), endpoint_,
          boost::bind(&sender::handle_send_to, this,
            boost::asio::placeholders::error));
    }
  }

private:
  boost::asio::ip::udp::endpoint endpoint_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::deadline_timer timer_;
  int message_count_;
  std::string message_;
  int _packet_size;
  char *message_buf;
//  boost::asio::streambuf request_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc > 3 || argc < 2)
    {
      std::cerr << "Usage: sender <multicast_address> [packet_size]\n";
      std::cerr << "  For IPv4, try:\n";
      std::cerr << "    sender 239.255.0.1 1024\n";
      std::cerr << "  For IPv6, try:\n";
      std::cerr << "    sender ff31::8000:1234 1024\n";
      return 1;
    }
    int packet_size = 1024;
    if (argc == 3) {
        packet_size = atoi(argv[2]);
    }

    boost::asio::io_service io_service;
    sender s(io_service, boost::asio::ip::address::from_string(argv[1]), packet_size);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
