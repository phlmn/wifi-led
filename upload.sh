#!/usr/bin/env sh
UPLOAD_PORT=/dev/`ls /dev | grep cu.wchusbserial`
echo "Detected device: $UPLOAD_PORT"
platformio run -t upload --upload-port $UPLOAD_PORT
