#!/bin/bash
# Copies into a .app any Homebrew libraries macdeployqt left behind, and points
# every reference at the bundled copy.
#
# macdeployqt only follows absolute library paths. Homebrew libraries refer to
# some of their siblings as @rpath/... (libbrotlidec -> libbrotlicommon,
# libwebp -> libsharpyuv), so those never get copied. The app then builds and
# signs cleanly but aborts at launch, on any Mac without those Homebrew
# packages, with "Could not load the Qt platform plugin cocoa".
#
# Exits non-zero if a reference cannot be resolved, so a bundle that would not
# start fails the build instead of being published. Run this before signing:
# install_name_tool invalidates existing signatures.
#
# Usage: bundle_missing_dylibs.sh path/to/noddy.app
set -euo pipefail

APP=${1:?usage: bundle_missing_dylibs.sh path/to/noddy.app}
FRAMEWORKS="$APP/Contents/Frameworks"
REF_PREFIX="@executable_path/../Frameworks"
BREW_PREFIX=${HOMEBREW_PREFIX:-$(brew --prefix)}

find_homebrew_lib() {
    local candidate
    for candidate in "$BREW_PREFIX/lib/$1" "$BREW_PREFIX"/opt/*/lib/"$1"; do
        if [ -e "$candidate" ]; then
            echo "$candidate"
            return 0
        fi
    done
    return 1
}

unresolved=0
changed=1
# Repeat until stable: a newly copied library can bring dependencies of its own.
while [ "$changed" -eq 1 ]; do
    changed=0
    while IFS= read -r file; do
        case "$(file -b "$file")" in *Mach-O*) ;; *) continue ;; esac
        own_id=$(otool -D "$file" | tail -n +2)

        while IFS= read -r dep; do
            [ "$dep" = "$own_id" ] && continue
            case "$dep" in
                /usr/lib/*|/System/*) continue ;;
                "$REF_PREFIX"/*)
                    [ -e "$FRAMEWORKS/${dep#"$REF_PREFIX"/}" ] && continue ;;
            esac

            name=${dep##*/}
            case "$name" in
                *.dylib) ;;
                *)
                    echo "error: ${file##*/} needs $dep, which is not bundled" >&2
                    unresolved=1
                    continue ;;
            esac

            if [ ! -e "$FRAMEWORKS/$name" ]; then
                if ! source_lib=$(find_homebrew_lib "$name"); then
                    echo "error: ${file##*/} needs $dep, not found under $BREW_PREFIX" >&2
                    unresolved=1
                    continue
                fi
                echo "Bundling $name"
                cp -L "$source_lib" "$FRAMEWORKS/$name"
                chmod u+w "$FRAMEWORKS/$name"
                install_name_tool -id "$REF_PREFIX/$name" "$FRAMEWORKS/$name" 2>/dev/null
                changed=1
            fi

            if [ "$dep" != "$REF_PREFIX/$name" ]; then
                install_name_tool -change "$dep" "$REF_PREFIX/$name" "$file" 2>/dev/null
                changed=1
            fi
        done < <(otool -L "$file" | tail -n +2 | awk '{print $1}')
    done < <(find "$APP/Contents" -type f)
done

exit "$unresolved"
