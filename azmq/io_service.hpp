#pragma once

#include <boost/version.hpp>

#if BOOST_VERSION >= 108700
    #include <boost/asio/io_context.hpp>
    namespace boost::asio {
        typedef io_context io_service;   
    }
#else
    #include <boost/asio/io_service.hpp>
#endif
