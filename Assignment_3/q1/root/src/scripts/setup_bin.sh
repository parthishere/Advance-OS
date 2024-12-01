#!/bin/bash

# Script to analyze and copy shared library dependencies to organized folders
# Usage: ./copy_ldd_deps.sh <source_binary> <destination_dir>

# Check if correct number of arguments provided
if [ "$#" -ne  1 ]; then
    echo "Usage: $0 <source_binary> "
    echo "Example: $0 /usr/bin/python3"
    exit 1
fi

SOURCE_BINARY="$1"
DEST_BASE_DIR="../.."

# Check if source binary exists
if [ ! -f "$SOURCE_BINARY" ]; then
    echo "Error: Source binary '$SOURCE_BINARY' not found"
    exit 1
fi


# Create directories based on library locations and copy files
ldd "$SOURCE_BINARY" | while read -r line; do
    # Skip lines that don't contain '=>'
    if [[ "$line" == *"=>"* ]]; then
        # Extract library path
        lib_path=$(echo "$line" | awk '{print $3}')

        # Skip if library not found
        if [[ "$lib_path" == "not" ]] || [[ -z "$lib_path" ]]; then
            continue
        fi

        # Get directory of the library
        lib_dir=$(dirname "$lib_path")
        
        # Copy library file
        if [ -f "$lib_path" ]; then
            echo "Copying $lib_path to $DEST_BASE_DIR$lib_dir"
            cp -L "$lib_path" "$DEST_BASE_DIR$lib_dir"
        else
            echo "Warning: Library $lib_path not found"
        fi
    else
        lib_path=$(echo "$line" | awk '{print $1}')
        lib_dir=$(dirname "$lib_path")
        
        if [ -f "$lib_path" ]; then
            echo "Copying dynamic linker $lib_path to $DEST_BASE_DIR$lib_dir"
            cp -L "$lib_path" "$DEST_BASE_DIR$lib_dir"
        else
            echo "Warning: Dynamic linker $lib_path not found"
        fi
    fi

    
done

echo "Library dependencies have been copied to $DEST_BASE_DIR"

# Generate summary report
echo -e "\nSummary Report:" > "$DEST_BASE_DIR/dependency_report.txt"
echo "Binary analyzed: $SOURCE_BINARY" >> "$DEST_BASE_DIR/dependency_report.txt"
echo -e "\nLibrary Dependencies:" >> "$DEST_BASE_DIR/dependency_report.txt"
ldd "$SOURCE_BINARY" >> "$DEST_BASE_DIR/dependency_report.txt"
