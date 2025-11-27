/*
    Copyright (c) 2013-2014 Contributors as noted in the AUTHORS file

    This file is part of azmq

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#include <azmq/actor.hpp>
#include <azmq/util/scope_guard.hpp>


#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>

#include <array>
#include <thread>
#include <iostream>
#include <future>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

std::array<boost::asio::const_buffer, 2> snd_bufs = {{
    boost::asio::buffer("A"),
    boost::asio::buffer("B")
}};

std::string subj(const char* name) {
    return std::string("inproc://") + name;
}

TEST_CASE( "Async Send/Receive", "[actor]" ) {
    boost::system::error_code ecc;
    std::promise<size_t> btc_promise;
    auto btc = btc_promise.get_future();

    boost::system::error_code ecb;
    std::promise<size_t> btb_promise;
    auto btb = btb_promise.get_future();
    {
        std::array<char, 2> a;
        std::array<char, 2> b;

        std::array<boost::asio::mutable_buffer, 2> rcv_bufs = {{
            boost::asio::buffer(a),
            boost::asio::buffer(b)
        }};

        boost::asio::io_context io;
        auto s = azmq::actor::spawn(io, [&](azmq::socket & ss) {
            ss.async_receive(rcv_bufs, [&](boost::system::error_code const& ec, size_t bytes_transferred) {
                ecb = ec;
                btb_promise.set_value(bytes_transferred);
                io.stop();
            });
            ss.get_io_context().run();
        });

        s.async_send(snd_bufs, [&] (boost::system::error_code const& ec, size_t bytes_transferred) {
            ecc = ec;
            btc_promise.set_value(bytes_transferred);
        });

        auto work = boost::asio::make_work_guard(io);
        io.run();
    }
    std::cout << "IO context run complete" << std::endl;

    REQUIRE(btb.wait_for(std::chrono::seconds(1)) == std::future_status::ready);
    REQUIRE(btc.wait_for(std::chrono::seconds(1)) == std::future_status::ready);

    REQUIRE(ecc == boost::system::error_code());
    REQUIRE(btc.get() == 4);
    REQUIRE(ecb == boost::system::error_code());
    REQUIRE(btb.get() == 4);
}
