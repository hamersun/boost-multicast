# boost-multicast
The source is from http://www.boost.org/doc/libs/1_57_0/doc/html/boost_asio/example/cpp03/multicast

Build:
 (without linking boost_system)
 $ g++ -I $BOOST_ROOT/include -DBOOST_ALL_NO_LIB -DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_ERROR_CODE_HEADER_ONLY sender.cpp -o sender
 $ g++ -I $BOOST_ROOT/include -DBOOST_ALL_NO_LIB -DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_ERROR_CODE_HEADER_ONLY receiver.cpp -o receiver

 (with linking boost_system
 $ g++ -I $BOOST_ROOT/include -lboost_system sender.cpp -o sender
 $ g++ -I $BOOST_ROOT/include -lboost_system receiver.cpp -o receiver

If android NDK has been installed and you want to build for android platform
 $ ndk-build

Then it would create sender and receiver executable files in libs directory.
