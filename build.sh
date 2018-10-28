#!/bin/bash
mkdir out
protoc --cpp_out=out/ *.proto
