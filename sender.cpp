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

#include "packet_header.h"
#include "utils.h"

const short multicast_port = 30001;
int max_message_count = 120*1000/33;

class sender {
public:
    sender(boost::asio::io_service& io_service,
           const boost::asio::ip::address& multicast_address,
           const int packet_size)
        : endpoint_(multicast_address, multicast_port),
          socket_(io_service, endpoint_.protocol()),
          timer_(io_service),
          message_count_(0),
          sent_count_(0),
          _packet_size(packet_size)
    {
//        std::ostringstream os;
//        os << "Message " << message_count_++;
//        message_ = os.str();
        message_buf = (char*)malloc(_packet_size);
        memset(message_buf, '-', _packet_size);
        //memcpy(message_buf, message_.c_str(), message_.length());

	    segment_send(message_buf, _packet_size);

        socket_.async_send_to(
            boost::asio::buffer(message_buf, 0), endpoint_,
            boost::bind(&sender::handle_send_to, this,
                        boost::asio::placeholders::error));
    }

    void handle_send_to(const boost::system::error_code& error)
    {
        if (!error && message_count_ < max_message_count) {
            timer_.expires_from_now(boost::posix_time::milliseconds(33));
            timer_.async_wait(
                boost::bind(&sender::handle_timeout, this,
                            boost::asio::placeholders::error));
        } else {
            std::cerr << error.message() << " " << sent_count_ << " segments have been sent" << std::endl;
            if (!error) {
                boost::system::error_code error2;
                char end[1];
                end[0] = '\0';
                socket_.send_to(boost::asio::buffer(end, sizeof(end)), endpoint_, 0, error2);
                //timer_.expires_from_now(boost::posix_time::seconds(20));
                //timer_.async_wait(boost::bind(&sender::close, this, boost::asio::placeholders::error));
            }
        }
    }

    void close(const boost::system::error_code& error)
    {
        std::cout << "close..." << std::endl;
    }

    void handle_timeout(const boost::system::error_code& error)
    {
        if (!error) {
            //std::ostringstream os;
            //os << "Message " << message_count_++;
            //message_ = os.str();
            //memcpy(message_buf, message_.c_str(), message_.length());

            segment_send(message_buf, _packet_size);

            socket_.async_send_to(
                boost::asio::buffer(message_buf, 0), endpoint_,
                boost::bind(&sender::handle_send_to, this,
                            boost::asio::placeholders::error));
        }
    }

    int segment_send(void *chunk, const size_t chunk_size)
    {
        const size_t SEGMENT_SIZE = 1400;
        boost::system::error_code error;
        char *pSegment = (char*)chunk;
        size_t segment_size = chunk_size > SEGMENT_SIZE ? SEGMENT_SIZE : chunk_size;
	    size_t totalsegments = chunk_size/SEGMENT_SIZE;
	    if ((chunk_size % SEGMENT_SIZE) != 0) totalsegments += 1;
        if (chunk_size <= segment_size) totalsegments = 1;
	    uint8_t segment_sn = 0;
        int remainder_size = chunk_size;
        packet_header header = {0};
        do {
	        //snprintf(pSegment, segment_size, "%lld%c%c", (int64_t)message_count_, (uint8_t)totalsegments, (uint8_t)segment_sn);
            header.SN = (int64_t)message_count_;
            header.segment_count = (uint8_t)totalsegments;
            header.order = (uint8_t)segment_sn;
            memcpy(pSegment, &header, sizeof(header));
            socket_.send_to(boost::asio::buffer(pSegment, segment_size), endpoint_, 0, error);
            if (error) {
                std::cerr << error.message() << std::endl;
                return error.value();
            }
            ++sent_count_;
	        ++segment_sn;
            pSegment += SEGMENT_SIZE;
            remainder_size -= SEGMENT_SIZE;
            if (remainder_size > 0 && remainder_size < SEGMENT_SIZE) segment_size = remainder_size;
            else if (remainder_size < 0) {
//                segment_size += (remainder_size);
                remainder_size = 0;
            }
        } while(remainder_size>0);
        ++message_count_;
        return 0;
    }

private:
    boost::asio::ip::udp::endpoint endpoint_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::deadline_timer timer_;
    int message_count_;
    std::string message_;
    int _packet_size;
    char *message_buf;
    int sent_count_;
};

int main(int argc, char* argv[])
{
    try {
        if (argc > 4 || argc < 2) {
            std::cerr << "Usage: sender <multicast_address> [packet_size] [max_packet_count]\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    sender 239.255.0.1 1024 1000\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    sender ff31::8000:1234 1024 1000\n";
            return 1;
        }
        int packet_size = 1024;
        if (argc >= 3) {
            packet_size = atoi(argv[2]);
        }
        if (argc >= 4) {
            max_message_count = atoi(argv[3]);
        }

        boost::asio::io_service io_service;
        sender s(io_service, boost::asio::ip::address::from_string(argv[1]), packet_size);
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
