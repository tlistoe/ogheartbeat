#!/bin/sh
# Copyright 2019 Tyler Listoe 
#
# Simply start and stop all apps



case "$1" in
    start)
        if /legato/systems/current/bin/app start imuLogger ; then
        #turn on tri green light, red off
        echo 1 > /sys/devices/platform/expander.0/tri_led_grn
        echo 0 > /sys/devices/platform/expander.0/tri_led_red
        else 
        echo 1 > /sys/devices/platform/expander.0/tri_led_red
        fi
        
        if /legato/systems/current/bin/app start gnssLogger ; then
        #turn on tri blue light, red off
        echo 1 > /sys/devices/platform/expander.0/tri_led_blu
        echo 0 > /sys/devices/platform/expander.0/tri_led_red
        else
        echo 1 > /sys/devices/platform/expander.0/tri_led_red
        fi
        /legato/systems/current/bin/app start rfidTemp
        ;;
    monitor)
        ;;
    stop)
        if /legato/systems/current/bin/app stop imuLogger ; then
        #turn off tri green light
        echo 0 > /sys/devices/platform/expander.0/tri_led_grn
        else
        echo 1 > /sys/devices/platform/expander.0/tri_led_red
        fi
        if /legato/systems/current/bin/app stop gnssLogger ; then
        # turn off tri blue light
        echo 0 > /sys/devices/platform/expander.0/tri_led_blu
        else
        echo 1 > /sys/devices/platform/expander.0/tri_led_red
        fi
        /legato/systems/current/bin/app stop rfidTemp
        ;;
    *)
        exit 1
        ;;
esac

exit 0
