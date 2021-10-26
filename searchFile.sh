echo "Where are we?"
echo "$(pwd)"

echo "Contents:"
ls -a

echo "Looking for libprotobufd.a"
find . -iname libprotobufd.a

cd build/_deps/grpc-build/third_party/protobuf
echo "Contents of $(pwd):"
ls -a
