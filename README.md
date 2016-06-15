# boost-multicast
The source is from http://www.boost.org/doc/libs/1_57_0/doc/html/boost_asio/example/cpp03/multicast

Build:
 (without linking boost_system)
 $ g++ -I $BOOST_ROOT/include -DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_ERROR_CODE_HEADER_ONLY sender.cpp -o sender

 (with linking boost_system
 $ g++ -I $BOOST_ROOT/include -lboost_system sender.cpp -o sender


