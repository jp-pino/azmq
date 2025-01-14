/*
    Copyright (c) 2013-2014 Contributors as noted in the AUTHORS file

    This file is part of azmq

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef AZMQ_DETAIL_SEND_OP_HPP_
#define AZMQ_DETAIL_SEND_OP_HPP_
#include "../error.hpp"
#include "../io_service.hpp"
#include "../message.hpp"
#include "socket_ops.hpp"
#include "reactor_op.hpp"

#include <zmq.h>
#include <iterator>

namespace azmq {
namespace detail {

template<typename ConstBufferSequence>
class send_buffer_op_base : public reactor_op {
public:
    send_buffer_op_base(ConstBufferSequence const& buffers, flags_type flags)
        : buffers_(buffers)
        , flags_(flags)
        { }

    virtual bool do_perform(socket_type& socket) override {
        ec_ = boost::system::error_code();
        bytes_transferred_ += socket_ops::send(buffers_, socket, flags_ | ZMQ_DONTWAIT, ec_);
        if (ec_) {
            return !try_again();
        }
        return true;
    }

private:
    ConstBufferSequence buffers_;
    flags_type flags_;
};

template<typename ConstBufferSequence,
         typename Handler>
class send_buffer_op : public send_buffer_op_base<ConstBufferSequence> {
public:
    send_buffer_op(ConstBufferSequence const& buffers,
                   Handler handler,
                   reactor_op::flags_type flags)
        : send_buffer_op_base<ConstBufferSequence>(buffers, flags)
        , handler_(std::move(handler))
    { }

    virtual void do_complete() override {
        handler_(this->ec_, this->bytes_transferred_);
    }

private:
    Handler handler_;
};

class send_op_base : public reactor_op {
public:
    send_op_base(message msg, flags_type flags)
        : msg_(std::move(msg))
        , flags_(flags)
        { }

    virtual bool do_perform(socket_type & socket) override {
        ec_ = boost::system::error_code();
        bytes_transferred_ = socket_ops::send(msg_, socket, flags_ | ZMQ_DONTWAIT, ec_);
        if (ec_)
            return !try_again(); // some other error
        return true;
    };

private:
    message msg_;
    flags_type flags_;
};

template<typename Handler>
class send_op : public send_op_base {
public:
    send_op(message msg,
            Handler handler,
            flags_type flags)
        : send_op_base(std::move(msg), flags)
        , handler_(std::move(handler))
    { }

    virtual void do_complete() override {
        handler_(ec_, bytes_transferred_);
    }

private:
    Handler handler_;
};

} // namespace detail
} // namespace azmq
#endif // AZMQ_DETAIL_SEND_OP_HPP_
