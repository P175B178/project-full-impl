@echo off
if not exist device\.env (
    copy device\.env.sample device\.env >nul
    echo Created device\.env from device\.env.sample - fill in your Pi connection details.
)
