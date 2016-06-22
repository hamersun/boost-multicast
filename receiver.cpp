//
// receiver.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "boost/bind.hpp"
#include <boost/date_time/posix_time/posix_time_types.hpp>

const short multicast_port = 30001;

class receiver
{
public:
  receiver(boost::asio::io_service& io_service,
      const boost::asio::ip::address& listen_address,
      const boost::asio::ip::address& multicast_address)
    : socket_(io_service), mPacketCount(0), timer_(io_service)
  {
    // Create the socket so that multiple may be bound to the same address.
    boost::asio::ip::udp::endpoint listen_endpoint(
        listen_address, multicast_port);
    socket_.open(listen_endpoint.protocol());
    socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket_.bind(listen_endpoint);

    // Join the multicast group.
    socket_.set_option(
        boost::asio::ip::multicast::join_group(multicast_address));

    socket_.async_receive_from(
        boost::asio::buffer(data_, max_length), sender_endpoint_,
        boost::bind(&receiver::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
    timer_.expires_from_now(boost::posix_time::seconds(20));
    timer_.async_wait(boost::bind(&receiver::close, this));
  }

  void handle_receive_from(const boost::system::error_code& error,
      size_t bytes_recvd)
  {
    if (!error)
    {
      //timer_.cancel();
      std::cout.write(data_, bytes_recvd>32?32:bytes_recvd);
      std::cout << bytes_recvd;
      std::cout << std::endl;
      mPacketCount++;

      socket_.async_receive_from(
          boost::asio::buffer(data_, max_length), sender_endpoint_,
          boost::bind(&receiver::handle_receive_from, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
      timer_.expires_from_now(boost::posix_time::seconds(20));
      timer_.async_wait(boost::bind(&receiver::close, this));
    } else {
        std::cerr << error.message() << std::endl;
        std::cerr << "packet count: " << mPacketCount << std::endl;
    }
  }

  void close()
  {
      if (timer_.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
          std::cout << "time out, packet count: " << mPacketCount << std::endl;
          std::cout << "close socket" << std::endl;
          socket_.close();
      }
  }

private:
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint sender_endpoint_;
  enum { max_length = 65536 };
  char data_[max_length];
  boost::asio::deadline_timer timer_;
  int mPacketCount;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: receiver <listen_address> <multicast_address>\n";
      std::cerr << "  For IPv4, try:\n";
      std::cerr << "    receiver 0.0.0.0 239.255.0.1\n";
      std::cerr << "  For IPv6, try:\n";
      std::cerr << "    receiver 0::0 ff31::8000:1234\n";
      return 1;
    }

    boost::asio::io_service io_service;
    receiver r(io_service,
        boost::asio::ip::address::from_string(argv[1]),
        boost::asio::ip::address::from_string(argv[2]));
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
