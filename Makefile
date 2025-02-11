.PHONY: test
test:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON -DCMAKE_PREFIX_PATH=$(HOME)/mQt
	cmake --build build --config Debug --parallel
	ctest --test-dir build/tests --rerun-failed --output-on-failure

.PHONY: cov
cov:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON -DENABLE_COVERAGE=ON -DCMAKE_PREFIX_PATH=$(HOME)/mQt
	cmake --build build --config Debug --target coverage --parallel
	ctest --test-dir build/tests --rerun-failed --output-on-failure

.PHONY: clean
clean:
	rm -rf build