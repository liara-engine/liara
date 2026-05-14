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

# -----------------------------------------------------------------------------
# Process resources and replacements
# -----------------------------------------------------------------------------

if [ -f /usr/local/bin/processor.py ]; then
    echo "==> Running resource and replacement processors"
    python3 /usr/local/bin/processor.py
else
    echo "WARNING: processor.py not found, skipping resource and replacement processing"
fi

# ----------------------------------------------------------------------------
# Doxygen
# ----------------------------------------------------------------------------

if [ -f "${SRC_DIR}/Doxyfile" ]; then
    echo "==> Running Doxygen"

    mkdir -p "${DOXYGEN_GEN_DIR}"
    (cat "${SRC_DIR}/Doxyfile"; echo "OUTPUT_DIRECTORY = ${DOXYGEN_GEN_DIR}") | doxygen -

    if [ -d "${DOXYGEN_GEN_DIR}/html" ]; then
        cp -r "${DOXYGEN_GEN_DIR}/html" "${OUTPUT_DIR}/doxygen"
    else
        echo "WARNING: Doxygen HTML output not found in ${DOXYGEN_GEN_DIR}/html"
    fi
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

# ----------------------------------------------------------------------------
# manifest.json validation
# ----------------------------------------------------------------------------

if [ -f "${SRC_DIR}/manifest.json" ]; then
    echo "==> Validating manifest.json"

    SCHEMA_URL=$(grep -oP '"\$schema"\s*:\s*"\K[^"]+' "${SRC_DIR}/manifest.json" || true)

    if [ -z "${SCHEMA_URL}" ]; then
        echo "ERROR: manifest.json does not declare a \$schema"
        exit 1
    fi

    TMP_SCHEMA="/tmp/module-manifest.schema.json"

    echo "==> Downloading schema: ${SCHEMA_URL}"

    curl -fsSL "${SCHEMA_URL}" -o "${TMP_SCHEMA}"

    ajv validate \
        -s "${TMP_SCHEMA}" \
        -d "${SRC_DIR}/manifest.json" \
        --strict=true

    echo "==> manifest.json is valid"

    echo "==> Copying manifest.json to output directory"
    cp "${SRC_DIR}/manifest.json" "${OUTPUT_DIR}/"
else
    echo "ERROR: manifest.json not found"
    exit 1
fi