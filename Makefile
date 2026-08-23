include config.mk

TEST_BINARY := test_orderbook
TEST_SOURCES := tests/book/test_orderbook.cpp src/book/orderbook.cpp \
	src/market/event.cpp

.PHONY: all test sanitize clean

all: $(TEST_BINARY)

$(TEST_BINARY): $(TEST_SOURCES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(TEST_SOURCES) -o $@ $(LDFLAGS)

test: $(TEST_BINARY)
	./$(TEST_BINARY)

sanitize:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -O1 -g \
		-fsanitize=address,undefined -fno-omit-frame-pointer \
		$(TEST_SOURCES) -o $(TEST_BINARY)_sanitized $(LDFLAGS)
	ASAN_OPTIONS=detect_leaks=0 ./$(TEST_BINARY)_sanitized

clean:
	rm -f $(TEST_BINARY) $(TEST_BINARY)_sanitized
