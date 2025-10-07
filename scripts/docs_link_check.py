#!/usr/bin/env python3
"""
Simple internal link checker for MkDocs output.

Scans the built site directory (default: _site_mkdocs) and verifies that all
relative links resolve to existing files. Absolute http(s) links are ignored.

Special handling:
- '/docs/'-prefixed absolute paths (as used in 404.html with site_url) are
  mapped to the local site root for validation.

Exit codes:
  0: OK (no broken links)
  1: Broken links found or site directory missing
"""

import os
import re
import sys


def main() -> int:
    root = os.environ.get("MKDOCS_SITE_DIR", "_site_mkdocs")
    if not os.path.isdir(root):
        print(f"Error: site directory '{root}' not found. Build docs first (mkdocs build).", file=sys.stderr)
        return 1

    broken = []
    for dirpath, _, filenames in os.walk(root):
        for fn in filenames:
            if not fn.endswith('.html'):
                continue
            path = os.path.join(dirpath, fn)
            try:
                with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                    html = f.read()
            except OSError:
                continue

            for m in re.finditer(r'href="([^"]+)"', html):
                href = m.group(1)
                if not href:
                    continue
                if href.startswith(('#', 'mailto:', 'javascript:')):
                    continue
                if href.startswith(('http://', 'https://')):
                    continue

                base = dirpath
                target = href

                # Map '/docs/..' absolute paths to local root
                if target.startswith('/docs/'):
                    target = target[len('/docs/'):]
                    full = os.path.normpath(os.path.join(root, target))
                elif target.startswith('/'):
                    full = os.path.normpath(os.path.join(root, target.lstrip('/')))
                elif target.startswith(('../', './')):
                    full = os.path.normpath(os.path.join(base, target))
                else:
                    # relative path without ./
                    full = os.path.normpath(os.path.join(base, target))

                # If the link points to a directory, expect index.html
                if os.path.isdir(full):
                    full = os.path.join(full, 'index.html')
                else:
                    # If no extension, assume directory URL and check index.html
                    _, ext = os.path.splitext(full)
                    if not ext:
                        full = os.path.join(full, 'index.html')

                # Ignore typical asset file types
                if any(full.endswith(ext) for ext in ('.css', '.js', '.png', '.jpg', '.jpeg', '.map', '.json', '.svg', '.ico', '.gif')):
                    continue

                if not os.path.exists(full):
                    # search page and other virtual pages may be missing; skip if under search/
                    if '/search/' in full:
                        continue
                    broken.append((path, href, full))

    if broken:
        print('Broken internal links found:')
        for src, href, full in broken:
            print(f'- {src} -> {href} (resolved {full})')
        print(f'Total broken: {len(broken)}')
        return 1
    else:
        print('No broken internal links detected')
        return 0


if __name__ == '__main__':
    raise SystemExit(main())

