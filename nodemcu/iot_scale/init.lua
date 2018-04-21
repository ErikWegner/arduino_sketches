MQTT_HOST = nil
MQTT_PORT = nil
mqtt_connector = nil
mqtt_client = nil
mqtt_status = " "
mqtt_delay = 0
send_status = " "
send_reset_delay = 0

config_loaded = false
wifi_counter = 0

base = -225603
scale = 21.49584
lastweightvalue = 0.0
lastweightcounter = 0

function start()
    uart.setup(1, 9600, 8, uart.PARITY_NONE, uart.STOPBITS_1)
    uart.write(1, string.char(254) .. string.char(1) .. 'Start')
    if file.exists("device.config.lua") then
        dofile("device.config.lua")
        hx711.init(5,6)
        mqtt_connector = mqtt.Client(MQTT_CLIENTID, 120, MQTT_USER, MQTT_PASSWORD)
        mqtt_connector:on("offline", function(client)
            mqtt_client = nil
            mqtt_status = "O"
            mqtt_delay = 5
        end)
        uart.write(1, string.char(254) .. string.char(1) .. 'Geladen')
        station_cfg={}
        station_cfg.ssid = WIFI_SSID
        station_cfg.pwd = WIFI_PASSWORD
        wifi.setmode(wifi.STATION)
        wifi.sta.config(station_cfg)
        wifi.sta.connect()
        config_loaded = true
    else
        uart.write(1, string.char(254) .. string.char(1) .. 'Konfiguration fehlerhaft')
    end
end

function loopfunc()
    if not config_loaded then return end
    local wifi_status
    if wifi.sta.getip ( ) == nil then
    wifi_status = (wifi_counter % 2 == 0) and "W . " or "W : "
        wifi_counter = wifi_counter + 1
    else
        wifi_status = "W ^ "
        if mqtt_delay > 0 then
            mqtt_delay = mqtt_delay - 1
            if mqtt_delay == 0 then
                mqtt_status = " "
            end
        end
        if mqtt_status == " " then
            mqtt_status = "-"
            mqtt_connector:connect(MQTT_HOST, MQTT_PORT, 1,
            function(client)
                mqtt_status = "C"
                mqtt_client = client
            end,
            function(client, reason)
                mqtt_client = nil
                mqtt_status = reason
                mqtt_delay = 5
            end)
        end
    end

    local raw_data = hx711.read(0)
    local weightvalue = math.floor((raw_data - base) / scale / 100)/10
    if weightvalue < 0 then
        weightvalue = 0.0
    end
    
    if lastweightvalue > (weightvalue - 0.2) and lastweightvalue < (weightvalue + 0.2) then
        if lastweightcounter < 5 and lastweightvalue > 0 then
            lastweightcounter = lastweightcounter + 1
        end
    else
        lastweightcounter = 0
        lastweightvalue = weightvalue
    end

    if lastweightcounter >= 5 and send_reset_delay == 0 then
        if mqtt_client == nil then
            send_status = "?"
        else
            mqtt_client:publish(
                "/iot/scale", 
                tostring(weightvalue), 0, 0,
                function(client)
                    send_status = "+"
                    send_reset_delay = 10
                end)
       end
    end

    if send_reset_delay > 0 then
        send_reset_delay = send_reset_delay - 1
        if send_reset_delay == 0 then
            send_status = " "
        end
    end
    
    uart.write(1, string.char(254) .. string.char(0x01) .. wifi_status .. 
     string.format("%6.1f", weightvalue) .. " kg   " .. mqtt_status .. " " .. send_status .. " " .. tostring(lastweightcounter))
end

if tmr.create():alarm(250, tmr.ALARM_SINGLE, start)
then
    tmr.create():alarm(1000, tmr.ALARM_AUTO, loopfunc)
end
