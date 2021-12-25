#!/bin/sh

/marker_entrypoint.sh -c /config/render.conf
/entrypoint.sh -b -j $threads -c /config/render.conf

