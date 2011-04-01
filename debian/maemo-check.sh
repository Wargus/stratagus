#!/bin/sh
if pkg-config maemo-version 1>/dev/null 2>&1 || test -f "/etc/maemo_version" || osso-product-info -q OSSO_PRODUCT_INFO_RELEASE_NAME 2>/dev/null | grep -q -i maemo; then echo maemo; exit 0; else exit 1; fi
