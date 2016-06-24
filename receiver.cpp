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

#include "packet_header.h"
#include "utils.h"

const short multicast_port = 30001;

class receiver {
public:
    receiver(boost::asio::io_service& io_service,
             const boost::asio::ip::address& listen_address,
             const boost::asio::ip::address& multicast_address)
        : socket_(io_service), mPacketCount(0), timer_(io_service), mTotalSegments(0), mTotalLostSegments(0) {
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
            boost::asio::buffer(data_, sizeof(data_)), sender_endpoint_,
            boost::bind(&receiver::handle_receive_from, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        timer_.expires_from_now(boost::posix_time::seconds(20));
        timer_.async_wait(boost::bind(&receiver::close, this));
    }

    void handle_receive_from(const boost::system::error_code& error,
                             size_t bytes_recvd) {
        static int current_chunk_size = 0;
        static bool lost_first_data = false;
        static int64_t packet_sn = 0;
        static int totalsegments = 0;
        static int lost_seg_count = 0;
        static int32_t prev_seg_sn = -1;
        static bool show_chunk = false;
        if (!error) {
            //timer_.cancel();
            packet_header *header = (packet_header*)data_;
            int seg_sn;
            if (bytes_recvd == 0) {
                if ((totalsegments - prev_seg_sn - 1) > 0) {
                    //std::cout << "lost final segments: add " << (totalsegments - prev_seg_sn - 1) << " lost" << std::endl;
                    lost_seg_count += (totalsegments - prev_seg_sn - 1);
                }
                show_chunk = true;
            } else {
                seg_sn = header->order;
                int seg_diff = seg_sn - prev_seg_sn;
                if (prev_seg_sn == -1 && seg_sn == 0) {
                    lost_first_data = false;
                } else if (prev_seg_sn == -1 && seg_sn != 0) {
                    lost_first_data = true;
                }
                if (seg_diff != 1) {
                    if (seg_diff > 0) lost_seg_count += (seg_diff-1);
                    else if (seg_diff < 0) {
                        //std::cout << "adnormal: prev: " << prev_seg_sn << ", cur: " << seg_sn << std::endl;
                        lost_seg_count += (totalsegments - prev_seg_sn - 1);
                        show_chunk = true;
                    }
                }
                prev_seg_sn = seg_sn;
                if (!show_chunk) totalsegments = header->segment_count;
            }
            if (show_chunk) {
                mTotalSegments += totalsegments;
                mTotalLostSegments += lost_seg_count;
                std::cout << "message: " << packet_sn << "----";
                if (lost_first_data) std::cout << "(lost first segment)";
                //std::cout.write(chunk_, current_chunk_size>32?32:current_chunk_size);
                std::cout << current_chunk_size;
                std::cout << "(lost rate: " << ((float)lost_seg_count)/totalsegments << "(" << lost_seg_count << "/" << totalsegments << ")";
                std::cout << std::endl;
                mPacketCount++;
                current_chunk_size = 0;
                show_chunk = false;
                prev_seg_sn = -1;
                lost_seg_count = 0;
            }
            if (bytes_recvd != 0 && current_chunk_size == 0) packet_sn = header->SN;
            memcpy(chunk_+current_chunk_size, data_, bytes_recvd);
            current_chunk_size += bytes_recvd;

            socket_.async_receive_from(
                boost::asio::buffer(data_, sizeof(data_)), sender_endpoint_,
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

    void close() {
        if (timer_.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
            std::cout << "time out, packet count: " << mPacketCount << std::endl;
            std::cout << "lost rate: " << ((float)mTotalLostSegments)/mTotalSegments << std::endl;
            std::cout << "close socket" << std::endl;
            socket_.close();
        }
    }

private:
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint sender_endpoint_;
    enum { max_length = 65536 };
    char data_[1400];
    char chunk_[max_length];
    boost::asio::deadline_timer timer_;
    int mPacketCount;
    int mTotalSegments;
    int mTotalLostSegments;
};

int main(int argc, char* argv[])
{
    try {
        if (argc != 3) {
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
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
