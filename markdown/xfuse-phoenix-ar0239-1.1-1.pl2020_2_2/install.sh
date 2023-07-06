#!/bin/bash

# Install each of the RPM packages needed for Phoenix HDR App.
#
# Order matters, the first ones that are installed are the dependencies for later packages.
#

dnf localinstall -y kernel-module-xilinx-dummy-src-5.4.0-xilinx-v2020.2-1.0-1.pl2020_2_2.zynqmp_generic.rpm
dnf localinstall -y xilinx-dummy-1.0-1.pl2020_2_2.zynqmp_generic.rpm
dnf localinstall -y xfuse-mosquitto-1.6.1-1.pl2020_2_2.aarch64.rpm
dnf localinstall -y xfuse-algos-1.1-1.pl2020_2_2.aarch64.rpm
dnf localinstall -y xfuse-ar0239-1.1-1.pl2020_2_2.zynqmp_generic.rpm
dnf localinstall -y xfuse-phoenix-ar0239-1.1-1.pl2020_2_2.zynqmp_generic.rpm
dnf localinstall -y xfuse-pi-hdr-239-tz-1.1-1.pl2020_2_2.zynqmp_generic.rpm
dnf localinstall -y xfuse-packagegroup-kv260-phoenix-ar0239-1.1-1.pl2020_2_2.noarch.rpm

