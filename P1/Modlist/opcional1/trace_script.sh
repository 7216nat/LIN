#!/bin/bash

cd /sys/kernel/debug/tracing
echo 0 > tracing_on
echo function > current_tracer
echo modlist_read > set_ftrace_filter
echo 1 > tracing_on
cat /sys/kernel/debug/tracing/trace_pipe

