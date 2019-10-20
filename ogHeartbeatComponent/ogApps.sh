#!/bin/sh
# Copyright 2019 Tyler Listoe 
#
# Simply start and stop all apps



case "$1" in
    start)
        /legato/systems/current/bin/app start imuLogger
        /legato/systems/current/bin/app start gnssLogger
        /legato/systems/current/bin/app start rfidTemp
        ;;
    monitor)
        ;;
    stop)
        /legato/systems/current/bin/app stop imuLogger
        /legato/systems/current/bin/app stop gnssLogger
        /legato/systems/current/bin/app stop rfidTemp
        ;;
    *)
        exit 1
        ;;
esac

exit 0
