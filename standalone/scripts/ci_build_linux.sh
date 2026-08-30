#!/bin/bash
meson setup build
meson configure -Dbuild_target=linux build
meson compile -C build
