#!/bin/bash
set -e

BUILD_DIR="build"
DOC_DIR="doc"
PROJECT_NAME="CanBusManager"
CORES=$(nproc 2>/dev/null || echo 4)

CLEAN_BUILD=false
RUN_TESTS=false
GEN_DOCS=false
ENABLE_COV=false
BUILD_TYPE="Release"
GENERATOR=""
TEST_ONLY=false

if grep -q Microsoft /proc/version; then
    export DISPLAY=:0
fi

usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -c,   --clean       Remove build directory before starting"
    echo "  -d,   --dev-mode    Dev Mode: Use Ninja generator and Debug build"
    echo "  -t,   --tests       Enable building and running tests"
    echo "  -doc, --docs        Enable generating documentation"
    echo "  -cov, --coverage    Enable Coverage (forces Debug mode, implies -t)"
    echo "  -to,  --test-only   Test-Only Mode: Tests + Coverage, no docs, no app launch"
    echo "  -h,   --help        Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                  Build app only (no tests, no docs)"
    echo "  $0 -t               Build app with tests"
    echo "  $0 -t -d            Build app with tests and docs"
    echo "  $0 -to              Quick test iteration (tests + coverage only)"
    echo "  $0 -dev -t          Dev mode with tests"
    exit 0
}

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -c|--clean)      CLEAN_BUILD=true ;;
        -d|--dev-mode)   BUILD_TYPE="Debug"; GENERATOR="-G Ninja" ;;
        -t|--tests)      RUN_TESTS=true ;;
        -doc|--docs)     GEN_DOCS=true ;;
        -cov|--coverage) ENABLE_COV=true; RUN_TESTS=true; BUILD_TYPE="Debug"; GENERATOR="-G Ninja" ;;
        -to|--test-only) TEST_ONLY=true; RUN_TESTS=true; GEN_DOCS=false; ENABLE_COV=true; BUILD_TYPE="Debug"; GENERATOR="-G Ninja" ;;
        -h|--help)       usage ;;
        *) echo "Unknown option: $1"; usage ;;
    esac
    shift
done

if [ "$CLEAN_BUILD" = true ]; then
    echo "--- Cleaning Build Directory ---"
    if [ -d "$BUILD_DIR" ]; then
            echo "Removing $BUILD_DIR..."
            rm -rf "$BUILD_DIR"
    fi

    if [ -d "$DOC_DIR" ]; then
        echo "Cleaning $DOC_DIR (preserving res/, CMakeLists.txt and Doxyfile.in)..."
        find "$DOC_DIR" -mindepth 1 -maxdepth 1 \
                    ! -name 'CMakeLists.txt' \
                    ! -name 'Doxyfile.in' \
                    ! -name 'res' \
                    -exec rm -rf {} +
    fi
fi

echo "--- 1. Submodules ---"
git submodule update --init --recursive

echo "--- 2. Configuring Project & Format ---"
TEST_OPTS="-DENABLE_TESTS=$( [ "$RUN_TESTS" = true ] && echo "ON" || echo "OFF" )"
DOC_OPTS="-DENABLE_DOCS=$( [ "$GEN_DOCS" = true ] && echo "ON" || echo "OFF" )"
COV_OPTS="-DENABLE_COVERAGE=$( [ "$ENABLE_COV" = true ] && echo "ON" || echo "OFF" )"

cmake -S . -B $BUILD_DIR \
      $GENERATOR \
      -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      $TEST_OPTS \
      $DOC_OPTS \
      $COV_OPTS \
      -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6

if command -v clang-format &> /dev/null; then
     cmake --build build --target format
fi

echo "--- 3. Building Project ---"
cmake --build $BUILD_DIR --parallel $CORES

if [ "$GEN_DOCS" = true ] && command -v doxygen &> /dev/null; then
    echo "--- 4. Generating Documentation ---"
    cmake --build $BUILD_DIR --target docs
else
    echo "--- 4. Skipping Documentation ---"
fi

if [ "$RUN_TESTS" = true ]; then
    echo "--- 5. Running Tests ---"
    (cd $BUILD_DIR && ctest --output-on-failure)
    if [ "$ENABLE_COV" = true ]; then
            echo "--- Generating Coverage Report ---"
            cmake --build $BUILD_DIR --target coverage_report
            echo "Coverage report: $BUILD_DIR/coverage_html/index.html"
        fi
else
    echo "--- 5. Skipping Tests ---"
fi

if [ "$TEST_ONLY" = true ]; then
    echo "--- Test-only mode: Skipping app launch ---"
    echo "Tests complete! Coverage report at: $BUILD_DIR/coverage_html/index.html"
    exit 0
fi

echo "--- 6. Launching App ---"
APP_PATH="./$BUILD_DIR/$PROJECT_NAME"
if [ -f "$APP_PATH" ]; then
    echo "--- Launching $PROJECT_NAME ($BUILD_TYPE) ---"
    "$APP_PATH"
else
    echo "Error: Binary not found at $APP_PATH"
    exit 1
fi