#pragma once

#include <boost/version.hpp>

#if BOOST_VERSION < 106600
    #include <boost/asio/io_service.hpp>
    namespace boost::asio {
        typedef io_service io_context;   
    }

    #define AZMQ_DETAIL_GET_IO_CONTEXT get_io_service
    #define AZMQ_DETAIL_USE_IO_SERVICE 1
#else
    #include <boost/asio/io_context.hpp>
    #define AZMQ_DETAIL_GET_IO_CONTEXT get_io_context
#endif
