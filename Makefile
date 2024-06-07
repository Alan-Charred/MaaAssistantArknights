MAACore-macos-x86_64:
	cmake -B build -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES='x86_64' && \
	cmake --build build && \
	cmake --install build --prefix .
