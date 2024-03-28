echo "-- BUILDING CPP..."
echo ""

cmake -S . -B build && cmake --build build
cmake --build build --target all

echo "-- BUILDING RUST..."
echo ""
cargo build