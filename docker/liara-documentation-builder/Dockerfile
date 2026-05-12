# ============================================================================
# Liara Engine Documentation Builder
# ============================================================================
#
# Tools:
# - Doxygen 1.16.1
# - mdBook 0.5.2
#
# Usage:
# docker buildx build -t liara-docs-builder .
#
# docker run --rm \
#   -v $(pwd):/src \
#   -v $(pwd)/generated-docs:/docs \
#   liara-docs-builder
#
# ============================================================================
#
# This dockerfile is used to build the documentation for Liara Engine.
#
# This dockerfile also downloads the docs-shared repository, which contains shared assets for the documentation, such as CSS and JavaScript files,
# and copies them to the appropriate locations in the generated documentation.
#
# To work as intended, you need to mount the source code of the engine to /src in the container.
# This folder need to contain the Doxyfile and book.toml files, which are used by doxygen and mdbook respectively to generate the documentation.
# The generated documentation will be output to the /docs folder in the container, which can be mounted to a local folder to access the generated documentation.
#
# ============================================================================

# ============================================================================
# STAGE 1 — Downloader
# ============================================================================

FROM debian:bookworm-slim AS downloader

ARG DOXYGEN_VERSION=1.16.1
ARG MDBOOK_VERSION=0.5.2
ARG DOCS_SHARED_VERSION=0.0.0
ARG DOCS_SHARED_REPO=https://github.com/liara-engine/docs-shared

WORKDIR /tmp

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    git \
    tar \
    && rm -rf /var/lib/apt/lists/*

# ----------------------------------------------------------------------------
# Download Doxygen
# ----------------------------------------------------------------------------

RUN curl -fsSL \
    "https://www.doxygen.nl/files/doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz" \
    -o doxygen.tar.gz \
    && tar -xzf doxygen.tar.gz \
    && mv "doxygen-${DOXYGEN_VERSION}" /opt/doxygen \
    && rm -f doxygen.tar.gz

# ----------------------------------------------------------------------------
# Download mdBook
# ----------------------------------------------------------------------------

RUN curl -fsSL \
    "https://github.com/rust-lang/mdBook/releases/download/v${MDBOOK_VERSION}/mdbook-v${MDBOOK_VERSION}-x86_64-unknown-linux-gnu.tar.gz" \
    -o mdbook.tar.gz \
    && tar -xzf mdbook.tar.gz \
    && chmod +x mdbook \
    && rm mdbook.tar.gz

# ----------------------------------------------------------------------------
# Download docs-shared
# ----------------------------------------------------------------------------

RUN git clone \
    --depth 1 \
    --branch "v${DOCS_SHARED_VERSION}" \
    "${DOCS_SHARED_REPO}" \
    /docs-shared



# ============================================================================
# STAGE 2 — Runtime
# ============================================================================


FROM debian:bookworm-slim

LABEL org.opencontainers.image.title="Liara Documentation Builder"
LABEL org.opencontainers.image.description="Ultra-light CI documentation builder"
LABEL org.opencontainers.image.licenses="MIT"

ENV SRC_DIR=/src
ENV OUTPUT_DIR=/docs
ENV MDBOOK_SOURCE_DIR=/src/docs
ENV DOXYGEN_GEN_DIR=/src/build/doxygen

WORKDIR /

# ----------------------------------------------------------------------------
# Runtime packages only
# ----------------------------------------------------------------------------


RUN apt-get update && apt-get install -y --no-install-recommends \
    bash \
    graphviz \
    findutils \
    coreutils \
    && rm -rf /var/lib/apt/lists/*

# ----------------------------------------------------------------------------
# Copy binaries and assets
# ----------------------------------------------------------------------------

COPY --from=downloader /opt/doxygen /opt/doxygen
COPY --from=downloader /tmp/mdbook /usr/local/bin/mdbook
COPY --from=downloader /docs-shared /opt/docs-shared

RUN ln -s /opt/doxygen/bin/doxygen /usr/local/bin/doxygen \
    && chmod +x /usr/local/bin/mdbook

WORKDIR ${SRC_DIR}

# ----------------------------------------------------------------------------
# Embedded build script
# ----------------------------------------------------------------------------

RUN cat <<'EOF' > /usr/local/bin/build-docs
#!/usr/bin/env bash

set -Eeuo pipefail

BOOK_DIR="${MDBOOK_SOURCE_DIR}"
SUMMARY_FILE="${BOOK_DIR}/SUMMARY.md"

# ----------------------------------------------------------------------------
# Configurable exclusions
# ----------------------------------------------------------------------------

EXCLUDED_DIRS=(
    "drafts"
    "templates"
)

# ----------------------------------------------------------------------------
# Pretty title helper
# ----------------------------------------------------------------------------

pretty_title() {
    local name="$1"

    name="${name%.md}"
    name="${name//-/ }"
    name="${name//_/ }"

    echo "$name" | awk '
    {
        for(i=1;i<=NF;i++) {
            $i=toupper(substr($i,1,1)) substr($i,2)
        }
        print
    }'
}

# ----------------------------------------------------------------------------
# Exclusion predicate builder
# ----------------------------------------------------------------------------

build_find_excludes() {
    local args=()

    for dir in "${EXCLUDED_DIRS[@]}"; do
        args+=(-not -path "*/${dir}/*")
    done

    echo "${args[@]}"
}

# ----------------------------------------------------------------------------
# Generate SUMMARY.md if missing or empty
# ----------------------------------------------------------------------------

generate_summary() {

    echo "==> Generating SUMMARY.md"

    local excludes
    excludes=$(build_find_excludes)

    {
        echo "# Summary"
        echo

        eval find "\"${BOOK_DIR}\"" \
            -type f \
            -name "\"*.md\"" \
            ! -name "\"SUMMARY.md\"" \
            ${excludes} \
            "|" sort \
        | while read -r file; do

            rel="${file#${BOOK_DIR}/}"

            # README.md becomes directory entry
            if [[ "$(basename "$file")" == "README.md" ]]; then
                title="$(pretty_title "$(basename "$(dirname "$file")")")"
            else
                title="$(pretty_title "$(basename "$file")")"
            fi

            depth=$(awk -F/ '{print NF-1}' <<< "${rel}")

            indent=""
            for ((i=0; i<depth; i++)); do
                indent="${indent}  "
            done

            echo "${indent}- [${title}](${rel})"

        done

    } > "${SUMMARY_FILE}"
}

# ----------------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------------

echo "==> Starting documentation build"

mkdir -p "${OUTPUT_DIR}"

if [ ! -d "${BOOK_DIR}" ]; then
    echo "ERROR: docs directory not found: ${BOOK_DIR}"
    exit 1
fi

if [ ! -f "${SUMMARY_FILE}" ] || [ ! -s "${SUMMARY_FILE}" ]; then
    generate_summary
fi

# ----------------------------------------------------------------------------
# Shared assets
# ----------------------------------------------------------------------------

if [ -d /opt/docs-shared ]; then
    echo "==> Copying shared assets"

    mkdir -p "${SRC_DIR}/docs-shared/"
    cp -r \
        /opt/docs-shared/* \
        "${SRC_DIR}/docs-shared/" \
        || true
fi

# ----------------------------------------------------------------------------
# Doxygen
# ----------------------------------------------------------------------------

if [ -f "${SRC_DIR}/Doxyfile" ]; then
    echo "==> Running Doxygen"
    doxygen "${SRC_DIR}/Doxyfile"
    cp -r "${DOXYGEN_GEN_DIR}/html" "${OUTPUT_DIR}/doxygen" \
        || echo "WARNING: Doxygen output not found, skipping copy"
fi

# ----------------------------------------------------------------------------
# mdBook
# ----------------------------------------------------------------------------

if [ -f "${SRC_DIR}/book.toml" ]; then
    echo "==> Running mdBook"

    mdbook build \
        "${SRC_DIR}" \
        --dest-dir "${OUTPUT_DIR}/book"
fi

echo "==> Documentation build completed"

EOF

RUN chmod +x /usr/local/bin/build-docs

ENTRYPOINT ["build-docs"]